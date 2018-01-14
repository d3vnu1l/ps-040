/*
 *
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef PLCD_H
#define	PLCD_H
#include <xc.h> // include processor files - each processor file is guarded.  

void lcdWrite(unsigned int);
void lcdClear(void);
void lcdReturn(void);
void lcdPwr(signed int);
void lcdCursorPwr(void);
void lcdCursorBlink(signed int);


#endif	/* XC_HEADER_TEMPLATE_H */

