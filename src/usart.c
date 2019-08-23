#include "usart.h"
#include "steppers.h"
#include "sel.h"
#include "dis.h"

volatile char command[USART_ARG_LENGTH+1]; //stores characters for commands that require an argument
volatile char char_count = 0; //for keeping track of how many characters have been sent and are stored in the command array

void usart_init()
{
    //set LED pin as output
    USART_LED_DDR |= (1 << USART_LED_PIN);

    //set baud; _VALUEs are from util/setbaud.h
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
}

/*
 * Pushes characters to the serial buffer for transmission.
 * After pushing a char, it waits for the char to leave the buffer (reg UDR0) by reading the UCSR0A flag before
 * sending the next one.
*/
void usart_putchar(char c, FILE *stream)
{
    USART_LED_PORT |= (1 << USART_LED_PIN);

    if(c == '\n')
        usart_putchar('\r', stream);

    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;

    USART_LED_PORT &= ~(1 << USART_LED_PIN);
}

/* 
 * Parses and interprets serial commands.
 * Some commands have arguments, others don't
 *
 * Non-argumental commands just do the specified operation after the one character is read
 * Argumental commands use the command[] array to store characters and char_count to keep track of how many
 * characters it read. It executes the command only after a full argument is received
 */
void usart_parse_command(char input_char)
{
    //check if the command is one that needs an argument
    if(input_char == USART_MOVE_SEL_COMMAND || input_char == USART_TOTAL_TUBES_COMMAND ||
            input_char == USART_DISPENSE_COMMAND || input_char == USART_DISPENSE_NO_HOME_COMMAND)
    {
        command[0] = input_char;
        char_count = 1;
    }
    else if(input_char == USART_HOME_SEL_COMMAND)
    {
        sel_home_init();
        char_count = 0;
    }
    else if(input_char == USART_ENABLE_SEL_COMMAND)
    {
        steppers_hold_sel();
        printf("enabled selector\n");
        char_count = 0;
    }
    else if(input_char == USART_DISABLE_SEL_COMMAND)
    {
        steppers_disable_sel();
        printf("disabled selector\n");
        char_count = 0;
    }
    else if(input_char == USART_MOVE_SEL_NEXT_COMMAND)
    {
        sel_move_next_init();
        char_count = 0;
    }
    else if(input_char == USART_HOME_DIS_COMMAND)
    {
        dis_home_init();
        char_count = 0;
    }
    else if(input_char == USART_ENABLE_DIS_COMMAND)
    {
        steppers_hold_dis();
        printf("enabled dispenser\n");
        char_count = 0;
    }
    else if(input_char == USART_DISABLE_DIS_COMMAND)
    {
        steppers_disable_dis();
        dis_is_dispense = 0;
        printf("disabled dispenser\n");
        char_count = 0;
    }
    else if(input_char == USART_GET_SEL_INDEX_COMMAND)
    {
        printf("sel index: %i\n", sel_index);
        char_count = 0;
    }
    else
    {
        if(char_count > 0 && char_count < USART_ARG_LENGTH + 1) //argument doesn't have enough characters yet
        {
            command[char_count] = input_char;
            char_count++;
        }
        else if(char_count == USART_ARG_LENGTH + 1) //full command with argument found
        {
            //calculate the numerical value of the argument
            unsigned long arg_value = ((command[1] - '0') * 100) + ((command[2] - '0') * 10) + (command[3] - '0');

            //do things based on the first letter (non-argument) of the command
            if(command[0] == USART_MOVE_SEL_COMMAND)
                sel_move_init(arg_value);
            if(command[0] == USART_TOTAL_TUBES_COMMAND)
                sel_set_max_index(arg_value);
            if(command[0] == USART_DISPENSE_COMMAND)
                dis_dispense_init(arg_value);
            if(command[0] == USART_DISPENSE_NO_HOME_COMMAND)
                dis_dispense_no_home_init(arg_value);

            char_count = 0;
        }
    }
}

/*
 * Serial receive interrupt
 */
ISR(USART_RX_vect)
{
    USART_LED_PORT |= (1 << USART_LED_PIN);
    usart_parse_command(UDR0);
    USART_LED_PORT &= ~(1 << USART_LED_PIN);
}
