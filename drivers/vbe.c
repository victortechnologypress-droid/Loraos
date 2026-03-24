#include "../kernel/include/kernel.h"
#include "vbe.h"

static uint32_t* framebuffer = NULL;
static uint32_t  fb_width    = 1024;
static uint32_t  fb_height   = 600; /* Rezolutia nativa Acer Aspire One D255 */

/* Font stub 8x8 (Asigura-te ca ai fontul complet in proiectul tau) */
static const uint8_t font8x8[96][8] = {
    [0x41-32] = { 0x18,0x3C,0x66,0x7E,0x66,0x66,0x66,0x00 }, /* A */
};

void vbe_init(void) {
    /* ATENTIE: Daca ingheata aici, adresa 0xFD000000 trebuie verificata in Multiboot */
    framebuffer = (uint32_t*)0xFD000000;
    fb_width    = 1024;
    fb_height   = 600;

    vbe_clear(COLOR_DESKTOP_BG);
}

void vbe_clear(uint32_t color) {
    uint32_t total = fb_width * fb_height;
    for (uint32_t i = 0; i < total; i++) {
        framebuffer[i] = color;
    }
}

void vbe_put_pixel(int x, int y, uint32_t color) {
    if ((uint32_t)x >= fb_width || (uint32_t)y >= fb_height) return;
    framebuffer[y * fb_width + x] = color;
}

void vbe_fill_rect(int x, int y, int w, int h, uint32_t color) {
    for (int row = y; row < y + h; row++) {
        for (int col = x; col < x + w; col++) {
            vbe_put_pixel(col, row, color);
        }
    }
}

void vbe_draw_rect(int x, int y, int w, int h, uint32_t color) {
    vbe_fill_rect(x, y, w, 1, color);
    vbe_fill_rect(x, y+h-1, w, 1, color);
    vbe_fill_rect(x, y, 1, h, color);
    vbe_fill_rect(x+w-1, y, 1, h, color);
}

void vbe_draw_char(int x, int y, char c, uint32_t fg, uint32_t bg) {
    if (c < 32 || c > 127) c = '?';
    int idx = c - 32;
    for (int row = 0; row < 8; row++) {
        uint8_t bits = font8x8[idx][row];
        for (int col = 0; col < 8; col++) {
            if (bits & (0x80 >> col)) {
                vbe_put_pixel(x + col, y + row, fg);
            } else if (bg != COLOR_TRANSPARENT) {
                vbe_put_pixel(x + col, y + row, bg);
            }
        }
    }
}

void vbe_draw_string(int x, int y, const char* str, uint32_t fg, uint32_t bg) {
    int cx = x;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\n') {
            cx = x; y += 10;
        } else {
            vbe_draw_char(cx, y, str[i], fg, bg);
            cx += 9;
        }
    }
}

uint32_t vbe_get_width(void)  { return fb_width; }
uint32_t vbe_get_height(void) { return fb_height; }
 *  In MVP: adresa hardcodata. In v2: vom citi din Multiboot.
 * ============================================================ */
void vbe_init(void)
{
    /* TODO v2: citeste fb_addr din multiboot_info->framebuffer_addr */
    /* Pentru acum: GRUB Linear Framebuffer standard */
    framebuffer = (uint32_t*)0xFD000000;
    fb_width    = 1024;
    fb_height   = 768;
    fb_pitch    = fb_width * 4; /* 32 biti per pixel = 4 bytes */

    /* Curata ecranul cu culoarea de fundal a desktop-ului */
    vbe_clear(COLOR_DESKTOP_BG);
}

/* ============================================================
 *  vbe_clear - Umple tot ecranul cu o culoare
 * ============================================================ */
void vbe_clear(uint32_t color)
{
    uint32_t total = fb_width * fb_height;
    for (uint32_t i = 0; i < total; i++) {
        framebuffer[i] = color;
    }
}

/* ============================================================
 *  vbe_put_pixel - Deseneaza un pixel la (x, y)
 * ============================================================ */
void vbe_put_pixel(int x, int y, uint32_t color)
{
    if ((uint32_t)x >= fb_width || (uint32_t)y >= fb_height) return;
    framebuffer[y * fb_width + x] = color;
}

/* ============================================================
 *  vbe_fill_rect - Deseneaza un dreptunghi plin
 * ============================================================ */
void vbe_fill_rect(int x, int y, int w, int h, uint32_t color)
{
    for (int row = y; row < y + h; row++) {
        for (int col = x; col < x + w; col++) {
            vbe_put_pixel(col, row, color);
        }
    }
}

/* ============================================================
 *  vbe_draw_rect - Deseneaza conturul unui dreptunghi
 * ============================================================ */
void vbe_draw_rect(int x, int y, int w, int h, uint32_t color)
{
    /* Linie sus */
    vbe_fill_rect(x, y, w, 1, color);
    /* Linie jos */
    vbe_fill_rect(x, y+h-1, w, 1, color);
    /* Linie stanga */
    vbe_fill_rect(x, y, 1, h, color);
    /* Linie dreapta */
    vbe_fill_rect(x+w-1, y, 1, h, color);
}

/* ============================================================
 *  vbe_draw_char - Deseneaza un caracter cu fontul 8x8
 * ============================================================ */
void vbe_draw_char(int x, int y, char c, uint32_t fg, uint32_t bg)
{
    if (c < 32 || c > 127) c = '?';
    int idx = c - 32;
    if (idx >= 96) idx = 0;

    for (int row = 0; row < 8; row++) {
        uint8_t bits = font8x8[idx][row];
        for (int col = 0; col < 8; col++) {
            if (bits & (0x80 >> col)) {
                vbe_put_pixel(x + col, y + row, fg);
            } else if (bg != COLOR_TRANSPARENT) {
                vbe_put_pixel(x + col, y + row, bg);
            }
        }
    }
}

/* ============================================================
 *  vbe_draw_string - Deseneaza un sir de caractere
 * ============================================================ */
void vbe_draw_string(int x, int y, const char* str, uint32_t fg, uint32_t bg)
{
    int cx = x;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\n') {
            cx = x;
            y += 10;
        } else {
            vbe_draw_char(cx, y, str[i], fg, bg);
            cx += 9; /* 8px font + 1px spatiu */
        }
    }
}

/* ============================================================
 *  vbe_get_width / vbe_get_height
 * ============================================================ */
uint32_t vbe_get_width(void)  { return fb_width; }
uint32_t vbe_get_height(void) { return fb_height; }
