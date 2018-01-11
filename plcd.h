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
void lcdPower(signed int);
void lcdCursor(void);


#endif	/* XC_HEADER_TEMPLATE_H */

