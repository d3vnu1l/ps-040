#include <xc.h>
#include <p33EP512GM310.h>
#include <dsp.h>
#include "common.h"
#include "devInits.h"
#include "utilities.h"
#include "audio.h"
#include "plcd.h"
#include "sounds.h"
#include "flash.h"

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
fractional outputA[STREAMBUF], outputB[STREAMBUF];
fractional streamA[STREAMBUF], streamB[STREAMBUF];
unsigned int write_ptr=0, rw=0, frameReady=0;

unsigned char TxBufferA[FLASH_DMAXFERS] __attribute__((space(xmemory)));
unsigned char TxBufferB[FLASH_DMAXFERS] __attribute__((space(xmemory)));
unsigned char RxBufferA[FLASH_DMAXFERS] __attribute__((space(xmemory)));
unsigned char RxBufferB[FLASH_DMAXFERS] __attribute__((space(xmemory)));

/* Debug Variables */
unsigned int process_time=0;

unsigned char hard_clipped=FALSE;                                              
volatile unsigned char recording=TRUE;
unsigned char UART_ON = TRUE;
unsigned char TEST_SIN = FALSE;
unsigned char FLASH_DMA = FALSE;


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
        ctrl.pad[i]=1;
    
    for(i=1; i<FLASH_DMAXFERS; i++){
        TxBufferA[i]=0;
        TxBufferB[i]=0;
        RxBufferA[i]=0;
        RxBufferB[i]=0;
    }
    
    
}

int main(void) {
    initPorts();                    // Configure io device & adc 
    initBuffers();
    initDMA();
    initSPI3_MEM();                 // Start flash 
    flashRead(NULL, 256);     // READBACK
    //flashRead(NULL, 256);     // READBACK
    initDCI_DAC();                  // Configure & enable DAC
    //genSine(STREAMBUF);
    initADC1();                     // Configure & enable internal ADC
    initPMP();
    initQEI_ENC();
    if(UART_ON) initUART1();        // Configure & enable UART
    
    initT1();                       // Configure & start T1 
    initT2();                       // Configure & start T2 
    initT3();                       // Configure & start T3 for lcd
    //initT5();
    fractional temp;
    fractional *ping, *pong;
    
    while(1){    
        if(frameReady) {
            if(rw){
                ping = streamA;
                pong = outputB;
            }else{
                ping = streamB;
                pong = outputA;
            }
            
            processAudio(ping, pong); 
            process_time=write_ptr;    //DEBUG
            //dma?
            frameReady=0;
        }
        if(_T2IF){
            scanButtons();                   //read button matrix
            if(_AD1IF) readPots();
            _T2IF=0;
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