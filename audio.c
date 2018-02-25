#include "xc.h"
#include "audio.h"
#include "common.h"
#include "definitions.h"
#include "sounds.h"
#include <dsp.h>
#include <math.h>
#include "utilities.h"
#include "flash.h"

extern fractional       TxBufferA[FLASH_DMAXFER_WORDS]__attribute__((space(xmemory))), 
                        RxBufferA[FLASH_DMA_RX_WORDS]__attribute__((space(xmemory)));

extern fractional sintab[SINRES];
extern fractional vol2log[LOGVOLUME_SIZE];
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


void (*fxFuncPointers[NUMFX])(fractional *, fractional *, fractional, fractional, fractional) = {NULL, runLPF, runTRM, runLOP, runBTC, runHPF};

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
    volatile register int resultA asm("A");
    static fractional lpf_alpha=Q15(0.5), lpf_inv_alpha=Q15(0.5);
    float costh, coef;
    static fractional del;
    int counter;
    
    //static fractional delayed_sample;
    volatile fractional sample;
    if(param3>=0x3FFF){     //LPF CONTROL
        /*
        if(param1>=310){                      
            costh=2.0-cos(2*PI*param1/Fout);
            coef=sqrt(costh*costh-1.0); 
        }
         */
        int *readPTR=source;
        int *rewritePTR=destination;
        
        lpf_alpha=param1; 
        lpf_inv_alpha=(32767-lpf_alpha); 

        for(counter=0; counter<STREAMBUF; counter++){
            sample=*readPTR++; //!rw

            //sample=(fractional)(sample*(1 + coef) - del*coef);
            //del = sample;
            
            //LPF-EMA//   y(i)= ??x(i)+(1-?)?y(i-1)
            resultA =__builtin_mpy(sample,lpf_alpha, NULL, NULL, 0, NULL, NULL, 0);
            resultA =__builtin_mac(resultA, del, lpf_inv_alpha, NULL, NULL, 0, NULL, NULL, 0, 0, resultA);
            del=__builtin_sac(resultA, 0);
            sample=del;

            *rewritePTR++=sample; //rw
        }
    }
    
}

void runTRM(fractional *source, fractional *destination, fractional param1, fractional param2, fractional param3){
    volatile register int resultA asm("A");
    volatile register int resultB asm("B");
    volatile fractional sample, trem_mod;
    int *readPTR=source;
    int *rewritePTR=destination;
    int counter=0;
    static unsigned int trem_delay=0, tremelo_ptr=0;
    const int pot_offset = 5;
    
    if(param3>=0x3FFF){     // TREM CONTROL
        resultA =__builtin_mpy(param1,Q15(0.0006), NULL, NULL, 0, NULL, NULL, 0);
        param1=__builtin_sac(resultA, 0);
        if(param2<-0x7) param2=0;
        
        for(; counter<STREAMBUF; counter++){
            sample=*readPTR++; //!rw
                                                           //TREMELO//
            if (trem_delay<=param1+pot_offset){
                trem_delay++;
                resultA =__builtin_mpy(param2,sintab[tremelo_ptr], NULL, NULL, 0, NULL, NULL, 0);
                trem_mod=__builtin_sac(resultA, 0);
                trem_mod=0x7FF0-trem_mod;
                resultB =__builtin_mpy(trem_mod,sample, NULL, NULL, 0, NULL, NULL, 0);
                sample=__builtin_sac(resultB, 0);
            }
            else{
                trem_delay=0;
                if(tremelo_ptr>=(SINRES-1))
                    tremelo_ptr=0;
                else tremelo_ptr++;
                
                resultA =__builtin_mpy(param2,sintab[tremelo_ptr], NULL, NULL, 0, NULL, NULL, 0);
                trem_mod=__builtin_sac(resultA, 0);
                trem_mod=0x7FF0-trem_mod;
                resultB =__builtin_mpy(trem_mod,sample, NULL, NULL, 0, NULL, NULL, 0);
                sample=__builtin_sac(resultB, 0);
            }

            *rewritePTR++=sample; //rw
        }
    }
}

void runBTC(fractional *source, fractional *destination, fractional param1, fractional param2, fractional param3){
    volatile fractional sample, dryScale;
    fractional temp[STREAMBUF];
    
    int *readPTR=source;
    int *rewritePTR=temp;
    int counter=0;
    int shift = scalePotCustom(14, param1);
    fractional sign;
    
    for(; counter<STREAMBUF; counter++){
        sample=*readPTR++; //!rw
        
        if(param3>=0x3FFF){     // BIT CONTROL
            sign = sample&0x8000;
            sample=sample&(-1<<(16-shift));
        }

        *rewritePTR++=sample; //rw
    }
    
    // Dry Wet Control
    VectorScale(STREAMBUF, temp, temp, param2);
    dryScale=32767-param2;
    VectorScale(STREAMBUF, source, source, dryScale);
    VectorAdd(STREAMBUF, destination, source, temp);
    
}

void runLOP(fractional *source, fractional *destination, fractional param1, fractional param2, fractional param3){
    volatile register int result asm("A");
    fractional temp[STREAMBUF];
    fractional dryScale;
    int loop_lim;
    
    result =__builtin_mpy(param1, POT_LOOP, NULL, NULL, 0, NULL, NULL, 0);
    loop_lim = __builtin_sac(result, 0);
    
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
        ClipCopy_eds(STREAMBUF, temp, history.read_ptr);
        // Dry Wet Control
        VectorScale(STREAMBUF, temp, temp, param2);
        dryScale=32767-param2;
        VectorScale(STREAMBUF, source, source, dryScale);
        VectorAdd(STREAMBUF, destination, source, temp);
    }
    history.read_ptr+=STREAMBUF;
}

