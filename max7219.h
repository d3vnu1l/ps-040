/*
*********************************************************************************************************
* Module     : MAX7219.H
* Author     : Randy Rasa
* Description: Header file for MAX7219.C (LED Display Driver Routines)
*********************************************************************************************************
*/


/*
*********************************************************************************************************
* Public Function Prototypes
*********************************************************************************************************
*/
void MAX7219_Init (void);
void MAX7219_ShutdownStart (void);
void MAX7219_ShutdownStop (void);
void MAX7219_DisplayTestStart (void);
void MAX7219_DisplayTestStop (void);
void MAX7219_SetBrightness (char brightness);
void MAX7219_Clear (void);
void MAX7219_DisplayChar (char digit, char character);