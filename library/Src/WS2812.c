/*
 * WS2812.c
 *
 *  Created on: Jul 17, 2021
 *      Author: bradleyschulz
 */


#include "main.h"
#include "WS2812.h"
#include <string.h>

// EMITTER VERSION
// Values are the upper 4 bits of each entry


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
//	if (LED < NUM_LEDS) {
//		LED_Data[LED][0] = Green;
//		LED_Data[LED][1] = Red;
//		LED_Data[LED][2] = Blue;
//	}
}


void clearLEDs()
{
//	for (int index = 0; index < NUM_LEDS; index++)
//	{
//		setLED(index, 0, 0, 0);
//	}
	memset(LED_data, 0, sizeof(LED_data));
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

	uint16_t idx = 0;
	//uint8_t mask = 0b11110000;
	// Load buffer with LED data
	for (uint32_t led = 0; led < NUM_LEDS; led++)
	{
		// Generate bits to describe color
		//color = ((LED_data[led*3] & mask) << 16) |
		//		((LED_data[led*3+1] & mask) << 8) |
		//		(LED_data[led*3+2] & mask);
		color = ((LED_data[led*3]) << 16) |
				((LED_data[led*3+1]) << 8) |
				((LED_data[led*3+2]));

		// Set the buffer for this LED
		// Send MSB to LSB
		for (int16_t bit = 23; bit >= 0; bit--)
		{
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
	dataSentFlag = 0;
	HAL_TIM_PWM_Start_DMA(getLEDTimer(), TIM_CHANNEL, (uint32_t *)PWM_data, idx);

//	int counter = 0;
	while(!dataSentFlag) {
//		if (counter > MAX_UPDATE_TIME) {
//			HAL_TIM_PWM_Stop_DMA(getLEDTimer(), TIM_CHANNEL);
//			dataSentFlag = 1;
//		}
//		counter++;
	}
}


