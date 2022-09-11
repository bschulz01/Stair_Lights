/*
 * animations.c
 *
 *  Created on: Sep 3, 2022
 *      Author: bradleyschulz
 */


#include "animations.h"
#include "config.h"
#include "main.h"
#include "WS2812.h"

int animation_len[NUM_ANIMATIONS] = {
		NUM_LEDS*2,
		ANIMATION_2_CYCLE*5,
		ANIMATION_3_TIME*3,
        ANIMATION_4_TIME*ANIMATION_4_LOOPS*2
};

int idx_debug = 0;
int num_debug = 0;

// Random numbers to be used in the animations
uint16_t randoms[NUM_RANDOM_NUMS];

uint8_t anim4_color1[3];
uint8_t anim4_color2[3];

// TODO: reset steps at each reset
// TODO: fix random animation

void updateAnimation(uint8_t num, uint8_t idx) {
    idx_debug = idx;
    num_debug = num;
   switch(num) {
    case 0: animation1(idx); break;
    case 1: animation2(idx); break;
    case 2: animation3(idx); break;
    case 3: animation4(idx); break;
    default: break; 
   }
}

/* ANIMATION IDEAS
 * 	Rainbow from the middle out
 * 	Side to side like footsteps
 * 	Expanding dots
 */

// Moving lines
void animation1(uint8_t index) {
	int num = index % (NUM_LEDS*2/3);
    // There are three groups of colors
    // 1st dimension = group
    // 2nd dimension = r, g, or b (respectively)
    uint8_t rgb[3][3] = {{0, 0, 0},{0, 0, 0},{0, 0, 0}};
    // Set colors for each staircase segment
#if (POSITION_LEVEL == LEVEL_TOP)
    #if (POSITION_SIDE  == SIDE_EMITTER)
        rgb[0][0] = 100;
        rgb[1][0] = 100;
        rgb[1][1] = 100;
        rgb[2][0] = 100;
        rgb[2][2] = 100;
    #else
        rgb[0][3] = 250;
        rgb[1][1] = 100;
        rgb[1][2] = 100;
        rgb[2][1] = 100;
    #endif
#else
    #if (POSITION_SIDE  == SIDE_EMITTER)
        rgb[0][0] = 100;
        rgb[0][1] = 100;
        rgb[1][1] = 100;
        rgb[1][2] = 100;
        rgb[2][0] = 100;
        rgb[2][2] = 100;
    #else
        rgb[0][0] = 100;
        rgb[1][1] = 100;
        rgb[1][2] = 100;
        rgb[2][2] = 100;
    #endif
#endif
	// Turn on going up
	if (num < NUM_LEDS/3) {
        // Set colors for 3 groups
        setLED(num                 , rgb[0][0], rgb[0][1], rgb[0][2]);
        setLED(num + NUM_LEDS * 1/3, rgb[1][0], rgb[1][1], rgb[1][2]);
        setLED(num + NUM_LEDS * 2/3, rgb[2][0], rgb[2][1], rgb[2][2]);
	} else {
		// Turn off moving down
        // Shift num to match going down
		num = NUM_LEDS/3 - index % (NUM_LEDS/3);
        setLED(num                 , 0, 0, 0);
        setLED(num + NUM_LEDS * 1/3, 0, 0, 0);
        setLED(num + NUM_LEDS * 2/3, 0, 0, 0);
	}
}

// Random dots that expand outwards
void animation2(uint8_t idx) {
	// Get the number of LEDs to have on in this iteration
	int cycleIdx = (idx  / NUM_RANDOM_NUMS) % ANIMATION_2_CYCLE;
    // Increase then decrease width
    int numOn = cycleIdx > (ANIMATION_2_CYCLE / 2) ? ANIMATION_2_CYCLE - cycleIdx : cycleIdx;
	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;
    clearLEDs();
	// Iterate through each group of LEDs
	for (int i = 0; i < NUM_RANDOM_NUMS; i++) {
		generateRGB(i, &r, &g, &b);
        // Iterate through each led within the group
        for (int j = 0; j < numOn; j++) {
		    setLED(randoms[i]+j, r, g, b);
		    setLED(randoms[i]-j, r, g, b);
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
void animation3(uint8_t index) {
	int step = (index % (ANIMATION_3_TIME)) / STEP_TIME;
	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;
	// Update when a new step is reached
//	if (index % STEP_TIME == 0) {       // commented out because indices may be skipped
		// Turn on new step
		int startIndex = step * (STEP_LEN+STEP_GAP);
        uint8_t side = step % 2 == 0 ? SIDE_EMITTER : SIDE_RECEIVER;
		// Only set steps that are within bounds
		if (startIndex < NUM_LEDS) {
			generateRGB(step, &r, &g, &b);
			for (int i = 0; i < STEP_LEN; i++) {
                if (side == POSITION_SIDE) {
                    #if (POSITION_LEVEL == LEVEL_TOP)
				    setLED(startIndex+i, r, g, b);
                    #else
				    // Bottom side needs to be inverted because indexing is reversed
				    setLED(NUM_LEDS - (startIndex+i), r, g, b);
                    #endif
                }
			}
		}
		// Turn off old step
		int offStep = step - NUM_STEPS_ON;
		if (offStep >= 0) {
			startIndex = offStep * (STEP_LEN+STEP_GAP);
			for (int i = 0; i < STEP_LEN; i++) {
#if (POSITION_LEVEL == LEVEL_TOP)
				setLED(startIndex+i, 0, 0, 0);
#else
				// Bottom side needs to be inverted because indexing is reversed
				setLED(NUM_LEDS - (startIndex+i), 0, 0, 0);
#endif
			}
		}
//	}
}

void animation4(uint8_t index) {
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

    for (int i = 0; i < NUM_LEDS; i += 2) {
        #if (POSITION_SIDE == SIDE_EMITTER)
        setLED(i, anim4_color1[0], anim4_color1[1], anim4_color1[2]);
        #else
        setLED(i, anim4_color2[0], anim4_color2[1], anim4_color2[2]);
        #endif
    }
}

// UTILITY FUNCTIONS

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
    case 6:         // red
        *r = 255;
        *g = 0;
        *b = 0;
        break;
	default: 		// off
		*r = 0;
		*g = 0;
		*b = 0;
		break;
	}
}
