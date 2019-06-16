#ifndef DIS_H
#define DIS_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define DIS_LIMIT_SWITCH_PIN (1 << PD2) //INT0
#define DIS_SPEED 30
#define DIS_DIR 0
#define DIS_HOME_DIR 1
#define DIS_ACCEL_DIFF_THRESHOLD 5000

void dis_limit_switch_init(void);
void dis_dispense_init(uint8_t num_items);
void dis_home_init(void);
static void dis_compare_accel_data(void);
static void dis_done(void);

#endif

