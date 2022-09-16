/*
 * animations.h
 *
 *  Created on: Sep 3, 2022
 *      Author: bradleyschulz
 */

#ifndef INC_ANIMATIONS_H_
#define INC_ANIMATIONS_H_

#include <stdint.h>

#ifndef NUM_LEDS
#define NUM_LEDS            128
#endif
#define NUM_ANIMATIONS      5
// Number of colors for the generateRGB function
#define NUM_COLORS          7
// Number of random numbers per side of the staircase
#define NUM_RANDOM_NUMS     3
// Max amplitude for the fluid rgb generation
#define MAX_AMPLITUDE       100

// Animation 2 is random moving dots expanding outwards
#define ANIMATION_2_CYCLE   30
#define ANIMATION_2_DELAY   3
#define ANIMATION_2_TIME    ANIMATION_2_CYCLE * ANIMATION_2_DELAY
#define ANIMATION_2_REPEATS 12

// Animation 3 definitions
// "footstops" moving up the staircase
#define STEP_TIME           20
#define STEP_LEN            10
#define STEP_GAP            6
#define NUM_STEPS           NUM_LEDS / (STEP_LEN + STEP_GAP)
#define NUM_STEPS_ON        4
#define ANIMATION_3_TIME    STEP_TIME * (NUM_STEPS + NUM_STEPS_ON)

// Animation 4 definitions
// RGB waves from center of each flight
#define ANIMATION_4_LOOPS   15
#define ANIMATION_4_RANGE   30
#define ANIMATION_4_DELAY   3
#define ANIMATION_4_WIDTH   10
#define ANIMATION_4_OFFSET1 0
#define ANIMATION_4_OFFSET2 ANIMATION_4_RANGE

// Animation 5 definitions
// RGB fades along entire length of strip
#define ANIMATION_5_TIME    180
#define ANIMATION_5_LOOPS   4
#define ANIMATION_5_START_1 0
#define ANIMATION_5_START_2 ANIMATION_5_TIME / 3

#ifdef IS_CENTRAL_HUB
void updateAnimation();
uint8_t getAnimationNum();
uint8_t getAnimationIdx();
#else
void updateAnimation(uint8_t num, uint8_t idx);
void animation1(uint8_t index);
void animation2(uint8_t index);
void animation3(uint8_t index);
void animation4(uint8_t index);
void animation5(uint8_t index);
#endif

// Utility functions
uint16_t generateRandom();
void generateRGB(uint8_t num, uint8_t* r, uint8_t* g, uint8_t* b);
void generateRGB_fluid(int16_t idx, int16_t colorRange, uint8_t* r, uint8_t* g, uint8_t* b);

#endif /* INC_ANIMATIONS_H_ */
