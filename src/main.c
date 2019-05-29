/*
 * IC Dispenser AVR Program
 * Written by Andrew Hollabaugh, started 1/14/19
*/

#define F_CPU 16000000UL
#define BAUD 9600

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/setbaud.h>
#include <stdio.h>

#define ARG_LENGTH 3 //length (number of characters) of command arguments

#define MOVE_TO_POS_COMMAND 'M'
#define HOME_COMMAND 'H'
#define ENABLE_COMMAND 'E'
#define DISABLE_COMMAND 'D'
#define NR_ITEMS_COMMAND 'T'
#define MOVE_ONE_COMMAND 'O'
#define DISPENSE_COMMAND 'I'
#define HOME_DISPENSER_COMMAND 'R'
#define ENABLE_DISPENSER_COMMAND 'N'
#define DISABLE_DISPENSER_COMMAND 'S'
#define TEST_DISPENSER_COMMAND 'Q'

#define MOVE_TO_POS_SPEED 165
#define MOVE_TO_POS_SLOW_SPEED 250
#define HOME_SPEED 180
#define HOME_SPEED_SLOW 250
#define DISPENSE_SPEED 30
#define DISPENSE_HOME_SPEED 30
#define DISPENSE_HOME_SPEED_SLOW 250

#define MOVE_TO_POS_DIR 1
#define HOME_DIR 1
#define DISPENSE_DIR 0
#define DISPENSE_HOME_DIR 1

#define HOME_POS_COUNT_THRESHOLD 700 //number of microsteps for the threshold how long a switch is pressed for homing

#define TWI_START 0x08
#define TWI_REPEATED_START 0x10
#define TWI_MT_SLA_ACK 0x18
#define TWI_MT_DATA_ACK 0x28
#define TWI_MR_SLA_ACK 0x40
#define TWI_MR_DATA_ACK 0x50
#define TWI_MR_DATA_NACK 0x58

unsigned char TWI_SLA_W = 0xD0; //b1101000_0 //MPU6050 address and 0 for WRITE
unsigned char TWI_SLA_R = 0xD1; //b1101000_1 //MPU6050 address and 1 for READ

char state = 0; //0 is disabled, 1 is hold, 2 is moving
char index = 0;
char dIndex = 0; //destination index
char maxIndex = 4; // total tubes - 1
char moveOneMode = 0;
char hasFound = 0;

char isHoming = 0;
char hasFoundHome = 0;
char isFirstHome = 1;
char homeBeforeNextMove = 0;
char homeFoundEdge = 0;

char isPosCount = 0;
int posCountPos = 0;

char dispenserState = 0;
char isDispense = 0;
char isDispenserHoming = 0;
char isTestingDispenser = 0;
unsigned long dispensePos = 0UL;
unsigned long destinationPos = 0UL;
unsigned long stepsPerMM = 320UL; //613
unsigned long mmOffset = 41UL; //offset distance in mm between dispenser limit switch and edge of tube

volatile char command[ARG_LENGTH+1]; //stores characters for commands that require an argument
volatile char charCount = 0; //for keeping track of how many characters have been sent and are stored in the command array

/*
 * Pushes characters to the serial buffer for transmission.
 * After pushing a char, it waits for the char to leave the buffer (reg UDR0) by reading the UCSR0A flag before
 * sending the next one.
*/
void uart_putchar(char c, FILE *stream)
{
    if(c == '\n')
        uart_putchar('\r', stream);

    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
}

/*
 * Enables selector motor and sets it to move at a given speed and direction.
 *
 * speed - motor speed; an 8-bit number (0 to 255) with larger numbers corresponding to slower speeds
 *         NOTE: speeds at the low and high extremes will likely cause the motor to not turn properly
 * 
 * dir - motor direction; 0 or 1 (arbitrary)
 *
 * Always returns 2, which is the new state of the motor (moving)
 *
 */
