/*
 * config.h
 *
 *  Created on: Dec 19, 2021
 *      Author: bradleyschulz
 */

#ifndef INC_CONFIG_H_
#define INC_CONFIG_H_

// Position definitions (numerical values are arbitrary)
#define SIDE_EMITTER    1
#define SIDE_RECEIVER   2
#define LEVEL_TOP       3
#define LEVEL_BOTTOM    4
#define TYPE_HUB        5
#define TYPE_SENSOR     6

// Set definition of this board instance
#define POSITION_SIDE   SIDE_RECEIVER
#define POSITION_LEVEL  LEVEL_TOP 
#define POSITION_TYPE   TYPE_SENSOR

// Number of sensor boards
#define NUM_BOARDS 4
#define SENSORS_PER_BOARD 10
// Number of this board (from 1 to NUM_BOARDS-1)
// Board index 0 is the receiver hub
#define BOARD_INDEX 2
// Whether or not this board is the last in the line
#define LAST_BOARD NUM_BOARDS - 1 == BOARD_INDEX
//#define REVERSE_ORDER

// If defined, send only led index and let hubs calculate updates
#define ANIMATION_BY_INDEX


#endif /* INC_CONFIG_H_ */
