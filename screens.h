
// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef SCREENS_H
#define	SCREENS_H

#include <xc.h> // include processor files - each processor file is guarded.  

void screenDebugAudio(void);
void screenDebugPots(void);
void screenDebugFlash(void);

void screenFX(void);
void screenStart(char);

void screenUpdate(void);

#endif	/* XC_HEADER_TEMPLATE_H */