char moveStepper(unsigned char speed, char dir)
{
    //set enable pin to low
    PORTD &= ~(1 << PD7);

    //set direction pin
    if(dir == 0)
        PORTB &= ~(1 << PB0);
    else if(dir == 1)
        PORTB |= (1 << PB0);

    //set TOP value for PWM comparison
    OCR0A = speed;

    //set timer clock select, 8x prescaling
    TCCR0B &= ~(1 << CS02);
    TCCR0B |= (1 << CS01);
    TCCR0B &= ~(1 << CS00);

    return 2;
}
/*
 * Enables dispenser motor and sets it to move at a given speed and direction.
 *
 * speed - motor speed; an 8-bit number (0 to 255) with larger numbers corresponding to slower speeds
 *         NOTE: speeds at the low and high extremes will likely cause the motor to not turn properly
 * 
 * dir - motor direction; 0 or 1 (arbitrary)
 *
 * Always returns 2, which is the new state of the motor (moving)
 *
 */
char moveDispenserStepper(unsigned char speed, char dir)
{
    //set enable pin to low
    PORTD &= ~(1 << PD4);

    //set direction pin
    if(dir == 0)
        PORTB &= ~(1 << PB1);
    else if(dir == 1)
        PORTB |= (1 << PB1);

    //set TOP value for PWM comparison
    OCR2A = speed;

    //set timer clock select to 8x prescaling
    TCCR2B &= ~(1 << CS22);
    TCCR2B |= (1 << CS21);
    TCCR2B &= ~(1 << CS20);

    return 2;
}

/*
 * Enables and stops selector motor; it will hold its position
 * Always returns 1, the new state of the motor (hold)
 */
char holdStepper()
{
    //set enable pin to low
    PORTD &= ~(1 << PD7);
    
    //set timer clock select to disabled
    TCCR0B &= ~(1 << CS02);
    TCCR0B &= ~(1 << CS01);
    TCCR0B &= ~(1 << CS00);

    return 1;
}

/*
 * Enables and stops dispenser motor; it will hold its position
 * Always returns 1, the new state of the motor (hold)
 */
char holdDispenserStepper()
{
    //set enable pin to low
    PORTD &= ~(1 << PD4);
    
    //set timer clock select to disabled
    TCCR2B &= ~(1 << CS22);
    TCCR2B &= ~(1 << CS21);
    TCCR2B &= ~(1 << CS20);

    return 1;
}

/* Disables and stops selector motor; it will be free to move
 * Always returns 0, the new state of the motor (disabled)
 */
char disableStepper()
{
    //set enable pin to high
    PORTD |= (1 << PD7);

    //set timer clock select to disabled
    TCCR0B &= ~(1 << CS02);
    TCCR0B &= ~(1 << CS01);
    TCCR0B &= ~(1 << CS00);

    return 0;
}

/* Disables and stops dispenser motor; it will be free to move
 * Always returns 0, the new state of the motor (disabled)
 */
char disableDispenserStepper()
{
    //set enable pin to high
    PORTD |= (1 << PD4);

    //set timer clock select to disabled
    TCCR2B &= ~(1 << CS22);
    TCCR2B &= ~(1 << CS21);
    TCCR2B &= ~(1 << CS20);

    return 0;
}

/*
 * Initializes selector homing
 */
void homeInit()
{
    isHoming = 1;
    index = 0;
    state = moveStepper(HOME_SPEED, HOME_DIR);
    printf("homing...\n");
}

/*
 * Initializes a move to position operation. Checks to see if it must home beforehand due to the index changing arbitrarily (ir sensor detection)
 * pos - index to move to
 */
void moveToPosInit(unsigned char pos)
{
    dIndex = pos;
    hasFound = 0;
    moveOneMode = 0;
    
    printf("start i%d d%d\n", index, dIndex);

    if(homeBeforeNextMove == 1)
    {
        homeInit();
    }
    else
    {
        if(dIndex > maxIndex)
        {
            printf("error: index out of bounds\n");
        }
        else if(index == dIndex)
        {
            state = holdStepper();
            printf("already there i%d d%d\n", index, dIndex);
            printf("done moving to pos\n");
        }
        else
        {
            state = moveStepper(MOVE_TO_POS_SPEED, MOVE_TO_POS_DIR); 
        }
    }
}

