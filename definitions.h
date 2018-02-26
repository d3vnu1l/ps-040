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

enum colors{
    OFF,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
    BLINK_RED,
    BLINK_GREEN,
    BLINK_YELLOW,
    BLINK_BLUE,
    BLINK_MAGENTA,
    BLINK_CYAN,
    BLINK_WHITE,
    
};
#endif	/* XC_HEADER_TEMPLATE_H */

