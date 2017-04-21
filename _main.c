/*
 * File:   main.c
 * Author: ryan
 *
 * Created on November 19, 2016, 5:37 PM
 */

#include <xc.h>
#include <p33EP512GM310.h>
#include <dsp.h>
#include <libpic30.h>
#include <stdio.h>
#include "common.h"
#include "devInits.c"

#pragma config ICS = PGD1       //pgeDC 1 is used
#pragma config JTAGEN = OFF     //disable jtag
#pragma config BOREN = OFF      //disable brown out 
#pragma config FWDTEN = OFF     //disable watchdog timer 
#pragma config GCP = OFF        //disable general segment code protect
#pragma config PLLKEN = ON      //wait for PLL lock
#pragma config POSCMD = NONE    //no external oscillator
#pragma config OSCIOFNC = ON   //OSC2 is clock output
#pragma config FNOSC = FRCPLL   //clock source

//VARIABLES
char pad[8]={0};

//FUNCTION DECLARATIONS
void __attribute__ ((interrupt)) _T1Interrupt(void);
void __attribute__ ((interrupt)) _T2Interrupt(void);
void __attribute__ ((interrupt)) _U1RXInterrupt(void);
void __attribute__ ((interrupt)) _U1TXInterrupt(void);


//INTERRUPTS
void __attribute__ ((interrupt)) _T1Interrupt(void){
    SLED=~SLED;
    //printf("I AM STILL ALIVE, PLS KILL ME\n");
    IFS0bits.T1IF = 0;              //clear interrupt flag & restart
}

void __attribute__ ((interrupt)) _T2Interrupt(void){
    scanMatrix();
     printf("I AM STILL ALIVE, PLS KILL ME\n");
    IFS0bits.T2IF = 0;              //clear interrupt flag & restart
}

void __attribute__ ((interrupt)) _U1RXInterrupt(void){
    IFS0bits.U1TXIF = 0;            //clear flag, restart
}

void __attribute__ ((interrupt)) _U1TXInterrupt(void){
    IFS0bits.U1RXIF = 0;            //clear flag, restart
}

int main(void) {
    CLKDIVbits.PLLPOST=0;
    CLKDIVbits.PLLPRE=0;
    PLLFBDbits.PLLDIV = 0x3F;       //d_63 for ~120Mhz
    while(OSCCONbits.LOCK!=1) {};   //wait for PLL to lock

    initPorts();                    //configure io & adc
    SLED=1;

    initT1();                       //configure & start T1 
    initT2();                       //configure & start T2 
    initUART1();
    
    while(1){
        //_LATC13=~_LATC13;
        __delay32(900000);
      
    }
    return 0;
}
