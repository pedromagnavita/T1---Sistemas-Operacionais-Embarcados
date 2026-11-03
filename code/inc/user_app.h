#ifndef USER_APP_H
#define	USER_APP_H

#include "types.h"

// Prioridades das tarefas (numero maior = prioridade maior)
#define PRIO_ALTA       5
#define PRIO_MEDIA      3
#define PRIO_ADC_PWM    2
#define PRIO_BAIXA      1

TASK tarefa_alta(void);     // usa o mutex do recurso (sofre a inversao de prioridade)
TASK tarefa_rr_a(void);     // Round-Robin com tarefa_rr_b (mesma prioridade)
TASK tarefa_rr_b(void);
TASK tarefa_adc(void);      // le o ADC e envia pela fila de mensagens
TASK tarefa_pwm(void);      // recebe da fila e gera o PWM
TASK tarefa_baixa(void);    // segura o mutex por muito tempo (recebe heranca)

void config_user_app(void);


#endif	/* USER_APP_H */
