#include <xc.h>
#include <dsp.h>
#include "controls.h"
#include "common.h"
#include "flash.h"

extern struct clip_flash clipmap[FLASH_NUMCHUNKS];
extern struct ctrlsrfc ctrl;

void consFXops(fractional* stream){
    int i;
    
    if(ctrl.pad[33]>1) flashBulkErase();
    
    //check write triggers
    for(i=0; i<FLASH_NUMCHUNKS; i++){
        if(ctrl.pad[i+17]>1){    //use shifted pads to trigger recording
            flashWritePage(stream, clipmap[i].write_index);
            if(clipmap[i].write_index<clipmap[i].end_address){
                clipmap[i].write_index+=FLASH_PAGE;
            }
            else {
                clipmap[i].write_index=clipmap[i].start_address;
            }
        } 
        else {
            clipmap[i].write_index=clipmap[i].start_address;
        }
    }
    
    //check read triggers
    if(ctrl.pad[BTN_ENC]<2){
        for(i=0; i<FLASH_NUMCHUNKS; i++){
            if(ctrl.pad[i]>1){
                flashStartRead(clipmap[i].read_index);     // READBACK
                if(clipmap[i].read_index<clipmap[i].end_address){
                    clipmap[i].read_index+=FLASH_PAGE;
                }
                else {
                    clipmap[i].read_index=clipmap[i].start_address;
                }
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
                    if(clipmap[i].write_index!=clipmap[i].end_address)
                        clipmap[i].erase_index+=FLASH_SECTOR;
                }
            } else clipmap[i].erase_index=clipmap[i].start_address;
        }
    }
}

