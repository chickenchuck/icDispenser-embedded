#include "prox.h"
#include "twi.h"

void prox_init()
{
    twi_init();

    //set IR LED current
    twi_write_reg(PROX_VCNL_ADDR, PROX_IRLED_REG, PROX_IRLED_CURRENT);
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

