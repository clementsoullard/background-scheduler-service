#include <stdio.h>
#include <stdlib.h> 
#include <wiringPi.h>
#include <time.h>
#include <dirent.h>
#include <string.h>

typedef unsigned char uchar;
typedef unsigned int uint;

#define LCD_E 23
#define LCD_RS 22
#define LCD_D4 24
#define LCD_D5 25
#define LCD_D6 21
#define LCD_D7 26
#define RELAY_IN 7
#define IS_CLOSED -1
#define IS_OPEN 1
#define ADC_CS 0
#define ADC_CLK 4
#define ADC_DIO 2


// LCD instructions 
#define lcd_Clear 0b00000001 // replace all characters with ASCII 'space' 
#define lcd_Home 0b00000010 // return cursor to first position on first line 
#define lcd_EntryMode 0b00000110 // shift cursor from left to right on read/write 
#define lcd_DisplayOff 0b00001000 // turn display off 
#define lcd_DisplayOn 0b00001100 // display on, cursor off, don't blink character 
#define lcd_FunctionReset 0b00110000 // reset the LCD 
#define lcd_FunctionSet4bit 0b00101000 // 4-bit data, 2-line display, 5 x 7 font 
#define lcd_SetCursor 0b10000000 // set cursor position 
/**
The file name for the countdown
**/
char filename[]="/tmp/scheduler/CD";

/**
Inti the adc converter
**/
void adc_init()
{
pinMode(ADC_CS, OUTPUT);
pinMode(ADC_CLK, OUTPUT);
}

/**
* Lecteur de valuer sur l'adc.
**/
uchar get_ADC_Result(void)
{
uchar i;
uchar dat1=0, dat2=0;

digitalWrite(ADC_CS, 0);
digitalWrite(ADC_CLK,0);
digitalWrite(ADC_DIO,1); delayMicroseconds(2);
digitalWrite(ADC_CLK,1); delayMicroseconds(2);

digitalWrite(ADC_CLK,0);
digitalWrite(ADC_DIO,1); delayMicroseconds(2);
digitalWrite(ADC_CLK,1); delayMicroseconds(2);

digitalWrite(ADC_CLK,0);
digitalWrite(ADC_DIO,0); delayMicroseconds(2);
digitalWrite(ADC_CLK,1);
digitalWrite(ADC_DIO,1); delayMicroseconds(2);
digitalWrite(ADC_CLK,0);
digitalWrite(ADC_DIO,1); delayMicroseconds(2);

for(i=0;i<8;i++)
{
digitalWrite(ADC_CLK,1); delayMicroseconds(2);
digitalWrite(ADC_CLK,0); delayMicroseconds(2);

pinMode(ADC_DIO, INPUT);
dat1=dat1<<1 | digitalRead(ADC_DIO);
}

for(i=0;i<8;i++)
{
dat2 = dat2 | ((uchar)(digitalRead(ADC_DIO))<<i);
digitalWrite(ADC_CLK,1); delayMicroseconds(2);
digitalWrite(ADC_CLK,0); delayMicroseconds(2);
}

digitalWrite(ADC_CS,1);

return(dat1==dat2) ? dat1 : 0;
}

void pulseEnable ()
{
	digitalWrite (LCD_E, HIGH) ;
	delay(0.5); //  1/2 microsecond pause - enable pulse must be > 450ns
	digitalWrite (LCD_E, LOW) ;
}




/*
send a byte to the lcd in two nibbles
before calling use SetChrMode or SetCmdMode to determine whether to send character or command
*/
void lcd_byte(char bits)
{
	digitalWrite (LCD_D4,(bits & 0x10)) ; 
	digitalWrite (LCD_D5,(bits & 0x20)) ; 
	digitalWrite (LCD_D6,(bits & 0x40)) ; 
	digitalWrite (LCD_D7,(bits & 0x80)) ; 
	pulseEnable();

	digitalWrite (LCD_D4,(bits & 0x1)) ; 
	digitalWrite (LCD_D5,(bits & 0x2)) ; 
	digitalWrite (LCD_D6,(bits & 0x4)) ; 
	digitalWrite (LCD_D7,(bits & 0x8)) ; 
	pulseEnable();         
}
/**
To pass command to the LCD
**/
void SetCmdMode()
{
	digitalWrite (LCD_RS, 0); // set for commands
}