void runHPF(fractional *source, fractional *destination, fractional param1, fractional param2, fractional param3){
    volatile register int resultA asm("A");
    volatile register int resultB asm("B");
    static fractional hpf_alpha=Q15(0.8360692637); 
    const fractional gain = Q15(0.91803463198);
    fractional temp[STREAMBUF];
    fractional dryScale;
    int counter;
    static fractional new_out, last_out, new_in, last_in;                 
    int *readPTR=source;
    int *rewritePTR=temp;

    VectorScale(STREAMBUF, readPTR, readPTR, gain);
    
    if(param3>0x3FFF){
        for(counter=0; counter<STREAMBUF; counter++){
            new_in=*readPTR++;
            resultA = __builtin_lac(new_in, 0);
            resultB = __builtin_lac(last_in, 0);
            resultA = __builtin_subab(resultA, resultB);
            resultA = __builtin_mac(resultA, last_out, hpf_alpha, NULL, NULL, 0, NULL, NULL, 0, 0, resultA);
            new_out=__builtin_sac(resultA, 0);

            *rewritePTR++=new_out; 
            last_out = new_out;
            last_in = new_in;
        }
        // Dry Wet Control
        VectorScale(STREAMBUF, temp, temp, param2);
        dryScale=32767-param2;
        VectorScale(STREAMBUF, source, source, dryScale);
        VectorAdd(STREAMBUF, destination, source, temp);
    }
}

// See http://www-users.cs.york.ac.uk/~fisher/cgi-bin/mkfscript
void dcHPF(fractional *source,  fractional *destination){
    volatile register int resultA asm("A");
    volatile register int resultB asm("B");
    static fractional hpf_alpha=Q15(0.9978651426);  // Tuned for 15 Hz
    const fractional gain = Q15(0.99893257162);
    int counter;
    static fractional new_out, last_out, new_in, last_in;
                
    int *readPTR=source;
    int *rewritePTR=destination;

    VectorScale(STREAMBUF, readPTR, readPTR, gain);
    for(counter=0; counter<STREAMBUF; counter++){
        new_in=*readPTR++;
        
        resultA = __builtin_lac(new_in, 0);
        resultB = __builtin_lac(last_in, 0);
        resultA = __builtin_subab(resultA, resultB);
        resultA = __builtin_mac(resultA, last_out, hpf_alpha, NULL, NULL, 0, NULL, NULL, 0, 0, resultA);
        new_out=__builtin_sac(resultA, 0);

        *rewritePTR++=new_out; 
        last_out = new_out;
        last_in = new_in;
    }
}

void getAudioIntensity(fractional *signal){
    volatile register int resultA asm("A");
    volatile register int resultB asm("B");
    static fractional lpf_alpha=Q15(0.0548501278);  // Tuned for 40 Hz
    const fractional gain = Q15(0.4725749361);
    //const fractional gain = Q15(0.75);
    static fractional new_out, last_out, new_in, last_in;
    
    new_in = VectorMax(STREAMBUF, signal, NULL);
    if((int)(new_in>32700)) stat.hard_clipped=TRUE;

    resultA =__builtin_mpy(new_in, gain, NULL, NULL, 0, NULL, NULL, 0);
    new_in=__builtin_sac(resultA, 0);
    
    resultA = __builtin_lac(new_in, 0);
    resultB = __builtin_lac(last_in, 0);
    resultA = __builtin_addab(resultA, resultB);
    resultA = __builtin_mac(resultA, last_out, lpf_alpha, NULL, NULL, 0, NULL, NULL, 0, 0, resultA);
    new_out=__builtin_sac(resultA, 0);
    last_out = new_out;
    last_in = new_in;
        
    if(stat.power_ack==TRUE){
        stat.power=new_out;
        stat.power_ack=FALSE;
    }
    else if(new_out>stat.power)
        stat.power=new_out;
    
}

void processAudio(fractional *source, fractional *destination){
    volatile register int resultA asm("A");
    const fractional scale_vollog = Q15(0.125003814814); // Scales value between 0-4096 for lookup
    fractional temp;
    
    if(state==scrnFX){
        if(fxUnits[0]==0); else fxFuncPointers[fxUnits[0]](source, source, ctrl.pots_filtered[FX_1], ctrl.pots_filtered[FX_2], ctrl.pots_filtered[FX_3]);
        if(fxUnits[1]==0); else fxFuncPointers[fxUnits[1]](source, source, ctrl.pots_filtered[FX_4], ctrl.pots_filtered[FX_5], ctrl.pots_filtered[FX_6]);
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
                
    }

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
    if(ctrl.pots_filtered[POT_VOLUME]>=0x7FF7);
    else if(ctrl.pots_filtered[POT_VOLUME]<=0x000F)
        VectorScale(STREAMBUF, destination, destination, 0);
    else{
        resultA =__builtin_mpy(scale_vollog, ctrl.pots_filtered[POT_VOLUME], NULL, NULL, 0, NULL, NULL, 0);
        temp=__builtin_sac(resultA, 0);
        temp=vol2log[temp];
        //VectorScale(STREAMBUF, destination, destination, temp);
        VectorScale(STREAMBUF, destination, destination, ctrl.pots_filtered[POT_VOLUME]); 
    }
}