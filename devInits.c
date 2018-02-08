/*
 ** This file contains functions to initialize various peripheral devices
 */
#include <xc.h>
#include <p33EP512GM310.h>
#include "common.h"
#include "devInits.h"
#include "utilities.h"
#include "plcd.h"
#include "screens.h"
#include "flash.h"
#include "dsp.h"

extern unsigned char    TxBufferA[FLASH_DMAXFER_BYTES]__attribute__((space(xmemory))), 
                        RxBufferA[FLASH_DMAXFER_BYTES]__attribute__((space(xmemory)));
extern fractional       RxBufferB[STREAMBUF] __attribute__((space(xmemory)));

void initPorts(void){
    /* Clock Setup */
    CLKDIVbits.PLLPOST=0;
    CLKDIVbits.PLLPRE=0;
    PLLFBDbits.PLLDIV = 0x49;       //d_73 for ~140Mhz
    while(OSCCONbits.LOCK!=1) {};   //wait for PLL to lock
    
    /* Remappable Pins*/
	__builtin_write_OSCCONL(OSCCON & ~(1<<6));      // Unlock Registers
        //RPINR18bits.U1RXR = 0x37; //U1 rx on RP55
        //RPOR6bits.RP54R=0x1;          // U1 tx on RP54 (in use)
        RPINR18bits.U1RXR = 0x48;       // U1 rx on RP72
        RPOR1bits.RP37R = 0x01;         // Pin 70, RP37 U1 tx
        RPINR24bits.CSDIR=0x3D;         //DCI IN on RPI61           
        RPOR3bits.RP40R=0x0C;           //DCI clock
        RPOR2bits.RP39R=0x0D;           //DCI frame sync
        RPOR3bits.RP41R=0x0B;           //DCI output
        RPINR7bits.IC1R=0x5F;           //Capture input on pin 95 re15
        RPINR29bits.SCK3R=0x39;         //SCK3 input on pin 84
        RPOR7bits.RP57R=0x20;           //SCK3 output on pin 84
        RPOR8bits.RP70R=0x1F;           //SDO3 on pin 83
        RPOR9bits.RP97R=0x21;           //SS3 on pin 88
        RPINR29bits.SDI3R=0x4C;         //SDI on pin 79, RPI76
        RPINR14bits.QEA1R=0x10;         //QEA on pin 22, RPI16
        RPINR14bits.QEB1R=0x1B;         //QEB on pin 21, RPI27
	__builtin_write_OSCCONL(OSCCON | (1<<6));       // Lock Registers
    
    /* PERIPHERAL ENABLE (0) - DISABLE (1) */
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
    TRISG=CNPUG=0xFFFF;   //PORTG all inputs, weak pull ups on all of G
    
    /* DIGITAL OUTPUT LATCH */
    LATA=LATB=LATC=LATD=LATE=LATF=LATG=0x0000;
    LATA=0x0040;
    SS3a=SS3b=FLASHCLK=1;            // Keep SPI CS & CLK asserted
    
    /* ANALOG PINS (1 = analog) */
    ANSELA=ANSELB=ANSELC=ANSELD=ANSELE=ANSELF=ANSELG=0x0000;
    ANSELCbits.ANSC0=1;     //AN6
    ANSELCbits.ANSC1=1;     //AN7
    ANSELCbits.ANSC2=1;     //AN8
    ANSELAbits.ANSA12=1;    //AN10 
    ANSELEbits.ANSE8=1;     //AN21
    ANSELEbits.ANSE9=1;     //AN20
    
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
    RPOR7bits.RP57R=0x20;       //SCK3 output on pin 84
    RPINR29bits.SCK3R=0x39;     //SCK3 input on pin 84
    RPOR8bits.RP70R=0x1F;       //SDO3 on pin 83
    RPOR9bits.RP97R=0x21;       //SS3 on pin 88
    RPINR29bits.SDI3R=0x4C;     //SDI on pin 79, RPI76
    RPINR14bits.QEA1R=0x10;     //QEA on pin 22, RPI16
    RPINR14bits.QEB1R=0x1B;     //QEB on pin 21, RPI27
	__builtin_write_OSCCONL(OSCCON | (1<<6));       // Lock Registers    
}

