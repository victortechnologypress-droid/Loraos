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

/* Dezactiveaza cursorul hardware care palpaie */
void k_disable_cursor() {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

/* Functie de scris text in mod text (0xB8000) */
void k_early_print(const char* str, int row) {
    volatile char* vga = (volatile char*)0xB8000;
    int offset = row * 80 * 2;
    for (int i = 0; str[i] != '\0'; i++) {
        vga[offset + i*2] = str[i];
        vga[offset + i*2 + 1] = 0x0B; /* Cyan */
    }
}

void kernel_main(uint32_t magic, multiboot_info_t* mbi) {
    /* Oprim cursorul imediat */
    k_disable_cursor();

    if (magic != 0x2BADB002) {
        k_early_print("EROARE: Bootloader incompatibil!", 0);
        for(;;) __asm__ volatile("hlt");
    }

    /* Afisare Logo */
    k_early_print(">>================================================<<", 2);
    k_early_print("||        LORA OS - MADE BY VICTOR ROSCA          ||", 3);
    k_early_print(">>================================================<<", 4);

    /* --- DEBUG PAS CU PAS --- */
    k_early_print("Pas 1: Initializare RAM...", 6);
    uint32_t total_ram_kb = mbi->mem_upper + 1024;
    pmm_init(total_ram_kb);
    k_early_print("OK!", 7);

    k_early_print("Pas 2: Initializare Video (VBE)...", 9);
    vbe_init(); 
    /* Daca sistemul ingheata aici, nu vei vedea mesajul de mai jos */
    k_early_print("OK!", 10);

    k_early_print("Pas 3: Initializare Drivere Input...", 12);
    keyboard_init();
    ps2mouse_init();
    k_early_print("OK!", 13);

    k_early_print("Pas 4: Pornire Scheduler si Desktop...", 15);
    scheduler_init();
    desktop_init();
    
    k_early_print("Sistemul porneste interfata grafica...", 17);
    for (volatile int d = 0; d < 50000000; d++); /* Scurta pauza sa citesti */
    
    desktop_run();

    for(;;) __asm__ volatile("hlt");
}
