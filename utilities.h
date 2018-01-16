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
void processData(int[2][STREAMBUF], int[2][STREAMBUF]);

#endif	/* UTILITIES_H */

