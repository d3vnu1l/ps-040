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
void lcdWrite(unsigned char);
void lcdCommand(unsigned char);
void lcdClear(void);
void lcdReturn(void);
void lcdPwr(signed int);
void lcdCursorPwr(void);
void lcdCursorBlink(signed int);
void lcdSetupPots(void);
void lcdWriteString(char *);
void lcdCustomSymbols(void);
void lcdSetCursor(unsigned char, unsigned char);


#endif	/* XC_HEADER_TEMPLATE_H */

