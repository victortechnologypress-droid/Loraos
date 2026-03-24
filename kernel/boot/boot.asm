; kernel/boot/boot.asm
; Entry point-ul kernelului LoraOS
; GRUB cauta header-ul Multiboot si sare la _start

; ============================================================
;  Constante Multiboot (GRUB le recunoaste automat)
; ============================================================
MULTIBOOT_MAGIC     equ 0x1BADB002
MULTIBOOT_FLAGS     equ 0x00000003   ; Bit0=aliniaza module, Bit1=info memorie
MULTIBOOT_CHECKSUM  equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

section .multiboot
align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

; ============================================================
;  Stack initial (16KB)
; ============================================================
section .bss
align 16
stack_bottom:
    resb 16384          ; 16 KB de stack
stack_top:

; ============================================================
;  Entry point: GRUB sare aici
; ============================================================
section .text
global _start
extern kernel_main      ; Definit in kernel/kernel.c

_start:
    ; 1. Seteaza stack-ul
    mov esp, stack_top

    ; 2. Salveaza informatiile Multiboot inainte de orice altceva
    ;    eax = magic number (0x2BADB002 daca vine de la GRUB)
    ;    ebx = adresa structurii multiboot_info
    push ebx            ; multiboot_info* ptr -> param 2
    push eax            ; magic number       -> param 1

    ; 3. Sari la kernel-ul C
    call kernel_main

    ; 4. Daca kernel_main returneaza (nu ar trebui), halt
.hang:
    cli                 ; Dezactiveaza intreruperile
    hlt                 ; Opreste procesorul
    jmp .hang           ; Loop de siguranta
