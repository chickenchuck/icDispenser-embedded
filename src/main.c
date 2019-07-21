/*
 * IC Dispenser AVR Program
 * Written by Andrew Hollabaugh, started 1/14/19, rewritten 6/16/19
 */

#include "main.h"
#include "steppers.h"
#include "usart.h"
#include "sel.h"
#include "dis.h"
#include "prox.h"
#include <stdio.h>

int main()
{
    steppers_init();
    prox_init();
    usart_init();
    sel_ir_init();
    dis_limit_switch_init();

    //setup serial output as stream and direct to stdout
    FILE usart_output = FDEV_SETUP_STREAM(usart_putchar, NULL, _FDEV_SETUP_WRITE);
    stdout = &usart_output;
    
    sei();

    printf("IC dispenser ready\n");

    while(1)
    {
        if(dis_is_dispense == 1)
        {
            printf("dde\n");
            dis_wait_for_dispense();
        }

        _delay_ms(1);
    }

    //while(1){printf("%i\n", prox_get_data());}
    return 0;
}
