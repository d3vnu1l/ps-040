#include <xc.h>
#include <p33EP512GM310.h>
#include <dsp.h>
#include "common.h"
#include "routines.h"
#include "flash.h"
#include "cons.h"

extern fractional   outputA[STREAMBUF], outputB[STREAMBUF],
                    streamA[STREAMBUF], streamB[STREAMBUF];
extern unsigned int write_ptr, rw, frameReady;

extern fractional       TxBufferA[FLASH_DMAXFER_WORDS]__attribute__((space(xmemory))), 
                        RxBufferA[FLASH_DMA_RX_WORDS]__attribute__((space(xmemory)));

extern unsigned long readQueue[VOICES];
extern struct clip_flash clipmap[FLASH_NUMCHUNKS];
extern struct sflags stat;
extern struct bluetooth bluet;

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
    IFS0bits.DMA0IF = 0; // Clear the DMA0 Interrupt flag
}

void __attribute__((interrupt, auto_psv)) _DMA1Interrupt(void){
    IFS0bits.DMA1IF = 0;        // Clear the DMA1 Interrupt flag
    SS3a=SS3b=1;
    
    DMA1CONbits.CHEN = 0;
    DMA0CONbits.CHEN = 0;
    SPI3STATbits.SPIROV = 0;    // Clear SPI1 receive overflow flag if set
    IFS5bits.SPI3IF = 0;        // Clear the Interrupt flag
    
    // Continue queue'd reads
    if(stat.dma_queue<stat.dma_framesize){
        flashStartRead(readQueue[stat.dma_queue], &RxBufferA[stat.dma_rx_index]);
        stat.dma_rx_index+=FLASH_DMAXFER_WORDS;
        stat.dma_queue++;
    }
    else if(stat.dma_writeQ_index!=-1){ 
        stat.dma_rts=TRUE;
        //flashWritePage(stat.dma_write_buffer, clipmap[stat.dma_writeQ_index].write_index); 
        //stat.dma_writeQ_index=-1; 
    } 
    

}

//Description: This interrupt handles UART reception
//Dependencies: initUART1();
void __attribute__ ((interrupt, auto_psv)) _U1RXInterrupt(void){
    //unsigned char trash;
    bluet.last=U1RXREG;
    
    *bluet.writePtr++=bluet.last;
    if(bluet.writePtr==&bluet.rxBuf[BTBUF_WORDS]){
        bluet.writePtr=&bluet.rxBuf[0];
        bluet.dataReady=TRUE;
        //BLOCKING SEND CHUNK TO flash
        consBTops();
        while(!SS3a);
        Delay_us(10000);
        printf("AA\n"); // Continue
    }
    
    IFS0bits.U1RXIF = 0;            //clear flag, restart
}

//Description: This interrupt handles UART transmission
//Dependencies: initUART1();
void __attribute__ ((interrupt, auto_psv)) _U1TXInterrupt(void){
    IFS0bits.U1TXIF = 0;            //clear flag, restart
} 
