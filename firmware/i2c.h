/**
 * I2C
 *
 * ATmega I2C interface driver
 *
 * @author Jeff Stubler
 * @date November 13 2012
 */

#ifndef I2C
#define I2C

#include <avr/io.h>

#define I2C_START 0x08
#define I2C_MT_SLAVE_ACK 0x18

/**
 * Initialize I2C for 100 kHz operation
 */
void i2c_init(void);

/**
 * Send start condition
 * @return Error code
 */
int8_t i2c_start(void);

/**
 * Send address and read/write bite
 * @param address Address and read/write bit
 * @return Error code
 */
int8_t i2c_send_address(uint8_t address);

/**
 * Send stop condition
 * @return Error code
 */
int8_t i2c_stop(void);


#endif