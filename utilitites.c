#include <xc.h>
#include <p33EP512GM310.h>
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

//CONTROL VARIABLES//
extern struct ctrlsrfc ctrl;
extern unsigned char UART_ON; 

//STATUS VARIABLES//
extern unsigned char hard_clipped;
extern unsigned char UART_EN;
extern unsigned char TEST_SIN;

extern enum fxStruct fxUnits[NUMFXUNITS];
extern enum screenStruc state;
extern struct clip_psv sine, kick, snare;

void scanButtons(void){
    
    static unsigned char pad_last[BUTTONS]={1};
    int portrdG, portrdD, portrdF;

    portrdG = PORTG;
    portrdD = PORTD;
    portrdF = PORTF;
    
    ctrl.pad[34]=(portrdF>>7)&1;     // Special function button
    
    if(ctrl.pad[34]){
        ctrl.pad[6]=(portrdF>>6)&1;
        ctrl.pad[5]=(portrdF>>5)&1;
        ctrl.pad[4]=(portrdF>>4)&1;
        ctrl.pad[0]=(portrdG)&1;
        ctrl.pad[1]=(portrdG>>1)&1;
        ctrl.pad[2]=(portrdG>>2)&1;
        ctrl.pad[3]=(portrdG>>3)&1;
        ctrl.pad[16]=(portrdG>>10)&1;    // Encoder button
        ctrl.pad[11]=(portrdG>>11)&1;
        ctrl.pad[12]=(portrdG>>12)&1;
        ctrl.pad[13]=(portrdG>>13)&1;
        ctrl.pad[14]=(portrdG>>14)&1;
        ctrl.pad[15]=(portrdG>>15)&1;
        ctrl.pad[7]=(portrdD>>1)&1;
        ctrl.pad[8]=(portrdD>>2)&1;
        ctrl.pad[9]=(portrdD>>3)&1;
        ctrl.pad[10]=(portrdD>>4)&1;
    } else {
        ctrl.pad[23]=(portrdF>>6)&1;
        ctrl.pad[22]=(portrdF>>5)&1;
        ctrl.pad[21]=(portrdF>>4)&1;
        ctrl.pad[17]=(portrdG)&1;
        ctrl.pad[18]=(portrdG>>1)&1;
        ctrl.pad[19]=(portrdG>>2)&1;
        ctrl.pad[20]=(portrdG>>3)&1;
        ctrl.pad[33]=(portrdG>>10)&1;    // Encoder button
        ctrl.pad[28]=(portrdG>>11)&1;
        ctrl.pad[29]=(portrdG>>12)&1;
        ctrl.pad[30]=(portrdG>>13)&1;
        ctrl.pad[31]=(portrdG>>14)&1;
        ctrl.pad[32]=(portrdG>>15)&1;
        ctrl.pad[24]=(portrdD>>1)&1;
        ctrl.pad[25]=(portrdD>>2)&1;
        ctrl.pad[26]=(portrdD>>3)&1;
        ctrl.pad[27]=(portrdD>>4)&1;
    }
    
    // SAMPLE TRIGGERS 
    if(ctrl.pad[0]==0){                                         //kick
        kick.playing=TRUE;
    }
    /*
    if(controls.pad[2]==0&&hat_playing==FALSE){                                          //hat
        hat_playing=TRUE;
    }
    */
    if(ctrl.pad[1]==0&&snare.playing==FALSE){                                        //snare
        snare.playing=TRUE;
    }
}

