ps-040 Sampler project, uses DSPI33ep512GM310

Table of Contents:
  main.c;
    Main program loop, responsible for scanning buttons, pots, and general program control flow
  devInits.c;
    Contains functions used to initialize device and it's peripherals on startup
  routines.c
    Contains ISR
  utilities.c
    Contains utility functions, i.e. read adc, scan buttons, print info to uart, etc.
  sounds.c
    Contains sound clips and their corresponding attributes
  audio.c
    Controls audio processing flow, functions in here are generally used AFTER DCI interrupt has fetched a new sample
  max7219.c
    Functions necessary to simplify LED output


Audio processing flow:
  A new sample is ready for processing after the DCI interrupt is triggered. When this happens; 
      1. the last processed samples are loaded into TXBUF0 & TXBUF 1
      2. new samples are fetched from RXBUF0 & RXBUF1 (ADC)
          -MSB is toggled to fix signing error
          -Check for clipping
          -Add to stream[rw][write_ptr] ping pong array
              *ping pong array allows writing to memory while simultanously transfering blocks to SD over DMA
      3. call fx function to process sample
          -overwrite stream with return value
      4. call mixer(stream[~rw][write_ptr])
          -set 'output' as return value
      'output' is sent to DAC on next interrupt
