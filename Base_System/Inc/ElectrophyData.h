#ifndef __ElectrophyData_H__
#define __ElectrophyData_H__

#include <stdint.h>
#include <stdlib.h>
#include "stm32f4xx.h"
#include "CommonInclude.h"
#include "FBAR.h"

/**************************************************************/
//					Enum
/**************************************************************/
typedef enum{Dac, Usb} Output_device_t;

/**************************************************************/
//					Structures
/**************************************************************/

// handler of the buffer NRF sample buffer
typedef struct
{
	uint8_t Data[SIZE_BUFFER_NRF][BYTES_PER_FRAME]; 
	
	uint16_t	ReadIndex;
	uint16_t	WriteIndex;
	
}ElectrophyData_NRF;

// handler of the buffer and its pointers for the USB buffer
typedef struct
{
	// a buffer of SIZE_BUFFER usb frames 
  // which countain USB_FRAME nrf frames
	uint16_t Data[SIZE_BUFFER_USB][USB_FRAME][BYTES_PER_FRAME]; 
		
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


// handler of the buffer and its pointers for the DAC buffer
typedef struct
{
	// Cuffer of SIZE_BUFFER of NRF_DAC_NRF NRF frames
	// each Nrf frame contain DAC_FRAME Dac frame 
	uint16_t Data[SIZE_BUFFER_DAC][DAC_FRAME][CHANNEL_NUMBER]; 
		
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

// Initialize the buffers and its pointers
void ElectrophyData_Init(Output_device_t  Output_dev);

// function called to manage the NRF buffer
uint16_t ElectrophyData_Checkfill_NRF(void);
uint8_t * ElectrophyData_Write_NRF(void);
uint8_t * ElectrophyData_Read_NRF(void);

// function called to manage the USB buffer
uint16_t ElectrophyData_Checkfill_USB(void);
//static uint16_t * ElectrophyData_Write_USB(void);
uint16_t * ElectrophyData_Read_USB(void);
	
// function called to manage the DAC buffer
uint16_t ElectrophyData_Checkfill_DAC(void);
//static uint16_t * ElectrophyData_Write_DAC(void);
uint16_t * ElectrophyData_Read_DAC(void);

//process function to decompress datas to output buffer (USB or DAC)
void ElectrophyData_Process(void);
	
#endif
















