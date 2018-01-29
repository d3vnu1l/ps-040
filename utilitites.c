#include "utilities.h"
#include <xc.h>
#include <p33EP512GM310.h>
#include <libpic30.h>
#include <dsp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "common.h"
#include "sounds.h"
#include "audio.h"
#include "plcd.h"

#include "devInits.h"


//CONTROL VARIABLES//
extern unsigned char pad[BUTTONS];
extern fractional pots[POTS];
extern fractional pots_scaled[POTS];
extern unsigned char UART_ON; 
extern int temp1, temp2;

extern char flash_readback[512];

//STATUS VARIABLES//
extern unsigned char hard_clipped;
extern unsigned char UART_EN;
extern unsigned int cycle;
extern unsigned char TEST_SIN;
extern fractional sampin;
extern fractional sampout;

//FX FLAGS & VARS
extern unsigned char tremelo, looper, lpf;
extern unsigned int tremelo_ptr, tremelo_length, trem_var;
extern unsigned int loop_lim;
extern fractional lpf_alpha, lpf_inv_alpha;
extern fractional tremelo_depth;
extern unsigned char kick_playing, snare_playing;   
extern unsigned char frame;

int  scanCounter=0;

void scanMatrix(void){
    static unsigned char pad_last[BUTTONS]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    int portrdG, portrdD, portrdF;

    portrdG = PORTG;
    portrdD = PORTD;
    portrdF = PORTF;
    
    pad[0]=(portrdG)&1;
    pad[1]=(portrdG>>1)&1;
    pad[2]=(portrdG>>2)&1;
    pad[3]=(portrdG>>3)&1;
    pad[17]=(portrdG>>10)&1;    // Encoder button
    pad[11]=(portrdG>>11)&1;
    pad[12]=(portrdG>>12)&1;
    pad[13]=(portrdG>>13)&1;
    pad[14]=(portrdG>>14)&1;
    pad[15]=(portrdG>>15)&1;
    
    pad[4]=(portrdF>>4)&1;
    pad[5]=(portrdF>>5)&1;
    pad[6]=(portrdF>>6)&1;
    pad[16]=(portrdF>>7)&1;     // Special function button
    
    pad[7]=(portrdD>>1)&1;
    pad[8]=(portrdD>>2)&1;
    pad[9]=(portrdD>>3)&1;
    pad[10]=(portrdD>>4)&1;
    

   
    
    if(pad[13]==0&&pad_last[13]==1){                                              //TREMELO CONTROL
        pad_last[13]=0;
        if(tremelo==FALSE)
            tremelo=TRUE;
        else tremelo=FALSE;
    }
    else{
        pad_last[13]=pad[13];
    }
    
    if(pad[14]==0) looper=TRUE;
    else looper=FALSE;
   
    if(pad[15]==0&&pad_last[15]==1){                                              //LPF CONTROL
        pad_last[15]=0;
        if(lpf==FALSE)
            lpf=TRUE;
        else lpf=FALSE;
    }
    else{
        pad_last[15]=pad[15];
    }
    
    
    // SAMPLE TRIGGERS 
    if(pad[0]==0&&kick_playing==FALSE){                                         //kick
        kick_playing=TRUE;
    }
    /*
    if(pad[2]==0&&hat_playing==FALSE){                                          //hat
        hat_playing=TRUE;
    }
    */
    if(pad[1]==0&&snare_playing==FALSE){                                        //snare
        snare_playing=TRUE;
    }
}

