#include <stdio.h>
#include <stdlib.h> 
#include <wiringPi.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "adc.h"
#include <lcd.h>
#include "scheduler.h"


/**
* If the relay is open or not
*/

time_t lastImmobileState=0;

char filenameStatus[]="/home/clement/scheduler/work/ST";
char filenameCountdown[]="/home/clement/scheduler/work/CD";
char filenameStandby[]="/home/clement/scheduler/work/SB";
char filenameLock[]="/home/clement/scheduler/work/LCK";
char filenameRemaining[]="/home/clement/scheduler/work/REM";
char filenameSecondLine[]="/home/clement/scheduler/work/SL";

char secondLine[100];

int state=0;
int pauseSt=IS_RUNNING;
static int lcdHandle ;

int remainingSecondsOld;
 

/**
* Program pooling a directory, and doing a scheduling.
**/
int main (int argc, char** argv)
{

	initPins();
	adc_init();
	lcdHandle = lcdInit (NB_ROW, NB_COL, 4, LCD_RS,LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7,0,0,0,0) ;
	  if (lcdHandle < 0)
  {
    fprintf (stderr, "%s: lcdInit failed\n", argv [0]) ;
    return -1 ;
  }
	lcdClear    (lcdHandle) ;

	/** The status does not exist at the launch of the schduler prgram*/
	clearFile();
	strcpy(secondLine, "") ;
	int pid=readPid();	
	stopIfPidExists(pid);
	writePid();
	
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
		#ifndef PROD
		printf("pause:%d,valueInFile:%d\n",pauseSt,valueInFile);
		#endif
	
		if(valueInFile>=0){
			nbSecond=valueInFile;
		// The countdown
			whenItsComplete = time(NULL)+nbSecond;
			remainingSeconds=whenItsComplete-time(NULL);
		}
		else{remainingSeconds=-1;}
		#ifndef PROD
		printf("nbsecond:%d\n",nbSecond);
		#endif
	
	
		do {
	
	   /**
	   * Do a regular reset of the LCD
	   **/
		if(cycle%1000==0){
			 lcdDisplay     (lcdHandle, TRUE) ;
			lcdCursor      (lcdHandle, FALSE) ;
			lcdCursorBlink (lcdHandle, FALSE) ;
			lcdClear    (lcdHandle) ;
		}
		else if(cycle%60==0){
			lcdClear    (lcdHandle) ;
		}
		/**
		* Write the remaining seconds
		**/
		if(cycle%60==0){
			writeRemaining(remainingSeconds);
		}
		

		/**
		* Increment only if not in pause (Cycle increments will be at the end of the cycle loop.
		**/	

		
		updateStandbyStatus();
		cycle++;

		
		
		if(remainingSeconds>-1){
		if(pauseSt==IS_RUNNING){
				remainingSeconds=whenItsComplete-time(NULL);
		}else{
			    whenItsComplete=remainingSeconds+time(NULL);
		}


				openRelay();
				if(lastImmobileState==0||(time(NULL)-lastImmobileState)<NBSECONDBEFORECREENSHUTDOWN){
				digitalWrite (TRANSISTOR, LOW);
				}
				else{
				digitalWrite (TRANSISTOR, HIGH);
				}
				int seconds=remainingSeconds%60;
				int hours=remainingSeconds/3600;
				int minutes=remainingSeconds/60%60;
			
				sprintf(timestr,"%02d:%02d:%02d",hours,minutes,seconds);
					
			//	goHome();
				  lcdPosition (lcdHandle, 0, 0) ; lcdPuts (lcdHandle, timestr);
				if(strlen(secondLine)>0){
					lcdPosition (lcdHandle, 0, 1) ; lcdPuts (lcdHandle, secondLine);
				}
				#ifndef PROD
				printf("%s\n",timestr);
				#endif
				sleep(1);
		}
			/**
			* Block measuring intensity
			*/
			/*if(cycle%20==0){
				intensity=get_ADC_Result();
				#ifndef PROD
				printf("Intensite: %d\n",intensity);
				#endif
			}*/
			#ifndef PROD
			printf("Remaining seconds %d,cycle=%d\n",remainingSeconds,cycle);
			#endif

	} while ( remainingSeconds>0&& !isFilePresent());
	
		#ifndef PROD
	//	printf("Sortie boucle décompte\n");
		#endif
			
		/**
		* Every 10 ccyle there is a full reset of the screen. Otherwise it is  light reset.
		**/
	
		if(cycle%100==0){
			lcdClear  (lcdHandle) ;

		}
		nbSecond=0;
		/**
		*
		**/
		if(valueInFile==NO_FILE){
				closeRelay();
			digitalWrite (TRANSISTOR, HIGH);
			lcdPosition (lcdHandle, 0, 0) ; lcdPuts (lcdHandle, "Expire         ");
			#ifndef PROD
			printf("Expire\n");
			#endif
		}
			else if(valueInFile==TV_ON){
				openRelay();
			digitalWrite (TRANSISTOR, LOW);
			lcdPosition (lcdHandle, 0, 0) ; lcdPuts (lcdHandle, "Tele on         ");
			}
		else if(valueInFile==TV_OFF){
			digitalWrite (TRANSISTOR, HIGH);
			closeRelay();
			lcdPosition (lcdHandle, 0, 0) ; lcdPuts (lcdHandle, "Tele off        ");
		}
		checkSecondLine();
		if(strlen(secondLine)>0){
					lcdPosition (lcdHandle, 0, 1) ;
					lcdPuts (lcdHandle, secondLine);
		}

		sleep(3);
	}
}
/**
Opens the relay
**/
int openRelay(){
		if(state != IS_OPEN){
#ifndef RELAY_DISABLED		
	digitalWrite (RELAY_IN, LOW);
#endif
			state = IS_OPEN;
			#ifndef PROD
			printf("Open relay\n");
			#endif
//			writeStatus(state);
}
		}
