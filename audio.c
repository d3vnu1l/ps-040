#include "xc.h"
#include "audio.h"
#include "common.h"
#include "sounds.h"
#include <dsp.h>
#include "utilities.h"
#include "flash.h"

extern unsigned char    TxBufferA[FLASH_DMAXFER_BYTES]__attribute__((space(xmemory))), 
                        RxBufferA[FLASH_DMAXFER_BYTES]__attribute__((space(xmemory)));
extern fractional       RxBufferB[STREAMBUF] __attribute__((space(xmemory)));

extern fractional sintab[SINRES];
static fractional loopbuf[LOOP_BUF_SIZE] __attribute__ ((eds)) = {0};
static fractional psvbuf[STREAMBUF]={0};

extern enum fxStruct fxUnits[NUMFXUNITS];
extern enum screenStruc state;
extern struct clip_psv sine, kick, snare;
extern struct sflags stat;
extern struct ctrlsrfc ctrl;
struct clip_eds history = { .size = LOOP_BUF_SIZE,
                            .blocks=LOOP_BUF_SIZE/STREAMBUF, 
                            .playing=FALSE, .flash=FALSE, 
                            .block_index=0, 
                            .start_ptr=loopbuf, 
                            .end_ptr=&loopbuf[LOOP_BUF_SIZE-1], 
                            .read_ptr=loopbuf};


void (*fxFuncPointers[NUMFX])(fractional *, fractional *, fractional, fractional, fractional) = {NULL, runLPF, runTRM, runLOP, runBTC};

void runBufferLooper(fractional *source){
    volatile fractional sample;
    unsigned int loop_ptr = 0;
    
    int *readPTR=source;
    
    int counter=0;
    for(; counter<STREAMBUF; counter++){
        sample=*readPTR++; //!rw
        if(loop_ptr<LOOP_BUF_SIZE)
            loopbuf[loop_ptr++]=sample;
        else {
            loop_ptr=0;
            loopbuf[loop_ptr++]=sample;
        }
    }
}

void runLPF(fractional *source, fractional *destination, fractional param1, fractional param2, fractional param3){
    volatile register int result asm("A");
    static fractional lpf_alpha=Q15(0.5), lpf_inv_alpha=Q15(0.5);
    
    static fractional delayed_sample;
    volatile fractional sample;
    if(param3>=0x3FFF){     //LPF CONTROL
        if(param1>=310){                      
            lpf_alpha=param1;
            lpf_inv_alpha=(32767-lpf_alpha); 
        }


        int *readPTR=source;
        int *rewritePTR=destination;


        int counter=0;
        for(; counter<STREAMBUF; counter++){
            sample=*readPTR++; //!rw

            //LPF-EMA//   y(i)= ??x(i)+(1-?)?y(i-1)
            result =__builtin_mpy(sample,lpf_alpha, NULL, NULL, 0, NULL, NULL, 0);
            result =__builtin_mac(result, delayed_sample, lpf_inv_alpha, NULL, NULL, 0, NULL, NULL, 0, 0, result);
            delayed_sample=__builtin_sac(result, 0);
            sample=delayed_sample;

            *rewritePTR++=sample; //rw
        }
    }
    
}

void runTRM(fractional *source, fractional *destination, fractional param1, fractional param2, fractional param3){
    volatile register int result1 asm("A");
    volatile register int result2 asm("B");
    volatile fractional sample, trem_mod;
    int *readPTR=source;
    int *rewritePTR=destination;
    int counter=0;
    static unsigned int trem_delay=0, tremelo_ptr=0;
    const int pot_offset = 5;
    
    if(param3>=0x3FFF){     // TREM CONTROL
        result1 =__builtin_mpy(param1,Q15(0.0006), NULL, NULL, 0, NULL, NULL, 0);
        param1=__builtin_sac(result1, 0);
        if(param2<-0x7) param2=0;
        
        for(; counter<STREAMBUF; counter++){
            sample=*readPTR++; //!rw
                                                           //TREMELO//
            if (trem_delay<=param1+pot_offset){
                trem_delay++;
                result1 =__builtin_mpy(param2,sintab[tremelo_ptr], NULL, NULL, 0, NULL, NULL, 0);
                trem_mod=__builtin_sac(result1, 0);
                trem_mod=0x7FF0-trem_mod;
                result2 =__builtin_mpy(trem_mod,sample, NULL, NULL, 0, NULL, NULL, 0);
                sample=__builtin_sac(result2, 0);
            }
            else{
                trem_delay=0;
                if(tremelo_ptr==1024)
                    tremelo_ptr=0;
                else tremelo_ptr++;
                
                result1 =__builtin_mpy(param2,sintab[tremelo_ptr], NULL, NULL, 0, NULL, NULL, 0);
                trem_mod=__builtin_sac(result1, 0);
                trem_mod=0x7FF0-trem_mod;
                result2 =__builtin_mpy(trem_mod,sample, NULL, NULL, 0, NULL, NULL, 0);
                sample=__builtin_sac(result2, 0);
            }

            *rewritePTR++=sample; //rw
        }
    }
}

