
// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef SCREENS_H
#define	SCREENS_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <dsp.h>

void screenDebugAudio(void);
void screenDebugPots(void);
void screenDebugFlash(void);
void screenDebugBuffers(void);
void screenDebugInput(void);

void screenNoFXmod(unsigned int, fractional, fractional, fractional);
void screenLPFmod(unsigned int, fractional, fractional, fractional);
void screenTRMmod(unsigned int, fractional, fractional, fractional);
void screenLOPmod(unsigned int, fractional, fractional, fractional);
void screenBTCmod(unsigned int, fractional, fractional, fractional);
void screenHPFmod(unsigned int, fractional, fractional, fractional);

void screenFX(void);
void screenSHIFT(void);
void screenEditOne(void);
void screenEditTwo(void);
void screenBluetooth(void);

void screenUpdate(void);

#endif	/* XC_HEADER_TEMPLATE_H */

