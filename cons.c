#include <xc.h>
#include <dsp.h>
#include "cons.h"
#include "common.h"
#include "flash.h"

extern struct clip_flash clipmap[FLASH_NUMCHUNKS];
extern enum fxStruct fxUnits[NUMFXUNITS];
extern struct ctrlsrfc ctrl;
extern *fxFuncPointers;

void consPADops(fractional* stream){
    int i;
    
    if(ctrl.pad[33]>1){
        flashBulkErase();
    }
    
    //check write triggers
    for(i=0; i<FLASH_NUMCHUNKS; i++){
        if(ctrl.pad[i+17]==3){                              // Use shifted pads to trigger recording
            flashWritePage(stream, clipmap[i].write_index);
            if(clipmap[i].write_index<clipmap[i].end_limit){
                clipmap[i].write_index+=FLASH_PAGE;
            }
        } 
    }
    
    //check read triggers
    if(ctrl.pad[BTN_ENC]==0){
        for(i=0; i<FLASH_NUMCHUNKS; i++){
            
            if(ctrl.pad[i]==2){                         // Pressed
                clipmap[i].playing=!clipmap[i].playing;
            }
            else if (ctrl.pad[i]==1){                   // Depressed
                if(clipmap[i].gate)
                    clipmap[i].playing=FALSE;
            }
            
            if(clipmap[i].playing){
                flashStartRead(clipmap[i].read_index);      // READBACK
                if(clipmap[i].read_index<clipmap[i].write_index){
                    clipmap[i].read_index+=FLASH_PAGE;
                }
                else if(clipmap[i].loop) clipmap[i].read_index=clipmap[i].start_address;    // Loop-back
                else clipmap[i].playing=FALSE;
            } 
            else {
                clipmap[i].read_index=clipmap[i].start_address;
            }
        }
    }
    
    // Check erase
    if(ctrl.pad[BTN_ENC]>1){
        for(i=0; i<FLASH_NUMCHUNKS; i++){
            if(ctrl.pad[i]>1){
                if(flashStatusCheck(FLASH_RDSR1)&&0x04);
                else{
                    flashEraseSector(clipmap[i].erase_index);
                    if(clipmap[i].write_index!=clipmap[i].end_limit)
                        clipmap[i].erase_index+=FLASH_SECTOR;
                }
            } else clipmap[i].erase_index=clipmap[i].start_address;
        }
    }
}

void consEDITops(void){    
    // loop
    if(ctrl.pot_moved[0]){
        if(ctrl.pots[0]>=0x3FFF) 
            clipmap[ctrl.last_pressed].loop = TRUE;
        else 
            clipmap[ctrl.last_pressed].loop = FALSE;
    }
    // gate
    if(ctrl.pot_moved[2]){
        if(ctrl.pots[2]>=0x3FFF) 
            clipmap[ctrl.last_pressed].gate = TRUE;
        else 
            clipmap[ctrl.last_pressed].gate = FALSE;
    }
    // choke
    if(ctrl.pot_moved[4]){
        if(ctrl.pots[4]>=0x3FFF) 
            clipmap[ctrl.last_pressed].choke = TRUE;
        else 
            clipmap[ctrl.last_pressed].choke = FALSE;
    }
}


    
    