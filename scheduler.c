#include <stdio.h>
#include <time.h>
int main (int argc, char** argv)
{
int msec = 0, trigger = 10; /* 10ms */
time_t before = time(NULL);
time_t whenItsComplete = time(NULL)+10;
printf("whenItsComplete %d \n",whenItsComplete);

int iterations=0;
do {
 sleep(1);
printf("Clock %d \n",time(NULL));
 } while ( time(NULL) < whenItsComplete );

printf("Compte à rebour expiré\n",
  msec/1000, msec%1000, iterations);
}