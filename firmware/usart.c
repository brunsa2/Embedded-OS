/**
 * USART driver
 *
 * Enables asynchronous serial communication, both receiving and transmitting, at single-character level
 *
 * @author Jeff Stubler
 * @date March 9, 2012
 */

#include "usart.h"

volatile static uint8_t usart_is_initialized = 0;

#define BAUD_HIGH ((207 & 0xff00) >> 8)
#define BAUD_LOW (207 & 0x00ff)

 /**
  * Initialize USART with specified baud rate and options
  * @param clock MCU clock rate (Hz) to calculate the baud rate value for
  * @param baud Baud rate
  * @param flags Flags for options for serial port
  */
void usart_init(uint8_t flags) {
	UCSRB = 0;
	if (flags & USART_TRANSMIT) {
		// Enable transmitting
		UCSRB |= (1 << TXEN);
	}
	if (flags & USART_RECEIVE) {
		// Enable receiving with receive interrupts enabled
		UCSRB |= ((0 << RXCIE) | (1 << RXEN));
	}
	UCSRA = (1 << U2X);
	UBRRH = (unsigned char) BAUD_HIGH;
	UBRRL = (unsigned char) BAUD_LOW;

	usart_is_initialized = 1;
}

/**
 * Send one byte over USART
 * @param data Data byte to transmit
 */
 // TODO: Error codes
void usart_putc(char data) {
	if (usart_is_initialized) {
		while (!(UCSRA & (1 << UDRE)));
		UDR = data;
	}
}

/**
 * Send string over USART
 * @param string
 */
void usart_puts(char *string) {
    while (*string) {
        usart_putc(*string++);
    }
}

/**
 * Receive one byte over USART
 * @param unused Unused file descriptor to enable stdio use
 * @return Byte from USART
 */
// TODO: Error codes
char usart_getc(void) {
	if (usart_is_initialized) {
		while (!(UCSRA & (1 << RXC)));
		return UDR;
	}
	return 0;
}

/**
 * Check if the USART has received a character
 * @return Number of available characters from the USART
 */
int usart_hasc(void) {
	if (usart_is_initialized) {
		return (UCSRA & (1 << RXC)) >> 1;
	}
	return 0;
}
