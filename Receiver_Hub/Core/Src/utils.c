/*
 * utils.c
 *
 *  Created on: Sep 9, 2021
 *      Author: bradleyschulz
 */

#include "utils.h"
#include "uart.h"
#include "WS2812.h"
#include "sensors.h"
#include <math.h> // for the declaration of the number of sensor

uint8_t min_readings[NUM_SENSORS];
uint8_t max_readings[NUM_SENSORS];
uint8_t sensor_activations[NUM_SENSORS];

void clearSensorHistory() {
	for (int i = 0; i < NUM_SENSORS; i++) {
		min_readings[i] = 255;
		max_readings[i] = 50;
	}
}

void registerReading(int sensor, uint8_t reading) {
	// update the min and max readings for this sensor
	if (reading < min_readings[sensor]) {
		min_readings[sensor] = reading;
		// Make the max reading no less than 50 more than the min reading
		max_readings[sensor] = max_readings[sensor] - 50 > reading ?
				max_readings[sensor] :
				min_readings[sensor] + 50;
	}
	if (reading > max_readings[sensor]) {
		max_readings[sensor] = reading;
	}
	// Update the sensor activations
	if (reading > min_readings[sensor] + 20) {
		sensor_activations[sensor] = ON_TIME;
	} else if (sensor_activations[sensor] > 0) {
		sensor_activations[sensor]--;
	}
}


void displaySense() {
	int numSensors = (1 + NUM_CHILDREN) * SENSORS_PER_BOARD;
	int ledsPerSensor = NUM_LEDS / numSensors;
	int extraLEDs = NUM_LEDS - numSensors * ledsPerSensor;
	int extraInterval = numSensors / extraLEDs;
	// Iterate through each sensor
	for (int sensor = 0; sensor < numSensors; sensor++) {
		uint8_t r = 0;
		uint8_t g = 0;
		uint8_t b = 0;
		// Determine if sensor is activated by reading the correct bit in the activation bitmap
		if (sensor_activations[sensor]) {
			generateRGB(sensor_activations[sensor], ON_TIME, 200, &r, &g, &b);
		}
		int startIndex = ledsPerSensor * sensor + sensor / extraInterval;
		for (int i = 0; i < ledsPerSensor; i++) {
			setLED(startIndex + i, r, g, b);
		}
		// Control an additional LED if this sensor is mapped to an extra LED
		if (sensor % extraInterval == (extraInterval - 1)) {
			setLED(startIndex + ledsPerSensor, r, g, b);
		}
	}
}


void generateRGB(uint32_t index, uint32_t maxIndex, uint8_t brightness, uint8_t* r, uint8_t* g, uint8_t* b)
{
//	float frequency = 3 * PI / maxIndex;
//	float amplitude = brightness / 2;
//	float cos1 = amplitude * cos(index * frequency) + amplitude;
//	float cos2 = -1 * amplitude * cos(index * frequency) + amplitude;
//	// red
//	if (index < maxIndex / 3)
//	{
//		*r = (uint8_t) cos1;
//	}
//	else if (index >= 2 * maxIndex / 3)
//	{
//		*r = (uint8_t) cos2;
//	}
//	// green
//	if (index < 2 * maxIndex / 3)
//	{
//		*g = (uint8_t) cos2;
//	}
//	// blue
//	if (index >= maxIndex / 3)
//	{
//		*b = (uint8_t) cos1;
//	}

	// Choose one of 6 colors
	uint8_t div = maxIndex / 6;
	if (index < div) {
		*r = 0;
		*g = brightness;
		*b = 0;
	} else if (index < 2*div) {
		*r = brightness/2;
		*g = brightness/2;
		*b = 0;
	} else if (index < 3*div) {
		*r = brightness;
		*g = 0;
		*b = 0;
	} else if (index < 4*div) {
		*r = brightness/2;
		*g = 0;
		*b = brightness/2;
	} else if (index < 5*div) {
		*r = 0;
		*g = 0;
		*b = brightness;
	} else {
		*r = 0;
		*g = brightness/2;
		*b = brightness/2;
	}
}
