/*
 * File:   flash.c
 * Author: root
 *
 * Created on January 28, 2018, 5:00 PM
 */


#include <xc.h>
#include <dsp.h>
#include "flash.h"
#include "common.h"
#include "utilities.h"

//char flashBuf[STREAMBUF*2];
char receive;

extern unsigned char    TxBufferA[FLASH_DMAXFER_BYTES]__attribute__((space(xmemory))), 
                        RxBufferA[FLASH_DMAXFER_BYTES]__attribute__((space(xmemory)));
extern fractional       RxBufferB[STREAMBUF] __attribute__((space(xmemory)));

extern unsigned char FLASH_DMA, DMA_READING, DMA_JUSTREAD;

void flashWriteReg(char command) {
    if(FLASH_DMA==FALSE){
        SS3a=0;
        SPI3BUF=command;               //WEL=1 for write enable
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        SS3a=1;
    }
}

void flashWriteBreg(char newreg){
    if(FLASH_DMA==FALSE){
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
    if(FLASH_DMA==FALSE){
        SS3a=0;
        SPI3BUF=command;               //WEL=1 for write enable
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        SPI3BUF=0x00;               //WEL=1 for write enable
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
        SS3a=1;
        return receive;
    }
    return 0;
}

void flashWritePage(fractional* source, long address){
    if(FLASH_DMA==FALSE){
        int i;
        flashWriteReg(FLASH_WREN);
        FLASH_DMA=TRUE;
        fractional sample;
        
        

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


        DMA1CONbits.CHEN = 1;
        DMA0CONbits.CHEN = 1;
        DMA0REQbits.FORCE = 1; // Manual mode: Kick-start the 1st transfer           
        //while(!_DMA1IF);  //works  
    }
}

void flashStartRead(long address){
    if(FLASH_DMA==FALSE){
        int i;

        for(i=0; i<FLASH_DMAXFER_BYTES; i++){
            TxBufferA[i]=0;
        }

        FLASH_DMA=TRUE;
        DMA_READING=TRUE;
        DMA_JUSTREAD=FALSE;
        
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
        DMA1CONbits.CHEN = 1;
        DMA0CONbits.CHEN = 1;
        DMA0REQbits.FORCE = 1; // Manual mode: Kick-start the 1st transfer
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

void flashEraseSector(long address){
    if(FLASH_DMA==FALSE){
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
