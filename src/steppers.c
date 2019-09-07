#include "steppers.h"

//motor states: 0=disabled, 1=hold, 2=moving
volatile uint8_t steppers_sel_state = 0;
volatile uint8_t steppers_dis_state = 0;

void steppers_init()
{
    //define all stepper control pins as outputs
    STEPPERS_SEL_EN_DDR |= (1 << STEPPERS_SEL_EN_PIN);
    STEPPERS_SEL_DIR_DDR |= (1 << STEPPERS_SEL_DIR_PIN);
    STEPPERS_SEL_STEP_DDR |= (1 << STEPPERS_SEL_STEP_PIN);
    STEPPERS_DIS_EN_DDR |= (1 << STEPPERS_DIS_EN_PIN);
    STEPPERS_DIS_DIR_DDR |= (1 << STEPPERS_DIS_DIR_PIN);
    STEPPERS_DIS_STEP_DDR |= (1 << STEPPERS_DIS_STEP_PIN);

    //disable both motors by setting enables to high
    STEPPERS_SEL_EN_PORT |= (1 << STEPPERS_SEL_EN_PIN);
    STEPPERS_DIS_EN_PORT |= (1 << STEPPERS_DIS_EN_PIN);

    //set timer compare output mode to toggle PWM pins on compare match
    TCCR0A &= ~(1 << COM0A1);
    TCCR0A |= (1 << COM0A0);

    TCCR2A &= ~(1 << COM2A1);
    TCCR2A |= (1 << COM2A0);

    //set timer mode to phase correct PWM
    TCCR0B |= (1 << WGM02);
    TCCR0A &= ~(1 << WGM01);
    TCCR0A |= (1 << WGM00);

    TCCR2B |= (1 << WGM22);
    TCCR2A &= ~(1 << WGM21);
    TCCR2A |= (1 << WGM20);

    //overflow interrupts (for position counting) will be enabled/disabled when needed
}

/*
 * Enables selector motor and sets it to move at a given speed and direction.
 *
 * speed - motor speed; an 8-bit number (0 to 255) with larger numbers corresponding to slower speeds
 *         NOTE: speeds at the low and high extremes will cause the motor to not turn properly
 * 
 * dir - motor direction; 0 or 1 
 */
void steppers_move_sel(uint8_t speed, uint8_t dir)
{
    //enable stepper by setting enable pin to low
    STEPPERS_SEL_EN_PORT &= ~(1 << STEPPERS_SEL_EN_PIN);

    //set dir pin
    if(dir == 0)
        STEPPERS_SEL_DIR_PORT &= ~(1 << STEPPERS_SEL_DIR_PIN);
    else if(dir == 1)
        STEPPERS_SEL_DIR_PORT |= (1 << STEPPERS_SEL_DIR_PIN);

    //set TOP value for output compare to generate PWM signal
    OCR0A = speed;

    //set timer clock select, 8x prescaling, starting the movement
    TCCR0B &= ~(1 << CS02);
    TCCR0B |= (1 << CS01);
    TCCR0B &= ~(1 << CS00);

    steppers_sel_state = 2;
}

/*
 * Enables and stops selector motor; it will hold its position
*/
void steppers_hold_sel()
{
    //enable stepper by setting enable pin to low
    STEPPERS_SEL_EN_PORT &= ~(1 << STEPPERS_SEL_EN_PIN);

    //set timer clock select to disabled
    TCCR0B &= ~(1 << CS02);
    TCCR0B &= ~(1 << CS01);
    TCCR0B &= ~(1 << CS00);

    steppers_sel_state = 1;
}

/* 
 * Disables and stops selector motor; it will be free to move
 */
void steppers_disable_sel()
{
    //set enable pin to high
    STEPPERS_SEL_EN_PORT |= (1 << STEPPERS_SEL_EN_PIN);

    //set timer clock select to disabled
    TCCR0B &= ~(1 << CS02);
    TCCR0B &= ~(1 << CS01);
    TCCR0B &= ~(1 << CS00);

    steppers_sel_state = 0;
}

/*
 * Enables dispenser motor and sets it to move at a given speed and direction.
 *
 * speed - motor speed; an 8-bit number (0 to 255) with larger numbers corresponding to slower speeds
 *         NOTE: speeds at the low and high extremes will cause the motor to not turn properly
 * 
 * dir - motor direction; 0 or 1 
 */
void steppers_move_dis(uint8_t speed, uint8_t dir)
{
    //enable stepper by setting enable pin to low
    STEPPERS_DIS_EN_PORT &= ~(1 << STEPPERS_DIS_EN_PIN);

    //set dir pin
    if(dir == 0)
        STEPPERS_DIS_DIR_PORT &= ~(1 << STEPPERS_DIS_DIR_PIN);
    else if(dir == 1)
        STEPPERS_DIS_DIR_PORT |= (1 << STEPPERS_DIS_DIR_PIN);

    //set TOP value for output compare to generate PWM signal
    OCR2A = speed;

    //set timer clock select, 8x prescaling, starting the movement
    TCCR2B &= ~(1 << CS22);
    TCCR2B |= (1 << CS21);
    TCCR2B &= ~(1 << CS20);

    steppers_dis_state = 2;
}

/*
 * Enables and stops dispenser motor; it will hold its position
*/
void steppers_hold_dis()
{
    //enable stepper by setting enable pin to low
    STEPPERS_DIS_EN_PORT &= ~(1 << STEPPERS_DIS_EN_PIN);

    //set timer clock select to disabled
    TCCR2B &= ~(1 << CS22);
    TCCR2B &= ~(1 << CS21);
    TCCR2B &= ~(1 << CS20);

    steppers_dis_state = 1;
}

/* 
 * Disables and stops dispenser motor; it will be free to move
 */
void steppers_disable_dis()
{
    //set enable pin to high
    STEPPERS_DIS_EN_PORT |= (1 << STEPPERS_DIS_EN_PIN);

    //set timer clock select to disabled
    TCCR2B &= ~(1 << CS22);
    TCCR2B &= ~(1 << CS21);
    TCCR2B &= ~(1 << CS20);

    steppers_dis_state = 0;
}

