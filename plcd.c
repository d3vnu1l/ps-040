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
int  lcdBuf[LCDBUF]={0};
int *lcdWritePtr=lcdBuf;
int *lcdSendPtr=lcdBuf;


void lcdWriteQ(unsigned char data){


    *lcdWritePtr++=data|0x0000;
    if(lcdWritePtr==&lcdBuf[LCDBUF])
        lcdWritePtr=lcdBuf;
    //if(!LCD_RS_P)LCD_RS=1;
    //if(!PMMODEbits.BUSY) PMDIN1=data; //(!skips if busy!)
}

void lcdCommandQ(unsigned char data){


    *lcdWritePtr++=data|0x0100;
    if(lcdWritePtr==&lcdBuf[LCDBUF])
        lcdWritePtr=lcdBuf;
    
    //if(LCD_RS_P)LCD_RS=0;
    //if(!PMMODEbits.BUSY) PMDIN1=data; //(!skips if busy!)
}

void lcdClearQ(void){
    *lcdWritePtr++=(LCD_CLEARDISPLAY)|0x0300;  //flag for rs=0 & long delay
}

void lcdReturnQ(void){
    *lcdWritePtr++=(LCD_RETURNHOME)|0x0300;  //flag for rs=0 & long delay
}

void lcdSetCursorQ(unsigned char col, unsigned char row) {
  static unsigned char offsets[] = { 0x00, 0x40, 0x14, 0x54 };
  if (row > 3) {
    row = 3;
  }

  *lcdWritePtr++=(LCD_SETDDRAMADDR | (col + offsets[row]))|0x0100;  //flag for rs=0
    if(lcdWritePtr==&lcdBuf[LCDBUF])
        lcdWritePtr=lcdBuf;
}

void lcdPwrQ(signed int pwr){
    //lcd soft power on/off, does not reset device
    if(pwr)
        statusReg&=0x04;
    else __builtin_btg(&statusReg, 2);
    
    lcdCommandQ(statusReg);
}

void lcdCursorEn(signed int pwr){
    //cursor on / off
    if(pwr)
        statusReg&=0x0A;
    else __builtin_btg(&statusReg, 1);
    
    lcdCommandQ(statusReg);
}

void lcdCursorBlinkQ(signed int pwr){
    if(pwr)
        statusReg&=0x09;
    else __builtin_btg(&statusReg, 0);
    
    lcdCommandQ(statusReg);
}

void lcdWriteStringQ(char *string) {
    char *it = string;
    for (; *it; it++) {
        lcdWriteQ(*it);
  }
}

void lcdSetupPots(){
    lcdSetCursorQ(0,0);
    lcdWriteStringQ("P1:");
    lcdSetCursorQ(6,0);
    lcdWriteStringQ("P2:");
    lcdSetCursorQ(12,0);
    lcdWriteStringQ("P3:");
    lcdSetCursorQ(0,1);
    lcdWriteStringQ("P4:");
    lcdSetCursorQ(6,1);
    lcdWriteStringQ("P5:");
    lcdSetCursorQ(12,1);
    lcdWriteStringQ("P6:");
    lcdSetCursorQ(0,2);
    lcdWriteStringQ("I:");
    lcdSetCursorQ(8,2);
    lcdWriteStringQ("O:");
    lcdSetCursorQ(0,3);
    lcdWriteStringQ("Cyc:");
    lcdSetCursorQ(11,3);
    if(TEST_SIN==TRUE)lcdWriteStringQ("SINE");
    else lcdWriteStringQ("THRU");
    
}

void lcdCustomSymbols(void){
    int i=0;
    lcdCommandQ(0x40);
    Delay_us(200);
   // for(; i<8; i++)lcdWriteString(loadingOne[i]);
    //lcdWriteString(loadingTwo);
    //lcdWriteString(loadingThree);
    //lcdWriteString(loadingFour);
}

