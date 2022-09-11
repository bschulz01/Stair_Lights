# Stair_Lights
This repository contains code for the infrared sensing stair lights. 
There are 4 sets of firmware for this system:
- Central hub
- IR Emitter side controllers
- IR Receiver side main controllers
- IR Receiver modules 

The modules are all wired together in series.
The hub communicates with both emitter controllers, 
which then talk to the receiver controllers,
which then talk to the receiver modules (in series).

## Repsitory structure
Code for uart, animations, and utils are in the `library` folder.
These functions are used by all sides except the hub,
and the individual directories are hard linked to the library folder.
Macros in the files compile code only relevant to the project
and definitions in each project's `config.h` file sets the behavior.
The central hub uses the macro `IS_CENTRAL_HUB` in its main.h 
to set the behavior of `animations.*` within the central hub code.

## Central Hub
The central hub manages the animations and is responsible for telling the
top and bottom side to update based on the most recent sensor readings.
The sensor readings do not actually go to the central hub.

To update animations, it sends a sync signal to the emitter hubs.
This sync signal sets the animation number and index for that animation.

## Emitter Code
The emitter controllers are responsible for controlling the WS2812 LEDs
on the emitter side of the system and also displaying sensed positions
if an object was sensed. 
It also passes required data along to the receiver side.

## Receiver Hub Code
The receiver hub controls the receiver side LEDs manages the sensor readings.
It reads all the sensor readings and determines which sensor are activated.
Once a sensor is activated, it remains activated for a certain amount of time
to smooth out the readings.
It gets readings from the downstream receiver modules, but only displays them
once an update is required from an upstream board (the emitter)

## Receiver Code
The receivers gather sensor data and pass it along to the next upstream board
once it's readings are updated. It is prompted to get new readings from downstream
boards, and the board at the end of the line is responsible for initiating new 
update cycles.

To update the different boards, alter the parameters in config.h
BOARD_INDEX defines the position of the board in the system.
REVERSE_INDEX reverses the mapping of the sensors 
because the top and bottom of the system are mirrors of each other
