#include <xc.h>
#include <p33EP512GM310.h>
#include <dsp.h>
#include <stdio.h>
#include "common.h"
#include "audio.h"
#include "utilities.h"
//CONTROL VARIABLES//
extern fractional outputA[STREAMBUF], outputB[STREAMBUF];
extern fractional streamA[STREAMBUF], streamB[STREAMBUF];
extern unsigned int write_ptr, rw, frameReady;
extern int txBufferA[STREAMBUF], txBufferB[STREAMBUF], rxBufferA[STREAMBUF], rxBufferB[STREAMBUF];  //doesnt work as fractional

//misc.
volatile fractional sampin=0;
volatile fractional sampout=0;
volatile int rxBufferIndicator = 0;
fractional *ping, *pong;

//Description: This interrupt triggers at the completion of DCI output
//Dependancies: initSPI2(); 
void __attribute__ ((interrupt, auto_psv)) _DCIInterrupt(void){
    TXBUF0=TXBUF1=sampout;                                    //output buffered sample to DAC
    sampin=RXBUF1;
    __builtin_btg(&sampin, 15);                             //convert to Q1.15 compatible format
    int trash=RXBUF0;
      
    
    if(write_ptr--==0){                       //reset pointer when out of bounds
        write_ptr=STREAMBUF-1;
        __builtin_btg(&rw,0);
        frameReady=1;
    }
   
    if(rw){
        streamB[write_ptr]=sampin;
        sampout=outputA[write_ptr]; 
    }
    else {
        streamA[write_ptr]=sampin; 
        sampout=outputB[write_ptr];  
    } 
    
    _DCIIF=0;
}

void __attribute__((__interrupt__,no_auto_psv)) _DMA2Interrupt(void){
    _DMA2IF = 0; /* Received one frame of data*/    
    
    if(rxBufferIndicator == 0)
    {
         processRxData((int *)rxBufferA, (int*)txBufferA);
    }
    else
    {
         processRxData((int *)rxBufferB, (int*)txBufferB);
    }
    rxBufferIndicator ^= 1; /* Toggle the indicator*/    
}

/*
//Description: This interrupt handles UART reception
//Dependencies: initUART1();
void __attribute__ ((interrupt, auto_psv)) _U1RXInterrupt(void){
    unsigned char trash;
    trash=U1RXREG;
    printf("RECIEVED: %d\r\n", trash);
    IFS0bits.U1RXIF = 0;            //clear flag, restart
}

//Description: This interrupt handles UART transmission
//Dependencies: initUART1();
void __attribute__ ((interrupt, auto_psv)) _U1TXInterrupt(void){
    YLED=~YLED;
    IFS0bits.U1TXIF = 0;            //clear flag, restart
} 
  
void __attribute__ ((interrupt, auto_psv)) _SPI3Interrupt(void){
    //SEG_SEL=1;
    int trash=SPI3BUF;
    SPI3STATbits.SPIROV = 0;                                //Clear SPI1 receive overflow flag if set
    IFS5bits.SPI3IF=0;
    
}
 */