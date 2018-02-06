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

char flashBuf[STREAMBUF*2];
char receive;

void flashWriteReg(char command) {
    SS3a=0;
    SPI3BUF=command;               //WEL=1 for write enable
    while(!_SPI3IF); _SPI3IF=0;
    receive=SPI3BUF;
    SS3a=1;
}

char flashStatusCheck(void){
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

void flashWritePage(int addressH, int addressL){
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
    
    for(i=0; i<512;i++){    //write 512 bytes
        //send byte
        SPI3BUF=0xAA;
        while(!_SPI3IF); _SPI3IF=0;
        receive=SPI3BUF;
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
    
    
    DMA1CONbits.CHEN = 1;
    DMA0CONbits.CHEN = 1;
    /* Kick off read here */
    
    DMA0REQbits.FORCE = 1; // Manual mode: Kick-start the 1st transfer
    //SPI3BUF=0x00;
    /*
    for(i=0; i<bytes;i++){    //read 512 bytes
        //receive byte
        SPI3BUF=0x00;
        while(!_SPI3IF); _SPI3IF=0;
        Delay_us(1);
        array[i]=SPI3BUF;
    }
    
    */ 
    Delay_us(200);
    SS3a=1;
    DMA1CONbits.CHEN = 0;
    DMA0CONbits.CHEN = 0;
}

void flashBulkErase(void) {
    flashWriteReg(FLASH_FRMT);
}
