#include <xc.h>
#include <p33EP512GM310.h>
#include <dsp.h>
#include <stdio.h>
#include "common.h"
#include "audio.h"
#include "utilities.h"
#include "routines.h"
//CONTROL VARIABLES//
extern fractional outputA[STREAMBUF], outputB[STREAMBUF];
extern fractional streamA[STREAMBUF], streamB[STREAMBUF];
extern unsigned int write_ptr, rw, frameReady;
extern int txBufferA[STREAMBUF], txBufferB[STREAMBUF],
            RxBufferA[STREAMBUF], RxBufferB[STREAMBUF];  //doesnt work as fractional

//misc.
volatile fractional sampinA=0, sampinB=0;

volatile int rxBufferIndicator = 0;
fractional *ping, *pong;

//Description: This interrupt triggers at the completion of DCI output
//Dependancies: initSPI2(); 
void __attribute__ ((interrupt, auto_psv)) _DCIInterrupt(void){
    static fractional sampoutA=0, sampoutB=0;
    
    int trashA=RXBUF0;
    sampinA=RXBUF1;
    int trashB=RXBUF2;
    sampinB=RXBUF3;
    TXBUF0=TXBUF1=sampoutA;                                    //output buffered sample to DAC
    TXBUF2=TXBUF3=sampoutB;

    __builtin_btg(&sampinA, 15);                             //convert to Q1.15 compatible format
    __builtin_btg(&sampinB, 15);                             //convert to Q1.15 compatible format
    
    if(write_ptr== STREAMBUF){                       //reset pointer when out of bounds
        write_ptr=0;
        __builtin_btg(&rw,0);
        frameReady=1;
    }
   
    if(rw){
        streamB[write_ptr]=sampinA;
        sampoutA=outputA[write_ptr++]; 
        streamB[write_ptr]=sampinB;
        sampoutB=outputA[write_ptr++];  
    } else {
        streamA[write_ptr]=sampinA; 
        sampoutA=outputB[write_ptr++];  
        streamA[write_ptr]=sampinB; 
        sampoutB=outputB[write_ptr++];  
    } 
    
    _DCIIF=0;
}

void __attribute__((__interrupt__)) _SPI3Interrupt(void)
{
    IFS5bits.SPI3IF = 0;
}

void __attribute__((__interrupt__)) _DMA0Interrupt(void) {
    static unsigned int BufferCount = 0; // Keep record of the buffer that contains TX data
    if(BufferCount == 0);
    {
        //TxData(TxBufferA); // Transmit SPI data in DMA RAM Primary buffer
        //TxData(TxBufferB); // Transmit SPI data in DMA RAM Secondary buffer
    }
    BufferCount ^= 1;
    IFS0bits.DMA0IF = 0; // Clear the DMA0 Interrupt flag
}

void __attribute__((__interrupt__)) _DMA1Interrupt(void){
    static unsigned int BufferCount = 0; // Keep record of the buffer that contains RX data
    if(BufferCount == 0) ;
        //ProcessRxData(TxBufferA); // Process received SPI data in DMA RAM Primary buffer
    else;
        //ProcessRxData(TxBufferB); // Process received SPI data in DMA RAM Secondary buffer
 
    BufferCount ^= 1;
    IFS0bits.DMA1IF = 0; // Clear the DMA1 Interrupt flag
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