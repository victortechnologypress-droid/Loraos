/* ui/desktop.h - Desktop Manager */

#ifndef DESKTOP_H
#define DESKTOP_H

#include "../kernel/include/kernel.h"

void desktop_init(void);
void desktop_run(void);
void window_create_desktop(const char* title, int x, int y, int w, int h);

#endif
