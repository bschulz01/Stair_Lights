/*
 * sensors.h
 *
 *  Created on: Aug 25, 2021
 *      Author: bradleyschulz
 */

#ifndef INC_SENSORS_H_
#define INC_SENSORS_H_

#define NUM_SENSORS 10

#define CALIBRATION_READINGS 10
#define SENSOR_THRESHOLD 100


// Determines if sensors are activated when higher or lower values are received
// Should not be defined if sensors measure reflected light (ie
#define SENSE_HIGH

uint8_t mapSensor(uint8_t idx);
uint16_t* getNewReadings();
uint16_t* getLastReadings();
uint16_t readSensor(uint8_t sensor);
uint16_t readADC(uint32_t channel);

#endif /* INC_SENSORS_H_ */
