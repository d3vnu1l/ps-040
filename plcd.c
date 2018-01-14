/*
 * File:   plcd.c
 * Author: root
 *
 * Created on January 10, 2018, 4:48 PM
 */


#include "xc.h"
#include "plcd.h"
#include "common.h"
#include "utilities.h"

unsigned int statusReg = 0x0C;  //internal copy of pwr reg
                                // 0b_0 0 0 0 - 1 D C B (display, cursor, blink)
char lcdBuf[80];

void lcdWrite(unsigned char data){
    if(!PMMODEbits.BUSY) PMDIN1=data; //(!skips if busy!)
}

void lcdClear(void){
    lcdWrite(LCD_CLEARDISPLAY);
}

void lcdReturn(void){
    lcdWrite(LCD_RETURNHOME);
}


void lcdPwr(signed int pwr){
    //lcd soft power on/off, does not reset device
    if(pwr)
        statusReg&=0x04;
    else __builtin_btg(&statusReg, 2);
    
    lcdWrite(statusReg);
}

void lcdCursorEn(signed int pwr){
    //cursor on / off
    if(pwr)
        statusReg&=0x0A;
    else __builtin_btg(&statusReg, 1);
    
    lcdWrite(statusReg);
}

void lcdCursorBlink(signed int pwr){
    if(pwr)
        statusReg&=0x09;
    else __builtin_btg(&statusReg, 0);
    
    lcdWrite(statusReg);
}

//BLOCKING!
void lcdWriteString(char *string) {
    char *it = string;
  for (; *it; it++) {
    lcdWrite(*it);
    Delay_us(50);
  }
}

void lcdSetupPots(){
    char *message = "Pot#1:    "
                    "Pot#2:    "
                    "          "
                    "          "
                    "Pot#3:    " 
                    "Pot#4:    ";
    lcdWriteString(message);
}

void lcdCustomSymbols(void){
    int i=0;
    lcdWrite(0x40);
    LCD_RS=1;
    Delay_us(200);
   // for(; i<8; i++)lcdWriteString(loadingOne[i]);
    //lcdWriteString(loadingTwo);
    //lcdWriteString(loadingThree);
    //lcdWriteString(loadingFour);
    LCD_RS=0;
}

void lcdInit(void){
    lcdWrite(0x38);                     //function set, 8 bits, 2 line disp, 5x8
    Delay_us(4500);                     //>4.1 mS required
    if(PMMODEbits.BUSY);  PMDIN1=0x0F;  //display on, cursor on, blink on
    Delay_us(4500);
    lcdClear();                         //Display Clear  
    Delay_us(1800);                     //>1.64mS required
    lcdWrite(0x06);                     // entry Mode Set
    Delay_us(200);
    lcdReturn();
    Delay_us(1500);
    
    //lcdCustomSymbols();
    Delay_us(200);
    lcdClear();                         //Display Clear  
    Delay_us(1500);
    lcdReturn();
    Delay_us(200);
    LCD_RS=1;
}

void lcdSetCursor(unsigned char col, unsigned char row) {
  static unsigned char offsets[] = { 0x00, 0x40, 0x14, 0x54 };

  if (row > 1) {
    row = 1;
  }

  lcdWrite(LCD_SETDDRAMADDR | (col + offsets[row]));
}
/*
 char loadingOne[8] = {
	0b10000,
	0b10000,
	0b10000,
	0b10000,
	0b10000,
	0b10000,
	0b10000,
	0b10000
};

char loadingTwo[8] = {
	0b11000,
	0b11000,
	0b11000,
	0b11000,
	0b11000,
	0b11000,
	0b11000,
	0b11000
};

char loadingThree[8] = {
	0b11100,
	0b11100,
	0b11100,
	0b11100,
	0b11100,
	0b11100,
	0b11100,
	0b11100
};

char loadingFour[8] = {
	0b11110,
	0b11110,
	0b11110,
	0b11110,
	0b11110,
	0b11110,
	0b11110,
	0b11110
};
 */

