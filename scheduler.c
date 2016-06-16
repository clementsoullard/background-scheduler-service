#include <stdio.h>
#include <stdlib.h> 
#include <wiringPi.h>
#include <time.h>
#include <dirent.h>
#include <string.h>


#define LCD_E 23
#define LCD_RS 22
#define LCD_D4 24
#define LCD_D5 25
#define LCD_D6 21
#define LCD_D7 26
// LCD instructions 
#define lcd_Clear 0b00000001 // replace all characters with ASCII 'space' 
#define lcd_Home 0b00000010 // return cursor to first position on first line 
#define lcd_EntryMode 0b00000110 // shift cursor from left to right on read/write 
#define lcd_DisplayOff 0b00001000 // turn display off 
#define lcd_DisplayOn 0b00001100 // display on, cursor off, don't blink character 
#define lcd_FunctionReset 0b00110000 // reset the LCD 
#define lcd_FunctionSet4bit 0b00101000 // 4-bit data, 2-line display, 5 x 7 font 
#define lcd_SetCursor 0b10000000 // set cursor position 

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

void SetCmdMode()
{
  digitalWrite (LCD_RS, 0); // set for commands
}

void SetChrMode()
{
  digitalWrite (LCD_RS, 1); // set for characters
}

void lcd_text(char *s)
{
  while(*s)
   lcd_byte(*s++);
 }

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
   
   // initialise LCD
   SetCmdMode(); // set for commands
   lcd_byte(0x33); // full init
   lcd_byte(0x32); // 4 bit mode
   lcd_byte(0x28); // 2 line mode
   lcd_byte(0x0C); // display on, cursor off, blink off
   lcd_byte(0x01);  // clear screen
   delay(3);        // clear screen is slow!
}

void goHome(){
SetCmdMode(); // set for commands
lcd_byte(lcd_Home);  // go home screen
SetChrMode();
delay(3);        // clear screen is slow!
}


/**
* Program pooling a directory, and doing a scheduling.
**/
int main (int argc, char** argv)
{
 lcd_init();
 SetChrMode();
int nbSecond=getCoundownValue();
//time_t before = time(NULL);
time_t whenItsComplete = time(NULL)+nbSecond;
int remainingSeconds;
char timestr[7];
do {
nbSecond=getCoundownValue();
if(nbSecond>0){
whenItsComplete = time(NULL)+nbSecond;
}

remainingSeconds=whenItsComplete-time(NULL);
int seconds=remainingSeconds%60;
int hours=remainingSeconds/3600;
int minutes=remainingSeconds/60%60;
sleep(1);
sprintf(timestr,"%02d:%02d:%02d",hours,minutes,seconds);
goHome();
lcd_text(timestr);
 } while ( remainingSeconds>0 );
printf("Compte à rebour expiré\n" );
}

/**
Get coundown value.
*/

int getCoundownValue(){
int nbSecond=-1;

// read the content of the file
char * buffer = 0;
long length;
char filename[]="/tmp/scheduler/CD";

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
unlink (filename);
if (buffer)
{
 printf(buffer);  // start to process your data / extract strings here...
}
 return nbSecond;

}