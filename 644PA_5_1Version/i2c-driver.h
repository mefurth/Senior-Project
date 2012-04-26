/*
*
* I2C Driver
* Compatible with SHT1X sensors' "almost I2C" protocol.
*
* $Id: i2c-driver.h 500 2010-12-11 00:21:40Z nuumio $
*
*/

#ifndef _I2C_DRIVER_
#define _I2C_DRIVER_

#include <stdbool.h>

/* Return values */
#define I2C_OK                  0
#define I2C_ERROR_NO_ACK       -1
#define I2C_ERROR_BUSY         -2
#define I2C_ERROR_UNKNOWN_MODE -3

/* Modes */
#define I2C_MODE_NORMAL 0 /* Normal I2C (Default) */
#define I2C_MODE_SHT1X  1 /* SHT1X compatible     */

/* Error values */

/* Function prototypes */
void i2c_init(void);
void i2c_transmission_start(void);
void i2c_transmission_stop(void);
uint8_t i2c_read_bytes(uint8_t *buffer, uint16_t len, bool ack_last, bool stop);
uint8_t i2c_write_bytes(const uint8_t *buffer, uint16_t len, bool start, bool stop);
uint8_t i2c_set_mode(const uint8_t mode);
#endif
