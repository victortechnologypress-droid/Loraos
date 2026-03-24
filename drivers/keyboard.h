/* drivers/keyboard.h - PS/2 Keyboard Driver */

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../kernel/include/kernel.h"

void    keyboard_init(void);
void    keyboard_poll(void);
char    keyboard_getchar(void);   /* Returneaza 0 daca nu e nicio tasta */
uint8_t keyboard_key_pressed(uint8_t scancode);

/* Scancode-uri comune */
#define KEY_ENTER     0x1C
#define KEY_BACKSPACE 0x0E
#define KEY_ESC       0x01
#define KEY_SPACE     0x39
#define KEY_UP        0x48
#define KEY_DOWN      0x50
#define KEY_LEFT      0x4B
#define KEY_RIGHT     0x4D

#endif
