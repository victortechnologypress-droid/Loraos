/* kernel/include/scheduler.h - Round-Robin Scheduler */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "kernel.h"

void     scheduler_init(void);
void     scheduler_create_task(void (*entry)(void), const char* name);
void     scheduler_yield(void);
uint32_t scheduler_get_current(void);

#endif
