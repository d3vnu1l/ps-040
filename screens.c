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

#include "sounds.h"

extern unsigned int process_time, flash_time;
extern unsigned char btread;

extern unsigned char    TxBufferA[FLASH_DMAXFER_BYTES]__attribute__((space(xmemory))), 
                        RxBufferA[FLASH_DMAXFER_BYTES]__attribute__((space(xmemory)));
extern fractional       RxBufferB[STREAMBUF] __attribute__((space(xmemory)));

extern enum screenStruc state, laststate;
extern enum fxStruct fxUnits[NUMFXUNITS];
extern struct clip sine;
extern struct ctrlsrfc ctrl;
extern struct sflags stat;

int fxLast=0, fxNow=0;

void (*fxModPointers[NUMFX])(unsigned int, fractional, fractional, fractional) = {screenNoFXmod, screenLPFmod, screenTRMmod, screenLOPmod, screenBTCmod};

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
        lcdWriteStringQ("Flash:");
        lcdSetCursorQ(0,3);
        lcdWriteStringQ("btRd:");
    } else {
        //update here
        lcdSetCursorQ(8,1);
        lcdWriteDecimalQ(process_time,3);
        lcdSetCursorQ(6,2);
        lcdWriteDecimalQ(flash_time,3);
        lcdSetCursorQ(6,3);
        lcdWriteByteQ(btread);
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
        if(!ctrl.pad[34])bank=POTS/2;
        lcdSetCursorQ(0,1);
        lcdWriteDecimalQ(ctrl.pots_scaled[bank], 3);
        lcdWriteQ(',');
        lcdWriteWordUnsignedQ(ctrl.pots[bank++]);
        lcdSetCursorQ(11,1);
        lcdWriteDecimalQ(ctrl.pots_scaled[bank], 3);
        lcdWriteQ(',');
        lcdWriteWordUnsignedQ(ctrl.pots[bank++]);
        lcdSetCursorQ(0,2);
        lcdWriteDecimalQ(ctrl.pots_scaled[bank], 3);
        lcdWriteQ(',');
        lcdWriteWordUnsignedQ(ctrl.pots[bank++]);
        lcdSetCursorQ(11,2);
        lcdWriteDecimalQ(ctrl.pots_scaled[bank], 3);
        lcdWriteQ(',');
        lcdWriteWordUnsignedQ(ctrl.pots[bank++]);
        lcdSetCursorQ(0,3);
        lcdWriteDecimalQ(ctrl.pots_scaled[bank], 3);
        lcdWriteQ(',');
        lcdWriteWordUnsignedQ(ctrl.pots[bank++]);
        lcdSetCursorQ(11,3);
        lcdWriteDecimalQ(ctrl.pots_scaled[bank], 3);
        lcdWriteQ(',');
        lcdWriteWordUnsignedQ(ctrl.pots[bank++]);
    }
}


