#include <xc.h>
#include "adc.h"


void adc_init(void)
{
    // Desliga o modulo durante a configuracao
    AD1CON1bits.ADON = 0;

    AD1PCFG = 0xFFFF;               // Todos os pinos digitais

    AD1CON1bits.FORM = 0b00;        // Resultado inteiro (0 a 1023)
    AD1CON1bits.SSRC = 0b111;       // Fim da amostragem inicia a conversao automaticamente
    AD1CON1bits.ASAM = 0;           // Amostragem iniciada por software (bit SAMP)

    AD1CON2 = 0;                    // Referencia AVdd/AVss, sem varredura, 1 amostra por interrupcao
    
    AD1CON3bits.ADRC = 0;           // Clock do ADC derivado do clock do sistema
    AD1CON3bits.SAMC = 16;          // Tempo de amostragem = 16 TAD
    AD1CON3bits.ADCS = 1;           // TAD = 2 * TCY = 500 ns (minimo exigido: 75 ns)

    AD1CON1bits.ADON = 1;           // Liga o modulo
}

void adc_config_channel(uint8_t channel)
{
    if (channel > 15) return;

    TRISB |= (1u << channel);       // Pino como entrada
    AD1PCFG &= ~(1u << channel);    // 0 = entrada analogica
}

uint16_t adc_read(uint8_t channel)
{
    if (channel > 15) return 0;

    AD1CHSbits.CH0SA = channel;     // Seleciona o canal analogico

    AD1CON1bits.DONE = 0;
    AD1CON1bits.SAMP = 1;           // Inicia a amostragem. A conversao comeca sozinha (SSRC = 111)

    while (!AD1CON1bits.DONE);      // Aguarda o fim da conversao

    return ADC1BUF0;
}

uint16_t adc_read_mv(uint8_t channel)
{
    uint32_t value = adc_read(channel);

    return (uint16_t)((value * 3300UL) / ADC_MAX_VALUE);
}

