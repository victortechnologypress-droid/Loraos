/* ui/taskbar.h - Taskbar */

#ifndef TASKBAR_H
#define TASKBAR_H

#include "../kernel/include/kernel.h"

void taskbar_init(void);
void taskbar_draw(void);
void taskbar_draw_clock(void);
void taskbar_update(void);
void taskbar_handle_click(int32_t x, int32_t y);

#endif
