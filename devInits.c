/*
 ** This file contains functions to initialize various peripheral devices
 */
#include <xc.h>
#include <p33EP512GM310.h>
#include <libpic30.h>
#include "common.h"
#include "devInits.h"
#include "utilities.h"
#include "plcd.h"
#include "screens.h"
#include "flash.h"
#include "dsp.h"

extern unsigned char UART_ON;
extern unsigned int TxBufferA[STREAMBUF], TxBufferB[STREAMBUF], 
            RxBufferA[STREAMBUF], RxBufferB[STREAMBUF];  //doesnt work as fractional

extern char flash_readback[512];

void initPorts(void){
    //CLOCK CONFIG
    CLKDIVbits.PLLPOST=0;
    CLKDIVbits.PLLPRE=0;
    PLLFBDbits.PLLDIV = 0x49;       //d_73 for ~140Mhz
    while(OSCCONbits.LOCK!=1) {};   //wait for PLL to lock
    //RP pin config
	__builtin_write_OSCCONL(OSCCON & ~(1<<6));      // Unlock Registers
    //RPINR18bits.U1RXR = 0x37; //U1 rx on RP55
    //RPOR6bits.RP54R=0x1;          //U1 tx on RP54
    RPOR2bits.RP38R = 0x1;
    RPINR24bits.CSDIR=0x3D;     //DCI IN on RPI61           
    RPOR3bits.RP40R=0x0C;       //DCI clock
    RPOR2bits.RP39R=0x0D;       //DCI frame sync
    RPOR3bits.RP41R=0x0B;       //DCI output
    RPINR7bits.IC1R=0x5F;       //Capture input on pin 95 re15
    RPINR29bits.SCK3R=0x39;     //SCK3 input on pin 84
    RPOR7bits.RP57R=0x20;       //SCK3 output on pin 84
    RPOR8bits.RP70R=0x1F;       //SDO3 on pin 83
    RPOR9bits.RP97R=0x21;       //SS3 on pin 88
    RPINR29bits.SDI3R=0x4C;     //SDI on pin 79, RPI76
    RPINR14bits.QEA1R=0x10;     //QEA on pin 22, RPI16
    RPINR14bits.QEB1R=0x1B;     //QEB on pin 21, RPI27
	__builtin_write_OSCCONL(OSCCON | (1<<6));       // Lock Registers
    /*PERIPHERAL ENABLE (0) - DISABLE (1)*/
    PMD1=PMD2=PMD3=PMD4=PMD6=PMD7=0xFFFF;
    PMD1bits.AD1MD=0;
    PMD1bits.SPI1MD=0;
    PMD1bits.SPI2MD=0;
    PMD1bits.U1MD=0;
    PMD1bits.U2MD=0;
    PMD1bits.DCIMD=0;
    PMD1bits.T1MD=0;
    PMD1bits.T2MD=0;
    PMD1bits.T3MD=0;
    PMD1bits.T4MD=0;
    PMD1bits.T5MD=0;
    PMD3bits.PMPMD=0;
    PMD3bits.CMPMD=0;
    PMD6bits.SPI3MD=0;
    PMD7bits.DMA0MD=0;
    
    /* Digital IO DIRECTION (1 = input) */
    TRISA=TRISB=TRISC=TRISD=TRISE=TRISF=TRISG=0x0000;
    TRISA=0x1E01; CNPUA=0x0801;
    TRISB=0x0000; CNPUB=0x0000;
    TRISC=0x2087;
    TRISD=0x011E; CNPUD=0x001E;
    TRISE=0x7300;
    TRISF=0x00F0; CNPUF=0x00F1;
    TRISG=CNPUG=0xFFFF;   //PORTG all inputs,//weak pull ups on all of G
    
    /* DIGITAL OUTPUT LATCH */
    LATA=LATB=LATC=LATD=LATE=LATF=LATG=0x0000;
    LATA=0x0040;
    
    /* ANALOG PINS (1 = analog) */
    ANSELA=ANSELB=ANSELC=ANSELD=ANSELE=ANSELF=ANSELG=0x0000;
    ANSELCbits.ANSC0=1;     //AN6
    ANSELCbits.ANSC1=1;     //AN7
    ANSELCbits.ANSC2=1;     //AN8
    ANSELAbits.ANSA12=1;    //AN10 
    ANSELEbits.ANSE8=1;     //AN21
    ANSELEbits.ANSE9=1;     //AN20
    
    
    
}

void initUART1(void){
    IFS0bits.U1TXIF = 0;        //clear flag
    IFS0bits.U1RXIF = 0;        //clear flag
    U1STA=0x1510;               //enable tx & rx
    U1BRG=BRGVAL;               //baud rate
    U1MODEbits.PDSEL=1;         //8 bit data, even parity
    IPC2bits.U1RXIP = 7;        //interrupt priority 3 (low)
    IPC3bits.U1TXIP = 7;        //interrupt priority 3 (low)
    //IEC0bits.U1TXIE = 1;        //enable tx interrupt
    //IEC0bits.U1RXIE = 1;        //enable rx interrupt
    U1MODEbits.UARTEN = 1;      //start uart
    UART_ON = TRUE;
    
}

