#ifndef __CFFT_H__
#define __CFFT_H__

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "ffft.h"

#define FFT_STATE_IDLE			0
#define FFT_STATE_PREP1			1
#define FFT_STATE_CALC11		2
#define FFT_STATE_CALC12		3
#define FFT_STATE_PREP2			4
#define FFT_STATE_CALC21		5
#define FFT_STATE_CALC22		6

typedef struct
{
	uint16_t spectrum[FFT_N / 2];
	uint16_t adc_min, adc_max;
} fft_result_t;

class CFFT
{
//variables
public:
protected:
private:
	complex_t fft_buff[FFT_N];
	int16_t temp_buff[FFT_N];
	fft_result_t ch1, ch2;
	uint8_t fft_state;

//functions
public:
	CFFT();
	~CFFT();
	
	uint8_t doStep();
	void doFFT();
	
	fft_result_t *getLeft();
	fft_result_t *getRight();
	
protected:
private:
	CFFT( const CFFT &c );
	CFFT& operator=( const CFFT &c );

}; //CFFT

#endif //__CFFT_H__
