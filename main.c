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
#include "definitions.h"

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

struct clip_flash clipmap[FLASH_NUMCHUNKS];

struct sflags stat = {  .UART_ON = TRUE,
                        .TEST_SIN = FALSE,
                        .hard_clipped = FALSE,
                        .dma_queue = 0,
                        .dma_framesize=0,
                        .dma_rx_index=0,
                        .dma_writeQ_index=-1,
                        .dma_rts=FALSE,
                        .rgb_led=0};

struct bluetooth bluet = {  .AT_MODE=FALSE,
                            .last=0,
                            .dataReady=FALSE,
                            .status=0};

/* Screen state variables */
enum screenStruc state = debugscrnPOTS;
enum screenStruc laststate = invalid;
enum fxStruct fxUnits[NUMFXUNITS]={0,0};
enum colors rgbLED = OFF;

/* Buttons & Potentiometers */
struct ctrlsrfc ctrl = {0};

void initBuffers(void){
    int i;
    for(i=0; i<STREAMBUF; i++){
        streamA[i]=0;
        streamB[i]=0;
        bluet.rxBuf[i]=0;
    }
    
    for(i=0; i<BUTTONS; i++)
        ctrl.pad[i]=0;
    
    for(i=0; i<FLASH_DMAXFER_WORDS; i++){
        TxBufferA[i]=0;
    }
    
    for(i=0; i<FLASH_DMA_RX_WORDS; i++){
        RxBufferA[i]=0;
    }
    
    bluet.writePtr=&bluet.rxBuf[0];
    bluet.btReadPtr=&bluet.rxBuf[0];
                            
}

int main(void) {
    fractional *ping, *pong;
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
            getAudioIntensity(ping);
            
            stat.dma_rx_index=0;
            while(stat.dma_framesize>0){
                //VectorCopy(STREAMBUF, ping, rcvPtr);
                VectorAdd(STREAMBUF, ping, ping, &RxBufferA[stat.dma_rx_index]);
                stat.dma_rx_index+=FLASH_DMAXFER_WORDS;
                stat.dma_framesize--;
            }
            
            /* State dependent controls*/
            if(state!=scrnBT) 
                consPADops(ping);
            
            if(state==scrnEDITone) 
                consEDITONEops();
            else if(state==scrnEDITtwo) 
                consEDITTWOops();
            else if(state==scrnBT){
                if(bluet.AT_MODE) consBTATops();
            }
            
            processAudio(ping, pong);

            if(stat.dma_writeQ_index!=-1){
                while(stat.dma_rts==FALSE){
                    if(_T3IF) {
                        lcdPoll();
                        _T3IF=0;
                    }
                }
                flashWritePage(pong, clipmap[stat.dma_writeQ_index].write_index); 
                stat.dma_writeQ_index=-1; 
            }
            while(!SS3a);
            
            stat.process_time=write_ptr;    //DEBUG
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