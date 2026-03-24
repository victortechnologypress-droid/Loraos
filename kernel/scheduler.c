/* kernel/scheduler.c
 * LoraOS - Round-Robin Scheduler de baza
 * Permite rularea a doua procese simultan (ex: ceas + UI)
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
 *  scheduler_init - Initializeaza scheduler-ul
 * ============================================================ */
void scheduler_init(void)
{
    task_count   = 0;
    current_task = 0;

    for (int i = 0; i < MAX_TASKS; i++) {
        tasks[i].active = FALSE;
    }
}

/* ============================================================
 *  scheduler_create_task - Adauga un task nou
 *  entry: functia care va rula ca task
 *  name:  nume pentru debugging
 * ============================================================ */
void scheduler_create_task(void (*entry)(void), const char* name)
{
    if (task_count >= MAX_TASKS) return;

    task_t* t = &tasks[task_count];

    /* Copiaza numele */
    int i;
    for (i = 0; name[i] && i < 31; i++) t->name[i] = name[i];
    t->name[i] = '\0';

    /* Initializeaza stack-ul task-ului
     * Stack-ul creste in jos, asa ca ESP incepe la capatul de sus */
    t->esp = (uint32_t)(t->stack + STACK_SIZE - 4);
    t->eip = (uint32_t)entry;

    /* Pune adresa functiei pe stack (pentru primul context switch) */
    *((uint32_t*)t->esp) = (uint32_t)entry;

    t->active = TRUE;
    task_count++;
}

/* ============================================================
 *  scheduler_tick - Apelat la fiecare tick de timer
 *  Salveaza contextul curent si trece la urmatorul task
 *
 *  NOTE: In MVP, scheduler-ul este cooperativ (nu preemptiv).
 *  Task-urile apeleaza scheduler_yield() manual.
 *  Un scheduler preemptiv necesita IDT + IRQ0 (PIT timer).
 * ============================================================ */
void scheduler_yield(void)
{
    if (task_count <= 1) return; /* Nimic de schimbat */

    /* Gaseste urmatorul task activ (Round-Robin) */
    uint32_t next = (current_task + 1) % task_count;
    uint32_t searched = 0;

    while (!tasks[next].active && searched < task_count) {
        next = (next + 1) % task_count;
        searched++;
    }

    if (next == current_task) return; /* Doar un task activ */

    uint32_t prev = current_task;
    current_task  = next;

    /* Context switch in assembly inline */
    __asm__ volatile (
        "mov %%esp, %0\n"   /* Salveaza ESP curent */
        "mov %2, %%esp\n"   /* Incarca ESP-ul noului task */
        "push %3\n"          /* Pune EIP-ul noului task pe stack */
        "mov %1, %0\n"       /* Actualizeaza EIP salvat */
        "ret\n"              /* Sare la noul task */
        : "=m"(tasks[prev].esp)
        : "m"(tasks[prev].eip),
          "m"(tasks[next].esp),
          "m"(tasks[next].eip)
        : "memory"
    );
}

/* ============================================================
 *  scheduler_get_current - Returneaza ID-ul task-ului curent
 * ============================================================ */
uint32_t scheduler_get_current(void)
{
    return current_task;
}
