#ifndef LCD_H
#define LCD_H

#define LCD_E 23
#define LCD_RS 22
#define LCD_D4 24
#define LCD_D5 25
#define LCD_D6 21
#define LCD_D7 26
#define RELAY_IN 7
#define RELAY_IN2 1
#define TRANSISTOR 27

// LCD instructions 
#define lcd_Clear 0b00000001 // replace all characters with ASCII 'space' 
#define lcd_Home 0b00000010 // return cursor to first position on first line 
#define lcd_EntryMode 0b00000110 // shift cursor from left to right on read/write 
#define lcd_DisplayOff 0b00001000 // turn display off 
#define lcd_DisplayOn 0b00001100 // display on, cursor off, don't blink character 
#define lcd_FunctionReset 0b00110000 // reset the LCD 
#define lcd_FunctionSet4bit 0b00101000 // 4-bit data, 2-line display, 5 x 7 font 
#define lcd_SetCursor 0b10000000 // set cursor position 

void pulseEnable ();
void lcd_byte(char bits);
void SetCmdMode();
void SetChrMode();
void lcd_text(char *s);
void lcd_init();
void goHome();
void resetLcd();
#endif