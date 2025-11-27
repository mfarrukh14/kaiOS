# KaiOS - Lightweight Operating System

A minimal, lightweight Linux-like operating system built from scratch in C++ and x86 assembly.

## Features

- **GRUB Bootloader**: Standard multiboot-compliant bootloader
- **VGA Graphics Mode**: 320x200 256-color graphical interface
- **Desktop Environment**: Raspberry Pi OS-style GUI with:
  - Boot splash screen with KaiOS logo
  - Desktop with icons (My PC, Files, Terminal)
  - Taskbar with Start menu and clock
  - Draggable windows with title bar and close button
  - File Manager application
- **PS/2 Mouse Driver**: Full mouse support with cursor
- **PS/2 Keyboard Driver**: Full keyboard support with shift, caps lock, ctrl
- **Memory Management**: Simple heap allocator with malloc/free
- **Persistent File System**: Disk-backed hierarchical directory structure with:
  - Root directory (`/`) with subdirectories (`bin`, `etc`, `home`, `tmp`)
  - File creation, deletion, reading, writing
  - Directory navigation
  - Automatic save to disk
- **Interactive Shell**: Command-line interface (accessible via Terminal)
- **Timer (PIT)**: Programmable Interval Timer at 100 Hz

## Shell Commands

| Command | Description |
|---------|-------------|
| `help` | Show available commands |
| `clear` | Clear the screen |
| `echo` | Print text to screen |
| `ls` / `dir` | List directory contents |
| `cd` | Change directory |
| `pwd` | Print working directory |
| `mkdir` | Create a directory |
| `rmdir` | Remove an empty directory |
| `touch` | Create an empty file |
| `rm` | Remove a file |
| `cat` | Display file contents |
| `write` | Write text to a file |
| `cp` | Copy a file |
| `mv` | Move/rename a file |
| `free` | Show memory usage |
| `uname` | Show system information |
| `date` | Show current date |
| `uptime` | Show system uptime |
| `reboot` | Reboot the system |
| `shutdown` | Halt the system |

## Building

### Prerequisites

You need a cross-compiler for i686-elf target:

```bash
# Install build dependencies (Ubuntu/Debian)
sudo apt install nasm qemu-system-x86 grub-pc-bin xorriso mtools

# You also need i686-elf-gcc cross-compiler
# See: https://wiki.osdev.org/GCC_Cross-Compiler
```

### Building the Kernel

```bash
# First time setup (installs dependencies)
make setup

# Build and run (preserves saved files)
make go

# Full rebuild and run (preserves saved files)  
make rebuild

# Fresh start (wipes disk, deletes all saved files)
make fresh

# Other commands:
make              # Build the kernel binary
make run-disk     # Run with persistent disk
make clean        # Clean build files (keeps disk)
make distclean    # Clean everything including disk image
```

## Project Structure

```
kaiOS/
├── include/
│   ├── kernel/
│   │   ├── types.h       # Basic type definitions
│   │   ├── idt.h         # Interrupt Descriptor Table
│   │   ├── memory.h      # Memory management
│   │   ├── string.h      # String utilities
│   │   ├── fs.h          # File system
│   │   ├── shell.h       # Shell/terminal
│   │   └── gui.h         # GUI system
│   └── drivers/
│       ├── vga.h         # VGA text driver
│       ├── graphics.h    # VGA graphics driver
│       ├── keyboard.h    # Keyboard driver
│       ├── mouse.h       # Mouse driver
│       ├── timer.h       # Timer driver
│       ├── ata.h         # ATA disk driver
│       └── io.h          # Port I/O operations
├── src/
│   ├── boot/
│   │   └── boot.asm      # Multiboot bootloader & ISRs
│   ├── kernel/
│   │   ├── kernel.cpp    # Main kernel entry
│   │   ├── idt.cpp       # Interrupt handling
│   │   ├── memory.cpp    # Heap allocator
│   │   ├── string.cpp    # String functions
│   │   ├── fs.cpp        # File system
│   │   ├── shell.cpp     # Command shell
│   │   └── gui.cpp       # Desktop environment
│   └── drivers/
│       ├── vga.cpp       # VGA text mode
│       ├── graphics.cpp  # VGA graphics mode
│       ├── keyboard.cpp  # PS/2 keyboard
│       ├── mouse.cpp     # PS/2 mouse
│       ├── timer.cpp     # PIT timer
│       └── ata.cpp       # ATA disk driver
├── isodir/
│   └── boot/
│       └── grub/
│           └── grub.cfg  # GRUB configuration
├── linker.ld             # Linker script
├── Makefile              # Build system
└── README.md             # This file
```

## Architecture

- **Target**: i686 (32-bit x86)
- **Boot Protocol**: Multiboot 1
- **Bootloader**: GRUB 2
- **Memory Model**: Flat memory model
- **Heap Size**: 4 MB

## Technical Details

### Memory Layout
- Kernel loads at 1 MB
- Stack: 16 KB
- Heap: 4 MB starting at heap area

### Interrupt Handling
- ISRs 0-31: CPU exceptions
- IRQs 0-15 (INT 32-47): Hardware interrupts
  - IRQ0 (INT 32): Timer
  - IRQ1 (INT 33): Keyboard

### File System
- Simple in-memory VFS
- Maximum 128 files/directories
- Maximum 64 KB per file
- Maximum 64 character filenames


