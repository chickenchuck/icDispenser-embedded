#include "prox.h"

void prox_init()
{
    //set twi bit rate register, 100 kHz
    TWBR = (uint8_t)PROX_TWBR_VAL;

    prox_TWI_start();
    prox_TWI_write_init();
    prox_TWI_write_data(PROX_IRLED);
    prox_TWI_write_data(PROX_IRLED_CURRENT); //set power management register to turn off sleep mode
    prox_TWI_stop();
}

void prox_TWI_start()
{
    //reset control register
    TWCR = 0;

    //send START condition
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

    while(!(TWCR & (1 << TWINT))); //loop until TWINT is set

    if((TWSR & 0xF8) != PROX_TWI_START && (TWSR & 0xF8) != PROX_TWI_REPEATED_START) //AND TWSR with a mask to only check most significant 5 bits of TWSR and compare to ensure start happened
        printf("I2C error: start condition not sent\n");
}

void prox_TWI_stop()
{
    //send STOP condition
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

void prox_TWI_write_init()
{
    TWDR = PROX_TWI_SLA_W; //write slave address and wrtie bit to TWDR for transmission
    TWCR = (1 << TWINT) | (1 << TWEN); //reset TWINT and enable TWI

    while(!(TWCR & (1 << TWINT))){} //loop until TWINT is set

    if((TWSR & 0xF8) != PROX_TWI_MT_SLA_ACK) //AND TWSR with a mask to only check most significant 5 bits of TWSR and compare to ensure ACK was received
        printf("I2C error: ACK not received when sending write address\n");
}

void prox_TWI_write_data(uint8_t data)
{
    TWDR = data; //set TWDR to data byte
    TWCR = (1 << TWINT) | (1 << TWEN); //reset TWINT, enable TWI

    while(!(TWCR & (1 << TWINT))){} //loop until TWINT is set

    if((TWSR & 0xF8) != PROX_TWI_MT_DATA_ACK) //AND TWSR with a mask to only check most significant 5 bits of TWSR and compare to ensure ACK was received
        printf("I2C error: ACK not received when sending data\n");
}

void prox_TWI_read_init()
{
    TWDR = PROX_TWI_SLA_R; //write slave address and read bit to TWDR for transmission
    TWCR = (1 << TWINT) | (1 << TWEN); //reset TWINT, enable TWI

    while(!(TWCR & (1 << TWINT))){} //loop until TWINT is set

    if((TWSR & 0xF8) != PROX_TWI_MR_SLA_ACK) //AND TWSR with a mask to only check most significant 5 bits of TWSR and compare to ensure ACK was received
        printf("I2C error: ACK not received when sending receive address\n");
}

uint8_t prox_TWI_read_data()
{
    TWCR = (1 << TWINT) | /*(1 << TWEA) | */(1 << TWEN); //reset TWINT, enable TWI, send NACK

    while(!(TWCR & (1 << TWINT))){} //loop until TWINT is set

    return TWDR;
}

uint16_t prox_get_data()
{
    prox_TWI_start();
    prox_TWI_write_init();
    prox_TWI_write_data(PROX_MPU6050_PROX_XOUT_H); //write address of XOUT_H register
    prox_TWI_start();
    prox_TWI_read_init();
    uint16_t prox_data = prox_TWI_read_data() << 8; //read upper byte of data from XOUT and shift
    prox_data |= prox_TWI_read_data(); //OR prox_data with lower byte from XOUT, address automatically increments
    prox_TWI_stop();
    //printf("PROX: %i\n", prox_data);
    return prox_data;
}