/**
 * Closes the relay
**/

int closeRelay(){
		if(state != IS_CLOSED){
			//pinMode (RELAY_IN, OUTPUT);
#ifndef RELAY_DISABLED
			digitalWrite (RELAY_IN, HIGH);
#endif
			state = IS_CLOSED;
				#ifndef PROD
			printf("Close relay\n");
				#endif
//			writeStatus(state);
			}
}

int checkSecondLine(){
	FILE * fSL=fopen (filenameSecondLine, "rb");
	if(fSL){
		fseek (fSL, 0, SEEK_END);
		int length = ftell (fSL);
		fseek (fSL, 0, SEEK_SET);
		fread (secondLine, 1, length, fSL);
		secondLine[length-1]=0;
		#ifndef PROD
		printf("Fichier secondLinePresent %s\n",secondLine);
		#endif
		fclose (fSL);
		remove (filenameSecondLine);

	}
}

/**
* Check if file is present
**/
int isFilePresent(){
	FILE * f=fopen (filenameCountdown, "rb");
	checkSecondLine();
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

	FILE * f=fopen (filenameCountdown, "rb");

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
	/**
	* It the status is resumed
	*/
	if(f){
	
		if(nbSecond>0){
	//	lcd_init();
		remove (filenameCountdown);
		}
	}
	
	return nbSecond;

}
/**
Read the standby status from the file.
**/
void updateStandbyStatus(){
	/**
	Checking is the counter is running or not
	**/
	FILE  * f=fopen (filenameStandby, "r");

	if (f)
	{
	if(pauseSt!=IS_PAUSE){
	  pauseSt=IS_PAUSE;
	  lastImmobileState=time(NULL);
 	}
     fclose(f);
	}else{
	lastImmobileState=0;
	pauseSt=IS_RUNNING;
	}

}
/**
Say if the relay is opebn or closed t oan external application by 
wrtiing it in a file
**/
void writeStatus(int status){
  FILE * f;
  f = fopen(filenameStatus, "w");
  /** Only write status if the file exists **/
  if(f){
   fprintf(f, "%d", status);
   fclose(f);
  }
}

/**
Save the PID
**/
void writePid(){
  int pid=getpid();
   FILE *  f = fopen(filenameLock, "w");
  /** Only write status if the file exists **/
  if(f){
   fprintf(f, "%d", pid);
   fclose(f);
  }
}
/**
Save the seconds remaining
**/
void writeRemaining(int remainingSeconds){
/*
Nothing is written if the number of second has not changed
*/
if(remainingSeconds==remainingSecondsOld){
return;
}
   FILE *  f = fopen(filenameRemaining, "w");
  /** Only write status if the file exists **/
  if(f){
   fprintf(f, "%d", remainingSeconds);
   fclose(f);
  }
remainingSecondsOld=remainingSeconds;
}
/**
* Remove all the files.
**/
void clearFile(){
remove(filenameStatus);
remove(filenameCountdown);
remove(filenameStandby);
}

/**
Read the standby status from the file.
**/
int readPid(){
       int pid=-1;
        // read the content of the file
        char * buffer = 0;
        long length;

        FILE * f=fopen (filenameLock, "rb");

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
                        pid = atoi(buffer);
                }
                fclose (f);
        }
return pid;

}
/**
* Stops the program execution, if the program is already running.
**/
int stopIfPidExists(int pid){
	char fileproc[20];
	sprintf(fileproc,"/proc/%d",pid);
	struct stat s;
	int err = stat(fileproc, &s);

	if(-1 == err) {
	}else{
		printf("Le processus %s is existing. Exiting\n",fileproc);    
		exit(1);
	}
	return 0;
}


/**
* Init GPIO PIN
**/
void initPins(){
	wiringPiSetup () ; // use wiring Pi numbering
	// set up pi pins for output
	pinMode (RELAY_IN, OUTPUT);
	pinMode (RELAY_IN2, OUTPUT);
	pinMode (TRANSISTOR, OUTPUT);
};
