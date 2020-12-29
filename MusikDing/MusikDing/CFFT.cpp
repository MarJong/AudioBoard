/*
ergebnis:
adc_min/adc_max: min./max. analoger wert
spectrum: 16kHz in 64 buckets -> 250Hz/bucket
*/
#include <avr/io.h>
#include <string.h>
#include <avr/pgmspace.h>

#include "CFFT.h"
#include "adc.h"

// werte für reverse a-weighting
const float PROGMEM fft_a_weight_tab[FFT_N / 2] = {
0.736858111,
1.107561091,
1.629120946,
1.869752889,
2,
2.515894266,
3.017939515,
3.506909136,
3.983207977,
4.446900946,
4.897828209,
5.335701756,
5.760173286,
6.170879716,
6.567473215,
6.949640771,
7.317116649,
7.669689914,
8.00720846,
8.329580529,
8.636774384,
8.92881663,
9.205789548,
9.467827701,
9.715114057,
9.947875769,
10.16637978,
10.37092835,
10.5618546,
10.73951813,
10.90430084,
11.05660286,
11.19683878,
11.32543413,
11.44282205,
11.54944032,
11.64572862,
11.73212611,
11.80906918,
11.87698961,
11.9363128,
11.9874564,
12.03082903,
12.06682925,
12.09584475,
12.11825162,
12.13441388,
12.14468304,
12.14939787,
12.14888425,
12.14345508,
12.13341031,
12.11903705,
12.10060974,
12.07839032,
12.05262852,
12.02356213,
11.99141732,
11.95640901,
11.9187412,
11.87860734,
11.83619076,
11.79166499,
11.74519419,
};

// default constructor
CFFT::CFFT()
{
	this->fft_state = FFT_STATE_IDLE;
} //CFFT

// default destructor
CFFT::~CFFT()
{
} //~CFFT

void CFFT::doFFT()
{
	this->fft_state = FFT_STATE_PREP1;
}

// ret = FFT_STATE_IDLE: fertig; sonst: nicht fertig
uint8_t CFFT::doStep()
{
	uint16_t i;
			
	switch(this->fft_state)
	{
	case FFT_STATE_PREP1:
		this->ch1.adc_max = 0;
		this->ch1.adc_min = 0xFFFF;
		
		for(i = 0; i < FFT_N; i++)
		{
			if((uint16_t)capture_ch1[i] > this->ch1.adc_max)
			{
				this->ch1.adc_max = capture_ch1[i];
			}
			if((uint16_t)capture_ch1[i] < this->ch1.adc_min)
			{
				this->ch1.adc_min = capture_ch1[i];
			}
			
			this->temp_buff[i] = (capture_ch1[i] << 4) - 0x7FFF;
		}
		fft_input(this->temp_buff, this->fft_buff);
	
		this->fft_state = FFT_STATE_CALC11;
		break;
	
	case FFT_STATE_CALC11:
		fft_execute(this->fft_buff);	
		
		this->fft_state = FFT_STATE_CALC12;
		break;
	
	case FFT_STATE_CALC12:
		fft_output(this->fft_buff, this->ch1.spectrum);
		
		// reverse a-weighting
		for(i = 0; i < FFT_N / 2; i++)
		{
			this->ch1.spectrum[i] = (this->ch1.spectrum[i]) * pgm_read_float(&fft_a_weight_tab[i]);
		}	
	
		this->fft_state = FFT_STATE_PREP2;
		break;
	
	case FFT_STATE_PREP2:
		this->ch2.adc_max = 0;
		this->ch2.adc_min = 0xFFFF;
		
		for(i = 0; i < FFT_N; i++)
		{
			if((uint16_t)capture_ch2[i] > this->ch2.adc_max)
			{
				this->ch2.adc_max = capture_ch2[i];
			}
			if((uint16_t)capture_ch2[i] < this->ch2.adc_min)
			{
				this->ch2.adc_min = capture_ch2[i];
			}
			
			this->temp_buff[i] = (capture_ch2[i] << 4) - 0x7FFF;
		}
		fft_input(this->temp_buff, this->fft_buff);	
	
		this->fft_state = FFT_STATE_CALC21;
		break;
	
	case FFT_STATE_CALC21:
		fft_execute(this->fft_buff);
		
		this->fft_state = FFT_STATE_CALC22;
		break;
	
	case FFT_STATE_CALC22:
		fft_output(this->fft_buff, this->ch2.spectrum);
		
		// reverse a-weighting
		for(i = 0; i < FFT_N / 2; i++)
		{
			this->ch2.spectrum[i] = (this->ch2.spectrum[i]) * pgm_read_float(&fft_a_weight_tab[i]);
		}
		
		this->fft_state = FFT_STATE_IDLE;
		break;
		
	default:
		this->fft_state = FFT_STATE_IDLE;
		break;
	}
	
	return this->fft_state;
}

fft_result_t *CFFT::getLeft()
{
	return &this->ch2;
}

fft_result_t *CFFT::getRight()
{
	return &this->ch1;
}