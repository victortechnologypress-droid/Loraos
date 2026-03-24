/* kernel/include/kernel.h
 * Tipuri de baza si utilitare comune pentru LoraOS
 * (Nu avem stdlib, deci le definim noi) */

#ifndef KERNEL_H
#define KERNEL_H

/* ---- Tipuri de baza (fara stdlib) ---- */
typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;
typedef signed char         int8_t;
typedef signed short        int16_t;
typedef signed int          int32_t;

typedef uint32_t size_t;

#define NULL ((void*)0)
#define TRUE  1
#define FALSE 0

/* ---- Macro-uri pentru porturi I/O ---- */

/* Citeste un byte de la un port I/O */
static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

/* Scrie un byte la un port I/O */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

/* Citeste un word (2 bytes) */
static inline uint16_t inw(uint16_t port) {
    uint16_t val;
    __asm__ volatile("inw %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

/* Scrie un word */
static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}

/* ---- Functii de memorie (implementate in kernel.c) ---- */
void* kmemset(void* ptr, int value, size_t num);
void* kmemcpy(void* dest, const void* src, size_t num);
int   kstrcmp(const char* a, const char* b);
int   kstrlen(const char* s);

#endif /* KERNEL_H */
