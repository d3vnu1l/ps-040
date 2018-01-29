/*
 * 
 */

#ifndef UTILITIES_H
#define	UTILITIES_H
#include <xc.h> // include processor files 
#include "common.h"

//
//function definitions
void scanButtons(void);
void readPots(void);
void display(void);
void processRxData(int *, int *);
void Delay_us(unsigned int);

#endif	/* UTILITIES_H */

