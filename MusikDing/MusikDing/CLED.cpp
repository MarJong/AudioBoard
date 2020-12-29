#include <avr/io.h>
#include <string.h>

#include "CLED.h"
#include "CWS2812.h"

CWS2812 ws2812;

// default constructor
CLED::CLED()
{
	memset(this->data, 0, sizeof(rgb_t) * LED_NUM);
} //CLED

// default destructor
CLED::~CLED()
{
} //~CLED

void CLED::update()
{
	ws2812.input((uint8_t*)this->data, LED_NUM * 3);
	ws2812.transfer();
}

void CLED::setLED_RGB(uint8_t x, uint8_t y, rgb_t col)
{
	this->setLED_RGB(x, y, col.r, col.g, col.b);
// 	if(x >= LED_NUM_X || y >= LED_NUM_Y)
// 	{
// 		return;
// 	}
// 	
// 	if(x & 1)	// ungerade
// 	{
// 		this->data[(x * LED_NUM_Y) + y].r = col.r;
// 		this->data[(x * LED_NUM_Y) + y].g = col.g;
// 		this->data[(x * LED_NUM_Y) + y].b = col.b;
// 	}
// 	else		// gerade
// 	{
// 		this->data[(x * LED_NUM_Y) + (LED_NUM_Y - 1 - y)].r = col.r;
// 		this->data[(x * LED_NUM_Y) + (LED_NUM_Y - 1 - y)].g = col.g;
// 		this->data[(x * LED_NUM_Y) + (LED_NUM_Y - 1 - y)].b = col.b;
// 	}
}

void CLED::setLED_RGB(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b)
{
	if(x >= LED_NUM_X || y >= LED_NUM_Y)
	{
		return;
	}
	
	if(x & 1)	// ungerade
	{
		this->data[(x * LED_NUM_Y) + (LED_NUM_Y - 1 - y)].r = r;
		this->data[(x * LED_NUM_Y) + (LED_NUM_Y - 1 - y)].g = g;
		this->data[(x * LED_NUM_Y) + (LED_NUM_Y - 1 - y)].b = b;
	}
	else		// gerade
	{
		this->data[(x * LED_NUM_Y) + (y)].r = r;
		this->data[(x * LED_NUM_Y) + (y)].g = g;
		this->data[(x * LED_NUM_Y) + (y)].b = b;
	}
}

void CLED::setLED_HSV(uint8_t x, uint8_t y, uint8_t hue, uint8_t sat, uint8_t val)
{
	uint16_t subhue;
	uint32_t y1, y2, y3;
	rgb_t rgb;
	
	if(x >= LED_NUM_X || y >= LED_NUM_Y)
	{
		return;
	}
	
	if(sat == 0)
	{
		rgb.r = val;
		rgb.g = val;
		rgb.b = val;
		this->setLED_RGB(x, y, rgb);
		return;
	}
	
	if(hue > 251)
	{
		hue = 251;
	}
	
	subhue = (uint16_t)(hue % 42) * 255 / 41;
	hue /= 42;
	
	y1 = (uint32_t)(val * (255 - sat)) / 255;
	y2 = (uint32_t)(val * (255 - (sat * subhue) / 255)) / 255;
	y3 = (uint32_t)(val * (255 - (sat * (255 - subhue)) / 255)) / 255;
	
	switch(hue)
	{
		case 0:
			rgb.r = val;
			rgb.g = y3;
			rgb.b = y1;
		break;
		
		case 1:
			rgb.r = y2;
			rgb.g = val;
			rgb.b = y1;
		break;
		
		case 2:
			rgb.r = y1;
			rgb.g = val;
			rgb.b = y3;
		break;
		
		case 3:
			rgb.r = y1;
			rgb.g = y2;
			rgb.b = val;
		break;
		
		case 4:
			rgb.r = y3;
			rgb.g = y1;
			rgb.b = val;
		break;
		
		default:
			rgb.r = val;
			rgb.g = y1;
			rgb.b = y2;
		break;
	}
	
	this->setLED_RGB(x, y, rgb);
	
// 	if(x & 1)	// ungerade
// 	{
// 		this->data[(x * LED_NUM_Y) + y].r = rgb.r;
// 		this->data[(x * LED_NUM_Y) + y].g = rgb.g;
// 		this->data[(x * LED_NUM_Y) + y].b = rgb.b;
// 	}
// 	else		// gerade
// 	{
// 		this->data[(x * LED_NUM_Y) + (LED_NUM_Y - 1 - y)].r = rgb.r;
// 		this->data[(x * LED_NUM_Y) + (LED_NUM_Y - 1 - y)].g = rgb.g;
// 		this->data[(x * LED_NUM_Y) + (LED_NUM_Y - 1 - y)].b = rgb.b;
// 	}
}

rgb_t* CLED::getLED(uint8_t x, uint8_t y)
{
	if(x >= LED_NUM_X || y >= LED_NUM_Y)
	{
		return NULL;
	}
	
	if(x & 1)	// ungerade
	{
		return &this->data[(x * LED_NUM_Y) + (LED_NUM_Y - 1 - y)];
	}
	
	return &this->data[(x * LED_NUM_Y) + (y)];
}

uint8_t CLED::isBusy()
{
	return ws2812.isBusy();
}

void CLED::clear()
{
	memset(this->data, 0, sizeof(rgb_t) * LED_NUM);
}

void CLED::setLED_HSV(rgb_t *rgb, uint8_t hue, uint8_t sat, uint8_t val)
{
	uint16_t subhue;
	uint32_t y1, y2, y3;
	
	if(sat == 0)
	{
		rgb->r = val;
		rgb->g = val;
		rgb->b = val;
		return;
	}
	
	if(hue > 251)
	{
		hue = 251;
	}
	
	subhue = (uint16_t)(hue % 42) * 255 / 41;
	hue /= 42;
	
	y1 = (uint32_t)(val * (255 - sat)) / 255;
	y2 = (uint32_t)(val * (255 - (sat * subhue) / 255)) / 255;
	y3 = (uint32_t)(val * (255 - (sat * (255 - subhue)) / 255)) / 255;
	
	switch(hue)
	{
		case 0:
		rgb->r = val;
		rgb->g = y3;
		rgb->b = y1;
		break;
		
		case 1:
		rgb->r = y2;
		rgb->g = val;
		rgb->b = y1;
		break;
		
		case 2:
		rgb->r = y1;
		rgb->g = val;
		rgb->b = y3;
		break;
		
		case 3:
		rgb->r = y1;
		rgb->g = y2;
		rgb->b = val;
		break;
		
		case 4:
		rgb->r = y3;
		rgb->g = y1;
		rgb->b = val;
		break;
		
		default:
		rgb->r = val;
		rgb->g = y1;
		rgb->b = y2;
		break;
	}
}