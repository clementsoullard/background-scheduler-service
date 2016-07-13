#include <stdio.h>
#include <stdlib.h> 
#include <wiringPi.h>
#include <time.h>
#include <dirent.h>
#include <string.h>
#include "adc.h"
#include "lcd.h"
 // #define PROD



#define IS_CLOSED -1
#define IS_OPEN 1
#define IS_PAUSE 2

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
int state=0;

/**
* Program pooling a directory, and doing a scheduling.
**/
int main (int argc, char** argv)
{
	wiringPiSetup () ; // use wiring Pi numbering
	adc_init();
	lcd_init();
	SetChrMode();
	/** The status does not exist at the launch of the schduler prgram*/
	remove (filenameStatus);


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
			}else{
				pauseRelay();
			}
		}
		// The countdown
		whenItsComplete = time(NULL)+nbSecond;
		do {
			cycle++;
			remainingSeconds=whenItsComplete-time(NULL);
			if(remainingSeconds>-1){
				openRelay();
				digitalWrite (TRANSISTOR, LOW);
				int seconds=remainingSeconds%60;
				int hours=remainingSeconds/3600;
				int minutes=remainingSeconds/60%60;
				sleep(1);
				sprintf(timestr,"%02d:%02d:%02d",hours,minutes,seconds);
				//printf("Reset LCD ? %d\n",cycle%5);
				if(cycle%30==0){
				resetLcd();
				}
				goHome();
				lcd_text(timestr);
				#ifndef PROD
				printf("%s\n",timestr);
				#endif
	    }
			/**
			* Block measuring intensity
			*/
			if(cycle%20==0){
				intensity=get_ADC_Result();
				#ifndef PROD
				printf("Intensite: %d\n",intensity);
				#endif
			}
	} while ( remainingSeconds>0&& !isFilePresent());
		
		/**
		* Every 30 ccyle there is a full reset of the screen. Otherwise it is  light reset.
		**/
		
		if(cycle%30==0){
			lcd_init();
		}
		else if (cycle%10==0){
		 resetLcd();
		}
		/**
		*
		**/
			if(nbSecond==NO_FILE){
				goHome();
				closeRelay();
				//	printf("Remaining %d\n",remainingSeconds);			
				digitalWrite (TRANSISTOR, HIGH);
				lcd_text("Expire    " );
				#ifndef PROD
				printf("Expire\n");
				#endif
			}
			else if(nbSecond==TV_ON){
			openRelay();
			digitalWrite (TRANSISTOR, LOW);
			lcd_init();
			goHome();
			lcd_text("Tele on      " );
			}
		else if(nbSecond==TV_OFF){
			digitalWrite (TRANSISTOR, HIGH);
			closeRelay();
			lcd_init();
			goHome();
			lcd_text("Tele off      " );
		}
		else if(nbSecond==RESUME){
			resumeRelay();
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
			//pinMode (RELAY_IN, OUTPUT);
			digitalWrite (RELAY_IN, LOW);
			state = IS_OPEN;
				#ifndef PROD
			printf("Open relay\n");
				#endif
			writeStatus(state);
}
		}
/**
 * Closes the relay
**/

int closeRelay(){
		if(state != IS_CLOSED){
		
			//pinMode (RELAY_IN, OUTPUT);
			digitalWrite (RELAY_IN, HIGH);
			state = IS_CLOSED;
				#ifndef PROD
			printf("Close relay\n");
				#endif
			writeStatus(state);
			}
}

/**
 * Pause the relay
**/

int pauseRelay(){
		if(state != IS_PAUSE){
			state = IS_PAUSE;
				#ifndef PROD
			printf("Pause relay\n");
				#endif
			writeStatus(state);
			}
}

/**
 * Resume
**/

int resumeRelay(){
		if(state == IS_PAUSE){
			state = IS_OPEN;
				#ifndef PROD
			printf("Resume relay\n");
				#endif
			writeStatus(state);
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
		lcd_init();
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
  f = fopen(filenameStatus, "w");
  /** Only write status if the file exists **/
  if(f){
   fprintf(f, "%d", status);
   fclose(f);
  }
}