void initUART1(void){
    IFS0bits.U1TXIF = 0;        //clear flag
    IFS0bits.U1RXIF = 0;        //clear flag
    U1STA=0x1510;               //enable tx & rx
    
    U1BRG=BRGVAL;               //baud rate
    //U1MODEbits.PDSEL=0;         //8 bit data, even parity
    IPC2bits.U1RXIP = 4;        //interrupt priority 3 (low)
    IPC3bits.U1TXIP = 4;        //interrupt priority 3 (low)
    IEC0bits.U1TXIE = 1;        //enable tx interrupt
    IEC0bits.U1RXIE = 1;        //enable rx interrupt
    U1MODEbits.UARTEN = 1;      //start uart
    U1STAbits.UTXEN = 1;
}

void initADC1(void){ 

    /* Assign MUXA inputs */
    AD1CON1 = 0x04E4;               // Enable 12-bit mode, auto-sample and auto-conversion
    AD1CON2 = 0x0408;               // Sample alternately using channel scanning
    AD1CON2bits.SMPI=(POTS/2)-1;    // Sample 6 channels
    AD1CON3 = 0x0F0F;               // Sample for n*TAD before converting
    AD1CON1bits.FORM=2;             // Signed fractional format 
    AD1CON3bits.ADCS=0x07;
    AD1CON3bits.SAMC=0x1F;
    AD1CSSLbits.CSS6=1;             //AN6 in channel scan
    AD1CSSLbits.CSS7=1;             //AN7 in channel scan
    AD1CSSLbits.CSS8=1;             //AN8 in channel scan
    AD1CSSLbits.CSS10=1;            //AN10 in channel scan
    AD1CSSHbits.CSS20=1;            //AN20 in channel scan
    AD1CSSHbits.CSS21=1;            //AN21 in channel scan
    
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
    PMMODEbits.MODE=3;              // Master mode 1 
    PMCONbits.PTWREN = 1;
    PMCONbits.PTRDEN = 1;
    PMCONbits.WRSP=1;               // Write strobe active high
    PMCONbits.RDSP=1;               // Read strobe active high

    PMMODEbits.WAITB = 0;
    PMMODEbits.WAITM = 0xC;
    PMMODEbits.WAITE = 0;
    LCD_RS=0;
    PMCONbits.PMPEN = 1;
    
    /* INIT DEVICE */
    Delay_us(40000);
    lcdInit();
}


//Description: Initializes timer for LED's UART and display
//Prereq: initUART1()
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
//Frequency: variable depending on lcd latency values in datasheet
void initT3(void){          //16/32 bit timer
    TMR3 = 0x0000;          //clear timer 3
    T3CONbits.TCKPS = 1;    //prescale 8:1
    T3CONbits.TCS = 0;      //use internal clock
    T3CONbits.TGATE = 0;    //gate accumulation disabled
    //PR3 = Fcy/(256*T3freq);           //period register
    PR3 = 0x01D0;           //45uS initial delay
    
    T3CONbits.TON = 1;
    
}

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
void initDCI_DAC(void){
    DCICON3bits.BCG=(Fcy/(64*Fout)-1);  // Calculate baud rate (WILL TRUNCATE)
    DCICON1bits.COFSM=1;                // I2S mode
    DCICON1bits.CSCKE=1;                // Sample on rising edge
    DCICON2bits.WS=0xF;                 // 16 bit data word
    DCICON2bits.COFSG=0;                // Data frame has 1 word (per frame)
    DCICON1bits.DJST=0;                 // Align data
    DCICON2bits.BLEN=3;                 // 4 words buffer btwn interrupts
    
    TSCONbits.TSE0 = 1;                 // Transmit on Time Slot 0     
    TSCONbits.TSE1 = 1;                 // Transmit on Time Slot 1   
    TSCONbits.TSE2 = 1;                 // Transmit on Time Slot 2     
    TSCONbits.TSE3 = 1;                 // Transmit on Time Slot 3   
    RSCONbits.RSE0 = 1;                 // Rcv on Time Slot 0     
    RSCONbits.RSE1 = 1;                 // Rcv on Time Slot 1 
    RSCONbits.RSE2 = 1;                 // Rcv on Time Slot 2     
    RSCONbits.RSE3 = 1;                 // Rcv on Time Slot 3 
    
    IPC15bits.DCIIP = 5;                // Interrput priority
    IFS3bits.DCIIF=0;
    IEC3bits.DCIIE=1;                   // =0 if letting dma handle interrupt
    
    TXBUF0=0;                           // Pre-load send registers.
    TXBUF1=0;   
    TXBUF2=0;
    TXBUF3=0;
    DCICON1bits.DCIEN=1;                // ENABLE
    Delay_us(20);                       // Stabilization delay
}

