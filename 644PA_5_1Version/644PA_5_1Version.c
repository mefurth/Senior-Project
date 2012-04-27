#ifndef F_CPU
#define F_CPU 10000000UL
#endif
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>
#include "types.h"
#include "defs.h"
#include "lcd.h"
/* Code for single pin addressing */


typedef struct
{
  unsigned char bit0:1;
  unsigned char bit1:1;
  unsigned char bit2:1;
  unsigned char bit3:1;
  unsigned char bit4:1;
  unsigned char bit5:1;
  unsigned char bit6:1;
  unsigned char bit7:1;

}io_reg;
 
#define D0      ((volatile io_reg*)_SFR_MEM_ADDR(PORTA))->bit4
#define D1      ((volatile io_reg*)_SFR_MEM_ADDR(PORTA))->bit5
#define LIGHT      ((volatile io_reg*)_SFR_MEM_ADDR(PORTA))->bit2
 
/* Code for 7 seg display */


static unsigned  char SEVEN_SEG[] = {
0x3F,
0x06,
0x5B,
0x4F,
0x66,
0x6D,
0x7D,
0x07,
0x7F,
0x6F,
0x77,
0x7C,
0x39,
0x5E,
0x79,
0x71};
int speed_limit()
 {
 uint16_t Condition;
 uint16_t Condition2;
 uint16_t adc_result0;
 Condition=250;
 Condition2=30;
 adc_result0 = adc_read(0);      // read adc value at PA0
	 int speed;
	 if ((adc_result0 <= Condition2))
	 {
		 speed = 55;
	 }
	else if((Condition2 < adc_result0)&&(adc_result0<=Condition) ) {
 speed = 65; }// this is where we set our speed limit
 else {
	 speed =95;}
  return speed; // returns value stored in speed variable 	
 }	
void main()
{
	
	//Initialize LCD module
	InitLCD();
	//Clear the screen
	LCDClear();
	//Simple string printing
	LCDWriteString("T:");
	LCDWriteStringXY(0,1,"A:");	
	LCDWriteStringXY(8,1,"L:");
	LCDWriteStringXY(7,0,"P:");
	//Print some numbers
	
unsigned char num = 0x01;
int i; 
int speed_limit(); // calling the speed_limit function
DDRB |= 0xFF;
DDRA |= 0xFE; 

	long temperature = 0;
	long pressure = 0;
	long alt = 0;
	long weatherDiff =0;
	long bmp085ReadTemp();
	long bmp085ReadPressure();
	long ut = 0;
	long up = 0;

	//long altitude = 0;
	//double temp = 0;
	
	ioinit();
	i2cInit();
	delay_ms(100);
	
	BMP085_Calibration();

	uint16_t adc_result0; 
    char int_buffer[10];
    char altitudes[10];
	char pressures[10];
	char temperatures[10];

    // initialize adc and lcd
    adc_init();

while (1) {
  D0=0;
  D1=0;
  bmp085Convert(&temperature, &pressure, &alt, &weatherDiff);
   ltoa(weatherDiff, altitudes, 10);
   ltoa(pressure, pressures, 10);
   itoa(temperature, temperatures, 10);
 for (i=0; i<50; i++) {
  D0=0;
  D1=0;
  num = speed_limit();
adc_result0 = adc_read(0);      // read adc value at PA0
 itoa(adc_result0, int_buffer, 10);
LCDWriteStringXY(2,0,temperatures);
LCDWriteStringXY(9,0,pressures);
LCDWriteStringXY(2,1,altitudes);
 LCDWriteStringXY(10,1,int_buffer);
  PORTB = SEVEN_SEG[num%10];
  D0=1;
  D1=0;
  _delay_us(500);
  D0=0;
  D1=0;
  PORTB = SEVEN_SEG[num/10];
  D0=0;
  D1=1;
  _delay_us(500);
  } 
 }
}
