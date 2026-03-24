/* drivers/vbe.h - Header pentru driver-ul VBE */

#ifndef VBE_H
#define VBE_H

#include "../kernel/include/kernel.h"

/* ---- Paleta de culori LoraOS (format 0x00RRGGBB) ---- */
#define COLOR_TRANSPARENT   0xFFFFFFFF  /* Valoare speciala: nu desena */

/* Desktop */
#define COLOR_DESKTOP_BG    0xFF1A1A2E  /* Bleumarin inchis */
#define COLOR_TASKBAR_BG    0xFF16213E  /* Albastru naval */
#define COLOR_TASKBAR_ACCENT 0xFF0F3460 /* Accent albastru */

/* Ferestre */
#define COLOR_WINDOW_BG     0xFF1E1E2E  /* Fundal fereastra */
#define COLOR_WINDOW_TITLE  0xFF313244  /* Bara de titlu */
#define COLOR_WINDOW_BORDER 0xFF45475A  /* Contur fereastra */

/* Text */
#define COLOR_TEXT_PRIMARY  0xFFCDD6F4  /* Alb-albastru (Catppuccin) */
#define COLOR_TEXT_DIM      0xFF6C7086  /* Gri */
#define COLOR_ACCENT_PINK   0xFFF38BA8  /* Accent roz */
#define COLOR_ACCENT_BLUE   0xFF89B4FA  /* Accent albastru */
#define COLOR_ACCENT_GREEN  0xFFA6E3A1  /* Accent verde */

/* Butoane */
#define COLOR_BTN_NORMAL    0xFF313244
#define COLOR_BTN_HOVER     0xFF45475A
#define COLOR_BTN_CLOSE     0xFFF38BA8

/* Alb/Negru */
#define COLOR_WHITE         0xFFFFFFFF
#define COLOR_BLACK         0xFF000000

/* ---- Prototipuri ---- */
void     vbe_init(void);
void     vbe_clear(uint32_t color);
void     vbe_put_pixel(int x, int y, uint32_t color);
void     vbe_fill_rect(int x, int y, int w, int h, uint32_t color);
void     vbe_draw_rect(int x, int y, int w, int h, uint32_t color);
void     vbe_draw_char(int x, int y, char c, uint32_t fg, uint32_t bg);
void     vbe_draw_string(int x, int y, const char* str, uint32_t fg, uint32_t bg);
uint32_t vbe_get_width(void);
uint32_t vbe_get_height(void);

#endif /* VBE_H */
