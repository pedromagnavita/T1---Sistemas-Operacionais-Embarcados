#include "sync.h"
#include "int.h"
#include <xc.h>


// Variáveis globais
extern queue_t ReadyQueue;
extern uint8_t task_running;
extern uint16_t *task_running_tos;


void sem_init(sem_t *s, uint8_t value)
{
    s->sem_count        = value;
    s->s_queue_input    = 0;
    s->s_queue_output   = 0;
}

void sem_wait(sem_t *s)
{
    __builtin_disable_interrupts();
    
    s->sem_count--;
    
    if (s->sem_count < 0) {
        // Bloqueia a tarefa
        s->sem_queue[s->s_queue_input] = task_running;
        s->s_queue_input = __builtin_modsd(s->s_queue_input+1, MAX_TASKS);
        //s->s_queue_input = (s->s_queue_input+1) % MAX_TASKS;
        ReadyQueue.tasks[task_running].task_state = BLOCKED;
        CONTEXT_SWITCH();        
    }
    
    __builtin_enable_interrupts();    
}

void sem_post(sem_t *s)
{
    __builtin_disable_interrupts();
    
    s->sem_count++;
    
    if (s->sem_count <= 0) {
        ReadyQueue.tasks[s->sem_queue[s->s_queue_output]].task_state = READY;
        s->s_queue_output = __builtin_modsd(s->s_queue_output+1, MAX_TASKS);
        //s->s_queue_output = (s->s_queue_output+1) % MAX_TASKS;
        ReadyQueue.tasks[task_running].task_state = READY;
        CONTEXT_SWITCH(); 
    }
    
    __builtin_enable_interrupts();    
}


void mutex_init(mutex_t *m)
{
    m->locked       = 0;
    m->owner        = MUTEX_NO_OWNER;
    m->wait_count   = 0;
}

void mutex_lock(mutex_t *m)
{
    __builtin_disable_interrupts();

    if (!m->locked) {
        // Mutex livre: a tarefa atual passa a ser a dona
        m->locked = 1;
        m->owner  = task_running;
    } else {
        // Mutex ocupado: entra na fila de espera e bloqueia
        m->wait_queue[m->wait_count++] = task_running;

        // Heranca de prioridade: a dona herda a prioridade da tarefa que bloqueou
        if (ReadyQueue.tasks[m->owner].task_prior < ReadyQueue.tasks[task_running].task_prior) {
            ReadyQueue.tasks[m->owner].task_prior = ReadyQueue.tasks[task_running].task_prior;
        }

        ReadyQueue.tasks[task_running].task_state = BLOCKED;
        CONTEXT_SWITCH();
    }

    __builtin_enable_interrupts();
}

void mutex_unlock(mutex_t *m)
{
    __builtin_disable_interrupts();

    // Somente a dona pode liberar o mutex
    if (m->locked && m->owner == task_running) {
        tcb_t *me = &ReadyQueue.tasks[task_running];
        uint8_t reschedule = 0;

        // Restaura a prioridade original da dona (fim da heranca)
        if (me->task_prior != me->task_base_prior) {
            me->task_prior = me->task_base_prior;
            reschedule = 1;
        }

        if (m->wait_count > 0) {
            uint8_t best = 0, next;

            // Escolhe a tarefa de maior prioridade entre as que esperam
            // ('>' estrito: em empate vence a que chegou primeiro)
            for (uint8_t i = 1; i < m->wait_count; i++) {
                if (ReadyQueue.tasks[m->wait_queue[i]].task_prior >
                    ReadyQueue.tasks[m->wait_queue[best]].task_prior) {
                    best = i;
                }
            }

            next = m->wait_queue[best];

            // Remove a escolhida da fila, mantendo a ordem das demais
            for (uint8_t i = best; i + 1 < m->wait_count; i++) {
                m->wait_queue[i] = m->wait_queue[i+1];
            }
            m->wait_count--;

            // Passa a posse diretamente para ela
            m->owner = next;
            ReadyQueue.tasks[next].task_state = READY;
            reschedule = 1;

            // A nova dona herda a maior prioridade entre as que ainda esperam
            for (uint8_t i = 0; i < m->wait_count; i++) {
                uint8_t p = ReadyQueue.tasks[m->wait_queue[i]].task_prior;
                if (ReadyQueue.tasks[next].task_prior < p) {
                    ReadyQueue.tasks[next].task_prior = p;
                }
            }
        } else {
            m->locked = 0;
            m->owner  = MUTEX_NO_OWNER;
        }

        // Ceder a CPU para o escalonador reavaliar as prioridades
        if (reschedule) {
            me->task_state = READY;
            CONTEXT_SWITCH();
        }
    }

    __builtin_enable_interrupts();
}