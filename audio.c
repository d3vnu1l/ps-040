#include "xc.h"
#include "audio.h"
#include "common.h"
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
    volatile register int resultB asm("B");
    float costh, coef;
    static fractional del;
    int counter;
    
    //static fractional delayed_sample;
    volatile fractional sample;
    if(param3>=0x3FFF){     //LPF CONTROL
        if(param1>=310){                      
            costh=2.0-cos(2*PI*param1/Fout);
            coef=sqrt(costh*costh-1.0); 
        }

        int *readPTR=source;
        int *rewritePTR=destination;

        for(counter=0; counter<STREAMBUF; counter++){
            sample=*readPTR++; //!rw

            sample=(fractional)(sample*(1 + coef) - del*coef);
            del = sample;
            
            //LPF-EMA//   y(i)= ??x(i)+(1-?)?y(i-1)
            //resultA =__builtin_mpy(sample,lpf_alpha, NULL, NULL, 0, NULL, NULL, 0);
            //resultA =__builtin_mac(resultA, delayed_sample, lpf_inv_alpha, NULL, NULL, 0, NULL, NULL, 0, 0, resultA);
            //delayed_sample=__builtin_sac(resultA, 0);
            //sample=delayed_sample;

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
                if(tremelo_ptr>=(SINRES-1))
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

void runHPF(fractional *source, fractional *destination, fractional param1, fractional param2, fractional param3){
    volatile register int resultA asm("A");
    volatile register int resultB asm("B");
    static fractional hpf_alpha=Q15(0.8360692637); 
    const fractional gain = Q15(0.91803463198);
    int counter;
    static fractional new_out, last_out, new_in, last_in;                 
    int *readPTR=source;
    int *rewritePTR=destination;

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
    if(ctrl.pots[POT_VOLUME]>=0x7FF7);
    else if(ctrl.pots[POT_VOLUME]<=0x000F)
        VectorScale(STREAMBUF, destination, destination, 0);
    else{
        resultA =__builtin_mpy(scale_vollog, ctrl.pots[POT_VOLUME], NULL, NULL, 0, NULL, NULL, 0);
        temp=__builtin_sac(resultA, 0);
        temp=vol2log[temp];
        //VectorScale(STREAMBUF, destination, destination, temp);
        VectorScale(STREAMBUF, destination, destination, ctrl.pots[POT_VOLUME]); 
    }
}