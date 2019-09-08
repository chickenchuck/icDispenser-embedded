#ifndef DIS_H
#define DIS_H

#include "pins_board.h"
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>

#define DIS_SPEED 50
#define DIS_HOME_SPEED 30
#define DIS_DIR 0
#define DIS_HOME_DIR 1
#define DIS_IR_BROKEN 0
#define DIS_IR_UNBROKEN 8
#define DIS_IR_STABLE_NUM 100
#define DIS_TOO_FAR_POS 210000UL

void dis_init(void);
void dis_hold(void);
void dis_disable(void);
void dis_dispense_init(uint16_t num_items);
void dis_dispense_no_home_init(uint16_t num_items);
void dis_home_init(void);
uint8_t dis_ir_get(void);
void dis_wait_for_dispense(void);
void dis_wait_for_stable(void);
void dis_done(void);

extern volatile uint8_t dis_is_dispense;
extern volatile uint8_t is_dis_homed;

#endif