/**
To write text on the lcd
*/
void SetChrMode()
{
	digitalWrite (LCD_RS, 1); // set for characters
}
/**
Wrtie some text on screen
**/
void lcd_text(char *s)
{
	while(*s)
	lcd_byte(*s++);
}
/**
Init the screen
**/
void lcd_init()
{

	// wiringPiSetupGpio () ; // use BCIM numbering
	wiringPiSetup () ; // use wiring Pi numbering
	// set up pi pins for output
	pinMode (LCD_E,  OUTPUT);
	pinMode (LCD_RS, OUTPUT);
	pinMode (LCD_D4, OUTPUT);
	pinMode (LCD_D5, OUTPUT);
	pinMode (LCD_D6, OUTPUT);
	pinMode (LCD_D7, OUTPUT);
	pinMode (RELAY_IN, OUTPUT);
	digitalWrite (RELAY_IN, HIGH) ;
	// initialise LCD
	SetCmdMode(); // set for commands
	lcd_byte(0x33); // full init
	lcd_byte(0x32); // 4 bit mode
	lcd_byte(0x28); // 2 line mode
	lcd_byte(0x0C); // display on, cursor off, blink off
	lcd_byte(0x01);  // clear screen
	delay(3);        // clear screen is slow!
}
/*
Return at the begining of the line
*/
void goHome(){
	SetCmdMode(); // set for commands
	lcd_byte(lcd_Home);  // go home screen
	SetChrMode();
	delay(3);        // clear screen is slow!
}

/**
* If the relay is open or not
*/
int state=IS_OPEN;

/**
* Program pooling a directory, and doing a scheduling.
**/
int main (int argc, char** argv)
{
	lcd_init();
	adc_init();
	SetChrMode();

	int nbSecond;
	int remainingSeconds;

	time_t whenItsComplete ;
	char timestr[7];
	pinMode (RELAY_IN, OUTPUT);
	// Loop checking file
	while(1){
		nbSecond=getCoundownValue();
		// The countdown
		whenItsComplete = time(NULL)+nbSecond;
		do {
			remainingSeconds=whenItsComplete-time(NULL);
			if(remainingSeconds>-1){
				openRelay();
				int seconds=remainingSeconds%60;
				int hours=remainingSeconds/3600;
				int minutes=remainingSeconds/60%60;
				sleep(1);
				sprintf(timestr,"%02d:%02d:%02d",hours,minutes,seconds);
				goHome();
				lcd_text(timestr);
			}
			else if(nbSecond==-3){
				goHome();
				closeRelay();
				//	printf("Remaining %d\n",remainingSeconds);
				lcd_text("Expire    " );
			}
		} while ( remainingSeconds>0&& !isFilePresent() );

		if(nbSecond==-2){
			goHome();
			openRelay();
			lcd_text("Tele on      " );
			}
		else if(nbSecond==-1){
		goHome();
		closeRelay();
		lcd_text("Tele off      " );
		}

		sleep(3);
	}
}
/**
Opens the relay
**/
int openRelay(){
		if(state != IS_OPEN){
			pinMode (RELAY_IN, OUTPUT);
			digitalWrite (RELAY_IN, HIGH);
			state = IS_OPEN;
			printf("Ouverture du relai %i\n",state);
}
		}
/**
 * Closes the relay
**/

int closeRelay(){
		if(state != IS_CLOSED){
			pinMode (RELAY_IN, INPUT);
			digitalWrite (RELAY_IN, LOW);
			state = IS_CLOSED;
			printf("Fermeture du relai %i\n",state);
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
		printf("f=%d, nbSecond=%d\n",f,nbSecond);
	
	if(f&&nbSecond>0){
		printf("Suppression du fichier\n");
		remove (filename);
	}
	return nbSecond;
}