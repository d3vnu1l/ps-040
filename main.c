#include <xc.h>
#include <p33EP512GM310.h>
#include <dsp.h>
#include <libpic30.h>
#include "common.h"
#include "devInits.h"
#include "utilities.h"
#include "audio.h"
#include "plcd.h"

#pragma config ICS = PGD1       //pgeDC 1 is used
#pragma config JTAGEN = OFF     //disable jtag
#pragma config BOREN = OFF      //disable brown out (FYI possible erratta case in 33eMU810)
#pragma config FWDTEN = OFF     //disable watchdog timer 
#pragma config GCP = OFF        //disable general segment code protect
#pragma config PLLKEN = ON      //wait for PLL lock
#pragma config POSCMD = NONE    //no external oscillator
#pragma config OSCIOFNC = OFF   //OSC2 is clock output
#pragma config FNOSC = FRCPLL   //clock source

unsigned char pad[BUTTONS];                                                                   //CONTROL VARIABLES//
fractional pots[POTS]={0};
fractional pots_scaled[POTS]={0};
fractional outputA[STREAMBUF], outputB[STREAMBUF];
fractional streamA[STREAMBUF], streamB[STREAMBUF];

/* DMA BUFFERS */
fractional txBufferA[STREAMBUF]__attribute__((space(eds)));
fractional txBufferB[STREAMBUF] __attribute__((space(eds)));
fractional rxBufferA[STREAMBUF] __attribute__((space(eds)));
fractional rxBufferB[STREAMBUF] __attribute__((space(eds)));

unsigned int bpm=0, rw=0, frameReady=0, write_ptr=STREAMBUF;
unsigned int idle=0, cycle=0;

unsigned char hard_clipped=FALSE;                                               //STATUS VARIABLES//
volatile unsigned char recording=TRUE;
unsigned char UART_ON = FALSE;
unsigned char TEST_SIN = FALSE;

volatile unsigned char tremelo=FALSE;                                           //FX FLAGS//
volatile unsigned char looper=FALSE;
volatile unsigned char lpf=FALSE;

volatile unsigned char frame=FALSE;
int temp1, temp2;

char flash_readback[512]={0};

enum screen state = debugscrnPOTS;
enum screen laststate = invalid;

void initBuffer(void){
    int i;
    for(i=0; i<STREAMBUF; i++){
        streamA[i]=0;
        streamB[i]=0;
        txBufferA[i]=0;
        txBufferB[i]=0;
        rxBufferA[i]=0;
        rxBufferB[i]=0;
    }
    
    for(i=0; i<BUTTONS; i++)
        pad[i]=1;
}

int main(void) {
    initPorts();                    //configure io device & adc 
    //initUART1();                    //configure & enable UART
    initBuffer();
    initADC1();                     //configure & enable internal ADC
    initPMP();
    //||||||||----
    //initDMA0();
    initDCI_DAC();                  //configure & enable DAC
    initT1();                       //configure & start T1 
    initT2();                       //configure & start T2 
    initSPI3_MEM();                  //start segment display
    //initCAP_BPM();                  //configure bpm capture
    initT3();                       //configure & start T3 for lcd
    initQEI_ENC();
    //initT5();
    fractional temp;
    int writePtr;
    fractional *ping, *pong;
    
    while(1){    
        if(frameReady) {
            writePtr=STREAMBUF-1;
            if(rw){
                ping = streamA+writePtr;
                pong = outputB+writePtr;
            }else{
                ping = streamB+writePtr;
                pong = outputA+writePtr;
            }
            
            for(; writePtr>=0; writePtr--){
                temp=*ping--; //!rw
                if(temp<=-32766||temp>=32766)hard_clipped=TRUE;
                temp=fx(temp);    //run fx on latest sample
                *pong--=mixer(temp); //rw
                
            }
            temp = 8*idle/STREAMBUF;
            cycle=STREAMBUF/(STREAMBUF-write_ptr);
            idle=0;
            frameReady=0;
        }
        if(_T2IF){
            scanButtons();                   //read button matrix
            readPots();                     //read control pots
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
// example test 