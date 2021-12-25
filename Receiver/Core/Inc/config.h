/*
 * config.h
 *
 *  Created on: Dec 19, 2021
 *      Author: bradleyschulz
 */

#ifndef INC_CONFIG_H_
#define INC_CONFIG_H_

// Number of sensor boards
#define NUM_BOARDS 4
// Number of this board (from 1 to NUM_BOARDS-1)
// Board index 0 is the receiver hub
#define BOARD_INDEX 2
// Whether or not this board is the last in the line
#define LAST_BOARD NUM_BOARDS - 1 == BOARD_INDEX
//#define REVERSE_ORDER


#endif /* INC_CONFIG_H_ */
