/*
 *
 */

#ifndef ROUTINES_H
#define	ROUTINES_H
#include <xc.h> // include processor files 

//
//function definitions
void __attribute__ ((interrupt, auto_psv)) _T1Interrupt(void);
void __attribute__ ((interrupt, auto_psv)) _T2Interrupt(void);
void __attribute__ ((interrupt, auto_psv)) _T3Interrupt(void);
void __attribute__ ((interrupt, auto_psv)) _U1RXInterrupt(void);
void __attribute__ ((interrupt, auto_psv)) _U1TXInterrupt(void);
void __attribute__ ((interrupt, auto_psv)) _SPI2Interrupt(void);
void __attribute__ ((interrupt, auto_psv)) _SPI3Interrupt(void);
void __attribute__ ((interrupt, auto_psv)) _DCIInterrupt(void);
void __attribute__ ((interrupt, auto_psv)) _IC1Interrupt(void);


#endif	/* ROUTINES_H */

