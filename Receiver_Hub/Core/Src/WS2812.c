/*
 * WS2812.c
 *
 *  Created on: Jul 17, 2021
 *      Author: bradleyschulz
 */


#include "main.h"
#include "WS2812.h"
#include <math.h>

// RECEIVER VERSION
// Values are the lower 4 bits of each entry

uint8_t LED_data[NUM_LEDS*3];
uint16_t PWM_data[BUFFER_LENGTH];

uint8_t dataSentFlag;


// Callback for when data transfer is complete
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
	HAL_TIM_PWM_Stop_DMA(getLEDTimer(), TIM_CHANNEL);
	dataSentFlag = 1;
}

uint8_t* getLEDBuf(uint8_t led) {
	return &LED_data[led*3];
}

void setLED(uint32_t LED, uint8_t Red, uint8_t Green, uint8_t Blue)
{
	if (LED < NUM_LEDS) {
		LED_data[LED*3] = Green;
		LED_data[LED*3+1] = Red;
		LED_data[LED*3+2] = Blue;
	}
}


void clearLEDs()
{
	for (int index = 0; index < NUM_LEDS; index++)
	{
		setLED(index, 0, 0, 0);
	}
}

//// Sets the brightness on a scale from 0-45
//void setBrightness(uint8_t brightness, uint8_t strip)
//{
//	// Truncate brightness
//	if (brightness > 45)
//	{
//		brightness = 45;
//	}
//	for (uint32_t i = 0; i < NUM_LEDS; i++)
//	{
//		for (uint8_t j = 0; j < 3; j++)
//		{
//			float angle = (90 - brightness) * PI / 180; // convert degrees to radians
//			LED_Data[strip][i][j] = (LED_Data[strip][i][j]) / tan(angle); // scale brightness
//		}
//	}
//}


void updateWS2812()
{
	uint32_t color;
	uint8_t mask = 0b00001111;

	uint16_t idx = 0;
	// Load buffer with LED data
	for (uint32_t led = 0; led < NUM_LEDS; led++)
	{
		// Generate bits to describe color
		// Shift an extra 4 bits since the relevant bits are the lower 4 bits
		color = ((LED_data[led*3] & mask) << 20) |
				((LED_data[led*3+1] & mask) << 12) |
				((LED_data[led*3+2] & mask) << 4);

		// Set the buffer for this LED
		// Send MSB to LSB
		for (int16_t bit = 23; bit >= 0; bit--)
		{
			//uint32_t idx = 24 * led + bit;
			if (color & (1<<bit))
			{
				PWM_data[idx] = HIGH_TIME; // High bit
			}
			else
			{
				PWM_data[idx] = LOW_TIME; // Low bit
			}
			idx++;
		}
	}

	// Reset the communication line to signify end of transmission
	while (idx < BUFFER_LENGTH)
	{
		PWM_data[idx] = 0;
		idx++;
	}

	// Begin transfer of data
	HAL_TIM_PWM_Start_DMA(getLEDTimer(), TIM_CHANNEL, (uint32_t *)PWM_data, idx);

	while(!dataSentFlag)
		continue;

	dataSentFlag = 0;
}


