/*
 * IC Dispenser AVR Program
 * Written by Andrew Hollabaugh, started 1/14/19, rewritten 6/16/19
 */

#include "main.h"
#include "steppers.h"
#include "USART.h"
#include "accel.h"
#include "sel.h"
#include "dis.h"

int main()
{
    steppers_init();
    USART_init();
    accel_init();
    sel_ir_init();
    dis_limit_switch_init();
    
    sei();
    while(1){}
    return 0;
}
