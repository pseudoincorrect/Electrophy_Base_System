#include "AudioBuffer.h"
	
// mask with the number of the channel in the right order 0x0100, 0x0200, 0x0300, ... 
uint16_t ChannelMask[NRF_FRAME];

AudioBuffer_Handle AudioBuffer;

// **************************************************************
//					AudioBuffer_Init
// **************************************************************
void AudioBuffer_Init(void)
{
	int i,j,k;
	for(i=0; i<SIZE_BUFFER; i++) 
		for(j=2; j<(1 + USB_FRAME); j++) 
			for(k=2; k<(1 + NRF_FRAME); k++)
				AudioBuffer.Data[i][j][k] = 0x00;
	
	for(i=0, j=0; i<NRF_FRAME; i++)
	{
		ChannelMask[i] = ( j + 1); // << 8;
		j++;
		if (j > 7) 
			j = 0;
	}
	
	AudioBuffer.ReadIndexUsb  = 1;
	AudioBuffer.WriteIndexNrf = 1;
	AudioBuffer.WriteIndexUsb = 1;
	AudioBuffer.MaskIndexNrf  = 1;
	AudioBuffer.MaskIndexUsb  = 1;
	AudioBuffer.MaskEnable    = 1;
	AudioBuffer.PreviousWriteIndexNrf = 0;
	AudioBuffer.PreviousWriteIndexUsb = 0;
}

// **************************************************************
//					AudioBuffer_CheckFill 
// **************************************************************
uint16_t AudioBuffer_Checkfill(void)
{
	int diff = AudioBuffer.WriteIndexUsb - AudioBuffer.ReadIndexUsb;
	uint16_t result; 
	
	if (diff > 0)
		result = (uint16_t) diff;
	else if (diff < 0)
		result = (uint16_t) (SIZE_BUFFER - AudioBuffer.ReadIndexUsb + AudioBuffer.WriteIndexUsb);
	else
		result = 0;

	return result;
}

// **************************************************************
//					AudioBuffer_WriteNrf 
// **************************************************************
uint16_t * AudioBuffer_WriteNrf(void)
{
	// MaskIndexNrf and MaskIndexBsb will countain the index of the
	// last writen Nrf Buffer to apply the mask of the channel number 
	AudioBuffer.MaskIndexNrf = AudioBuffer.PreviousWriteIndexNrf;
	AudioBuffer.MaskIndexUsb = AudioBuffer.PreviousWriteIndexUsb;
	AudioBuffer.MaskEnable 	 = 1;
	
	// We keep the index datas of the buffer to be written to return it
	AudioBuffer.PreviousWriteIndexUsb = AudioBuffer.WriteIndexUsb;
	AudioBuffer.PreviousWriteIndexNrf = AudioBuffer.WriteIndexNrf;
	
	// We increment the buffer indexes fo the nex Nrf write
	AudioBuffer.WriteIndexNrf++;
	if ( AudioBuffer.WriteIndexNrf >= USB_FRAME)
	{
		AudioBuffer.WriteIndexNrf = 0;
		AudioBuffer.WriteIndexUsb++;
		if (AudioBuffer.WriteIndexUsb >=  SIZE_BUFFER)
			AudioBuffer.WriteIndexUsb = 0;
	}			
	// return a pointer to the buffer to be written by the DMA 
	return AudioBuffer.Data[AudioBuffer.PreviousWriteIndexUsb][AudioBuffer.PreviousWriteIndexNrf];
}

// **************************************************************
//					AudioBuffer_ReadNrf 
// **************************************************************
uint16_t * AudioBuffer_ReadUsb(void)
{
	static uint16_t previousUsb;
	previousUsb = AudioBuffer.ReadIndexUsb;
	
	AudioBuffer.ReadIndexUsb++;
	
	if(AudioBuffer.ReadIndexUsb >= SIZE_BUFFER) 
		AudioBuffer.ReadIndexUsb = 0;	
		
	return AudioBuffer.Data[previousUsb][0];
}


void AudioBuffer_ApplyMask(void)
{
	if (AudioBuffer.MaskEnable)
	{
		// get the pointer to where we need to apply the channel mask 
		uint16_t * ToMask = AudioBuffer.Data[AudioBuffer.MaskIndexUsb][AudioBuffer.MaskIndexNrf];
		// get the pointer to the mask
		uint16_t * Mask = ChannelMask;
		int8_t tmp;
		
		uint8_t i = 0;
		for (i=0; i<NRF_FRAME; i++)
		{	
			tmp = ((*ToMask & 0x00FF) - 128);
			*ToMask  =  tmp << 8;
			*ToMask |= *Mask;
			ToMask++;
			Mask++;
		}
		AudioBuffer.MaskEnable = 0;
	}
}


volatile static uint8_t flag_refresh = 1;
uint16_t DataTest[USB_FRAME][NRF_FRAME]; 
uint16_t time = 0, freq = 0, cnt = 0;

void AudioBuffer_RefreshTest(void)
{
	if(flag_refresh)		
	{	
		uint8_t i, j, cnt;
		for(i = 0; i < USB_FRAME; i++) 
		{		
			for(j = 0; j < NRF_FRAME; j++)
			{
				DataTest[i][j] = time << 8;
				if (cnt>=8)
				{	
					time++;
					cnt = 0;
				}
				if (time > freq)
				{
					time = 0;
					freq++;
					if (freq > 127)
						freq = 0;
				}
			}
			flag_refresh = 0;
		}
	}
}

uint16_t * AudioBuffer_TestUsb(void)
{
	flag_refresh = 1;
	return &(DataTest[0][0]);	
}







