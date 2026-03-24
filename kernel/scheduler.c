/* kernel/scheduler.c
 * LoraOS - Round-Robin Scheduler (Varianta Finala si Curata)
 */

#include "include/kernel.h"
#include "include/scheduler.h"

/* ---- Numarul maxim de task-uri ---- */
#define MAX_TASKS   8
#define STACK_SIZE  4096    /* 4KB stack per task */

/* ---- Structura unui task ---- */
typedef struct {
    uint32_t esp;       /* Stack pointer salvat */
    uint32_t eip;       /* Instruction pointer (unde am ramas) */
    uint8_t  stack[STACK_SIZE]; /* Stack-ul propriu al task-ului */
    uint8_t  active;    /* Este task-ul activ? */
    char     name[32];  /* Nume pentru debug */
} task_t;

/* ---- Lista de task-uri ---- */
static task_t   tasks[MAX_TASKS];
static uint32_t current_task  = 0;
static uint32_t task_count    = 0;

/* ============================================================
 * scheduler_init - Initializeaza scheduler-ul
 * ============================================================ */
void scheduler_init(void)
{
    task_count   = 0;
    current_task = 0;

    for (int i = 0; i < MAX_TASKS; i++) {
        tasks[i].active = 0; // FALSE
    }
}

/* ============================================================
 * scheduler_create_task - Adauga un task nou
 * ============================================================ */
void scheduler_create_task(void (*entry)(void), const char* name)
{
    if (task_count >= MAX_TASKS) return;

    task_t* t = &tasks[task_count];

    /* Copiaza numele */
    int i;
    for (i = 0; name[i] && i < 31; i++) t->name[i] = name[i];
    t->name[i] = '\0';

    /* Initializeaza stack-ul task-ului */
    t->esp = (uint32_t)(t->stack + STACK_SIZE - 4);
    t->eip = (uint32_t)entry;

    /* Pune adresa functiei pe stack */
    *((uint32_t*)t->esp) = (uint32_t)entry;

    t->active = 1; // TRUE
    task_count++;
}

/* ============================================================
 * scheduler_yield - Salveaza contextul si trece la urmatorul task
 * ============================================================ */
void scheduler_yield(void)
{
    if (task_count <= 1) return; 

    uint32_t next = (current_task + 1) % task_count;
    uint32_t searched = 0;

    while (!tasks[next].active && searched < task_count) {
        next = (next + 1) % task_count;
        searched++;
    }

    if (next == current_task) return; 

    uint32_t prev = current_task;
    current_task  = next;

    /* ASM Inline corectat pentru x86 (32-bit) */
    __asm__ volatile (
        "movl %%esp, %0\n\t"   
        "movl %2, %%esp\n\t"   
        "pushl %3\n\t"         
        "ret\n\t"              
        : "=m" (tasks[prev].esp)
        : "m" (tasks[prev].eip),
          "r" (tasks[next].esp),
          "r" (tasks[next].eip)
        : "memory"
    );
}

/* ============================================================
 * scheduler_get_current - ID-ul task-ului curent
 * ============================================================ */
uint32_t scheduler_get_current(void)
{
    return current_task;
}
