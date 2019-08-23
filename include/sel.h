#ifndef SEL_H
#define SEL_H

#include "pins.h"
#include <avr/interrupt.h>
#include <stdio.h>

#define SEL_MOVE_SPEED 100 //165
#define SEL_MOVE_SPEED_SLOW 250
#define SEL_HOME_SPEED 120 //180
#define SEL_HOME_SPEED_SLOW 250

#define SEL_DIR 1
#define SEL_HOME_POS_COUNT_THRESHOLD 700 //minimum number of microsteps for how long the IR sensor detects for homing

void sel_ir_init(void);
void sel_set_max_index(uint8_t num_tubes);
void sel_home_init(void);
void sel_move_init(uint8_t destination_index);
void sel_move_next_init(void);

extern uint8_t sel_index;

#endif

