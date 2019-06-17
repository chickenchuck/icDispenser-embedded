#ifndef ACCEL_H
#define ACCEL_H

#include <avr/io.h>
#include <stdio.h>
#include "global_definitions.h"

#define ACCEL_F_SCL 100000UL
#define ACCEL_TWI_PRESCALER 1
#define ACCEL_TWBR_VAL ((((F_CPU / ACCEL_F_SCL) / ACCEL_TWI_PRESCALER) - 16) / 2)
#define ACCEL_TWI_START 0x08
#define ACCEL_TWI_REPEATED_START 0x10
#define ACCEL_TWI_MT_SLA_ACK 0x18
#define ACCEL_TWI_MT_DATA_ACK 0x28
#define ACCEL_TWI_MR_SLA_ACK 0x40
#define ACCEL_TWI_SLA_W 0xD0 //b1101000_0 //MPU6050 address and 0 for WRITE
#define ACCEL_TWI_SLA_R 0xD1 //b1101000_1 //MPU6050 address and 1 for READ
#define ACCEL_MPU6050_PWR_MGMT_1 0x6B
#define ACCEL_MPU6050_ACCEL_XOUT_H 0x3B
#define ACCEL_MPU6050_ACCEL_XOUT_L 0x3C

void accel_init(void);
static void accel_TWI_start(void);
static void accel_TWI_stop(void);
static void accel_TWI_write_init(void);
static void accel_TWI_write_data(uint8_t data);
static void accel_TWI_read_init(void);
static uint8_t accel_TWI_read_data(void);
uint8_t accel_get_data(void);

#endif

