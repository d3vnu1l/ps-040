#ifndef COMMON_H
#define	COMMON_H
#include <xc.h> 

/*
#define Fosc 138240000                                  
*/
#define Fin   73728000
#define Fosc 138240000
#define Fcy Fosc/2

#define BAUDRATE 57600                  //target baud rate for *UART*           //peripheral configs
#define BRGVAL ((Fcy/BAUDRATE)/16)-1    //calculate baud value (MAY TRUNCATE)
#define Fscan 256                       //target buttons & pots sampling rate
#define Fout 44100                      //target output sampling rate (~45k last measured)
#define STREAMBUF 256                   //input adc stack size, must be large enough to avoid SD write respond latency
#define LCDBUF  528                     //lcd command buffer size
#define Fdisp 15                        //15hz display update rate
#define BUTTONS 17

//#define  SAMP_BUFF_SIZE	 		4		// Size of the input buffer per analog input
#define  POTS			8		// Number of channels enabled for channel scan

#define LOOP_BUF_SIZE 10000             //about 1/4th of a second               //FX settings

#define SLED _LATF0                                                             //various status LED's
#define YLED _LATE1
#define GLED _LATC4
#define RLED _LATC0
#define HARD_CLIP_LED _LATC1            //Clip indicator LED
#define TREMELO_LED _LATD15
#define LCD_RS _LATE15
#define LCD_RS_P PORTEbits.RE15

//misc
#define TRUE 0xFF
#define FALSE 0x00

#endif	/* COMMON_H */

