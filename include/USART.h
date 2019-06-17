#ifndef USART_H
#define USART_H

#include "global_definitions.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define BAUD 9600
#include <util/setbaud.h>

#define USART_LED_PIN (1 << PC0)
#define USART_ARG_LENGTH 3 //length (number of characters) of command arguments

#define USART_MOVE_SEL_COMMAND 'M'
#define USART_HOME_SEL_COMMAND 'H'
#define USART_ENABLE_SEL_COMMAND 'E'
#define USART_DISABLE_SEL_COMMAND 'D'
#define USART_TOTAL_TUBES_COMMAND 'T'
#define USART_MOVE_SEL_NEXT_COMMAND 'O'
#define USART_DISPENSE_COMMAND 'I'
#define USART_HOME_DIS_COMMAND 'R'
#define USART_ENABLE_DIS_COMMAND 'N'
#define USART_DISABLE_DIS_COMMAND 'S'
#define USART_DISPENSE_NO_HOME_COMMAND 'Q'

void USART_init(void);
static void USART_putchar(char c, FILE *stream);
static void USART_parse_command(char input_char);

#endif

