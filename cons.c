#include <xc.h>
#include <dsp.h>
#include "cons.h"
#include "common.h"
#include "flash.h"

extern struct clip_flash clipmap[FLASH_NUMCHUNKS];
extern enum fxStruct fxUnits[NUMFXUNITS];
extern struct ctrlsrfc ctrl;

//enum action_states;

void consPADops(fractional* stream){
    int i;
    
    if(ctrl.pad[BTN_ENCSPEC]==2){
        flashBulkErase();
    }
    
    //check write triggers
    for(i=0; i<FLASH_NUMCHUNKS; i++){
        /* Check for records */
        if(ctrl.pad[BTN_SPECIAL]==3){
            if(ctrl.pad[i]==2){
                clipmap[i].action=2;            // Record
            }
            else if(ctrl.pad[i]==1){
                clipmap[i].action=0;            // off
                clipmap[i].end_address=clipmap[i].write_index;
            }
        }
        else if(ctrl.pad[BTN_SPECIAL]==1 && clipmap[i].action==2){
            clipmap[i].action=0;                // off
            clipmap[i].end_address=clipmap[i].write_index;
        }
        
        if(clipmap[i].action==2){
            flashWritePage(stream, clipmap[i].write_index);
            if(clipmap[i].write_index<clipmap[i].end_limit)
                clipmap[i].write_index+=FLASH_PAGE;
        }
        /* Check for erase */
        if(ctrl.pad[BTN_ENC]==3){
            if(ctrl.pad[i]==2){
                clipmap[i].action=3;            // Erase
            }
        }
        if(clipmap[i].action==3){
            if(flashStatusCheck(FLASH_RDSR1)&&0x04);
            else{
                flashEraseSector(clipmap[i].erase_index);
                if(clipmap[i].erase_index<(clipmap[i].end_limit-FLASH_SECTOR))
                    clipmap[i].erase_index+=FLASH_SECTOR;
                else {
                    clipmap[i].action=0; 
                    clipmap[i].write_index=clipmap[i].start_address;
                    clipmap[i].erase_index=clipmap[i].start_address;
                    clipmap[i].end_address=clipmap[i].start_address;
                }
            }
        } 
        /* Check for read */
        if(ctrl.pad[BTN_ENC]==0){
            if(ctrl.pad[i]==2){                             // Pressed
                if(clipmap[i].end_address!=clipmap[i].start_address){
                    if(!clipmap[i].gate){
                        if(clipmap[i].action==0)
                            clipmap[i].action=1;
                        else 
                            clipmap[i].action=0;
                    }
                    else 
                        clipmap[i].action=1;
                }
            }
            else if (ctrl.pad[i]==1){                       // Depressed
                if(clipmap[i].gate)
                    clipmap[i].action=0;
            }
            if(clipmap[i].action==1){
                flashStartRead(clipmap[i].read_index);      // READBACK
                if(clipmap[i].read_index<clipmap[i].end_address)
                    clipmap[i].read_index+=FLASH_PAGE;
                else if(clipmap[i].loop) 
                    clipmap[i].read_index=clipmap[i].start_address;    // Loop-back
                else 
                    clipmap[i].action=0;
            } 
            else {
                clipmap[i].read_index=clipmap[i].start_address;
            }
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


    
    