void runBTC(fractional *source, fractional *destination, fractional param1, fractional param2, fractional param3){
    volatile fractional sample;
    
    int *readPTR=source;
    int *rewritePTR=destination;
    int counter=0;
    int shift = scalePotsCustom(14, param1);
    fractional sign;
    
    for(; counter<STREAMBUF; counter++){
        sample=*readPTR++; //!rw
        
        if(param3>=0x3FFF){     // BIT CONTROL
            sign = sample&0x8000;
            sample=sample&(-1<<(16-shift));
        }

        *rewritePTR++=sample; //rw
    }
}

void runLOP(fractional *source, fractional *destination, fractional param1, fractional param2, fractional param3){
    volatile register int result asm("A");
    
    result =__builtin_mpy(param1, POT_LOOP, NULL, NULL, 0, NULL, NULL, 0);
    int loop_lim = __builtin_sac(result, 0);
    
    __eds__ fractional *ptrr = history.start_ptr;
    ptrr+=loop_lim;
    
    //Run looper Buffer
    if(history.read_ptr>=(ptrr)) {
        history.read_ptr=history.start_ptr;
    }
    if(param3<0x3FFF){
        ClipCopy_toeds(STREAMBUF, history.read_ptr, destination);
    }
    
    else {
        ClipCopy_eds(STREAMBUF, destination, history.read_ptr);
    }
    history.read_ptr+=STREAMBUF;
}

void processAudio(fractional *source, fractional *destination){
    volatile register int result1 asm("A");
    
    if(state==scrnFX){
        if(fxUnits[0]==0); else fxFuncPointers[fxUnits[0]](source, source, ctrl.pots[FX_1], ctrl.pots[FX_2], ctrl.pots[FX_3]);
        if(fxUnits[1]==0); else fxFuncPointers[fxUnits[1]](source, source, ctrl.pots[FX_4], ctrl.pots[FX_5], ctrl.pots[FX_6]);
    }

    if(kick.playing==TRUE){
        ClipCopy_psv(STREAMBUF, psvbuf, kick.read_ptr);
           
        //ClipCopy_psv(STREAMBUF, source, sine.read_ptr);
        kick.block_index++;
        if(kick.block_index==kick.blocks) {
            kick.read_ptr=kick.start_ptr;
            kick.block_index=0;
            kick.playing=FALSE;
        }
        else kick.read_ptr+=STREAMBUF;
                
        //VectorScale(STREAMBUF, psvbuf, psvbuf, Q15(0.9));
        VectorAdd(STREAMBUF, source, source, psvbuf);
                
        //result1 =__builtin_mpy(sample,Q15(0.85), NULL, NULL, 0, NULL, NULL, 0);
        //result1 = __builtin_add(result1,kick[kick_ptr++],0);
        //sample=__builtin_sac(result1, 0);
    }
    /*
    else if (pad[0]==1&&kick_playing==TRUE&&kick_ptr==kick_max){
        kick_playing=FALSE;
        kick_ptr=0;
    }

    if(snare.playing==TRUE){
        result1 =__builtin_mpy(sample,Q15(0.85), NULL, NULL, 0, NULL, NULL, 0);

        result1 = __builtin_add(result1,snare[snare_ptr++],0);
        sample=__builtin_sac(result1, 0);

        snare_playing=FALSE;
    }
    */

    if (stat.TEST_SIN==TRUE){
        ClipCopy_psv(STREAMBUF, source, sine.read_ptr);
        sine.block_index++;
        if(sine.block_index==sine.blocks) {
            sine.read_ptr=sine.start_ptr;
            sine.block_index=0;
        }
        //else sine.read_ptr+=STREAMBUF;
    }
        
    VectorCopy(STREAMBUF, destination, source);     //copy from ping to pong buffer
    
    //VOLUME CONTROL
    //if(pots[POT_VOLUME]<=0x000F); 
    if(ctrl.pots[POT_VOLUME]>=0x7FF7);
    else{
        VectorScale(STREAMBUF, destination, destination, ctrl.pots[POT_VOLUME]);
    }
}