#include <xc.h>
#include "user_app.h"
#include "kernel.h"
#include "sync.h"
#include "message.h"
#include "io.h"


// tick do escalonador = 4 ms
#define TICK_MS             4
#define MS_TO_TICKS(ms)     ((ms) / TICK_MS)


m_queue_t fila_duty;                    // ADC -> PWM (valor do duty em %)
sem_t     sem_ack;                      // PWM avisa a ADC que aplicou o duty
mutex_t   mtx_uart;                     // exclusao mutua no terminal (UART)
mutex_t   mtx_recurso;                  // protege o recurso compartilhado abaixo
volatile uint16_t recurso_compartilhado = 0;


// Espera ocupada de ~ms milissegundos de CPU (calibragem aproximada)
static void busy_ms(uint16_t ms)
{
    while (ms--) {
        for (volatile uint16_t i = 0; i < 400; i++);
    }
}

// Log na UART protegido por mutex (uma linha nunca se mistura com outra)
static void log_msg(const char *msg)
{
    mutex_lock(&mtx_uart);
    UART_write_new_line(msg);
    mutex_unlock(&mtx_uart);
}

static void log_msg_int(const char *msg, int value)
{
    mutex_lock(&mtx_uart);
    UART_write_string(msg);
    UART_write_int(value);
    UART_write_string("\r\n");
    mutex_unlock(&mtx_uart);
}



// Prioridade alta: precisa do recurso que a tarefa baixa costuma estar usando.
// Ao bloquear no mutex, faz a baixa herdar a prioridade alta.
TASK tarefa_alta(void)
{
    task_delay(MS_TO_TICKS(200));       // deixa a baixa pegar o mutex primeiro

    while (1) {
        log_msg("[ALTA] quer o recurso");

        mutex_lock(&mtx_recurso);
        recurso_compartilhado++;
        log_msg_int("[ALTA] obteve o recurso, contador = ", recurso_compartilhado);
        mutex_unlock(&mtx_recurso);

        task_delay(MS_TO_TICKS(600));
    }
}

// Prioridade média (A e B): mesma prioridade, ficam sempre prontas ao mesmo
// tempo e se revezam por Round-Robin. Tambem ocupam a CPU, o que causaria
// inversao de prioridade sem a heranca.
TASK tarefa_rr_a(void)
{
    task_delay(MS_TO_TICKS(100));

    while (1) {
        for (int passo = 1; passo <= 4; passo++) {
            busy_ms(8);
            LATEbits.LATE0 ^= 1;
            log_msg_int("[RR-A] passo ", passo);
        }
        task_delay(MS_TO_TICKS(500));
    }
}

TASK tarefa_rr_b(void)
{
    task_delay(MS_TO_TICKS(100));

    while (1) {
        for (int passo = 1; passo <= 4; passo++) {
            busy_ms(8);
            LATEbits.LATE1 ^= 1;
            log_msg_int("[RR-B] passo ", passo);
        }
        task_delay(MS_TO_TICKS(500));
    }
}

// Produtora: le o ADC (API de E/S), converte para duty (0 a 100 %) e envia pela
// fila de mensagens. Depois espera o semaforo, avisando que o PWM foi atualizado.
TASK tarefa_adc(void)
{
    uint16_t leitura;
    uint16_t duty;

    while (1) {
        leitura = adc_read(ADC_CHANNEL_0);
        duty    = (uint16_t)(((uint32_t)leitura * 100UL) / ADC_MAX_VALUE);

        log_msg_int("[ADC] leitura = ", leitura);

        m_queue_write(&fila_duty, (char)duty);      // comunicacao por fila de mensagens
        sem_wait(&sem_ack);                         // sincronizacao por semaforo

        task_delay(MS_TO_TICKS(500));
    }
}

// Consumidora: recebe o duty pela fila e ajusta o PWM (API de E/S).
TASK tarefa_pwm(void)
{
    pwm_driver_t pwm;
    char duty;

    pwm_config(&pwm, 1000);         // 1 kHz no pino RD0/OC1
    pwm_start(&pwm);

    while (1) {
        m_queue_read(&fila_duty, &duty);
        pwm_set_duty(&pwm, (uint16_t)duty);

        log_msg_int("[PWM] duty = ", duty);

        sem_post(&sem_ack);         // libera a tarefa ADC
    }
}

// Prioridade baixa: segura o mutex por muito tempo. Quando a alta bloqueia
// esperando por ele, esta tarefa herda a prioridade alta ate liberar o mutex.
// Os logs sao feitos fora da secao critica, depois do unlock.
TASK tarefa_baixa(void)
{
    uint8_t prior_inicio, prior_durante;

    while (1) {
        mutex_lock(&mtx_recurso);
        LATEbits.LATE2 = 1;                         // LED aceso = dentro da secao critica

        prior_inicio = task_get_priority();
        recurso_compartilhado++;
        busy_ms(300);                               // secao critica longa
        prior_durante = task_get_priority();        // 5 se a ALTA ficou bloqueada esperando

        LATEbits.LATE2 = 0;
        mutex_unlock(&mtx_recurso);

        log_msg_int("[BAIXA] prioridade ao pegar o mutex    = ", prior_inicio);
        log_msg_int("[BAIXA] prioridade no fim da sec. crit. = ", prior_durante);
        log_msg_int("[BAIXA] prioridade apos o unlock       = ", task_get_priority());

        task_delay(MS_TO_TICKS(200));
    }
}


void config_user_app(void)
{
    TRISEbits.TRISE0 = 0;
    TRISEbits.TRISE1 = 0;
    TRISEbits.TRISE2 = 0;
    TRISFbits.TRISF3 = 0;

    UART_init(9600);

    adc_init();
    adc_config_channel(ADC_CHANNEL_0);      // potenciometro em RB0/AN0

    m_queue_init(&fila_duty);
    sem_init(&sem_ack, 0);
    mutex_init(&mtx_uart);
    mutex_init(&mtx_recurso);
}
