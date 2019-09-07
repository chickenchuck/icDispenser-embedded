//dis.c - dispensing functionality

#include "dis.h"
#include "sel.h"
#include "steppers.h"
#include "usart.h"

volatile uint8_t dis_is_dispense = 0;
volatile uint8_t is_dispense_no_home = 0;
volatile uint8_t is_dis_homing = 0;
volatile uint8_t is_dis_homed = 0;
volatile uint16_t items_left_to_dispense = 0;
volatile uint64_t dispense_pos = 0;

void dis_init()
{
    //set limit switch pin as input, INT0 pin (external interrupt)
    DIS_LS_DDR &= ~(1 << DIS_LS_PIN);

    //turn on internal pullup for limit switch
    DIS_LS_PORT |= (1 << DIS_LS_PIN);

    //set INT0 to trigger on any logical change
    EICRA &= ~(1 << ISC01);
    EICRA |= (1 << ISC00);

    //enable INT0
    EIMSK |= (1 << INT0);

    //initially determine if dispenser is homed by checking limit switch
    if(DIS_LS_PIN_REG & (1 << DIS_LS_PIN)) //pin is high = button is not pressed (pin is pulled up)
        is_dis_homed = 1;
    else
        is_dis_homed = 0;

    //set ir pin as input
    DIS_IR_DDR &= ~(1 << DIS_IR_PIN);

    //turn on internal pullup for ir
    DIS_IR_PORT |= (1 << DIS_IR_PIN);
}

void dis_hold()
{
    steppers_hold_dis();
    printf("enabled dispenser\n");
}

void dis_disable()
{
    steppers_disable_dis();
    dis_is_dispense = 0;
    printf("disabled dispenser\n");
}

/*
 * Initializes a dispense operation
 * num_items - number of ICs to dispense
 */
void dis_dispense_init(uint16_t num_items)
{
    if(!(DIS_LS_PIN_REG & (1 << DIS_LS_PIN))) //pin is low = falling edge of signal = switch is pressed = not homed
        printf("error: dis not homed\n");
    else if(num_items <= 0)
        printf("error: not a valid number of ICs\n");
    else
    {
        items_left_to_dispense = num_items;
        dispense_pos = 0;
        dis_is_dispense = 1;
        TIMSK2 |= (1 << TOIE2); //enable overflow interrupt for timer2, for counting steps
        steppers_move_dis(DIS_SPEED, DIS_DIR);
        printf("dispense start, %i items\n", num_items);
    }
}

void dis_dispense_no_home_init(uint16_t num_items)
{
    is_dispense_no_home = 1;
    dis_dispense_init(num_items);
}

/*
 * Initializes a dispenser homing operation
 */
void dis_home_init()
{
    if(!is_dis_homed)
    {
        is_dis_homing = 1;
        steppers_move_dis(DIS_HOME_SPEED, DIS_HOME_DIR);
        printf("dispenser homing\n");
    }
    else
        printf("dispenser already homed\n");
}

uint8_t dis_ir_get()
{
    return DIS_IR_PIN_REG & (1 << DIS_IR_PIN);
}

/*
 * Loops until dispenser ir beam is broken. While looping checks command queue, checks if dispenser went too far,
 * and if dis_is_dispense is low (another routine changed it to disable dispense)
 */
void dis_wait_for_dispense()
{
    while(dis_ir_get() == DIS_IR_UNBROKEN)
    {
        usart_rx_check_queue();
        if(dispense_pos >= DIS_TOO_FAR_POS)
        {
            steppers_disable_dis();
            dis_done();
            printf("error: dispenser went too far!\n");
        }
        if(!dis_is_dispense)
        {
            printf("wait_for_dispense exited\n");
            return;
        }
    }

    steppers_disable_dis();
    
    items_left_to_dispense--;
    printf("dispense, %i ICs left\n", items_left_to_dispense);
    if(items_left_to_dispense == 0)
        dis_done();
    else
        dis_wait_for_stable();
}

/*
 * Loops, checking ir beam state and counts until beam is unbroken for DIS_IR_STABLE_NUM times. While looping, checks
 * serial queue and dis_is_dispense
 */
void dis_wait_for_stable()
{
    uint8_t stable_count = 0;

    while(stable_count < DIS_IR_STABLE_NUM)
    {
        usart_rx_check_queue();
        if(dis_ir_get() == DIS_IR_UNBROKEN)
            stable_count++;
        else
            stable_count = 0;
        
        //printf("dis_stable: %i\n", data);

        if(!dis_is_dispense)
        {
            printf("wait_for_stable exited\n");
            return;
        }
    }
    
    printf("ir now stable\n");

    steppers_move_dis(DIS_SPEED, DIS_DIR);
    dis_wait_for_dispense();
}

//Resets things after dispense operation is done
void dis_done()
{
    dis_is_dispense = 0;
    is_dis_homed = 0;
    TIMSK2 &= ~(1 << TOIE2); //disable overflow interrupt for timer2

    printf("done dispensing\n");

    if(!is_dispense_no_home)
        dis_home_init();

    is_dispense_no_home = 0;
}

/*
 * Dispenser limit switch interrupt. Used to stop the dispenser motor when the limit switch is released when the
 * dispenser is homing.
 */
ISR(INT0_vect)
{
    //only done homing when switch is unpressed
    if(DIS_LS_PIN_REG & (1 << DIS_LS_PIN)) //pin is high = rising edge of signal = switch is unpressed
    {
        if(is_dis_homing)
        {
            steppers_disable_dis();
            is_dis_homing = 0;
            is_dis_homed = 1;
            printf("done homing dispenser\n");

            //set sel to home or move if dis was homed in preparation for sel moving
            if(sel_move_after_dis_home[0] == 1)
                sel_home_init();
            else if(sel_move_after_dis_home[0] == 2)
                sel_move_init(sel_move_after_dis_home[1]);
            sel_move_after_dis_home[0] = 0;
        }
    }
    else //pin is low = falling edge of signal = switch is pressed
    {
        printf("disswitch pressed\n");
        is_dis_homed = 0;
    }
}

// Timer2 overflow interrupt. Counts dispenser position for checking if the dispenser went too far.
ISR(TIMER2_OVF_vect)
{
    cli();
    dispense_pos++;
    sei();
}
