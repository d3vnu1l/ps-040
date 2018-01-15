#include <xc.h>
#include <p33EP512GM310.h>
#include <dsp.h>
#include <stdio.h>
#include "common.h"
#include "audio.h"
//#include "utilities.h"
//CONTROL VARIABLES//
extern char pad[8];
extern fractional pots[4]; 
extern fractional stream[2][STREAMBUF];
extern fractional output;
extern unsigned int write_ptr, rw;

//STATUS VARIABLES
extern unsigned char hard_clipped;
extern unsigned char recording;
extern const int sintab[1024];
extern unsigned char t1flag, t2flag;

//FX FLAGS
extern unsigned char tremelo, looper;

//misc.
volatile fractional sample=0;

//Description: This interrupt toggles status led, runs UART1 and handles display
//Dependencies: initUART1();
//Frequency: 60
void __attribute__ ((interrupt, auto_psv)) _T1Interrupt(void){
    t1flag=TRUE;
    IFS0bits.T1IF = 0;              //clear interrupt flag & restart
}

//Description: This interrupt handles polling button input
//Dependencies: initADC1(); 
//Frequency: 512Hz
void __attribute__ ((interrupt, auto_psv)) _T2Interrupt(void){
    t2flag=TRUE;
    IFS0bits.T2IF = 0;              //clear interrupt flag & restart
}

//Description: This interrupt triggers at the completion of DCI output
//Dependancies: initSPI2(); 
void __attribute__ ((interrupt, auto_psv)) _DCIInterrupt(void){
    TXBUF0=output;                                    //output buffered sample to DAC
    TXBUF1=output;
    sample=RXBUF1;
    int trash=RXBUF0;
    __builtin_btg(&sample, 15);                             //convert to Q1.15 compatible format
    if(sample<=-32766||sample>=32766)
        hard_clipped=TRUE;
    if(write_ptr==(STREAMBUF-1)){                       //reset pointer when out of bounds
        write_ptr=0;
        __builtin_btg(&rw,0);
    }
    else write_ptr++;
    if(recording==TRUE){
        stream[rw][write_ptr]=sample;                 //get output
    }
    stream[rw][write_ptr]=fx(stream[rw][write_ptr]);    //run fx on latest sample
    if(rw==0)
        output=mixer(stream[1][write_ptr]);             //mix  new output
    else output = mixer(stream[0][write_ptr]);
    IFS3bits.DCIIF=0;
}

void __attribute__ ((interrupt, auto_psv)) _IC1Interrupt(void){
    IFS0bits.IC1IF=0;   
    //bpm=IC1BUF;
    RLED=~RLED;
}

void __attribute__ ((interrupt, auto_psv)) _SPI3Interrupt(void){
    //SEG_SEL=1;
    int trash=SPI3BUF;
    SPI3STATbits.SPIROV = 0;                                //Clear SPI1 receive overflow flag if set
    IFS5bits.SPI3IF=0;   
}

void __attribute__((interrupt, no_auto_psv)) _DMA0Interrupt(void)
{
    IFS0bits.DMA0IF = 0;			// Clear the DMA0 Interrupt Flag
}

/*
//Description: This interrupt handles polling button input
//Dependencies: initADC1(); 
//Frequency: 44.1kHz
void __attribute__ ((interrupt, auto_psv)) _T3Interrupt(void){
    SEG_SEL=0;
    SPI3BUF=0x0100;
    //SEG_SEL=1;
    IFS0bits.T3IF = 0;              //clear flag, restart
}

void __attribute__ ((interrupt, auto_psv)) _SPI2Interrupt(void){
    //GLED=~GLED;                                             //check half freq w/ RLED                                        //returns SS2 to idle state
    SPI2STATbits.SPIROV = 0;                                //Clear SPI1 receive overflow flag if set
    IFS2bits.SPI2IF = 0;                                    //reset flag & restart
}
//Description: This interrupt handles UART reception
//Dependencies: initUART1();
void __attribute__ ((interrupt, auto_psv)) _U1RXInterrupt(void){
    unsigned char trash;
    GLED=~GLED;
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
 */