#ifndef STEPPERS_H
#define STEPPERS_H

#include "pins.h"

void steppers_init(void);
void steppers_move_sel(uint8_t speed, uint8_t dir);
void steppers_hold_sel(void);
void steppers_disable_sel(void);
void steppers_move_dis(uint8_t speed, uint8_t dir);
void steppers_hold_dis(void);
void steppers_disable_dis(void);

extern volatile uint8_t steppers_sel_state;
extern volatile uint8_t steppers_dis_state;

#endif

