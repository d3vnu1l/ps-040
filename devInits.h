/*
 * 
 */

#ifndef DEVINITS_H
#define	DEVINITS_H
#include <xc.h> // include processor files    

//
//function definitions
void initPorts(void);
void initT1(void);
void initT2(void);
void initT3(void);
void initUART1(void);
void initADC1(void);
void initSPI2_ADC(void);
//void initSPI3_SEG(void);
void initDCI_DAC(void);
void initCAP_BPM(void);
void initPMP(void);
void Delay_us(unsigned int);
void initDMA0(void);

#endif	/* DEVINITS_H */
