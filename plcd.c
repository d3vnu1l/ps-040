/*
 * File:   plcd.c
 * Author: root
 *
 * Created on January 10, 2018, 4:48 PM
 */


#include "xc.h"
unsigned int statusReg = 0x0C;  //internal copy of pwr reg
                                // 0b_0 0 0 0 - 1 D C B (display, cursor, blink)

void lcdWrite(unsigned int data){
    if(!PMMODEbits.BUSY) PMDIN1=data; //(!skips if busy!)
}

void lcdClear(void){
    lcdWrite(0x01);
}

void lcdReturn(void){
    lcdWrite(0x02);
}


void lcdPwr(signed int pwr){
    //lcd soft power on/off, does not reset device
    if(pwr)
        statusReg&=0x04;
    else __builtin_btg(&statusReg, 2);
    
    lcdWrite(statusReg);
}

void lcdCursorEn(signed int pwr){
    //cursor on / off
    if(pwr)
        statusReg&=0x0A;
    else __builtin_btg(&statusReg, 1);
    
    lcdWrite(statusReg);
}

void lcdCursorBlink(signed int pwr){
    if(pwr)
        statusReg&=0x09;
    else __builtin_btg(&statusReg, 0);
    
    lcdWrite(statusReg);
}
