/*
 General utilities, button and pot scanning
 */
#include <xc.h>
#include <p33EP512GM310.h>
#include <dsp.h>
#include "common.h"
#include "sounds.h"
#include "flash.h"
#include "screens.h"
#include "utilities.h"
#include "definitions.h"

extern enum fxStruct fxUnits[NUMFXUNITS];
extern enum screenStruc state;
extern enum screenStruc laststate;
extern struct ctrlsrfc ctrl;
extern struct sflags stat;
extern struct clip_psv sine, kick, snare;
extern struct clip_flash clipmap[FLASH_NUMCHUNKS];

/* Buttons have 4 states
    held          0 0 (represented as 3)
    pressed       1 0 (represented as 1)
    depressed     0 1 (represented as 2)
    inactive      1 1 (represented as 0)
 * So that a pad value > 1 indicates playing
*/
void scanButtons(void){
    int i;
    static unsigned char pad_last[BUTTONS]={[0 ... (BUTTONS-1)] = 1};
    static unsigned char pad_now[BUTTONS]={[0 ... (BUTTONS-1)] = 1};
    int portrdG, portrdD, portrdF;

    portrdG = PORTG;
    portrdD = PORTD;
    portrdF = PORTF;
    
    for(i=0; i<BUTTONS; i++){      // Get last button history
        pad_last[i]=pad_now[i];
    }
    
    pad_now[BTN_SPECIAL]=(portrdF>>7)&1;        // Special function button
    
    pad_now[6]=(portrdF>>6)&1;
    pad_now[5]=(portrdF>>5)&1;
    pad_now[4]=(portrdF>>4)&1;
    pad_now[0]=(portrdG)&1;
    pad_now[1]=(portrdG>>1)&1;
    pad_now[2]=(portrdG>>2)&1;
    pad_now[3]=(portrdG>>3)&1;
    pad_now[16]=(portrdG>>10)&1;    // Encoder button
    pad_now[11]=(portrdG>>11)&1;
    pad_now[12]=(portrdG>>12)&1;
    pad_now[13]=(portrdG>>13)&1;
    pad_now[14]=(portrdG>>14)&1;
    pad_now[15]=(portrdG>>15)&1;
    pad_now[7]=(portrdD>>1)&1;
    pad_now[8]=(portrdD>>2)&1;
    pad_now[9]=(portrdD>>3)&1;
    pad_now[10]=(portrdD>>4)&1;  
    
    if(!pad_now[BTN_SPECIAL])
        pad_now[BTN_ENCSPEC]=pad_now[16];
    else pad_now[BTN_ENCSPEC]=1;
    
    
    for(i=0; i<BUTTONS; i++){
        unsigned char temp = ((pad_last[i]&1)<<1) + (pad_now[i]&1);
        switch(temp){
            case 0x00: 
                ctrl.pad[i] = 3;    // HELD
                break;
            case 0x01: 
                ctrl.pad[i] = 1;    // DEPRESSED
                break;
            case 0x02: 
                ctrl.pad[i] = 2;    // PRESSED
                break;
            case 0x03: 
                ctrl.pad[i] = 0;    // INACTIVE
                break;
            default: 
                ctrl.pad[i]=0;
                break;
        }
    }
    
    //find last pressed button
    for(i=0; i<(BUTTONS-3); i++){
        if(ctrl.pad[i]>1) ctrl.last_pressed=i;
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
    int i, j;
    
    _AD1IF = 0; // Clear conversion done status bit
    
    if(ctrl.pad[BTN_SPECIAL]<2)i=0;
    else i=POTS/2;
    
    for(j=0; j<POTS; j++){
        ctrl.pot_moved[j]=FALSE;
    }
    
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
    
    for(j=0; j<(POTS/2); j++){
        if((pots_smoothed[j]&shift)!=pots_last[j]){ 
            ctrl.pots[i+j]=pots_buf[j];
            ctrl.pot_moved[i+j]=TRUE;
        }
    }
}

void scalePots(void){
    /* Potentiometer scaling for fx or lcd display 
     * Shift right 9 times, for 0-127
     */
    volatile register int scaled asm("A"); 
    int i;
    
    for(i=0; i<POTS; i++)
        ctrl.pots_scaled[i]=(ctrl.pots[i]>>8);
    
    scaled=__builtin_mpy(ctrl.pots[POT_FX_SELECT1],FXSCALE, NULL, NULL, 0, NULL, NULL, 0); 
    ctrl.pots_scaled[POT_FX_SELECT1]=__builtin_sac(scaled, 0); 
    scaled=__builtin_mpy(ctrl.pots[POT_FX_SELECT2],FXSCALE, NULL, NULL, 0, NULL, NULL, 0); 
    ctrl.pots_scaled[POT_FX_SELECT2]=__builtin_sac(scaled, 0); 
}

fractional scalePotCustom(unsigned int steps, fractional scaleme){
    volatile register int scaled asm("A");
    fractional scale = Q15(steps*0.000030518509476);
    
    scaled=__builtin_mpy(scaleme,scale, NULL, NULL, 0, NULL, NULL, 0);
    return(__builtin_sac(scaled, 0));
    
}

void display(void){
    long int newstate;
    static long int _laststate=0;
    
    scalePots();
    
    // Update ui state logic here
    newstate = (ENCODERCNTL/4);
    if(newstate > _laststate){
        if(state<(SCREENS-1))
            state++;
    } 
    else if(newstate < _laststate){
        if(state>0)
            state--;
    }
    _laststate=newstate;
    
    // Change FX
    fxUnits[0]=ctrl.pots_scaled[POT_FX_SELECT1];
    fxUnits[1]=ctrl.pots_scaled[POT_FX_SELECT2];

    // Update screen here
    screenUpdate();
   
    
    // RGB LED
    stat.rgb_led=state;
    if(stat.rgb_led>7)
        stat.rgb_led-=7;
    LED_R = (stat.rgb_led&1);
    LED_G = ((stat.rgb_led>>1)&1);
    LED_B = ((stat.rgb_led>>2)&1);
    
   SLED=~SLED;
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