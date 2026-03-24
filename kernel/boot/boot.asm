; ============================================================
;  LoraOS v1 - Boot Entry Point
;  Locație: kernel/boot/boot.asm
; ============================================================

MULTIBOOT_PAGE_ALIGN  equ  1 << 0
MULTIBOOT_MEMORY_INFO equ  1 << 1
MULTIBOOT_MAGIC       equ  0x1BADB002
MULTIBOOT_FLAGS       equ  MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
MULTIBOOT_CHECKSUM    equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

section .multiboot
align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

section .bss
align 16
stack_bottom:
    resb 16384 ; 16 KB de stack
stack_top:

section .text
global _start
extern kernel_main

_start:
    ; Inițializăm stiva
    mov esp, stack_top

    ; Trimitem parametrii Multiboot către kernel_main
    push ebx ; Adresa structurii de info
    push eax ; Magic number

    call kernel_main

.halt:
    hlt
    jmp .halt
