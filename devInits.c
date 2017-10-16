/*
 ** This file contains functions to initialize various peripheral devices
 */

#include <xc.h>
#include <p33EP512GM310.h>
#include <libpic30.h>
#include "common.h"
#include "devInits.h"
#include "max7219.h"

extern unsigned char UART_ON;

//Description: Responsible i/o, clock, & RP pin config setup
//Prereq: NONE
//Dependencies: NONE
//*Note: This is ALWAYS the first function called in main*
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
    RPINR24bits.CSDIR=0x2D;     //DCI IN on RPI45
    RPOR3bits.RP40R=0x0C;       //DCI clock
    RPOR2bits.RP39R=0x0D;       //DCI frame sync
    RPOR3bits.RP41R=0x0B;       //DCI output
    RPINR7bits.IC1R=0x5F;       //Capture input on pin 95 re15
    RPINR29bits.SCK3R=0x39;      //SCK3 input on pin 84
    RPOR7bits.RP57R=0x20;        //SCK3 output on pin 84
    RPOR8bits.RP70R=0x1F;       //SDO3 on pin 83
    //RPOR8bits.RP69R=0x21;       //SS3 on pin 82
	__builtin_write_OSCCONL(OSCCON | (1<<6));       // Lock Registers
    //enable disable peripherals:
    PMD1=0x06C6;
    PMD2=0x00FF;
    PMD3=0x0FFF;
    PMD4=0xFFFF;
    PMD6=0xFFFE;
    PMD7=0xFFEF;
    //set analog ports
    ANSELA=0x0003; 
    ANSELB=0x0003;
    ANSELC=0x0000;
    ANSELD=0x0000;
    ANSELE=0x0000;
    ANSELF=0x0000;  //0400
    ANSELG=0x0000;
    //set digital i/o direction
    TRISA=0x0203;
    TRISB=0x2003;
    TRISC=0x0080;
    TRISD=0x0000;
    TRISE=0x8000;
    TRISF=0x0000;
    TRISG=0xFFFF;
    //set digital outputs
    PORTA=0x0000;
    PORTB=0x0000;
    PORTC=0x0000;
    PORTD=0x0040;
    PORTE=0x0000;
    PORTF=0x0000;
    PORTG=0x0000;
    //weak internal pull ups
    CNPUG=0xFFFF;       //weak pull ups on G
    
        ANSELBbits.ANSB0 = 1; // Ensure AN0/RB0 is analog
    ANSELBbits.ANSB1 = 1; // Ensure AN1/RB1 is analog
    ANSELBbits.ANSB2 = 1; // Ensure AN2/RB2 is analog
    ANSELBbits.ANSB3 = 1; // Ensure AN5/RB5 is analog
}

//Description: Initializes UART1 device & interrupts
//Prereq: NONE
//Dependencies: NONE
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

//Description: Initializes onboard ADC 
//Prereq: NONE
//Dependencies: NONE
void initADC1(void){
    AD1CON1bits.ASAM = 1;       //enable simultaneous sample
    AD1CON1bits.SIMSAM = 0;     //enable simultaneous sample
    AD1CON1bits.FORM=2;         //signed fractional format
    AD1CON2bits.CHPS = 0b01;    //sample channels 0-3
    AD1CON3bits.ADCS = 0x3F;    //tad ~ 1us
    //AD1CHS0bits.CH0SA = 0x03;   //ch0->an3 (no use an0)
    AD1CHS0bits.CH0NA = 0; // Select Vref- for CH0 -ve input
    AD1CHS123bits.CH123NA = 0; // Select Vref- for CH1/CH2/CH3 -ve input
    AD1CON1bits.ADON = 1;       //start ADC module
    Delay_us(20);
}

void initSPI3_SEG(void){
    SEG_SEL = 1;
    IFS5bits.SPI3IF = 0;        // Clear the Interrupt flag
    IEC5bits.SPI3IE = 0;        // Disable the interrupt
    SPI3CON1bits.MSTEN=1;       //master mode
    SPI3CON1bits.DISSCK = 0;    //Internal serial clock is enabled
    SPI3CON1bits.MODE16=1;      //16 bit
    SPI3CON1bits.DISSDO=0;      //enable SDO 
    SPI3CON1bits.SSEN=0;        //use SS
    SPI3CON2bits.FRMEN=0;       //no enable framed mode
    SPI3CON2bits.SPIBEN=0;      //enhanced buffer mode
    SPI2STATbits.SISEL=5;       //interrupt when done sending
    SPI3CON1bits.SMP=0;         //data sampled at end of output time
    SPI3CON1bits.CKP=0;         //idle clock is low
    SPI3CON1bits.CKE=1;         //data changes from H to L
    SPI3CON1bits.PPRE=1;        //4:1 primary prescale
    SPI3CON1bits.SPRE=7;        //1:1 secondary
    SPI3STATbits.SPIROV = 0;    // Clear SPI1 receive overflow flag if set
    IPC22bits.SPI3IP = 3;        // Interrupt priority
    IFS5bits.SPI3IF = 0;        // Clear the Interrupt flag
    IEC5bits.SPI3IE = 0;        // Enable the interrupt
    SPI3STATbits.SPIEN = 1;     //start SPI module
    __delay32(40);
    MAX7219_Init();
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
    IFS0bits.T1IF = 0;      //clear timer 1 interrupt flag
    IEC0bits.T1IE = 1;      //enable timer 1 interrupt
    IPC0bits.T1IP = 2;      //interrupt priority 2 (low)
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
    IFS0bits.T2IF = 0;      //clear timer 2 interrupt flag
    IEC0bits.T2IE = 1;      //enable timer 2 interrupt
    IPC1bits.T2IP = 2;      //interrupt priority 2 (low)
    T2CONbits.TON = 1;      //start timer
}

