#include "include/kernel.h"
#include "include/pmm.h"
#include "include/scheduler.h"
#include "../drivers/vbe.h"
#include "../drivers/keyboard.h"
#include "../drivers/ps2mouse.h"
#include "../ui/desktop.h"

/* Structura Multiboot necesara pentru info RAM */
typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
} multiboot_info_t;

/* Functie pentru desenat Logo pe Acer Aspire One */
void k_early_print(const char* str, int row) {
    volatile char* vga = (volatile char*)0xB8000;
    int offset = row * 80 * 2;
    for (int i = 0; str[i] != '\0'; i++) {
        vga[offset + i*2] = str[i];
        vga[offset + i*2 + 1] = 0x0B; /* Cyan */
    }
}

void kernel_main(uint32_t magic, multiboot_info_t* mbi) {
    /* 1. Verificam daca am bootat corect */
    if (magic != 0x2BADB002) {
        volatile char* vga = (volatile char*)0xB8000;
        const char* msg = "BOOT ERROR: Non-Multiboot!";
        for (int i = 0; msg[i]; i++) {
            vga[i*2] = msg[i];
            vga[i*2+1] = 0x4F;
        }
        for(;;) { __asm__ volatile("hlt"); }
    }

    /* 2. Afisare Logo ASCII LoraOS */
    k_early_print(">>================================================================<<", 2);
    k_early_print("||  `..                                     `....       `.. ..    ||", 4);
    k_early_print("||  `..                                   `..    `..  `..    `..  ||", 5);
    k_early_print("||  `..         `..    `. `...   `..    `..        `.. `..        ||", 6);
    k_early_print("||  `..       `..  `..  `..    `..  `.. `..        `..   `..      ||", 7);
    k_early_print("||  `..      `..    `.. `..   `..   `.. `..        `..      `..   ||", 8);
    k_early_print("||  `..       `..  `..  `..   `..   `..   `..     `.. `..    `..  ||", 9);
    k_early_print("||  `........   `..    `...     `.. `...    `....       `.. ..    ||", 10);
    k_early_print(">>================================================================<<", 12);
    k_early_print("                Made by Rosca Victor", 14);

    /* 3. Animatie Loading */
    for (int i = 0; i < 2; i++) {
        k_early_print("Loading .      ", 16);
        for (volatile int d = 0; d < 10000000; d++);
        k_early_print("Loading . .    ", 16);
        for (volatile int d = 0; d < 10000000; d++);
        k_early_print("Loading . . .  ", 16);
        for (volatile int d = 0; d < 10000000; d++);
    }

    /* 4. Initializare Hardware */
    uint32_t total_ram_kb = mbi->mem_upper + 1024;
    pmm_init(total_ram_kb);
    
    vbe_init();
    keyboard_init();
    ps2mouse_init();
    scheduler_init();

    /* 5. Pornire Desktop */
    desktop_init();
    desktop_run();

    /* Bucla infinita de siguranta */
    for(;;) { __asm__ volatile("hlt"); }
}
