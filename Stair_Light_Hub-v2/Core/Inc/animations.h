/*
 * animations.h
 *
 *  Created on: Dec 8, 2021
 *      Author: bradleyschulz
 */

#ifndef INC_ANIMATIONS_H_
#define INC_ANIMATIONS_H_

#include <stdint.h>

#define NUM_ANIMATIONS 4
// Number of colors for the generateRGB function
#define NUM_COLORS 6
// Number of random numbers per side of the staircase
#define NUM_RANDOM_NUMS 6

#define ANIMATION_2_CYCLE 20

// Animation 3 definitions
#define STEP_TIME 6
#define STEP_LEN 10
#define STEP_GAP 6
#define NUM_STEPS NUM_TOP_LEDS / (STEP_LEN + STEP_GAP)
#define NUM_STEPS_ON 4
#define ANIMATION_3_TIME (NUM_STEPS + NUM_STEPS_ON) * STEP_TIME

// Animation 4 definitions
#define ANIMATION_4_TIME 64
#define ANIMATION_4_LOOPS 4


void updateAnimation();

void animation1(int index);
void animation2(int index);
void animation3(int index);
void animation4(int index);

// Utility functions
uint16_t generateRandom();
void generateRGB(uint8_t num, uint8_t* r, uint8_t* g, uint8_t* b);

#endif /* INC_ANIMATIONS_H_ */
