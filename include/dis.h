#ifndef DIS_H
#define DIS_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>

#define DIS_LIMIT_SWITCH_PIN (1 << PD2) //INT0
#define DIS_SPEED 90 //30
#define DIS_HOME_SPEED 30
#define DIS_DIR 0
#define DIS_HOME_DIR 1
#define DIS_ACCEL_DIFF_THRESHOLD 2000
#define DIS_ACCEL_STABLE_THRESHOLD 1000
#define DIS_ACCEL_STABLE_NUM 3

void dis_limit_switch_init(void);
void dis_dispense_init(uint8_t num_items);
void dis_dispense_no_home_init(uint8_t num_items);
void dis_home_init(void);
void dis_accel_wait_for_dispense(void);
void dis_accel_wait_for_stable(void);
void dis_done(void);

#endif

