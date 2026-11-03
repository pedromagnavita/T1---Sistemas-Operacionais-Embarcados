#include <builtins.h>

#include "scheduler.h"
#include "os_config.h"
#include "types.h"
#include "kernel.h"

extern queue_t ReadyQueue;
extern uint8_t task_running;
extern uint16_t *task_running_tos;


uint8_t scheduler()
{
#if DEFAULT_SCHEDULER == RR_SCHEDULER
    return rr_scheduler();
#else
    return prior_scheduler();
#endif    
}

uint8_t rr_scheduler()
{
    uint8_t next_task = task_running, idle_count = 0;
    
    do {
        //next_task = (next_task+1) % ReadyQueue.queue_size;
        next_task = __builtin_modsd(next_task+1, ReadyQueue.queue_size);
        
        if (ReadyQueue.tasks[next_task].task_func == idle) {
            idle_count++;
            if (idle_count >= 2) {
                return 0;
            }
        }
        
    } while (ReadyQueue.tasks[next_task].task_state != READY ||
             ReadyQueue.tasks[next_task].task_func == idle);
    
    return next_task;
}

uint8_t prior_scheduler()
{
    uint8_t candidate = task_running;
    uint8_t best = 0xFF;

    // Percorre a fila circularmente a partir da tarefa seguinte a que estava
    // rodando. A tarefa que estava rodando e a ultima a ser examinada.
    for (uint8_t i = 0; i < ReadyQueue.queue_size; i++) {
        candidate = __builtin_modsd(candidate+1, ReadyQueue.queue_size);

        if (ReadyQueue.tasks[candidate].task_state != READY) {
            continue;
        }

        // '>' estrito: em empate vence a primeira encontrada apos a atual,
        // o que produz o revezamento Round-Robin entre prioridades iguais.
        if (best == 0xFF ||
            ReadyQueue.tasks[candidate].task_prior > ReadyQueue.tasks[best].task_prior) {
            best = candidate;
        }
    }

    // Nenhuma tarefa pronta: executa a idle (sempre no indice 0)
    if (best == 0xFF) {
        return 0;
    }

    return best;
}



