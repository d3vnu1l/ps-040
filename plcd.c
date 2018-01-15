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
    if(!LCD_RS_P)LCD_RS=1;
    if(!PMMODEbits.BUSY) PMDIN1=data; //(!skips if busy!)
}

void lcdCommand(unsigned char data){
    if(LCD_RS_P)LCD_RS=0;
    if(!PMMODEbits.BUSY) PMDIN1=data; //(!skips if busy!)
}

void lcdClear(void){
    lcdCommand(LCD_CLEARDISPLAY);
}

void lcdReturn(void){
    lcdCommand(LCD_RETURNHOME);
}

void lcdSetCursor(unsigned char col, unsigned char row) {
  static unsigned char offsets[] = { 0x00, 0x40, 0x14, 0x54 };

  if (row > 3) {
    row = 3;
  }
  lcdCommand(LCD_SETDDRAMADDR | (col + offsets[row]));
  Delay_us(50);
}

void lcdPwr(signed int pwr){
    //lcd soft power on/off, does not reset device
    if(pwr)
        statusReg&=0x04;
    else __builtin_btg(&statusReg, 2);
    
    lcdCommand(statusReg);
}

void lcdCursorEn(signed int pwr){
    //cursor on / off
    if(pwr)
        statusReg&=0x0A;
    else __builtin_btg(&statusReg, 1);
    
    lcdCommand(statusReg);
}

void lcdCursorBlink(signed int pwr){
    if(pwr)
        statusReg&=0x09;
    else __builtin_btg(&statusReg, 0);
    
    lcdCommand(statusReg);
}

//BLOCKING!
void lcdWriteString(char *string) {
    char *it = string;
  for (; *it; it++) {
    lcdWrite(*it);
    Delay_us(50);
  }
    Delay_us(40);
}

void lcdSetupPots(){
    lcdSetCursor(0,0);
    lcdWriteString("Pot#1:");
    lcdSetCursor(11,0);
    lcdWriteString("Pot#2:");
    lcdSetCursor(0,1);
    lcdWriteString("Pot#3:");
    lcdSetCursor(11,1);
    lcdWriteString("Pot#4:");
    lcdSetCursor(6,0);
    lcdWriteString("1234");
    lcdSetCursor(0,2);
    lcdWriteString("TEST______");
    lcdSetCursor(10,3);
    lcdWriteString("TEST______");
}

void lcdCustomSymbols(void){
    int i=0;
    lcdCommand(0x40);
    Delay_us(200);
   // for(; i<8; i++)lcdWriteString(loadingOne[i]);
    //lcdWriteString(loadingTwo);
    //lcdWriteString(loadingThree);
    //lcdWriteString(loadingFour);
}

void lcdInit(void){
    lcdCommand(0x38);              //function set, 8 bits, 2 line disp, 5x8
    Delay_us(4500);                //>4.1 mS required
    lcdCommand(0x0C);              //display on, cursor on, blink on   (0f for blink+cursor)
    Delay_us(4500);
    lcdClear();                    //Display Clear  
    Delay_us(1800);                //>1.64mS required
    lcdCommand(0x06);               // entry Mode Set
    Delay_us(200);
    lcdReturn();
    Delay_us(1500);
    
    //lcdCustomSymbols();
    Delay_us(200);
    lcdClear();                         //Display Clear  
    Delay_us(1500);
    lcdReturn();
    Delay_us(200);
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