void initADC1(void){ 

    /* Assign MUXA inputs */
    AD1CON1 = 0x04E4; // Enable 12-bit mode, auto-sample and auto-conversion
    AD1CON2 = 0x0408; // Sample alternately using channel scanning
    AD1CON2bits.SMPI=(POTS/2)-1; // Sample 6 channels
    AD1CON3 = 0x0F0F; // Sample for 15*TAD before converting
    AD1CON1bits.FORM=2;         //signed fractional format 
    AD1CON3bits.ADCS=0x07;
    AD1CON3bits.SAMC=0x1F;
    //select  AN6,7,8
    AD1CSSLbits.CSS6=1; //AN6
    AD1CSSLbits.CSS7=1; //AN7
    AD1CSSLbits.CSS8=1; //AN8
    AD1CSSLbits.CSS10=1;//AN10
    AD1CSSHbits.CSS20=1;//AN20
    AD1CSSHbits.CSS21=1;//AN21
    //AD1CSSLbits.CSS9=1; //AN9
    /* Enable ADC module and provide ADC stabilization delay */
    AD1CON1bits.ADON = 1;
    Delay_us(30);
}

void initPMP(void){
    /*
     Data is clocked on falling edge of E
     *  RS = 44 = PMAO (H = display data, L = display instruction)
     *  E = 81 = PMWR (pulse width 450ns MIN, data triggers from H to L) 
     */
    PMMODEbits.MODE=3;  //master mode 1 
    PMCONbits.PTWREN = 1;
    PMCONbits.PTRDEN = 1;
    PMCONbits.WRSP=1;   //write strobe active high
    PMCONbits.RDSP=1;   //read strobe active high

    PMMODEbits.WAITB = 0;
    PMMODEbits.WAITM = 0xC;
    PMMODEbits.WAITE = 0;
    LCD_RS=0;
    PMCONbits.PMPEN = 1;
    
    /* INIT DEVICE */
    Delay_us(40000);
    lcdInit();
    
    /* SETUP SCREEN */
    //lcdSetupPots();
}


//Description: Initializes timer for LED's UART and display
//Prereq: initUART1()
//Dependencies: _T1Interrupt(void)
//Frequency: 15Hz
void initT1(void){          //16 bit timer
    TMR1 = 0x0000;          //clear timer 4
    T1CONbits.TCKPS = 3;    //prescale 1:256
    T1CONbits.TCS = 0;      //use internal clock
    T1CONbits.TGATE = 0;    //gate accumulation disabled
    PR1 = Fcy/(256*Fdisp);    //period register
    //PR1=0x7FFF;

    T1CONbits.TON = 1;      //start timer
}

//Description:  Initializes timer handles polling button input
//Prereq: initADC1() 
//Dependencies: _T2Interrupt(void)
//Frequency: 512Hz
void initT2(void){          //16/32 bit timer
    TMR2 = 0x0000;          //clear timer 4
    T2CONbits.T32 = 0;      //16 bit mode
    T2CONbits.TCKPS = 1;    //prescale 1:8
    T2CONbits.TCS = 0;      //use internal clock
    T2CONbits.TGATE = 0;    //gate accumulation disabled
    PR2 = Fcy/(8*Fscan);      //period register about 512hz, PR2 = 0x3938 

    T2CONbits.TON = 1;      //start timer
}

//Description: Initialize timer handling LCD sending
//Dependencies: _T3Interrupt(void)
//Frequency: 44.1kHz
void initT3(void){          //16/32 bit timer
    TMR3 = 0x0000;          //clear timer 3
    T3CONbits.TCKPS = 1;    //prescale 8:1
    T3CONbits.TCS = 0;      //use internal clock
    T3CONbits.TGATE = 0;    //gate accumulation disabled
    //PR3 = Fcy/(256*T3freq);           //period register
    PR3 = 0x01D0;           //45uS initial delay
    
    T3CONbits.TON = 1;
    
}

/*=============================================================================  
Timer 5 is setup to time-out every 125 microseconds (8Khz Rate). As a result, the module 
will stop sampling and trigger a conversion on every Timer3 time-out, i.e., Ts=125us. 
=============================================================================*/
void initT5() 
{
        TMR5 = 0x0000;
        PR5 = 4999;
        T5CONbits.TCKPS = 2;    //prescale 8:1
        IFS1bits.T5IF = 0;
        IEC1bits.T5IE = 0;

        //Start Timer 3
        T5CONbits.TON = 1;

}

