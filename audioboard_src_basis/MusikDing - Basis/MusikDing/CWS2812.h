#ifndef __CWS2812_H__
#define __CWS2812_H__

#define LED_NUM			10

#define WS2812_HIGH		0x07 // 0x07
#define WS2812_LOW		0x3F // 0x1F

#define WS2812_DATASIZE		((LED_NUM * 3 * 8))

class CWS2812
{
//variables
public:
	
protected:

private:
	uint8_t data[WS2812_DATASIZE];

//functions
public:
	void transfer();
	void input(uint8_t *led_data, uint16_t len);
	
	uint8_t isBusy();
	
	CWS2812();
	~CWS2812();
protected:
private:
	CWS2812( const CWS2812 &c );
	CWS2812& operator=( const CWS2812 &c );

}; //CWS2812

#endif //__CWS2812_H__
