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
#include "flash.h"
#include "screens.h"
#include "utilities.h"
#include "devInits.h"

fractional FXSCALE = Q15(NUMFX*0.000030518509476);

//CONTROL VARIABLES//
extern unsigned char pad[BUTTONS];
extern fractional pots[POTS];
extern fractional pots_scaled[POTS];
extern fractional pots_custom[POTS];
extern unsigned char UART_ON; 
extern enum screenStruc state;

//STATUS VARIABLES//
extern unsigned char hard_clipped;
extern unsigned char UART_EN;
extern unsigned char TEST_SIN;
extern fractional sampin;
extern fractional sampout;

extern enum fxStruct fxUnits[NUMFXUNITS];


//FX FLAGS & VARS
extern unsigned char kick_playing, snare_playing;   

void scanButtons(void){
    
    static unsigned char pad_last[BUTTONS]={1};
    int portrdG, portrdD, portrdF;

    portrdG = PORTG;
    portrdD = PORTD;
    portrdF = PORTF;
    
    pad[34]=(portrdF>>7)&1;     // Special function button
    
    if(pad[34]){
        pad[6]=(portrdF>>6)&1;
        pad[5]=(portrdF>>5)&1;
        pad[4]=(portrdF>>4)&1;
        pad[0]=(portrdG)&1;
        pad[1]=(portrdG>>1)&1;
        pad[2]=(portrdG>>2)&1;
        pad[3]=(portrdG>>3)&1;
        pad[16]=(portrdG>>10)&1;    // Encoder button
        pad[11]=(portrdG>>11)&1;
        pad[12]=(portrdG>>12)&1;
        pad[13]=(portrdG>>13)&1;
        pad[14]=(portrdG>>14)&1;
        pad[15]=(portrdG>>15)&1;
        pad[7]=(portrdD>>1)&1;
        pad[8]=(portrdD>>2)&1;
        pad[9]=(portrdD>>3)&1;
        pad[10]=(portrdD>>4)&1;
    } else {
        pad[23]=(portrdF>>6)&1;
        pad[22]=(portrdF>>5)&1;
        pad[21]=(portrdF>>4)&1;
        pad[17]=(portrdG)&1;
        pad[18]=(portrdG>>1)&1;
        pad[19]=(portrdG>>2)&1;
        pad[20]=(portrdG>>3)&1;
        pad[33]=(portrdG>>10)&1;    // Encoder button
        pad[28]=(portrdG>>11)&1;
        pad[29]=(portrdG>>12)&1;
        pad[30]=(portrdG>>13)&1;
        pad[31]=(portrdG>>14)&1;
        pad[32]=(portrdG>>15)&1;
        pad[24]=(portrdD>>1)&1;
        pad[25]=(portrdD>>2)&1;
        pad[26]=(portrdD>>3)&1;
        pad[27]=(portrdD>>4)&1;
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
    volatile register int result asm("A");
    fractional pots_buf[POTS];
    const fractional pot_alpha = 0x0F00;    //larger = rougher, lower = more latency
    const fractional pot_alpha_inv = 32767-pot_alpha;
    int i;
    _AD1IF = 0; // Clear conversion done status bit
    if(pad[34])i=0;
    else i=POTS/2;
    pots_buf[i++]=(ADC1BUF5>>1)|0x7;
    pots_buf[i++]=(ADC1BUF2>>1)|0x7;
    pots_buf[i++]=(ADC1BUF4>>1)|0x7;
    pots_buf[i++]=(ADC1BUF1>>1)|0x7;
    pots_buf[i++]=(ADC1BUF3>>1)|0x7;
    pots_buf[i]=(ADC1BUF0>>1)|0x7;

    result =__builtin_mpy(pots_buf[i],pot_alpha, NULL, NULL, 0, NULL, NULL, 0);
    result =__builtin_mac(result, pots[i], pot_alpha_inv, NULL, NULL, 0, NULL, NULL, 0, 0, result);
    pots[i--]=__builtin_sac(result, 0);
    
    result =__builtin_mpy(pots_buf[i],pot_alpha, NULL, NULL, 0, NULL, NULL, 0);
    result =__builtin_mac(result, pots[i], pot_alpha_inv, NULL, NULL, 0, NULL, NULL, 0, 0, result);
    pots[i--]=__builtin_sac(result, 0);
    result =__builtin_mpy(pots_buf[i],pot_alpha, NULL, NULL, 0, NULL, NULL, 0);
    result =__builtin_mac(result, pots[i], pot_alpha_inv, NULL, NULL, 0, NULL, NULL, 0, 0, result);
    pots[i--]=__builtin_sac(result, 0);
    result =__builtin_mpy(pots_buf[i],pot_alpha, NULL, NULL, 0, NULL, NULL, 0);
    result =__builtin_mac(result, pots[i], pot_alpha_inv, NULL, NULL, 0, NULL, NULL, 0, 0, result);
    pots[i--]=__builtin_sac(result, 0);
    result =__builtin_mpy(pots_buf[i],pot_alpha, NULL, NULL, 0, NULL, NULL, 0);
    result =__builtin_mac(result, pots[i], pot_alpha_inv, NULL, NULL, 0, NULL, NULL, 0, 0, result);
    pots[i--]=__builtin_sac(result, 0);
    result =__builtin_mpy(pots_buf[i],pot_alpha, NULL, NULL, 0, NULL, NULL, 0);
    result =__builtin_mac(result, pots[i], pot_alpha_inv, NULL, NULL, 0, NULL, NULL, 0, 0, result);
    pots[i]=__builtin_sac(result, 0);
}

void scalePots(void){
    /* Potentiometer scaling for fx or lcd display */
    volatile register int scaled asm("A");
    
    
    scaled=__builtin_mpy(pots[0],POT_PERCENT, NULL, NULL, 0, NULL, NULL, 0);
    pots_scaled[0]=__builtin_sac(scaled, 7);
    scaled=__builtin_mpy(pots[1],POT_PERCENT, NULL, NULL, 0, NULL, NULL, 0);
    pots_scaled[1]=__builtin_sac(scaled, 7);
    scaled=__builtin_mpy(pots[2],POT_PERCENT, NULL, NULL, 0, NULL, NULL, 0);
    pots_scaled[2]=__builtin_sac(scaled, 7);
    scaled=__builtin_mpy(pots[3],POT_PERCENT, NULL, NULL, 0, NULL, NULL, 0);
    pots_scaled[3]=__builtin_sac(scaled, 7);
    scaled=__builtin_mpy(pots[4],POT_PERCENT, NULL, NULL, 0, NULL, NULL, 0);
    pots_scaled[4]=__builtin_sac(scaled, 7);
    scaled=__builtin_mpy(pots[5],POT_PERCENT, NULL, NULL, 0, NULL, NULL, 0);
    pots_scaled[5]=__builtin_sac(scaled, 7);
    
    scaled=__builtin_mpy(pots[POT_FX_SELECT1],FXSCALE, NULL, NULL, 0, NULL, NULL, 0);
    pots_scaled[POT_FX_SELECT1]=__builtin_sac(scaled, 0);
    scaled=__builtin_mpy(pots[POT_FX_SELECT2],FXSCALE, NULL, NULL, 0, NULL, NULL, 0);
    pots_scaled[POT_FX_SELECT2]=__builtin_sac(scaled, 0);
}

void scalePotsCustom(unsigned int steps){
    volatile register int scaled asm("A");
    fractional scale = Q15(steps*0.000030518509476);
    
    scaled=__builtin_mpy(pots[4],scale, NULL, NULL, 0, NULL, NULL, 0);
    pots_scaled[4]=__builtin_sac(scaled, 0);
}

void changeFX(void){
    fxUnits[0]=pots_scaled[POT_FX_SELECT1];
    fxUnits[1]=pots_scaled[POT_FX_SELECT2];
}

void display(void){
    scalePots();
    changeFX();
    // Update ui state logic here
    state = (ENCODERCNTL/4)+1;
    
    // Update screen here
    screenUpdate();
   
   if(UART_ON==TRUE){
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