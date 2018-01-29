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
extern enum screen state, laststate;
extern char flash_readback[512];

void screenDebugAudio(){

}

void screenDebugPots(void){
    if(state!=laststate){
        // Setup here
        lcdClearQ();
        lcdSetCursorQ(0,0);
        lcdWriteStringQ("1:");
        lcdSetCursorQ(8,0);
        lcdWriteStringQ("2:");
        lcdSetCursorQ(0,1);
        lcdWriteStringQ("3:");
        lcdSetCursorQ(8,1);
        lcdWriteStringQ("4:");
        lcdSetCursorQ(0,2);
        lcdWriteStringQ("5:");
        lcdSetCursorQ(8,2);
        lcdWriteStringQ("6:");
        lcdSetCursorQ(11,3);
        if(TEST_SIN==TRUE)lcdWriteStringQ("SINE");
        else lcdWriteStringQ("THRU");  
        lcdDrawPads(16);
    } else {
        // Update here
        lcdSetCursorQ(2,0);
        lcdWriteWordQ(pots[0]);
        lcdSetCursorQ(10,0);
        lcdWriteWordQ(pots[1]);
        lcdSetCursorQ(2,1);
        lcdWriteWordQ(pots[2]);
        lcdSetCursorQ(10,1);
        lcdWriteWordQ(pots[3]);
        lcdSetCursorQ(2,2);
        lcdWriteWordQ(pots[4]);
        lcdSetCursorQ(10,2);
        lcdWriteWordQ(pots[5]);
        lcdSetCursorQ(11,3);
        if(TEST_SIN==TRUE)lcdWriteStringQ("SINE");
        else lcdWriteStringQ("THRU");
        lcdDrawPads(16);
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
    } else {
        //update here
        lcdSetCursorQ(0,2);
        lcdWriteDecimalQ(pots[0]>>8);
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


