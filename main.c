#include <xc.h>
#include <p33EP512GM310.h>
#include <dsp.h>
#include "common.h"
#include "devInits.h"
#include "utilities.h"
#include "audio.h"
#include "sounds.h"
#include "flash.h"
#include "plcd.h"
#include "cons.h"

#pragma config ICS = PGD1       //pgeDC 1 is used
#pragma config JTAGEN = OFF     //disable jtag
#pragma config BOREN = OFF      //disable brown out (FYI possible erratta case in 33eMU810)
#pragma config FWDTEN = OFF     //disable watchdog timer 
#pragma config GCP = OFF        //disable general segment code protect
#pragma config PLLKEN = ON      //wait for PLL lock
#pragma config POSCMD = NONE    //no external oscillator
#pragma config OSCIOFNC = OFF   //OSC2 is clock output
#pragma config FNOSC = FRCPLL   //clock source

/* Data Buffers & index variables */
fractional  outputA[STREAMBUF], outputB[STREAMBUF],
            streamA[STREAMBUF], streamB[STREAMBUF];
unsigned int write_ptr=0, rw=0, frameReady=0;

fractional      TxBufferA[FLASH_DMAXFER_WORDS] __attribute__((space(xmemory))),
                RxBufferA[FLASH_DMA_RX_WORDS] __attribute__((space(xmemory)));

unsigned long readQueue[VOICES];


/* Debug Variables */
unsigned int process_time=0, flash_time = 0;

struct sflags stat = {  .UART_ON = FALSE,
                        .TEST_SIN = FALSE,
                        .hard_clipped = FALSE,
                        .dma_queue = 0,
                        .dma_framesize=0,
                        .dma_rx_index=0};


/* Screen state variables */
enum screenStruc state = scrnFX;
enum screenStruc laststate = invalid;
enum fxStruct fxUnits[NUMFXUNITS]={0,0};

/* Buttons & Potentiometers */
struct ctrlsrfc ctrl = {0};

unsigned char btread;

void initBuffers(void){
    int i;
    
    for(i=0; i<STREAMBUF; i++){
        streamA[i]=0;
        streamB[i]=0;
    }
    
    for(i=0; i<BUTTONS; i++)
        ctrl.pad[i]=0;
    
    for(i=0; i<FLASH_DMAXFER_WORDS; i++){
        TxBufferA[i]=0;
    }
    
    for(i=0; i<FLASH_DMA_RX_WORDS; i++){
        RxBufferA[i]=0;
    }
    
}

int main(void) {
    fractional *ping, *pong;
    fractional* rcvPtr;
    
    initPorts();                    // Configure io device & adc 
    initBuffers();
    initDMA();
    initSPI3_MEM();                 // Start flash 
    initDCI_DAC();                  // Configure & enable DAC
    //genSine(STREAMBUF);
    initADC1();                     // Configure & enable internal ADC
    initPMP();
    initQEI_ENC();
    if(stat.UART_ON) initUART1();       // Configure & enable UART
    initT1();                           // Configure & start T1 display
    //initT2();                         // Configure & start T2 btns & pots
    initT3();                           // Configure & start T3 for lcd
    //initT5();
    lcdDrawSplash();
    
    while(1){    
        if(frameReady) {
            scanButtons();                  // Read button matrix
            if(_AD1IF) readPots();          // Check potentiometers
            
            if(rw){
                ping = streamA;
                pong = outputB;
            }else{
                ping = streamB;
                pong = outputA;
            }
            dcHPF(ping, ping);          // Remove DC offset
            
            stat.dma_rx_index=0;
            while(stat.dma_framesize>0){
                //VectorCopy(STREAMBUF, ping, rcvPtr);
                VectorAdd(STREAMBUF, ping, ping, &RxBufferA[stat.dma_rx_index]);
                stat.dma_rx_index+=FLASH_DMAXFER_WORDS;
                stat.dma_framesize--;
            }
            
            consPADops(ping);
            /* State dependent controls*/
            if(state==scrnEDITone) 
                consEDITONEops();
            else if(state== scrnEDITtwo) 
                consEDITTWOops();
            
            processAudio(ping, pong);
            process_time=write_ptr;    //DEBUG
            while(!SS3a);               //wait for flash transmissions to complete
            flash_time=write_ptr;
            frameReady=0;
        }
        if(_T1IF){
            display();
            _T1IF=0;
        }
        if(_T3IF) {
            lcdPoll();
            _T3IF=0;
        }
    }
    return 0;
}