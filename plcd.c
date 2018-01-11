/*
 * File:   plcd.c
 * Author: root
 *
 * Created on January 10, 2018, 4:48 PM
 */


#include "xc.h"

void lcdWrite(unsigned int){
    if(!PMMODEbits.BUSY) PMDIN1=int; //(!skips if busy!)
}

void lcdClear(void){
    
}

void lcdPower(signed int pwr){
    
}

void lcdCursor(void){
    
}
