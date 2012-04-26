/*
*
* Bosch BMP085 digital pressure sensor driver
*
* $Id: bmp085-driver.c 500 2010-12-11 00:21:40Z nuumio $
*
*/

#include <stdlib.h>
#include <stdint.h>
#include <util/delay.h>
#include "bmp085-driver.h"
#include "i2c-driver.h"

/* I2C Address and read/write bit (lsb) of BMP085 */
#define BMP085_ADDRESS_READ  0xEF
#define BMP085_ADDRESS_WRITE 0xEE 

/* Register addresses */
#define BMP085_REG_CONTROL   0xF4 /* Control register ("command register") */
#define BMP085_REG_CAL_START 0xAA /* Calibration data start (total 22 bytes/11 words) */
#define BMP085_REG_MSB       0xF6 /* Temperature/pressure value register MSB  */
#define BMP085_REG_LSB       0xF7 /* Temperature/pressure value register LSB  */
#define BMP085_REG_XLSB      0xF8 /* Temperature/pressure value register XLSB */

/* Control register values */
#define BMP085_CREG_TEMP   0x2E /* Temperature                        */
#define BMP085_CREG_PRESS  0x34 /* Pressure (over sampling setting 0) */

int8_t bmp085_read_calibration_data(uint8_t *cdata) {
	uint8_t data[] = {BMP085_ADDRESS_WRITE, BMP085_REG_CAL_START};

	/* Set I2C driver to normal mode */
	if(i2c_set_mode(I2C_MODE_NORMAL) != I2C_OK) {
		return BMP085_ERROR_MODE_CHANGE;
	}

	/* Write module address, control register address and control register data */
	if(i2c_write_bytes(data, 2, true, false) != I2C_OK) {
		return BMP085_ERROR_I2C;
	}

	/* Send read */
	data[0] = BMP085_ADDRESS_READ;
	if(i2c_write_bytes(data, 1, true, false) != I2C_OK) {
		return BMP085_ERROR_I2C;
	}

	/* Read 22 bytes */
	if(i2c_read_bytes(cdata, 22, false, true) != I2C_OK) {
		return BMP085_ERROR_I2C;
	}

	/* Swap endianess to make correct values */
	for(data[0] = 0; data[0] < 11; data[0]++) {
		data[1] = cdata[data[0] * 2 + 1];
		cdata[data[0] * 2 + 1] = cdata[data[0] * 2];
		cdata[data[0] * 2] = data[1];
	}

	return BMP085_OK;
}

int8_t bmp085_read_pressure(uint32_t *p_pr, bmp085_oss_t oss) {
	/* Set I2C driver to normal mode */
	if(i2c_set_mode(I2C_MODE_NORMAL) != I2C_OK) {
		return BMP085_ERROR_MODE_CHANGE;
	}

	/* Write module address, control register address and control register data */
	uint8_t data[] = {BMP085_ADDRESS_WRITE, BMP085_REG_CONTROL, 0};

	/* Control register value depends on oss */
	data[2] = BMP085_CREG_PRESS | (uint8_t)oss << 6;

	/* Write module address, control register address and control register data */
	if(i2c_write_bytes(data, 3, true, true) != I2C_OK) {
		return BMP085_ERROR_I2C;
	}

#if 0
	/* Wait for conversion complete (depends on oss) */
	switch(oss) {
		case bmp085_oss_0:
			_delay_ms(5);
			break;
		case bmp085_oss_1:
			_delay_ms(8);
			break;
		case bmp085_oss_2:
			_delay_ms(14);
			break;
		case bmp085_oss_3:
		default:
			_delay_ms(26);
			break;
	}
#endif
#if 0
	/* This takes over 800 bytes of space! */
	_delay_ms(2 + (3 << ((uint8_t)oss)));
#endif
	/* Save some flash and always wait enough */
	_delay_ms(26);


	/* Set register to MSB */
	data[0] = BMP085_ADDRESS_WRITE;
	data[1] = BMP085_REG_MSB;
	if(i2c_write_bytes(data, 2, true, false) != I2C_OK) {
		return BMP085_ERROR_I2C;
	}

	/* Send read */
	data[0] = BMP085_ADDRESS_READ;
	if(i2c_write_bytes(data, 1, true, false) != I2C_OK) {
		return BMP085_ERROR_I2C;
	}

	/* Read MSB, LSB and XLSB */
	if(i2c_read_bytes(data, 3, false, true) != I2C_OK) {
		return BMP085_ERROR_I2C;
	}

	/* Align bytes to little endian */
	*p_pr = 0;
	*p_pr = (((uint32_t)data[0] << 16) | ((uint32_t)data[1] << 8) | (uint32_t)data[2]) >> (8-oss);
	
	return BMP085_OK;
}

int8_t bmp085_read_temperature(uint16_t *p_temp) {
	uint8_t data[] = {BMP085_ADDRESS_WRITE, BMP085_REG_CONTROL, BMP085_CREG_TEMP};

	/* Set I2C driver to normal mode */
	if(i2c_set_mode(I2C_MODE_NORMAL) != I2C_OK) {
		return BMP085_ERROR_MODE_CHANGE;
	}

	/* Write module address, control register address and control register data */
	if(i2c_write_bytes(data, 3, true, true) != I2C_OK) {
		return BMP085_ERROR_I2C;
	}

	/* Wait for conversion complete (max time = 4.5ms) */
	_delay_ms(5);

	/* Set register to MSB */
	data[0] = BMP085_ADDRESS_WRITE;
	data[1] = BMP085_REG_MSB;
	if(i2c_write_bytes(data, 2, true, false) != I2C_OK) {
		return BMP085_ERROR_I2C;
	}

	/* Send read */
	data[0] = BMP085_ADDRESS_READ;
	if(i2c_write_bytes(data, 1, true, false) != I2C_OK) {
		return BMP085_ERROR_I2C;
	}

	/* Read MSB and LSB */
	if(i2c_read_bytes(data, 2, false, true) != I2C_OK) {
		return BMP085_ERROR_I2C;
	}
	
	/* Align bytes to little endian */
	*p_temp = ((uint16_t)data[0] << 8) | ((uint16_t)data[1]);

	return BMP085_OK;
}


