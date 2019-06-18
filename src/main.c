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
#include <stdio.h>

int main()
{
    steppers_init();
    USART_init();
    accel_init();
    sel_ir_init();
    dis_limit_switch_init();

    //setup serial output as stream and direct to stdout, has to be in main method for some reason
    FILE usart_output = FDEV_SETUP_STREAM(USART_putchar, NULL, _FDEV_SETUP_WRITE);
    stdout = &usart_output;
    
    sei();
    while(1){}
    return 0;
}
