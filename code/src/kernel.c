#include "kernel.h"
#include <stdio.h>
#include "user_app.h"
#include "timer.h"
#include "int.h"
#include <xc.h>
#include "mem.h"

// Variáveis globais
queue_t ReadyQueue;
uint8_t task_running = 0;
uint16_t *task_running_tos = NULL;

void os_config(void)
{
    ReadyQueue.queue_size   = 0;
    config_timer_0();
    config_int0();
    #if DYNAMIC_ALLOCATION == ON
    os_heap_init();
    #endif
    
    config_user_app();
    
    create_task(idle, 0);
}

void os_start_scheduler(void)
{
    task_running_tos = ReadyQueue.tasks[0].sp;    
    T1CONbits.TON = 1;
    RESTORE_CONTEXT();
}

void create_task(callback task, uint8_t prior)
{
    tcb_t *new_task = &ReadyQueue.tasks[ReadyQueue.queue_size]; // pega o ponteiro para a TCB da nova tarefa
    
    new_task->task_id       = ReadyQueue.queue_size+1;
    new_task->task_func     = task;
    new_task->task_prior    = prior;
    new_task->task_base_prior = prior;
    new_task->task_state    = READY;
    new_task->task_delay    = 0;
    #if DYNAMIC_ALLOCATION == ON
    new_task->task_stack = (uint16_t *)os_malloc(MAX_STACK_SIZE * sizeof(uint16_t)); // aloca a pilha dinamicamente
    if (new_task->task_stack == NULL) { // evitar falha de alocação
        return; 
    }
    #endif

    // Ajustar o SP
    new_task->sp            = &new_task->task_stack[0]; // aponta para o topo da pilha
    
    // Inicializar a pilha
    init_stack(new_task);
    
    // Insere na fila de aptos    
    ReadyQueue.queue_size++;
}

void init_stack(tcb_t *task)
{
    uint16_t pc_low, pc_high, *sp;
    
    sp = task->sp;
    
    pc_low  = (uint16_t)task->task_func;
    pc_high = 0x0000;
    
    *sp++ = pc_low;
    *sp++ = pc_high;
    
    // Inicializar os Ws (W0 até W14)
    for (int i = 0; i < 15; i++) {
        *sp++ = 0x0000;
    }
  
    *sp++ = 0x0000; // RCOUNT
    *sp++ = TBLPAG; // TBLPAG
    *sp++ = PSVPAG; // PSVPAG
    *sp++ = CORCON; // CORCON
    
    task->sp = sp;
}

void task_yield()
{   
    __builtin_disable_interrupts();

    ReadyQueue.tasks[task_running].task_state = READY;
    CONTEXT_SWITCH();

    __builtin_enable_interrupts();
}

void task_delay(uint16_t time)
{
    __builtin_disable_interrupts();
    
    //__builtin_disi(16383);
    
    ReadyQueue.tasks[task_running].task_delay = time;
    ReadyQueue.tasks[task_running].task_state = WAITING;
    CONTEXT_SWITCH();
    
    __builtin_enable_interrupts();
}

void delay_release()
{
    for (int i = 0; i < ReadyQueue.queue_size; i++) {
        if (ReadyQueue.tasks[i].task_state == WAITING) {
            if (ReadyQueue.tasks[i].task_delay > 0) {
                ReadyQueue.tasks[i].task_delay--;
            }
            if (ReadyQueue.tasks[i].task_delay == 0) {
                ReadyQueue.tasks[i].task_state = READY;
            }
        }
    }
}

uint8_t task_get_priority(void)
{
    return ReadyQueue.tasks[task_running].task_prior; // retorna a prioridade efetiva da tarefa em execução
}

TASK idle()
{
    while (1) {
        asm("NOP");
        Nop();
    }
}