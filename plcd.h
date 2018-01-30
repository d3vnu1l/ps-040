/*
 *
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef PLCD_H
#define	PLCD_H
#include <xc.h> // include processor files - each processor file is guarded.  

#define LCD_CLEARDISPLAY   0x01
#define LCD_RETURNHOME     0x02
#define LCD_ENTRYMODESET   0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT    0x10
#define LCD_FUNCTIONSET    0x20
#define LCD_SETCGRAMADDR   0x40
#define LCD_SETDDRAMADDR   0x80

void lcdInit(void);
void lcdWriteQ(unsigned char);
void lcdClearQ(void);
void lcdCommandQMac(unsigned char);
void lcdBlockingReturn(void);
void lcdWriteStringQ(char *);
void lcdCustomSymbols(void);
void lcdSetCursorQ(unsigned char, unsigned char);
void lcdWriteWordQ(int);
void lcdWriteWordUnsignedQ(unsigned int);
void lcdWriteDecimalQ(char, int);
void lcdDrawPads(unsigned char);

void lcdFrameQ(char);
void lcdPoll(void);

/* Blocking send functions */
void lcdBlockingSend(unsigned char);
void lcdBlockingClear(void);
void lcdBlockingReturn(void);

#define lcdWriteQMac(data) {\
    *lcdWritePtr++=data|0x0000;\
    if(lcdWritePtr==&lcdBuf[LCDBUF])\
        lcdWritePtr=lcdBuf;\
}

#define lcdCommandQMac(data){\
    *lcdWritePtr++=data|0x0100;\
    if(lcdWritePtr==&lcdBuf[LCDBUF])\
        lcdWritePtr=lcdBuf;\
}

#endif	/* XC_HEADER_TEMPLATE_H */

