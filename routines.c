#include <xc.h>
#include <p33EP512GM310.h>
#include <dsp.h>
#include "common.h"
#include "routines.h"

extern fractional   outputA[STREAMBUF], outputB[STREAMBUF],
                    streamA[STREAMBUF], streamB[STREAMBUF];
extern unsigned int write_ptr, rw, frameReady;

extern unsigned char    TxBufferA[FLASH_DMAXFER_BYTES]__attribute__((space(xmemory))), 
                        RxBufferA[FLASH_DMAXFER_BYTES]__attribute__((space(xmemory)));

extern struct sflags stat;

extern unsigned char btread;

//Description: This interrupt triggers at the completion of DCI output
//Dependancies: initSPI2(); 
void __attribute__ ((interrupt, auto_psv)) _DCIInterrupt(void){
    static fractional sampoutA=0, sampoutB=0;
    fractional sampinA=0, sampinB=0;
    
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

void __attribute__((interrupt, auto_psv)) _DMA0Interrupt(void) {
    //static unsigned int BufferCount = 0; // Keep record of the buffer that contains TX data
    //if(BufferCount == 0);
    //{
        //TxData(TxBufferA); // Transmit SPI data in DMA RAM Primary buffer
        //TxData(TxBufferB); // Transmit SPI data in DMA RAM Secondary buffer
    //}
    //BufferCount ^= 1;
    IFS0bits.DMA0IF = 0; // Clear the DMA0 Interrupt flag
}

void __attribute__((interrupt, auto_psv)) _DMA1Interrupt(void){
    //static unsigned int BufferCount = 0; // Keep record of the buffer that contains RX data
    //if(BufferCount == 0) ;
        //ProcessRxData(TxBufferA); // Process received SPI data in DMA RAM Primary buffer
    //else;
        //ProcessRxData(TxBufferB); // Process received SPI data in DMA RAM Secondary buffer
 
    //BufferCount ^= 1;
    IFS0bits.DMA1IF = 0; // Clear the DMA1 Interrupt flag
    //BufferCount ^= 1;
    SS3a=SS3b=1;
    stat.FLASH_DMA=FALSE;
    
    if(stat.DMA_READING==TRUE){
        stat.DMA_READING=FALSE;
        stat.DMA_JUSTREAD=TRUE;
    }
    
    DMA1CONbits.CHEN = 0;
    DMA0CONbits.CHEN = 0;
    IFS5bits.SPI3IF = 0;        // Clear the Interrupt flag
}

//Description: This interrupt handles UART reception
//Dependencies: initUART1();
void __attribute__ ((interrupt, auto_psv)) _U1RXInterrupt(void){
    //unsigned char trash;
    btread=U1RXREG;
    IFS0bits.U1RXIF = 0;            //clear flag, restart
}

//Description: This interrupt handles UART transmission
//Dependencies: initUART1();
void __attribute__ ((interrupt, auto_psv)) _U1TXInterrupt(void){
    IFS0bits.U1TXIF = 0;            //clear flag, restart
} 