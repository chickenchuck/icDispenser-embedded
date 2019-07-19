#ifndef PROX_H
#define PROX_H

#include <avr/io.h>
#include <stdio.h>

#define PROX_TWI_ADDR 0x13

//VCNL4010 registers
#define PROX_COMMAND_REG 0x80
#define PROX_PROXRATE_REG 0x82
#define PROX_IRLED_REG 0x83
#define PROX_PROXDATA_REG 0x87

#define PROX_IRLED_CURRENT 20 //200mA
#define PROX_PROXRATE_16_625 3

//command register values
#define PROX_MEASUREPROXIMITY 0x08
#define PROX_PROXREADY 0x20

void prox_init(void);
uint16_t prox_get_data(void);

#endif

