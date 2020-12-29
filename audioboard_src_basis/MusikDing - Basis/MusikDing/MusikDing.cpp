/*
TCC1 - System timer
TCC0 - Sampling

DMA0 - LED
USARTC1 - LED

DMA1 - ADC
DMA2 - ADC

ADCA
ADCB
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "global.h"
#include "adc.h"
#include "CFFT.h"
#include "animation.h"

CFFT fft;

volatile uint32_t systick;

#define FLAG_FRAME		0x01
#define FLAG_100HZ		0x02
#define FLAG_DOSAMPLE	0x04
#define FLAG_FFTDONE	0x08
uint8_t flags, adc_state;

int main(void)
{
	// auf ext. Clock umstellen und PLL aktivieren
	OSC.XOSCCTRL = OSC_FRQRANGE_12TO16_gc | OSC_XOSCSEL_XTAL_16KCLK_gc;
	OSC.CTRL |= OSC_XOSCEN_bm;
	while(!(OSC.STATUS & OSC_XOSCRDY_bm))
		;
	OSC.PLLCTRL = OSC_PLLSRC_XOSC_gc | OSC_PLLFAC2_bm;
	OSC.CTRL |= OSC_PLLEN_bm;
	while(!(OSC.STATUS & OSC_PLLRDY_bm))
		;
	CCP = CCP_IOREG_gc;
	CLK.CTRL = CLK_SCLKSEL_PLL_gc;
	OSC.CTRL &= ~OSC_RC2MEN_bm;
	
	// Systemtimer	alle 1 ms
	systick = 0;
	TCC1.CTRLA = TC_CLKSEL_DIV1_gc;
	TCC1.INTCTRLA = TC_OVFINTLVL_MED_gc;
	TCC1.PER = (F_CPU / 1000) - 1;
	
	DMA.CTRL = DMA_ENABLE_bm;

	anim_init();
	adc_init();
	
	flags = 0;
	adc_state = ADC_STATE_IDLE;
	
	PMIC.CTRL |= (PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm);
	sei();
	
	anim_start(0);
	
    while(1)
    {	
		// sampling läuft
		if(adc_state == ADC_STATE_SAMPLING)
		{
			adc_check();
		}
		
		// alle 10 ms
		if(flags & FLAG_100HZ)
		{				
			// TODO: Events die alle 10ms aufgerufen werden; z.B. Inputs abfragen
			
			flags &= ~FLAG_100HZ;
		}
		
		// neues bild berechnen
		else if(flags & FLAG_FRAME)
		{
			anim_frame();

			flags &= ~FLAG_FRAME;
		}
		
		// neue Daten einlesen
		else if((adc_state == ADC_STATE_IDLE) && (flags & FLAG_DOSAMPLE))
		{
			adc_startSampling();
			
			flags &= ~FLAG_DOSAMPLE;
		}
		
		// auswerten starten
		else if(adc_state == ADC_STATE_SAMPLING_DONE)
		{	
			fft.doFFT();
			adc_state = ADC_STATE_WAIT;
		}
		
		 // auswerten
		 else if(adc_state == ADC_STATE_WAIT)
		 {
			if(!fft.doStep())
			{
				flags |= FLAG_FFTDONE;
				adc_state = ADC_STATE_IDLE;
			}
		 }
		 
		 // fft fertig -> daten ins animationssystem übertragen
		 else if(flags & FLAG_FFTDONE)
		 {
			anim_inputData(fft.getLeft(), fft.getRight()); 
			flags &= ~FLAG_FFTDONE;
		 }
		
		// sonst anderen kram
		else
		{
			// TODO: anderer Kram mit niedriger Priorität; z.B. Daten auf ein Display ausgeben
		}
    }
}

#define TIME_FRAME		16		//33
#define TIME_100HZ		10
#define TIME_SAMPLE		100

ISR(TCC1_OVF_vect)
{
	static uint8_t cnt_frame = TIME_FRAME, cnt_100hz = TIME_100HZ;
	static uint16_t cnt_capture = TIME_SAMPLE;
	
	cnt_frame--;
	if(!cnt_frame)
	{
		flags |= FLAG_FRAME;
		cnt_frame = TIME_FRAME;
	}
	
	cnt_100hz--;
	if(!cnt_100hz)
	{
		flags |= FLAG_100HZ;
		cnt_100hz = TIME_100HZ;
	}
	
	cnt_capture--;
	if(!cnt_capture)
	{
		flags |= FLAG_DOSAMPLE;
		cnt_capture = TIME_SAMPLE;
	}
	
	systick++;
}