void initDMA(void){
    /* RX */
    IFS0bits.DMA1IF = 0;
    DMA1CONbits.SIZE=1;                             // Byte size
    DMA1CONbits.DIR=0;                              // Read from flash
    DMA1CONbits.MODE=1;                             // One shot, ping pong
    //DMA1CONbits.NULLW=1;                          // NULL WRITE (debug))
    DMA1STAL = (unsigned int)(&RxBufferA);
    //DMA1STAH = (unsigned int)(&RxBufferB);
    DMA1PAD = (volatile unsigned int) &SPI3BUF;
    DMA1CNT = (unsigned int)(FLASH_DMAXFER_BYTES-1);
    DMA1REQbits.IRQSEL = 0x5B;
    
    IFS0bits.DMA1IF = 0;
    IEC0bits.DMA1IE = 1;
    IPC3bits.DMA1IP = 6;
    DMA1CONbits.CHEN = 0;
    
    
    /* TX */
    IFS0bits.DMA0IF = 0;
    DMAPWC = 0;
    DMA0CONbits.SIZE=1;                             // Byte size
    DMA0CONbits.DIR=1;                              // Write to flash
    DMA0CONbits.MODE=1;                             // One shot, ping pong
    DMA0STAL = (unsigned int)(&TxBufferA);
    //DMA0STAH = (unsigned int)(&TxBufferB);
    DMA0PAD = (volatile unsigned int) &SPI3BUF;
    DMA0CNT = FLASH_DMAXFER_BYTES-1;
    DMA0REQbits.IRQSEL = 0x5B;
    
    IFS0bits.DMA0IF = 0;
    IEC0bits.DMA0IE = 1;
    IPC1bits.DMA0IP = 6;
    DMA0CONbits.CHEN = 0;
}

void initSPI3_MEM(void){
    SS3a=SS3b=FLASHCLK=1;
    
    IFS5bits.SPI3IF = 0;        // Clear the Interrupt flag
    IEC5bits.SPI3IE = 0;        // Disable the interrupt
    SPI3CON1bits.MSTEN=1;       // Master mode
    SPI3CON1bits.DISSCK = 0;    // Internal serial clock is enabled
    SPI3CON1bits.MODE16=0;      // 8 bitBufferB
    SPI3CON1bits.DISSDO=0;      // Enable SDO 
    SPI3CON2bits.FRMEN=0;       // No enable framed mode
    SPI3CON2bits.SPIBEN=0;      // Enhanced buffer mode
    
    SPI3CON1bits.SMP=1;         // Data sampled at end of output time
    SPI3CON1bits.CKP=1;         // Idle clock is low
    SPI3CON1bits.CKE=0;         // Data changes from H to L
    
    SPI3CON1bits.PPRE=3;        // 1:1 primary prescale (3)
    SPI3CON1bits.SPRE=6;        // 2:1 secondary (6)
    
    SPI3STATbits.SPIROV = 0;    // Clear SPI1 receive overflow flag if set
    SPI3STATbits.SPIEN = 1;     // Start SPI module
    //IEC5bits.SPI3IE = 0;      // Enable the Interrupt

    //flashWriteReg(FLASH_WREN);
    //flashWriteBreg(0x80);
    Delay_us(10000);
    //flashWritePage(NULL,NULL,NULL);
    //flashWriteReg(FLASH_WREN);
    Delay_us(10000);
    //flashBulkErase();
    
    
}

/* Quadradure Encoder */
void initQEI_ENC(void){
    QEI1CONbits.INTDIV=6;       // 1:64 prescaler
    QEI1IOCbits.FLTREN=1;       // Enable input filter
    QEI1CONbits.QEIEN=1;        // Enable Quad encoder inteface
}

