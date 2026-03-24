/* drivers/keyboard.c - PS/2 Keyboard Driver */

#include "../kernel/include/kernel.h"
#include "keyboard.h"

#define KB_DATA     0x60
#define KB_STATUS   0x64

/* Scancode -> ASCII map (US layout, fara Shift) */
static const char scancode_map[128] = {
    0,  0, '1','2','3','4','5','6','7','8','9','0','-','=', 0,  0,
  'q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
  'a','s','d','f','g','h','j','k','l',';','\'','`',  0, '\\',
  'z','x','c','v','b','n','m',',','.','/',  0, '*',  0, ' ',
};

static char key_buffer[64];
static uint8_t buf_head = 0;
static uint8_t buf_tail = 0;

void keyboard_init(void)
{
    buf_head = 0;
    buf_tail = 0;
    /* Flush orice date vechi din buffer-ul PS/2 */
    while (inb(KB_STATUS) & 0x01) inb(KB_DATA);
}

void keyboard_poll(void)
{
    if (!(inb(KB_STATUS) & 0x01)) return;

    uint8_t scancode = inb(KB_DATA);

    /* Ignoram key-release (bit 7 setat) */
    if (scancode & 0x80) return;
    if (scancode >= 128)  return;

    char c = scancode_map[scancode];
    if (c == 0) return;

    /* Adauga in buffer circular */
    uint8_t next = (uint8_t)((buf_head + 1) % 64);
    if (next != buf_tail) {
        key_buffer[buf_head] = c;
        buf_head = next;
    }
}

char keyboard_getchar(void)
{
    keyboard_poll();
    if (buf_head == buf_tail) return 0;
    char c = key_buffer[buf_tail];
    buf_tail = (uint8_t)((buf_tail + 1) % 64);
    return c;
}

uint8_t keyboard_key_pressed(uint8_t scancode)
{
    (void)scancode;
    return FALSE;
}
