/* ui/taskbar.c
 * LoraOS - Taskbar (bara de jos)
 * Afiseaza: buton Start, ora citita din BIOS RTC, notificari
 */

#include "../kernel/include/kernel.h"
#include "../drivers/vbe.h"
#include "taskbar.h"

/* ---- Inaltimea taskbar-ului ---- */
#define TASKBAR_H   40
#define TASKBAR_Y   (vbe_get_height() - TASKBAR_H)

/* ---- Porturi BIOS RTC (Real Time Clock) ---- */
#define CMOS_ADDR   0x70
#define CMOS_DATA   0x71

/* ---- Stare ---- */
static uint8_t last_hour   = 0xFF; /* FF = neinitialized */
static uint8_t last_minute = 0xFF;

/* ============================================================
 *  Citire ora din BIOS RTC
 *  RTC-ul BIOS foloseste format BCD (Binary Coded Decimal)
 * ============================================================ */
static uint8_t cmos_read(uint8_t reg)
{
    outb(CMOS_ADDR, reg);
    return inb(CMOS_DATA);
}

static uint8_t bcd_to_bin(uint8_t bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

static void rtc_get_time(uint8_t* hours, uint8_t* minutes, uint8_t* seconds)
{
    /* Asteapta sa nu fim in mijlocul unui update RTC */
    while (cmos_read(0x0A) & 0x80); /* Bit7: update in progress */

    *seconds = bcd_to_bin(cmos_read(0x00));
    *minutes = bcd_to_bin(cmos_read(0x02));
    *hours   = bcd_to_bin(cmos_read(0x04));
}

/* ============================================================
 *  Conversie numar -> string (fara sprintf, suntem in kernel)
 * ============================================================ */
static void uint_to_str_pad2(uint8_t val, char* out)
{
    out[0] = '0' + (val / 10);
    out[1] = '0' + (val % 10);
    out[2] = '\0';
}

/* ============================================================
 *  taskbar_init - Prima desenare
 * ============================================================ */
void taskbar_init(void)
{
    last_hour   = 0xFF;
    last_minute = 0xFF;
}

/* ============================================================
 *  taskbar_draw - Deseneaza taskbar-ul complet
 * ============================================================ */
void taskbar_draw(void)
{
    uint32_t w = vbe_get_width();
    uint32_t ty = TASKBAR_Y;

    /* ---- Fundal taskbar ---- */
    vbe_fill_rect(0, ty, w, TASKBAR_H, COLOR_TASKBAR_BG);

    /* ---- Linie de separare sus ---- */
    vbe_fill_rect(0, ty, w, 1, COLOR_TASKBAR_ACCENT);

    /* ---- Buton START ---- */
    vbe_fill_rect(5, ty + 5, 80, 30, COLOR_BTN_NORMAL);
    vbe_draw_rect(5, ty + 5, 80, 30, COLOR_ACCENT_BLUE);
    vbe_draw_string(15, ty + 13, "START", COLOR_ACCENT_BLUE, COLOR_TRANSPARENT);

    /* ---- Ora ---- */
    taskbar_draw_clock();

    /* ---- Indicator LoraOS ---- */
    vbe_draw_string(100, ty + 13, "LoraOS v1", COLOR_TEXT_DIM, COLOR_TRANSPARENT);
}

/* ============================================================
 *  taskbar_draw_clock - Deseneaza doar sectiunea de ceas
 * ============================================================ */
void taskbar_draw_clock(void)
{
    uint8_t h, m, s;
    rtc_get_time(&h, &m, &s);

    uint32_t w  = vbe_get_width();
    uint32_t ty = TASKBAR_Y;

    /* Sterge zona ceasului */
    vbe_fill_rect(w - 70, ty + 1, 69, TASKBAR_H - 2, COLOR_TASKBAR_BG);

    /* Format: HH:MM */
    char time_str[6];
    char hh[3], mm[3];
    uint_to_str_pad2(h, hh);
    uint_to_str_pad2(m, mm);

    time_str[0] = hh[0];
    time_str[1] = hh[1];
    time_str[2] = ':';
    time_str[3] = mm[0];
    time_str[4] = mm[1];
    time_str[5] = '\0';

    vbe_draw_string(w - 60, ty + 13, time_str, COLOR_TEXT_PRIMARY, COLOR_TRANSPARENT);

    last_hour   = h;
    last_minute = m;
}

/* ============================================================
 *  taskbar_update - Update ceas (apelat din event loop)
 *  Redeseneaza doar daca minutul s-a schimbat
 * ============================================================ */
void taskbar_update(void)
{
    uint8_t h, m, s;
    rtc_get_time(&h, &m, &s);

    if (h != last_hour || m != last_minute) {
        taskbar_draw_clock();
    }
}

/* ============================================================
 *  taskbar_handle_click - Procesare click pe taskbar
 * ============================================================ */
void taskbar_handle_click(int32_t x, int32_t y)
{
    (void)y; /* Nu folosim Y pentru acum */

    /* Click pe butonul START (x: 5-85) */
    if (x >= 5 && x <= 85) {
        /* TODO: Deschide meniu Start */
        /* In MVP: redesenam butonul ca "apasat" */
        uint32_t ty = TASKBAR_Y;
        vbe_fill_rect(5, ty + 5, 80, 30, COLOR_ACCENT_BLUE);
        vbe_draw_string(15, ty + 13, "START", COLOR_TASKBAR_BG, COLOR_TRANSPARENT);
    }
}
