#ifndef SCHEDULER_H

// This variable will cause the log not to be displayed
#define PROD
//#define RELAY_DISABLED

#define IS_CLOSED -1
#define IS_OPEN 1


#define TV_OFF -1
#define TV_ON -2
#define NO_FILE -3

#define NBSECONDBEFORECREENSHUTDOWN 30


#define ADC_CS 0
#define ADC_CLK 4
#define ADC_DIO 2


#define IS_PAUSE 2
#define IS_RUNNING 0

#define RELAY_IN 7
#define RELAY_IN2 1
#define TRANSISTOR 27

void clearFile();
int readPid();
void writePid();
void writeRemaining(int);
void writeStatus(int);
void updateStandbyStatus();
int stopIfPidExists();
#endif
