/*
 * 
 */

#ifndef UTILITIES_H
#define	UTILITIES_H
#include <xc.h> // include processor files 
#include "common.h"

//
//function definitions
void scanMatrix(void);
void readPots(void);
void display(void);
void processRxData(int *, int *);
void processData(int *, int *);

#endif	/* UTILITIES_H */

