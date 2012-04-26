/*
 * SHT1x.c
 *
 * Created: 4/13/2012 2:03:24 PM
 *  Author: Michael
 */ 

#include <math.h>
#include <stdio.h>
#include <compat/ina90.h>
#include <avr/io.h>
#include <avr/sfr_defs.h>


typedef union
{ 
unsigned int i;
float f;
} value;
enum {TEMP,HUMI};

#define _NOP() do { __asm__ __volatile__ ("nop"); } while (0)

#define read_status_reg       0x07     //*the following 5 vals are obtained 
#define write_status_reg      0x06    //*from the datasheet, sensor
      								 //*commands
#define measure_temp          0x03
#define measure_humidity      0x05
#define reset                 0x1e
#define noACK 0
#define ACK 1
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
#define DATA      ((volatile io_reg*)_SFR_MEM_ADDR(PORTC))->bit2
#define SCK      ((volatile io_reg*)_SFR_MEM_ADDR(PORTC))->bit3

void SPI_MasterInit(void)
{
/* Set MOSI and SCK output, all others input */
DDRB |= (1<<PB5)|(1<PB7);
/* Enable SPI, Master, set clock rate fck/16 */
SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
}
void SPI_MasterTransmit(char cData)
{
/* Start transmission */
SPDR = cData;
/* Wait for transmission complete */
while(!(SPSR & (1<<SPIF)));
}

char s_write_byte(unsigned char value)
{
//PORTA= (0<<PA0);DDRA= (1<<PA0);PORTB= (1<<PB7);DDRB= (1<<DDB7);
	
unsigned char i,error=0;
for (i=0x80;i>0;i/=2); //shift bit for masking
 if (i & value)
DATA == 1; //masking value with i , write to SENSI-BUS
else
{ 
DATA == 0;
_NOP(); //observe setup time
SCK == 1; //clk for SENSI-BUS
_NOP();_NOP();_NOP();
SCK == 0;
_NOP(); //observe hold time
}
DATA == 1; //release DATA-line
_NOP(); //observe setup time
SCK == 1; //clk #9 for ack
error=DATA; //check ack (DATA will be pulled down by SHT11)
SCK == 0;
return error; //error=1 in case of no acknowledge	
}

char s_read_byte(unsigned char ack)
{
	unsigned char i,val=0;
DATA == 1;                                  //release DATA-line
for (i=0x80;i>0;i/=2)                         //shift bit for masking
 {
SCK== 0;                                //clk for SENSI-BUS
if (DATA) val=(val | i);                      //read bit
SCK == 0;
}
DATA == !ACK;                             //in case of "ack==1" pull down DATA-Line
_NOP();                             //observe setup time
SCK == 1;                                  //clk #9 for ack
_NOP();_NOP();_NOP();                 //pulswith approx. 5 us
SCK == 0;
_NOP();                                  //observe hold time
DATA == 1;                                      //release DATA-line
return val;
}

void s_transstart(void)
{
DATA == 1;
SCK == 0; //Initial state
 _NOP();
SCK == 1;
 _NOP();
DATA == 0;
 _NOP();
SCK == 0;
 _NOP(); _NOP(); _NOP();
SCK == 1;
 _NOP();
DATA == 1;
 _NOP();
SCK == 0;	
}

void s_connectionreset(void)
{
	unsigned char i;
DATA == 1;
SCK == 0; //Initial state
for(i=0;i<9;i++) //9 SCK cycles
{
SCK == 1;
SCK == 0;
}
s_transstart(); //transmission start
}

char s_read_statusreg(unsigned char *p_value, unsigned char *p_checksum)
{
unsigned char error=0;  // reads the status register with checksum (8-bit)
s_transstart(); //transmission start
error=s_write_byte(read_status_reg); //send command to sensor
*p_value=s_read_byte(ACK); //read status register (8-bit)
*p_checksum=s_read_byte(noACK); //read checksum (8-bit)
return error;
}

char s_write_statusreg(unsigned char *p_value)
{
unsigned char error=0;  // writes the status register with checksum (8-bit)
s_transstart(); //transmission start
error+=s_write_byte(write_status_reg);//send command to sensor
error+=s_write_byte(*p_value); //send value of status register
return error;
}

char s_measure(unsigned char *p_value, unsigned char *p_checksum, unsigned char mode)
{
unsigned char error=0;  // makes a measurement (humidity/temperature) with checksum
unsigned int i;
s_transstart(); //transmission start
switch(mode)
{
 //send command to sensor
case TEMP : error+=s_write_byte (measure_temp); break;
case HUMI : error+=s_write_byte(measure_humidity); break;
default : break;
}
for (i=0;i<65535;i++)
if(DATA == 0)
break; //wait until sensor has finished the
//measurement
if(DATA) error+=1; // or timeout (~2 sec.) is reached
*(p_value) =s_read_byte(ACK); //read the first byte (MSB)
*(p_value+1)=s_read_byte(ACK); //read the second byte (LSB)
*p_checksum =s_read_byte(noACK); //read checksum
return error;
}

void calcth(float fhumidity, float ftemperature)
{
const float c1=-2.0468;  //* All of these constants are defined by how many bit
//*is the input readin, 8 or 12
const float c2=0.0367;
const float c3=-0.0000015955;
const float t1=0.01;
const float t2=0.00008;
const float d1=-40.1;  //*Defined by supply voltage
const float d2=0.01;  //*Defined by temp read in of 12 or 14 bits
float rh;   //*Input humidity read in
float t;  //*input temp read in
float rh_linear;  //*Linear relative humitidy
float rh_true;  //*Final humidity value after calculations
float t_C;  //*Final temp value after calculations
rh=fhumidity;
t=ftemperature;
t_C=d1+d2*t;
rh_linear=c1+c2*rh+c3*rh*rh;
rh_true=(t_C-25)*(t1+t2*rh)+rh_linear;
ftemperature=t_C;
fhumidity=rh_true;
}


int SHT1x(void)
{
unsigned int humi_val, temp_val;
unsigned char error, checksum;
float rh;
float t;
    while(1)
    {
     error=0;
error+=s_measure((unsigned char*) &humi_val, &checksum, HUMI);
error+=s_measure((unsigned char*) &temp_val, &checksum, TEMP);
if(error!=0)
s_connectionreset();
else
{
rh=humi_val;
t=humi_val;
void calcth(float fhumidity, float ftemperature);
}
   
    
}
}