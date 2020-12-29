/*
Port E
2, 3 - Encoder
4 - Taster
*/

#include <avr/io.h>

#include "CInput.h"

#define BTN_STATE_NONE			0
#define BTN_STATE_PRESSED		1
#define BTN_STATE_HOLD			2

#define BTN_THRESHOLD			10

const int8_t enc_table[16] = {0,0,-1,0,0,0,0,1,1,0,0,0,0,-1,0,0};

// default constructor
CInput::CInput()
{
	this->btn_state = BTN_STATE_NONE;
	this->btn_cnt = 0;
	
	this->enc_delta = 0;
	this->enc_last = 0;
	
	PORTE.DIRCLR |= (PIN2_bm | PIN3_bm | PIN4_bm);
	PORTE.PIN4CTRL = PORT_OPC_PULLUP_gc;
} //CInput

// default destructor
CInput::~CInput()
{
} //~CInput

void CInput::pollBtn()
{
	switch(this->btn_state)
	{
	case BTN_STATE_NONE:
		if(!(PORTE.IN & PIN4_bm))
		{
			if(this->btn_cnt < BTN_THRESHOLD)
			{
				this->btn_cnt++;
			}
			else
			{
				this->btn_state = BTN_STATE_PRESSED;
				this->btn_cnt = 0;
			}
		}
		else
		{
			this->btn_cnt = 0;
		}
		break;
		
	case BTN_STATE_HOLD:
		if(PORTE.IN & PIN4_bm)
		{
			this->btn_state = BTN_STATE_NONE;
			this->btn_cnt = 0;
		}
		break;
	}
}

void CInput::pollEnc()
{
// 	int8_t enc_new = 0, enc_diff = 0;
// 	
// 	if(PORTE.IN & PIN3_bm)
// 	{
// 		enc_new = 3;
// 	}
// 	if(PORTE.IN & PIN2_bm)
// 	{
// 		enc_new ^= 1;
// 	}
// 	
// 	enc_diff = this->enc_last - enc_new;
// 	
// 	if(enc_diff & 1)
// 	{
// 		this->enc_last = enc_new;
// 		this->enc_delta += (enc_diff & 2) - 1;
// 	}

	this->enc_last = (this->enc_last << 2) & 0x0F;
	
	if(PORTE.IN & PIN3_bm)
	{
		this->enc_last |= 2;
	}
	if(PORTE.IN & PIN2_bm)
	{
		this->enc_last |= 1;
	}
	
	this->enc_delta += enc_table[this->enc_last];
}

uint8_t CInput::getBtn()
{
	uint8_t ret = 0;
	
	if(this->btn_state == BTN_STATE_PRESSED)
	{
		ret = 1;
		this->btn_state = BTN_STATE_HOLD;
	}
	
	return ret;
}

int8_t CInput::getEnc()
{
	int8_t ret;
	
	ret = this->enc_delta;
	this->enc_delta = 0;
	
	return ret;
}