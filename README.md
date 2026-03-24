# LoraOS v1 - MVP

Un sistem de operare minimal, bootabil direct de pe USB.

## Structura Proiectului

```
loraos/
├── Makefile              ← Compileaza totul intr-un ISO pentru Rufus
├── linker.ld             ← Script de link: kernel incarcat la 1MB
├── grub.cfg              ← Configuratie GRUB (bootloader)
│
├── kernel/
│   ├── boot/
│   │   └── boot.asm      ← Entry point Multiboot (primul cod executat)
│   ├── include/
│   │   ├── kernel.h      ← Tipuri de baza, I/O ports (inb/outb)
│   │   ├── pmm.h         ← Physical Memory Manager header
│   │   └── scheduler.h   ← Scheduler header
│   ├── kernel.c          ← kernel_main() - orchestreaza totul
│   ├── pmm.c             ← Gestionare RAM fizic (bitmap de pagini)
│   └── scheduler.c       ← Round-Robin scheduler cooperativ
│
├── drivers/
│   ├── vbe.h / vbe.c     ← Framebuffer VBE 1024x768 32bpp
│   ├── keyboard.h / .c   ← Driver keyboard PS/2 + scancode map
│   └── ps2mouse.h / .c   ← Driver mouse/trackpad PS/2 + tap-to-click
│
├── fs/
│   └── fat32.h / fat32.c ← Driver FAT32: citire/scriere pe USB
│
└── ui/
    ├── desktop.h / .c    ← Desktop manager + event loop
    ├── taskbar.h / .c    ← Taskbar cu ceas din BIOS RTC
    └── window.h / .c     ← Window manager de baza
```

## Cum Compilezi

### Prerequisite (Linux / WSL)
```bash
# Toolchain cross-compiler pentru i686-elf
sudo apt install gcc-i686-elf nasm grub-pc-bin grub-common xorriso

# QEMU pentru testare
sudo apt install qemu-system-x86
```

### Build
```bash
make        # Compileaza kernel-ul
make iso    # Genereaza loraos.iso
make run    # Testeaza in QEMU
make clean  # Sterge build-urile
```

### Scriere pe USB cu Rufus
1. Descarca [Rufus](https://rufus.ie)
2. Selecteaza `loraos.iso`
3. Selecteaza stick-ul USB
4. Partition scheme: **MBR** (pentru compatibilitate maxima)
5. Click **START**

## Specificatii Tehnice

| Component | Detaliu |
|---|---|
| Bootloader | GRUB 2 / Multiboot 1 |
| Arhitectura | x86 32-bit (i686) |
| Grafică | VBE Framebuffer 1024×768 @ 32bpp |
| Memorie | PMM bitmap, suport 4GB |
| Input | PS/2 keyboard + mouse cu tap-to-click |
| Stocare | FAT32 read/write pe USB |
| Scheduler | Round-Robin cooperativ (2+ procese) |
| Ceas | BIOS RTC (citit direct din CMOS) |

## Roadmap v2

- [ ] IDT + IRQ (intreruperi hardware reale)
- [ ] Scheduler preemptiv (IRQ0 - PIT timer)
- [ ] Font complet 8x8 pentru toate caracterele ASCII
- [ ] Keyboard input complet in terminal
- [ ] FAT32 write + creare fisiere noi
- [ ] Meniu Start functional
- [ ] Driver NVMe/USB mass storage nativ

## Licenta

LoraOS este open-source. Construieste, modifica, distribuie liber.
