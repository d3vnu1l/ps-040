/*
 * File:   plcd.c
 * Author: root
 *
 * Created on January 10, 2018, 4:48 PM
 */


#include "xc.h"
unsigned int statusReg = 0x0C;  //internal copy of pwr reg
                                // 0b_0 0 0 0_1 D C B (display, cursor, blink)

void lcdWrite(unsigned int){
    if(!PMMODEbits.BUSY) PMDIN1=int; //(!skips if busy!)
}

void lcdClear(void){
    lcdWrite(0x01);
}

void lcdPower(signed int pwr){
    //lcd soft power on/off, does not reset device
    if(pwr)
        statusReg&=0x0C;
    else statusReg&=0x08;
    
    lcdWrite(statusReg);
}

void lcdCursor(signed int pwr){
    //cursor on / off
    if(pwr)
        statusReg&=0x0A;
    else statusReg&=0x0C;
    
    lcdWrite(statusReg);
}
