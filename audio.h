#ifndef XC_HEADER_TEMPLATE_H
#define	XC_HEADER_TEMPLATE_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <dsp.h>
 
void runBufferLooper(fractional *);
void runLPF(fractional *, fractional *, fractional, fractional, fractional);
void runTRM(fractional *, fractional *, fractional, fractional, fractional);
void runLOP(fractional *, fractional *, fractional, fractional, fractional);
void runBIT(fractional *, fractional *, fractional, fractional, fractional);
void processAudio(fractional *, fractional *);

#endif	/* XC_HEADER_TEMPLATE_H */

//test