/*
 * uart.c
 *
 *  Created on: Dec 6, 2021
 *      Author: bradleyschulz
 */

#include "uart.h"
#include "main.h"
#include "config.h"
#include "sensors.h"

// RECEIVER VERSION

// UFP = upstream facing port
// DFP = downstream facing port

// Buffer length definitions
// Defined here because they are dependent on definitions in other files
#define SENSOR_BUF_LEN NUM_BOARDS * NUM_SENSORS

// Stores command received from UFP
uint8_t ufp_cmd;
// Stores command received from DFP
uint8_t dfp_cmd;
// Stores if a new command is ready to be processed
uint8_t cmd_ready;
uint8_t sensor_buf[SENSOR_BUF_LEN];


void abortIT() {
	HAL_UART_Abort_IT(getUFP());
	HAL_UART_Abort_IT(getDFP());
}

// initialize polling for receive interrupts
void receiveIT() {
	ufp_cmd = 0;
	dfp_cmd = 0;
	cmd_ready = 0;
	HAL_UART_Receive_IT(getUFP(), &ufp_cmd, sizeof(ufp_cmd));
	HAL_UART_Receive_IT(getDFP(), &dfp_cmd, sizeof(dfp_cmd));
//	HAL_UART_Receive_IT(getDFP(), sensor_buf, SENSOR_BUF_LEN);
}

uint8_t cmdReady() {
	return cmd_ready;
}

// Send ACK to UFP
void sendACK(UART_HandleTypeDef* huart) {
	uint8_t data = ACK;
	HAL_UART_Transmit(huart, &data, 1, SEND_TIMEOUT);
}

// Adds this boards readings into the send buffer (buffer 1)
void insertReadings() {
	// Get most recent readings
	uint16_t* readings = getLastReadings();
	int startIndex = NUM_SENSORS * BOARD_INDEX;
	// Add data to the buffer
	for (int i = 0; i < NUM_SENSORS; i++) {
		sensor_buf[startIndex + i] = (uint8_t) (readings[i] >> 4);
	}
}

// Reads command from hub in order to see if action needs to be taken
void processCommand() {

	if (dfp_cmd == SENSOR_UPDATE) {
		// Send ack to receive updated sensor readings
		sendACK(getDFP());
		HAL_UART_Receive(getDFP(), sensor_buf, sizeof(sensor_buf), SENSOR_UPDATE_TIMEOUT);
		// Get new sensor readings
		getNewReadings();
		// Add this sensor's readings
		insertReadings();
		// Send update to downstream board
		comm_stat_t status = sendCommand(getUFP(), SENSOR_UPDATE);
		if (status == COMM_OK) {
			HAL_UART_Transmit(getUFP(), sensor_buf, sizeof(sensor_buf), SENSOR_UPDATE_TIMEOUT);
		}
	}
}

comm_stat_t initiateSensorUpdate() {
	// Stop receiving interrupts
	HAL_UART_Abort_IT(getUFP());
	HAL_UART_Abort_IT(getDFP());
	// Get new sensor readings
	getNewReadings();
	insertReadings();
	// Send update to downstream board
	comm_stat_t status = sendCommand(getUFP(), SENSOR_UPDATE);
	if (status == COMM_OK) {
		HAL_UART_Transmit(getUFP(), sensor_buf, sizeof(sensor_buf), SENSOR_UPDATE_TIMEOUT);
	}
	// Resume receiving interrupts
	receiveIT();
	return status;
}

comm_stat_t sendCommand(UART_HandleTypeDef* huart, cmd_t cmd) {
	// Retry if needed, then give up after too many attempts
	int numAttempts = 0;
	uint8_t rec = 0;
	while(numAttempts < MAX_ATTEMPTS) {
		// Transmit command
		HAL_UART_Transmit(huart, &cmd, sizeof(cmd), SEND_TIMEOUT);
		// Wait for ACK
		HAL_UART_Receive(huart, &rec, sizeof(rec), ACK_TIMEOUT);
		if (rec == ACK) {
			return COMM_OK;
			break;
		} else {
			numAttempts++;
//			HAL_Delay(1);
		}
	}
	if (rec != ACK) {
		return COMM_ERROR;
	} else {
		return COMM_OK;
	}
}

// Pass received data through
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	cmd_ready = 1;
}
