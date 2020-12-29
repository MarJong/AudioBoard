#ifndef __CLED_H__
#define __CLED_H__

typedef struct 
{
	uint8_t r, g, b;
} rgb_t;

#define LED_NUM_X	7
#define LED_NUM_Y	6
#define LED_NUM		(LED_NUM_X * LED_NUM_Y)

class CLED
{
//variables
public:

protected:

private:
	rgb_t data[LED_NUM];

//functions
public:
	CLED();
	~CLED();
	
	void update();
	
	void setLED_RGB(uint8_t x, uint8_t y, rgb_t col);
	void setLED_RGB(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b);
	void setLED_HSV(uint8_t x, uint8_t y, uint8_t hue, uint8_t sat, uint8_t val);
	void setLED_HSV(rgb_t *rgb, uint8_t hue, uint8_t sat, uint8_t val);
	
	rgb_t* getLED(uint8_t x, uint8_t y);
	
	void clear();
	
	uint8_t isBusy();
	
protected:

private:
	CLED( const CLED &c );
	CLED& operator=( const CLED &c );

}; //CLED

#endif //__CLED_H__
