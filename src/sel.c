//sel.c - tube selector functionality

#include "sel.h"
#include "dis.h"
#include "steppers.h"

volatile uint16_t sel_index = 0;
volatile uint16_t target_index = 0;
volatile uint16_t max_index = 5; //total tubes - 1
volatile uint8_t is_move_next_mode = 0; //is the selector in a move_next operation
volatile uint8_t has_found_target = 0;

volatile uint8_t is_homing = 0;
volatile uint8_t has_found_home = 0;
volatile uint8_t has_found_home_edge = 0;
volatile uint8_t home_before_next_move = 0;
volatile uint8_t sel_move_after_dis_home[2] = {0, 0};

volatile uint8_t is_pos_counting = 0;
volatile uint16_t pos_count = 0;

void sel_ir_init()
{
    //set ir sensor pin as input, PCINT10 pin (external interrupt)
    SEL_IR_DDR &= ~(1 << SEL_IR_PIN);

    //enable PCINT[14:8] (we only use PCINT10)
    PCICR |= (1 << PCIE1);

    //enable PCINT10 on pin
    PCMSK1 |= (1 << PCINT10);

    /* OLD PINOUT
    //set ir sensor pin as input, INT1 pin (external interrupt)
    SEL_IR_DDR &= ~(1 << SEL_IR_PIN);

    //set INT1 to trigger on any logical change
    EICRA &= ~(1 << ISC11);
    EICRA |= (1 << ISC10);

    //enable INT1
    EIMSK |= (1 << INT1);
    */
}

void sel_hold()
{
    steppers_hold_sel();
    printf("enabled selector\n");
}

void sel_disable()
{
    steppers_disable_sel();
    target_index = 0;
    printf("disabled selector\n");
}

void sel_print_index()
{
    printf("sel index: %i\n", sel_index); 
}

/*
 * Checks if dispenser is homed, and sets it to home and go back to what sel was doing after it homes
 */
uint8_t sel_move_check_dis(uint8_t caller, uint8_t arg)
{
    if(is_dis_homed)
        return 1;
    else
    {
        sel_move_after_dis_home[0] = caller;
        sel_move_after_dis_home[1] = arg;
        dis_home_init();
        printf("dispenser not homed! dispensing now\n");
        return 0;
    }
}

/*
 * Sets the maximum index based on the total number of tubes
 */
void sel_set_max_index(uint16_t num_tubes)
{
    max_index = num_tubes - 1;
    printf("total tubes set to: %i\n", num_tubes);
}

/*
 * Sets the selector to home
 */
void sel_home_init()
{
    if(sel_move_check_dis(1, 0) == 1)
    {
        is_homing = 1;
        sel_index = 0;
        steppers_move_sel(SEL_HOME_SPEED, SEL_DIR);
        printf("start sel home\n");
    }
}

/*
 * Sets the selector to move to a specified index. Checks to see if it must home beforehand due to the index changing
 * arbitrarily (ir sensor detection)
 * destination_index - index to move to
 */
void sel_move_init(uint16_t destination_index)
{
    if(sel_move_check_dis(2, destination_index) == 1)
    {
        target_index = destination_index;
        has_found_target = 0;
        is_move_next_mode = 0;

        printf("start sel move i%i t%i\n", sel_index, target_index);

        if(home_before_next_move)
            sel_home_init();
        else
        {
            if(target_index > max_index)
                printf("error: target index out of bounds\n");
            else if(sel_index == target_index)
            {
                steppers_hold_sel();
                printf("done moving to index\n");
            }
            else
                steppers_move_sel(SEL_MOVE_SPEED, SEL_DIR);
        }
    }
}

/*
 * Sets the selector to move to the next index (current index + 1)
 */
void sel_move_next_init()
{
    has_found_target = 0;
    is_move_next_mode = 1;
    steppers_move_sel(SEL_MOVE_SPEED, SEL_DIR);
    printf("start sel move next\n");
}

