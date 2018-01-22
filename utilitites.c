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
extern unsigned char pad[8];
extern fractional pots[4];
extern fractional pots_scaled[4];
extern unsigned char UART_ON; 

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

void scanMatrix(void){
    static unsigned char pad_last[17]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    int portrdG, portrdD, portrdF;
    //
    portrdG = PORTG;
    portrdD = PORTD;
    portrdF = PORTF;
    
   
    pad[0]=(portrdG)&1;
    pad[1]=(portrdG>>1)&1;
    pad[2]=(portrdG>>2)&1;
    pad[3]=(portrdG>>3)&1;
    pad[11]=(portrdG>>11)&1;
    pad[12]=(portrdG>>12)&1;
    pad[13]=(portrdG>>13)&1;
    pad[14]=(portrdG>>14)&1;
    pad[15]=(portrdG>>15)&1;
    
    pad[4]=(portrdF>>4)&1;
    pad[5]=(portrdF>>5)&1;
    pad[6]=(portrdF>>6)&1;
    pad[16]=(portrdF>>7)&1;
    
    pad[7]=(portrdD>>1)&1;
    pad[8]=(portrdD>>2)&1;
    pad[9]=(portrdD>>3)&1;
    pad[10]=(portrdD>>4)&1;
   
    
    if(pad[7]==0&&pad_last[7]==1){                                              //TREMELO CONTROL
        pad_last[7]=0;
        if(tremelo==FALSE)
            tremelo=TRUE;
        else tremelo=FALSE;
        TREMELO_LED=tremelo;
    }
    else{
        pad_last[7]=pad[7];
    }
    
    if(pad[4]==0){                                                              //LOOPER CONTROL
        looper=TRUE;
        YLED=looper;
    }
    else {
        looper=FALSE;
        YLED=looper;
    }
   
    if(pad[15]==0&&pad_last[15]==1){                                              //LPF CONTROL
        pad_last[15]=0;
        if(lpf==FALSE)
            lpf=TRUE;
        else lpf=FALSE;
    }
    else{
        pad_last[15]=pad[15];
    }
    
    
    /* SAMPLE TRIGGERS */
    if(pad[15]==0&&kick_playing==FALSE){                                         //kick
        kick_playing=TRUE;
    }
    /*
    if(pad[2]==0&&hat_playing==FALSE){                                          //hat
        hat_playing=TRUE;
    }
    */
    if(pad[2]==0&&snare_playing==FALSE){                                        //snare
        snare_playing=TRUE;
    }
    
    //lpf=TRUE;
    
    
}

void readPots(void){
    volatile register int scaled asm("A");
    
    AD1CON1bits.SAMP = 0;      // start sampling
    while (!_AD1IF);           //wait for conversions to complete
    _AD1IF = 0;                //clear status bit
    pots[0]=(ADC1BUF0>>1)+0x1F;
    pots[1]=(ADC1BUF1>>1)+0x1F;
    pots[2]=(ADC1BUF2>>1)+0x1F;
    pots[3]=(ADC1BUF3>>1)+0x1F;
    //pot scaling 
    
    if(pots[1]<=2048)
        pots_scaled[1]=8;
    else if(pots[1]<=4096)
        pots_scaled[1]=12;
    else if(pots[1]<=6144)
        pots_scaled[1]=14;
    else if(pots[1]<=8192)
        pots_scaled[1]=16;
    else if(pots[1]<=10240)
        pots_scaled[1]=19;
    else if(pots[1]<=12288)
        pots_scaled[1]=23;
    else if(pots[1]<=14336)
        pots_scaled[1]=28;
    else if(pots[1]<=16384)
        pots_scaled[1]=32;
    else if(pots[1]<=18432)
        pots_scaled[1]=36;
    else if(pots[1]<=20480)
        pots_scaled[2]=40;
    else if(pots[1]<=22528)
        pots_scaled[2]=44;
    else if(pots[1]<=24576)
        pots_scaled[1]=48;
    else if(pots[1]<=26624)
        pots_scaled[1]=52;
    else if(pots[1]<=28672)
        pots_scaled[1]=56;
    else if(pots[1]<=30720)
        pots_scaled[1]=60;
    else if(pots[1]<=32768)
        pots_scaled[1]=64;
    else if(pots[1]<=34816)
        pots_scaled[1]=68;
    else 
        pots_scaled[1]=72;
     
                                                                                 
    loop_lim=154*pots_scaled[1];                                                //LOOPER CONTROL
    if(pots[1]>=310){                                                           //LPF CONTROL
        lpf_alpha=pots[1];
        lpf_inv_alpha=(32767-lpf_alpha); 
    }
    //tremelo_depth=pots[4];
            
}

void display(void){
    IFS0bits.SPI1IF=0;
    SPI1STATbits.SPIROV = 0;
    int trash = SPI1BUF;
    SPI1BUF=0xFAAF;
    
    lcdDrawPads(16);
   lcdSetCursorQ(2,2);
   lcdWriteWord(sampin);
   lcdSetCursorQ(10,2);
   lcdWriteWord(sampout);
   lcdSetCursorQ(4,3);
    if(pad[14])lcdWriteWord(cycle);

    lcdSetCursorQ(11,3);
    if(hard_clipped==TRUE){                                                     //CLIP CONTROL    
        lcdWriteStringQ("CLIP");
        hard_clipped=FALSE;  
    }
    else if(TEST_SIN==TRUE)lcdWriteStringQ("SINE");
    else lcdWriteStringQ("THRU");
    

    
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