//Description: Initialize timer handling ADC sampling
//Prereq: initSPI_ADC()
//Dependencies: _T3Interrupt(void)
//Frequency: 44.1kHz
//*Note: Currently unused as readDac is chained to DCI interrupt*
void initT3(void){          //16/32 bit timer
    TMR3 = 0x0000;          //clear timer 3
    T3CONbits.TCKPS = 3;    //prescale 256:1
    T3CONbits.TCS = 0;      //use internal clock
    T3CONbits.TGATE = 0;    //gate accumulation disabled
    PR3 = Fcy/(256*T3freq);           //period register
    //PR3 = 0xFFFF;
    IFS0bits.T3IF = 0;      //clear timer 3 interrupt flag
    IEC0bits.T3IE = 1;      //enable timer 3 interrupt
    IPC2bits.T3IP = 2;      //interrupt priority 3 (low)
    T3CONbits.TON = 1;      //start timer
}

//Description: Initializes & starts 16 bit DCI I2S DAC
//Prereq: initSPI_ADC(void)
//Dependencies: readDAC(void)
void initDCI_DAC(void){
    DCICON1bits.CSCKD=0;
    DCICON3bits.BCG=(Fcy/(64*Fout)-1);  //calculate baud rate (WILL TRUNCATE)
    DCICON1bits.COFSM=1;    //i2s mode
    DCICON1bits.CSCKE=1;    //sample on rising edge
    DCICON2bits.WS=0xF;     //16 bit data word
    DCICON2bits.COFSG=0;    //data frame has 1 words
    TSCONbits.TSE0 = 1;     // Transmit on Time Slot 0     
    TSCONbits.TSE1 = 1;     // Transmit on Time Slot 1   
    RSCONbits.RSE0 = 1;     // rcv on Time Slot 0     
    RSCONbits.RSE1 = 1;     // rcv on Time Slot 1 
    DCICON1bits.DJST=0;     //align data
    DCICON2bits.BLEN=1;     //2 words buffer btwn interrupts
    IPC15bits.DCIIP = 6;    // Interrput priority
    IFS3bits.DCIIF=0;
    IEC3bits.DCIIE=1; 
    TXBUF0=0;
    TXBUF1=0;   
    DCICON1bits.DCIEN=1;    //ENABLE
    
    
        static int ij=0;
    if (ij<8)
        ij++;
    else ij=0;
    //seg_display(ij);
}

void initCAP_BPM(void){
    IFS0bits.IC1IF=0;
    IPC0bits.IC1IP=3;
    IC1CON1bits.ICTSEL=0;   //Fp is clock source (7))
    IC1CON1bits.ICM=2;      //capture every falling edge
    IC1CON2bits.ICTRIG=1;
    IC1CON2bits.SYNCSEL=0xD;
    IEC0bits.IC1IE=1;
}

/*
//Description: Initializes 16 bit SPI ADC 
//Prereq: NONE
//Dependencies: NONE
void initSPI2_ADC(void){
    //ADC_CONV=1;                 //prevent shift as per errata
    PORTBbits.RB10=1;           //prevent shift as per errata
    IFS2bits.SPI2IF = 0;        // Clear the Interrupt flag
    IEC2bits.SPI2IE = 0;        // Disable the interrupt
    SPI2CON1bits.MSTEN=1;       //master mode
    SPI2CON1bits.DISSCK = 0;    //Internal serial clock is enabled
    SPI2CON1bits.MODE16=1;      //16 bit
    SPI2CON1bits.DISSDO=1;      //no SDO 
    SPI2CON1bits.SSEN=0;        //no use SS
    SPI2CON2bits.FRMEN=0;       //no enable framed mode
    SPI2CON2bits.SPIBEN=1;      //enhanced buffer mode
    SPI2STATbits.SISEL=5;       //interrupt when done sending
    SPI2CON1bits.SMP=1;         //data sampled at end of output time
    SPI2CON1bits.CKP=1;         //idle clock is high
    SPI2CON1bits.CKE=1;         //data changes from H to L
    SPI2CON1bits.PPRE=2;        //4:1 primary prescale
    SPI2CON1bits.SPRE=0;        //8:1 secondary
    SPI2STATbits.SPIROV = 0;    // Clear SPI1 receive overflow flag if set
    IPC8bits.SPI2IP = 5;        // Interrupt priority
    IFS2bits.SPI2IF = 0;        // Clear the Interrupt flag
    IEC2bits.SPI2IE = 1;        // Enable the interrupt
    SPI2STATbits.SPIEN = 1;     //start SPI module
}
*/

void Delay_us(unsigned int delay)
{
    int i;
    for (i = 0; i < delay; i++)
    {
        __asm__ volatile ("repeat #39");
        __asm__ volatile ("nop");
    }
}