#include "dis.h"
#include "steppers.h"
#include "accel.h"

uint8_t is_dispense = 0;
uint8_t is_dispense_no_home = 0;
uint8_t is_dis_homing = 0;
uint8_t is_dis_homed = 0;

void dis_limit_switch_init()
{
    //set limit switch pin as input, INT0 pin (external interrupt)
    DDRD &= ~DIS_LIMIT_SWITCH_PIN;

    //turn on internal pullup for limit switch
    PORTD |= DIS_LIMIT_SWITCH_PIN;

    //set INT0 to trigger on any logical change
    EICRA &= ~(1 << ISC01);
    EICRA |= (1 << ISC00);

    //enable INT0
    EIMSK |= (1 << INT0);

    //initially determine if dispenser is homed by checking limit switch
    if(PIND & DIS_LIMIT_SWITCH_PIN) //pin is high = button is not pressed (pin is pulled up)
        is_dis_homed = 1;
    else
        is_dis_homed = 0;
}

/*
 * Initializes a dispense operation
 * num_items - number of ICs to dispense
 */
void dis_dispense_init(uint8_t num_items)
{
    is_dispense = 1;
    steppers_move_dis(DIS_SPEED, DIS_DIR);
    printf("dispense start\n");
    dis_compare_accel_data();
}

void dis_dispense_no_home_init(uint8_t num_items)
{
    is_dispense_no_home = 1;
    dis_dispense_init(num_items);
}

/*
 * Initializes a dispenser homing operation
 */
void dis_home_init()
{
    if(is_dis_homed == 0)
    {
        is_dis_homing = 1;
        steppers_move_dis(DIS_SPEED, DIS_HOME_DIR);
        printf("dispenser homing\n");
    }
    else
        printf("dispenser already homed\n");
}

void dis_compare_accel_data()
{
    uint16_t data = accel_get_data();
    uint16_t last_data = data;

    while(abs(data - last_data) < DIS_ACCEL_DIFF_THRESHOLD)
    {
        last_data = data;
        data = accel_get_data();
        printf("%i\n", data);
    }

    dis_done();
}

void dis_done()
{
    steppers_disable_dis();
    is_dispense = 0;
    is_dis_homed = 0;

    printf("done dispensing\n");

    if(is_dispense_no_home == 0)
        dis_home_init();

    is_dispense_no_home = 0;
}

/*
 * Dispenser limit switch interrupt. Used to stop the dispenser motor when the limit switch is released when the
 * dispenser is homing.
 */
ISR(INT0_vect)
{
    if(PIND & DIS_LIMIT_SWITCH_PIN) //pin is high = rising edge of signal = switch is unpressed
    {
        if(is_dis_homing == 1)
        {
            steppers_disable_dis();
            steppers_disable_sel();
            is_dis_homing = 0;
            is_dis_homed = 1;
            printf("done homing dispenser\n");
        }
    }
    else //pin is low = falling edge of signal = switch is pressed
    {
        is_dis_homed = 0;
        //printf("wer\n");
    }
}

