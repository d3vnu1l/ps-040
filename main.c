#include <xc.h>
#include <p33EP512GM310.h>
#include <dsp.h>
#include <libpic30.h>
#include "common.h"
#include "devInits.h"
#include "utilities.h"
//#include "fatfs/src/ff.h"

#pragma config ICS = PGD1       //pgeDC 1 is used
#pragma config JTAGEN = OFF     //disable jtag
#pragma config BOREN = OFF      //disable brown out (FYI possible erratta case in 33eMU810)
#pragma config FWDTEN = OFF     //disable watchdog timer 
#pragma config GCP = OFF        //disable general segment code protect
#pragma config PLLKEN = ON      //wait for PLL lock
#pragma config POSCMD = NONE    //no external oscillator
#pragma config OSCIOFNC = ON   //OSC2 is clock output
#pragma config FNOSC = FRCPLL   //clock source

unsigned char pad[8]={0};                                                                //CONTROL VARIABLES//
fractional pots[4]={0};
fractional pots_scaled[4]={0};
fractional output=0;
fractional stream[2][STREAMBUF];
unsigned int bpm=0, write_ptr=0, rw=0;

unsigned char hard_clipped=FALSE;                                               //STATUS VARIABLES//
volatile unsigned char t1flag=FALSE;
volatile unsigned char t2flag=FALSE;
volatile unsigned char recording=TRUE;
unsigned char UART_ON = FALSE;
unsigned char TEST_SIN = FALSE;

volatile unsigned char tremelo=FALSE;                                           //FX FLAGS//
volatile unsigned char looper=FALSE;
volatile unsigned char lpf=FALSE;

void initBuffer(void){
    int i=0;
    for(; i<STREAMBUF; i++){
        stream[0][i]=0;
        stream[1][i]=0;
    }
}

int main(void) {
    initPorts();                    //configure io device & adc 
    //initUART1();                    //configure & enable UART
    initBuffer();
    initADC1();                     //configure & enable internal ADC
    //initSPI2_ADC();                  //configure & enable SPI ADC !!!DEFUNCT!!!
    initPMP();
    //||||||||----
    initDCI_DAC();                  //configure & enable DAC
    initT1();                       //configure & start T1 
    initT2();                       //configure & start T2 
    //initSPI3_SEG();                  //start segment display

    //initCAP_BPM();                  //configure bpm capture
    //initT3();                       //configure & start T3
    while(1){   
        if(t2flag==TRUE){
            scanMatrix();                   //read button matrix
            readPots();                     //read control pots
            t2flag=FALSE;
        }
        if(t1flag==TRUE){
            display();
            t1flag=FALSE; 
            
        }
    }
    return 0;
}
// example test 