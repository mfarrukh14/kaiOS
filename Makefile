# KaiOS Makefile
# Build system for the lightweight operating system

# Tools
AS = nasm
CC = g++
LD = ld

# Flags
ASFLAGS = -f elf32
CFLAGS = -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -nostdlib -fno-builtin -fno-stack-protector -m32 -fno-pie -no-pie -I.
LDFLAGS = -T linker.ld -nostdlib -m elf_i386

# Directories
SRC_DIR = src
BUILD_DIR = build
ISO_DIR = isodir

# Source files
ASM_SOURCES = $(SRC_DIR)/boot/boot.asm
CPP_SOURCES = $(SRC_DIR)/kernel/kernel.cpp \
              $(SRC_DIR)/kernel/idt.cpp \
              $(SRC_DIR)/kernel/memory.cpp \
              $(SRC_DIR)/kernel/string.cpp \
              $(SRC_DIR)/kernel/fs.cpp \
              $(SRC_DIR)/kernel/shell.cpp \
              $(SRC_DIR)/kernel/gui.cpp \
              $(SRC_DIR)/drivers/vga.cpp \
              $(SRC_DIR)/drivers/keyboard.cpp \
              $(SRC_DIR)/drivers/timer.cpp \
              $(SRC_DIR)/drivers/ata.cpp \
              $(SRC_DIR)/drivers/mouse.cpp \
              $(SRC_DIR)/drivers/graphics.cpp

# Object files
ASM_OBJECTS = $(BUILD_DIR)/boot.o
CPP_OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CPP_SOURCES))

# Output
KERNEL = $(BUILD_DIR)/kaios.bin
ISO = kaios.iso
DISK = kaios.img

# Default target
all: $(KERNEL)

# Create build directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)/kernel
	mkdir -p $(BUILD_DIR)/drivers

# Create disk image (10 MB)
$(DISK):
	dd if=/dev/zero of=$(DISK) bs=1M count=10

# Assemble boot code
$(BUILD_DIR)/boot.o: $(SRC_DIR)/boot/boot.asm | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# Compile kernel C++ files
$(BUILD_DIR)/kernel/%.o: $(SRC_DIR)/kernel/%.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile driver C++ files
$(BUILD_DIR)/drivers/%.o: $(SRC_DIR)/drivers/%.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link kernel
$(KERNEL): $(ASM_OBJECTS) $(CPP_OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

# Create bootable ISO
iso: $(KERNEL)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL) $(ISO_DIR)/boot/kaios.bin
	grub-mkrescue -o $(ISO) $(ISO_DIR)

# Run in QEMU (no disk)
run: $(KERNEL)
	qemu-system-i386 -kernel $(KERNEL)

# Run in QEMU with persistent disk (recommended)
run-disk: $(KERNEL) $(DISK)
	qemu-system-i386 -kernel $(KERNEL) -drive file=$(DISK),format=raw,if=ide

# Run ISO in QEMU with disk (best for reboots)
run-iso: iso $(DISK)
	qemu-system-i386 -cdrom $(ISO) -drive file=$(DISK),format=raw,if=ide -boot d

# Quick run - shows usage
go: all $(DISK)
	@echo "Usage: make go-gui  (graphical mode)"
	@echo "       make go-term (terminal mode)"

# Run in GUI mode
go-gui: all $(DISK)
	qemu-system-i386 -kernel $(KERNEL) -drive file=$(DISK),format=raw,if=ide -append "mode=gui"

# Run in Terminal mode
go-term: all $(DISK)
	qemu-system-i386 -kernel $(KERNEL) -drive file=$(DISK),format=raw,if=ide -append "mode=term"

# Full rebuild and run GUI (preserves disk data)
rebuild: clean all $(DISK)
	qemu-system-i386 -kernel $(KERNEL) -drive file=$(DISK),format=raw,if=ide -append "mode=gui"

# Fresh start - wipes disk and rebuilds (GUI mode)
fresh: distclean all $(DISK)
	qemu-system-i386 -kernel $(KERNEL) -drive file=$(DISK),format=raw,if=ide -append "mode=gui"

# Run with debug output
debug: $(KERNEL) $(DISK)
	qemu-system-i386 -kernel $(KERNEL) -drive file=$(DISK),format=raw,if=ide -d int -no-reboot -no-shutdown

# Clean build files
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(ISO)

# Clean everything including disk image
distclean: clean
	rm -f $(DISK)

# Install dependencies (Ubuntu/Debian)
deps:
	sudo apt install -y nasm qemu-system-x86 grub-pc-bin xorriso mtools g++ make

# Setup: Install deps + build + create disk + run
setup: deps all $(DISK)
	@echo "Setup complete! Run 'make go' to start KaiOS"

.PHONY: all iso run run-disk run-iso go rebuild fresh debug clean distclean deps setup
