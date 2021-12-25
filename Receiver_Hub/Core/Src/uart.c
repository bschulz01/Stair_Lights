/*
 * uart.c
 *
 *  Created on: Dec 6, 2021
 *      Author: bradleyschulz
 */

#include "uart.h"
#include "WS2812.h"
#include "sensors.h"
#include "utils.h"

// RECEIVER HUB VERSION

// UFP = upstream facing port
// DFP = downstream facing port

// Stores if LEDs should be updated
uint8_t update_leds;

// Stores if a new command is ready to be processed
uint8_t cmd_ready;

uint8_t cmd;
uint8_t dfp_cmd;
// Sensor readings received from sensor boards
uint8_t sensor_buf[NUM_SENSORS];
// Bitmap of sensor activations to send to emitter board
extern uint8_t sensor_activations[NUM_SENSORS];
// Stores if an object was sensed
uint8_t sense_state;
// Whether or not you are allowed to display sensed data
uint8_t display_sense;

uint8_t led_state;

// Returns whether LEDs should be updated
uint8_t updateLEDs() {
	if (update_leds) {
		update_leds = 0;
		return 1;
	} else {
		return 0;
	}
}

void abortIT() {
	HAL_UART_Abort_IT(getUFP());
	HAL_UART_Abort_IT(getDFP());
}

// initialize polling for data from UFP
void receiveIT() {
	cmd_ready = 0;
	cmd = 0;
	dfp_cmd = 0;
	abortIT();
	HAL_UART_Receive_IT(getUFP(), &cmd, 1);
	HAL_UART_Receive_IT(getDFP(), &dfp_cmd, 1);
//	HAL_UART_Receive_IT(getDFP(), sensor_buf, SENSOR_BUF_LEN);
}

uint8_t cmdReady() {
	return cmd_ready;
}

// Send ACK
void sendACK(UART_HandleTypeDef* huart) {
	uint8_t data = ACK;
	HAL_UART_Transmit(huart, &data, 1, SEND_TIMEOUT);
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

// Adds this boards readings into the send buffer (buffer 1)
void registerReadings() {
	// Get most recent readings
	uint16_t* readings = getLastReadings();
	// Add data to the buffer
	for (int i = 0; i < SENSORS_PER_BOARD; i++) {
		sensor_buf[i] = (uint8_t) (readings[i] / 16);
	}
	for (int i = 0; i < NUM_SENSORS; i++) {
		registerReading(i, sensor_buf[i]);
	}
}


uint8_t getSensorVal(uint8_t sensor) {
	if (sensor < NUM_SENSORS) {
		return sensor_buf[sensor];
	} else {
		return 0;
	}
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

// Reads command from hub in order to see if action needs to be taken
HAL_StatusTypeDef processCommand() {
	HAL_StatusTypeDef ret = HAL_OK;
	if (cmd == UPDATE_COMPLETE) {
		toggleLED();
		// Update the LED strips if an object was sensed
		if (display_sense && sense_state == OBJECT_SENSED) {
			clearLEDs();
			displaySense();
		}
		update_leds = 1;
	} else if (cmd == SEND_SENSOR_DATA) {
		// Send data once ACK is received
		HAL_UART_Transmit(getUFP(), sensor_activations, NUM_SENSORS, SENSOR_UPDATE_TIMEOUT);
		display_sense = 1;
	} else if (dfp_cmd == SENSOR_UPDATE) {
		// Send ACK to dfp
		sendACK(getDFP());
		// Get updated sensor values from dfp
		HAL_UART_Receive(getDFP(), sensor_buf, sizeof(sensor_buf), SENSOR_UPDATE_TIMEOUT);
		// Get new readings
		getNewReadings();
		// Scale all the sensor readings
		registerReadings();

		// Process sensor values to determine if an object was sensed
		sense_state = OBJECT_NOT_SENSED;
		for (int i = 0; i < NUM_SENSORS; i++) {
			if (sensor_activations[i] > 0) {
				sense_state = OBJECT_SENSED;
			}
		}
	} else if (cmd == ENABLE_SENSE) {
		sendACK(getUFP());
		display_sense = 1;
	} else if (cmd == DISABLE_SENSE) {
		sendACK(getUFP());
		display_sense = 0;
	} else if (cmd == RECALIBRATE) {
		// Send ACK to UFP
		sendACK(getUFP());
		// recalibrate the sensors
		clearSensorHistory();
	} else if (cmd == CLEAR_LEDS) {
		// Send ACK to UFP
		sendACK(getUFP());
		// Pass data to downstream port
		sendCommand(getDFP(), cmd);
		// Clear LEDs and update strips
		clearLEDs();
		update_leds = 1;
	} else if (cmd == SET_LED_VALS) {
		// Begin waiting for LED data from UFP
		uint8_t info[2] = {0, 0};
		// Return an ACK to begin handshake
		sendACK(getUFP());
		// Receive info about the data transfer
		HAL_UART_Receive(getUFP(), info, sizeof(info), INFO_TIMEOUT);
		int led = info[0];
		int bytes = info[1]*3;
		// Send ack back to UFP to begin transmission of data
		sendACK(getUFP());
		// Get data
		uint8_t* ledBufPtr = getLEDBuf(led);
		ret = HAL_UART_Receive(getUFP(), ledBufPtr, bytes, LED_UPDATE_TIMEOUT);
		if (ret != HAL_OK) {
			return ret;
		}
	}
	return ret;
}

// Pass received data through
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	cmd_ready = 1;
}
