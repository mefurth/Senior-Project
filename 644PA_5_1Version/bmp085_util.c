/*
*
* BMP085 Utility functions. This file is adopted from the original
* file made by Bosch Sensortec GmbH. See below for original copyright,
* licencing information and disclaimer.
*
* $Id: bmp085_util.c 645 2011-03-05 00:37:47Z nuumio $
*
*/

/*
* Copyright (C) 2009 Bosch Sensortec GmbH
*
* BMP085 pressure sensor API
* 
* Usage:  Application Programming Interface for BMP085 configuration and data read out
*
* 
* Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in 
  compliance with the License and the following stipulations. The Apache License , Version 2.0 is applicable unless 
  otherwise stated by the stipulations of the disclaimer below. 
 
* You may obtain a copy of the License at 

   http://www.apache.org/licenses/LICENSE-2.0
  
 

Disclaimer 

* Common:
* This Work is developed for the consumer goods industry. It may only be used 
* within the parameters of the respective valid product data sheet.  The Work 
* provided with the express understanding that there is no warranty of fitness for a particular purpose. 
* It is not fit for use in life-sustaining, safety or security sensitive systems or any system or device 
* that may lead to bodily harm or property damage if the system or device malfunctions. In addition, 
* the Work is not fit for use in products which interact with motor vehicle systems.  
* The resale and/or use of the Work are at the purchaser’s own risk and his own responsibility. The 
* examination of fitness for the intended use is the sole responsibility of the Purchaser. 
*
* The purchaser shall indemnify Bosch Sensortec from all third party claims, including any claims for 
* incidental, or consequential damages, arising from any Work or Derivative Work use not covered by the parameters of 
* the respective valid product data sheet or not approved by Bosch Sensortec and reimburse Bosch 
* Sensortec for all costs in connection with such claims.
*
* The purchaser must monitor the market for the purchased Work and Derivative Works, particularly with regard to 
* product safety and inform Bosch Sensortec without delay of all security relevant incidents.
*
* Engineering Samples are marked with an asterisk (*) or (e). Samples may vary from the valid 
* technical specifications of the product series. They are therefore not intended or fit for resale to third 
* parties or for use in end products. Their sole purpose is internal client testing. The testing of an 
* engineering sample may in no way replace the testing of a product series. Bosch Sensortec 
* assumes no liability for the use of engineering samples. By accepting the engineering samples, the 
* Purchaser agrees to indemnify Bosch Sensortec from all claims arising from the use of engineering 
* samples.
*
* Special:
* This Work and any related information (hereinafter called "Information") is provided free of charge 
* for the sole purpose to support your application work. The Woek and Information is subject to the 
* following terms and conditions: 
*
* The Work is specifically designed for the exclusive use for Bosch Sensortec products by 
* personnel who have special experience and training. Do not use this Work or Derivative Works if you do not have the 
* proper experience or training. Do not use this Work or Derivative Works fot other products than Bosch Sensortec products.  
*
* The Information provided is believed to be accurate and reliable. Bosch Sensortec assumes no 
* responsibility for the consequences of use of such Information nor for any infringement of patents or 
* other rights of third parties which may result from its use. No license is granted by implication or 
* otherwise under any patent or patent rights of Bosch. Specifications mentioned in the Information are 
* subject to change without notice.
*
*/

#include <stdint.h>
#include "protocol.h"
#include "bmp085_util.h"

/* See BMP085 datasheet for reference */
void calculate_bmp085_values(ws_sensor_bmp085_t *bmp_data, int32_t *t, int32_t *p) {
	int32_t b3;
	uint32_t b4;
	int32_t b5;
	int32_t b6;
	uint32_t b7;
	int32_t x1;
	int32_t x2;
	int32_t x3;

	/* Calculate true temperature */
	x1 = (((long) bmp_data->temperature - (long) bmp_data->ac6) * (long) bmp_data->ac5) >> 15;
	x2 = ((long) bmp_data->mc << 11) / (x1 + bmp_data->md);
	b5 = x1 + x2;
	*t = ((b5 + 8) >> 4);

	/* Calculate true pressure */
	b6 = b5 - 4000;
	//*****calculate B3************
	x1 = (b6*b6) >> 12;	 	 
	x1 *= bmp_data->b2;
	x1 >>=11;

	x2 = (bmp_data->ac2*b6);
	x2 >>=11;

	x3 = x1 +x2;

	b3 = (((((long)bmp_data->ac1 )*4 + x3) <<bmp_data->oversampling) + 2) >> 2;

	//*****calculate B4************
	x1 = (bmp_data->ac3* b6) >> 13;
	x2 = (bmp_data->b1 * ((b6*b6) >> 12) ) >> 16;
	x3 = ((x1 + x2) + 2) >> 2;
	b4 = (bmp_data->ac4 * (unsigned long) (x3 + 32768)) >> 15;

	b7 = ((unsigned long)(bmp_data->pressure - b3) * (50000>>bmp_data->oversampling));   
	if (b7 < 0x80000000) {
		*p = (b7 << 1) / b4;
	}
	else { 
		*p = (b7 / b4) << 1;
	}

	x1 = *p >> 8;
	x1 *= x1;
	x1 = (x1 * SMD500_PARAM_MG) >> 16;
	x2 = (*p * SMD500_PARAM_MH) >> 16;
	*p += (x1 + x2 + SMD500_PARAM_MI) >> 4;	// pressure in Pa

	/* For some reason BMP085 seems to give 1600 pascals to high values. */
	/* NOTE: This should probably be sensor dependent.                   */
	*p += 1600;
}
