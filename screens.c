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

extern fractional       TxBufferA[FLASH_DMAXFER_WORDS]__attribute__((space(xmemory))), 
                        RxBufferA[FLASH_DMA_RX_WORDS]__attribute__((space(xmemory)));

extern enum screenStruc state, laststate;
extern enum fxStruct fxUnits[NUMFXUNITS];
extern struct clip sine;
extern struct ctrlsrfc ctrl;
extern struct sflags stat;
extern struct clip_flash clipmap[FLASH_NUMCHUNKS];
extern struct bluetooth bluet;

int fxLast=0, fxNow=0;

void (*fxModPointers[NUMFX])(unsigned int, fractional, fractional, fractional) = {screenNoFXmod, screenLPFmod, screenTRMmod, screenLOPmod, screenBTCmod, screenHPFmod};
void (*screenPointers[SCREENS])(void) = {screenFX, screenEditOne, screenEditTwo, screenBluetooth, screenDebugPots, screenDebugFlash, screenDebugBuffers, screenDebugInput, screenSHIFT};

void screenDebugAudio(void){

}

void screenDebugBuffers(void){
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
        lcdWriteByteQ(bluet.btread);
        //lcdWriteDecimalQ();
        //lcdSetCursorQ(6,3);
        //lcdWriteDecimalQ();
        //lcdWriteDecimalQ(3);
    }
}

void screenDebugPots(void){
    int bank=0, i;
    
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
        if(ctrl.pad[BTN_SPECIAL]>1)
            bank=POTS/2;
        
        for(i=1; i<4; i++){
            lcdSetCursorQ(0,i);
            lcdWriteDecimalQ(ctrl.pots_scaled[bank], 3);
            lcdWriteQ(',');
            lcdWriteWordUnsignedQ(ctrl.pots[bank++]);
            lcdSetCursorQ(11,i);
            lcdWriteDecimalQ(ctrl.pots_scaled[bank], 3);
            lcdWriteQ(',');
            lcdWriteWordUnsignedQ(ctrl.pots[bank++]);
        }
    }
}

