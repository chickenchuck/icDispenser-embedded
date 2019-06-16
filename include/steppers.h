#ifndef STEPPERS_H
#define STEPPERS_H

#include <avr/io.h>

#define STEPPERS_SEL_EN_PIN (1 << 7) //PD7
#define STEPPERS_SEL_DIR_PIN (1 << 0) //PB0
#define STEPPERS_SEL_STEP_PIN (1 << 6) //PD6; OC0A
#define STEPPERS_DIS_EN_PIN (1 << 4) //PD4
#define STEPPERS_DIS_DIR_PIN (1 << 1) //PB1
#define STEPPERS_DIS_STEP_PIN (1 << 3) //PB3; OC2A

void steppers_init(void);
void steppers_move_sel(uint8_t speed, uint8_t dir);
void steppers_hold_sel(void);
void steppers_disable_sel(void);
void steppers_move_dis(uint8_t speed, uint8_t dir);
void steppers_hold_dis(void);
void steppers_disable_dis(void);

extern uint8_t steppers_sel_state;
extern uint8_t steppers_dis_state;

#endif

