#ifndef COMMON_H
#define	COMMON_H
#include <xc.h> 
#include <dsp.h>

#define Fin     73728000
#define Fosc    138240000
#define Fcy     Fosc/2

/* Timers and Frequencies */
#define BAUDRATE 9600                  //target baud rate for *UART*           //peripheral configs
#define BRGVAL  ((Fcy/BAUDRATE)/16)-1    //calculate baud value (MAY TRUNCATE)
#define Fscan   200                       //target buttons & pots sampling rate
#define Fout    44100                      //target output sampling rate (~45k last measured)
#define Fdisp   10                        //15hz display update rate

#define SINRES  1024

/* BUFFERS */

#define STREAMBUF       256                   //input adc stack size, must be divisible by 2
#define LOOP_BUF_SIZE   17408             
#define LCDBUF          256                     //lcd command buffer size

/* MISC */
#define TRUE    0xFF
#define FALSE   0x00
#define BUTTONS 35
#define POTS	12                       
#define NUMFX   5
#define NUMFXUNITS 2

/* Pin Map */
#define SLED _LATF0                                                            
#define LCD_RS _LATE15
#define SS3a _LATF1
#define SS3b _LATD15
#define FLASHCLK _LATC9

#define ENCODERCNTH POS1CNTH
#define ENCODERCNTL POS1CNTL

#define POT_PERCENT Q15(0.3937)      // For pot 0-100 display
#define POT_LOOP Q15(1.0*LOOP_BUF_SIZE/32767.0)
#define FXSCALE Q15(NUMFX*0.000030518509476)

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
    12  13  14  15  |   29  30  31  32      Encoder:
    8   9   10  11  |   25  26  27  28              16  |   33
    4   5   6   7   |   21  22  23  24      Function:
 `` 0   1   2   3   |   17  18  19  20              34
 */
#define BTN_SPECIAL         34
#define BTN_ENC             16

/* Flash Variable */
#define FLASH_DMAXFER_BYTES 512 // Must be double of streambuf
#define FLASH_PAGE 512
#define FLASH_SECTOR 256000
/* Memory Map */
#define FLASH_MAX 0x04000000
#define FLASH_NUMCHUNKS 16

enum screenStruc{
    invalid,
    scrnFX,
    scrnSHIFT,
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
    lop,
    btc
};

struct ctrlsrfc {
    unsigned char pad[BUTTONS];
    fractional pots[POTS];
    fractional pots_scaled[POTS];
    fractional pots_custom[POTS];
};

struct sflags {
    unsigned char UART_ON;
    unsigned char TEST_SIN;
    unsigned char DMA_JUSTREAD;
    unsigned char DMA_READING;
    unsigned char hard_clipped; 
};

#endif	/* COMMON_H */

