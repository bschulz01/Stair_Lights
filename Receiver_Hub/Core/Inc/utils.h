/*
 * utils.h
 *
 *  Created on: Sep 9, 2021
 *      Author: bradleyschulz
 */

#ifndef INC_UTILS_H_
#define INC_UTILS_H_

#include <stdint.h>
#include "uart.h"
#if (POSITION_SIDE == SIDE_RECEIVER)
#include "sensors.h"
#endif
#include "config.h"
#include "WS2812.h"

// Buffer length definitions
// Defined here because they are dependent on definitions in other files
#define NUM_SENSORS (NUM_BOARDS * SENSORS_PER_BOARD)
// Time a sensor stays on after being activated
#define ON_TIME 50

// Macros for displaying sensors
#define LEDS_PER_SENSOR (NUM_LEDS / NUM_SENSORS)

#define PI 3.14159265
#define SENSOR_THRESHOLD 100

void clearSensorHistory();
void decrementSensorTimes();
#if (POSITION_SIDE == SIDE_RECEIVER)
// Defined here because they are dependent on definitions in other files
void registerReading(int sensor, uint8_t reading);
#endif
void displaySense();
void generateRGB_sense(uint32_t index, uint32_t maxIndex, uint8_t brightness, uint8_t* r, uint8_t* g, uint8_t* b);

#endif /* INC_UTILS_H_ */
