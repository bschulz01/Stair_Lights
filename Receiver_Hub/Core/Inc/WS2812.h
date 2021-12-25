/*
 * WS2812.h
 *
 *  Created on: Jul 17, 2021
 *      Author: bradleyschulz
 *
 *  Based on guide from
 *  	https://controllerstech.com/interface-ws2812-with-stm32/
 */

#ifndef INC_WS2812_H_
#define INC_WS2812_H_

#include <stdint.h>

// CUSTOMIZE THESE DEFINITIONS
#define NUM_LEDS 128
#define LOW_TIME 27
#define HIGH_TIME 54
#define TIM_CHANNEL TIM_CHANNEL_2

// END CUSTOM DEFINITIONS
#define RESET_LENGTH 50
#define BUFFER_LENGTH (24*NUM_LEDS + RESET_LENGTH)
#define PI 3.14159265

void initWS2812();
uint8_t* getLEDBuf(uint8_t led);
void setLED(uint32_t LED, uint8_t Red, uint8_t Green, uint8_t Blue);
void clearLEDs();
//void setBrightness(uint8_t brightness, uint8_t strip);
void updateWS2812();

#endif /* INC_WS2812_H_ */