/*
 * IR sensor rising/falling edge interrupt. Contains most of the logic for homing and moving to position.
 *
 * How homing works: It determines which index is home based on how long the nub is on the chainChip that is
 * detected by the IR sensor. The longer nub represents home. The stepper is turned on initially and some variables
 * are set by the sel_home_init function. Every time a rising edge on the IR detector occurs,
 * the position counter is initialized and the motor is slowed down. When the switch is released the state of the
 * position counter is checked: if it is still on, it has not hit the threshold so that index is not home. If it is 
 * turned off it has finished counting, meaning that index is home.
 *
 * How move to position works: The sel_move_init function initializes and starts the motor. If a rising edge on the
 * IR detector occurs it updates the index and checks if it is at its destination. If it is, it slows the motor down
 * so it can stop more accurately. If a falling edge on the IR detector occurs, it checks if it is at its
 * destination. If it is, it stops the motor.
 */
ISR(PCINT1_vect)
{
    if(steppers_sel_state == 2) //motor is moving
    {
        if(SEL_IR_PIN_REG & (1 << SEL_IR_PIN)) //pin is high = rising edge = nub exits the IR detector
        {
            //homing logic
            if(is_homing && has_found_home_edge)
            {
                has_found_home_edge = 0;
                TIMSK0 &= ~(1 << TOIE0); //disable overflow interrupt for timer0 to stop position counting

                if(!is_pos_counting)
                {   //home had been found because position counter is not counting anymore
                    is_homing = 0;
                    is_pos_counting = 0;

                    steppers_hold_sel();
                    printf("done homing selector\n");

                    if(home_before_next_move)
                    {
                        home_before_next_move = 0;
                        if(sel_index != target_index)
                            sel_move_init(target_index);
                        else
                        {
                            steppers_hold_sel();
                            printf("done moving to index\n");
                        }
                    } 
                }
                else
                {
                    steppers_move_sel(SEL_HOME_SPEED, SEL_DIR); //return to fast speed
                    printf("not home\n");
                }
            }
            //move to index logic
            else if(has_found_target)
            {
                steppers_hold_sel();

                if(is_move_next_mode)
                    printf("done (next)\n");
                else //normal operation
                    printf("done moving to index\n");
            }
        }
        else //pin is low = falling edge = nub enters the IR detector
        {
            //homing logic
            if(is_homing)
            {
                has_found_home_edge = 1;
                TIMSK0 |= (1 << TOIE0); //enable overflow interrupt for timer0, for position counting
                steppers_move_sel(SEL_HOME_SPEED_SLOW, SEL_DIR); //slow down

                //reset and enable position counting
                pos_count = 0;
                is_pos_counting = 1;
            }
            //move to next index logic
            else if(is_move_next_mode)
            {
                has_found_target = 1;
                steppers_move_sel(SEL_MOVE_SPEED_SLOW, SEL_DIR); //slow down
                printf("found target (next)\n");
            }
            //move to index logic
            else
            {
                //update index
                if(sel_index == max_index)
                    sel_index = 0;
                else
                    sel_index++;

                printf("increment index i%i t%i mi%i", sel_index, target_index, max_index);

                if(sel_index == target_index) //found target
                {
                    has_found_target = 1;
                    steppers_move_sel(SEL_MOVE_SPEED_SLOW, SEL_DIR); //slow down
                    printf(" found target\n");
                }
                else
                    printf("\n");
            }
        }
    }
    else if(steppers_sel_state == 0 && !(SEL_IR_PIN_REG & (1 << SEL_IR_PIN))) //IR sensor detection while sel stepper is disabled
    {
        home_before_next_move = 1;
        printf("warning: IR detection while not moving; will home before next move\n");
    }
}

/*
 * TIMER0 overflow interrupt. Used for counting motor steps for selector homing. Is run every time the motor moves one * microstep. The microsteps are counted until it hits a threshold
 * value.
 */
ISR(TIMER0_OVF_vect)
{
    cli();
    pos_count++;
    if(pos_count >= SEL_HOME_POS_COUNT_THRESHOLD) //have we hit the threshold
        is_pos_counting = 0; //turn off position counting
    sei();
}

