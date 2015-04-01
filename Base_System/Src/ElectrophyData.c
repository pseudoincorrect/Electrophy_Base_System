#include "ElectrophyData.h"

// *************************************************************************
// *************************************************************************
// 						static function declarations, see .h file	
// *************************************************************************
// *************************************************************************
static uint16_t ElectrophyData_Checkfill_Usb(void);
static uint16_t ElectrophyData_Checkfill_Dac(void);
static uint16_t * ElectrophyData_WriteNrfUSB(void);
static uint16_t * ElectrophyData_WriteNrfDAC(void);		
	
// *************************************************************************
// *************************************************************************
// 							variables	private and public
// *************************************************************************
// *************************************************************************	
// mask with the number of the channel in the right order 0x0100, 0x0200, 0x0300, ... 
uint16_t ChannelMask[NRF_FRAME];

static ElectrophyData_USB ElectrophyDataUSB;
static ElectrophyData_DAC ElectrophyDataDAC;

// Set which output device we use (DAC or USB)
Output_device_t  Output_device;




// *************************************************************************
// *************************************************************************
// 										Function definitions	
// *************************************************************************
// *************************************************************************

// **************************************************************
//					ElectrophyData_Init
// **************************************************************
void ElectrophyData_Init(void)
{
	Output_device = Dac;
	
	//**********************************
	// Initialization of the USB buffer
	int i,j,k;
	for(i=0; i<SIZE_BUFFER; i++) 
		for(j=2; j<(1 + USB_FRAME); j++) 
			for(k=2; k<(1 + NRF_FRAME); k++)
				ElectrophyDataUSB.Data[i][j][k] = 0x0000;
	
	for(i=0, j=0; i<NRF_FRAME; i++)
	{
		ChannelMask[i] = ( j + 1); // << 8;
		j++;
		if (j > 7) 
			j = 0;
	}
	
	ElectrophyDataUSB.ReadIndexUsb  = 1;
	ElectrophyDataUSB.WriteIndexNrf = 1;
	ElectrophyDataUSB.WriteIndexUsb = 1;
	ElectrophyDataUSB.MaskIndexNrf  = 1;
	ElectrophyDataUSB.MaskIndexUsb  = 1;
	ElectrophyDataUSB.MaskEnable    = 1;
	ElectrophyDataUSB.PreviousWriteIndexNrf = 0;
	ElectrophyDataUSB.PreviousWriteIndexUsb = 0;
	
	//**********************************	
	// Initialization of the DAC buffer
	for(i=0; i<SIZE_BUFFER; i++) 
		for(j=2; j<(1 + NRF_DAC_FRAME); j++) 
			for(k=2; k<(1 + DAC_FRAME); k++)
				ElectrophyDataDAC.Data[i][j][k] = 0x0000;	
	
	ElectrophyDataDAC.ReadIndexDac  = 0;
	ElectrophyDataDAC.ReadIndexNrf  = 0;
	ElectrophyDataDAC.WriteIndexNrf = 0;
}

// **************************************************************
//					ElectrophyData_CheckFill 
// **************************************************************
uint16_t ElectrophyData_Checkfill(void)
{
	if (Output_device == Dac)
		return ElectrophyData_Checkfill_Dac();
	else
		return ElectrophyData_Checkfill_Usb();
}

// **************************************************************
//					ElectrophyData_CheckFill_Usb 
// **************************************************************
static uint16_t ElectrophyData_Checkfill_Usb(void)
{
	int diff = ElectrophyDataUSB.WriteIndexUsb - ElectrophyDataUSB.ReadIndexUsb;
	uint16_t result; 
	
	if (diff > 0)
		result = (uint16_t) diff;
	else if (diff < 0)
		result = (uint16_t) (SIZE_BUFFER - ElectrophyDataUSB.ReadIndexUsb + ElectrophyDataUSB.WriteIndexUsb);
	else
		result = 0;

	return result;
}

// **************************************************************
//					ElectrophyData_CheckFill_Dac 
// **************************************************************
static uint16_t ElectrophyData_Checkfill_Dac(void)
{
	int diff = ElectrophyDataDAC.WriteIndexNrf - ElectrophyDataDAC.ReadIndexNrf;
	uint16_t result; 
	
	if (diff > 0)
		result = (uint16_t) diff;
	else if (diff < 0)
		result = (uint16_t) (SIZE_BUFFER - ElectrophyDataUSB.ReadIndexUsb + ElectrophyDataUSB.WriteIndexUsb);
	else
		result = 0;

	return result;
}

// **************************************************************
//					ElectrophyData_WriteNrf 
// **************************************************************
uint16_t * ElectrophyData_WriteNrf(void)
{
	if (Output_device == Dac)
		return ElectrophyData_WriteNrfDAC();
	else
		return ElectrophyData_WriteNrfUSB();
}

