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
void initT4(void);
void initT5(void);
void initUART1(void);
void initADC1(void);
//void initSPI1_MEM(void);
void initSPI2_ADC(void);
void initSPI3_MEM(void);
void initDCI_DAC(void);
void initDMA(void);
void initPMP(void);
void initQEI_ENC(void);

#endif	/* DEVINITS_H */
