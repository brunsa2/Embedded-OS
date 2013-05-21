/**
 * ADC
 *
 * ATmega ADC control for data logger
 *
 * @author Jeff Stubler
 * @date November 13 2012
 */

#ifndef ADC
#define ADC

/**
 * Initialize ADC for logger
 */
void adc_init(void);

/**
 * Get data from ADC
 * @param channel Channel number 0-7
 * @return MS byte from ADC
 */
uint8_t adc_acquire(uint8_t channel);

#endif