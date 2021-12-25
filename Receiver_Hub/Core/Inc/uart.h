/*
 * uart.h
 *
 *  Created on: Dec 6, 2021
 *      Author: bradleyschulz
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#include <stdint.h>
#include "main.h"

// Number of downstream sensor boards
#define NUM_CHILDREN 3

// timeout for sending basic commands
#define SEND_TIMEOUT 2
// Max amount of time to wait for ack
#define ACK_TIMEOUT 2
// Max time to wait for LED update info
#define INFO_TIMEOUT 5
// Max number of attempts to transmit LED data
#define MAX_ATTEMPTS 5
// Max amount of time to send data to LEDs
#define LED_UPDATE_TIMEOUT 30
// Max time to wait for sensor values
#define SENSOR_UPDATE_TIMEOUT 75


typedef enum {
	SEND_SENSOR_DATA = 0b00000001,
	SENSOR_UPDATE = 0b00000110,
	ENABLE_SENSE = 0b00010010,
	DISABLE_SENSE = 0b00011011,
	RECALIBRATE = 0b00000010,
	SET_LED_VALS = 0b00000100,
	CLEAR_LEDS = 0b00001111,
	EXTERN_BOARD = 0b00010000,
	ACK = 0b00101010,
	OBJECT_SENSED = 0b01000101,
	OBJECT_NOT_SENSED = 0b01100000,
	UPDATE_COMPLETE = 0b00010000
} cmd_t;

typedef enum {
	idle,
	active
} uart_state_t;

// Error states for communication
typedef enum {
	COMM_OK,
	COMM_ERROR,
	TIMEOUT,
	INDEX_ERROR
} comm_stat_t;


void abortIT();
void receiveIT();
uint8_t cmdReady();
void toggleLED();
void sendACK(UART_HandleTypeDef* huart);
comm_stat_t sendCommand(UART_HandleTypeDef* huart, cmd_t cmd);
uint8_t updateLEDs();
void insertReadings();
uint8_t getSensorVal(uint8_t sensor);
HAL_StatusTypeDef processCommand();

#endif /* INC_UART_H_ */
