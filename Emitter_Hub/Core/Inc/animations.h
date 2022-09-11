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
#define NUM_ANIMATIONS      4
// Number of colors for the generateRGB function
#define NUM_COLORS          7
// Number of random numbers per side of the staircase
#define NUM_RANDOM_NUMS     6

#define ANIMATION_2_CYCLE   30
#define ANIMATION_2_DELAY   3
#define ANIMATION_2_TIME    ANIMATION_2_CYCLE * ANIMATION_2_DELAY
#define ANIMATION_2_REPEATS 5

// Animation 3 definitions
#define STEP_TIME           20
#define STEP_LEN            10
#define STEP_GAP            6
#define NUM_STEPS           NUM_LEDS / (STEP_LEN + STEP_GAP)
#define NUM_STEPS_ON        4
#define ANIMATION_3_TIME    STEP_TIME * (NUM_STEPS + NUM_STEPS_ON)

// Animation 4 definitions
#define ANIMATION_4_TIME    64
#define ANIMATION_4_LOOPS   4

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
#endif

// Utility functions
uint16_t generateRandom();
void generateRGB(uint8_t num, uint8_t* r, uint8_t* g, uint8_t* b);

#endif /* INC_ANIMATIONS_H_ */
