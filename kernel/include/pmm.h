/* kernel/include/pmm.h - Physical Memory Manager */

#ifndef PMM_H
#define PMM_H

#include "kernel.h"

void     pmm_init(uint32_t total_kb);
void*    pmm_alloc_page(void);
void     pmm_free_page(void* addr);
uint32_t pmm_free_count(void);

#endif
