/*
 * animations.c
 *
 *  Created on: Dec 8, 2021
 *      Author: bradleyschulz
 */


#include "animations.h"
#include "communication.h"
#include "main.h"

int animation_len[NUM_ANIMATIONS] = {
		NUM_TOP_LEDS*2,
		ANIMATION_2_CYCLE*5,
		ANIMATION_3_TIME*3,
		ANIMATION_4_TIME*ANIMATION_4_LOOPS*2
};
int animation_idx;
int animation_num;

// Random numbers to be used in the animations
uint16_t randoms1[NUM_RANDOM_NUMS];
uint16_t randoms2[NUM_RANDOM_NUMS];

uint8_t anim4_color1[3];
uint8_t anim4_color2[3];

void updateAnimation() {
	animation_idx++;
	if (animation_idx > animation_len[animation_num]) {
		animation_idx = 0;
		animation_num++;
		if (animation_num >= NUM_ANIMATIONS) {
			animation_num = 0;
		}
		clearLEDs();
	}
	switch(animation_num) {
	case 0: animation1(animation_idx); break;
	case 1: animation2(animation_idx); break;
	case 2: animation2(animation_idx); break;
	case 3: animation4(animation_idx); break;
	}
}

/* ANIMATION IDEAS
 * 	Rainbow from the middle out
 * 	Side to side like footsteps
 * 	Expanding dots
 */

// Moving lines
void animation1(int index) {
	int num = index % (NUM_TOP_LEDS*2/3);
	// Turn on going up
	if (num < NUM_TOP_LEDS/3) {
		setLED(TOP, EMITTER, num, 100, 0, 0);
		setLED(TOP, RECEIVER, num, 0, 0, 250);
		setLED(TOP, EMITTER, num+NUM_TOP_LEDS/3, 100, 100, 0);
		setLED(TOP, RECEIVER, num+NUM_TOP_LEDS/3, 0, 100, 100);
		setLED(TOP, EMITTER, num+NUM_TOP_LEDS*2/3, 100, 0, 100);
		setLED(TOP, RECEIVER, num+NUM_TOP_LEDS*2/3, 0, 100, 0);

		setLED(BOT, EMITTER, num, 100, 100, 0);
		setLED(BOT, RECEIVER, num, 100, 0, 0);
		setLED(BOT, EMITTER, num+NUM_BOT_LEDS/3, 0, 100, 100);
		setLED(BOT, RECEIVER, num+NUM_BOT_LEDS/3, 0, 100, 0);
		setLED(BOT, EMITTER, num+NUM_BOT_LEDS*2/3, 100, 0, 100);
		setLED(BOT, RECEIVER, num+NUM_BOT_LEDS*2/3, 0, 0, 100);
	} else {
		num = NUM_TOP_LEDS/3 - index % (NUM_TOP_LEDS/3);
		// Turn off moving down
		setLED(TOP, EMITTER, num, 0, 0, 0);
		setLED(TOP, RECEIVER, num, 0, 0, 0);
		setLED(TOP, EMITTER, num+NUM_TOP_LEDS/3, 0, 0, 0);
		setLED(TOP, RECEIVER, num+NUM_TOP_LEDS/3, 0, 0, 0);
		setLED(TOP, EMITTER, num+NUM_TOP_LEDS*2/3, 0, 0, 0);
		setLED(TOP, RECEIVER, num+NUM_TOP_LEDS*2/3, 0, 0, 0);

		setLED(BOT, EMITTER, num, 0, 0, 0);
		setLED(BOT, RECEIVER, num, 0, 0, 0);
		setLED(BOT, EMITTER, num+NUM_BOT_LEDS/3, 0, 0, 0);
		setLED(BOT, RECEIVER, num+NUM_BOT_LEDS/3, 0, 0, 0);
		setLED(BOT, EMITTER, num+NUM_BOT_LEDS*2/3, 0, 0, 0);
		setLED(BOT, RECEIVER, num+NUM_BOT_LEDS*2/3, 0, 0, 0);
	}
}

// Random dots that expand outwards
void animation2(int index) {
	// Get the number of LEDs to have on in this iteration
	int numOn = index % ANIMATION_2_CYCLE;
	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;
	// Generate new random numbers if starting the animation
	if (numOn == 0) {
		clearLEDs();
		for (int i = 0; i < NUM_RANDOM_NUMS; i++) {
			randoms1[i] = generateRandom() % (2*NUM_TOP_LEDS);
			randoms2[i] = generateRandom() % (2*NUM_BOT_LEDS);
			generateRGB(i, &r, &g, &b);
			setLEDIndexByLevel(TOP, randoms1[i], r, g, b);
			setLEDIndexByLevel(BOT, randoms2[i], r, g, b);
		}
	} else {
		// Turn on surrounding LEDS one by one
		for (int i = 0; i < NUM_RANDOM_NUMS; i++) {
			generateRGB(i, &r, &g, &b);
			// Ensure it doesn't bleed over between sides
//			if (randoms1[i] < NUM_TOP_LEDS ^ randoms1[i] + numOn > NUM_TOP_LEDS) {
				setLEDIndexByLevel(TOP, randoms1[i]+numOn, r, g, b);
//			}
//			if (randoms1[i] >= NUM_TOP_LEDS ^ randoms1[i] - numOn < NUM_TOP_LEDS) {
				setLEDIndexByLevel(TOP, randoms1[i]-numOn, r, g, b);
//			}
//			if (randoms2[i] < NUM_BOT_LEDS ^ randoms2[i] + numOn > NUM_BOT_LEDS) {
				setLEDIndexByLevel(BOT, randoms2[i]+numOn, r, g, b);
//			}
//			if (randoms2[i] >= NUM_BOT_LEDS ^ randoms2[i] - numOn < NUM_BOT_LEDS) {
				setLEDIndexByLevel(BOT, randoms2[i]-numOn, r, g, b);
//			}
		}
	}
}

