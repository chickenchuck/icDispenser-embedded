/*
 * IC Dispenser AVR Program
 * Written by Andrew Hollabaugh, started 1/14/19, rewritten 6/16/19
 */

#include "main.h"
#include "steppers.h"
#include "usart.h"
#include "sel.h"
#include "dis.h"

int main()
{
    steppers_init();
    usart_init();
    sel_ir_init();
    dis_init();

    //setup serial output as stream and direct to stdout
    FILE usart_output = FDEV_SETUP_STREAM(usart_tx_putchar, NULL, _FDEV_SETUP_WRITE);
    stdout = &usart_output;
    
    sei();

    printf("IC dispenser ready\n");

    while(1)
    {
        usart_rx_check_queue();
        if(dis_is_dispense == 1)
        {
            dis_wait_for_dispense();
        }
        //printf("%i\n", dis_ir_get());
    }

    return 0;
}
