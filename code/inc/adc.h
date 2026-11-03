#ifndef ADC_H
#define	ADC_H

#include <stdint.h>

#define ADC_MAX_VALUE   1023    // conversor de 10 bits
#define ADC_CHANNEL_0   0       // AN0 -> pino RB0
#define ADC_CHANNEL_1   1       // AN1 -> pino RB1

// Inicializa o modulo ADC (10 bits, resultado inteiro, conversao automatica)
void adc_init(void);

// Configura o pino ANx como entrada analogica
void adc_config_channel(uint8_t channel);

// Realiza uma leitura (0 a 1023) do canal informado
uint16_t adc_read(uint8_t channel);

// Retorna a leitura convertida para milivolts (referencia de 3300 mV)
uint16_t adc_read_mv(uint8_t channel);

#endif	/* ADC_H */

