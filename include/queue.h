#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX 30

char queue_peek(void);
bool queue_is_empty(void);
bool queue_is_full(void);
uint32_t queue_size(void);
void queue_push(char data);
char queue_pop(void);

#endif

