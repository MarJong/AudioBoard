#ifndef __CINPUT_H__
#define __CINPUT_H__

#include <avr/io.h>

class CInput
{
//variables
public:
protected:
private:
	uint8_t btn_state, btn_cnt;
	int8_t enc_delta, enc_last;

//functions
public:
	CInput();
	~CInput();
	
	void pollBtn();
	void pollEnc();
	
	uint8_t getBtn();
	int8_t getEnc();
	
protected:
private:
	CInput( const CInput &c );
	CInput& operator=( const CInput &c );

}; //CInput

#endif //__CINPUT_H__
