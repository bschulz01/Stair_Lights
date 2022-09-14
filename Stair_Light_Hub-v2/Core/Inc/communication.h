/*
 * communication.h
 *
 *  Created on: Dec 8, 2021
 *      Author: bradleyschulz
 */

#ifndef INC_COMMUNICATION_H_
#define INC_COMMUNICATION_H_

#include <stdint.h>
#include "main.h"

// Error states for communication
typedef enum {
	COMM_OK,
	COMM_ERROR,
	TIMEOUT,
	INDEX_ERROR
} comm_stat_t;

typedef enum {
	TOP,
	BOT
} level_t;

typedef enum {
	EMITTER,
	RECEIVER
} side_t;

typedef enum {
	SEND_SENSOR_DATA = 0b00000001,
	RECALIBRATE = 0b00000010,
	ENABLE_SENSE = 0b00010010,
	DISABLE_SENSE = 0b00011011,
	SET_LED_VALS = 0b00100100,
	CLEAR_LEDS = 0b00001111,
	EXTERN_BOARD = 0b00010000,
	ACK = 0b00101010,
	OBJECT_SENSED = 0b01000101,
	OBJECT_NOT_SENSED = 0b01100000,
	UPDATE_COMPLETE = 0b00010000
} cmd_t;

typedef enum {
	requesting_sense,
	waiting,
	initializing_leds,
	sending_data,
	data_sent,
	idle
} transmit_state_t;


// Definitions for LED control
#define NUM_TOP_LEDS 128
#define NUM_BOT_LEDS 128
// timeout for sending basic commands
#define SEND_TIMEOUT 2
// Max amount of LEDs to update in one transfer
#define MAX_LED_UPDATE 30
// Max amount of time to wait for ack
#define ACK_TIMEOUT 2
// Max number of attempts to transmit LED data
#define MAX_ATTEMPTS 8
// Max amount of time to send data to LEDs
#define LED_UPDATE_TIMEOUT 30
// Max time to wait for sensor values
#define SENSOR_UPDATE_TIMEOUT 5

void resetIT();
comm_stat_t initReception();

void setLEDIndex(uint32_t led, uint8_t red, uint8_t green, uint8_t blue);
void setLEDIndexByLevel(level_t level, uint32_t led, uint8_t red, uint8_t green, uint8_t blue);
void setLED(level_t level, side_t side, uint32_t index, uint8_t red, uint8_t green, uint8_t blue);

comm_stat_t sendCommand(UART_HandleTypeDef* huart, cmd_t cmd);
void pollSensors();
void recalibrate();
void clearLEDs();
void enableSense();
void disableSense();

comm_stat_t sendAnimationIdx(uint8_t animation, uint8_t idx);

uint8_t objectSensed();
void updateLEDs();
comm_stat_t sendLEDVals(level_t level, uint8_t startIndex, uint8_t len);


#endif /* INC_COMMUNICATION_H_ */
