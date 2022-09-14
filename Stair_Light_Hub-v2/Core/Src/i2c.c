/*
 * i2c.c
 *
 *  Created on: Dec 23, 2021
 *      Author: bradleyschulz
 */

#include "i2c.h"
#include "main.h"
#include "communication.h"

uint8_t cmd_buf;
uint8_t cmd_ready;

uint8_t active = 0;

uint8_t I2CActive() {
	return active;
}

uint8_t I2CUpdateReady() {
	return cmd_ready;
}

void receiveI2C() {
	cmd_ready = 0;
	HAL_I2C_Slave_Receive_IT(getI2C(), &cmd_buf, sizeof(cmd_buf));
}

void processI2C() {
	if (!active && cmd_buf != EXIT) {
		disableSense();
		active = 1;
	}
	clearLEDs();
	if (cmd_buf == BLUE) {
		for (int i = 0; i < NUM_TOP_LEDS; i += 2) {
			setLED(TOP, EMITTER, i, 0, 0, 100);
			setLED(TOP, RECEIVER, i, 0, 0, 100);
		}
		for (int i = 0; i < NUM_BOT_LEDS; i += 2) {
			setLED(BOT, EMITTER, i, 0, 0, 100);
			setLED(BOT, RECEIVER, i, 0, 0, 100);
		}
	} else if (cmd_buf == GREEN) {
		for (int i = 0; i < NUM_TOP_LEDS; i += 2) {
			setLED(TOP, EMITTER, i, 0, 100, 0);
			setLED(TOP, RECEIVER, i, 0, 100, 0);
		}
		for (int i = 0; i < NUM_BOT_LEDS; i += 2) {
			setLED(BOT, EMITTER, i, 0, 100, 0);
			setLED(BOT, RECEIVER, i, 0, 100, 0);
		}
	} else if (cmd_buf == RED) {
		for (int i = 0; i < NUM_TOP_LEDS; i += 2) {
			setLED(TOP, EMITTER, i, 100, 0, 0);
			setLED(TOP, RECEIVER, i, 100, 0, 0);
		}
		for (int i = 0; i < NUM_BOT_LEDS; i += 2) {
			setLED(BOT, EMITTER, i, 100, 0, 0);
			setLED(BOT, RECEIVER, i, 100, 0, 0);
		}
	} else if (cmd_buf == SHOW_TOP) {
		for (int i = 0; i < NUM_TOP_LEDS; i += 2) {
			setLED(TOP, EMITTER, i, 100, 0, 100);
			setLED(TOP, RECEIVER, i, 100, 0, 100);
		}
	} else if (cmd_buf == SHOW_BOTTOM) {
		for (int i = 0; i < NUM_BOT_LEDS; i += 2) {
			setLED(BOT, EMITTER, i, 0, 100, 100);
			setLED(BOT, RECEIVER, i, 0, 100, 100);
		}
	} else if (cmd_buf == SHOW_LEFT) {
		for (int i = 0; i < NUM_TOP_LEDS; i += 2) {
			setLED(TOP, RECEIVER, i, 100, 0, 0);
		}
		for (int i = 0; i < NUM_BOT_LEDS; i += 2) {
			setLED(BOT, EMITTER, i, 100, 0, 0);
		}
	} else if (cmd_buf == SHOW_RIGHT) {
		for (int i = 0; i < NUM_TOP_LEDS; i += 2) {
			setLED(TOP, EMITTER, i, 0, 0, 100);
		}
		for (int i = 0; i < NUM_BOT_LEDS; i += 2) {
			setLED(BOT, RECEIVER, i, 0, 0, 100);
		}
	} else if (cmd_buf == EXIT) {
		enableSense();
		active = 0;
	}
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef * hi2c) {
	cmd_ready = 1;
}


