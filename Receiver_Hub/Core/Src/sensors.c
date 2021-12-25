/*
 * sensors.c
 *
 *  Created on: Aug 25, 2021
 *      Author: bradleyschulz
 */

#include "main.h"
#include "sensors.h"

//uint32_t adc_buf[NUM_SAMPLES];
uint8_t complete;

uint16_t sensor_refs[SENSORS_PER_BOARD];

uint16_t prev_readings[SENSORS_PER_BOARD];

uint32_t ADC_channels[SENSORS_PER_BOARD] = {
		ADC_CHANNEL_0,
		ADC_CHANNEL_1,
		ADC_CHANNEL_2,
		ADC_CHANNEL_3,
		ADC_CHANNEL_4,
		ADC_CHANNEL_5,
		ADC_CHANNEL_6,
		ADC_CHANNEL_7,
		ADC_CHANNEL_8,
		ADC_CHANNEL_9,
};

// Maps the sensor index to the correct location
uint8_t mapSensor(uint8_t idx) {
#ifndef REVERSE_ORDER
	if (idx == 0) {
		return 4;
	} else if (idx < 5) {
		return idx - 1;
	} else {
		return idx;
	}
#else
	if (idx == 0) {
		return 5;
	} else if (idx < 5) {
		return 10 - idx;
	} else {
		return 9 - idx;
	}
#endif
}

//. collect new readings from the sensors
uint16_t* getNewReadings()
{
	for (int sensor = 0; sensor < SENSORS_PER_BOARD; sensor++) {
		prev_readings[mapSensor(sensor)] = readSensor(sensor);
	}
	return prev_readings;
}


// retrieve most recent sensor readings
uint16_t* getLastReadings()
{
	return prev_readings;
}

uint16_t readSensor(uint8_t sensor)
{
	// Do not use the 9th sensor for the reversed side
#ifdef REVERSE_ORDER
	if (sensor == 9) {
		return 0;
	}
#endif
	return readADC(ADC_channels[sensor]);;
}


uint16_t readADC(uint32_t channel)
{


	ADC_ChannelConfTypeDef sConfig = {0}; //this initializes the IR ADC [Analog to Digital Converter]
	ADC_HandleTypeDef *adcPtr = getADCPtr(); //this is a pointer to your hal_adc
	//this pointer will also be used to read the analog value, val = HAL_ADC_GetValue(hadc1_ptr);

	//this picks the IR direction to choose the right ADC.
	sConfig.Channel = channel;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;

	// make sure everything was set up correctly
	if (HAL_ADC_ConfigChannel(adcPtr, &sConfig) != HAL_OK)
	{
		return 0;
	}

	HAL_ADC_Start(adcPtr);

	HAL_ADC_PollForConversion(adcPtr, 1);

	uint16_t val = HAL_ADC_GetValue(adcPtr);

	HAL_ADC_Stop(adcPtr);

	return val;
}
