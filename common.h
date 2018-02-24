#ifndef COMMON_H
#define	COMMON_H
#include <xc.h> 
#include <dsp.h>

#define Fin     73728000
#define Fosc    138240000
#define Fcy     Fosc/2

/* Timers and Frequencies */
#define BAUDRATE 115200  //38400                  //target baud rate for *UART*           //peripheral configs
#define BRGVAL  ((Fcy/BAUDRATE)/16)-1       //calculate baud value (MAY TRUNCATE)
#define Fscan   200                         //target buttons & pots sampling rate
#define Fout    44100                       //target output sampling rate (~45k last measured)
#define Fdisp   20                          //15hz display update rate

#define SINRES  1024

/* BUFFERS */

#define STREAMBUF       256                   //input adc stack size, must be divisible by 2
#define LOOP_BUF_SIZE   17408             
#define LCDBUF          256                     //lcd command buffer size
#define BTBUF_BYTES     512
#define BTBUF_WORDS     BTBUF_BYTES/2

/* MISC */
#define TRUE    0xFF
#define FALSE   0x00
#define BUTTONS 19
#define POTS	12                       
#define NUMFX   6
#define NUMFXUNITS 2
#define SCREENS 9
#define VOICES 6

#define LOGMETER_SIZE 64
#define LOGVOLUME_SIZE 4096

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

#define LED_R   _LATF13
#define LED_G   _LATD14
#define LED_B   _LATF12

/* Button Mapping
    12  13  14  15  |         Encoder:
    8   9   10  11  |                 16  |   17
    4   5   6   7   |         Function:
    0   1   2   3   |                 18
 */
#define BTN_SPECIAL         18
#define BTN_ENC             16
#define BTN_ENCSPEC         17

/* Flash Variable */
#define FLASH_DMAXFER_BYTES     512                     // Must be double of streambuf
#define FLASH_DMAXFER_WORDS     FLASH_DMAXFER_BYTES/2   
//#define FLASH_DMA_RX_BYTES      FLASH_DMAXFER_BYTES*VOICES
#define FLASH_DMA_RX_WORDS      FLASH_DMAXFER_WORDS*VOICES
#define FLASH_PAGE              512
#define FLASH_SECTOR            256000
/* Memory Map */
#define FLASH_MAX               0x04000000
#define FLASH_NUMCHUNKS         16
#define CHUNKSIZE               (FLASH_MAX/FLASH_NUMCHUNKS)
#define FLASH_CHUNKSPER         CHUNKSIZE/(2*STREAMBUF)

struct ctrlsrfc {
    unsigned char   pad[BUTTONS];
    unsigned char   last_pressed;
    fractional      pots[POTS];
    unsigned char   pot_moved[POTS];
    fractional      pots_scaled[POTS];
    fractional      pots_custom[POTS];
};

struct sflags {
    unsigned char UART_ON;
    unsigned char TEST_SIN;
    unsigned char AT_MODE;
    unsigned char hard_clipped; 
    unsigned int  process_time;
    fractional    power;
    unsigned char power_ack;
    unsigned char dma_rts;
    unsigned char dma_queue; 
    unsigned char dma_framesize;
    unsigned int  dma_rx_index;
    unsigned long dma_writeQ_index;
    fractional*   dma_write_buffer;
    char          rgb_led;
};


struct bluetooth {
    const unsigned char AT_MODE;
    fractional          rxBuf[BTBUF_WORDS];
    unsigned char       *writePtr;
    unsigned char       *btReadPtr;
    unsigned char       last;
    unsigned char       dataReady;
    unsigned char       status;
};

#endif	/* COMMON_H */

