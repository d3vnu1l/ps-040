#include <xc.h>
#include <p33EP512GM310.h>
#include <libpic30.h>
#include "common.h"

#define ON                        1
#define OFF                       0

#define MAX7219_MODE_DECODE       0x09
#define MAX7219_MODE_INTENSITY    0x0A
#define MAX7219_MODE_SCAN_LIMIT   0x0B
#define MAX7219_MODE_POWER        0x0C
#define MAX7219_MODE_TEST         0x0F
#define MAX7219_MODE_NOOP         0x00

#define MAX7219_DIGIT0            0x01
#define MAX7219_DIGIT1            0x02
#define MAX7219_DIGIT2            0x03

#define MAX7219_CHAR_BLANK        0x0F 
#define MAX7219_CHAR_NEGATIVE     0x0A 

int digitsInUse=1;

void seg_send(int messagea, int messageb){
    int message = (messagea<<2)&&messageb;
    SPI3BUF=message;
    SEG_SEL=0;
    seg_wait();
}

void seg_wait(void){
    while (!SPI3STATbits.SPIRBF);
    SEG_SEL=1;
    int trash=SPI3BUF;
    
}

void seg_clear(void){
    seg_send(digitsInUse, MAX7219_CHAR_BLANK);
}

void seg_setup(void){
    seg_send(MAX7219_MODE_DECODE, 0x00);
    seg_send(MAX7219_MODE_SCAN_LIMIT, digitsInUse - 1);
    seg_send(MAX7219_MODE_INTENSITY, 8);
    seg_send(MAX7219_MODE_POWER, ON);
}

void seg_display(char num){
    seg_clear();
    seg_send(MAX7219_DIGIT0, 5);
}
