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
#define STREAMBUF 64                   //input adc stack size, must be large enough to avoid SD write respond latency
#define Fdisp 30                        //15hz display update rate
#define BUTTONS 17

#define LOOP_BUF_SIZE 11025             //about 1/4th of a second               //FX settings

#define SLED _LATF0                                                             //various status LED's
#define YLED _LATE1
#define GLED _LATC4
#define RLED _LATC0
#define HARD_CLIP_LED _LATC1            //Clip indicator LED
#define TREMELO_LED _LATD15
#define LCD_RS _LATE15
#define LCD_RS_P PORTEbits.RE15
#define lcd_rs_toggle {__builtin_btg(&LATE,15);}    //slower!
#define PADS PORTG


//#define ADC_CONV _LATB11             //was originally for adc conv, now unused
//spi1 (SD-card) manula CS
#define SD_EN _LATC5

//misc
#define TRUE 0xFF
#define FALSE 0x00

#endif	/* COMMON_H */