//Description: Initializes & starts 16 bit DCI I2S DAC
//Prereq: initSPI_ADC(void)
//Dependencies: readDAC(void)
void initDCI_DAC(void){
    DCICON3bits.BCG=(Fcy/(64*Fout)-1);  //calculate baud rate (WILL TRUNCATE)
    DCICON1bits.COFSM=1;                //i2s mode
    DCICON1bits.CSCKE=1;                //sample on rising edge
    DCICON2bits.WS=0xF;                 //16 bit data word
    DCICON2bits.COFSG=0;    //data frame has 1 word (per frame)
    DCICON1bits.DJST=0;     //align data
    DCICON2bits.BLEN=3;     //4 words buffer btwn interrupts
    
    TSCONbits.TSE0 = 1;     // Transmit on Time Slot 0     
    TSCONbits.TSE1 = 1;     // Transmit on Time Slot 1   
    TSCONbits.TSE2 = 1;     // Transmit on Time Slot 2     
    TSCONbits.TSE3 = 1;     // Transmit on Time Slot 3   
    RSCONbits.RSE0 = 1;     // rcv on Time Slot 0     
    RSCONbits.RSE1 = 1;     // rcv on Time Slot 1 
    RSCONbits.RSE2 = 1;     // rcv on Time Slot 2     
    RSCONbits.RSE3 = 1;     // rcv on Time Slot 3 
    

    
    IPC15bits.DCIIP = 6;    // Interrput priority
    IFS3bits.DCIIF=0;
    IEC3bits.DCIIE=1;       //=0 to let dma handle interrupt
    
    // Pre-load send registers.
    TXBUF0=0;
    TXBUF1=0;   
    TXBUF2=0;
    TXBUF3=0;
    DCICON1bits.DCIEN=1;    //ENABLE
    // Stabilization delay
    Delay_us(20);
}

void initDMA(void){
    
    IFS0bits.DMA0IF = 0;
    IEC0bits.DMA0IE = 1;
    DMAPWC = 0;
    DMA0CON.SIZE=1;                             // Byte size
    DMA0CON.DIR=1;                              // Write to flash
    DMA0CON.MODE=3;                             // One shot, ping pong
    DMA0STAL = (unsigned int)&TxBufferA;
    DMA0STAH = (unsigned int)&TxBufferB;
    DMA0PAD = (volatile unsigned int) &SPI3BUF;
    DMA0CNT = FLASH_DMAXFERS-1;
    DMA0REQ = 0x005B;
    //Set up DMA Channel 1 to Receive in Continuous Ping-Pong Mode:

    IFS0bits.DMA1IF = 0;
    IEC0bits.DMA1IE = 1;
    DMA0CON.SIZE=1;                             // Byte size
    DMA0CON.DIR=0;                              // Read from flash
    DMA0CON.MODE=3;                             // One shot, ping pong
    DMA1STAL = (unsigned int)&RxBufferA;
    DMA1STAH = (unsigned int)&RxBufferB;
    DMA1PAD = (volatile unsigned int) &SPI3BUF;
    DMA1CNT = FLASH_DMAXFERS-1;
    DMA1REQ = 0x005B;
    
    DMA1CONbits.CHEN = 1;
    DMA0CONbits.CHEN = 1;
}

void initSPI3_MEM(void){
    SS3=1;                     // Assert chip select (active low)
    IFS5bits.SPI3IF = 0;        // Clear the Interrupt flag
    IEC5bits.SPI3IE = 0;        // Disable the interrupt
    SPI3CON1bits.MSTEN=1;       //master mode
    SPI3CON1bits.DISSCK = 0;    //Internal serial clock is enabled
    SPI3CON1bits.MODE16=0;      //8 bitBufferB
    SPI3CON1bits.DISSDO=0;      //enable SDO 
    SPI3CON1bits.SSEN=1;        //use SS
    SPI3CON2bits.FRMEN=0;       //no enable framed mode
    SPI3CON2bits.SPIBEN=0;      //enhanced buffer mode
    SPI2STATbits.SISEL=5;       //interrupt when done sending
    
    SPI3CON1bits.SMP=1;         //data sampled at end of output time
    SPI3CON1bits.CKP=0;         //idle clock is low
    SPI3CON1bits.CKE=1;         //data changes from H to L
    
    SPI3CON1bits.PPRE=3;        //1:1 primary prescale
    SPI3CON1bits.SPRE=6;        //2:1 secondary
    
    SPI3STATbits.SPIROV = 0;        // Clear SPI1 receive overflow flag if set
    //IPC22bits.SPI3IP = 3;         // Interrupt priority
    //IFS5bits.SPI3IF = 0;          // Clear the Interrupt flag
    //IEC5bits.SPI3IE = 0;          // Enable the interrupt
    SPI3STATbits.SPIEN = 1;         //start SPI module
   
    Delay_us(20);                       // Stabilization Delay
    
    flashWriteReg(FLASH_WREN);
    /*
    flashBulkErase();
    while(flashStatusCheck()&1);

    flashWritePage
    while(flashStatusCheck()&1);
    */
    
    flashRead(flash_readback, 256);     // READBACK
    
}

void initQEI_ENC(void){
    QEI1CONbits.INTDIV=6;       // 1:64 prescaler
    QEI1IOCbits.FLTREN=1;       // Enable input filter
    QEI1CONbits.QEIEN=1;        // Enable Quad encoder inteface
}

