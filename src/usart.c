#include "usart.h"
#include "sel.h"
#include "dis.h"
#include "queue.h"

char cmd[USART_CMD_BUFFER_LENGTH];
uint8_t cmd_len = 0;

void usart_init()
{
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
void usart_tx_putchar(char c, FILE *stream)
{
    if(c == '\n')
        usart_tx_putchar('\r', stream);

    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
}

void usart_rx_check_queue()
{
    if(!queue_is_empty())
        usart_rx_add_char_to_cmd(queue_pop());
}

void usart_rx_add_char_to_cmd(char c)
{
    if(c != '\n')
    {
        cmd[cmd_len++] = c;
    }
    else if(cmd_len > 0)
    {
        usart_rx_parse_cmd();
        cmd_len = 0;
    }
}

/* 
 * Parses and interprets serial commands.
 * Some commands have arguments, others don't
 *
 * Non-argumental commands just do the specified operation after the one character is read
 * Argumental commands use the command[] array to store characters and char_count to keep track of how many
 * characters it read. It executes the command only after a full argument is received
 */
void usart_rx_parse_cmd()
{
    uint8_t is_cmd_valid = 1;

    if(cmd_len == 1)
    {
        if(cmd[0] == USART_HOME_SEL_COMMAND)
            sel_home_init();
        else if(cmd[0] == USART_HOLD_SEL_COMMAND)
            sel_hold();
        else if(cmd[0] == USART_DISABLE_SEL_COMMAND)
            sel_disable();
        else if(cmd[0] == USART_MOVE_SEL_NEXT_COMMAND)
            sel_move_next_init();
        else if(cmd[0] == USART_HOME_DIS_COMMAND)
            dis_home_init();
        else if(cmd[0] == USART_HOLD_DIS_COMMAND)
            dis_hold();
        else if(cmd[0] == USART_DISABLE_DIS_COMMAND)
            dis_disable();
        else if(cmd[0] == USART_GET_SEL_INDEX_COMMAND)
            printf("sel index: %i\n", sel_index);
        else
            is_cmd_valid = 0;
    }
    else if(cmd_len > 1)
    {
        uint8_t arg = 0;
        uint8_t arg_len = cmd_len - 1;
        for(uint8_t i = 1; i <= arg_len; i++)
        {
            uint8_t exp_factor = arg_len - i;
            uint8_t mul_factor = 1;
            for(uint8_t j = 0; j < exp_factor; j++)
                mul_factor *= 10;
            arg += (cmd[i]-'0') * mul_factor;
        }

        if(cmd[0] == USART_MOVE_SEL_COMMAND)
            sel_move_init(arg);
        else if(cmd[0] == USART_TOTAL_TUBES_COMMAND)
            sel_set_max_index(arg);
        else if(cmd[0] == USART_DISPENSE_COMMAND)
            dis_dispense_init(arg);
        else if(cmd[0] == USART_DISPENSE_NO_HOME_COMMAND)
            dis_dispense_no_home_init(arg);
        else
            is_cmd_valid = 0;
    }
    else
        is_cmd_valid = 0;

    if(is_cmd_valid == 1)
        printf("r\n");
    else
        printf("invalid cmd\n");
}

/*
 * Serial receive interrupt
 */
ISR(USART_RX_vect)
{
    cli();
    if(!queue_is_full())
        queue_push(UDR0);
    else
        printf("queue overflow\n");
    sei();
}

