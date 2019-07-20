#include "prox.h"
#include "twi.h"

void prox_init()
{
    twi_init();

    //set IR LED current
    twi_write_reg8(PROX_TWI_ADDR, PROX_IRLED_REG, PROX_IRLED_CURRENT);

    //set prox reading rate
    twi_write_reg8(PROX_TWI_ADDR, PROX_PROXRATE_REG, PROX_PROXRATE_16_625);
}

uint16_t prox_get_data()
{
    //write command register to proximity mode
    twi_write_reg8(PROX_TWI_ADDR, PROX_COMMAND_REG, PROX_MEASUREPROXIMITY);

    while(1)
    {
        uint8_t result = twi_read_reg8(PROX_VCNL_ADDR, PROX_COMMAND_REG);
        if(result & PROX_PROXREADY)
            return twi_read_reg16(PROX_TWI_ADDR, PROX_PROXDATA_REG);
    }
}

