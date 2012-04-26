/*
*
* I2C Driver
* Compatible with SHT1X sensors' "almost I2C" protocol.
*
* $Id: i2c-driver.c 571 2011-02-20 19:53:19Z nuumio $
*
*/

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdbool.h>
#include "i2c-config.h"
#include "i2c-driver.h"

/* Data delays (in us) */
#define I2C_DELAY_SCL        4 /* Clock hi/low time */
#define I2C_DELAY_DATA_SETUP 1 /* Delay that data line takes to rise (Tr) plus "safety setup time" (Tsu) */
                               /* Ts and Tsu are together for the sake of simplicity. For more robust    */
							   /* communication they can be separate.                                    */
#define I2C_DELAY_DATA_HOLD  1 /* Delay that data line must be held after SCL has been set low.          */

/* Macros to help with pin control */
#define I2C_DATA_LOW()     (I2C_DATA_CONTROL |= _BV(I2C_DATA_PIN))
#define I2C_DATA_HIGH()    (I2C_DATA_CONTROL &= ~(_BV(I2C_DATA_PIN)))
#define I2C_DATA_RELEASE() (I2C_DATA_HIGH()) /* Means the same thing as high because we use pull-up to get data line high */
#define I2C_SCL_LOW()      (I2C_SCL_CONTROL |= _BV(I2C_SCL_PIN))
#define I2C_SCL_HIGH()     (I2C_SCL_CONTROL &= ~(_BV(I2C_SCL_PIN)))
#define I2C_SCL_RELEASE()  (I2C_SCL_HIGH())  /* Means the same thing as high because we use pull-up to get data line high */

/* Clock stretching: Wait until SCL is really high */
#define I2C_WAIT_CLOCK_STR() {while(!(I2C_SCL_PINS & _BV(I2C_SCL_PIN)));}

/* Prototypes for local helper functions */
void i2c_transmission_start(void);
void i2c_transmission_end(void);

/* I2C status bits */
volatile uint8_t i2c_sb;
#define I2C_SB_STARTED    0 /* Transmission started */
#define I2C_SB_MODE_SHT1X 1 /* SHT1X mode           */

/* Initialize SHT1x sensor */
void i2c_init(void) {
	/* Tri-state clock pin, set output low (when turned to output) and release it */
	I2C_SCL_CONTROL &= ~(_BV(I2C_SCL_PIN));
	I2C_SCL_PORT &= ~(_BV(I2C_SCL_PIN));
	I2C_SCL_RELEASE();

	/* Tri-state data pin, set output low (when turned to output) and release it */
	I2C_DATA_CONTROL &= ~(_BV(I2C_DATA_PIN));
	I2C_DATA_PORT &= ~(_BV(I2C_DATA_PIN));
	I2C_DATA_RELEASE();
}

/* Send transmission start sequence. */
void i2c_transmission_start(void) {
	/* Start common I2C transmission */
	I2C_SCL_RELEASE();
	I2C_DATA_RELEASE();
	_delay_us(I2C_DELAY_SCL);
	I2C_WAIT_CLOCK_STR();

	I2C_DATA_LOW();		
	_delay_us(I2C_DELAY_SCL);
	I2C_SCL_LOW();

	/* Start transmision for SHT1X (is I2C start + I2C stop) */
	if(i2c_sb & _BV(I2C_SB_MODE_SHT1X)) {
		_delay_us(I2C_DELAY_SCL);
		I2C_SCL_RELEASE();
		_delay_us(I2C_DELAY_SCL);
		I2C_DATA_RELEASE();
		_delay_us(I2C_DELAY_SCL);
		I2C_SCL_LOW();
		_delay_us(I2C_DELAY_SCL);
	}

	i2c_sb |= _BV(I2C_SB_STARTED);
}

/* Send transmission stop sequence. */
void i2c_transmission_stop(void) {
	/* Stop common I2C transmission (Let's do the same with SHT1X just in case) */
	I2C_SCL_RELEASE();
	I2C_DATA_LOW();
	_delay_us(I2C_DELAY_SCL);
	I2C_WAIT_CLOCK_STR();

	I2C_DATA_RELEASE();		
	_delay_us(I2C_DELAY_SCL);

	i2c_sb &= ~(_BV(I2C_SB_STARTED));
}

