/*
 * uart.c
 *
 *  Created on: Dec 6, 2021
 *      Author: bradleyschulz
 */

#include "uart.h"
#include "utils.h"
#include "WS2812.h"
#include "config.h"

// EMITTER HUB VERSION

// UFP = upstream facing port
// DFP = downstream facing port

uart_state_t UFP_state;

// Stores if LEDs should be updated
uint8_t update_leds;

// Says if command is ready to be processed
uint8_t cmd_ready;

// Stores command received from hub
uint8_t cmd;
// Stores ack from receiver
uint8_t rec;
// Stores sensor values
// Sensor activations are stored as a bitmap
uint8_t sensor_activations[NUM_SENSORS];
// Stores whether or not the last sensor readings indicated an activated sensor
uint8_t sense_state;
// Whether or not sensing can be displayed
uint8_t display_sense;
// Current state of the user LED
uint8_t led_state;


void resetIT() {
	HAL_UART_Abort_IT(getUFP());
	HAL_UART_Abort_IT(getDFP());
}

// initialize polling for receive interrupts
void receiveIT() {
	cmd_ready = 0;
	HAL_UART_Abort_IT(getUFP());
	HAL_UART_Receive_IT(getUFP(), &cmd, 1);
//	HAL_UART_Receive_IT(getDFP(), sensor_buf, SENSOR_BUF_LEN);
}

void toggleLED() {
	if (led_state) {
	  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
	  led_state = 0;
	} else {
	  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
	  led_state = 1;
	}
}

// Returns whether LEDs should be updated
uint8_t updateLEDs() {
	if (update_leds) {
		update_leds = 0;
		return 1;
	} else {
		return 0;
	}
}

uint8_t cmdReady() {
	return cmd_ready;
}

// Send ACK to UFP
void sendACK() {
	uint8_t data = ACK;
	HAL_UART_Transmit(getUFP(), &data, 1, SEND_TIMEOUT);
}


// extract the corred
uint8_t getActivation(uint8_t sensor) {
	if (sensor < NUM_SENSORS) {
		return sensor_activations[sensor];
	} else {
		return 0;
	}
}

comm_stat_t sendCommand(UART_HandleTypeDef* huart, cmd_t cmd) {
	// Retry if needed, then give up after too many attempts
	HAL_StatusTypeDef ret = HAL_ERROR;
	int numAttempts = 0;
	uint8_t rec = 0;
	while(ret != HAL_OK && numAttempts < MAX_ATTEMPTS) {
		// Transmit command
		HAL_UART_Transmit(huart, &cmd, sizeof(cmd), SEND_TIMEOUT);
		// Wait for ACK
		ret = HAL_UART_Receive(huart, &rec, sizeof(rec), ACK_TIMEOUT);
		numAttempts++;
	}
	if (rec != ACK) {
		return COMM_ERROR;
	} else {
		return COMM_OK;
	}
}

// Reads command from hub in order to see if action needs to be taken
HAL_StatusTypeDef processCommand() {
	HAL_StatusTypeDef ret = HAL_OK;
	if (cmd == UPDATE_COMPLETE) {
		sendACK();
		// Toggle the LED
//		toggleLED();
		// Once update is complete, get sensor values
		uint8_t cmd = SEND_SENSOR_DATA;
		HAL_StatusTypeDef ret = HAL_ERROR;
		int numAttempts = 0;
		while (ret != HAL_OK && numAttempts < MAX_ATTEMPTS) {
			HAL_UART_Transmit(getDFP(), &cmd, sizeof(cmd), SEND_TIMEOUT);
			ret = HAL_UART_Receive(getDFP(), sensor_activations, NUM_SENSORS, SENSOR_UPDATE_TIMEOUT);
			numAttempts++;

		}
		// Update sense state so it is ready to return to hub when requested
		// Sensors are activated if any sensor (individual bit) indicates that a sensor was activated
		sense_state = OBJECT_NOT_SENSED;
		for (int i = 0; i < NUM_SENSORS; i++) {
			if (sensor_activations[i] > 0) {
				sense_state = OBJECT_SENSED;
			}
		}
		// Overwrite sent values with sense display if an object was sensed
		if (display_sense && sense_state == OBJECT_SENSED) {
			clearLEDs();
			displaySense();
		}
		// Set leds to update
		update_leds = 1;
	} else if (cmd == SEND_SENSOR_DATA) {
		// Send last known sense state back to hub
		ret = HAL_UART_Transmit(getUFP(), &sense_state, sizeof(sense_state), SEND_TIMEOUT);
		display_sense = 1;
	} else if (cmd == RECALIBRATE) {
		sendACK();
		// Pass command to downstream boards
		sendCommand(getDFP(), cmd);
		if (ret != HAL_OK) {
			return ret;
		}
	} else if (cmd == ENABLE_SENSE) {
		sendACK(getUFP());
		display_sense = 1;
		sendCommand(getDFP(), ENABLE_SENSE);
	} else if (cmd == DISABLE_SENSE) {
		sendACK(getUFP());
		display_sense = 0;
		sendCommand(getDFP(), DISABLE_SENSE);
	}  else if (cmd == CLEAR_LEDS) {
		sendACK();
		// Pass command to downstream boards
		sendCommand(getDFP(), cmd);
		// Clear LEDs and update strips
		clearLEDs();
		// Set leds to update
		update_leds = 1;
	} else if (cmd == SET_LED_VALS) {
		// Begin waiting for LED data from UFP
		uint8_t info[2] = {0, 0};
		// Return an ACK to begin handshake
		sendACK();
		// Receive info about the data transfer
		HAL_UART_Receive(getUFP(), info, sizeof(info), INFO_TIMEOUT);
		int led = info[0];
		int bytes = info[1]*3;
		// Send ack back to UFP to begin transmission of data
		sendACK();
		// Get data
		uint8_t* ledBufPtr = getLEDBuf(led);
		ret = HAL_UART_Receive(getUFP(), ledBufPtr, bytes, LED_UPDATE_TIMEOUT);
		if (ret != HAL_OK) {
			return ret;
		}
	}
	return ret;

}


// Send all the LED data to downstream receiver board
void sendAllLEDs() {
	int startIndex = 0;
	// Update in batches until all leds have been updated
	while (startIndex < NUM_LEDS) {
		sendLEDVals(startIndex, MAX_LED_UPDATE);
		startIndex += MAX_LED_UPDATE;
	}
	// Notify that update is complete
	sendCommand(getDFP(), UPDATE_COMPLETE);
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
comm_stat_t sendLEDVals(uint8_t startIndex, uint8_t len) {
	// Truncate indices if too long
	startIndex = startIndex > NUM_LEDS ? NUM_LEDS : startIndex;
	len = startIndex + len > NUM_LEDS ? NUM_LEDS - startIndex : len;

	// Send request to send data with start index and number of LEDs to update
	UART_HandleTypeDef* uartPtr = getDFP();
	uint8_t* LEDdata = getLEDBuf(startIndex);

	// Send request to update leds
	comm_stat_t status = sendCommand(uartPtr, SET_LED_VALS);
	// Return if command was not successful
	if (status != COMM_OK) {
		return status;
	}
	// Send info about data transfer
	uint8_t info[2] = {startIndex, len};
	HAL_UART_Transmit(uartPtr, info, sizeof(info), INFO_TIMEOUT);
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

// Receive commands from hub
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	cmd_ready = 1;
}
