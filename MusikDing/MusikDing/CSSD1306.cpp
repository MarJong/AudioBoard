#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdarg.h>

#include "CSSD1306.h"
#include "font.h"

#define IIC_ADDR		0x78

#define LCD_X					128
#define LCD_Y					64
#define MAX_CHAR_IN_LINE		(LCD_X / FONT_WIDTH)

// default constructor
CSSD1306::CSSD1306()
{
	TWIE.MASTER.BAUD = (F_CPU / (2 * 400000UL)) - 5;	// 400.000 Hz
	TWIE.MASTER.CTRLA = TWI_MASTER_ENABLE_bm;
//	TWIE.MASTER.CTRLB = TWI_MASTER_TIMEOUT_200US_gc;
	TWIE.MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
} //CSSD1306

// default destructor
CSSD1306::~CSSD1306()
{
} //~CSSD1306

void CSSD1306::StartTransfer(uint8_t rw)
{
	TWIE.MASTER.ADDR = IIC_ADDR | rw;
	while(!(TWIE.MASTER.STATUS & TWI_MASTER_WIF_bm))
		;
}

void CSSD1306::EndTransfer()
{
	TWIE.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
}

void CSSD1306::Send(uint8_t c)
{
	TWIE.MASTER.DATA = c;
	while(!(TWIE.MASTER.STATUS & TWI_MASTER_WIF_bm))
		;	
}

void CSSD1306::SendCommand(uint8_t cmd)
{
	this->StartTransfer();
	
	this->Send(0);		// CMD Mode
	this->Send(cmd);
	
	this->EndTransfer();
}

void CSSD1306::SendData(uint8_t data)
{
	this->StartTransfer();
	
	this->Send(0x40);		// Data Mode
	this->Send(data);
	
	this->EndTransfer();	
}

void CSSD1306::SendData(uint8_t *data, uint16_t data_len)
{
	uint16_t i;
	
	this->StartTransfer();
	
	this->Send(0x40);		// Data Mode
	
	for(i = 0; i < data_len; i++)
	{
		this->Send(data[i]);
	}
	
	this->EndTransfer();	
}

void CSSD1306::init()
{	
	uint8_t i;
	
	this->SendCommand(0xAE);	// Off	
	this->SendCommand(0xD5);	// set clock divider
	this->SendCommand(0x80);

	this->SendCommand(0xA8);	// set multiplex
	this->SendCommand(0x3F);
	
	this->SendCommand(0xD3);	// set offset
	this->SendCommand(0);
  
	this->SendCommand(0x40);	// set start line 0
	
	this->SendCommand(0x8D);	// charge pump
	this->SendCommand(0x14);
	
	this->SendCommand(0x20);	// memory mode
	this->SendCommand(2);		// Page-Mode
	this->SendCommand(0x00);
	this->SendCommand(0x10);
	
	this->SendCommand(0xA0);	// segment re-map
	
	this->SendCommand(0xC0);	// scan
	
	this->SendCommand(0xDA);	// com pins
	this->SendCommand(0x12);
	
	this->SendCommand(0x81);	// Contrast
	this->SendCommand(0xCF);
	
	this->SendCommand(0xD9);	// pre-charge
	this->SendCommand(0xF1);
	
	this->SendCommand(0xDB);	// comdetect
	this->SendCommand(0x40);
	
	this->SendCommand(0xA4);	// all on
	
	this->SendCommand(0xA6);	// normal
	
	this->SendCommand(0xAF);	// on
			
	this->update_flags = 0;
	this->data_pos = 0;
	
	memset(this->data, 0, 1024);
	
	for(i = 0; i < 8; i++)
	{
		this->SendCommand(0xB0 | i);
		this->SendData(&this->data[128], 128);		
	}
}

void CSSD1306::update()
{
	uint8_t i;
	
	if(!this->update_flags && !this->data_pos)
	{
		return;
	}
	

	for(i = 0; i < 8; i++)
	{
		if(this->update_flags & (1 << i))
		{	
			this->SendCommand(0xB0 | i);
			this->SendData(&this->data[128 * i], 128);

			this->update_flags &= ~(1 << i);
			
			return;
		}
	}
}


void CSSD1306::clear()
{
	memset(this->data, 0, 1024);
}

