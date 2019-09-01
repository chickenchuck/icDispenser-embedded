#ifndef USART_H
#define USART_H

#include "global_definitions.h"
#include <avr/interrupt.h>
#include <stdio.h>
#include <math.h>

#define BAUD 9600
#include <util/setbaud.h>

#define USART_CMD_BUFFER_LENGTH 5
#define USART_MOVE_SEL_COMMAND 'M'
#define USART_HOME_SEL_COMMAND 'H'
#define USART_HOLD_SEL_COMMAND 'E'
#define USART_DISABLE_SEL_COMMAND 'D'
#define USART_TOTAL_TUBES_COMMAND 'T'
#define USART_MOVE_SEL_NEXT_COMMAND 'O'
#define USART_DISPENSE_COMMAND 'I'
#define USART_HOME_DIS_COMMAND 'R'
#define USART_HOLD_DIS_COMMAND 'N'
#define USART_DISABLE_DIS_COMMAND 'S'
#define USART_DISPENSE_NO_HOME_COMMAND 'Q'
#define USART_GET_SEL_INDEX_COMMAND 'G'

void usart_init(void);
void usart_tx_putchar(char c, FILE *stream);
void usart_rx_check_queue(void);
void usart_rx_add_char_to_cmd(char c);
void usart_rx_parse_cmd(void);

#endif