void screenDebugFlash(void){
        if(state!=laststate){
        //setup here
        lcdClearQ();
        lcdSetCursorQ(0,0);
        lcdWriteByteQ(RxBufferA[0]);
         lcdSetCursorQ(3,0);
        lcdWriteByteQ(RxBufferA[1]);
         lcdSetCursorQ(6,0);
        lcdWriteByteQ(RxBufferA[2]);
         lcdSetCursorQ(9,0);
        lcdWriteByteQ(RxBufferA[3]);
        lcdSetCursorQ(12,0);
        lcdWriteByteQ(RxBufferA[4]);
        lcdSetCursorQ(15,0);
        lcdWriteByteQ(RxBufferA[5]);
        lcdSetCursorQ(18,0);
        lcdWriteByteQ(RxBufferA[6]);
        lcdSetCursorQ(0,1);
        lcdWriteByteQ(RxBufferA[7]);
         lcdSetCursorQ(3,1);
        lcdWriteByteQ(RxBufferA[8]);
         lcdSetCursorQ(6,1);
        lcdWriteByteQ(RxBufferA[9]);
         lcdSetCursorQ(9,1);
        lcdWriteByteQ(RxBufferA[10]);
        lcdSetCursorQ(12,1);
        lcdWriteByteQ(RxBufferA[11]);
        lcdSetCursorQ(15,1);
        lcdWriteByteQ(RxBufferA[12]);
        lcdSetCursorQ(18,1);
        lcdWriteByteQ(RxBufferA[13]);
        lcdSetCursorQ(0,3);
        lcdWriteStringQ("Stat:");
    } else {
        //update here
        lcdSetCursorQ(0,0);
        lcdWriteByteQ(RxBufferA[0]);
         lcdSetCursorQ(3,0);
        lcdWriteByteQ(RxBufferA[1]);
         lcdSetCursorQ(6,0);
        lcdWriteByteQ(RxBufferA[2]);
         lcdSetCursorQ(9,0);
        lcdWriteByteQ(RxBufferA[3]);
        lcdSetCursorQ(12,0);
        lcdWriteByteQ(RxBufferA[4]);
        lcdSetCursorQ(15,0);
        lcdWriteByteQ(RxBufferA[5]);
        lcdSetCursorQ(18,0);
        lcdWriteByteQ(RxBufferA[6]);
        lcdSetCursorQ(0,1);
        lcdWriteByteQ(RxBufferA[7]);
         lcdSetCursorQ(3,1);
        lcdWriteByteQ(RxBufferA[8]);
         lcdSetCursorQ(6,1);
        lcdWriteByteQ(RxBufferA[9]);
         lcdSetCursorQ(9,1);
        lcdWriteByteQ(RxBufferA[10]);
        lcdSetCursorQ(12,1);
        lcdWriteByteQ(RxBufferA[11]);
        lcdSetCursorQ(15,1);
        lcdWriteByteQ(RxBufferA[12]);
        lcdSetCursorQ(18,1);
        lcdWriteByteQ(RxBufferA[13]);

        lcdSetCursorQ(0,3);
        lcdWriteStringQ("Stat:");
        lcdWriteWordUnsignedQ(flashStatusCheck(FLASH_RDSR1));
        lcdWriteStringQ(", ");
        lcdWriteWordUnsignedQ(flashStatusCheck(FLASH_BRRD));
        
    }
}

void screenDebugInput(void){
    if(state!=laststate){
        //setup here
        lcdClearQ();
        lcdSetCursorQ(0,0);
        lcdWriteStringQ("Input Debug");
        lcdSetCursorQ(0,1);
        lcdWriteStringQ("Size:");
        lcdSetCursorQ(0,2);
        lcdWriteStringQ("Playing:");
        lcdSetCursorQ(0,3);
        lcdWriteStringQ("Blocks:");
    } else {
        //update here
        lcdSetCursorQ(5,1);
        lcdWriteWordUnsignedQ(sine.size);
        lcdSetCursorQ(8,2);
        lcdWriteWordUnsignedQ(sine.playing);
        lcdSetCursorQ(7,3);
        lcdWriteDecimalQ(sine.block_index, 3);
        
        lcdDrawPads(16);
    }
}

void screenNoFXmod(unsigned int col, fractional param1, fractional param2, fractional param3){
        if(fxNow!=fxLast || state!=laststate){
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
    if(fxNow!=fxLast || state!=laststate){
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
    if(fxNow!=fxLast || state!=laststate){
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
    if(fxNow!=fxLast || state!=laststate){
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

void screenBTCmod(unsigned int col, fractional param1, fractional param2, fractional param3){
    if(fxNow!=fxLast || state!=laststate){
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
        fxModPointers[fxUnits[0]](0,  ctrl.pots_scaled[FX_1], ctrl.pots_scaled[FX_2], ctrl.pots_scaled[FX_3]);
        fxModPointers[fxUnits[1]](10, ctrl.pots_scaled[FX_4], ctrl.pots_scaled[FX_5], ctrl.pots_scaled[FX_6]);
        
    } else {
        //update here 
        fxModPointers[fxUnits[0]](0,  ctrl.pots_scaled[FX_1], ctrl.pots_scaled[FX_2], ctrl.pots_scaled[FX_3]);
        fxModPointers[fxUnits[1]](10, ctrl.pots_scaled[FX_4], ctrl.pots_scaled[FX_5], ctrl.pots_scaled[FX_6]);
    }
         
}

void screenSHIFT(void){
    if(state!=laststate){
        //setup here
        lcdClearQ();
        lcdSetCursorQ(0,0);
        lcdWriteStringQ("SHIFT");
        lcdSetCursorQ(0,1);
        lcdWriteStringQ("Volume: ");
    } else {
        //update here 
        lcdSetCursorQ(8,1);
        lcdWriteDecimalQ(ctrl.pots_scaled[POT_VOLUME], 3);
    }
}

void screenUpdate(void){
    fxNow=fxUnits[0]+fxUnits[1];
    
    switch(state){
        case start: break;
        case scrnFX:            screenFX(); 
        break;
        case scrnSHIFT:         screenSHIFT();
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


