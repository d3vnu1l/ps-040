#include "xc.h"
#include "audio.h"
#include "common.h"
#include <dsp.h>



extern char pad[BUTTONS];                                                             //CONTROL VARIABLES//
extern fractional pots[POTS];
extern fractional pots_scaled[POTS];

volatile unsigned int loop_ptr = 0;                                             //FX FLAGS & VARS//
extern unsigned char hard_clipped;
volatile fractional loop[LOOP_BUF_SIZE]={0};
fractional lpf_alpha=Q15(0.5), lpf_inv_alpha=Q15(0.5);

extern fractional sintab[1024];     //misc//
extern unsigned char TEST_SIN;

extern unsigned char kick_playing;                                              //SAMPLES//
extern unsigned int kick_max, kick_ptr;
extern fractional kick[5552];
//extern unsigned char kick_mode;
/*
extern unsigned char hat_playing;
extern unsigned int hat_max, hat_ptr;
extern fractional hat[5552];
 */
extern unsigned char snare_playing;
extern unsigned int snare_max, snare_ptr;
extern fractional snare[5552];
extern enum fxStruct fxUnits[NUMFXUNITS];

void (*fxFuncPointers[NUMFX])(fractional *, fractional *, fractional, fractional, fractional) = {NULL, runLPF, runTRM, runLOP, runBIT};

void runBufferLooper(fractional *source){
    static fractional delayed_sample;
    volatile fractional sample;
    
    int *readPTR=source;
    
    int counter=0;
    for(; counter<STREAMBUF; counter++){
        sample=*readPTR++; //!rw
        if(loop_ptr<LOOP_BUF_SIZE)
            loop[loop_ptr++]=sample;
        else {
            loop_ptr=0;
            loop[loop_ptr++]=sample;
        }
    }
}

void runLPF(fractional *source, fractional *destination, fractional param1, fractional param2, fractional param3){
    volatile register int result asm("A");
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

void runBIT(fractional *source, fractional *destination, fractional param1, fractional param2, fractional param3){
    volatile fractional sample;
    
    int *readPTR=source;
    int *rewritePTR=destination;
    int counter=0;
    int shift = scalePotsCustom(14, param1);
    fractional shiftedsample, sign;
    
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
    volatile fractional sample;
    
    result =__builtin_mpy(param1,Q15(0.3052), NULL, NULL, 0, NULL, NULL, 0);
    int loop_lim = __builtin_sac(result, 0);
    
    int *readPTR=source;
    int *rewritePTR=destination;
    int counter=0;
    
    //Run looper Buffer
    if(param3<0x3FFF){
        runBufferLooper(source); 
    }
    else {
        for(; counter<STREAMBUF; counter++){
            sample=*readPTR++; //!rw    

            if(param3>=0x3FFF){
                if(loop_lim>=LOOP_BUF_SIZE)
                    loop_lim=LOOP_BUF_SIZE;
                if(loop_ptr<loop_lim){
                    sample = (loop[loop_ptr++]);
                }
                else {
                    loop_ptr=0;
                    sample = (loop[loop_ptr++]);
                }
            }
            *rewritePTR++=sample; //rw
        }               
    }
}

void processAudio(fractional *source, fractional *destination){
    volatile register int result1 asm("A");
    static int i=0;
    volatile fractional sample;
    int counter=0;
    
    int *readPTR=source;
    int *rewritePTR=destination;
    
    //Run each FX unit
    if(fxUnits[0]==0); else fxFuncPointers[fxUnits[0]](source, source, pots[FX_1], pots[FX_2], pots[FX_3]);
    if(fxUnits[1]==0); else fxFuncPointers[fxUnits[1]](source, source, pots[FX_4], pots[FX_5], pots[FX_6]);
   
    if(kick_playing==TRUE&&kick_ptr<kick_max){
        result1 =__builtin_mpy(sample,Q15(0.85), NULL, NULL, 0, NULL, NULL, 0);
        result1 = __builtin_add(result1,kick[kick_ptr++],0);
        sample=__builtin_sac(result1, 0);
    }
    else if (pad[0]==1&&kick_playing==TRUE&&kick_ptr==kick_max){
        kick_playing=FALSE;
        kick_ptr=0;
    }

    if(snare_playing==TRUE){
        result1 =__builtin_mpy(sample,Q15(0.85), NULL, NULL, 0, NULL, NULL, 0);

        result1 = __builtin_add(result1,snare[snare_ptr++],0);
        sample=__builtin_sac(result1, 0);

        snare_playing=FALSE;
    }


    if (TEST_SIN==TRUE){
        i++;
        if(i==1024)
            i=0;
        sample=sintab[i];
    }
    
    for(; counter<STREAMBUF; counter++){
        sample=*readPTR++; //!rw

        //return sample;
        *rewritePTR++=sample; //rw
    }
    
    //VOLUME CONTROL
    //if(pots[POT_VOLUME]<=0x000F); 
    if(pots[POT_VOLUME]>=0x7FF7);
    else{
        VectorScale(STREAMBUF, destination, destination, pots[POT_VOLUME]);
    }
}