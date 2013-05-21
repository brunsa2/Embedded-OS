/**
 * USART driver
 *
 * Enables asynchronous serial communication, both receiving and transmitting, at single-character level
 *
 * @author Jeff Stubler
 * @date March 9, 2012
 */

#ifndef USART
#define USART

#include <inttypes.h>

#include <avr/io.h>

/**
 * Enable transmitting
 */
#define USART_TRANSMIT 0x01

/**
 * Enable receiving
 */
#define USART_RECEIVE 0x02

/**
 * Initialize USART with specified baud rate and options
 * @param flags Flags for options for serial port
 * @return Nothing
 */
void usart_init(uint8_t flags);

/**
 * Send one byte over USART
 * @param data Data byte to transmit
 * @param unusued Unused file descriptor to enable stdio use
 */
void usart_putc(char data);

/**
 * Send string over USART
 * @param string
 */
void usart_puts(char *string);

/**
 * Receive one byte over USART
 * @param unused Unused file descriptor to enable stdio use
 * @return Byte from USART
 */
char usart_getc(void); 

/**
 * Check if the USART has received a character
 * @return Number of available characters from the USART
 */
int usart_hasc(void);

#endif
