#include <stdio.h>
#include <stdlib.h> 
#include <wiringPi.h>
#include <time.h>
#include <dirent.h>
#include <string.h>
#include "adc.h"
#include "lcd.h"
 //
 #define PROD



#define IS_CLOSED -1
#define IS_OPEN 1

#define TV_OFF -1
#define TV_ON -2
#define NO_FILE -3
#define PAUSE -4
#define RESUME -5

/**
The file name for the countdown
**/
char filename[]="/tmp/scheduler/CD";
char filenameStatus[]="/tmp/scheduler/ST";
void writeStatus(int status);

#define ADC_CS 0
#define ADC_CLK 4
#define ADC_DIO 2




/**
* If the relay is open or not
*/
int state=IS_OPEN;

/**
* Program pooling a directory, and doing a scheduling.
**/
int main (int argc, char** argv)
{
	adc_init();
	lcd_init();
	SetChrMode();

	int nbSecond;
	int remainingSeconds;
	uchar intensity;
	uchar tmpintensity;
    int i;
	int valueInFile;
	time_t whenItsComplete ;
	char timestr[7];
	pinMode (RELAY_IN, OUTPUT);
	// Permanent loop checking file.
	int cycle=0;
	while(1){
		valueInFile=getCoundownValue();
		if(valueInFile>0||valueInFile>-4){
			nbSecond=valueInFile;
		}
		/**
		iF a pause is required.
		**/
		else if (valueInFile==PAUSE){
			nbSecond=remainingSeconds;
			/***
			 * In case we are in pause, but not seconds are remaining, 
			 * then we should remove the file. and set the status to noFile
			**/
			if(nbSecond==0){
				remove (filename);
				nbSecond=-3;
			}
		}
		// The countdown
		whenItsComplete = time(NULL)+nbSecond;
		do {
			cycle++;
			remainingSeconds=whenItsComplete-time(NULL);
			if(remainingSeconds>-1){
				openRelay();
				digitalWrite (TRANSISTOR, HIGH);
				int seconds=remainingSeconds%60;
				int hours=remainingSeconds/3600;
				int minutes=remainingSeconds/60%60;
				sleep(1);
				sprintf(timestr,"%02d:%02d:%02d",hours,minutes,seconds);
				//printf("Reset LCD ? %d\n",cycle%5);
				if(cycle%5==0){
				resetLcd();
				}
				goHome();
				lcd_text(timestr);
				#ifndef PROD
				printf("%s\n",timestr);
				#endif
	      }
			else if(nbSecond==NO_FILE){
				goHome();
				closeRelay();
				//	printf("Remaining %d\n",remainingSeconds);			
				lcd_text("Expire    " );
				#ifndef PROD
				printf("Expire\n");
				#endif
			}
			/**
			* Block measuring intensity
			*/
			
			intensity=get_ADC_Result();
			#ifndef PROD
			printf("Intensite: %d\n",intensity);
			#endif
	} while ( remainingSeconds>0&& !isFilePresent());
		
		/**
		* 
		**/
		
		if(cycle%10==0){
		 resetLcd();
		}		
		if(nbSecond==TV_ON){
			goHome();
			openRelay();
			digitalWrite (TRANSISTOR, HIGH);
			lcd_text("Tele on      " );
			}
		else if(nbSecond==TV_OFF){
			digitalWrite (TRANSISTOR, LOW);
			goHome();
			closeRelay();
			lcd_text("Tele off      " );
		}
			sleep(3);
		cycle++;
	}
}
/**
Opens the relay
**/
int openRelay(){
		if(state != IS_OPEN){
			pinMode (RELAY_IN, INPUT);
			digitalWrite (RELAY_IN, HIGH);
			state = IS_OPEN;
				#ifndef PROD
			printf("Open relay\n");
				#endif
			writeStatus(1);
}
		}
/**
 * Closes the relay
**/

int closeRelay(){
		if(state != IS_CLOSED){
			pinMode (RELAY_IN, OUTPUT);
			digitalWrite (RELAY_IN, LOW);
			state = IS_CLOSED;
				#ifndef PROD
			printf("Close relay\n");
				#endif
			writeStatus(0);
			}
}


/**
* Check if file is present
**/
int isFilePresent(){
	FILE * f=fopen (filename, "rb");
	return f!=0;
}

/**
Get coundown value.
*/
int getCoundownValue(){
	int nbSecond=-3;

	// read the content of the file
	char * buffer = 0;
	long length;

	FILE * f=fopen (filename, "rb");

	if (f)
	{
		fseek (f, 0, SEEK_END);
		length = ftell (f);
		fseek (f, 0, SEEK_SET);
		/** The buffer stor the content of the file */
		buffer = malloc (length);
		if (buffer)
		{
			fread (buffer, 1, length, f);
			nbSecond = atoi(buffer);
		}
		fclose (f);
	}
	#ifndef PROD
		printf("valueInfile=%d\n",nbSecond);
	#endif
	/**
	* It the status is resumed
	*/
	if(f&&nbSecond>=0||nbSecond==RESUME){
	#ifndef PROD
		printf("Remove file\n");
	#endif
		remove (filename);
	}
	return nbSecond;
}
/**
Say if the relay is opebn or closed t oan external application by 
wrtiing it in a file
**/
void writeStatus(int status){
  FILE *f;
  f = fopen(filenameStatus, "a");
  /** Only write status if the file exists **/
  if(f){
   fprintf(f, "%d", status);
   fclose(f);
  }
}