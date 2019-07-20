#include "twi.h"

void twi_init()
{
    //set twi bit rate register, 100 kHz
    TWBR = (uint8_t)PROX_TWBR_VAL;
}

void twi_start()
{
    //reset control register
    TWCR = 0;

    //send START condition
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

    while(!(TWCR & (1 << TWINT))); //loop until TWINT is set

    if((TWSR & 0xF8) != TWI_START && (TWSR & 0xF8) != TWI_REPEATED_START) //AND TWSR with a mask to only check most significant 5 bits of TWSR and compare to ensure start happened
        printf("I2C error: start condition not sent\n");
}

void twi_stop()
{
    //send STOP condition
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

void twi_write_init(uint8_t twi_addr)
{
    TWDR = (twi_addr << 1); //write SLA_W (slave address and write bit) to TWDR for transmission
    TWCR = (1 << TWINT) | (1 << TWEN); //reset TWINT and enable TWI

    while(!(TWCR & (1 << TWINT))){} //loop until TWINT is set

    if((TWSR & 0xF8) != TWI_MT_SLA_ACK) //AND TWSR with a mask to only check most significant 5 bits of TWSR and compare to ensure ACK was received
        printf("I2C error: ACK not received when sending write address\n");
}

void twi_write_data(uint8_t data)
{
    TWDR = data; //set TWDR to data byte
    TWCR = (1 << TWINT) | (1 << TWEN); //reset TWINT, enable TWI

    while(!(TWCR & (1 << TWINT))){} //loop until TWINT is set

    if((TWSR & 0xF8) != TWI_MT_DATA_ACK) //AND TWSR with a mask to only check most significant 5 bits of TWSR and compare to ensure ACK was received
        printf("I2C error: ACK not received when sending data\n");
}

void twi_read_init(uint8_t twi_addr)
{
    TWDR = (twi_addr << 1) + 1; //write SLA_R (slave address and read bit) to TWDR for transmission
    TWCR = (1 << TWINT) | (1 << TWEN); //reset TWINT, enable TWI

    while(!(TWCR & (1 << TWINT))){} //loop until TWINT is set

    if((TWSR & 0xF8) != TWI_MR_SLA_ACK) //AND TWSR with a mask to only check most significant 5 bits of TWSR and compare to ensure ACK was received
        printf("I2C error: ACK not received when sending receive address\n");
}

uint8_t twi_read_data()
{
    TWCR = (1 << TWINT) | (1 << TWEN); //reset TWINT, enable TWI, send NACK

    while(!(TWCR & (1 << TWINT))){} //loop until TWINT is set

    return TWDR;
}

void twi_write_reg8(uint8_t twi_adddr, uint8_t reg_addr, uint8_t data)
{
    twi_start();
    twi_write_init(twi_addr);
    twi_write_data(reg_addr);
    twi_write_data(data);
    twi_stop();
}

uint8_t twi_read_reg8(uint8_t twi_adddr, uint8_t reg_addr)
{
    twi_start();
    twi_write_init(twi_addr);
    twi_write_data(reg_addr);
    twi_stop();

    twi_start();
    twi_read_init(twi_addr);
    return twi_read_data();
}

uint16_t twi_read_reg16(uint8_t twi_adddr, uint8_t reg_addr)
{
    twi_start();
    twi_write_init(twi_addr);
    twi_write_data(reg_addr);
    twi_stop();

    twi_start();
    twi_read_init(twi_addr);
    uint16_t data = twi_read_data() << 8;
    data |= twi_read_data();
    twi_stop();

    return data;
}

