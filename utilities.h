/*
 * 
 */

#ifndef UTILITIES_H
#define	UTILITIES_H
#include <xc.h> // include processor files 
#include "common.h"
#include <dsp.h>

//
//function definitions
void scanButtons(void);
void readPots(void);
void scalePots(void);
fractional scalePotsCustom(unsigned int, fractional);
void changeFX(void);
void checkFunctions(void);
void display(void);
void processRxData(int *, int *);
void ClipCopy_psv(int, fractional *, __psv__ fractional *);
void ClipCopy_eds(int, fractional *, __eds__ fractional *);
void ClipCopy_toeds(int, __eds__ fractional *, fractional *);

void Delay_us(unsigned int);

#endif	/* UTILITIES_H */

