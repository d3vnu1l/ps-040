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
#include "screens.h"

extern unsigned char pad[BUTTONS];
int lcdBuf[LCDBUF+1]={0};
int *lcdWritePtr=lcdBuf;
int *lcdReadPtr=lcdBuf;

// There are write and command macros in the header, make sure these match.
void lcdWriteQ(unsigned char data){
    *lcdWritePtr++=data|0x0000;
    if(lcdWritePtr==&lcdBuf[LCDBUF])
        lcdWritePtr=lcdBuf;
}

// There are write and command macros in the header, make sure these match.
void lcdCommandQ(unsigned char data){
    *lcdWritePtr++=data|0x0100;     //flag for rs=1
    if(lcdWritePtr==&lcdBuf[LCDBUF])
        lcdWritePtr=lcdBuf;
}

void lcdClearQ(void){
    *lcdWritePtr++=LCD_CLEARDISPLAY|0x0300;     //flag for rs=1 & long delay
    if(lcdWritePtr==&lcdBuf[LCDBUF])
        lcdWritePtr=lcdBuf;
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

void lcdWriteStringQ(char *string) {
    char *it = string;
    for (; *it; it++) {
        lcdWriteQMac(*it);
  }
}

void lcdCustomSymbols(void){
    int i=0;
    lcdCommandQMac(0x40);
    Delay_us(200);
   // for(; i<8; i++)lcdWriteString(loadingOne[i]);
    //lcdWriteString(loadingTwo);
    //lcdWriteString(loadingThree);
    //lcdWriteString(loadingFour);
}

void lcdWriteWordQ(int word){
    int i;
    char inchar[4];
    
    if(word<0) {
        lcdWriteQMac('-');
        word=~word+1;
    }else lcdWriteQMac(' ');
   
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
   lcdWriteQMac(inchar[3]);
   lcdWriteQMac(inchar[2]);
   lcdWriteQMac(inchar[1]);
   lcdWriteQMac(inchar[0]);
}

void lcdWriteWordUnsignedQ(unsigned int word){
    int i;
    char inchar[4];
    
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
   lcdWriteQMac(inchar[3]);
   lcdWriteQMac(inchar[2]);
   lcdWriteQMac(inchar[1]);
   lcdWriteQMac(inchar[0]);
}

void lcdWriteDecimalQ(char word, int digits){
    const char maxdigits = 4;
    char result[maxdigits];
    char i = 3;
    do {
        result[i] = '0' + word % 10;
        word /= 10;
        i--;
    }
    while (word > 0);
    while (i>=0) result[i--] = ' '; 
  
    for (i=maxdigits-digits; i<maxdigits; i++) {
        lcdWriteQMac(result[i]);
    }
}

//4x4 pad debug
void lcdDrawPads(unsigned char col){
    char block=0xFF;
    if(!pad[34]||!pad[16]) block='*';
    
    lcdSetCursorQ(col, 0);
    if(!pad[12])lcdWriteStringQ(" "); else lcdWriteQMac(block);
    if(!pad[13])lcdWriteStringQ(" "); else lcdWriteQMac(block);
    if(!pad[14])lcdWriteStringQ(" "); else lcdWriteQMac(block);
    if(!pad[15])lcdWriteStringQ(" "); else lcdWriteQMac(block);
    lcdSetCursorQ(col, 1);
    if(!pad[8])lcdWriteStringQ(" "); else lcdWriteQMac(block);
    if(!pad[9])lcdWriteStringQ(" "); else lcdWriteQMac(block);
    if(!pad[10])lcdWriteStringQ(" "); else lcdWriteQMac(block);
    if(!pad[11])lcdWriteStringQ(" "); else lcdWriteQMac(block);
    lcdSetCursorQ(col, 2);
    if(!pad[4])lcdWriteStringQ(" "); else lcdWriteQMac(block);
    if(!pad[5])lcdWriteStringQ(" "); else lcdWriteQMac(block);
    if(!pad[6])lcdWriteStringQ(" "); else lcdWriteQMac(block);
    if(!pad[7])lcdWriteStringQ(" "); else lcdWriteQMac(block);
    lcdSetCursorQ(col, 3);
    if(!pad[0])lcdWriteStringQ(" "); else lcdWriteQMac(block);
    if(!pad[1])lcdWriteStringQ(" "); else lcdWriteQMac(block);
    if(!pad[2])lcdWriteStringQ(" "); else lcdWriteQMac(block);
    if(!pad[3])lcdWriteStringQ(" "); else lcdWriteQMac(block);
}

void lcdPoll(void){  
    if(lcdWritePtr!=lcdReadPtr){   
        if((*lcdReadPtr>>8)&1) LCD_RS=0;
        else LCD_RS=1;
        if((*lcdReadPtr>>9)&1) PR3=0x2DF0; //2DF0 for ~1.3mS
        else PR3=0x0120; //120 for ~40uS
        PMDIN1=(*lcdReadPtr++)&0x00FF;
        if(lcdReadPtr==&lcdBuf[LCDBUF]) lcdReadPtr=lcdBuf;
    } 
    TMR3=0x0000; 
}

//ALWAYS SENDS, blocks program
void lcdBlockingSend(unsigned char data){
    while(PMMODEbits.BUSY)Delay_us(2);
    PMDIN1=data; 
}
//ALWAYS SENDS, blocks program
void lcdBlockingClear(void){
    while(PMMODEbits.BUSY)Delay_us(2);
    PMDIN1=LCD_CLEARDISPLAY; 
}

//ALWAYS SENDS, blocks program
void lcdBlockingReturn(void){
    while(PMMODEbits.BUSY)Delay_us(2);
    PMDIN1=LCD_RETURNHOME;
}

void lcdInit(void){
    LCD_RS=0;
    lcdBlockingSend(0x38);              //function set, 8 bits, 2 line disp, 5x8
    Delay_us(4500);                //>4.1 mS required
    lcdBlockingSend(0x0C);              //display on, cursor on, blink on   (0f for blink+cursor)
    Delay_us(4500);
    lcdBlockingClear();                    //Display Clear  
    Delay_us(1800);                //>1.64mS required
    lcdBlockingSend(0x06);               // entry Mode Set
    Delay_us(200);
    lcdBlockingReturn();
    Delay_us(1500);
    
    //lcdCustomSymbols();
    Delay_us(200);
    lcdBlockingClear();                         //Display Clear  
    Delay_us(1500);
    lcdBlockingReturn();
    Delay_us(200);
}
