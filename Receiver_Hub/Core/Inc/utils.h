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
#include "sensors.h"


// Buffer length definitions
// Defined here because they are dependent on definitions in other files
#define NUM_SENSORS (1 + NUM_CHILDREN) * SENSORS_PER_BOARD
// Time a sensor stays on after being activated
#define ON_TIME 30

#define PI 3.14159265
#define SENSOR_THRESHOLD 100

void clearSensorHistory();
void decrementSensorTimes();
void registerReading(int sensor, uint8_t reading);
void displaySense();
void generateRGB(uint32_t index, uint32_t maxIndex, uint8_t brightness, uint8_t* r, uint8_t* g, uint8_t* b);

#endif /* INC_UTILS_H_ */
