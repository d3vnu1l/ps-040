/*
 * File:   screens.c
 * Author: ryan
 *
 * Created on January 28, 2018, 11:17 PM
 */


#include <xc.h>
#include "screens.h"
#include "common.h"
#include "plcd.h"
#include "dsp.h"
#include "flash.h"

extern unsigned char TEST_SIN;
extern fractional pots[POTS];
extern fractional pots_scaled[POTS];
extern unsigned char pad[BUTTONS];
extern enum screenStruc state, laststate;
extern char flash_readback[512];
extern unsigned int process_time;

extern enum fxStruct fxUnits[NUMFXUNITS];

int fxLast=0;
int fxNow=0;

void (*fxModPointers[NUMFX])(unsigned int, fractional, fractional, fractional) = {screenNoFXmod, screenLPFmod, screenTRMmod, screenLOPmod, screenBITmod};

void screenDebugAudio(){

}

void screenDebugBuffers(){
    if(state!=laststate){
        //setup here
        lcdClearQ();
        lcdSetCursorQ(0,0);
        lcdWriteStringQ("Buffer Debug");
        lcdSetCursorQ(0,1);
        lcdWriteStringQ("FX time:");
        lcdSetCursorQ(11,1);
        lcdWriteStringQ("/256");
        lcdSetCursorQ(0,2);
        lcdWriteStringQ("LCD:");
        lcdSetCursorQ(0,3);
        lcdWriteStringQ("Flash:");
    } else {
        //update here
        lcdSetCursorQ(8,1);
        lcdWriteDecimalQ(process_time,3);
        //lcdSetCursorQ(4,2);
        //lcdWriteDecimalQ();
        //lcdSetCursorQ(6,3);
        //lcdWriteDecimalQ();
        
    }
}

