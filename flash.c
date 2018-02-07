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

extern unsigned char TxBufferA[FLASH_DMAXFERS]__attribute__((space(xmemory))),
                     TxBufferB[FLASH_DMAXFERS]__attribute__((space(xmemory))), 
                     RxBufferA[FLASH_DMAXFERS]__attribute__((space(xmemory))),
                     RxBufferB[FLASH_DMAXFERS]__attribute__((space(xmemory))); 

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

char flashStatusCheck(void){
    if(FLASH_DMA==FALSE){
        SS3a=0;
        SPI3BUF=FLASH_RDSR1;               //WEL=1 for write enable
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
    /*
    for(i=0; i<512;i++){    //write 512 bytes
        //send byte
        SPI3BUF=0xAA;
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
    }
    */
    for(i=0; i<256;i++){    //write 512 bytes
        //RxBufferA[i]=//send byte
    }
    SS3a=1;
}

void flashRead(char *array, int bytes){
    int i;
    
    SS3a=0;
    /*
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
    */
    
    TxBufferA[0]=0x03;
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
