/* ui/window.h - Window Manager */

#ifndef WINDOW_H
#define WINDOW_H

#include "../kernel/include/kernel.h"

/* Tipuri de continut fereastra */
#define WINDOW_WELCOME     0
#define WINDOW_TERMINAL    1
#define WINDOW_FILEMANAGER 2

typedef struct {
    const char* title;
    int         x, y;
    int         width, height;
    uint8_t     visible;
    uint8_t     focused;
    uint8_t     content_type;
} window_t;

void window_create(const char* title, int x, int y, int w, int h);
void window_draw(window_t* win);
void window_draw_welcome(window_t* win);
void window_draw_terminal(window_t* win);
void window_draw_filemanager(window_t* win);
int  window_hit_test(window_t* win, int32_t x, int32_t y);
void window_handle_click(window_t* win, int32_t x, int32_t y);

#endif
