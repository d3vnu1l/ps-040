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
#define T3freq 44100                    //target timer 3 frequency
#define Fscan 512                       //target buttons & pots sampling rate
#define Fout 44100                      //target output sampling rate (~45k last measured)
#define STREAMBUF 256                   //input adc stack size, must be large enough to avoid SD write respond latency
#define Fdisp 30                        //15hz display update rate

#define LOOP_BUF_SIZE 11025             //about 1/4th of a second               //FX settings

#define SLED _LATF0                                                             //various status LED's
#define YLED _LATE1
#define GLED _LATC4
#define RLED _LATC0
#define HARD_CLIP_LED _LATC1            //Clip indicator LED
#define TREMELO_LED _LATD15

#define p3 PORTGbits.RG0                //raw button inputs
#define p2 PORTGbits.RG1
#define p5 PORTGbits.RG3
#define p1 PORTGbits.RG10
#define p6 PORTGbits.RG11
#define p4 PORTGbits.RG12
#define p7 PORTGbits.RG13
#define p0 PORTGbits.RG14

//#define ADC_CONV _LATB11             //was originally for adc conv, now unused
//spi1 (SD-card) manula CS
#define SD_EN _LATC5
#define SEG_SEL _LATD13

//misc
#define TRUE 0xFF
#define FALSE 0x00


#endif	/* COMMON_H */

