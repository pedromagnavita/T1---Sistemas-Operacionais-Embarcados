#include "kernel.h"
#include "user_app.h"

int main()
{
    // Cria as estruturas de dados do SO
    os_config();
    
    // Cria as tarefas de usuário
    create_task(tarefa_alta, PRIO_ALTA);
    create_task(tarefa_rr_a, PRIO_MEDIA);
    create_task(tarefa_rr_b, PRIO_MEDIA);
    create_task(tarefa_adc,  PRIO_ADC_PWM);
    create_task(tarefa_pwm,  PRIO_ADC_PWM);
    create_task(tarefa_baixa, PRIO_BAIXA);
    
    // Inicializa o escalonador
    os_start_scheduler();
    
    while (1);
    
    return 0;
}
