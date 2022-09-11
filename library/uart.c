#include "uart.h"
#include "config.h"
#if (POSITION_TYPE == TYPE_HUB)
#include "utils.h"
#include "animations.h"
#include "WS2812.h"
#endif
#if (POSITION_SIDE == SIDE_RECEIVER)
#include "sensors.h"
#endif

// UFP = upstream facing port
// DFP = downstream facing port

// Only hubs update LEDs
#if (POSITION_TYPE == TYPE_HUB)
// Stores if LEDs should be updated
uint8_t update_leds;
#endif

// Says if command is ready to be processed
uint8_t cmd_ready;

// Stores command received from hub
uint8_t cmd;
// Receier hub gets commmand from emitter hub and sensors
#if (POSITION_SIDE == SIDE_RECEIVER)
uint8_t dfp_cmd;
#endif
// Stores ack from receiver
uint8_t rec;

// Structures to store sensor information
// Only hubs determing on/off state of sensors
#if (POSITION_TYPE == TYPE_HUB)
// Bitmap of sensor activations to send to emitter board
// Comes from utils.h
extern uint8_t sensor_activations[NUM_SENSORS];
// Stores if an object was sensed
uint8_t sense_state;
// Whether or not you are allowed to display sensed data
uint8_t display_sense = 1;
#endif
// Only receivers need actual sensor values
#if (POSITION_SIDE == SIDE_RECEIVER)
// Sensor readings received from sensor boards
uint8_t sensor_buf[NUM_SENSORS];
#endif

// state of user LED on the board
uint8_t led_state = 0;


void abortIT() {
	HAL_UART_Abort_IT(getUFP());
// Only receiver side receives interrupts from downstream boards
#if (POSITION_SIDE == SIDE_RECEIVER)
	HAL_UART_Abort_IT(getDFP());
#endif
}