// "Footsteps" going up
/*
 * Steps are 10 LEDs long with a 6 led buffer in between
 * This means that there are 8 steps
 * There are at most 4 steps visible at any point in time
 * Each step is visible for ANIMATION_3_STEP_TIME
 */
void animation3(int index) {
	int step = (index % (ANIMATION_3_TIME)) / STEP_TIME;
	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;
	// Update when a new step is reached
	if (index % STEP_TIME == 0) {
		// Turn on new step
		int startIndex = step * (STEP_LEN+STEP_GAP);
		side_t side = step % 2 == 0 ? EMITTER : RECEIVER;
		// Only set steps that are within bounds
		if (startIndex < NUM_TOP_LEDS) {
			generateRGB(step, &r, &g, &b);
			for (int i = 0; i < STEP_LEN; i++) {
				setLED(TOP, side, startIndex+i, r, g, b);
				// Bottom side needs to be inverted because indexing is reversed
				setLED(BOT, side, NUM_BOT_LEDS - (startIndex+i), r, g, b);
			}
		}
		// Turn off old step
		int offStep = step - NUM_STEPS_ON;
		if (offStep >= 0) {
			startIndex = offStep * (STEP_LEN+STEP_GAP);
			for (int i = 0; i < STEP_LEN; i++) {
				setLED(TOP, side, startIndex+i, 0, 0, 0);
				// Bottom side needs to be inverted because indexing is reversed
				setLED(BOT, side, NUM_BOT_LEDS - (startIndex+i), 0, 0, 0);
			}
		}
	}
}

void animation4(int index) {
	int step = index % (ANIMATION_4_TIME * 2);
	int base1[3] = {48, 176, 240};
	int base2[3] = {255, 0, 0};
	int c1;
	int c2;
	int b1;
	int b2;

	if (index == 1) {
		clearLEDs();
		for (int i = 0; i < 3; i++) {
			anim4_color1[i] = base1[i];
			anim4_color2[i] = base2[i];
		}
	}
	else {
		// calculate the difference between the two colors
		for (int i = 0; i < 3; i++) {
			c1 = anim4_color1[i];
			c2 = anim4_color2[i];
			if (step < ANIMATION_4_TIME) {
				b1 = base1[i];
				b2 = base2[i];
			}
			else {
				b1 = base2[i];
				b2 = base1[i];
			}

			if (c1 < b2) {
				c1 = c1 + 4;
				if (c1 > 255) {
					c1 = 255;
				}
			} else if (c1 > b2) {
				c1 = c1 - 4;
				if (c1 < 0) {
					c1 = 0;
				}
			} else {
				c1 = b2;
			}

			if (c2 < b1) {
				c2 = c2 + 4;
				if (c2 > 255) {
					c2 = 255;
				}
			} else if (c2 > b1) {
				c2 = c2 - 4;
				if (c2 < 0) {
					c2 = 0;
				}
			} else {
				c2 = b1;
			}

			anim4_color1[i] = c1;
			anim4_color2[i] = c2;
		}
	}

	for (int i = 0; i < NUM_TOP_LEDS; i += 2) {
		setLED(TOP, EMITTER, i, anim4_color1[0], anim4_color1[1], anim4_color1[2]);
		setLED(TOP, RECEIVER, i, anim4_color2[0], anim4_color2[1], anim4_color2[2]);
	}
	for (int i = 0; i < NUM_BOT_LEDS; i += 2) {
		setLED(BOT, EMITTER, i, anim4_color2[0], anim4_color2[1], anim4_color2[2]);
		setLED(BOT, RECEIVER, i, anim4_color1[0], anim4_color1[1], anim4_color1[2]);
	}
}



// UTILITy FUNCTIONS

// Generate a random 16 bit unsigned interger
uint16_t generateRandom() {
	// Generate random 32 bit number
	uint32_t num;
	HAL_RNG_GenerateRandomNumber(getRNG(), &num);
	// Randomly take either the upper or lower 16 bits
	// Extracts the upper 16 bits
	uint32_t mask = (~0) >> 16;
	if (num >> 31) {
		num = num & mask;
	} else {
		num = num >> 16;
	}
	return (uint16_t) num;
}

// Sets the rgb value given a number
void generateRGB(uint8_t num, uint8_t* r, uint8_t* g, uint8_t* b) {
	int color = num % NUM_COLORS;
	switch (color) {
	case 0: 		// purple
		*r = 100;
		*g = 0;
		*b = 100;
		break;
	case 1: 		// orange
		*r = 150;
		*g = 40;
		*b = 70;
		break;
	case 2: 		// yellow
		*r = 100;
		*g = 100;
		*b = 0;
		break;
	case 3: 		// green
		*r = 0;
		*g = 150;
		*b = 0;
		break;
	case 4: 		// cyan
		*r = 0;
		*g = 100;
		*b = 100;
		break;
	case 5: 		// white
		*r = 70;
		*g = 70;
		*b = 70;
		break;
	case 6:			// red
		*r = 255;
		*g = 0;
		*b = 0;
	default: 		// off
		*r = 0;
		*g = 0;
		*b = 0;
		break;
	}
}
