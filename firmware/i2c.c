/**
 * I2C
 *
 * ATmega I2C interface driver
 *
 * @author Jeff Stubler
 * @date November 13 2012
 */

#include "i2c.h"

/**
 * Initialize I2C for 100 kHz operation
 */
void i2c_init(void) {
    TWBR = 5;
    TWCR = 0b10000100; // Enable device and clear flags
    TWSR = 0b00000010; // Prescale by 16
}

/**
 * Send start condition
 */
int8_t i2c_start(void) {
    TWCR |= 0b10100000; // Start
    while (!(TWCR & 0b10000000)); // Wait for TWINT
    if ((TWSR & 0xf8) != I2C_START) {
        return TWSR & 0xf8;
    }
    return 0;
}

/**
 * Send address and read/write bite
 * @param address Address and read/write bit
 * @return Error code
 */
int8_t i2c_send_address(uint8_t address) {
    TWDR = address;
    TWCR |= 0b1000000; // Clear interrupt flag to send
    while (!(TWCR & 0b1000000)); // Wait for TWINT
    if ((TWSR && 0xf8) != I2C_MT_SLAVE_ACK) {
        return TWSR & 0xf8;
    }
    return 0;
}

/**
 * Send stop condition
 * @return Error code
 */
int8_t i2c_stop(void) {
    TWCR |= 0b10010000; // Stop
    return 0;
}