void screenDebugPots(void){
    int bank=0;
    
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
        if(!pad[34])bank=POTS/2;
        lcdSetCursorQ(0,1);
        lcdWriteDecimalQ(pots_scaled[bank], 3);
        lcdWriteQ(',');
        lcdWriteWordUnsignedQ(pots[bank++]);
        lcdSetCursorQ(11,1);
        lcdWriteDecimalQ(pots_scaled[bank], 3);
        lcdWriteQ(',');
        lcdWriteWordUnsignedQ(pots[bank++]);
        lcdSetCursorQ(0,2);
        lcdWriteDecimalQ(pots_scaled[bank], 3);
        lcdWriteQ(',');
        lcdWriteWordUnsignedQ(pots[bank++]);
        lcdSetCursorQ(11,2);
        lcdWriteDecimalQ(pots_scaled[bank], 3);
        lcdWriteQ(',');
        lcdWriteWordUnsignedQ(pots[bank++]);
        lcdSetCursorQ(0,3);
        lcdWriteDecimalQ(pots_scaled[bank], 3);
        lcdWriteQ(',');
        lcdWriteWordUnsignedQ(pots[bank++]);
        lcdSetCursorQ(11,3);
        lcdWriteDecimalQ(pots_scaled[bank], 3);
        lcdWriteQ(',');
        lcdWriteWordUnsignedQ(pots[bank++]);
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

void screenDebugInput(void){
    if(state!=laststate){
        //setup here
        lcdClearQ();
        lcdSetCursorQ(0,0);
        lcdWriteStringQ("Input Debug");
        lcdSetCursorQ(0,1);
        lcdWriteStringQ("ADC variance: ");
    } else {
        //update here
        lcdDrawPads(16);
    }
}

void screenNoFXmod(unsigned int col, fractional param1, fractional param2, fractional param3){
        if(fxNow!=fxLast){
        // Setup here
            
        lcdSetCursorQ(col+5,0);
        lcdWriteStringQ("OFF");
        lcdSetCursorQ(col,1);
        lcdWriteStringQ("        ");
        lcdSetCursorQ(col,2);
        lcdWriteStringQ("        ");
        lcdSetCursorQ(col,3);
        lcdWriteStringQ("        ");

    } else {
        // Update here
        lcdSetCursorQ(col+5,0);
        lcdWriteStringQ("OFF");
    }
}

void screenLPFmod(unsigned int col, fractional param1, fractional param2, fractional param3){
    if(fxNow!=fxLast){
        // Setup here
        lcdSetCursorQ(col+5,0);
        lcdWriteStringQ("LPF");
        lcdSetCursorQ(col,1);
        lcdWriteStringQ("frq");
        lcdSetCursorQ(col,2);
        lcdWriteStringQ("d/w");
        lcdSetCursorQ(col,3);
        lcdWriteStringQ("pwr");
    } else {
        // Update here
        lcdSetCursorQ(col+5,1);
        lcdWriteDecimalQ(param1, 3);
        lcdSetCursorQ(col+5,2);
        lcdWriteDecimalQ(param2, 3);
        lcdSetCursorQ(col+5,3);
        if(param3>=50)
            lcdWriteStringQ(" ON");
        else lcdWriteStringQ("OFF");
    }
}

void screenTRMmod(unsigned int col, fractional param1, fractional param2, fractional param3){
    if(fxNow!=fxLast){
        // Setup here
        lcdSetCursorQ(col+5,0);
        lcdWriteStringQ("TRM");
        lcdSetCursorQ(col,1);
        lcdWriteStringQ("rat");
        lcdSetCursorQ(col,2);
        lcdWriteStringQ("dep");
        lcdSetCursorQ(col,3);
        lcdWriteStringQ("pwr");
    } else {
        // Update here
        lcdSetCursorQ(col+5,1);
        lcdWriteDecimalQ(param1, 3);
        lcdSetCursorQ(col+5,2);
        lcdWriteDecimalQ(param2, 3);
        lcdSetCursorQ(col+5,3);
        if(param3>=50)
            lcdWriteStringQ(" ON");
        else lcdWriteStringQ("OFF");
    }
}

void screenLOPmod(unsigned int col, fractional param1, fractional param2, fractional param3){
    if(fxNow!=fxLast){
        // Setup here
        lcdSetCursorQ(col+5,0);
        lcdWriteStringQ("LOP");
        lcdSetCursorQ(col,1);
        lcdWriteStringQ("siz");
        lcdSetCursorQ(col,2);
        lcdWriteStringQ("d/w");
        lcdSetCursorQ(col,3);
        lcdWriteStringQ("pwr");
    } else {
        // Update here
        lcdSetCursorQ(col+5,1);
        lcdWriteDecimalQ(param1, 3);
        lcdSetCursorQ(col+5,2);
        lcdWriteDecimalQ(param2, 3);
        lcdSetCursorQ(col+5,3);
        if(param3>=50)
            lcdWriteStringQ(" ON");
        else lcdWriteStringQ("OFF");
    }
}

void screenBITmod(unsigned int col, fractional param1, fractional param2, fractional param3){
    if(fxNow!=fxLast){
        // Setup here
        lcdSetCursorQ(col+5,0);
        lcdWriteStringQ("BIT");
        lcdSetCursorQ(col,1);
        lcdWriteStringQ("siz");
        lcdSetCursorQ(col,2);
        lcdWriteStringQ("d/w");
        lcdSetCursorQ(col,3);
        lcdWriteStringQ("pwr");
    } else {
        // Update here
        lcdSetCursorQ(col+5,1);
        lcdWriteDecimalQ(param1, 3);
        lcdSetCursorQ(col+5,2);
        lcdWriteDecimalQ(param2, 3);
        lcdSetCursorQ(col+5,3);
        if(param3>=50)
            lcdWriteStringQ(" ON");
        else lcdWriteStringQ("OFF");
    }
}

void screenFX(void){
    if(state!=laststate){
        //setup here
        lcdClearQ();
        lcdSetCursorQ(0,0);
        lcdWriteStringQ("Fx1:");
        lcdSetCursorQ(10,0);
        lcdWriteStringQ("Fx2:");
        
    } else {
        //update here 
        fxModPointers[fxUnits[0]](0,  pots_scaled[FX_1], pots_scaled[FX_2], pots_scaled[FX_3]);
        fxModPointers[fxUnits[1]](10, pots_scaled[FX_4], pots_scaled[FX_5], pots_scaled[FX_6]);
    }
         
}

void screenUpdate(void){
    fxNow=fxUnits[0]+fxUnits[1];
    
    switch(state){
        case start: break;
        case scrnFX:            screenFX(); 
        break;
        case debugscrnPOTS:     screenDebugPots();
        break;
        case debugscrnFLASH:    screenDebugFlash();
        break;
        case debugscrnBUFFERS:  screenDebugBuffers();
        break;
        case debugscrnINPUT:    screenDebugInput();
        break;
                        
        default: break;
    }
    
    laststate=state;
    fxLast=fxUnits[0]+fxUnits[1];
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


