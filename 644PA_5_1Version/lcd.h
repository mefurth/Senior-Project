#include <avr/io.h>

#ifndef F_CPU
	#define F_CPU 12000000UL
#endif

#include <util/delay.h>

#include "myutils.h"

#ifndef _LCD_H
#define _LCD_H
/*_________________________________________________________________________________________*/

/************************************************
	LCD CONNECTIONS
*************************************************/

#define LCD_DATA C		//Port PB0-PB3 are connected to D4-D7
#define LCD_DATA_POS 4	

#define LCD_E D 		//Enable OR strobe signal
#define LCD_E_POS	PD7	//Position of enable in above port

#define LCD_RS D	
#define LCD_RS_POS 	PD6

#define LCD_RW D
#define LCD_RW_POS 	PD5


//************************************************

#define LS_BLINK 0B00000001
#define LS_ULINE 0B00000010



/***************************************************
			F U N C T I O N S
****************************************************/



void InitLCD();
void LCDWriteString(const char *msg);
void LCDWriteInt(int val,unsigned int field_length);
void LCDGotoXY(uint8_t x,uint8_t y);
//Low level
void LCDByte(uint8_t,uint8_t);
#define LCDCmd(c) (LCDByte(c,0))
#define LCDData(d) (LCDByte(d,1))

void LCDBusyLoop();





/***************************************************
			F U N C T I O N S     E N D
****************************************************/


/***************************************************
	M A C R O S
***************************************************/
#define LCDClear() LCDCmd(0b00000001)
#define LCDHome() LCDCmd(0b00000010);

#define LCDWriteStringXY(x,y,msg) {\
 LCDGotoXY(x,y);\
 LCDWriteString(msg);\
}

#define LCDWriteNumXY(x,y,val,fl) {\
 LCDGotoXY(x,y);\
 LCDWriteNum(val,fl);\
}
/***************************************************/




/*_________________________________________________________________________________________*/
#endif






