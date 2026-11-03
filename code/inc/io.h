#ifndef IO_H
#define	IO_H

// API de Entrada/Saida do RTOS.
// Reune os drivers dos perifericos em um unico ponto de acesso:
//   - UART : uart.h  (UART_init, UART_write_*)
//   - ADC  : adc.h   (adc_init, adc_config_channel, adc_read, adc_read_mv)
//   - PWM  : pwm.h   (pwm_config, pwm_start, pwm_stop, pwm_set_duty)

#include "uart.h"
#include "adc.h"
#include "pwm.h"

#endif	/* IO_H */
