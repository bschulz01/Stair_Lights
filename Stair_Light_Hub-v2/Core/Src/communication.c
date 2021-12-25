/*
 * communication.c
 *
 *  Created on: Dec 8, 2021
 *      Author: bradleyschulz
 */

#include "communication.h"
#include "main.h"


// Buffers for sending and receiving data
uint8_t top_data;
uint8_t bot_data;
uint8_t top_sense_state;
uint8_t bot_sense_state;
uint8_t top_led_buf[NUM_TOP_LEDS * 3];
uint8_t bot_led_buf[NUM_BOT_LEDS * 3];


void resetIT() {
	HAL_UART_Abort_IT(getTopUART());
	HAL_UART_Abort_IT(getBotUART());
}

// Initializes polling of sense states
comm_stat_t initReception() {
	if (HAL_UART_Receive_IT(getTopUART(), &top_data, sizeof(top_data)) != HAL_OK) {
		return COMM_ERROR;
	}
	if (HAL_UART_Receive_IT(getBotUART(), &bot_data, sizeof(bot_data)) != HAL_OK) {
		return COMM_ERROR;
	}
	return COMM_OK;
}


/* FORMAT FOR LED DATA STORAGE
 * 	Emitter side is upper 4 bits
 * 	Receiver side is lower 4 bits
 * 	Indices go from bottom to top, receiver and then emitter
 */
void setLEDIndex(uint32_t led, uint8_t red, uint8_t green, uint8_t blue) {
	uint8_t mask = 0b11110000;
	if (led < NUM_BOT_LEDS) {
		// Bottom receiver side
		int idx = led;
		bot_led_buf[idx] = (bot_led_buf[idx] & mask) | (green >> 4);
		bot_led_buf[idx+1] = (bot_led_buf[idx+1] & mask) | (red >> 4);
		bot_led_buf[idx+2] = (bot_led_buf[idx+2] & mask) | (blue >> 4);
	} else if (led < 2*NUM_BOT_LEDS) {
		// Bottom emitter side
		int idx = (led-NUM_BOT_LEDS)*3;
		bot_led_buf[idx] = (green & mask) | (bot_led_buf[idx] & ~mask);
		bot_led_buf[idx+1] = (red & mask) | (bot_led_buf[idx+1] & ~mask);
		bot_led_buf[idx+2] = (blue & mask) | (bot_led_buf[idx+2] & ~mask);
	} else if (led < 2* NUM_BOT_LEDS + NUM_TOP_LEDS) {
		// Top receiver side
		int idx = (led-2*NUM_BOT_LEDS)*3;
		top_led_buf[idx] = (top_led_buf[idx] & mask) | (green >> 4);
		top_led_buf[idx+1] = (top_led_buf[idx+1] & mask) | (red >> 4);
		top_led_buf[idx+2] = (top_led_buf[idx+2] & mask) | (blue >> 4);
	} else if (led < 2*NUM_BOT_LEDS + 2*NUM_TOP_LEDS) {
		// Top emitter side
		int idx = (led-2*NUM_BOT_LEDS-NUM_TOP_LEDS)*3;
		top_led_buf[idx] = (green & mask) | (top_led_buf[idx] & ~mask);
		top_led_buf[idx+1] = (red & mask) | (top_led_buf[idx+1] & ~mask);
		top_led_buf[idx+2] = (blue & mask) | (top_led_buf[idx+2] & ~mask);
	}
}

void setLEDIndexByLevel(level_t level, uint32_t led, uint8_t red, uint8_t green, uint8_t blue) {
	if (level == TOP) {
		if (led < NUM_TOP_LEDS) {
			setLED(TOP, RECEIVER, led, red, green, blue);
		} else {
			setLED(TOP, EMITTER, led-NUM_TOP_LEDS, red, green, blue);
		}
	} else {
		if (led < NUM_BOT_LEDS) {
			setLED(BOT, RECEIVER, led, red, green, blue);
		} else {
			setLED(BOT, EMITTER, led-NUM_BOT_LEDS, red, green, blue);
		}
	}
}


