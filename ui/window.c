/* ui/window.c
 * LoraOS - Window Manager de baza
 * Deseneaza ferestre cu bara de titlu, buton de inchidere
 */

#include "../kernel/include/kernel.h"
#include "../drivers/vbe.h"
#include "window.h"

#define TITLEBAR_H  28
#define BORDER_W     2

/* ============================================================
 *  window_draw - Deseneaza o fereastra completa
 * ============================================================ */
void window_draw(window_t* win)
{
    if (!win || !win->visible) return;

    int x = win->x;
    int y = win->y;
    int w = win->width;
    int h = win->height;

    /* ---- Shadow (umbra subtila) ---- */
    vbe_fill_rect(x + 4, y + 4, w, h, 0x88000000 & 0xFF1A1A2E);

    /* ---- Contur fereastra ---- */
    uint32_t border_color = win->focused ? COLOR_ACCENT_BLUE : COLOR_WINDOW_BORDER;
    vbe_draw_rect(x - BORDER_W, y - BORDER_W,
                  w + BORDER_W*2, h + BORDER_W*2, border_color);

    /* ---- Bara de titlu ---- */
    vbe_fill_rect(x, y, w, TITLEBAR_H, COLOR_WINDOW_TITLE);

    /* Gradient subtil pe titlebar */
    for (int i = 0; i < TITLEBAR_H; i++) {
        uint8_t alpha = (uint8_t)(i * 10 / TITLEBAR_H);
        uint32_t c = (0xFF << 24) | ((49 + alpha) << 16) | ((50 + alpha) << 8) | (68 + alpha);
        vbe_fill_rect(x, y + i, w, 1, c);
    }

    /* ---- Titlu fereastra ---- */
    vbe_draw_string(x + 10, y + 9, win->title, COLOR_TEXT_PRIMARY, COLOR_TRANSPARENT);

    /* ---- Butoane de control (dreapta titlebar) ---- */
    /* Buton Close (X) */
    vbe_fill_rect(x + w - 28, y + 4, 24, 20, COLOR_BTN_CLOSE);
    vbe_draw_string(x + w - 22, y + 8, "X", COLOR_WHITE, COLOR_TRANSPARENT);

    /* Buton Minimize */
    vbe_fill_rect(x + w - 56, y + 4, 24, 20, COLOR_BTN_NORMAL);
    vbe_draw_string(x + w - 50, y + 8, "_", COLOR_TEXT_PRIMARY, COLOR_TRANSPARENT);

    /* ---- Continut fereastra ---- */
    vbe_fill_rect(x, y + TITLEBAR_H, w, h - TITLEBAR_H, COLOR_WINDOW_BG);

    /* Separator sub titlebar */
    vbe_fill_rect(x, y + TITLEBAR_H, w, 1, border_color);

    /* ---- Continut implicit (Welcome screen) ---- */
    if (win->content_type == WINDOW_WELCOME) {
        window_draw_welcome(win);
    } else if (win->content_type == WINDOW_TERMINAL) {
        window_draw_terminal(win);
    } else if (win->content_type == WINDOW_FILEMANAGER) {
        window_draw_filemanager(win);
    }
}

/* ============================================================
 *  window_draw_welcome - Ecranul de bun venit
 * ============================================================ */
void window_draw_welcome(window_t* win)
{
    int cx = win->x + 20;
    int cy = win->y + TITLEBAR_H + 20;

    vbe_draw_string(cx, cy,      "Bun venit in LoraOS v1!",    COLOR_ACCENT_BLUE,  COLOR_TRANSPARENT);
    vbe_draw_string(cx, cy + 20, "Un OS open-source minimal.", COLOR_TEXT_PRIMARY, COLOR_TRANSPARENT);
    vbe_draw_string(cx, cy + 40, "Rulezi direct din RAM.",     COLOR_TEXT_PRIMARY, COLOR_TRANSPARENT);

    /* Separator */
    vbe_fill_rect(cx, cy + 60, win->width - 40, 1, COLOR_WINDOW_BORDER);

    vbe_draw_string(cx, cy + 75,  "Specificatii:", COLOR_ACCENT_PINK, COLOR_TRANSPARENT);
    vbe_draw_string(cx, cy + 95,  "CPU: x86 32-bit",            COLOR_TEXT_DIM, COLOR_TRANSPARENT);
    vbe_draw_string(cx, cy + 113, "Grafica: VBE 1024x768 32bpp",COLOR_TEXT_DIM, COLOR_TRANSPARENT);
    vbe_draw_string(cx, cy + 131, "Memorie: PMM 4GB",           COLOR_TEXT_DIM, COLOR_TRANSPARENT);
    vbe_draw_string(cx, cy + 149, "FS: FAT32 persistence USB",  COLOR_TEXT_DIM, COLOR_TRANSPARENT);
    vbe_draw_string(cx, cy + 167, "Input: PS/2 + Tap-to-click", COLOR_TEXT_DIM, COLOR_TRANSPARENT);

    /* Boot pe USB */
    vbe_fill_rect(cx, cy + 200, win->width - 40, 1, COLOR_WINDOW_BORDER);
    vbe_draw_string(cx, cy + 215, "Scrie loraos.iso cu Rufus pe USB!", COLOR_ACCENT_GREEN, COLOR_TRANSPARENT);
}

/* ============================================================
 *  window_draw_terminal - Terminal simplu (stub)
 * ============================================================ */
void window_draw_terminal(window_t* win)
{
    int cx = win->x + 10;
    int cy = win->y + TITLEBAR_H + 10;

    vbe_draw_string(cx, cy, "LoraOS Terminal v1", COLOR_ACCENT_GREEN, COLOR_TRANSPARENT);
    vbe_draw_string(cx, cy + 20, "> _", COLOR_ACCENT_GREEN, COLOR_TRANSPARENT);
}

/* ============================================================
 *  window_draw_filemanager - LoraExplorer (stub)
 * ============================================================ */
void window_draw_filemanager(window_t* win)
{
    int cx = win->x + 10;
    int cy = win->y + TITLEBAR_H + 10;

    vbe_draw_string(cx, cy, "LoraExplorer - USB:/", COLOR_ACCENT_BLUE, COLOR_TRANSPARENT);
    vbe_draw_string(cx, cy + 20, "[DIR]  System/",   COLOR_ACCENT_PINK,  COLOR_TRANSPARENT);
    vbe_draw_string(cx, cy + 36, "[DIR]  Documents/",COLOR_ACCENT_PINK,  COLOR_TRANSPARENT);
    vbe_draw_string(cx, cy + 52, "[FILE] loraos.iso",COLOR_TEXT_PRIMARY, COLOR_TRANSPARENT);
}

/* ============================================================
 *  window_hit_test - Verifica daca un punct e in fereastra
 * ============================================================ */
int window_hit_test(window_t* win, int32_t x, int32_t y)
{
    if (!win || !win->visible) return FALSE;
    return (x >= win->x && x < win->x + win->width &&
            y >= win->y && y < win->y + win->height);
}

/* ============================================================
 *  window_handle_click - Procesare click pe fereastra
 * ============================================================ */
void window_handle_click(window_t* win, int32_t x, int32_t y)
{
    if (!win) return;

    /* Click pe butonul Close */
    if (x >= win->x + win->width - 28 &&
        x <= win->x + win->width - 4  &&
        y >= win->y + 4 && y <= win->y + 24) {
        win->visible = FALSE;
    }

    /* Marcheaza ca focused */
    win->focused = TRUE;
    window_draw(win);
}
