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


extern unsigned char    TxBufferA[FLASH_DMAXFER_BYTES]__attribute__((space(xmemory))), 
                        RxBufferA[FLASH_DMAXFER_BYTES]__attribute__((space(xmemory)));
extern fractional       RxBufferB[STREAMBUF] __attribute__((space(xmemory)));

extern struct sflags stat;
extern struct ctrlsrfc ctrl;

void flashSoftSetup(void){
    int i=1;
    unsigned long chunksize = (FLASH_MAX/FLASH_NUMCHUNKS);
    
    clipmap[0].start_address=0;
    clipmap[0].read_index=clipmap[0].start_address;
    clipmap[0].write_index=clipmap[0].start_address;
    clipmap[0].erase_index=clipmap[0].start_address;
    clipmap[0].gate=TRUE;
    clipmap[i].loop=FALSE;
    clipmap[0].choke=FALSE;
    clipmap[0].playing=FALSE;
    
    for(; i<FLASH_NUMCHUNKS; i++){
        clipmap[i].start_address=chunksize*i;
        clipmap[i].read_index=clipmap[i].start_address;
        clipmap[i].write_index=clipmap[i].start_address;
        clipmap[i].erase_index=clipmap[i].start_address;
        clipmap[i].gate=TRUE;
        clipmap[i].loop=FALSE;
        clipmap[i].choke=FALSE;
        clipmap[i].playing=FALSE;
        
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
        while(!_SPI3IF); _SPI3IF=0;
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
        while(!_SPI3IF); _SPI3IF=0;
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
        
        flashWriteReg(FLASH_WREN);
        
        for(i=0; i<FLASH_DMAXFER_BYTES; i++){
            sample=*source++;
            TxBufferA[i++]=(sample>>8)&0xFF;
            TxBufferA[i]=sample&0xFF;
        }
        SS3a=0;

        SPI3BUF=FLASH_PP;
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
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        
        //SPI3STATbits.SPIROV = 0;    // Clear SPI1 receive overflow flag if set
        //DMA1CONbits.NULLW=0;                          // NULL WRITE (debug))
        DMA0CONbits.CHEN = 1;
        DMA1CONbits.CHEN = 1;
        DMA0REQbits.FORCE = 1; // Manual mode: Kick-start the 1st transfer 
        //SPI3BUF = 0x00;
        //while (DMA0REQbits.FORCE == 1);
        //DMA1REQbits.FORCE = 1; // Manual mode: Kick-start the 1st transfer  
    }
}

void flashStartRead(unsigned long address){
    if(SS3a){
        int i;

        for(i=0; i<FLASH_DMAXFER_BYTES; i++){
            TxBufferA[i]=0;
        }

        stat.DMA_READING=TRUE;
        stat.DMA_JUSTREAD=FALSE;
        
        SS3a=0;
        SPI3BUF=FLASH_READ;
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
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;

        /* Kick off dma read here */
        //SPI3STATbits.SPIROV = 0;    // Clear SPI1 receive overflow flag if set
        //DMA1CONbits.NULLW=1;                          // NULL WRITE (debug))
        DMA0CONbits.CHEN = 1;
        DMA1CONbits.CHEN = 1;
        DMA0REQbits.FORCE = 1; // Manual mode: Kick-start the 1st transfer
        //SPI3BUF = 0x00;
        //while (DMA0REQbits.FORCE == 1);
        //DMA1REQbits.FORCE = 1; // Manual mode: Kick-start the 1st transfer
    }
}

void flashProcessRead(void){
    int i,j; 
    
    for(i=j=0; i<STREAMBUF; i++){
        unsigned int temp = (RxBufferA[j++]<<8)&0xFF00;
        RxBufferB[i]=(temp|RxBufferA[j++]);
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