uint8_t i2c_read_bytes(uint8_t *buffer, uint16_t len, bool ack_last, bool stop) {
	uint8_t i;
	uint8_t p = 0;

	/* When reading from SHT1X the first bit is always 0. */
	/* Also, we must wait till SHT1X pulls data line low  */
	/* after asking for measurement and before reading    */
	/* the result. (Not sure if this works for all cmds.) */
	if(i2c_sb & _BV(I2C_SB_MODE_SHT1X)) {
		/* Wait until sensor pulls data line low */
		while((I2C_DATA_PINS & _BV(I2C_DATA_PIN)));
	}

	while(len > 0) {
		/* Read 8 bits of data. */
		for(i = 0; i < 8; i++) {
			/* Read data */
			buffer[p] <<= 1;
			if((I2C_DATA_PINS & _BV(I2C_DATA_PIN))) {
				buffer[p] |= 1; /* Set lsb high if data was high */
			}

			/* SCL high */
			I2C_SCL_RELEASE();
			_delay_us(I2C_DELAY_SCL);

			/* SCL low */
			I2C_SCL_LOW();
			_delay_us(I2C_DELAY_DATA_SETUP); /* Give data line enough time to rise if bit is high */
		}
		len--;
		p++;

		/* Send ACK or NACK */
		if(ack_last || len > 0) {
			/* Data line is free now, just pull to low and pulse SCL to ACK */
			/* Otherwise the data line should be high (released by slave)   */
			/* and pulsing SCL will NACK.                                   */
			I2C_DATA_LOW();
		}
		_delay_us(I2C_DELAY_DATA_SETUP);
		I2C_SCL_HIGH();
		_delay_us(I2C_DELAY_SCL);
		I2C_SCL_LOW();
		_delay_us(I2C_DELAY_SCL);
		I2C_DATA_RELEASE(); /* In case we ACKed */
		_delay_us(I2C_DELAY_DATA_SETUP);
	}

	if(stop) {
		i2c_transmission_stop();
	}

	return I2C_OK;
}

uint8_t i2c_write_bytes(const uint8_t *buffer, uint16_t len, bool start, bool stop) {
	uint8_t data;
	uint8_t p = 0;
	uint8_t i;
	uint8_t retval = I2C_OK;

	if(start) {
		i2c_transmission_start();
	}


	while(len > 0) {
		data = buffer[p++];
		len--;
		
		for(i = 0; i < 8; i++) {
			/* Set data line */
			if((data & 0x80)) {
				I2C_DATA_RELEASE();
			} else {
				I2C_DATA_LOW();
			}
			data <<= 1;
			_delay_us(I2C_DELAY_DATA_SETUP);

			/* SCL tick */
			I2C_SCL_RELEASE();
			_delay_us(I2C_DELAY_SCL);
			I2C_SCL_LOW();
			_delay_us(I2C_DELAY_SCL);
		}
		I2C_DATA_RELEASE();
		_delay_us(I2C_DELAY_DATA_SETUP);

		/* Check ACK from sensor */
		I2C_SCL_RELEASE();
		_delay_us(I2C_DELAY_SCL);
		if((I2C_DATA_PINS & _BV(I2C_DATA_PIN))) {
			/* No ACK (data remains high) */
			I2C_SCL_LOW();
			_delay_us(I2C_DELAY_SCL);
			retval = I2C_ERROR_NO_ACK;
			break;
		} else {
			/* ACK (data pulled low by sensor) */
			I2C_SCL_LOW();
			/* Wait until data line is released by sensor */
		}
	}

	if(stop) {
		i2c_transmission_stop();
	}

	return retval;
}

uint8_t i2c_set_mode(const uint8_t mode) {
	if(i2c_sb & I2C_SB_STARTED) {
		/* Can't switch mode when transmission is on */
		return I2C_ERROR_BUSY;
	}

	switch(mode) {
		case I2C_MODE_NORMAL:
			i2c_sb &= ~(_BV(I2C_SB_MODE_SHT1X));
			break;
		case I2C_MODE_SHT1X:
			i2c_sb |= _BV(I2C_SB_MODE_SHT1X);
			break;
		default:
			return I2C_ERROR_UNKNOWN_MODE;
	}

	return I2C_OK;
}
