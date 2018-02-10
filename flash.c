/*
 This file has hardware routines for flash writing and DMA read/writes
 */
#include <xc.h>
#include <dsp.h>
#include "flash.h"
#include "common.h"

char receive;
unsigned long  writeAddr=0, readAddr=0, eraseAddr=0;

extern unsigned char    TxBufferA[FLASH_DMAXFER_BYTES]__attribute__((space(xmemory))), 
                        RxBufferA[FLASH_DMAXFER_BYTES]__attribute__((space(xmemory)));
extern fractional       RxBufferB[STREAMBUF] __attribute__((space(xmemory)));

extern struct sflags stat;
extern struct ctrlsrfc ctrl;


void flashWriteReg(char command) {
    if(stat.FLASH_DMA==FALSE){
        SS3a=0;
        SPI3BUF=command;               //WEL=1 for write enable
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        SS3a=1;
    }
}

void flashWriteBreg(char newreg){
    if(stat.FLASH_DMA==FALSE){
        flashWriteReg(FLASH_WREN);
        SS3a=0;
        SPI3BUF=FLASH_BRWR;               //WEL=1 for write enable
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        SPI3BUF=newreg;               //WEL=1 for write enable
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        SS3a=1;
    }
}

char flashStatusCheck(char command){
    if(stat.FLASH_DMA==FALSE){
        SS3a=0;
        SPI3BUF=command;               //WEL=1 for write enable
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        SPI3BUF=0x00;               //WEL=1 for write enable
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        SS3a=1;
        return receive;
    }else return 0xFF;
}

void flashWritePage(fractional* source, unsigned long address){
    if(stat.FLASH_DMA==FALSE){
        int i;
        fractional sample;
        
        flashWriteReg(FLASH_WREN);
        stat.FLASH_DMA=TRUE;
        
        for(i=0; i<FLASH_DMAXFER_BYTES; i++){
            sample=*source++;
            TxBufferA[i++]=(sample>>8)&0xFF;
            TxBufferA[i]=sample&0xFF;
        }
        SS3a=0;

        SPI3BUF=FLASH_PP;
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        SPI3BUF=(address>>16)&0xFF;                  
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        SPI3BUF=(address>>8)&0xFF;                  
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        SPI3BUF=(address&0xFF);               
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;

        //DMA1CONbits.NULLW=0;                          // NULL WRITE (debug))
        DMA0CONbits.CHEN = 1;
        DMA1CONbits.CHEN = 1;
        DMA0REQbits.FORCE = 1; // Manual mode: Kick-start the 1st transfer 
        //SPI3BUF = 0x00;
        //while (DMA0REQbits.FORCE == 1);
        //DMA1REQbits.FORCE = 1; // Manual mode: Kick-start the 1st transfer  
        //while(!_DMA1IF);  //works  
    }
}

void flashStartRead(unsigned long address){
    if(stat.FLASH_DMA==FALSE){
        int i;

        for(i=0; i<FLASH_DMAXFER_BYTES; i++){
            TxBufferA[i]=0;
        }

        stat.FLASH_DMA=TRUE;
        stat.DMA_READING=TRUE;
        stat.DMA_JUSTREAD=FALSE;
        
        SS3a=0;
        SPI3BUF=FLASH_READ;
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        SPI3BUF=(address>>16)&0xFF;                  
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        SPI3BUF=(address>>8)&0xFF;                  
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        SPI3BUF=(address&0xFF);               
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;

        //DMA1STAL = (unsigned int)(&RxBufferA);
        //DMA0STAL = (unsigned int)(&outputA);
        //DMA1CNT = (unsigned int)(FLASH_DMAXFERS-1);
        //DMA0CNT = (unsigned int)(FLASH_DMAXFERS-1);

        /* Kick off dma read here */
        //DMA1CONbits.NULLW=1;                          // NULL WRITE (debug))
        DMA0CONbits.CHEN = 1;
        DMA1CONbits.CHEN = 1;
        DMA0REQbits.FORCE = 1; // Manual mode: Kick-start the 1st transfer
        //SPI3BUF = 0x00;
        //while (DMA0REQbits.FORCE == 1);
        //DMA1REQbits.FORCE = 1; // Manual mode: Kick-start the 1st transfer
        //while(!_DMA1IF);  //works
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
    if(stat.FLASH_DMA==FALSE){
        flashWriteReg(FLASH_WREN);
        SS3a=0;
        SPI3BUF=FLASH_SE;                 
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        SPI3BUF=(address>>16)&0xFF;                  
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        SPI3BUF=(address>>8)&0xFF;                  
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        SPI3BUF=(address&0xFF);               
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        SS3a=1;
    }
}

void flashBulkErase(void) {
    flashWriteReg(FLASH_WREN);
    flashWriteReg(FLASH_FRMT);
}

void flashFXops(fractional* stream){
    if(!ctrl.pad[33]) flashBulkErase();
    
    if(!ctrl.pad[3]){
        flashWritePage(stream, writeAddr);
        writeAddr+=FLASH_PAGE;
    } else writeAddr=0;
    
    if(!ctrl.pad[4]){
        flashStartRead(readAddr);     // READBACK
        readAddr+=FLASH_PAGE;
    } else readAddr=0;
    
    if(!ctrl.pad[5]){
        if(flashStatusCheck(FLASH_RDSR1)==0x03);
        else{
            flashEraseSector(eraseAddr);
            eraseAddr+=FLASH_PAGE;
        }
    } else eraseAddr=0;
    if(!ctrl.pad[6])flashWriteReg(FLASH_WREN);
    
    if(!ctrl.pad[7]) stat.TEST_SIN=TRUE;
    else stat.TEST_SIN = FALSE;
}
