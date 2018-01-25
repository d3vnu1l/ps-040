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
void lcdCommandQ(unsigned char);
void lcdClearQ(void);
void lcdReturnQ(void);
void lcdPwrQ(signed int);
void lcdCursorPwrQ(void);
void lcdCursorBlinkQ(signed int);
void lcdWriteStringQ(char *);
void lcdCustomSymbols(void);
void lcdSetCursorQ(unsigned char, unsigned char);
void lcdWriteWordQ(int);
void lcdVU(char, char, int);
void lcdDrawPads(unsigned char);
void lcdPoll(void);

void lcdSetupDebug(void);
void lcdSetupPots(void);

/* Blocking Functions */
void lcdSend(unsigned char);
void lcdClear(void);
void lcdReturn(void);



#endif	/* XC_HEADER_TEMPLATE_H */