void screenDebugFlash(void){
    int i, j, k;
    
    if(state!=laststate){
        //setup here
        lcdClearQ();
        
        k=0;
        for(i=0; i<3; i++){
            for(j=0; j<18; j+=3){
                lcdSetCursorQ(j,i);
                lcdWriteByteQ(RxBufferA[k++]);
            }
        }  
        lcdSetCursorQ(0,3);
        lcdWriteStringQ("Stat:");
    } else {
        //update here
        
        k=0;
        for(i=0; i<3; i++){
            for(j=0; j<=18; j+=3){
                lcdSetCursorQ(j,i);
                lcdWriteByteQ(RxBufferA[k++]);
            }
        }
        lcdSetCursorQ(6,3);
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

void screenHPFmod(unsigned int col, fractional param1, fractional param2, fractional param3){
    if(fxNow!=fxLast || state!=laststate){
        // Setup here
        lcdSetCursorQ(col+5,0);
        lcdWriteStringQ("HPF");
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

void screenFX(void){
    int i,j ;
    
    if(state!=laststate){
        //setup here
        lcdClearQ();
        lcdSetCursorQ(0,0);
        lcdWriteStringQ("Fx1:");
        lcdSetCursorQ(9,0);
        lcdWriteStringQ("Fx2:");
        for(j=0; j<4; j++){
           lcdSetCursorQ(8, j);
           lcdWriteQ('|');
       }
        for(j=0; j<4; j++){
           lcdSetCursorQ(17, j);
           lcdWriteQ('|');
       }


        fxModPointers[fxUnits[0]](0,  ctrl.pots_scaled[FX_1], ctrl.pots_scaled[FX_2], ctrl.pots_scaled[FX_3]);
        fxModPointers[fxUnits[1]](9, ctrl.pots_scaled[FX_4], ctrl.pots_scaled[FX_5], ctrl.pots_scaled[FX_6]);
        
    } else {
        //update here 
        fxModPointers[fxUnits[0]](0,  ctrl.pots_scaled[FX_1], ctrl.pots_scaled[FX_2], ctrl.pots_scaled[FX_3]);
        fxModPointers[fxUnits[1]](9, ctrl.pots_scaled[FX_4], ctrl.pots_scaled[FX_5], ctrl.pots_scaled[FX_6]);
        
        lcdDrawMeter(18);
        lcdDrawMeter(19);
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
        lcdSetCursorQ(0,3);
        lcdWriteDecimalQ(ctrl.pad[BTN_SPECIAL], 1);
    }
}

void screenEditOne(void){
    int i;
    char grid[16];
    for(i=0; i<16; i++){
        if(ctrl.last_pressed==i)
            grid[i]=1;
        else grid[i]=0;
    }
    
    if(state!=laststate){
        //setup here
        lcdClearQ();
        lcdSetCursorQ(0,0);
        lcdWriteStringQ("EDIT");
        lcdSetCursorQ(0,1);
        lcdWriteStringQ("LOOP :");
        lcdSetCursorQ(0,2);
        lcdWriteStringQ("GATE :");
        lcdSetCursorQ(0,3);
        lcdWriteStringQ("CHOKE:");
        
        lcdSetCursorQ(5,0);
        lcdWriteQ('(');
        lcdSetCursorQ(8,0);
        lcdWriteStringQ(")(");
        lcdSetCursorQ(14,0);
        lcdWriteQ(')');
    } else {
        //update here 
        lcdSetCursorQ(6,0);
        lcdWriteDecimalQ((ctrl.last_pressed+1), 2);
        lcdSetCursorQ(10,0);
        if(clipmap[ctrl.last_pressed].start_address==clipmap[ctrl.last_pressed].end_address)
            lcdWriteStringQ("empt");
        else 
            lcdWriteStringQ("full");
        
        lcdSetCursorQ(7,1);
        if(clipmap[ctrl.last_pressed].loop==TRUE) 
            lcdWriteStringQ("repeat");
        else
            lcdWriteStringQ("1-shot");
        lcdSetCursorQ(7,2);
                if(clipmap[ctrl.last_pressed].gate) 
            lcdWriteStringQ("on ");
        else
            lcdWriteStringQ("off");
        lcdSetCursorQ(7,3);
        if(clipmap[ctrl.last_pressed].choke) 
            lcdWriteStringQ("y");
        else
            lcdWriteStringQ("n");
        lcdDrawSlots(16, grid);
    }
}

void screenEditTwo(void){
    int i;
    
    if(state!=laststate){
        //setup here
        lcdClearQ();
        lcdSetCursorQ(0,0);
        lcdWriteStringQ("EDIT :");
        lcdSetCursorQ(0,1);
        lcdWriteStringQ("BEGIN:");
        lcdSetCursorQ(0,2);
        lcdWriteStringQ("END  :");
        lcdSetCursorQ(0,3);
        lcdWriteStringQ("VOL  :");
        
        lcdSetCursorQ(5,0);
        lcdWriteQ('(');
        lcdSetCursorQ(8,0);
        lcdWriteStringQ(")(");
        lcdSetCursorQ(14,0);
        lcdWriteQ(')');
    } else {
        //update here 
        lcdSetCursorQ(6,0);
        lcdWriteDecimalQ((ctrl.last_pressed+1), 2);
        lcdSetCursorQ(10,0);
        if(clipmap[ctrl.last_pressed].start_address==clipmap[ctrl.last_pressed].end_address)
            lcdWriteStringQ("empt");
        else 
            lcdWriteStringQ("full");
        
        
        lcdSetCursorQ(7,1);
        lcdWriteDecimalQ(clipmap[ctrl.last_pressed].start_chunk, 4);
        lcdSetCursorQ(11,1);
        lcdWriteQ('/');
        lcdWriteDecimalQ(clipmap[ctrl.last_pressed].end_chunk, 4);
        

        lcdSetCursorQ(7,2);
        lcdWriteDecimalQ(clipmap[ctrl.last_pressed].end_chunk, 4);
        lcdSetCursorQ(11,2);
        lcdWriteQ('/');
        lcdWriteDecimalQ(clipmap[ctrl.last_pressed].size_chunks, 4);
        
        
    }
}

void screenBluetooth(void){
    int i;
    
    if(state!=laststate){
        //setup here
        lcdClearQ();
        lcdSetCursorQ(0,0);
        lcdWriteStringQ("BT LOAD");

        
        lcdSetCursorQ(9,0);
        lcdWriteQ('(');
        lcdSetCursorQ(12,0);
        lcdWriteStringQ(")(");
        lcdSetCursorQ(18,0);
        lcdWriteQ(')');
    } else {
        //update here 
        lcdSetCursorQ(10,0);
        lcdWriteDecimalQ((ctrl.last_pressed+1), 2);
        lcdSetCursorQ(14,0);
        if(clipmap[ctrl.last_pressed].start_address==clipmap[ctrl.last_pressed].end_address)
            lcdWriteStringQ("empt");
        else 
            lcdWriteStringQ("full");
    }
}

void screenUpdate(void){
    fxNow=fxUnits[0]+fxUnits[1];
    
    screenPointers[state]();
    
    laststate=state;
    fxLast=fxUnits[0]+fxUnits[1];
}