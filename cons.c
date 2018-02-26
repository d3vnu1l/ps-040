#include <xc.h>
#include <dsp.h>
#include "cons.h"
#include "common.h"
#include "definitions.h"
#include "utilities.h"
#include "flash.h"
#include <stdio.h>              /* Required for printf */
#include <stdarg.h>             /* Required for printf */

extern struct clip_flash clipmap[FLASH_NUMCHUNKS];
extern struct ctrlsrfc ctrl;
extern struct sflags stat;
extern struct dmaVars dmaStat;
extern struct bluetooth bluet;
extern unsigned long readQueue[VOICES];

extern fractional RxBufferA[FLASH_DMA_RX_WORDS]__attribute__((space(xmemory)));

void consPADops(fractional* source){
    int i;
    dmaStat.dma_writeQ_index = -1;
    dmaStat.dma_queue=dmaStat.dma_rx_index=0;
    
    if(ctrl.pad[BTN_ENCSPEC]==2){
        flashBulkErase();
    }
    
    //check write triggers
    for(i=0; i<FLASH_NUMCHUNKS; i++){  
        /* Check for erase triggers */
        if(ctrl.pad[BTN_ENC]>=2){
            if(ctrl.pad[i]==2){
                clipmap[i].action=3;            // Erase
            }
        }    
        /* Check for record triggers */
        else if(ctrl.pad[BTN_SPECIAL]>=2 && clipmap[i].action==0){
            if(ctrl.pad[i]==2){
                clipmap[i].action=2;            // Trigger record
            }
        }
        /* Check for read triggers */
        else if(ctrl.pad[i]==2 && clipmap[i].action<2){                             // Pressed
            if(clipmap[i].end_address!=clipmap[i].start_address){
                if(!clipmap[i].gate){
                    if(clipmap[i].action==0)
                        clipmap[i].action=1;
                    else {
                        clipmap[i].action=0;
                        clipmap[i].read_index=clipmap[i].start_address+(clipmap[i].start_chunk*FLASH_PAGE);
                    }
                }
                else 
                    clipmap[i].action=1;
            }
        }
        
 
        /* ERASE actions */
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
        /* PLAY actions */
        else if(clipmap[i].action==1){
            if (ctrl.pad[i]==1){                       // Depressed
                if(clipmap[i].gate){
                    clipmap[i].action=0;
                    clipmap[i].read_index=clipmap[i].start_address+(clipmap[i].start_chunk*FLASH_PAGE);
                }
            }
            else if(dmaStat.dma_queue<VOICES){                  // Check queue size to avoid overflow
                readQueue[dmaStat.dma_queue++]=clipmap[i].read_index;
                if(clipmap[i].read_index<clipmap[i].end_address)
                    clipmap[i].read_index+=FLASH_PAGE;
                else if(clipmap[i].loop) 
                    clipmap[i].read_index=clipmap[i].start_address+(clipmap[i].start_chunk*FLASH_PAGE);    // Loop-back
                else{ 
                    clipmap[i].read_index=clipmap[i].start_address+(clipmap[i].start_chunk*FLASH_PAGE);
                    clipmap[i].action=0;
                }
            }
            else 
                clipmap[i].action=0;
        }
        /* RECORD actions */
        else if(clipmap[i].action==2){
            stat.recording=TRUE;
            dmaStat.dma_write_buffer=source;
            dmaStat.dma_writeQ_index=i;                                             //Store write pad
            if(clipmap[i].write_index<clipmap[i].end_limit)
                clipmap[i].write_index+=FLASH_PAGE;
            
            if(ctrl.pad[i]==1){                                    // Reset case 1
                stat.recording=FALSE;
                clipmap[i].action=0;            // off
                clipmap[i].end_address=clipmap[i].write_index;
                clipmap[i].size_chunks=(clipmap[i].write_index-clipmap[i].start_address)/FLASH_PAGE;
                clipmap[i].end_chunk=clipmap[i].size_chunks;
            } 
        }
    }
    
    // Kick off reads
    dmaStat.dma_framesize=dmaStat.dma_queue;
    dmaStat.dma_queue=0;
    if(dmaStat.dma_queue<dmaStat.dma_framesize){
        dmaStat.dma_rts=FALSE;
        flashStartRead(readQueue[0], &RxBufferA[0]);
        dmaStat.dma_rx_index+=FLASH_DMAXFER_WORDS;
        dmaStat.dma_queue++;
    }
    else if(dmaStat.dma_writeQ_index!=-1){
        dmaStat.dma_rts=TRUE;
    }
}

