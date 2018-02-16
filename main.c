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

unsigned char   TxBufferA[FLASH_DMAXFER_BYTES] __attribute__((space(xmemory))),
                RxBufferA[FLASH_DMAXFER_BYTES] __attribute__((space(xmemory)));
fractional      RxBufferB[STREAMBUF] __attribute__((space(xmemory)));

unsigned long readQueue[VOICES];


/* Debug Variables */
unsigned int process_time=0, flash_time = 0;

struct sflags stat = {  .UART_ON = FALSE,
                        .TEST_SIN = FALSE,
                        .DMA_JUSTREAD = FALSE,
                        .DMA_READING = FALSE,
                        .hard_clipped = FALSE,
                        .dma_queue = 0};


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
    
    for(i=0; i<FLASH_DMAXFER_BYTES; i++){
        TxBufferA[i]=0;
        RxBufferA[i]=0;
    }
}

int main(void) {
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
    fractional *ping, *pong;
    
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

            if(stat.DMA_JUSTREAD==TRUE){    
                flashProcessRead();                             // Process DMA requested read data
                stat.DMA_JUSTREAD=FALSE;
                VectorCopy(STREAMBUF, ping, RxBufferB);
                //VectorAdd(STREAMBUF, ping, ping, RxBufferB);
            }
            
            consPADops(ping);
            
            /* State dependent controls*/
            if(state==scrnEDITone || state== debugscrnBUFFERS) consEDITops();
            
            
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