void setLED(level_t level, side_t side, uint32_t index, uint8_t red, uint8_t green, uint8_t blue) {
	// Update the correct buffer
	uint8_t* buf;
	uint32_t maxLeds;
	if (level == BOT) {
		buf = bot_led_buf;
		maxLeds = NUM_BOT_LEDS;
	} else {
		buf = top_led_buf;
		maxLeds = NUM_TOP_LEDS;
	}
	uint8_t mask = 0b11110000;
	if (index < maxLeds) {
		if (side == RECEIVER) {
			// Set the lower 4 bits of each value to set the receiver side
			*(buf+3*index) = (*(buf+3*index)&mask) | (green >> 4);
			*(buf+3*index+1) = (*(buf+3*index+1)&mask) | (red >> 4);
			*(buf+3*index+2) = (*(buf+3*index+2)&mask) | (blue >> 4);
		} else if (side == EMITTER) {
			// Set the upper 4 bits of each value to set the emitter side
			*(buf+3*index) = (green & mask) | (*(buf+3*index) & ~mask);
			*(buf+3*index+1) = (red & mask) | (*(buf+3*index+1) & ~mask);
			*(buf+3*index+2) = (blue & mask) | (*(buf+3*index+2) & ~mask);
		}
	}
}

// Return 1 if an object was sensed
uint8_t objectSensed() {
	// Return 8 if no sensors were above the threshold
	return top_sense_state | bot_sense_state;
}


void pollSensors() {
	// Get data from top sensors
	uint8_t res = 0;
	uint8_t cmd = SEND_SENSOR_DATA;
	HAL_StatusTypeDef ret = HAL_ERROR;
	int numAttempts = 0;
	while (ret != HAL_OK && numAttempts < MAX_ATTEMPTS) {
		HAL_UART_Transmit(getTopUART(), &cmd, sizeof(cmd), SEND_TIMEOUT);
		ret = HAL_UART_Receive(getTopUART(), &res, sizeof(res), SENSOR_UPDATE_TIMEOUT);
		numAttempts++;
	}
	if (res == OBJECT_SENSED) {
		top_sense_state = 1;
	} else {
		top_sense_state = 0;
	}
	// Get data from bottom sensors
	ret = HAL_ERROR;
	res = 0;
	numAttempts = 0;
	while (ret != HAL_OK && numAttempts < MAX_ATTEMPTS) {
		HAL_UART_Transmit(getBotUART(), &cmd, sizeof(cmd), SEND_TIMEOUT);
		ret = HAL_UART_Receive(getBotUART(), &res, sizeof(res), SENSOR_UPDATE_TIMEOUT);
		numAttempts++;
	}
	if (res == OBJECT_SENSED) {
		bot_sense_state = 1;
	} else {
		bot_sense_state = 0;
	}
}
void recalibrate() {
	uint8_t data = RECALIBRATE;
	sendCommand(getTopUART(), data);
	sendCommand(getBotUART(), data);
}

void clearLEDs() {
	// reset the LED buffers
	for (int i = 0; i < NUM_TOP_LEDS * 3; i++) {
		top_led_buf[i] = 0;
	}
	for (int i = 0; i < NUM_BOT_LEDS * 3; i++) {
		bot_led_buf[i] = 0;
	}
	// Send message to clear LEDs
	uint8_t data = CLEAR_LEDS;
	sendCommand(getTopUART(), data);
	sendCommand(getBotUART(), data);
}


