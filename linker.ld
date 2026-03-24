/* linker.ld - Script de link pentru LoraOS kernel */
/* Kernelul este incarcat de GRUB la adresa fizica 1MB */

ENTRY(_start)

SECTIONS
{
    /* GRUB incarca kernelul la 1MB (0x100000) */
    . = 0x100000;

    /* Sectiunea de cod executabil */
    .text ALIGN(4K) :
    {
        *(.multiboot)   /* Header-ul Multiboot TREBUIE sa fie primul */
        *(.text)
    }

    /* Date read-only (constante, string-uri) */
    .rodata ALIGN(4K) :
    {
        *(.rodata)
    }

    /* Date initializate (variabile globale cu valoare) */
    .data ALIGN(4K) :
    {
        *(.data)
    }

    /* BSS: variabile globale neinitializate (zero la boot) */
    .bss ALIGN(4K) :
    {
        *(COMMON)
        *(.bss)
    }
}
