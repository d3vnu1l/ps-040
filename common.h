#ifndef COMMON_H
#define	COMMON_H
#include <xc.h> 

#define Fin     73728000
#define Fosc    138240000
#define Fcy     Fosc/2

/* Timers and Frequencies */
#define BAUDRATE 57600                  //target baud rate for *UART*           //peripheral configs
#define BRGVAL  ((Fcy/BAUDRATE)/16)-1    //calculate baud value (MAY TRUNCATE)
#define Fscan   256                       //target buttons & pots sampling rate
#define Fout    44100                      //target output sampling rate (~45k last measured)
#define Fdisp   20                        //15hz display update rate

/* BUFFERS */
#define LOOP_BUF_SIZE 10000             //about 1/4th of a second               //FX settings
#define STREAMBUF   256                   //input adc stack size, must be divisible by 2
#define LCDBUF      128                     //lcd command buffer size
#define LCDBUFMASK  0x007F               //lcd buffer access mask


//misc
#define TRUE    0xFF
#define FALSE   0x00
#define BUTTONS 18
#define POTS	6                       // Number of channels enabled for channel scan
#define SLED _LATF0                                                            
#define LCD_RS _LATE15
#define SS3L _LATF1
#define ENCODERCNTH POS1CNTH
#define ENCODERCNTL POS1CNTL

#endif	/* COMMON_H */