/*
 * Sets the maximum index
 * maxItems - max number of tubes
 */
void setMaxIndex(unsigned char maxItems)
{
    maxIndex = maxItems - 1;
    printf("total items %d\n", maxItems);
}

/*
 * Initializes a move one operation, which moves the selector to the next index
 */
void moveOne()
{
    hasFound = 0;
    moveOneMode = 1;
    state = moveStepper(MOVE_TO_POS_SPEED, MOVE_TO_POS_DIR);
    printf("start (one)\n");
}

/*
 * Initializes a dispense operation.
 * mm - number of millimeters to move the zip tie into the tube
 */
void dispenseInit(unsigned long mm)
{
    destinationPos = (mm + mmOffset) * stepsPerMM; //mmOfset is the distance between the limit switch and the start of the tube
    dispensePos = 0;
    isDispense = 1;
    printf("dispense %lumm %lusteps\n", mm, destinationPos);
    dispenserState = moveDispenserStepper(DISPENSE_SPEED, DISPENSE_DIR);
}

/*
 * Initializes dispenser homing
 */
void homeDispenserInit()
{
    isDispenserHoming = 1;
    dispenserState = moveDispenserStepper(DISPENSE_HOME_SPEED, DISPENSE_HOME_DIR);
    printf("dispenser homing\n");
}

/* 
 * Parses and interprets serial commands.
 * Some commands have arguments, others don't
 *
 * Non-argumental commands just do the specified operation after the one character is read
 * Argumental commands use the command[] array to store characters and charCount to keep track of how many
 * characters it read. It executes the command only after a full argument is received
 */
void parseCommand(char inputChar)
{
    if(inputChar == MOVE_TO_POS_COMMAND || inputChar == NR_ITEMS_COMMAND || inputChar == DISPENSE_COMMAND || inputChar == TEST_DISPENSER_COMMAND) //if the command is one that needs an argument
    {
        command[0] = inputChar;
        charCount = 1;
    }
    else if(inputChar == HOME_COMMAND)
    {
        homeInit();
        charCount = 0;
    }
    else if(inputChar == ENABLE_COMMAND)
    {
        state = holdStepper();
        printf("enabled\n");
        charCount = 0;
    }
    else if(inputChar == DISABLE_COMMAND)
    {
        state = disableStepper();
        printf("disabled\n");
        charCount = 0;
    }
    else if(inputChar == MOVE_ONE_COMMAND)
    {
        moveOne();
        charCount = 0;
    }
    else if(inputChar == HOME_DISPENSER_COMMAND)
    {
        homeDispenserInit();
        charCount = 0;
    }
    else if(inputChar == ENABLE_DISPENSER_COMMAND)
    {
        dispenserState = holdDispenserStepper();
        printf("enabled dispenser\n");
        charCount = 0;
    }
    else if(inputChar == DISABLE_DISPENSER_COMMAND)
    {
        dispenserState = disableDispenserStepper();
        printf("disabled dispenser\n");
        charCount = 0;
    }
    else
    {
        if(charCount > 0 && charCount < ARG_LENGTH + 1) //argument doesn't have enough characters yet
        {
            command[charCount] = inputChar;
            charCount++;
        }
        else if(charCount == ARG_LENGTH + 1) //full command with argument found
        {
            //calculate the numerical value of the argument
            unsigned long argValue = ((command[1] - '0') * 100) + ((command[2] - '0') * 10) + (command[3] - '0');

            //do things based on the first letter (non-argument) of the command
            if(command[0] == MOVE_TO_POS_COMMAND)
            {
                moveToPosInit(argValue);
            }
            if(command[0] == NR_ITEMS_COMMAND)
            {
                setMaxIndex(argValue);
            }
            if(command[0] == DISPENSE_COMMAND)
            {
                dispenseInit(argValue);
            }
            if(command[0] == TEST_DISPENSER_COMMAND)
            {
                isTestingDispenser = 1;
                dispenseInit(argValue);
            }

            charCount = 0;
        }
    }
}

