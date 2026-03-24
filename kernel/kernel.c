#include "include/kernel.h"
#include "include/pmm.h"
#include "include/scheduler.h"
#include "../drivers/vbe.h"
#include "../drivers/keyboard.h"
#include "../drivers/ps2mouse.h"
#include "../ui/desktop.h"

typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
} multiboot_info_t;

/* Funcție pentru a scrie text în format ASCII pe ecranul Acer-ului */
void k_draw_ascii(const char* str, int row) {
    volatile char* vga = (volatile char*)0xB8000;
    int offset = row * 80 * 2;
    for (int i = 0; str[i] != '\0'; i++) {
        vga[offset + i*2] = str[i];
        vga[offset + i*2 + 1] = 0x0B; /* Cyan deschis pe negru - arată bine pe netbook */
    }
}

void kernel_main(uint32_t magic, multiboot_info_t* mbi) {
    /* 1. Verificare obligatorie Multiboot */
    if (magic != 0x2BADB002) {
        volatile char* vga = (volatile char*)0xB8000;
        const char* err = "EROARE CRITICA: Bootloader incompatibil!";
        for (int i = 0; err[i]; i++) {
            vga[i*2] = err[i];
            vga[i*2+1] = 0x4F; 
        }
        for(;;) __asm__("hlt");
    }

    /* 2. Afișare Logo LoraOS */
    k_draw_ascii(">>================================================================<<", 2);
    k_draw_ascii("||                                                                ||", 3);
    k_draw_ascii("||  `..                                     `....       `.. ..    ||", 4);
    k_draw_ascii("||  `..                                   `..    `..  `..    `..  ||", 5);
    k_draw_ascii("||  `..         `..    `. `...   `..    `..        `.. `..        ||", 6);
    k_draw_ascii("||  `..       `..  `..  `..    `..  `.. `..        `..   `..      ||", 7);
    k_draw_ascii("||  `..      `..    `.. `..   `..   `.. `..        `..      `..   ||", 8);
    k_draw_ascii("||  `..       `..  `..  `..   `..   `..   `..     `.. `..    `..  ||", 9);
    k_draw_ascii("||  `........   `..    `...     `.. `...    `....       `.. ..    ||", 10);
    k_draw_ascii(">>================================================================<<", 11);
    
    k_draw_ascii("                Made by Rosca Victor", 13);

    /* 3. Animația Loading cu puncte care apar/dispar */
    for (int loop = 0; loop < 4; loop++) {
        k_draw_ascii("Loading .      ", 15);
        for (volatile int d = 0; d < 15000000; d++);
        k_draw_ascii("Loading . .    ", 15);
        for (volatile int d = 0; d < 15000000; d++);
        k_draw_ascii("Loading . . .  ", 15);
        for (volatile int d = 0; d < 15000000; d++);
    }

    /* 4. Inițializare hardware Acer Aspire One */
    uint32_t total_ram = mbi->mem_upper + 1024;
    pmm_init(total_ram);
    
    vbe_init();
    keyboard_init();
    ps2mouse_init();
    scheduler_init();

    /* 5. Start Desktop */
    desktop_init();
    desktop_run();

    while(1) __asm__("hlt");
}
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
