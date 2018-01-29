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
    SS3=0;
    SPI3BUF=command;               //WEL=1 for write enable
    while(!_SPI3IF); _SPI3IF=0;
    receive=SPI3BUF;
    SS3=1;
}

char flashStatusCheck(void){
    SS3=0;
    SPI3BUF=FLASH_RDSR1;               //WEL=1 for write enable
    while(!_SPI3IF); _SPI3IF=0;
    receive=SPI3BUF;
    SPI3BUF=0x00;               //WEL=1 for write enable
    while(!_SPI3IF); _SPI3IF=0;
    receive=SPI3BUF;
    SS3=1;
    
    return receive;
}

void flashWritePage(int addressH, int addressL){
    int i;
    SS3=0;
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
    SS3=1;
}

void flashRead(char *array, int bytes){
    int i;
    
    SS3=0;
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
    
    for(i=0; i<bytes;i++){    //read 512 bytes
        //receive byte
        SPI3BUF=0x00;
        while(!_SPI3IF); _SPI3IF=0;
        Delay_us(1);
        array[i]=SPI3BUF;
    }
    SS3=1;
}

void flashBulkErase(void) {
    flashWriteReg(FLASH_FRMT);
}
