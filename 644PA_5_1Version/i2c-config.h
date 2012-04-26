/*
*
* I2C Driver pin configuration for main unit
*
* $Id: i2c-config.h 540 2010-12-26 14:27:05Z nuumio $
*
*/

#ifndef _I2C_CONFIG_
#define _I2C_CONFIG_

/* Config for the sensor */
#define I2C_SCL_PORT      (PORTB)
#define I2C_SCL_PINS      (PINB)
#define I2C_SCL_PIN       (PB6)
#define I2C_SCL_CONTROL   (DDRB)
#define I2C_DATA_PORT     (PORTB)
#define I2C_DATA_PINS     (PINB)
#define I2C_DATA_PIN      (PB7)
#define I2C_DATA_CONTROL  (DDRB)

#endif
