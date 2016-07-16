#include <wiringPi.h>
#include <stdio.h>
#include "scheduler.h"
#include "lcd.h"

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
	digitalWrite (LCD_D4,(bits & 0x10)); 
	digitalWrite (LCD_D5,(bits & 0x20)); 
	digitalWrite (LCD_D6,(bits & 0x40)); 
	digitalWrite (LCD_D7,(bits & 0x80)); 
	pulseEnable();

	digitalWrite (LCD_D4,(bits & 0x1)); 
	digitalWrite (LCD_D5,(bits & 0x2)); 
	digitalWrite (LCD_D6,(bits & 0x4)); 
	digitalWrite (LCD_D7,(bits & 0x8)); 
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
	SetChrMode();
	while(*s)
	lcd_byte(*s++);
	SetCmdMode();
}
/**
* Init GPIO PIN
**/
void initPins(){
	wiringPiSetup () ; // use wiring Pi numbering
	// set up pi pins for output
	pinMode (LCD_E,  OUTPUT);
	pinMode (LCD_RS, OUTPUT);
	pinMode (LCD_D4, OUTPUT);
	pinMode (LCD_D5, OUTPUT);
	pinMode (LCD_D6, OUTPUT);
	pinMode (LCD_D7, OUTPUT);
	pinMode (RELAY_IN, OUTPUT);
	pinMode (RELAY_IN2, OUTPUT);
	pinMode (TRANSISTOR, OUTPUT);
}
/**
Init the screen
**/
void lcd_init()
{
	#ifndef PROD
	printf("Full reset LCD\n");
	#endif
	SetCmdMode(); // set for commands
	lcd_byte(0x33); // full init
	lcd_byte(0x32); // 4 bit mode
	lcd_byte(0x28); // 2 line mode
	lcd_byte(0x0C); // display on, cursor off, blink off
	lcd_byte(0x01);  // clear screen
	delay(10);        // clear screen is slow!
}
/*
Return at the begining of the line
*/
void goHome(){
	SetCmdMode(); // set for commands
	lcd_byte(lcd_Home);  // go home screen
	delay(7);        // You cannot write directly after putting the cursor at the 
	//beginning of the line. You have to wait for some time before it
}
/**
 * This method is called periodically to have the display in a correct state
 no matter the random interference that could happen from time to time.
 **/
void resetLcd(){
	#ifndef PROD
	printf("Reset LCD\n");
	#endif

	SetCmdMode(); // set for commands
	lcd_byte(0x0C); // display on, cursor off, blink off
	lcd_byte(0x01);  // clear screen
	delay(10);        // clear screen is slow!

}
