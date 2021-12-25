/*
 * config.h
 *
 *  Created on: Dec 22, 2021
 *      Author: bradleyschulz
 */

#ifndef INC_CONFIG_H_
#define INC_CONFIG_H_


#define NUM_SENSOR_BOARDS 4
#define SENSORS_PER_BOARD 10
#define NUM_SENSORS NUM_SENSOR_BOARDS * SENSORS_PER_BOARD

#define SENSOR_BUF_LEN NUM_SENSORS/8 + 1

#endif /* INC_CONFIG_H_ */
