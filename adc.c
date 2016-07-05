#include <wiringPi.h>
#include "adc.h"

/**
Init the adc converter
**/
void adc_init()
{
pinMode(ADC_CS, OUTPUT);
pinMode(ADC_CLK, OUTPUT);
}

/**
* Lecteur de valuer sur l'adc.
**/
uchar get_ADC_Result()
{
pinMode(ADC_DIO, OUTPUT);

uchar i;
uchar dat1=0, dat2=0;

digitalWrite(ADC_CS, 0);
digitalWrite(ADC_CLK,0);
digitalWrite(ADC_DIO,1); delayMicroseconds(2);
digitalWrite(ADC_CLK,1); delayMicroseconds(2);

digitalWrite(ADC_CLK,0);
digitalWrite(ADC_DIO,1); delayMicroseconds(2);
digitalWrite(ADC_CLK,1); delayMicroseconds(2);

digitalWrite(ADC_CLK,0);
digitalWrite(ADC_DIO,0); delayMicroseconds(2);
digitalWrite(ADC_CLK,1);
digitalWrite(ADC_DIO,1); delayMicroseconds(2);
digitalWrite(ADC_CLK,0);
digitalWrite(ADC_DIO,1); delayMicroseconds(2);

for(i=0;i<8;i++)
{
digitalWrite(ADC_CLK,1); delayMicroseconds(2);
digitalWrite(ADC_CLK,0); delayMicroseconds(2);

pinMode(ADC_DIO, INPUT);
dat1=dat1<<1 | digitalRead(ADC_DIO);
}

for(i=0;i<8;i++)
{
dat2 = dat2 | ((uchar)(digitalRead(ADC_DIO))<<i);
digitalWrite(ADC_CLK,1); delayMicroseconds(2);
digitalWrite(ADC_CLK,0); delayMicroseconds(2);
}

digitalWrite(ADC_CS,1);

return(dat1==dat2) ? dat1 : 0;
}