// initialize polling for data from UFP
void receiveIT() {
	cmd_ready = 0;
	cmd = 0;
	abortIT();
	HAL_UART_Receive_IT(getUFP(), &cmd, 1);
// Only receiver side receives interrupts from downstream boards
// Downstream commands read new sensor values
#if (POSITION_SIDE == SIDE_RECEIVER)
	dfp_cmd = 0;
	HAL_UART_Receive_IT(getDFP(), &dfp_cmd, 1);
#endif
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

#if (POSITION_TYPE == TYPE_HUB)
// Returns whether LEDs should be updated
uint8_t updateLEDs() {
	if (update_leds) {
		update_leds = 0;
		return 1;
	} else {
		return 0;
	}
}
#endif

uint8_t cmdReady() {
	return cmd_ready;
}

// Send ACK to UFP
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

// Insert sensor readings to buffer
#if (POSITION_SIDE == SIDE_RECEIVER)
// Adds this boards readings into the send buffer (buffer 1)
void insertReadingsToBuf() {
	// Get most recent readings
	uint16_t* readings = getLastReadings();
	// Add data to the buffer
	for (int i = 0; i < SENSORS_PER_BOARD; i++) {
		sensor_buf[i] = (uint8_t) (readings[i] / 16);
	}
    #if (POSITION_TYPE == TYPE_HUB)
    // Update sensor activations (in utils.h)
	for (int i = 0; i < NUM_SENSORS; i++) {
		registerReading(i, sensor_buf[i]);
	}
    #endif
}

uint8_t getSensorVal(uint8_t sensor) {
	if (sensor < NUM_SENSORS) {
		return sensor_buf[sensor];
	} else {
		return 0;
	}
}
#endif

/* Command overview
    UPDATE_COMPLETE: LED update is complete
        Emitter hub now gets sensor data from receiver hub
        Hubs process sensor readings to determine if they should light up
    SEND_SENSOR_DATA: Pass along most recent data from sensors
        Emitter hub: returns if an object was sensed or not
        Receiver hub: return all sensor readings 
        * Only for hubs
    SENSOR_UPDATE: Read more data from sensors and pass upstream
        * Only for receivers
    TODO: make these situations reality
*/

// Reads command from hub in order to see if action needs to be taken
HAL_StatusTypeDef processCommand() {
	HAL_StatusTypeDef ret = HAL_OK;
	if (cmd == UPDATE_COMPLETE) {
		sendACK(getUFP());
        #if (POSITION_SIDE == SIDE_EMITTER) 
		// Once update is complete, get sensor values from receiver hub
		ret = HAL_ERROR;
        comm_stat_t dfp_ret = sendCommand(getDFP(), SEND_SENSOR_DATA);
        if (dfp_ret == COMM_OK) {
			ret = HAL_UART_Receive(getDFP(), sensor_activations, sizeof(sensor_activations), SENSOR_UPDATE_TIMEOUT);
		    // Update sense state so it is ready to return to hub when requested
		    // Sensors are activated if any sensor (individual bit) indicates that a sensor was activated
		    sense_state = OBJECT_NOT_SENSED;
		    for (int i = 0; i < NUM_SENSORS; i++) {
		    	if (sensor_activations[i] > 0) {
		    		sense_state = OBJECT_SENSED;
		    	}
		    }
        }
        // Tell receiver hub update is complete
        sendCommand(getDFP(), UPDATE_COMPLETE);
        #endif
        // Hubs then display sense state
        #if (POSITION_TYPE == TYPE_HUB)
		// Update the LED strips if an object was sensed
		if (display_sense && sense_state == OBJECT_SENSED) {
			clearLEDs();
			displaySense();
		    update_leds = 1;
		}
        #endif
		//// Send LED values to receiver hub when update is complete
        //#if (POSITION_SIDE == SIDE_EMITTER) 
        //sendLEDData();
        //#endif       
	} else if (cmd == SEND_SENSOR_DATA) {
        // Receiver side sends last sensed state
        #if (POSITION_SIDE == SIDE_RECEIVER)
            sendACK(getUFP());
            #if (POSITION_TYPE == TYPE_HUB) // Hub sends activations to emitter hub
		    HAL_UART_Transmit(getUFP(), sensor_activations, NUM_SENSORS, SENSOR_UPDATE_TIMEOUT);
            #else // receiver sends sensor buf; NOTE: this should not get used at runtime
		    HAL_UART_Transmit(getUFP(), sensor_buf, NUM_SENSORS, SENSOR_UPDATE_TIMEOUT);
            #endif
        #else
            // Emitter side only sends if an object was sensed
		    // Send last known sense state back to hub
		    ret = HAL_UART_Transmit(getUFP(), &sense_state, sizeof(sense_state), SEND_TIMEOUT);
        #endif
#if (POSITION_SIDE == SIDE_RECEIVER)
	} else if (dfp_cmd == SENSOR_UPDATE) {
		// Send ACK to dfp
		sendACK(getDFP());
		// Get updated sensor values from dfp
		ret = HAL_UART_Receive(getDFP(), sensor_buf, sizeof(sensor_buf), SENSOR_UPDATE_TIMEOUT);
		// Get new readings
		getNewReadings();
		// Scale all the sensor readings
		insertReadingsToBuf();
		// Process sensor values to determine if an object was sensed
        // Only the hub determines sense state
        #if (POSITION_TYPE == TYPE_HUB)
		sense_state = OBJECT_NOT_SENSED;
		for (int i = 0; i < NUM_SENSORS; i++) {
			if (sensor_activations[i] > 0) {
				sense_state = OBJECT_SENSED;
			}
		}
        #else // Non-hub sensor boards pass command to upstream board
		// Send update to downstream board
		comm_stat_t status = sendCommand(getUFP(), SENSOR_UPDATE);
		if (status == COMM_OK) {
			HAL_UART_Transmit(getUFP(), sensor_buf, sizeof(sensor_buf), SENSOR_UPDATE_TIMEOUT);
		}
        #endif
#endif
#if (POSITION_TYPE == TYPE_HUB)         // Hub commands for sensors
	} else if (cmd == RECALIBRATE) {
		sendACK(getUFP());
		// Pass command to downstream boards
        #if (POSITION_SIDE == SIDE_EMITTER)
		sendCommand(getDFP(), cmd);
        #endif
		// recalibrate the sensors by clearing the history
		clearSensorHistory();
	} else if (cmd == ENABLE_SENSE) {
		sendACK(getUFP());
		display_sense = 1;
        #if (POSITION_SIDE == SIDE_EMITTER)
		sendCommand(getDFP(), ENABLE_SENSE);
        #endif
	} else if (cmd == DISABLE_SENSE) {
		sendACK(getUFP());
		display_sense = 0;
        #if (POSITION_SIDE == SIDE_EMITTER)
		sendCommand(getDFP(), DISABLE_SENSE);
        #endif
#endif
#if (POSITION_TYPE == TYPE_HUB)         // Only hubs control LEDs
	} else if (cmd == CLEAR_LEDS) {
		sendACK(getUFP());
		// Pass command to downstream boards
        #if (POSITION_SIDE == SIDE_EMITTER)
		sendCommand(getDFP(), cmd);
        #endif
		// Clear LEDs and update strips
		clearLEDs();
		// Set leds to update
		update_leds = 1;
	} else if (cmd == SET_LED_VALS) {
		// Begin waiting for LED data from UFP
		uint8_t info[2] = {0, 0};
		// Return an ACK to begin handshake
		sendACK(getUFP());
		// Receive info about the data transfer
		HAL_UART_Receive(getUFP(), info, sizeof(info), INFO_TIMEOUT);
    #ifdef ANIMATION_BY_INDEX
        updateAnimation(info[0], info[1]);
        update_leds = 1;
        // Pass led data onto receiver
        #if (POSITION_SIDE == SIDE_EMITTER)
            sendLEDInfo(info[0], info[1]);
        #endif
    #else
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
        // Then would pass data on to downstream board in UPDATE_COMPLETE command
    #endif
#endif
	}
	return ret;
}

#if (POSITION_TYPE == TYPE_HUB)
// Send data to downstream board
// Send all the LED data to downstream receiver board
void sendLEDData() {
#ifndef ANIMATION_BY_INDEX
	int startIndex = 0;
	// Update in batches until all leds have been updated
	while (startIndex < NUM_LEDS) {
		comm_stat_t ret = sendLEDInfo(startIndex, MAX_LED_UPDATE);
		startIndex += MAX_LED_UPDATE;
	}
	// Notify that update is complete
	sendCommand(getDFP(), UPDATE_COMPLETE);
#endif
}

#ifdef ANIMATION_BY_INDEX
/* Synchronize the animations
   Format of transmission:
    First send command that LEDs are being updated
    Then send 2 byte sync information for the animations
        Byte 1: animation number
        Byte 2: index in animation
*/
comm_stat_t sendLEDInfo(uint8_t animation, uint8_t idx) {
	// Format data
	uint8_t data[2] = {animation, idx};
    
	// Send request to update leds
	comm_stat_t status = sendCommand(getDFP(), SET_LED_VALS);
	// If successful, send data
	if (status == COMM_OK) {
	    HAL_UART_Transmit(getDFP(), data, sizeof(data), INFO_TIMEOUT);
	}
    return status;
}
#else
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
comm_stat_t sendLEDInfo(uint8_t startIndex, uint8_t len) {
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
#endif
#endif /* Hub LED commands */

// Sensor boards can initiate a sensor update
#if (POSITION_TYPE == TYPE_SENSOR) 
comm_stat_t initiateSensorUpdate() {
	// Stop receiving interrupts
	HAL_UART_Abort_IT(getUFP());
	HAL_UART_Abort_IT(getDFP());
	// Get new sensor readings
	getNewReadings();
	insertReadingsToBuf();
	// Send update to downstream board
	comm_stat_t status = sendCommand(getUFP(), SENSOR_UPDATE);
	if (status == COMM_OK) {
		HAL_UART_Transmit(getUFP(), sensor_buf, sizeof(sensor_buf), SENSOR_UPDATE_TIMEOUT);
	}
	// Resume receiving interrupts
	receiveIT();
	return status;
}
#endif

// Receive commands from hub
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	cmd_ready = 1;
}
