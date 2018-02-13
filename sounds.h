// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef SOUNDS_H
#define	SOUNDS_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <dsp.h>

struct clip{
    const unsigned int size, blocks;
    unsigned char playing, flash;
    unsigned int block_index;
    fractional* start_ptr;
    fractional* read_ptr;
};

struct clip_psv{
    const unsigned int size, blocks;
    unsigned char playing, flash;
    unsigned int block_index;
    __psv__ fractional* start_ptr;
    __psv__ fractional* read_ptr;
};

struct clip_eds{
    const unsigned int size, blocks;
    unsigned char playing, flash;
    unsigned int block_index;
    __eds__ fractional* start_ptr;
    __eds__ fractional* end_ptr;
    __eds__ fractional* read_ptr;
};




void genSine(unsigned int);

#endif	/* XC_HEADER_TEMPLATE_H */

