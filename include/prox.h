#ifndef PROX_H
#define PROX_H

#include "global_definitions.h"
#include <avr/io.h>
#include <stdio.h>

#define PROX_F_SCL 100000UL
#define PROX_TWI_PRESCALER 1
#define PROX_TWBR_VAL ((((F_CPU / PROX_F_SCL) / PROX_TWI_PRESCALER) - 16) / 2)
#define PROX_TWI_START 0x08
#define PROX_TWI_REPEATED_START 0x10
#define PROX_TWI_MT_SLA_ACK 0x18
#define PROX_TWI_MT_DATA_ACK 0x28
#define PROX_TWI_MR_SLA_ACK 0x40
#define PROX_VCNL_ADDR 0x13
#define PROX_TWI_SLA_W (PROX_VCNL_ADDR << 1) //b0010011_0 //VCNL address and 0 for WRITE
#define PROX_TWI_SLA_R ((PROX_VCNL_ADDR << 1) + 1) //b0010011_1 //MPU6050 address and 1 for READ

//VCNL4010 registers
#define PROX_COMMAND 0x80
#define PROX_PROXRATE 0x82
#define PROX_IRLED 0x83
#define PROX_PROXDATA 0x87

#define PROX_IRLED_CURRENT 20 //200mA

void prox_init(void);
void prox_TWI_start(void);
void prox_TWI_stop(void);
void prox_TWI_write_init(void);
void prox_TWI_write_data(uint8_t data);
void prox_TWI_read_init(void);
uint8_t prox_TWI_read_data(void);
uint16_t prox_get_data(void);

#endif

