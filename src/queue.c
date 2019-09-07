//queue.h - array instance of a queue used for received sreial commands

#include "queue.h"

volatile char queue_array[MAX];
volatile int32_t front = 0;
volatile int32_t rear = -1;
volatile uint32_t count = 0;

char queue_peek()
{
   return queue_array[front];
}

bool queue_is_empty()
{
   return count == 0;
}

bool queue_is_full()
{
   return count == MAX;
}

uint32_t queue_size()
{
   return count;
}  

void queue_push(char data)
{
   if(!queue_is_full())
   {
      if(rear == MAX-1)
         rear = -1; 

      queue_array[++rear] = data;
      count++;
   }
}

char queue_pop()
{
   char data = queue_array[front++];
	
   if(front == MAX)
      front = 0;
	
   count--;
   return data;  
}