void readPots(void){
    volatile register int scaled asm("A");
    _AD1IF = 0; // Clear conversion done status bit
    
    pots[0]=(ADC1BUF0>>1)|0x7;
    pots[1]=(ADC1BUF1>>1)|0x7;
    pots[2]=(ADC1BUF2>>1)|0x7;
    pots[3]=(ADC1BUF3>>1)|0x7;
    pots[4]=(ADC1BUF4>>1)|0x7;
    pots[5]=(ADC1BUF5>>1)|0x7;
    
    loop_lim=pots_scaled[5];                                                //LOOPER CONTROL
    if(pots[0]>=310){                                                           //LPF CONTROL
        lpf_alpha=pots[0];
        lpf_inv_alpha=(32767-lpf_alpha); 
    }
    tremelo_depth=pots[1];
    
    scaled=__builtin_mpy(pots[5],Q15(0.33), NULL, NULL, 0, NULL, NULL, 0);
    pots_scaled[5]=__builtin_sac(scaled, 0);
}

void display(void){
    IFS0bits.SPI1IF=0;
    SPI1STATbits.SPIROV = 0;
   
    
   lcdDrawPads(16);
   
   lcdSetCursorQ(0,0);
   lcdWriteWordQ(flash_readback[0]);
    lcdSetCursorQ(7,0);
   lcdWriteWordQ(flash_readback[1]);
    lcdSetCursorQ(0,1);
   lcdWriteWordQ(flash_readback[2]);
    lcdSetCursorQ(7,1);
   lcdWriteWordQ(flash_readback[3]);
    lcdSetCursorQ(0,2);
   lcdWriteWordQ(flash_readback[4]);
    lcdSetCursorQ(7,2);
   lcdWriteWordQ(flash_readback[5]);
   
   /*
   lcdSetCursorQ(0,3);
   lcdWriteStringQ("Stat:");
    SS3L=0; 
    char trash=SPI3BUF; 
    SPI3BUF=0x05; 
    while(!_SPI3IF); _SPI3IF=0; 
    trash=SPI3BUF; 
    SPI3BUF=0x00; 
    while(!_SPI3IF); _SPI3IF=0; 
    temp1=SPI3BUF; 
    SPI3BUF=0x00; 
    while(!_SPI3IF); _SPI3IF=0; 
    temp2=SPI3BUF; 
    SS3L=1; 
    lcdWriteWordQ(temp1);
    */ 

    /*
    lcdSetCursorQ(0,3);
    if(hard_clipped==TRUE){                                                     //CLIP CONTROL    
        lcdWriteStringQ("CLIP");
        hard_clipped=FALSE;  
    }
    else if(TEST_SIN==TRUE)lcdWriteStringQ("SINE");
    else lcdWriteStringQ("THRU");
    */
   
   lcdSetCursorQ(0,3);
   lcdWriteWordQ(ENCODERCNTH);
   lcdWriteWordQ(ENCODERCNTL);
   
   if(UART_ON==TRUE){
        //IC1CON2bits.TRIGSTAT = ~pad[4]; 
        //printf("b1 %d, b2 %d, b3 %d, b4 %d\r\n", pad[0], pad[1], pad[2], pad[3]);
        //printf("b4 %d, b5 %d, b6 %d, b7 %d\r\n", pad[4], pad[5], pad[6], pad[7]);
        //printf("P1 %x  P1 %d bpm %d\r\n", pots[0], pots[0], bpm);   //check pots
        printf("P1 %d  P2 %d P3 %d\r\n", pots[0], pots[1], pots[2]);   //check pots
        //printf("%d\r\n", sample);  //check input ADC
        //printf("%d, pot1 %x, pot2 %x, avg %x\r\n", sample, pots[1], pots[2], average);  //check input ADC
    }
   
   SLED=~SLED;
}

void processRxData(fractional *sourceBuffer, fractional *targetBuffer){
    /* This procedure loops back the received data to the*/
    /* the codec output. The user application could process*/
    /* this data as per application requirements.*/
    int index;
    for(index = 0;index < STREAMBUF;index ++)
    {
        targetBuffer[index] = sourceBuffer[index];
    }
}

//A blocking delay function. Not very accurate but good enough.
void Delay_us(unsigned int delay)
{
    int i;
    for (i = 0; i < delay; i++)
    {
        __asm__ volatile ("repeat #50");
        __asm__ volatile ("nop");
    }
}