// Send all the LED data to each board
void updateLEDs() {
	int startIndex = 0;
//	comm_stat_t status;
	// Send data to bottom leds
	// Update in batches until all leds have been updated
	while (startIndex < NUM_BOT_LEDS) {
		sendLEDVals(BOT, startIndex, MAX_LED_UPDATE);
//		if (status != COMM_OK) {
//			return;
//		}
		startIndex += MAX_LED_UPDATE;
	}
//	uint8_t complete_flag = UPDATE_COMPLETE;
	sendCommand(getBotUART(), UPDATE_COMPLETE);
	// Send data to top leds
	startIndex = 0;
	while (startIndex < NUM_TOP_LEDS) {
		sendLEDVals(TOP, startIndex, MAX_LED_UPDATE);
		startIndex += MAX_LED_UPDATE;
//		if (status != COMM_OK) {
//			return;
//		}
	}
	sendCommand(getTopUART(), UPDATE_COMPLETE);

}

// Send data for specific leds
/* Protocol for sending LED values
 * 	1) Send request to update LED values
 * 		Request is 3 bytes:
 * 			- The command
 * 			- The index of the first LED being updated
 * 			- The number of LEDs to be updated
 * 	2) Wait for ACK from downstream board
 * 	3) Send LED data once ACK is received
 */
comm_stat_t sendLEDVals(level_t level, uint8_t startIndex, uint8_t len) {

	// Send request to send data with start index and number of LEDs to update
	UART_HandleTypeDef* uartPtr;
	uint8_t* LEDdata;

	len = len > MAX_LED_UPDATE ? MAX_LED_UPDATE : len;
	if (level == TOP) {	// transmit to top side
		// Truncate indices if too long
		startIndex = startIndex > NUM_TOP_LEDS ? NUM_TOP_LEDS : startIndex;
		len = startIndex + len > NUM_TOP_LEDS ? NUM_TOP_LEDS - startIndex : len;
		// Set the uart pointer
		uartPtr = getTopUART();
		// Get the address of the buffer to transmit
		LEDdata = &top_led_buf[3*startIndex];
	} else { // transmot to bottom side
		// Truncate indices if too long
		startIndex = startIndex > NUM_BOT_LEDS ? NUM_BOT_LEDS : startIndex;
		len = startIndex + len > NUM_BOT_LEDS ? NUM_BOT_LEDS - startIndex : len;
		// Set the uart pointer
		uartPtr = getBotUART();
		// Get the address of the buffer to transmit
		LEDdata = &bot_led_buf[3*startIndex];
	}
	// Send request to update leds
	comm_stat_t status = sendCommand(uartPtr, SET_LED_VALS);
	// Return if command was not successful
	if (status != COMM_OK) {
		return status;
	}
	// Send info about data transfer
	uint8_t info[2] = {startIndex, len};
	HAL_UART_Transmit_DMA(uartPtr, info, sizeof(info));
	uint8_t rec = 0;
	HAL_UART_Receive(uartPtr, &rec, sizeof(rec), ACK_TIMEOUT);
	// Send LED data if ack was received
	if (rec == ACK) {
		HAL_UART_Transmit(uartPtr, LEDdata, len*3, LED_UPDATE_TIMEOUT);
	} else {
		return COMM_ERROR;
	}
	return COMM_OK;
}

void enableSense() {
	sendCommand(getTopUART(), ENABLE_SENSE);
	sendCommand(getBotUART(), ENABLE_SENSE);
}

void disableSense() {
	sendCommand(getTopUART(), DISABLE_SENSE);
	sendCommand(getBotUART(), DISABLE_SENSE);
}

comm_stat_t sendCommand(UART_HandleTypeDef* huart, cmd_t cmd) {
	// Retry if needed, then give up after too many attempts
	int numAttempts = 0;
	uint8_t data = cmd;
	uint8_t rec = 0;
	while(numAttempts < MAX_ATTEMPTS) {
//		if (numAttempts > 0) {
//			HAL_Delay(1);
//		}
		HAL_UART_Transmit_DMA(huart, &data, sizeof(data));
		HAL_UART_Receive(huart, &rec, sizeof(rec), ACK_TIMEOUT);
		if (rec == ACK) {
			return COMM_OK;
		}
		numAttempts++;
	}
	return COMM_ERROR;
}
