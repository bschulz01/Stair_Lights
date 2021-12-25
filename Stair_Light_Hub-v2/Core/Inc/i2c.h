/*
 * i2c.h
 *
 *  Created on: Dec 23, 2021
 *      Author: bradleyschulz
 */

#ifndef INC_I2C_H_
#define INC_I2C_H_

#include <stdint.h>

typedef enum {
	BLUE = 0b00000100,
	GREEN = 0b00000010,
	RED = 0b00000001,
	SHOW_TOP = 0b01000000,
	SHOW_BOTTOM = 0b01010000,
	SHOW_LEFT = 0b00100000,
	SHOW_RIGHT = 0b00101000,
	EXIT = 0b01010101
} i2c_cmd_t;


void receiveI2C();
uint8_t I2CActive();
uint8_t I2CUpdateReady();
void processI2C();

#endif /* INC_I2C_H_ */