void readPots(void){
    volatile register int result asm("A");
    fractional pots_buf[POTS/2]; 
    fractional pots_last[POTS/2];
    static fractional pots_smoothed[POTS/2];
    const fractional pot_alpha = 0x0F80;    //larger = rougher, lower = more latency
    const fractional pot_alpha_inv = 32767-pot_alpha;
    const unsigned int shift = 0xFE00;
    int i;
    _AD1IF = 0; // Clear conversion done status bit
    if(ctrl.pad[34])i=0;
    else i=POTS/2;
    pots_buf[0]=(ADC1BUF5>>1)|0x7;
    pots_buf[1]=(ADC1BUF2>>1)|0x7;
    pots_buf[2]=(ADC1BUF4>>1)|0x7;
    pots_buf[3]=(ADC1BUF1>>1)|0x7;
    pots_buf[4]=(ADC1BUF3>>1)|0x7;
    pots_buf[5]=(ADC1BUF0>>1)|0x7;
    
    pots_last[0]=(pots_smoothed[0]&shift);
    pots_last[1]=(pots_smoothed[1]&shift);
    pots_last[2]=(pots_smoothed[2]&shift);
    pots_last[3]=(pots_smoothed[3]&shift);
    pots_last[4]=(pots_smoothed[4]&shift);
    pots_last[5]=(pots_smoothed[5]&shift);
    
    result =__builtin_mpy(pots_buf[0],pot_alpha, NULL, NULL, 0, NULL, NULL, 0);
    result =__builtin_mac(result, pots_smoothed[0], pot_alpha_inv, NULL, NULL, 0, NULL, NULL, 0, 0, result);
    pots_smoothed[0]=__builtin_sac(result, 0);
    result =__builtin_mpy(pots_buf[1],pot_alpha, NULL, NULL, 0, NULL, NULL, 0);
    result =__builtin_mac(result, pots_smoothed[1], pot_alpha_inv, NULL, NULL, 0, NULL, NULL, 0, 0, result);
    pots_smoothed[1]=__builtin_sac(result, 0);
    result =__builtin_mpy(pots_buf[2],pot_alpha, NULL, NULL, 0, NULL, NULL, 0);
    result =__builtin_mac(result, pots_smoothed[2], pot_alpha_inv, NULL, NULL, 0, NULL, NULL, 0, 0, result);
    pots_smoothed[2]=__builtin_sac(result, 0);
    result =__builtin_mpy(pots_buf[3],pot_alpha, NULL, NULL, 0, NULL, NULL, 0);
    result =__builtin_mac(result, pots_smoothed[3], pot_alpha_inv, NULL, NULL, 0, NULL, NULL, 0, 0, result);
    pots_smoothed[3]=__builtin_sac(result, 0);
    result =__builtin_mpy(pots_buf[4],pot_alpha, NULL, NULL, 0, NULL, NULL, 0);
    result =__builtin_mac(result, pots_smoothed[4], pot_alpha_inv, NULL, NULL, 0, NULL, NULL, 0, 0, result);
    pots_smoothed[4]=__builtin_sac(result, 0);
    result =__builtin_mpy(pots_buf[5],pot_alpha, NULL, NULL, 0, NULL, NULL, 0);
    result =__builtin_mac(result, pots_smoothed[5], pot_alpha_inv, NULL, NULL, 0, NULL, NULL, 0, 0, result);
    pots_smoothed[5]=__builtin_sac(result, 0);
    
    if((pots_smoothed[0]&shift)!=pots_last[0]) 
        ctrl.pots[i]=pots_buf[0];
    if((pots_smoothed[1]&shift)!=pots_last[1]) 
        ctrl.pots[i+1]=pots_buf[1];
    if((pots_smoothed[2]&shift)!=pots_last[2]) 
        ctrl.pots[i+2]=pots_buf[2];
    if((pots_smoothed[3]&shift)!=pots_last[3]) 
        ctrl.pots[i+3]=pots_buf[3];
    if((pots_smoothed[4]&shift)!=pots_last[4]) 
        ctrl.pots[i+4]=pots_buf[4];
    if((pots_smoothed[5]&shift)!=pots_last[5]) 
        ctrl.pots[i+5]=pots_buf[5];     
}

