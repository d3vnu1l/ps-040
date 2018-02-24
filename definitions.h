#ifndef DEFINITIONS_H
#define	DEFINITIONS_H_H

#include <xc.h>

enum screenStruc{
    scrnFX,
    scrnEDITone,
    scrnEDITtwo,
    scrnBT,
    debugscrnPOTS,
    debugscrnFLASH,
    debugscrnBUFFERS,
    debugscrnINPUT,
    debugscrnAUDIO,
    scrnSHIFT,
    start,
    invalid
};

enum fxStruct{
    off,
    lpf,
    trm,
    lop,
    btc,
    hpf
};

#endif	/* XC_HEADER_TEMPLATE_H */

