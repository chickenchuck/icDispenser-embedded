#ifndef TWI_H
#define TWI_H

#include "global_definitions.h"
#include <avr/io.h>
#include <stdio.h>

#define TWI_F_SCL 100000UL
#define TWI_PRESCALER 1
#define TWI_TWBR_VAL ((((F_CPU / PROX_F_SCL) / PROX_TWI_PRESCALER) - 16) / 2)
#define TWI_START 0x08
#define TWI_REPEATED_START 0x10
#define TWI_MT_SLA_ACK 0x18
#define TWI_MT_DATA_ACK 0x28
#define TWI_MR_SLA_ACK 0x40

void twi_init(void);
void twi_start(void);
void twi_stop(void);
void twi_write_init(uint8_t twi_addr);
void twi_write_data(uint8_t data);
void twi_read_init(uint8_t twi_addr);
uint8_t twi_read_data(void);
void twi_write_reg(uint8_t twi_addr, uint8_t reg_addr, uint8_t data);
uint8_t twi_read_reg8(uint8_t twi_addr, uint8_t reg_addr);
uint16_t twi_read_reg16(uint8_t twi_addr, uint8_t reg_addr);

#endif

