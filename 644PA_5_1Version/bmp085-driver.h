/*
*
* Bosch BMP085 digital pressure sensor driver
*
* $Id: bmp085-driver.h 500 2010-12-11 00:21:40Z nuumio $
*
*/

#ifndef _BMP085_DRIVER_
#define _BMP085_DRIVER_

#include <stdint.h>

/* Oversampling settings */
typedef enum {
	bmp085_oss_0 = 0,	
	bmp085_oss_1 = 1,	
	bmp085_oss_2 = 2,	
	bmp085_oss_3 = 3
} bmp085_oss_t;

/* Return values */
#define BMP085_OK                 0
#define BMP085_ERROR_I2C         -1
#define BMP085_ERROR_MODE_CHANGE -2

/* Function prototypes */
int8_t bmp085_read_calibration_data(uint8_t *cdata);
int8_t bmp085_read_pressure(uint32_t *p_pr, bmp085_oss_t oss);
int8_t bmp085_read_temperature(uint16_t *p_temp);

#endif