void consEDITONEops(void){    
    // loop
    if(ctrl.pot_moved[0]){
        if(ctrl.pots_filtered[0]>=0x3FFF) 
            clipmap[ctrl.last_pressed].loop = TRUE;
        else 
            clipmap[ctrl.last_pressed].loop = FALSE;
    }
    // gate
    if(ctrl.pot_moved[2]){
        if(ctrl.pots_filtered[2]>=0x3FFF) 
            clipmap[ctrl.last_pressed].gate = TRUE;
        else 
            clipmap[ctrl.last_pressed].gate = FALSE;
    }
    // choke
    if(ctrl.pot_moved[4]){
        if(ctrl.pots_filtered[4]>=0x3FFF) 
            clipmap[ctrl.last_pressed].choke = TRUE;
        else 
            clipmap[ctrl.last_pressed].choke = FALSE;
    }
}

void consEDITTWOops(void){    
    int tempp;
    
    if(ctrl.pot_moved[0]){
        tempp= scalePotCustom(clipmap[ctrl.last_pressed].end_chunk , ctrl.pots_filtered[0]);
        clipmap[ctrl.last_pressed].start_chunk= tempp;
        clipmap[ctrl.last_pressed].read_index=clipmap[ctrl.last_pressed].start_address+(clipmap[ctrl.last_pressed].start_chunk*FLASH_PAGE);
    }
    if(ctrl.pot_moved[2]){
        tempp= scalePotCustom(clipmap[ctrl.last_pressed].size_chunks , ctrl.pots_filtered[2]);
        clipmap[ctrl.last_pressed].end_chunk= tempp;
        if(tempp<clipmap[ctrl.last_pressed].start_chunk)
            clipmap[ctrl.last_pressed].start_chunk= tempp;
            clipmap[ctrl.last_pressed].end_address=clipmap[ctrl.last_pressed].start_address+clipmap[ctrl.last_pressed].end_chunk*FLASH_PAGE;
    }
}

void consBTops(void){
    
    flashWritePage(bluet.rxBuf, clipmap[ctrl.last_pressed].write_index);

    clipmap[ctrl.last_pressed].end_address=clipmap[ctrl.last_pressed].write_index;
    clipmap[ctrl.last_pressed].size_chunks=(clipmap[ctrl.last_pressed].write_index-clipmap[ctrl.last_pressed].start_address)/FLASH_PAGE;
    clipmap[ctrl.last_pressed].end_chunk=clipmap[ctrl.last_pressed].size_chunks;
    
    if(clipmap[ctrl.last_pressed].write_index<clipmap[ctrl.last_pressed].end_limit)
        clipmap[ctrl.last_pressed].write_index+=FLASH_PAGE;
    else{                              // Reset case 1
        clipmap[ctrl.last_pressed].end_address=clipmap[ctrl.last_pressed].write_index;
        clipmap[ctrl.last_pressed].size_chunks=(clipmap[ctrl.last_pressed].write_index-clipmap[ctrl.last_pressed].start_address)/FLASH_PAGE;
        clipmap[ctrl.last_pressed].end_chunk=clipmap[ctrl.last_pressed].size_chunks;
    }
    //printf("AA\n");
    bluet.dataReady=FALSE;
}
    
void consBTATops(void){
    if(ctrl.pad[0]==2)      printf("AT+ROLE=1\r\n");
    else if(ctrl.pad[1]==2) printf("AT+UART=115200,0,0\r\n");
    else if(ctrl.pad[2]==2) printf("AT+RESET\r\n");
    else if(ctrl.pad[3]==2) printf("AT+ROLE=0\r\n");
}