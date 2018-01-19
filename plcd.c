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

extern unsigned char TEST_SIN;
extern unsigned char pad[BUTTONS];

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
    lcdWriteString("P1:");
    lcdSetCursor(6,0);
    lcdWriteString("P2:");
    lcdSetCursor(12,0);
    lcdWriteString("P3:");
    lcdSetCursor(0,1);
    lcdWriteString("P4:");
    lcdSetCursor(6,1);
    lcdWriteString("P5:");
    lcdSetCursor(12,1);
    lcdWriteString("P6:");
    lcdSetCursor(0,2);
    lcdWriteString("I:");
    lcdSetCursor(8,2);
    lcdWriteString("O:");
    lcdSetCursor(0,3);
    lcdWriteString("Cyc:");
    lcdSetCursor(11,3);
    if(TEST_SIN==TRUE)lcdWriteString("SINE");
    else lcdWriteString("THRU");
    
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

void lcdWriteWord(int word){
    int i;
    char inchar[4];
    
    if(word<0) {
        lcdWrite('-');
        word=~word+1;
    }else lcdWrite(' ');
   Delay_us(40);
   
   inchar[0] = word&0x000F; 
   if (inchar[0] > 9) 
       inchar[0]+=55;
   else inchar[0]+=48;
   
   for(i=1; i<4; i++){ 
      inchar[i] = ((word>>(i*4))&0x0000F); 
      if (inchar[i] > 9) 
          inchar[i]+=55;
      else inchar[i]+=48;
   } 
   lcdWrite(inchar[3]);
   Delay_us(50);
   lcdWrite(inchar[2]);
    Delay_us(50);
   lcdWrite(inchar[1]);
   Delay_us(50);
   lcdWrite(inchar[0]);
   Delay_us(50);
}

void lcdVUvertical(unsigned char col, unsigned char row, int data){
    lcdSetCursor(col,row);
}

//4x4 pad debug
void lcdDrawPads(unsigned char col){
    lcdSetCursor(col, 0);
    if(pad[12])lcdWriteString(" "); else {lcdWrite(0xFF);Delay_us(50);}
    if(pad[13])lcdWriteString(" "); else {lcdWrite(0xFF);Delay_us(50);}
    if(pad[14])lcdWriteString(" "); else {lcdWrite(0xFF);Delay_us(50);}
    if(pad[15])lcdWriteString(" "); else {lcdWrite(0xFF);Delay_us(50);}
    lcdSetCursor(col, 1);
    if(pad[8])lcdWriteString(" "); else {lcdWrite(0xFF);Delay_us(50);}
    if(pad[9])lcdWriteString(" "); else {lcdWrite(0xFF);Delay_us(50);}
    if(pad[10])lcdWriteString(" "); else {lcdWrite(0xFF);Delay_us(50);}
    if(pad[11])lcdWriteString(" "); else {lcdWrite(0xFF);Delay_us(50);}
    lcdSetCursor(col, 2);
    if(pad[4])lcdWriteString(" "); else {lcdWrite(0xFF);Delay_us(50);}
    if(pad[5])lcdWriteString(" "); else {lcdWrite(0xFF);Delay_us(50);}
    if(pad[6])lcdWriteString(" "); else {lcdWrite(0xFF);Delay_us(50);}
    if(pad[7])lcdWriteString(" "); else {lcdWrite(0xFF);Delay_us(50);}
    lcdSetCursor(col, 3);
    if(pad[0])lcdWriteString(" "); else {lcdWrite(0xFF);Delay_us(50);}
    if(pad[1])lcdWriteString(" "); else {lcdWrite(0xFF);Delay_us(50);}
    if(pad[2])lcdWriteString(" "); else {lcdWrite(0xFF);Delay_us(50);}
    if(pad[3])lcdWriteString(" "); else lcdWrite(0xFF);
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

