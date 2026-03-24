/* drivers/ps2mouse.h - PS/2 Mouse & TrackPad Driver */

#ifndef PS2MOUSE_H
#define PS2MOUSE_H

#include "../kernel/include/kernel.h"

void    ps2mouse_init(void);
void    ps2mouse_poll(void);
int32_t ps2mouse_get_x(void);
int32_t ps2mouse_get_y(void);
uint8_t ps2mouse_left_pressed(void);
uint8_t ps2mouse_right_pressed(void);

#endif
