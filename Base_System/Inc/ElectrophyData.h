#ifndef __ElectrophyData_H__
#define __ElectrophyData_H__

#include <stdint.h>
#include <stdlib.h>
#include "stm32f4xx.h"

#define	SIZE_BUFFER	 200

#define NRF_FRAME	   32
#define USB_FRAME	 	 4 

#define NRF_DAC_FRAME	4
#define DAC_FRAME	    8

/**************************************************************/
//					Enum
/**************************************************************/
typedef enum{Dac, Usb} Output_device_t;

/**************************************************************/
//					Structures
/**************************************************************/

// handler of the buffer and its pointers
typedef struct
{
	// a buffer of SIZE_BUFFER usb frames 
  // which countain USB_FRAME nrf frames
	uint16_t Data[SIZE_BUFFER][USB_FRAME][NRF_FRAME]; 
		
	// index of the X1th element of Data[X1][0][0]
	// used to send a USB packet form the buffer to the USB periph
	uint16_t	ReadIndexUsb;

	// index of the X2th element of Data[X2][0][0]
	// used to write a NRF packet to the buffer
	uint16_t	WriteIndexUsb;
	uint16_t	PreviousWriteIndexUsb; // Data[X2(t-1)][0][0]
	
	// index of the Y2th element of Data[X2][Y2][0]
	// used to write a NRF packet to the buffer
	uint16_t	WriteIndexNrf;
	uint16_t	PreviousWriteIndexNrf; // Data[X2(t-1)][Y2(t-1)][0]
	
	// index of the X3th element of Data[X3][0][0]
	// used to mask the channel number in a NRF packet
	uint16_t  MaskIndexUsb;
	
	// index of the Y3th element of Data[X3][Y3][0]
	// used to mask the channel number in a NRF packet
	uint16_t  MaskIndexNrf;
	
	// enable the masking the Data[X3][Y3][0] Nref packet
	volatile uint8_t   MaskEnable;
}ElectrophyData_USB;


// handler of the buffer and its pointers
typedef struct
{
	// Cuffer of SIZE_BUFFER of NRF_DAC_NRF NRF frames
	// each Nrf frame contain DAC_FRAME Dac frame 
	uint16_t Data[SIZE_BUFFER][NRF_DAC_FRAME][DAC_FRAME]; 
		
	// index of the X2th element of Data[X1][X2][0]
	// used to send a USB packet form the buffer to the USB periph
	uint16_t	ReadIndexNrf; //X1
	uint16_t	ReadIndexDac; //X2 

	// index of the Y2th element of Data[Y1][0][0]
	// used to write a NRF packet to the buffer
	uint16_t	WriteIndexNrf; //Y1	
}ElectrophyData_DAC;


/**************************************************************/
//					Functions, public and static
/**************************************************************/

// Initialize the buffer and its pointers
void ElectrophyData_Init(void);

// Check how many buffer frames are ready to be send by USB
uint16_t ElectrophyData_Checkfill(void);
//static uint16_t ElectrophyData_Checkfill_Usb(void);
//static uint16_t ElectrophyData_Checkfill_Dac(void);

// function called to manage the writing of ONE Nrf buffer to the buffer
uint16_t * ElectrophyData_WriteNrf(void);
//static uint16_t * ElectrophyData_WriteNrfUSB(void);
//static uint16_t * ElectrophyData_WriteNrfDAC(void);	

// function called to manage the reading of one USB buffer 
uint16_t * ElectrophyData_ReadUSB(void);
uint16_t  * ElectrophyData_ReadDAC(void);

// function called to manage the masking of a NRF packet
void ElectrophyData_ApplyMask (void);

// USB frame test functions   
void ElectrophyData_RefreshTest(void);
uint16_t * ElectrophyData_TestUsb(void);

 #endif
















