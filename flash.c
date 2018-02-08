/*
 * File:   flash.c
 * Author: root
 *
 * Created on January 28, 2018, 5:00 PM
 */


#include "xc.h"
#include "flash.h"
#include "common.h"
#include "utilities.h"

//char flashBuf[STREAMBUF*2];
char receive;

extern unsigned char  TxBufferA[FLASH_DMAXFERS]__attribute__((space(xmemory))),
                     TxBufferB[FLASH_DMAXFERS]__attribute__((space(xmemory))), 
                     RxBufferA[FLASH_DMAXFERS]__attribute__((space(xmemory))),
                     RxBufferB[FLASH_DMAXFERS]__attribute__((space(xmemory))); 

extern fractional outputA[STREAMBUF], outputB[STREAMBUF];
extern fractional streamA[STREAMBUF], streamB[STREAMBUF];

extern unsigned char FLASH_DMA;

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

void flashWritePage(int addressH, int addressL, fractional* source){
    int i;
    
    SS3a=0;
    /*
    SPI3BUF=FLASH_PP;
    while(!_SPI3IF); _SPI3IF=0;
    receive=SPI3BUF;
    SPI3BUF=addressH&&0xFF;
    while(!_SPI3IF); _SPI3IF=0;
    receive=SPI3BUF;
    SPI3BUF=(addressL&&0xFF00)>>8;
    while(!_SPI3IF); _SPI3IF=0;
    receive=SPI3BUF;
    SPI3BUF=addressL&&0xFF;
    while(!_SPI3IF); _SPI3IF=0;
    receive=SPI3BUF;
    SPI3BUF=addressL&&0xFF;
    while(!_SPI3IF); _SPI3IF=0;
    receive=SPI3BUF;
    */
    SPI3BUF=FLASH_PP;
    while(!_SPI3IF); _SPI3IF=0;
    receive=SPI3BUF;
    SPI3BUF=0x00;
    while(!_SPI3IF); _SPI3IF=0;
    receive=SPI3BUF;
    SPI3BUF=0x00;
    while(!_SPI3IF); _SPI3IF=0;
    receive=SPI3BUF;
    SPI3BUF=0x00;
    while(!_SPI3IF); _SPI3IF=0;
    receive=SPI3BUF;
    //SPI3BUF=0x00;
    //while(!_SPI3IF); _SPI3IF=0;
    //receive=SPI3BUF;     
    
    DMA1CONbits.CHEN = 1;
    DMA0CONbits.CHEN = 1;
    DMA0REQbits.FORCE = 1; // Manual mode: Kick-start the 1st transfer
    FLASH_DMA=TRUE;
            
    //while(!_DMA1IF);  //works        

}

void flashRead(char *array, int bytes){
    int i;
    
    SS3a=0;
    
    SPI3BUF=FLASH_READ;
    while(!_SPI3IF); _SPI3IF=0;
    receive=SPI3BUF;
    SPI3BUF=0x00;
    while(!_SPI3IF); _SPI3IF=0;
    receive=SPI3BUF;
    SPI3BUF=0x00;
    while(!_SPI3IF); _SPI3IF=0;
    receive=SPI3BUF;
    SPI3BUF=0x00;
    while(!_SPI3IF); _SPI3IF=0;
    receive=SPI3BUF;
    //SPI3BUF=0x00;
    //while(!_SPI3IF); _SPI3IF=0;
    //receive=SPI3BUF;
    
    
    //DMA1STAL = (unsigned int)(&RxBufferA);
    //DMA0STAL = (unsigned int)(&outputA);
    //DMA1CNT = (unsigned int)(FLASH_DMAXFERS-1);
    //DMA0CNT = (unsigned int)(FLASH_DMAXFERS-1);
    /* Kick off dma read here */
    DMA1CONbits.CHEN = 1;
    DMA0CONbits.CHEN = 1;
    DMA0REQbits.FORCE = 1; // Manual mode: Kick-start the 1st transfer
    FLASH_DMA=TRUE;
    //Delay_us(100);
    //while()
    //while(!_DMA1IF);  //works

}

void flashBulkErase(void) {
    flashWriteReg(FLASH_FRMT);
}
