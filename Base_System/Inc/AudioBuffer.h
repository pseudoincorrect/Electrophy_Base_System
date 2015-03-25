#ifndef __AUDIOBUFFER_H__
#define __AUDIOBUFFER_H__

#include <stdint.h>
#include "stm32f4xx.h"

#define NRF_FRAME	   32
#define USB_FRAME	 	 4 //4
#define	SIZE_BUFFER	 200

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
	
}AudioBuffer_Handle;

// Initialize the buffer and its pointers
void AudioBuffer_Init(void);

// Check how many buffer frames are ready to be send by USB
uint16_t AudioBuffer_Checkfill(void);

// function called to manage the writing of ONE Nrf buffer 
// to the buffer
uint16_t * AudioBuffer_WriteNrf(void);

// function called to manage the reading of one USB buffer 
uint16_t * AudioBuffer_ReadUsb(void);

// function called to manage the masking of a NRF packet
void AudioBuffer_ApplyMask (void);

void AudioBuffer_RefreshTest(void);

uint16_t * AudioBuffer_TestUsb(void);
 #endif
