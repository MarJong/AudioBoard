#ifndef ADC_H_
#define ADC_H_

#include <avr/pgmspace.h>
#include "ffft.h"

#define ADC_STATE_IDLE				0
#define ADC_STATE_SAMPLING			1
#define ADC_STATE_SAMPLING_DONE		2
#define ADC_STATE_WAIT				3

extern int16_t capture_ch1[FFT_N], capture_ch2[FFT_N];

void adc_init();
void adc_startSampling();
void adc_check();


#endif /* ADC_H_ */