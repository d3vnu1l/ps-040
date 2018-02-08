// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef FLASH_H
#define	FLASH_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <dsp.h>

#define FLASH_WREN  0x06    // Write enable
#define FLASH_WRDI  0x04    // Write disable
#define FLASH_RDSR1 0x05    // Read sreg 1
#define FLASH_RDSR2 0x07    // Read sreg 2
#define FLASH_BRAC  0xB9    // Bank register access
#define FLASH_BRRD  0x16    // Bank register read
#define FLASH_BRWR  0x17    // Bank register write
#define FLASH_FRMT  0x60    // Bulk erase, takes ~100 seconds for each die
#define FLASH_SE    0xD8    // Erase 256kB sector       (0.5 MB/s)

#define FLASH_READ  0x03    // Read                     (0.78 MB/s @<=50MHZ)
#define FLASH_4READ 0x13    // Read                     (0.78 MB/s @<=50MHZ)
#define FLASH_PP    0x02    // Page program 512 bytes   (1.5 MB/s)
#define FLASH_4PP   0x12    // Page program 512 bytes   (1.5 MB/s)


void flashWriteReg(char);
void flashWriteBreg(char);
char flashStatusCheck(char);
void flashWritePage(int, int, fractional*);
void flashRead(char *, int);
void flashBulkErase(void);


#endif	/* XC_HEADER_TEMPLATE_H */

