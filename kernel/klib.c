/* kernel/klib.c - Functii utilitare de baza (fara stdlib) */

#include "include/kernel.h"

void* kmemset(void* ptr, int value, size_t num)
{
    uint8_t* p = (uint8_t*)ptr;
    while (num--) *p++ = (uint8_t)value;
    return ptr;
}

void* kmemcpy(void* dest, const void* src, size_t num)
{
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    while (num--) *d++ = *s++;
    return dest;
}

int kstrcmp(const char* a, const char* b)
{
    while (*a && (*a == *b)) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

int kstrlen(const char* s)
{
    int len = 0;
    while (s[len]) len++;
    return len;
}