void scalePots(void){
    /* Potentiometer scaling for fx or lcd display */
    volatile register int scaled asm("A");
    
    scaled=__builtin_mpy(ctrl.pots[0],POT_PERCENT, NULL, NULL, 0, NULL, NULL, 0);
    ctrl.pots_scaled[0]=__builtin_sac(scaled, 7);
    scaled=__builtin_mpy(ctrl.pots[1],POT_PERCENT, NULL, NULL, 0, NULL, NULL, 0);
    ctrl.pots_scaled[1]=__builtin_sac(scaled, 7);
    scaled=__builtin_mpy(ctrl.pots[2],POT_PERCENT, NULL, NULL, 0, NULL, NULL, 0);
    ctrl.pots_scaled[2]=__builtin_sac(scaled, 7);
    scaled=__builtin_mpy(ctrl.pots[3],POT_PERCENT, NULL, NULL, 0, NULL, NULL, 0);
    ctrl.pots_scaled[3]=__builtin_sac(scaled, 7);
    scaled=__builtin_mpy(ctrl.pots[4],POT_PERCENT, NULL, NULL, 0, NULL, NULL, 0);
    ctrl.pots_scaled[4]=__builtin_sac(scaled, 7);
    scaled=__builtin_mpy(ctrl.pots[5],POT_PERCENT, NULL, NULL, 0, NULL, NULL, 0);
    ctrl.pots_scaled[5]=__builtin_sac(scaled, 7);
    
    scaled=__builtin_mpy(ctrl.pots[POT_FX_SELECT1],FXSCALE, NULL, NULL, 0, NULL, NULL, 0);
    ctrl.pots_scaled[POT_FX_SELECT1]=__builtin_sac(scaled, 0);
    scaled=__builtin_mpy(ctrl.pots[POT_FX_SELECT2],FXSCALE, NULL, NULL, 0, NULL, NULL, 0);
    ctrl.pots_scaled[POT_FX_SELECT2]=__builtin_sac(scaled, 0);
}

fractional scalePotsCustom(unsigned int steps, fractional scaleme){
    volatile register int scaled asm("A");
    fractional scale = Q15(steps*0.000030518509476);
    
    scaled=__builtin_mpy(scaleme,scale, NULL, NULL, 0, NULL, NULL, 0);
    return(__builtin_sac(scaled, 0));
}

void changeFX(void){
    fxUnits[0]=ctrl.pots_scaled[POT_FX_SELECT1];
    fxUnits[1]=ctrl.pots_scaled[POT_FX_SELECT2];
}

void display(void){
    scalePots();
    changeFX();
    // Update ui state logic here
    state = (ENCODERCNTL/4)+1;
    
    // Update screen here
    screenUpdate();
   
   if(UART_ON==TRUE){
        //printf("b1 %d, b2 %d, b3 %d, b4 %d\r\n", controls.pad[0], controls.pad[1], controls.pad[2], controls.pad[3]);
        //printf("b4 %d, b5 %d, b6 %d, b7 %d\r\n", controls.pad[4], controls.pad[5], controls.pad[6], controls.pad[7]);
        //printf("P1 %x  P1 %d bpm %d\r\n", controls.pots[0], controls.pots[0], bpm);   //check pots
        printf("P1 %d  P2 %d P3 %d\r\n", ctrl.pots[0], ctrl.pots[1], ctrl.pots[2]);   //check pots
        //printf("%d\r\n", sample);  //check input ADC
        //printf("%d, pot1 %x, pot2 %x, avg %x\r\n", sample, controls.pots[1], controls.pots[2], average);  //check input ADC
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

void ClipCopy_psv(int numElems, fractional * dstV, __psv__ fractional * srcV){
    int i;
    
    for(i=0; i<numElems; i++){
        *dstV++=*srcV++;
    }
}

void ClipCopy_eds(int numElems, fractional * dstV, __eds__ fractional * srcV){
    int i;
    
    for(i=0; i<numElems; i++){
        *dstV++=*srcV++;
    } 
}

void ClipCopy_toeds(int numElems, __eds__ fractional * dstV, fractional * srcV){
    int i;
    
    for(i=0; i<numElems; i++){
        *dstV++=*srcV++;
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