void lcdWriteWord(int word){
    int i;
    char inchar[4];
    
    if(word<0) {
        lcdWriteQ('-');
        word=~word+1;
    }else lcdWriteQ(' ');
   
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
   lcdWriteQ(inchar[3]);
   lcdWriteQ(inchar[2]);
   lcdWriteQ(inchar[1]);
   lcdWriteQ(inchar[0]);
}

void lcdVUvertical(unsigned char col, unsigned char row, int data){
    lcdSetCursorQ(col,row);
}

//4x4 pad debug
void lcdDrawPads(unsigned char col){
    lcdSetCursorQ(col, 0);
    if(!pad[12])lcdWriteStringQ(" "); else {lcdWriteQ(0xFF);}
    if(!pad[13])lcdWriteStringQ(" "); else {lcdWriteQ(0xFF);}
    if(!pad[14])lcdWriteStringQ(" "); else {lcdWriteQ(0xFF);}
    if(!pad[15])lcdWriteStringQ(" "); else {lcdWriteQ(0xFF);}
    lcdSetCursorQ(col, 1);
    if(!pad[8])lcdWriteStringQ(" "); else {lcdWriteQ(0xFF);}
    if(!pad[9])lcdWriteStringQ(" "); else {lcdWriteQ(0xFF);}
    if(!pad[10])lcdWriteStringQ(" "); else {lcdWriteQ(0xFF);}
    if(!pad[11])lcdWriteStringQ(" "); else {lcdWriteQ(0xFF);}
    lcdSetCursorQ(col, 2);
    if(!pad[4])lcdWriteStringQ(" "); else {lcdWriteQ(0xFF);}
    if(!pad[5])lcdWriteStringQ(" "); else {lcdWriteQ(0xFF);}
    if(!pad[6])lcdWriteStringQ(" "); else {lcdWriteQ(0xFF);}
    if(!pad[7])lcdWriteStringQ(" "); else {lcdWriteQ(0xFF);}
    lcdSetCursorQ(col, 3);
    if(!pad[0])lcdWriteStringQ(" "); else {lcdWriteQ(0xFF);}
    if(!pad[1])lcdWriteStringQ(" "); else {lcdWriteQ(0xFF);}
    if(!pad[2])lcdWriteStringQ(" "); else {lcdWriteQ(0xFF);}
    if(!pad[3])lcdWriteStringQ(" "); else lcdWriteQ(0xFF);
}

void lcdPoll(void){  
    if(lcdWritePtr!=lcdSendPtr){   
        if((*lcdSendPtr>>8)&1) LCD_RS=0;
        else LCD_RS=1;
        PMDIN1=(*lcdSendPtr++)&0x00FF;
        if(lcdSendPtr==&lcdBuf[LCDBUF]) lcdSendPtr=lcdBuf;
        if((*lcdSendPtr>>9)&1)
            PR3=0x2D00; //2D
        else PR3=0x0120; //120
    } 
    TMR3=0x0000;
    IFS0bits.T3IF = 0;              //clear flag, restart
}

//ALWAYS SENDS, blocks program
void lcdSend(unsigned char data){
    while(PMMODEbits.BUSY)Delay_us(2);
    PMDIN1=data; 
}
//ALWAYS SENDS, blocks program
void lcdClear(void){
    while(PMMODEbits.BUSY)Delay_us(2);
    PMDIN1=LCD_CLEARDISPLAY; 
}

//ALWAYS SENDS, blocks program
void lcdReturn(void){
    while(PMMODEbits.BUSY)Delay_us(2);
    PMDIN1=LCD_RETURNHOME;
}

void lcdInit(void){
    LCD_RS=0;
    lcdSend(0x38);              //function set, 8 bits, 2 line disp, 5x8
    Delay_us(4500);                //>4.1 mS required
    lcdSend(0x0C);              //display on, cursor on, blink on   (0f for blink+cursor)
    Delay_us(4500);
    lcdClear();                    //Display Clear  
    Delay_us(1800);                //>1.64mS required
    lcdSend(0x06);               // entry Mode Set
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