// **************************************************************
//					ElectrophyData_WriteNrfUSB 
// **************************************************************
static uint16_t * ElectrophyData_WriteNrfUSB(void)
{
	// MaskIndexNrf and MaskIndexBsb will countain the index of the
	// last writen Nrf Buffer to apply the mask of the channel number 
	ElectrophyDataUSB.MaskIndexNrf = ElectrophyDataUSB.PreviousWriteIndexNrf;
	ElectrophyDataUSB.MaskIndexUsb = ElectrophyDataUSB.PreviousWriteIndexUsb;
	ElectrophyDataUSB.MaskEnable 	 = 1;
	
	// We keep the index datas of the buffer to be written to return it
	ElectrophyDataUSB.PreviousWriteIndexUsb = ElectrophyDataUSB.WriteIndexUsb;
	ElectrophyDataUSB.PreviousWriteIndexNrf = ElectrophyDataUSB.WriteIndexNrf;
	
	// We increment the buffer indexes fo the nex Nrf write
	ElectrophyDataUSB.WriteIndexNrf++;
	if ( ElectrophyDataUSB.WriteIndexNrf >= USB_FRAME)
	{
		ElectrophyDataUSB.WriteIndexNrf = 0;
		ElectrophyDataUSB.WriteIndexUsb++;
		if (ElectrophyDataUSB.WriteIndexUsb >=  SIZE_BUFFER)
			ElectrophyDataUSB.WriteIndexUsb = 0;
	}			
	// return a pointer to the buffer to be written by the DMA 
	return ElectrophyDataUSB.Data[ElectrophyDataUSB.PreviousWriteIndexUsb][ElectrophyDataUSB.PreviousWriteIndexNrf];
}

// **************************************************************
//					ElectrophyData_WriteNrfDAC 
// **************************************************************
static uint16_t * ElectrophyData_WriteNrfDAC(void)
{
	static uint16_t PreviousWriteIndexNrf;
	PreviousWriteIndexNrf = ElectrophyDataDAC.WriteIndexNrf;
	
	// We increment the buffer indexes fo the next Nrf write
	ElectrophyDataDAC.WriteIndexNrf++;
	if ( ElectrophyDataDAC.WriteIndexNrf >= SIZE_BUFFER)
		ElectrophyDataDAC.WriteIndexNrf = 0;
				
	// return a pointer to the buffer to be written by the DMA 
	return ElectrophyDataDAC.Data[PreviousWriteIndexNrf][0];
}


// **************************************************************
//					ElectrophyData_ReadUSB
// **************************************************************
uint16_t * ElectrophyData_ReadUSB(void)
{
	static uint16_t previousReadUsb;
	previousReadUsb = ElectrophyDataUSB.ReadIndexUsb;
	
	ElectrophyDataUSB.ReadIndexUsb++;
	
	if(ElectrophyDataUSB.ReadIndexUsb >= SIZE_BUFFER) 
		ElectrophyDataUSB.ReadIndexUsb = 0;	
		
	return ElectrophyDataUSB.Data[previousReadUsb][0];
}

// **************************************************************
//					ElectrophyData_ReadDAC
// **************************************************************
uint16_t * ElectrophyData_ReadDAC(void)
{
	static uint16_t previousReadIndexDac, previousReadIndexNrf;
	
	previousReadIndexNrf = ElectrophyDataDAC.ReadIndexNrf;
	previousReadIndexDac = ElectrophyDataDAC.ReadIndexDac;
	 
	
	ElectrophyDataDAC.ReadIndexDac++;
	if(ElectrophyDataDAC.ReadIndexDac >= NRF_DAC_FRAME)
	{
		ElectrophyDataDAC.ReadIndexDac = 0;
		
		ElectrophyDataDAC.ReadIndexNrf++;
		if(ElectrophyDataDAC.ReadIndexNrf >= SIZE_BUFFER) 
			ElectrophyDataDAC.ReadIndexNrf = 0;	
	}	
	return ElectrophyDataDAC.Data[previousReadIndexNrf][previousReadIndexDac];
}
	
// **************************************************************
//					ElectrophyData_ApplyMask
// **************************************************************
void ElectrophyData_ApplyMask(void)
{
	if (ElectrophyDataUSB.MaskEnable)
	{
		// get the pointer to where we need to apply the channel mask 
		uint16_t * ToMask = ElectrophyDataUSB.Data[ElectrophyDataUSB.MaskIndexUsb][ElectrophyDataUSB.MaskIndexNrf];
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
		ElectrophyDataUSB.MaskEnable = 0;
	}
}


volatile static uint8_t flag_refresh = 1;
uint16_t DataTest[USB_FRAME][NRF_FRAME]; 
uint16_t time = 0, freq = 0, cnt = 0;
// **************************************************************
//					ElectrophyData_RefreshTest
// **************************************************************
void ElectrophyData_RefreshTest(void)
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

// **************************************************************
//					ElectrophyData_TestUsb
// **************************************************************
uint16_t * ElectrophyData_TestUsb(void)
{
	flag_refresh = 1;
	return &(DataTest[0][0]);	
}







