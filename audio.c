#include "xc.h"
#include "common.h"
#include <dsp.h>

extern char pad[BUTTONS];                                                             //CONTROL VARIABLES//
extern fractional pots[POTS];
extern fractional pots_scaled[POTS];

volatile unsigned int loop_ptr = 0;                                             //FX FLAGS & VARS//
unsigned int loop_lim=0;
unsigned int tremelo_ptr=0, tremelo_length=0, trem_var=0;
extern unsigned char hard_clipped, tremelo, looper, lpf;
volatile fractional loop[LOOP_BUF_SIZE]={0};
unsigned int delay_samples=0;
fractional lpf_alpha=Q15(0.5), lpf_inv_alpha=Q15(0.5);
fractional tremelo_depth=Q15(1.0);

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

void processAudio(fractional *source, fractional *destination){
    volatile register int result1 asm("A");
    static int i=0;
    volatile fractional sample;
    int counter=(STREAMBUF-1);
    
    runFX(source, source);
    

    for(; counter>=0; counter--){
        sample=*source--; //!rw

        if(kick_playing==TRUE&&kick_ptr<kick_max){
            result1 =__builtin_mpy(sample,Q15(0.90), NULL, NULL, 0, NULL, NULL, 0);
            result1 = __builtin_add(result1,kick[kick_ptr++],0);
            sample=__builtin_sac(result1, 0);
        }
        else if (pad[0]==1&&kick_playing==TRUE&&kick_ptr==kick_max){
            kick_playing=FALSE;
            kick_ptr=0;
        }
        /*
        if(hat_playing==TRUE&&hat_ptr<hat_max){
            result1 =__builtin_mpy(sample,Q15(0.85), NULL, NULL, 0, NULL, NULL, 0);
            result1 = __builtin_add(result1,hat[hat_ptr++],0);
            sample=__builtin_sac(result1, 0);
        }
        else if (pad[2]==1&&hat_playing==TRUE&&hat_ptr==hat_max){
            hat_playing=FALSE;
            hat_ptr=0;
        }
         */

        if(snare_playing==TRUE&&snare_ptr<snare_max){
            result1 =__builtin_mpy(sample,Q15(0.85), NULL, NULL, 0, NULL, NULL, 0);
            result1 = __builtin_add(result1,snare[snare_ptr++],0);
            sample=__builtin_sac(result1, 0);
        }
        else if (pad[1]==1&&snare_playing==TRUE&&snare_ptr==snare_max){
            snare_playing=FALSE;
            snare_ptr=0;
        }


        if (TEST_SIN==TRUE){
            i++;
            if(i==1024)
                i=0;
            sample=sintab[i];
        }

        //VOLUME CONTROL
        if(pots[POT_VOLUME]==0x0007)
            sample=0;
        else if(pots[POT_VOLUME]>=0x7FF7);
        else{
            result1 =__builtin_mpy(sample,pots[POT_VOLUME], NULL, NULL, 0, NULL, NULL, 0);
            sample=__builtin_sac(result1, 0);
        }
        
        //return sample;
        *destination--=sample; //rw
    }
}

void runFX(fractional *source, fractional *destination){
    volatile register int result1 asm("A");
    volatile register int result2 asm("B");
    static fractional delayed_sample;
    static fractional trem_mod; 
    volatile fractional sample;
    
    
    int *readPTR=source;
    int *rewritePTR=destination;
    
    
    int counter=(STREAMBUF-1);
    for(; counter>=0; counter--){
        sample=*readPTR--; //!rw

        if(tremelo==TRUE){                                                          //TREMELO//
            if (trem_var<=pots[3]){
                trem_var++;
                result1 =__builtin_mpy(tremelo_depth,sintab[tremelo_ptr], NULL, NULL, 0, NULL, NULL, 0);
                trem_mod=__builtin_sac(result1, 0);
                result2 =__builtin_mpy(trem_mod,sample, NULL, NULL, 0, NULL, NULL, 0);
                sample=__builtin_sac(result2, 0);
            }
            else{
                trem_var=0;
                if(tremelo_ptr==1024)
                    tremelo_ptr=0;
                result1 =__builtin_mpy(tremelo_depth,sintab[tremelo_ptr++], NULL, NULL, 0, NULL, NULL, 0);
                trem_mod=__builtin_sac(result1, 0);
                result2 =__builtin_mpy(trem_mod,sample, NULL, NULL, 0, NULL, NULL, 0);
                sample=__builtin_sac(result2, 0);
            }
        }
        if(looper==TRUE){                           //LOOPER// 
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
        else {                                      //buffer
            if(loop_ptr<LOOP_BUF_SIZE)
                loop[loop_ptr++]=sample;
            else {
                loop_ptr=0;
                loop[loop_ptr++]=sample;
            }
        }

        //LPF-EMA//   y(i)= ??x(i)+(1-?)?y(i-1)
        if(lpf==TRUE){
            result2 =__builtin_mpy(sample,lpf_alpha, NULL, NULL, 0, NULL, NULL, 0);
            result2 =__builtin_mac(result2, delayed_sample, lpf_inv_alpha, NULL, NULL, 0, NULL, NULL, 0, 0, result2);
            delayed_sample=__builtin_sac(result2, 0);
            sample=delayed_sample;
        }
        
        *rewritePTR--=sample; //rw
    }
}

//test