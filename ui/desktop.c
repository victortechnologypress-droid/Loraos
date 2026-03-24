/* ui/desktop.c
 * LoraOS - Desktop Manager
 * Deseneaza wallpaper-ul, taskbar-ul si gestioneaza ferestrele
 */

#include "../kernel/include/kernel.h"
#include "../drivers/vbe.h"
#include "../drivers/ps2mouse.h"
#include "../drivers/keyboard.h"
#include "desktop.h"
#include "taskbar.h"
#include "window.h"

/* ---- Starea desktop-ului ---- */
#define MAX_WINDOWS 8

static window_t windows[MAX_WINDOWS];
static int      window_count = 0;
static int      focused_window = -1;

/* ---- Cursorul mouse ---- */
static int32_t cursor_prev_x = -1;
static int32_t cursor_prev_y = -1;

/* Forma cursorului: 12x20 pixeli (1=desenat, 0=transparent) */
static const uint8_t cursor_shape[20][12] = {
    {1,0,0,0,0,0,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0,0,0,0,0},
    {1,2,1,0,0,0,0,0,0,0,0,0},
    {1,2,2,1,0,0,0,0,0,0,0,0},
    {1,2,2,2,1,0,0,0,0,0,0,0},
    {1,2,2,2,2,1,0,0,0,0,0,0},
    {1,2,2,2,2,2,1,0,0,0,0,0},
    {1,2,2,2,2,2,2,1,0,0,0,0},
    {1,2,2,2,2,2,2,2,1,0,0,0},
    {1,2,2,2,2,2,2,2,2,1,0,0},
    {1,2,2,2,2,2,2,1,1,1,0,0},
    {1,2,2,2,1,2,2,1,0,0,0,0},
    {1,2,2,1,0,1,2,2,1,0,0,0},
    {1,2,1,0,0,1,2,2,1,0,0,0},
    {1,1,0,0,0,0,1,2,2,1,0,0},
    {1,0,0,0,0,0,1,2,2,1,0,0},
    {0,0,0,0,0,0,0,1,2,2,1,0},
    {0,0,0,0,0,0,0,1,2,2,1,0},
    {0,0,0,0,0,0,0,0,1,1,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0},
};

/* ---- Wallpaper: gradient simplu ---- */
static void draw_wallpaper(void)
{
    uint32_t w = vbe_get_width();
    uint32_t h = vbe_get_height() - 40; /* Minus taskbar */

    for (uint32_t y = 0; y < h; y++) {
        /* Gradient vertical: bleumarin -> violet inchis */
        uint8_t r = (uint8_t)(26  + (y * 10) / h);
        uint8_t g = (uint8_t)(26  + (y * 5)  / h);
        uint8_t b = (uint8_t)(46  + (y * 40) / h);
        uint32_t color = (0xFF << 24) | (r << 16) | (g << 8) | b;

        for (uint32_t x = 0; x < w; x++) {
            vbe_put_pixel(x, y, color);
        }
    }

    /* Logo LoraOS centrat pe desktop */
    const char* logo1 = "L O R A O S";
    const char* logo2 = "v 1 . 0  M V P";
    uint32_t lx = w/2 - 55;
    uint32_t ly = h/2 - 20;
    vbe_draw_string(lx,    ly,    logo1, COLOR_ACCENT_BLUE,  COLOR_TRANSPARENT);
    vbe_draw_string(lx+10, ly+14, logo2, COLOR_ACCENT_PINK,  COLOR_TRANSPARENT);
}

/* ---- Desenare cursor ---- */
static void draw_cursor(int32_t x, int32_t y)
{
    for (int row = 0; row < 20; row++) {
        for (int col = 0; col < 12; col++) {
            uint8_t pixel = cursor_shape[row][col];
            if (pixel == 1) {
                vbe_put_pixel(x + col, y + row, COLOR_WHITE);
            } else if (pixel == 2) {
                vbe_put_pixel(x + col, y + row, COLOR_BLACK);
            }
        }
    }
}

/* ============================================================
 *  desktop_init - Initializeaza shell-ul grafic
 * ============================================================ */
void desktop_init(void)
{
    window_count   = 0;
    focused_window = -1;
    cursor_prev_x  = -1;
    cursor_prev_y  = -1;

    /* Deseneaza desktop-ul initial */
    draw_wallpaper();
    taskbar_init();
    taskbar_draw();

    /* Deseneaza cursorul initial */
    draw_cursor(512, 384);
    cursor_prev_x = 512;
    cursor_prev_y = 384;

    /* Deschide fereastra Welcome */
    window_create_desktop("Bun venit in LoraOS!", 200, 150, 624, 400);
}

/* ============================================================
 *  desktop_run - Bucla principala (event loop)
 *  NU returneaza niciodata
 * ============================================================ */
void desktop_run(void)
{
    while (TRUE) {
        /* ---- Polling input ---- */
        ps2mouse_poll();

        int32_t mx = ps2mouse_get_x();
        int32_t my = ps2mouse_get_y();

        /* ---- Update cursor daca s-a miscat ---- */
        if (mx != cursor_prev_x || my != cursor_prev_y) {
            /* Redeseneaza zona veche a cursorului */
            /* In MVP: redesenam tot (simplu dar functional) */
            draw_wallpaper();
            taskbar_draw();

            /* Redeseneaza ferestrele */
            for (int i = 0; i < window_count; i++) {
                window_draw(&windows[i]);
            }

            /* Deseneaza cursor la noua pozitie */
            draw_cursor(mx, my);
            cursor_prev_x = mx;
            cursor_prev_y = my;
        }

        /* ---- Update taskbar (ceas) ---- */
        taskbar_update();

        /* ---- Procesare click ---- */
        if (ps2mouse_left_pressed()) {
            /* Verifica daca click-ul e pe taskbar */
            if (my >= (int32_t)(vbe_get_height() - 40)) {
                taskbar_handle_click(mx, my);
            } else {
                /* Verifica ferestrele */
                for (int i = window_count - 1; i >= 0; i--) {
                    if (window_hit_test(&windows[i], mx, my)) {
                        focused_window = i;
                        window_handle_click(&windows[i], mx, my);
                        break;
                    }
                }
            }
        }

        /* ---- Mica pauza pentru CPU ---- */
        /* In MVP: busy-wait simplu */
        for (volatile int i = 0; i < 10000; i++);
    }
}

/* ============================================================
 *  window_create - Deschide o fereastra noua
 * ============================================================ */
void window_create_desktop(const char* title, int x, int y, int w, int h)
{
    if (window_count >= MAX_WINDOWS) return;

    window_t* win = &windows[window_count];
    win->title  = title;
    win->x      = x;
    win->y      = y;
    win->width  = w;
    win->height = h;
    win->visible = TRUE;
    win->focused = FALSE;

    focused_window = window_count;
    window_count++;

    window_draw(win);
}
