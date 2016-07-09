#ifndef ADC_H
#define ADC_H

typedef unsigned char uchar;
typedef unsigned int uint;

#define ADC_CS 0
#define ADC_CLK 4
#define ADC_DIO 2

void adc_init();
uchar get_ADC_Result();

#endif