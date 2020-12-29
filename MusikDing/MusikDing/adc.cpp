/*
ADCB CH0
ADCA CH0

DMA CH1
DMA CH2

Links: ADCA - channel2
Rechts: ADBC - channel1

TCC0
*/

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "ffft.h"
#include "global.h"
#include "adc.h"

int16_t capture_ch1[FFT_N], capture_ch2[FFT_N];
volatile uint8_t adc_cycle_ch1, adc_cycle_ch2;

void adc_init()
{
	// ADC
	ADCB.REFCTRL = ADC_REFSEL_AREFB_gc;
	ADCB.PRESCALER = ADC_PRESCALER_DIV8_gc;
	ADCB.CTRLB = ADC_RESOLUTION_12BIT_gc;
	ADCB.CTRLA = ADC_ENABLE_bm;
	
	ADCA.REFCTRL = ADC_REFSEL_AREFB_gc;
	ADCA.PRESCALER = ADC_PRESCALER_DIV8_gc;
	ADCA.CTRLB = ADC_RESOLUTION_12BIT_gc;
	ADCA.CTRLA = ADC_ENABLE_bm;
	
	// Channel
	ADCB.CH0.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;
	ADCB.CH0.MUXCTRL =  ADC_CH_MUXPOS_PIN1_gc;
//	ADCB.CTRLA |= ADC_FLUSH_bm;
	ADCB.CTRLB |= ADC_FREERUN_bm;
	ADCB.CH0.CTRL |= ADC_CH_START_bm;
	
	ADCA.CH0.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;
	ADCA.CH0.MUXCTRL =  ADC_CH_MUXPOS_PIN7_gc;
//	ADCA.CTRLA |= ADC_FLUSH_bm;
	ADCA.CTRLB |= ADC_FREERUN_bm;
	ADCA.CH0.CTRL |= ADC_CH_START_bm;
	
	// Timer - samplerate 32khz
	TCC0.CTRLA = TC_CLKSEL_DIV1_gc;
	TCC0.INTCTRLA = TC_OVFINTLVL_MED_gc;
	TCC0.PER = 999;
}

void adc_startSampling()
{	
    DMA.CH1.CTRLA &= ~DMA_CH_ENABLE_bm;
	DMA.CH2.CTRLA &= ~DMA_CH_ENABLE_bm;
   
    while (DMA.CH1.CTRLA & DMA_CH_ENABLE_bm)
		;
    while (DMA.CH2.CTRLA & DMA_CH_ENABLE_bm)
		;
		
	DMA.CH1.CTRLA |= DMA_CH_RESET_bm;
	DMA.CH2.CTRLA |= DMA_CH_RESET_bm;
	
	while(DMA.CH1.CTRLA & DMA_CH_RESET_bm)
		;
	while(DMA.CH2.CTRLA & DMA_CH_RESET_bm)
		;

	DMA.CH1.ADDRCTRL = DMA_CH_SRCRELOAD_BURST_gc | DMA_CH_SRCDIR_INC_gc | DMA_CH_DESTRELOAD_NONE_gc | DMA_CH_DESTDIR_INC_gc;
	DMA.CH1.TRIGSRC = DMA_CH_TRIGSRC_TCC0_OVF_gc; //DMA_CH_TRIGSRC_ADCB_CH0_gc;
	DMA.CH1.CTRLA = DMA_CH_SINGLE_bm | DMA_CH_BURSTLEN_2BYTE_gc | DMA_CH_REPEAT_bm;
	DMA.CH1.TRFCNT = 2;
	DMA.CH1.REPCNT = 128;
	DMA.CH1.SRCADDR0 = ((uint16_t)&ADCB.CH0.RES) & 0xFF;
	DMA.CH1.SRCADDR1 = ((uint16_t)&ADCB.CH0.RES >> 8) & 0xFF;
	DMA.CH1.SRCADDR2 = 0;
	DMA.CH1.DESTADDR0 = ((uint16_t)capture_ch1) & 0xFF;
	DMA.CH1.DESTADDR1 = ((uint16_t)capture_ch1 >> 8) & 0xFF;
	DMA.CH1.DESTADDR2 = 0;
	DMA.CH1.CTRLA |= DMA_CH_ENABLE_bm;
	
	DMA.CH2.ADDRCTRL = DMA_CH_SRCRELOAD_BURST_gc | DMA_CH_SRCDIR_INC_gc | DMA_CH_DESTRELOAD_NONE_gc | DMA_CH_DESTDIR_INC_gc;
	DMA.CH2.TRIGSRC = DMA_CH_TRIGSRC_TCC0_OVF_gc; //DMA_CH_TRIGSRC_ADCA_CH0_gc;
	DMA.CH2.CTRLA = DMA_CH_SINGLE_bm | DMA_CH_BURSTLEN_2BYTE_gc | DMA_CH_REPEAT_bm;
	DMA.CH2.TRFCNT = 2;
	DMA.CH2.REPCNT = 128;
	DMA.CH2.SRCADDR0 = ((uint16_t)&ADCA.CH0.RES) & 0xFF;
	DMA.CH2.SRCADDR1 = ((uint16_t)&ADCA.CH0.RES >> 8) & 0xFF;
	DMA.CH2.SRCADDR2 = 0;
	DMA.CH2.DESTADDR0 = ((uint16_t)capture_ch2) & 0xFF;
	DMA.CH2.DESTADDR1 = ((uint16_t)capture_ch2 >> 8) & 0xFF;
	DMA.CH2.DESTADDR2 = 0;
	DMA.CH2.CTRLA |= DMA_CH_ENABLE_bm;
	
	adc_cycle_ch1 = 0;
	adc_cycle_ch2 = 0;
	adc_state = ADC_STATE_SAMPLING;
}

void adc_check()
{
	if(!(DMA.CH1.CTRLB & (DMA_CH_CHBUSY_bm | DMA_CH_CHPEND_bm)) && (DMA.INTFLAGS & DMA_CH1TRNIF_bm))
	{
		adc_cycle_ch1++;
	}
	
	if(!(DMA.CH2.CTRLB & (DMA_CH_CHBUSY_bm | DMA_CH_CHPEND_bm)) && (DMA.INTFLAGS & DMA_CH2TRNIF_bm))
	{
		adc_cycle_ch2++;
	}

	if(adc_cycle_ch1 >= 1 && adc_cycle_ch2 >= 1)
	{
		adc_state = ADC_STATE_SAMPLING_DONE;
	}
}

ISR(TCC0_OVF_vect)
{
// 	ADCA.CH0.CTRL |= ADC_CH_START_bm;
// 	ADCB.CH0.CTRL |= ADC_CH_START_bm;
}