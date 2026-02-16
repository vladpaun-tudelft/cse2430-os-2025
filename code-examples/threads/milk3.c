#include <pthread.h>
#include <stdio.h>
#include <stdint.h>

static const char* P[2] = {"you", "your roommate"};

int milk = 0;

uint32_t noteA = 0;
uint32_t noteB = 0;

void* you(void* arg) 
{
   // leave note
   noteA=1; 

   while (noteB) { } // wait

   printf("You look into the fridge\n");
      
   if (milk == 0) 
   {  
      printf("You: no milk, I go and buy some\n");
      milk++;
   } else {
      printf("You: there is milk, no need to buy more milk\n");
   }

   noteA=0;

   return 0;
}

void* roommate(void* arg) 
{

   // leave note
   noteB=1;

   if (!noteA) {

      printf("Your roommate looks into the fridge\n");
      
      if (milk == 0)
      { 
         printf("Your roommate: no milk, I go and buy some\n");
         milk++;
      } else {
         printf("Your roommate: there is milk, no need to buy more milk\n");
      }
   }

   noteB=0;

   return 0;
}

int main() 
{
   pthread_t t_you;
   pthread_t t_roommate;

   pthread_create(&t_you, NULL, you, (void*) NULL);
   pthread_create(&t_roommate, NULL, roommate, (void*) NULL);

   pthread_join(t_you, NULL);
   pthread_join(t_roommate, NULL);

   printf("we ended up with %d bottles of milk\n", milk);

   return 0;
}

