/* kernel/pmm.c
 * LoraOS - Physical Memory Manager
 * Gestioneaza RAM-ul fizic prin bitmap de pagini (4KB fiecare)
 */

#include "include/kernel.h"
#include "include/pmm.h"

/* ---- Configuratie ---- */
#define PAGE_SIZE       4096            /* 4KB per pagina */
#define MAX_PAGES       (4096 * 256)    /* Suport pana la 4GB RAM */

/* ---- Bitmap: 1 bit per pagina (1=ocupat, 0=liber) ---- */
static uint32_t pmm_bitmap[MAX_PAGES / 32];
static uint32_t total_pages = 0;
static uint32_t free_pages  = 0;

/* ---- Macro-uri pentru bitmap ---- */
#define BIT_SET(map, bit)   ((map)[(bit)/32] |=  (1 << ((bit)%32)))
#define BIT_CLEAR(map, bit) ((map)[(bit)/32] &= ~(1 << ((bit)%32)))
#define BIT_TEST(map, bit)  ((map)[(bit)/32] &   (1 << ((bit)%32)))

/* ============================================================
 *  pmm_init - Initializeaza managerul de memorie
 *  total_kb: total RAM disponibil in kilobytes
 * ============================================================ */
void pmm_init(uint32_t total_kb)
{
    total_pages = (total_kb * 1024) / PAGE_SIZE;
    if (total_pages > MAX_PAGES) total_pages = MAX_PAGES;

    /* Marcheaza toata memoria ca ocupata implicit */
    kmemset(pmm_bitmap, 0xFF, sizeof(pmm_bitmap));

    /* Elibereaza memoria disponibila (deasupra primului 1MB + kernel) */
    /* Primele 4MB sunt rezervate pentru kernel si hardware */
    uint32_t kernel_end_page = (4 * 1024 * 1024) / PAGE_SIZE; /* 4MB */

    for (uint32_t i = kernel_end_page; i < total_pages; i++) {
        BIT_CLEAR(pmm_bitmap, i);
        free_pages++;
    }
}

/* ============================================================
 *  pmm_alloc_page - Aloca o pagina fizica de 4KB
 *  Returneaza adresa fizica sau NULL daca nu mai e memorie
 * ============================================================ */
void* pmm_alloc_page(void)
{
    for (uint32_t i = 0; i < total_pages; i++) {
        if (!BIT_TEST(pmm_bitmap, i)) {
            BIT_SET(pmm_bitmap, i);
            free_pages--;
            return (void*)(i * PAGE_SIZE);
        }
    }
    return NULL; /* Out of memory! */
}

/* ============================================================
 *  pmm_free_page - Elibereaza o pagina fizica
 * ============================================================ */
void pmm_free_page(void* addr)
{
    uint32_t page = (uint32_t)addr / PAGE_SIZE;
    if (page < total_pages && BIT_TEST(pmm_bitmap, page)) {
        BIT_CLEAR(pmm_bitmap, page);
        free_pages++;
    }
}

/* ============================================================
 *  pmm_free_count - Returneaza numarul de pagini libere
 * ============================================================ */
uint32_t pmm_free_count(void)
{
    return free_pages;
}
