#include "accel.h"

void accel_init()
{
    //set bit rate register, 100 kHz
    TWBR = (uint8_t)ACCEL_TWBR_VAL;

    accel_TWI_start();
    accel_TWI_write_init();
    accel_TWI_write_data(ACCEL_MPU6050_PWR_MGMT_1);
    accel_TWI_write_data(0x01); //set power management register to turn off sleep mode
    accel_TWI_stop();
}

void accel_TWI_start()
{
    //reset control register
    TWCR = 0;

    //send START condition
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

    while(!(TWCR & (1 << TWINT))); //loop until TWINT is set

    if((TWSR & 0xF8) != ACCEL_TWI_START && (TWSR & 0xF8) != ACCEL_TWI_REPEATED_START) //AND TWSR with a mask to only check most significant 5 bits of TWSR and compare to ensure start happened
        printf("I2C error: start condition not sent\n");
}

void accel_TWI_stop()
{
    //send STOP condition
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

void accel_TWI_write_init()
{
    TWDR = ACCEL_TWI_SLA_W; //write slave address and wrtie bit to TWDR for transmission
    TWCR = (1 << TWINT) | (1 << TWEN); //reset TWINT and enable TWI

    while(!(TWCR & (1 << TWINT))){} //loop until TWINT is set

    if((TWSR & 0xF8) != ACCEL_TWI_MT_SLA_ACK) //AND TWSR with a mask to only check most significant 5 bits of TWSR and compare to ensure ACK was received
        printf("I2C error: ACK not received when sending write address\n");
}

void accel_TWI_write_data(uint8_t data)
{
    TWDR = data; //set TWDR to data byte
    TWCR = (1 << TWINT) | (1 << TWEN); //reset TWINT, enable TWI

    while(!(TWCR & (1 << TWINT))){} //loop until TWINT is set

    if((TWSR & 0xF8) != ACCEL_TWI_MT_DATA_ACK) //AND TWSR with a mask to only check most significant 5 bits of TWSR and compare to ensure ACK was received
        printf("I2C error: ACK not received when sending data\n");
}

void accel_TWI_read_init()
{
    TWDR = ACCEL_TWI_SLA_R; //write slave address and read bit to TWDR for transmission
    TWCR = (1 << TWINT) | (1 << TWEN); //reset TWINT, enable TWI

    while(!(TWCR & (1 << TWINT))){} //loop until TWINT is set

    if((TWSR & 0xF8) != ACCEL_TWI_MR_SLA_ACK) //AND TWSR with a mask to only check most significant 5 bits of TWSR and compare to ensure ACK was received
        printf("I2C error: ACK not received when sending receive address\n");
}

uint8_t accel_TWI_read_data()
{
    TWCR = (1 << TWINT) | /*(1 << TWEA) | */(1 << TWEN); //reset TWINT, enable TWI, send NACK

    while(!(TWCR & (1 << TWINT))){} //loop until TWINT is set

    return TWDR;
}

uint8_t accel_get_data()
{
    accel_TWI_start();
    accel_TWI_write_init();
    accel_TWI_write_data(ACCEL_MPU6050_ACCEL_XOUT_H); //write address of XOUT_H register
    accel_TWI_start();
    accel_TWI_read_init();
    uint16_t accel_data = accel_TWI_read_data() << 8; //read upper byte of data from XOUT and shift
    accel_data |= accel_TWI_read_data(); //OR accel_data with lower byte from XOUT, address automatically increments
    accel_TWI_stop();
    //printf("ACCEL: %i\n", accel_data);
    return accel_data;
}

