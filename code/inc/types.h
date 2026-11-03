#ifndef TYPES_H
#define	TYPES_H

#include "os_config.h"
#include <stdint.h>

//#define IDLE 1

typedef void (*callback)(void);

typedef void TASK;

typedef enum {READY = 0, RUNNING, WAITING, BLOCKED} state_t;

typedef struct tcb {
    uint8_t task_id;
    uint8_t task_prior;         // prioridade efetiva (pode ser herdada)
    uint8_t task_base_prior;    // prioridade original (restaurada no unlock)
    uint16_t task_delay;
    callback task_func;
    state_t task_state;
#if DYNAMIC_ALLOCATION == ON
    uint16_t *task_stack;       // pilha alocada com os_malloc em create_task
#else
    uint16_t task_stack[MAX_STACK_SIZE];
#endif
    uint16_t *sp;
} tcb_t;

typedef struct queue {
    tcb_t tasks[MAX_TASKS+1];
    uint8_t queue_size;
} queue_t;

typedef struct sem
{
    uint8_t sem_queue[MAX_TASKS];
    int sem_count;
    uint8_t s_queue_input;
    uint8_t s_queue_output;
} sem_t;

#define MUTEX_NO_OWNER  0xFF

typedef struct mutex {
    uint8_t locked;                 // 0 = livre, 1 = ocupado
    uint8_t owner;                  // indice (em ReadyQueue.tasks) da tarefa dona
    uint8_t wait_queue[MAX_TASKS];  // tarefas bloqueadas (em ordem de chegada)
    uint8_t wait_count;             // quantas tarefas estao esperando
} mutex_t;

typedef struct m_queue {
    char queue[SIZE_MESSAGE_QUEUE];
    uint8_t pos_write;
    uint8_t pos_read;
    sem_t s_write;
    sem_t s_read;
} m_queue_t;

typedef struct block_info {
    uint16_t size;
    uint8_t free;
} block_info_t;

#endif	/* TYPES_H */

