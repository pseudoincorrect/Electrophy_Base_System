#ifndef __ElectrophyData_H__
#define __ElectrophyData_H__

#include <stdint.h>
#include <stdlib.h>
#include "stm32f4xx.h"
#include "CommonInclude.h"
#include "FBAR.h"


/**************************************************************/
//					Structures
/**************************************************************/

// handler of the buffer NRF sample buffer
typedef struct
{
	uint8_t Data[SIZE_BUFFER_NRF + 1][BYTES_PER_FRAME]; 
	
	uint16_t	ReadIndex;
	uint16_t	WriteIndex;
	
}ElectrophyData_NRF;                          

// handler of the buffer and its pointers for the USB buffer
typedef struct
{
	// a buffer of SIZE_BUFFER usb frames 
  // which countain USB_FRAME nrf frames
	uint16_t Data[SIZE_BUFFER_USB + 1][USB_FRAME][BYTES_PER_FRAME]; 
		
	// index of the X1th element of Data[X1][0][0]
	// used to send a USB packet form the buffer to the USB periph
	uint16_t	ReadIndexUsb;

	// index of the X2th element of Data[X2][0][0]
	// used to write a NRF packet to the buffer
	uint16_t	WriteIndexUsb;
	
	// index of the Y2th element of Data[X2][Y2][0]
	// used to write a NRF packet to the buffer
	uint16_t	WriteIndexNrf;
	
}ElectrophyData_USB;


// handler of the buffer and its pointers for the DAC buffer
typedef struct
{
	// Cuffer of SIZE_BUFFER of NRF_DAC_NRF NRF frames
	// each Nrf frame contain DAC_FRAME Dac frame 
	uint16_t Data[SIZE_BUFFER_DAC + 1][DAC_FRAME][CHANNEL_SIZE]; 
		
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
void ElectrophyData_Init(void);

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
uint8_t  ElectrophyData_Process(void);

// set the  
void ElectrophyData_SetOutPut(Output_device_t  Output);
void ElectrophyData_SetState(DataStateTypeDef state);
#endif
















