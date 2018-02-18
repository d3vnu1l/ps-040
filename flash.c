/*
 This file has hardware routines for flash writing and DMA read/writes
 */
#include <xc.h>
#include <dsp.h>
#include "flash.h"
#include "common.h"

char receive;
unsigned long  eraseAddr=0;
struct clip_flash clipmap[FLASH_NUMCHUNKS];


extern struct sflags stat;
extern struct ctrlsrfc ctrl;

extern fractional       TxBufferA[FLASH_DMAXFER_WORDS]__attribute__((space(xmemory))), 
                        RxBufferA[FLASH_DMA_RX_WORDS]__attribute__((space(xmemory)));

void flashSoftSetup(void){
    int i=1;
    
    clipmap[0].start_address=0;
    clipmap[0].read_index=clipmap[0].start_address;
    clipmap[0].write_index=clipmap[0].start_address;
    clipmap[0].erase_index=clipmap[0].start_address;
    clipmap[0].end_address=clipmap[0].start_address;
    clipmap[0].start_chunk=0;
    clipmap[0].end_chunk=0;
    clipmap[0].size_chunks=0;
    clipmap[0].gate=TRUE;
    clipmap[0].loop=FALSE;
    clipmap[0].choke=FALSE;
    clipmap[0].action=0;
    
    for(; i<FLASH_NUMCHUNKS; i++){
        clipmap[i].start_address=CHUNKSIZE*i;
        clipmap[i].read_index=clipmap[i].start_address;
        clipmap[i].write_index=clipmap[i].start_address;
        clipmap[i].erase_index=clipmap[i].start_address;
        clipmap[i].end_address=clipmap[i].start_address;
        clipmap[i].start_chunk=0;
        clipmap[i].end_chunk=0;
        clipmap[i].size_chunks=0;
        clipmap[i].gate=TRUE;
        clipmap[i].loop=FALSE;
        clipmap[i].choke=FALSE;
        clipmap[i].action=0;
        
        clipmap[i-1].end_limit=clipmap[i].start_address;
    }
    clipmap[FLASH_NUMCHUNKS-1].end_limit=FLASH_MAX;
}

void flashWriteReg(char command) {
    if(SS3a){
        SS3a=0;
        SPI3BUF=command;               //WEL=1 for write enable
        while(!_SPI3IF);
        SS3a=1;
        _SPI3IF=0;
        receive=SPI3BUF;
    }
}

void flashWriteBreg(char newreg){
    if(SS3a){
        //flashWriteReg(FLASH_WREN);
        SS3a=0;
        SPI3BUF=FLASH_BRWR;               //WEL=1 for write enable
        while(!_SPI3IF); 
        _SPI3IF=0;
        receive=SPI3BUF;
        SPI3BUF=newreg;               //WEL=1 for write enable
        while(!_SPI3IF); 
        SS3a=1;
        _SPI3IF=0;
        receive=SPI3BUF;
        
    }
}

char flashStatusCheck(char command){
    if(SS3a){
        SS3a=0;
        SPI3BUF=command;               //WEL=1 for write enable
        while(!_SPI3IF); 
        _SPI3IF=0;
        receive=SPI3BUF;
        SPI3BUF=0x00;               //WEL=1 for write enable
        while(!_SPI3IF); 
        SS3a=1;
        _SPI3IF=0;
        receive=SPI3BUF;
        
        return receive;
    }else return 0xFF;
}

void flashWritePage(fractional* source, unsigned long address){
    if(SS3a){
        int i;
        fractional sample;
        char* sendPtr = (char*)(source);
        
        flashWriteReg(FLASH_WREN);
        
        SS3a=0;
        SPI3BUF=FLASH_PP;
        while(!_SPI3IF); 
        _SPI3IF=0;
        receive=SPI3BUF;
        
        SPI3BUF=(address & 0xff000000UL) >> 24;                 
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        SPI3BUF=(address & 0x00ff0000UL) >> 16;                 
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        SPI3BUF=(address & 0x0000ff00UL) >>  8;                 
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        SPI3BUF=(address & 0x000000ffUL);            
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        
        /* Kick off dma write here */
        DMA0STAL = (unsigned int)(sendPtr);
        DMA1STAL = (unsigned int)(&RxBufferA);
        DMA1CONbits.NULLW=0;                          // NULL WRITE (debug))
        DMA0CONbits.CHEN = 1;
        DMA1CONbits.CHEN = 1;
        DMA0REQbits.FORCE = 1; // Manual mode: Kick-start the 1st transfer 
        //SPI3BUF = 0x00;
        //while (DMA0REQbits.FORCE == 1);
        //DMA1REQbits.FORCE = 1; // Manual mode: Kick-start the 1st transfer  
    }
}

void flashStartRead(unsigned long address, fractional* rcvPtr){
    if(SS3a){
        int i;  
        
        SS3a=0;
        SPI3BUF=FLASH_READ;
        while(!_SPI3IF); 
        _SPI3IF=0;
        receive=SPI3BUF;
        
        SPI3BUF=(address & 0xff000000UL) >> 24;                 
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        SPI3BUF=(address & 0x00ff0000UL) >> 16;                 
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        SPI3BUF=(address & 0x0000ff00UL) >>  8;                 
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        SPI3BUF=(address & 0x000000ffUL);            
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;

        DMA0STAL = (unsigned int)(&TxBufferA);
        DMA1STAL = (unsigned int)(rcvPtr);
        
        
        /* Kick off dma read here */
        DMA1CONbits.NULLW=1;                          // NULL WRITE (debug))
        DMA0CONbits.CHEN = 1;
        DMA1CONbits.CHEN = 1;
        DMA0REQbits.FORCE = 1; // Manual mode: Kick-start the 1st transfer
        //SPI3BUF = 0x00;
        //while (DMA0REQbits.FORCE == 1);
        //DMA1REQbits.FORCE = 1; // Manual mode: Kick-start the 1st transfer
    }
}

void flashEraseSector(unsigned long address){
    if(SS3a){
        flashWriteReg(FLASH_WREN);
        SS3a=0;
        SPI3BUF=FLASH_SE;                 
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        SPI3BUF=(address & 0xff000000UL) >> 24;                 
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        SPI3BUF=(address & 0x00ff0000UL) >> 16;                 
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        SPI3BUF=(address & 0x0000ff00UL) >>  8;                 
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        SPI3BUF=(address & 0x000000ffUL);            
        while(!_SPI3IF); 
        SS3a=1;
        _SPI3IF=0;
        receive=SPI3BUF;
        
    }
}

void flashBulkErase(void) {
    flashWriteReg(FLASH_WREN);
    flashWriteReg(FLASH_FRMT);
}

