/**
 * ADC
 *
 * ATmega ADC control for data logger
 *
 * @author Jeff Stubler
 * @date November 13 2012
 */

#include <avr/io.h>
#include "adc.h"

/**
 * Initialize ADC for logger
 */
void adc_init(void) {
    ADMUX = 0b00100000; // AREF, left-adjusted, channel 0
    ADCSRA = 0b10010111; // Enabled, clear interrupt flag, interrupts disabled, clk/128
}

/**
 * Get data from ADC
 * @param channel Channel number 0-7
 * @return MS byte from ADC
 */
uint8_t adc_acquire(uint8_t channel) {
    channel &= 0x07; // Single-ended only
    ADMUX = 0b00100000 | channel; // AREF, left-adjusted, channel as given
    ADCSRA |= 0b01000000; // Start
    while (!(ADCSRA & 0b00010000)); // Wait for interrupt flag
    ADCSRA |= 0b00010000; // Clear interrupt flag
    return ADCH;
}