#ifndef COMMON_H
#define	COMMON_H
#include <xc.h> 
#include <dsp.h>

#define Fin     73728000
#define Fosc    138240000
#define Fcy     Fosc/2

/* Timers and Frequencies */
#define BAUDRATE 57600                  //target baud rate for *UART*           //peripheral configs
#define BRGVAL  ((Fcy/BAUDRATE)/16)-1    //calculate baud value (MAY TRUNCATE)
#define Fscan   300                       //target buttons & pots sampling rate
#define Fout    44100                      //target output sampling rate (~45k last measured)
#define Fdisp   15                        //15hz display update rate

/* BUFFERS */
#define LOOP_BUF_SIZE 10000             //about 1/4th of a second               //FX settings
#define STREAMBUF   256                   //input adc stack size, must be divisible by 2
#define LCDBUF      256                     //lcd command buffer size
#define LCDBUFMASK  0x00FF               //lcd buffer access mask


//misc
#define TRUE    0xFF
#define FALSE   0x00
#define BUTTONS 35
#define POTS	12                       
#define NUMFX   4
#define NUMFXUNITS 2

#define SLED _LATF0                                                            
#define LCD_RS _LATE15
#define SS3 _LATF1
#define ENCODERCNTH POS1CNTH
#define ENCODERCNTL POS1CNTL

#define POT_PERCENT Q15(0.3937)      // For pot 0-100 display

/* Potentiometer mapping 
    0   1  |  6   7
    2   3  |  8   9
    4   5  |  10  11      */
#define POT_VOLUME          7
#define POT_FX_SELECT1      10
#define POT_FX_SELECT2      11
#define FX_1                0
#define FX_2                2
#define FX_3                4
#define FX_4                1
#define FX_5                3
#define FX_6                5



/* Button Mapping
    12  13  14  15  |   28  29  30  31      Encoder:
    8   9   10  11  |   24  25  26  27              32  |   33
    4   5   6   7   |   20  21  22  23      Function:
 `` 0   1   2   3   |   16  17  18  19              34
 */
#define BTN_SPECIAL         34

enum screenStruc{
    invalid,
    scrnFX,
    debugscrnPOTS,
    debugscrnFLASH,
    debugscrnBUFFERS,
    debugscrnINPUT,
    debugscrnAUDIO,
    start
};

enum fxStruct{
    off,
    lpf,
    trm,
    lop
};

#endif	/* COMMON_H */

