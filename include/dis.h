#ifndef DIS_H
#define DIS_H

#include "pins.h"
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>

#define DIS_SPEED 80 //30
#define DIS_HOME_SPEED 30
#define DIS_DIR 0
#define DIS_HOME_DIR 1
#define DIS_PROX_DIFF_THRESHOLD 18
#define DIS_PROX_STABLE_THRESHOLD 18
#define DIS_PROX_STABLE_NUM 20

void dis_limit_switch_init(void);
void dis_dispense_init(uint8_t num_items);
void dis_dispense_no_home_init(uint8_t num_items);
void dis_home_init(void);
void dis_wait_for_dispense(void);
void dis_wait_for_stable(void);
void dis_done(void);

extern uint8_t dis_is_dispense;

#endif