void CSSD1306::print(uint8_t line, uint8_t style, const char *str, ...)
{
	va_list vl;
	char temp[MAX_CHAR_IN_LINE + 1];
	uint8_t tpos, start, len, i;
	uint16_t lpos;
	
	if(line > 7)
	{
		return;
	}
	
	va_start(vl, str);
	len = vsnprintf(temp, MAX_CHAR_IN_LINE + 1, str, vl);
	va_end(vl);
	
	if(!(style & LCD_STYLE_NOCLEAR))
	{
		if(style & LCD_STYLE_INVERT_L)
		{
			memset(&this->data[128 * line], 0xFF, 128);
		}
		else
		{
			memset(&this->data[128 * line], 0, 128);
		}
	}
	
	start = 0;
	if((style & LCD_STYLE_CENTER) == LCD_STYLE_CENTER)
	{
		start = 64 - ((len * FONT_WIDTH) / 2);
	}
	else if(style & LCD_STYLE_RIGHT)
	{
		start = 127 - (len * FONT_WIDTH);
	}
	
	lpos = start + (line * 128);
	
	for(tpos = 0; tpos < len; tpos++)
	{
		for(i = 0; i < FONT_WIDTH; i++)
		{
			if(style & LCD_STYLE_INVERT_L || style & LCD_STYLE_INVERT_T)
			{
				this->data[lpos] = ~pgm_read_byte(&font[(uint8_t)temp[tpos]][i]);
			}
			else
			{
				this->data[lpos] = pgm_read_byte(&font[(uint8_t)temp[tpos]][i]);
			}
			
			lpos++;
		}
	}
	
	this->update_flags |= (1 << line);
}

void CSSD1306::graph(uint8_t style, uint16_t *data, uint8_t num, uint16_t max)
{
	uint8_t i, temp, a, b;
	float fac;
	
	if(num > LCD_X)
	{
		num = LCD_X;
	}
	
	fac = ((float)max / (float)(LCD_Y - 1)) + 1.0f;
	
	memset(&this->data[0], 0, 1024);
	
	for(i = 0; i < num; i++)
	{
		if(data[i] >= max)
		{
			temp = (max - 1) / fac;
		}
		else
		{
			temp = data[i] / fac;
		}
			
		a = 7 - (temp / 8);
		b = 7 - (temp % 8);
		
		this->data[(i + (a * 128))] = (1 << b);
		
		if(style & LCD_GSTYLE_FILLED)
		{
			uint8_t j;
			
			for(j = b; j < 8; j++)
			{
				this->data[(i + (a * 128))] |= (1 << j);
			}
			
			for(j = a + 1; j < 8; j++)
			{
				this->data[(i + (j * 128))] |= 0xff;
			}
		}
	}
	
	this->update_flags = 0xFF;
}

void CSSD1306::graph2(uint16_t *data1, uint16_t *data2, uint8_t num, uint16_t max, uint8_t offset)
{
	uint8_t i, temp, a, b, lcd_pos, j, k;
	float fac;
	
	if(num > LCD_X)
	{
		num = LCD_X;
	}
	
	fac = ((float)max / (float)(LCD_Y - 1)) + 1.0f;
	
	memset(&this->data[0], 0, 1024);
	lcd_pos = 0;
	
	for(i = 0; i < num; i++)
	{
		if(data1[i] >= max)
		{
			temp = (max - 1) / fac;
		}
		else
		{
			temp = data1[i] / fac;
		}
		
		a = 7 - (temp / 8);
		b = 7 - (temp % 8);
		
		this->data[(lcd_pos + (a * 128))] = (1 << b);
		
		for(j = b; j < 8; j++)
		{
			this->data[(lcd_pos + (a * 128))] |= (1 << j);
		}
		
		for(j = a + 1; j < 8; j++)
		{
			this->data[(lcd_pos + (j * 128))] |= 0xff;
		}
		
		for(j = 1; j < 5; j++)
		{
			for(k = 0; k < 8; k++)
			{
				this->data[lcd_pos + j + (k * 128)] = this->data[lcd_pos + (k * 128)];
			}
		}
		lcd_pos += 6;
	}
	
	lcd_pos += offset;
	
	for(i = 0; i < num; i++)
	{
		if(data2[i] >= max)
		{
			temp = (max - 1) / fac;
		}
		else
		{
			temp = data2[i] / fac;
		}
		
		a = 7 - (temp / 8);
		b = 7 - (temp % 8);
		
		this->data[(lcd_pos + (a * 128))] = (1 << b);
		
		for(j = b; j < 8; j++)
		{
			this->data[(lcd_pos + (a * 128))] |= (1 << j);
		}
		
		for(j = a + 1; j < 8; j++)
		{
			this->data[(lcd_pos + (j * 128))] |= 0xff;
		}
		
		for(j = 1; j < 5; j++)
		{
			for(k = 0; k < 8; k++)
			{
				this->data[lcd_pos + j + (k * 128)] = this->data[lcd_pos + (k * 128)];
			}
		}
		lcd_pos += 6;
	}	
	
	this->update_flags = 0xFF;
}

uint8_t CSSD1306::isBusy()
{
	if(TWIE.MASTER.CTRLA & TWI_MASTER_WIEN_bm)
	{
		return 1;
	}
	return 0;
}