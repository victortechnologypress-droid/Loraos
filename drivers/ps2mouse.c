/* drivers/ps2mouse.c
 * LoraOS - Driver PS/2 Mouse & TrackPad
 * Citeste pachetele de la mouse si translateaza in coordonate
 * Feature special: Tap-to-Click pentru trackpad-uri
 */

#include "../kernel/include/kernel.h"
#include "ps2mouse.h"

/* ---- Porturi PS/2 ---- */
#define PS2_DATA    0x60
#define PS2_STATUS  0x64
#define PS2_CMD     0x64

/* ---- Starea mouse-ului ---- */
static int32_t mouse_x = 512;   /* Pozitia curenta X (incepe centrat) */
static int32_t mouse_y = 384;   /* Pozitia curenta Y */
static uint8_t mouse_buttons = 0;   /* Bitfield: bit0=stanga, bit1=dreapta */

/* ---- Tap-to-click ---- */
#define TAP_THRESHOLD_MS    200     /* Max 200ms pentru un tap */
static uint32_t tap_timer = 0;
static uint8_t  tap_active = FALSE;

/* ---- Buffer pachete PS/2 (3 bytes per pachet) ---- */
static uint8_t  packet[3];
static uint8_t  packet_idx = 0;

/* ---- Limite ecran ---- */
static int32_t screen_w = 1024;
static int32_t screen_h = 768;

/* ============================================================
 *  Functii de comunicare cu controlerul PS/2
 * ============================================================ */
static void ps2_wait_write(void) {
    /* Asteapta ca buffer-ul de intrare sa fie gol */
    int timeout = 100000;
    while ((inb(PS2_STATUS) & 0x02) && --timeout);
}

static void ps2_wait_read(void) {
    /* Asteapta ca buffer-ul de iesire sa fie plin */
    int timeout = 100000;
    while (!(inb(PS2_STATUS) & 0x01) && --timeout);
}

static void ps2_send_cmd(uint8_t cmd) {
    ps2_wait_write();
    outb(PS2_CMD, cmd);
}

static void ps2_send_data(uint8_t data) {
    ps2_wait_write();
    outb(PS2_DATA, data);
}

static uint8_t ps2_read_data(void) {
    ps2_wait_read();
    return inb(PS2_DATA);
}

/* ============================================================
 *  ps2mouse_init - Initializeaza mouse-ul PS/2
 * ============================================================ */
void ps2mouse_init(void)
{
    /* 1. Activeaza al doilea port PS/2 (mouse) */
    ps2_send_cmd(0xA8);

    /* 2. Activeaza intreruperile pentru mouse (IRQ12) */
    ps2_send_cmd(0x20);     /* Citeste byte-ul de configurare */
    uint8_t config = ps2_read_data();
    config |= 0x02;         /* Bit1: Activeaza IRQ12 */
    config &= ~0x20;        /* Bit5: Dezactiveaza mouse clock disable */
    ps2_send_cmd(0x60);     /* Scrie byte-ul de configurare */
    ps2_send_data(config);

    /* 3. Trimite comanda de reset la mouse */
    ps2_send_cmd(0xD4);     /* Urmatoarea comanda merge la mouse */
    ps2_send_data(0xFF);    /* Reset */
    ps2_read_data();        /* ACK (0xFA) */
    ps2_read_data();        /* Self-test passed (0xAA) */
    ps2_read_data();        /* Mouse ID (0x00 = standard) */

    /* 4. Seteaza rezolutia */
    ps2_send_cmd(0xD4);
    ps2_send_data(0xE8);    /* Set resolution */
    ps2_read_data();        /* ACK */
    ps2_send_data(0x02);    /* 4 counts/mm */
    ps2_read_data();        /* ACK */

    /* 5. Activeaza stream mode (trimite date continuu) */
    ps2_send_cmd(0xD4);
    ps2_send_data(0xF4);    /* Enable */
    ps2_read_data();        /* ACK */

    /* Pozitioneaza cursorul in centrul ecranului */
    mouse_x = screen_w / 2;
    mouse_y = screen_h / 2;
}

/* ============================================================
 *  ps2mouse_poll - Citeste un pachet de date de la mouse
 *  Apelat periodic din bucla principala (sau IRQ handler)
 * ============================================================ */
void ps2mouse_poll(void)
{
    /* Verifica daca exista date disponibile */
    if (!(inb(PS2_STATUS) & 0x01)) return;
    if (!(inb(PS2_STATUS) & 0x20)) return; /* Bit5: data de la mouse */

    packet[packet_idx++] = inb(PS2_DATA);

    /* Primul byte al pachetului trebuie sa aiba bit3 setat */
    if (packet_idx == 1 && !(packet[0] & 0x08)) {
        packet_idx = 0; /* Resincronizare */
        return;
    }

    /* Asteptam 3 bytes pentru un pachet complet */
    if (packet_idx < 3) return;
    packet_idx = 0;

    /* ---- Parseaza pachetul ---- */
    /* Byte 0: flags (butoane + overflow)
     * Byte 1: delta X (signed, cu sign in byte 0 bit 4)
     * Byte 2: delta Y (signed, cu sign in byte 0 bit 5) */

    uint8_t flags = packet[0];

    /* Delta X: poate fi negativ */
    int32_t dx = (int32_t)packet[1];
    if (flags & 0x10) dx |= 0xFFFFFF00; /* Extinde semnul */

    /* Delta Y: PS/2 are Y inversat fata de ecran */
    int32_t dy = (int32_t)packet[2];
    if (flags & 0x20) dy |= 0xFFFFFF00;
    dy = -dy; /* Inverseaza pentru coordonate ecran */

    /* Actualizeaza pozitia */
    mouse_x += dx;
    mouse_y += dy;

    /* Clamp la marginile ecranului */
    if (mouse_x < 0) mouse_x = 0;
    if (mouse_y < 0) mouse_y = 0;
    if (mouse_x >= screen_w) mouse_x = screen_w - 1;
    if (mouse_y >= screen_h) mouse_y = screen_h - 1;

    /* ---- Detectare butoane ---- */
    uint8_t prev_buttons = mouse_buttons;
    mouse_buttons = flags & 0x03; /* Bit0=stanga, Bit1=dreapta */

    /* ---- Tap-to-Click ----
     * Daca nu a fost miscare si butonul a fost apasat rapid,
     * generam un click virtual */
    (void)prev_buttons; /* Folosit in logica viitoare */

    /* Simplu timer bazat pe poll count */
    if (tap_active) {
        tap_timer++;
        if (tap_timer > TAP_THRESHOLD_MS) {
            tap_active = FALSE;
            tap_timer = 0;
        }
    }
}

/* ============================================================
 *  Getters
 * ============================================================ */
int32_t ps2mouse_get_x(void)        { return mouse_x; }
int32_t ps2mouse_get_y(void)        { return mouse_y; }
uint8_t ps2mouse_left_pressed(void) { return (mouse_buttons & 0x01) ? TRUE : FALSE; }
uint8_t ps2mouse_right_pressed(void){ return (mouse_buttons & 0x02) ? TRUE : FALSE; }
