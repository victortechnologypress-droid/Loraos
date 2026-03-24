/* kernel/kernel.c
 * LoraOS v1 MVP - Kernel principal
 * Prima functie C care ruleaza dupa boot
 */

#include "include/kernel.h"
#include "include/pmm.h"
#include "include/scheduler.h"
#include "../drivers/vbe.h"
#include "../drivers/keyboard.h"
#include "../drivers/ps2mouse.h"
#include "../ui/desktop.h"

/* Structura Multiboot (primita de la GRUB) */
typedef struct {
    uint32_t flags;
    uint32_t mem_lower;     /* KB de RAM sub 1MB */
    uint32_t mem_upper;     /* KB de RAM deasupra 1MB */
    uint32_t boot_device;
    uint32_t cmdline;
    /* ... alte campuri pe care le vom adauga ulterior */
} multiboot_info_t;

/* ============================================================
 *  kernel_main - Entry point C
 *  Apelat din boot.asm cu: magic si multiboot_info*
 * ============================================================ */
void kernel_main(uint32_t magic, multiboot_info_t* mbi)
{
    /* 1. VERIFICARE MULTIBOOT
     * Daca magic-ul e gresit, ceva e in neregula cu boot-ul */
    if (magic != 0x2BADB002) {
        /* Panic: scriem direct in memoria video text-mode */
        volatile char* vga = (volatile char*)0xB8000;
        const char* msg = "BOOT ERROR: Nu e GRUB/Multiboot!";
        for (int i = 0; msg[i]; i++) {
            vga[i*2]   = msg[i];
            vga[i*2+1] = 0x4F; /* Alb pe rosu */
        }
        for(;;) { __asm__ volatile("hlt"); }
    }

    /* 2. PHYSICAL MEMORY MANAGER
     * Initializam gestionarea RAM-ului
     * mbi->mem_upper = KB disponibili deasupra 1MB */
    uint32_t total_ram_kb = mbi->mem_upper + 1024;
    pmm_init(total_ram_kb);

    /* 3. VBE FRAMEBUFFER (Grafica)
     * GRUB a setat deja modul grafic inainte de a ne apela.
     * Noi preluam framebuffer-ul si il folosim pentru desen. */
    vbe_init();

    /* 4. DRIVERE INPUT */
    keyboard_init();
    ps2mouse_init();

    /* 5. SCHEDULER (Round-Robin simplu)
     * Initializam task-urile de baza ale OS-ului */
    scheduler_init();

    /* 6. DESKTOP UI
     * Desenam desktop-ul, taskbar-ul si pornim event loop-ul */
    desktop_init();
    desktop_run(); /* <-- Bucla principala, nu returneaza */

    /* Niciodata nu ajungem aici */
    for(;;) { __asm__ volatile("hlt"); }
}