/*
 * Start the position counter
 */
void initPosCount()
{
    posCountPos = 0;
    isPosCount = 1;
}

/*
 * IR sensor rising/falling edge interrupt. Contains most of the logic for homing and moving to position.
 *
 * How homing works: It determines which index is home based on how long the nub is on the chainChip that is
 * detected by the IR sensor. The longer nub represents home. The stepper is turned on initially and some variables
 * are set by the homeInit function when the command is parsed. Every time a rising edge on the IR detector occurs,
 * the position counter is initialized and the motor is slowed down. When the switch is released the state of the
 * position counter is checked: if it is still on, it has not hit the threshold so that index is not home. If it is 
 * turned off it has finished counting, meaning that index is home.
 *
 * How move to position works: The moveToPosInit function initializes and starts the motor. If a rising edge on the
 * IR detector occurs it updates the index and checks if it is at its destination. If it is, it slows the motor down
 * so it can stop more accurately. If a falling edge on the IR detector occurs, it checks if it is at its
 * destination. If it is, it stops the motor.
 */
ISR(INT1_vect)
{
    if(state == 2) //motor is moving
    {
        if(PIND & (1 << PD3)) //pin is high = rising edge = nub exits the IR detector
        {
            if(isHoming == 1 && homeFoundEdge == 1)
            {
                homeFoundEdge = 0;

                if(isPosCount == 0) //we are not counting position anymore; home is found
                {
                    isHoming = 0;
                    isPosCount = 0;
                    state = disableStepper();
                    printf("done homing\n");

                    if(homeBeforeNextMove == 1)
                    {
                        homeBeforeNextMove = 0;
                        if(index != dIndex)
                        {
                            moveToPosInit(dIndex);
                        }
                        else
                        {
                            state = holdStepper();
                            printf("done moving to pos\n");
                        }
                    } 
                    isFirstHome = 0;
                }
                else
                {
                    state = moveStepper(HOME_SPEED, HOME_DIR); //return to fast speed
                    printf("not home\n");
                }
            }
            else if(hasFound == 1)
            {
                state = holdStepper();

                if(moveOneMode == 1)
                {
                    printf("done (one)\n");
                }
                else //normal operation
                {
                    printf("done moving to pos\n");
                }
            }
        }
        else //pin is low = falling edge = nub enters the IR detector
        {
            if(isHoming == 1)
            {
                homeFoundEdge = 1;
                state = moveStepper(HOME_SPEED_SLOW, HOME_DIR); //slow down
                initPosCount();
            }
            else if(moveOneMode == 1)
            {
                state = moveStepper(MOVE_TO_POS_SLOW_SPEED, MOVE_TO_POS_DIR); //slow down
                hasFound = 1;
                printf("found (one)\n");
            }
            else //normal (move to index) operation
            {
                //update index
                if(index == maxIndex)
                    index = 0;
                else
                    index++;

                //print status line showing the index, destination index, and max index when index is incremented
                printf("inc i%d d%d mi%d", index, dIndex, maxIndex);

                if(index == dIndex) //found target
                {
                    state = moveStepper(MOVE_TO_POS_SLOW_SPEED, MOVE_TO_POS_DIR); //slow down
                    hasFound = 1;
                    printf(" found\n");
                }
                else
                {
                    printf("\n");
                }
            }
        }
    }
    else if(state == 0 && !(PIND & (1 << PD3))) //IR sensor detection while disabled probably because some idiot bumped it
    {
        homeBeforeNextMove = 1;
        printf("warning: IR detection while not moving\n");
    }
}

/*
 * Dispenser limit switch interrupt. Used to stop the dispenser motor when the limit switch is released when the
 * dispenser is homing.
 */
