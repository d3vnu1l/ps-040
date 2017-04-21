#include "utilities.h"
#include <xc.h>
#include <p33EP512GM310.h>
#include <libpic30.h>
#include <dsp.h>
#include <stdio.h>
#include <math.h>
#include "common.h"
#include "sounds.h"
#include "audio.h"
#include "max7219.h"


//CONTROL VARIABLES//
extern char pad[8];
extern fractional pots[4];
extern fractional pots_scaled[4];
extern unsigned char UART_ON;

//STATUS VARIABLES//
extern unsigned char hard_clipped;
extern unsigned char UART_EN;

extern fractional sample;

//FX FLAGS & VARS
extern unsigned char tremelo, looper, lpf;
extern unsigned int tremelo_ptr, tremelo_length, trem_var;
extern unsigned int loop_lim;
extern fractional lpf_alpha, lpf_inv_alpha;
extern fractional tremelo_depth;

void scanMatrix(void){
    static unsigned char pad_last[8] = {1,1,1,1,1,1,1,1};
    MAX7219_DisplayChar('0', 'A');
    pad[0]=p0;
    pad[1]=p1;
    pad[2]=p2;
    pad[3]=p3;
    pad[4]=p4;
    pad[5]=p5;
    pad[6]=p6;
    pad[7]=p7;
    if(pad[4]==0&&pad_last[4]==1){                                              //TREMELO CONTROL
        pad_last[4]=0;
        if(tremelo==FALSE)
            tremelo=TRUE;
        else tremelo=FALSE;
        TREMELO_LED=tremelo;
    }
    else{
        pad_last[4]=pad[4];
    }
   
    if(pad[5]==0){                                                              //LOOPER CONTROL
        looper=TRUE;
        YLED=looper;
    }
    else {
        looper=FALSE;
        YLED=looper;
    }
   
    if(pad[6]==0&&pad_last[6]==1){                                              //LPF CONTROL
        pad_last[6]=0;
        if(lpf==FALSE)
            lpf=TRUE;
        else lpf=FALSE;
        GLED=lpf;
    }
    else{
        pad_last[6]=pad[6];
    }
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
    if(pots[2]<=2048)
        pots_scaled[2]=8;
    else if(pots[2]<=4096)
        pots_scaled[2]=12;
    else if(pots[2]<=6144)
        pots_scaled[2]=14;
    else if(pots[2]<=8192)
        pots_scaled[2]=16;
    else if(pots[2]<=10240)
        pots_scaled[2]=19;
    else if(pots[2]<=12288)
        pots_scaled[2]=23;
    else if(pots[2]<=14336)
        pots_scaled[2]=28;
    else if(pots[2]<=16384)
        pots_scaled[2]=32;
    else if(pots[2]<=18432)
        pots_scaled[2]=36;
    else if(pots[2]<=20480)
        pots_scaled[2]=40;
    else if(pots[2]<=22528)
        pots_scaled[2]=44;
    else if(pots[2]<=24576)
        pots_scaled[2]=48;
    else if(pots[2]<=26624)
        pots_scaled[2]=52;
    else if(pots[2]<=28672)
        pots_scaled[2]=56;
    else if(pots[2]<=30720)
        pots_scaled[2]=60;
    else if(pots[2]<=32768)
        pots_scaled[2]=64;
    else if(pots[2]<=34816)
        pots_scaled[2]=68;
    else 
        pots_scaled[2]=72;
                                                                                 
    loop_lim=154*pots_scaled[2];                                                //LOOPER CONTROL
    if(pots[1]>=310){                                                           //LPF CONTROL
        lpf_alpha=pots[1];
        lpf_inv_alpha=(32767-lpf_alpha); 
    }
    //tremelo_depth=pots[4];
            
}

void display(void){
   SLED=~SLED;
    if(hard_clipped==TRUE){                                                     //CLIP CONTROL    
        HARD_CLIP_LED=1;
        hard_clipped=FALSE;
    } else HARD_CLIP_LED=0;
    if(UART_ON==TRUE){
        //IC1CON2bits.TRIGSTAT = ~pad[4]; 
        //printf("b1 %d, b2 %d, b3 %d, b4 %d\r\n", pad[0], pad[1], pad[2], pad[3]);
        //printf("b4 %d, b5 %d, b6 %d, b7 %d\r\n", pad[4], pad[5], pad[6], pad[7]);
        //printf("P1 %x  P1 %d bpm %d\r\n", pots[0], pots[0], bpm);   //check pots
        //printf("P1 %d  P2 %d P3 %d\r\n", pots[0], pots[1], pots[2]);   //check pots
        printf("%d\r\n", sample);  //check input ADC
        //printf("%d, pot1 %x, pot2 %x, avg %x\r\n", sample, pots[1], pots[2], average);  //check input ADC
    }
}
