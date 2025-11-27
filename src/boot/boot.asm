; KaiOS Bootloader - Multiboot compatible boot code
; This file contains the multiboot header and initial boot code

MBALIGN  equ  1 << 0            ; Align loaded modules on page boundaries
MEMINFO  equ  1 << 1            ; Provide memory map
FLAGS    equ  MBALIGN | MEMINFO ; Multiboot flags
MAGIC    equ  0x1BADB002        ; Multiboot magic number
CHECKSUM equ -(MAGIC + FLAGS)   ; Checksum to prove we are multiboot

; Multiboot header
section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

; Stack setup
section .bss
align 16
stack_bottom:
    resb 16384              ; 16 KB stack
stack_top:

; Entry point
section .text
global _start
extern kernel_main

_start:
    ; Set up the stack
    mov esp, stack_top
    
    ; Push multiboot info pointer and magic number
    push ebx                ; Multiboot info structure pointer
    push eax                ; Multiboot magic number
    
    ; Call kernel main
    call kernel_main
    
    ; If kernel returns, hang
    cli
.hang:
    hlt
    jmp .hang

; GDT (Global Descriptor Table) for protected mode
global gdt_flush
gdt_flush:
    mov eax, [esp + 4]
    lgdt [eax]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush
.flush:
    ret

; IDT (Interrupt Descriptor Table) loading
global idt_flush
idt_flush:
    mov eax, [esp + 4]
    lidt [eax]
    ret

; ISR (Interrupt Service Routine) handlers
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    push dword 0            ; Dummy error code
    push dword %1           ; Interrupt number
    jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
global isr%1
isr%1:
    push dword %1           ; Interrupt number
    jmp isr_common_stub
%endmacro

; Define ISRs
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE 8
ISR_NOERRCODE 9
ISR_ERRCODE 10
ISR_ERRCODE 11
ISR_ERRCODE 12
ISR_ERRCODE 13
ISR_ERRCODE 14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

; IRQ handlers
%macro IRQ 2
global irq%1
irq%1:
    push dword 0
    push dword %2
    jmp irq_common_stub
%endmacro

IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

extern isr_handler
extern irq_handler

isr_common_stub:
    pusha
    mov ax, ds
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    push esp              ; Push pointer to registers_t structure
    call isr_handler
    add esp, 4            ; Clean up pushed pointer
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
    add esp, 8
    sti
    iret

irq_common_stub:
    pusha
    mov ax, ds
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    push esp              ; Push pointer to registers_t structure
    call irq_handler
    add esp, 4            ; Clean up pushed pointer
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
    add esp, 8
    sti
    iret