ISR(INT0_vect)
{
    if(isDispenserHoming == 1)
    {
        dispenserState = disableDispenserStepper();
        state = disableStepper();
        isDispenserHoming = 0;
        printf("done homing dispenser\n");
    }
}

/*
 * Serial receive interrupt
 */
ISR(USART_RX_vect)
{
    volatile char receivedChar = UDR0; //serial buffer register
    parseCommand(receivedChar);
}

/*
 * TIMER0 overflow interrupt. Used for counting motor steps for selector homing. This interrupt is run every time
 * TIMER0 overflows, indicating the motor moved one microstep. The microsteps are counted until it hits a threshold
 * value.
 */
ISR(TIMER0_OVF_vect)
{
    if(isPosCount == 1) //is relative move enabled
    {
        posCountPos++;

        if(posCountPos == HOME_POS_COUNT_THRESHOLD) //have we hit the threshold
        {
            isPosCount = 0; //since we have hit the threshold, turn off position counting
        }
    }
}

/*
 * TIMER2 overfow interrupt. Used for counting motor steps to make the dispenser go to the right position. Runs every
 * time TIMER2 overflows, indicated the motor moved one microstep. Microsteps are counted until the destination
 * position is reached.
 */
ISR(TIMER2_OVF_vect)
{
    if(isDispense == 1)
    {
        dispensePos++;

        if(dispensePos == destinationPos)
        {
            dispenserState = disableDispenserStepper();
            isDispense = 0;
            printf("done dispensing\n");

            if(isTestingDispenser == 0)
            {
                homeDispenserInit();
            }

            isTestingDispenser = 0;
        }
    }
}

void TWI_stop()
{
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN); //reset TWINT, enable TWI, enable stop condition
    //printf("stop: TWCR=%x, TWSR=%x\n", TWCR, TWSR);
}

void TWI_start()
{
    //send START condition
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); //reset TWINT, set TWSTA, enable TWI

    while(!(TWCR & (1 << TWINT))){} //loop until TWINT is set

    //printf("start: TWCR=%x, TWSR=%x\n", TWCR, TWSR);
    if((TWSR & 0xF8) != TWI_START && (TWSR & 0xF8) != TWI_REPEATED_START) //AND TWSR with a mask to only check most significant 5 bits of TWSR and compare to ensure start happened
    {
        printf("I2C error: start condition not sent\n");
    }
}

void TWI_write_init()
{
    TWDR = TWI_SLA_W; //write slave address and wrtie bit to TWDR for transmission
    TWCR = (1 << TWINT) | (1 << TWEN); //reset TWINT and enable TWI

    while(!(TWCR & (1 << TWINT))){} //loop until TWINT is set

    //printf("write_address: TWCR=%x, TWSR=%x\n", TWCR, TWSR);
    if((TWSR & 0xF8) != TWI_MT_SLA_ACK) //AND TWSR with a mask to only check most significant 5 bits of TWSR and compare to ensure ACK was received
    {
        printf("I2C error: ACK not received when sending write address\n");
    }
}

void TWI_write_data(unsigned char data)
{
    //char numBytes = sizeof(bytes) / sizeof(bytes[0]);
    //for(char i = 0; i < numBytes; i++)

    TWDR = data; //set TWDR to data byte
    TWCR = (1 << TWINT) | (1 << TWEN); //reset TWINT, enable TWI

    while(!(TWCR & (1 << TWINT))){} //loop until TWINT is set

    //printf("write_data: TWCR=%x, TWSR=%x\n", TWCR, TWSR);
    if((TWSR & 0xF8) != TWI_MT_DATA_ACK) //AND TWSR with a mask to only check most significant 5 bits of TWSR and compare to ensure ACK was received
    {
        printf("I2C error: ACK not received when sending data\n");
    }
    else
    {
        //printf("data transmission successful\n");
    }
}

