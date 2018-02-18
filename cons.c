#include <xc.h>
#include <dsp.h>
#include "cons.h"
#include "common.h"
#include "utilities.h"
#include "flash.h"

extern struct clip_flash clipmap[FLASH_NUMCHUNKS];
extern struct ctrlsrfc ctrl;
extern struct sflags stat;
extern unsigned long readQueue[VOICES];

extern fractional       RxBufferA[FLASH_DMA_RX_WORDS]__attribute__((space(xmemory)));

void consPADops(fractional* stream){
    int i;
    stat.dma_queue=stat.dma_rx_index=0;
    
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
                clipmap[i].size_chunks=(clipmap[i].write_index-clipmap[i].start_address)/(2*STREAMBUF);
                clipmap[i].end_chunk=clipmap[i].size_chunks;
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
            if(ctrl.pad[i]==3){
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
                    clipmap[i].start_chunk=0;
                    clipmap[i].end_chunk=0;
                    clipmap[i].size_chunks=0;
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
                if(stat.dma_queue<VOICES){                  // Check queue size to avoid overflow
                    readQueue[stat.dma_queue++]=clipmap[i].read_index;
                    if(clipmap[i].read_index<clipmap[i].end_address)
                        clipmap[i].read_index+=FLASH_PAGE;
                    else if(clipmap[i].loop) 
                        clipmap[i].read_index=clipmap[i].start_address;    // Loop-back
                    else 
                        clipmap[i].action=0;
                }
                else 
                        clipmap[i].action=0;
            } 
            else {
                clipmap[i].read_index=clipmap[i].start_address;
            }
        }
    }
    
    
    // Kick off reads
    stat.dma_framesize=stat.dma_queue;
    stat.dma_queue=0;
    if(stat.dma_queue<stat.dma_framesize){
        flashStartRead(readQueue[0], &RxBufferA[0]);
        stat.dma_rx_index+=FLASH_DMAXFER_WORDS;
        stat.dma_queue++;
    }
}

void consEDITONEops(void){    
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

void consEDITTWOops(void){    
    int tempp;
    
    if(ctrl.pot_moved[0]){
        tempp= scalePotsCustom(clipmap[ctrl.last_pressed].end_chunk , ctrl.pots[0]);
        clipmap[ctrl.last_pressed].start_chunk= tempp;
    }
    // gate
    if(ctrl.pot_moved[2]){
        tempp= scalePotsCustom(clipmap[ctrl.last_pressed].size_chunks , ctrl.pots[2]);
        clipmap[ctrl.last_pressed].end_chunk= tempp;
        if(tempp<clipmap[ctrl.last_pressed].start_chunk)
            clipmap[ctrl.last_pressed].start_chunk= tempp;
    }
    // choke
    if(ctrl.pot_moved[4]){
        if(ctrl.pots[4]>=0x3FFF) 
            clipmap[ctrl.last_pressed].choke = TRUE;
        else 
            clipmap[ctrl.last_pressed].choke = FALSE;
    }
}


    
    