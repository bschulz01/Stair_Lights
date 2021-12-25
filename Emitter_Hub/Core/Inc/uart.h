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

// timeout for sending basic commands
#define SEND_TIMEOUT 2
// Max amount of LEDs to update in one transfer
#define MAX_LED_UPDATE 30
// Max amount of time to wait for ack
#define ACK_TIMEOUT 3
// Max time to wait for LED update info
#define INFO_TIMEOUT 5
// Max number of attempts to transmit LED data
#define MAX_ATTEMPTS 3
// Max amount of time to send data to LEDs
#define LED_UPDATE_TIMEOUT 30
// Max time to wait for sensor values
#define SENSOR_UPDATE_TIMEOUT 10

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


void resetIT();
void receiveIT();
uint8_t cmdReady();
void toggleLED();
uint8_t updateLEDs();
uint8_t getActivation(uint8_t sensor);
void sendACK();
comm_stat_t sendCommand(UART_HandleTypeDef* huart, cmd_t cmd);
HAL_StatusTypeDef processCommand();
void sendAllLEDs();
comm_stat_t sendLEDVals(uint8_t startIndex, uint8_t len);

#endif /* INC_UART_H_ */