void TWI_read_init()
{
    TWDR = TWI_SLA_R; //write slave address and read bit to TWDR for transmission
    TWCR = (1 << TWINT) | (1 << TWEN); //reset TWINT, enable TWI

    while(!(TWCR & (1 << TWINT))){} //loop until TWINT is set

    //printf("read_address: TWCR=%x, TWSR=%x\n", TWCR, TWSR);
    if((TWSR & 0xF8) != TWI_MR_SLA_ACK) //AND TWSR with a mask to only check most significant 5 bits of TWSR and compare to ensure ACK was received
    {
        printf("I2C error: ACK not received when sending receive address\n");
    }
}

unsigned char TWI_read_data()
{
    TWCR = (1 << TWINT) | /*(1 << TWEA) | */(1 << TWEN); //reset TWINT, enable TWI, send NACK

    while(!(TWCR & (1 << TWINT))){} //loop until TWINT is set

    unsigned char data = TWDR; //get data from TWDR

    printf("read_data: TWCR=%x, TWSR=%x, TWDR=%x\n", TWCR, TWSR, TWDR);
    if((TWSR & 0xF8) != TWI_MR_DATA_NACK) //AND TWSR with a mask to only check most significant 5 bits of TWSR and compare to ensure NACK was sent
    {
        printf("I2C error: ACK not sent when receiving data\n");
    }

    return data;
}

void get_accel_data()
{
    TWI_start();
    TWI_write_init();
    TWI_write_data(68);
    TWI_start();
    TWI_read_init();
    unsigned char accel_data = TWI_read_data();
    TWI_stop();
    printf("ACCEL: %i\n", accel_data);
}

int main()
{
    //PIN INIT

    DDRD &= ~(1 << DDD3); //ir sensor input
    DDRD &= ~(1 << DDD2); //dispenser limit switch input
    PORTD |= (1 << PD2); //turn on internal pullup for limit switch
    DDRD |= 1 << DDD4; //sm2 en - PD4 - output
    DDRD |= 1 << DDD6; //sm1 step - PD6 - output (OCR0A)
    DDRD |= 1 << DDD7; //sm1 en - PD7 - output
    DDRB |= 1 << DDB0; //sm1 dir - PB0 - output
    DDRB |= 1 << DDB1; //sm2 dir - PB1 - output
    DDRB |= 1 << DDB3; //sm2 step - PB3 - output (OCR2A)

    //STEPPER/TIMER INIT
    
    //disable; set enable to high
    PORTD |= (1 << PD7);
    PORTD |= (1 << PD4);

    //set timer compare output mode toggle OC0A/OC2A on compare match
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

    //enable overflow interrupt
    TIMSK0 |= (1 << TOIE0);
    TIMSK2 |= (1 << TOIE2);


    //EXTERNAL INTERRUPT INIT

    //set INT1/INT0 to trigger on any logical change
    EICRA &= ~(1 << ISC11);
    EICRA |= (1 << ISC10);
    EICRA &= ~(1 << ISC01);
    EICRA |= (1 << ISC00);

    //enable  INT1/INT0
    EIMSK |= (1 << INT1);
    EIMSK |= (1 << INT0);

    //SERIAL INIT

    //set baud
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

#if USE_2X
    UCSR0A |= (1 << U2X0);
#else
    UCSR0A &= ~(1 << U2X0);
#endif

    //enable tx and rx
    UCSR0B |= (1 << RXEN0);
    UCSR0B |= (1 << TXEN0);

    //enable receiver interrupt
    UCSR0B |= (1 << RXCIE0);

    //set frame: 8 bits, 1 stop bit
    UCSR0B &= ~(1 << UCSZ02);
    UCSR0C |= (1 << UCSZ01);
    UCSR0C |= (1 << UCSZ00); 

    //setup serial output as a stream
    FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
    
    //direct stdout to the serial output 
    stdout = &uart_output;

    
    //TWI INIT
    
    //set bit rate register to 72, corresponding to 100kHz
    TWBR = 72;

    //turn on interrupts
    sei();

    //get_accel_data();

    while(1){get_accel_data();}

    return 0;
}
