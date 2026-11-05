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
    uint8_t candidate = task_running; //task que saiu da CPU
    uint8_t best = 0xFF;

    for (uint8_t i = 0; i < ReadyQueue.queue_size; i++) { 
        candidate = __builtin_modsd(candidate+1, ReadyQueue.queue_size); //percorre a partir de task_running + 1

        if (ReadyQueue.tasks[candidate].task_state != READY) {
            continue; // ignora tarefas que não estão prontas
        }

        if (best == 0xFF || 
            ReadyQueue.tasks[candidate].task_prior > ReadyQueue.tasks[best].task_prior) {
            best = candidate; // atualiza a maior prioridade encontrada, se empatar, fica com a primeira encontrada
        }
    }
    if (best == 0xFF) { // se não encontrou nenhuma tarefa pronta, retorna a idle
        return 0;
    }

    return best;
}



