/*
 * File:   screens.c
 * Author: ryan
 *
 * Created on January 28, 2018, 11:17 PM
 */


#include <xc.h>
#include "common.h"
#include "plcd.h"
#include "dsp.h"
#include "flash.h"

extern unsigned char TEST_SIN;
extern fractional pots[POTS];
extern fractional pots_percent[POTS];
extern enum screen state, laststate;
extern char flash_readback[512];
extern unsigned int cycle;

void screenDebugAudio(){

}

void screenDebufBuffers(){
    if(state!=laststate){
        //setup here
        lcdClearQ();
        lcdSetCursorQ(0,0);
        lcdWriteStringQ("Buffer Debug");
        lcdSetCursorQ(0,1);
        lcdWriteStringQ("Audio:");
        lcdSetCursorQ(0,2);
        lcdWriteStringQ("LCD:");
        lcdSetCursorQ(0,3);
        lcdWriteStringQ("Flash:");
    } else {
        //update here
        lcdSetCursorQ(6,1);
        lcdWriteDecimalQ(cycle,3);
        lcdSetCursorQ(4,2);
        //lcdWriteDecimalQ();
        lcdSetCursorQ(6,3);
        //lcdWriteDecimalQ();
        
    }
}

void screenDebugPots(void){
    if(state!=laststate){
        // Setup here
        lcdClearQ();
        lcdSetCursorQ(0,0);
        lcdWriteStringQ("Potentiometer Debug");
        lcdSetCursorQ(9,1);
        lcdWriteStringQ("|");
         lcdSetCursorQ(9,2);
        lcdWriteStringQ("|");
         lcdSetCursorQ(9,3);
        lcdWriteStringQ("|");
    } else {
        // Update here
        lcdSetCursorQ(0,1);
        lcdWriteDecimalQ(pots_percent[0], 3);
        lcdWriteQ(',');
        lcdWriteWordUnsignedQ(pots[0]);
        lcdSetCursorQ(11,1);
        lcdWriteDecimalQ(pots_percent[1], 3);
        lcdWriteQ(',');
        lcdWriteWordUnsignedQ(pots[1]);
        lcdSetCursorQ(0,2);
        lcdWriteDecimalQ(pots_percent[2], 3);
        lcdWriteQ(',');
        lcdWriteWordUnsignedQ(pots[2]);
        lcdSetCursorQ(11,2);
        lcdWriteDecimalQ(pots_percent[3], 3);
        lcdWriteQ(',');
        lcdWriteWordUnsignedQ(pots[3]);
        lcdSetCursorQ(0,3);
        lcdWriteDecimalQ(pots_percent[4], 3);
        lcdWriteQ(',');
        lcdWriteWordUnsignedQ(pots[4]);
        lcdSetCursorQ(11,3);
        lcdWriteDecimalQ(pots_percent[5], 3);
        lcdWriteQ(',');
        lcdWriteWordUnsignedQ(pots[5]);
    }
}

void screenFX(void){
    if(state!=laststate){
        //setup here
        lcdClearQ();
        lcdSetCursorQ(0,0);
        lcdWriteStringQ("FX1:");
        lcdSetCursorQ(10,0);
        lcdWriteStringQ("FX2:");
        
        lcdSetCursorQ(0,1);
        lcdWriteStringQ("1:");
        lcdSetCursorQ(10,1);
        lcdWriteStringQ("4:");
        lcdSetCursorQ(0,2);
        lcdWriteStringQ("2:");
        lcdSetCursorQ(10,2);
        lcdWriteStringQ("5:");
        lcdSetCursorQ(0,3);
        lcdWriteStringQ("3:");
        lcdSetCursorQ(10,3);
        lcdWriteStringQ("6:");
    } else {
        //update here
        lcdSetCursorQ(2,1);
        lcdWriteDecimalQ(pots_percent[0], 3);
        lcdSetCursorQ(12,1);
        lcdWriteDecimalQ(pots_percent[1], 3);
        lcdSetCursorQ(2,2);
        lcdWriteDecimalQ(pots_percent[2], 3);
        lcdSetCursorQ(12,2);
        lcdWriteDecimalQ(pots_percent[3], 3);
        lcdSetCursorQ(2,3);
        lcdWriteDecimalQ(pots_percent[4], 3);
        lcdSetCursorQ(12,3);
        lcdWriteDecimalQ(pots_percent[5], 3);

    }
}

void screenDebugFlash(void){
        if(state!=laststate){
        //setup here
        lcdClearQ();
        lcdSetCursorQ(0,0);
        lcdWriteWordQ(flash_readback[2]);
         lcdSetCursorQ(6,0);
        lcdWriteWordQ(flash_readback[3]);
         lcdSetCursorQ(12,0);
        lcdWriteWordQ(flash_readback[4]);
         lcdSetCursorQ(0,1);
        lcdWriteWordQ(flash_readback[5]);
        lcdSetCursorQ(6,1);
        lcdWriteWordQ(flash_readback[6]);
         lcdSetCursorQ(12,1);
        lcdWriteWordQ(flash_readback[7]);
        lcdSetCursorQ(0,2);
        lcdWriteWordQ(flash_readback[8]);
        lcdSetCursorQ(6,2);
        lcdWriteWordQ(flash_readback[9]);
         lcdSetCursorQ(12,2);
        lcdWriteWordQ(flash_readback[10]);
        lcdSetCursorQ(0,3);
        lcdWriteWordQ(flash_readback[8]);
    } else {
        //update here
        lcdSetCursorQ(9,3);
        lcdWriteStringQ("Stat:");
        lcdWriteWordQ(flashStatusCheck());
    }
}

void screenUpdate(void){
    switch(state){
        case start: break;
        case scrnFX:            screenFX(); 
        break;
        case debugscrnPOTS:     screenDebugPots();
        break;
        case debugscrnFLASH:    screenDebugFlash();
        break;
        case debugscrnBUFFERS:  screenDebufBuffers();
        break;
                        
        default: break;
    }
    
    laststate=state;
}

    /*
    lcdSetCursorQ(0,3);
    if(hard_clipped==TRUE){                                                     //CLIP CONTROL    
        lcdWriteStringQ("CLIP");
        hard_clipped=FALSE;  
    }
    else if(TEST_SIN==TRUE)lcdWriteStringQ("SINE");
    else lcdWriteStringQ("THRU");
     * 
     *    lcdSetCursorQ(10,3);
   lcdWriteWordQ(ENCODERCNTL);
    */


