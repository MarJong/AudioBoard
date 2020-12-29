/*
USART C1 & DMA channel 0
PC5: CLK
PC6: RX
PC7: TX
*/
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

#include "CWS2812.h"

// default constructor
CWS2812::CWS2812()
{
	// USART 1 PORT C
	// ports setzen, TX invertieren, Master SPI
	PORTC.DIRSET = PIN5_bm | PIN7_bm;
	PORTC.DIRCLR = PIN6_bm;
	
	PORTC.PIN7CTRL = PORT_INVEN_bm;
	
	// testen: 3/0 ca. 1µs; 4/0 ca. 1.25µs
	USARTC1.CTRLA = 0;
	USARTC1.CTRLB = USART_TXEN_bm;
	USARTC1.CTRLC = USART_CMODE_MSPI_gc;
	USARTC1.BAUDCTRLA = 4;					// 16MHz: 1
	USARTC1.BAUDCTRLB = (0 << 4);			// 16MHz: (-3 << 4);
	
	// DMA channel 0
	DMA.CH0.ADDRCTRL = DMA_CH_SRCRELOAD_TRANSACTION_gc | DMA_CH_SRCDIR_INC_gc | DMA_CH_DESTRELOAD_NONE_gc | DMA_CH_DESTDIR_FIXED_gc;
	DMA.CH0.TRIGSRC = DMA_CH_TRIGSRC_USARTC1_DRE_gc;
	DMA.CH0.CTRLA = DMA_CH_SINGLE_bm | DMA_CH_BURSTLEN_1BYTE_gc;
	
	DMA.CH0.DESTADDR0 = ((uint16_t)&USARTC1.DATA) & 0xFF;
	DMA.CH0.DESTADDR1 = ((uint16_t)&USARTC1.DATA >> 8) & 0xFF;
	DMA.CH0.DESTADDR2 = 0;
	
	memset(this->data, WS2812_LOW, WS2812_DATASIZE);
} //CWS2812

// default destructor
CWS2812::~CWS2812()
{
} //~CWS2812

void CWS2812::transfer()
{
	DMA.CH0.SRCADDR0 = ((uint16_t)this->data) & 0xFF;
	DMA.CH0.SRCADDR1 = ((uint16_t)this->data >> 8) & 0xFF;
	DMA.CH0.SRCADDR2 = 0;
	
	DMA.CH0.TRFCNT = WS2812_DATASIZE;
	DMA.CH0.CTRLA |= DMA_CH_ENABLE_bm;
}

void CWS2812::input(uint8_t *led_data, uint16_t len)
{
	uint8_t i, b;
	uint16_t dpos;
	
	dpos = 0;
	
	for(i = 0; i < len; i++)
	{
		for(b = 0x80; b; b >>= 1)
		{	
			if(led_data[i] & b)
			{
				this->data[dpos] = WS2812_HIGH;
			}
			else
			{
				this->data[dpos] = WS2812_LOW;
			}
			
			dpos++;
		}
	}
}

uint8_t CWS2812::isBusy()
{	
	if((DMA.CH0.CTRLB & DMA_CH_CHBUSY_bm) || (DMA.CH0.CTRLA & DMA_CH_ENABLE_bm))
	{
		return 1;
	}
	
	return 0;
}