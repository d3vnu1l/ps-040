/*
 * File:   plcd.c
 */
#include <xc.h>
#include <stdlib.h> // For splash rand
#include <dsp.h>
#include "plcd.h"
#include "common.h"
#include "screens.h"
#include "utilities.h"

extern struct ctrlsrfc ctrl;
extern struct sflags stat;

int lcdBuf[LCDBUF+1]={0};
int *lcdWritePtr=lcdBuf;
int *lcdReadPtr=lcdBuf;

int __attribute__((space(psv))) log2Meter[LOGMETER_SIZE] = {
    0, 5, 8, 11, 12, 14, 15, 16, 17, 18, 18, 19, 20, 20, 21, 21, 22, 22, 23, 
    23, 23, 24, 24, 24, 25, 25, 25, 26, 26, 26, 26, 27, 27, 27, 27, 28, 28, 
    28, 28, 28, 29, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 31, 31, 31, 
    31, 31, 31, 31, 32, 32, 32, 32, 32};


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
    const int limit = 20;
    char *it = string;
    int i=0;
    
    for (; *it; it++) {
        lcdWriteQMac(*it);
        if(i++==(limit-1)) return;
    }
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

void lcdWriteByteQ(char word){
    int i;
    char inchar[2];
     
   inchar[0] = word&0x0F; 
   if (inchar[0] > 9) 
       inchar[0]+=55;
   else inchar[0]+=48;
   
   for(i=1; i<2; i++){ 
      inchar[i] = ((word>>(i*4))&0x0000F); 
      if (inchar[i] > 9) 
          inchar[i]+=55;
      else inchar[i]+=48;
   } 
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

void lcdWriteDecimalQ(int word, int digits){
    const char maxdigits = 8;
    char result[maxdigits];
    char i = 0;
    do {
        result[i] = '0' + word % 10;
        word /= 10;
        i++;
    }
    while ((word > 0)&&(i<digits));
    while (i<digits) result[i++] = ' '; 
  
    for (i=(digits-1); i>=0; i--) {
        lcdWriteQMac(result[i]);
    }
}

//4x4 pad debug
void lcdDrawPads(unsigned char col){
    unsigned char block=0xFF;
    if(ctrl.pad[BTN_SPECIAL]>1||ctrl.pad[BTN_ENC]>1) block='*';
    
    lcdSetCursorQ(col, 0);
    if(ctrl.pad[12]>1){lcdWriteQMac(' ');} else lcdWriteQMac(block);
    if(ctrl.pad[13]>1){lcdWriteQMac(' ');} else lcdWriteQMac(block);
    if(ctrl.pad[14]>1){lcdWriteQMac(' ');} else lcdWriteQMac(block);
    if(ctrl.pad[15]>1){lcdWriteQMac(' ');} else lcdWriteQMac(block);
    lcdSetCursorQ(col, 1);
    if(ctrl.pad[8]>1){lcdWriteQMac(' ');} else lcdWriteQMac(block);
    if(ctrl.pad[9]>1){lcdWriteQMac(' ');} else lcdWriteQMac(block);
    if(ctrl.pad[10]>1){lcdWriteQMac(' ');} else lcdWriteQMac(block);
    if(ctrl.pad[11]>1){lcdWriteQMac(' ');} else lcdWriteQMac(block);
    lcdSetCursorQ(col, 2);
    if(ctrl.pad[4]>1){lcdWriteQMac(' ');} else lcdWriteQMac(block);
    if(ctrl.pad[5]>1){lcdWriteQMac(' ');} else lcdWriteQMac(block);
    if(ctrl.pad[6]>1){lcdWriteQMac(' ');} else lcdWriteQMac(block);
    if(ctrl.pad[7]>1){lcdWriteQMac(' ');} else lcdWriteQMac(block);
    lcdSetCursorQ(col, 3);
    if(ctrl.pad[0]>1){lcdWriteQMac(' ');} else lcdWriteQMac(block);
    if(ctrl.pad[1]>1){lcdWriteQMac(' ');} else lcdWriteQMac(block);
    if(ctrl.pad[2]>1){lcdWriteQMac(' ');} else lcdWriteQMac(block);
    if(ctrl.pad[3]>1){lcdWriteQMac(' ');} else lcdWriteQMac(block);
}

void lcdDrawMeter(unsigned char col){
    volatile register int resultA asm("A");
    const fractional scale = Q15(0.00195318460646); // Scales value between 0-64 for lookup
    int i;
    const int divs=32, row=8;
    int scaledPower;
    int remainder;
    
                    
    resultA =__builtin_mpy(scale, stat.power, NULL, NULL, 0, NULL, NULL, 0);
    scaledPower=__builtin_sac(resultA, 0);
    scaledPower=log2Meter[scaledPower];
    
    if(stat.hard_clipped==TRUE){
        lcdSetCursorQ(col, 0);
        lcdWriteQMac('C');
        lcdSetCursorQ(col, 1);
        lcdWriteQMac('L');
        lcdSetCursorQ(col, 2);
        lcdWriteQMac('I');
        lcdSetCursorQ(col, 3);
        lcdWriteQMac('P');
        stat.hard_clipped=FALSE;
    }
    else {
        for(i=3; i>=0; i--){
            lcdSetCursorQ(col, i);
            if(scaledPower<=0){ 
                lcdWriteQMac(' ');
            }   
            else if(scaledPower>=(divs-i*row)){
                lcdWriteQMac(0xFF);
            }
            else if(scaledPower<(divs-i*row)){
                lcdWriteQMac((char)(0xFF&scaledPower));
            }
            else{
                remainder = scaledPower%row;
                 if(remainder==0){ 
                    lcdWriteQMac(' ');
                } 
                else{
                    lcdWriteQMac((char)(0xFF&remainder));
                }
            }
            scaledPower-=row;
        }
    }
  
    stat.power_ack=TRUE;
}

void lcdDrawSlots(unsigned char col, unsigned char grid[16]){
    
    lcdSetCursorQ(col, 0);
    if(grid[12]){lcdWriteQMac('*');} else lcdWriteQMac(0xFF);
    if(grid[13]){lcdWriteQMac('*');} else lcdWriteQMac(0xFF);
    if(grid[14]){lcdWriteQMac('*');} else lcdWriteQMac(0xFF);
    if(grid[15]){lcdWriteQMac('*');} else lcdWriteQMac(0xFF);
    lcdSetCursorQ(col, 1);
    if(grid[8]){lcdWriteQMac('*');} else lcdWriteQMac(0xFF);
    if(grid[9]){lcdWriteQMac('*');} else lcdWriteQMac(0xFF);
    if(grid[10]){lcdWriteQMac('*');} else lcdWriteQMac(0xFF);
    if(grid[11]){lcdWriteQMac('*');} else lcdWriteQMac(0xFF);
    lcdSetCursorQ(col, 2);
    if(grid[4]){lcdWriteQMac('*');} else lcdWriteQMac(0xFF);
    if(grid[5]){lcdWriteQMac('*');} else lcdWriteQMac(0xFF);
    if(grid[6]){lcdWriteQMac('*');} else lcdWriteQMac(0xFF);
    if(grid[7]){lcdWriteQMac('*');} else lcdWriteQMac(0xFF);
    lcdSetCursorQ(col, 3);
    if(grid[0]){lcdWriteQMac('*');} else lcdWriteQMac(0xFF);
    if(grid[1]){lcdWriteQMac('*');} else lcdWriteQMac(0xFF);
    if(grid[2]){lcdWriteQMac('*');} else lcdWriteQMac(0xFF);
    if(grid[3]){lcdWriteQMac('*');} else lcdWriteQMac(0xFF);
}

void lcdDrawSplash(void){
    int i, j, k, size=20;
    int frames = Fdisp/2;               // About 1/2 second
    unsigned char splash[4][size];
    //unsigned char* ptrA = splash;
    
    i=0;
    while(i<frames){
        if(_T1IF){    
            _T1IF=0;
            for(j=0; j<4; j++){
                for(k=0; k<size; k++){
                    if(j>=0 && j<size){
                        srand(i+j+k+TMR1);
                        unsigned char randchar = (unsigned char)(rand() % 93 + 33);
                        splash[j][k]=randchar;
                    }
                    else splash[j][k] = ' ';
                }
            }
            lcdClearQ(); 
            lcdSetCursorQ(0,0);
            lcdWriteStringQ(&splash[3][0]);
            lcdSetCursorQ(0,1);
            lcdWriteStringQ(&splash[2][0]);
            lcdSetCursorQ(0,2);
            lcdWriteStringQ(&splash[1][0]);
            lcdSetCursorQ(0,3);
            lcdWriteStringQ(&splash[0][0]);
            //ptrA++;
            i++;
        }
        if(_T5IF) {
            lcdPoll();
            _T5IF=0;
        }
        Delay_us(60);
    }
}

void lcdPoll(void){  
    if(lcdWritePtr!=lcdReadPtr){   
        if((*lcdReadPtr>>8)&1) LCD_RS=0;
        else LCD_RS=1;
        if((*lcdReadPtr>>9)&1) PR5=0x2DF0; //2DF0 for ~1.3mS
        else PR5=0x0120; //120 for ~40uS
        PMDIN1=(*lcdReadPtr++)&0x00FF;
        if(lcdReadPtr==&lcdBuf[LCDBUF]) lcdReadPtr=lcdBuf;
    } 
    TMR5=0x0000; 
}

//ALWAYS SENDS, blocks program
void lcdBlockingSend(unsigned char data){
    while(PMMODEbits.BUSY)Delay_us(2);
    PMDIN1=data; 
}

void lcdBlockingCommand(unsigned char data){
    LCD_RS=0;
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

void lcdCustomSymbols(void){
    unsigned char customChars[8][8];
    const unsigned char line = 0b11111;
    int i, j;
    // Generate custom characters
    for(i=0; i<8; i++){
        for(j=0; j<8; j++){
            if(j<=i) 
                customChars[i][7-j] = line;
            else 
                customChars[i][7-j] = 0x00;
        }
    }
    
    // Send custom chars
    lcdBlockingCommand(0x40);
    Delay_us(200);
    LCD_RS=1;
    for(i=0; i<8; i++){
        for(j=0; j<8; j++){
            lcdBlockingSend(customChars[i][j]);
            Delay_us(200);
        }
    }
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

