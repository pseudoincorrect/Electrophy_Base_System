#include "ElectrophyData.h"

// *************************************************************************
// *************************************************************************
// 						static function declarations, see .h file	
// *************************************************************************
// *************************************************************************
static uint16_t * ElectrophyData_Write_USB(void);
static uint16_t * ElectrophyData_Write_DAC(void);		
	
// *************************************************************************
// *************************************************************************
// 							variables	private and public
// *************************************************************************
// *************************************************************************	
uint16_t ChannelMask[BYTES_PER_FRAME]; // mask with the number of the channel in the right order 0x0100, 0x0200, 0x0300, ... 

static ElectrophyData_NRF ElectrophyDataNRF;
static ElectrophyData_USB ElectrophyDataUSB;
static ElectrophyData_DAC ElectrophyDataDAC;

Output_device_t  Output_device; // Set which output device we use (DAC or USB)

// *************************************************************************
// *************************************************************************
// 										Function definitions	
// *************************************************************************
// *************************************************************************

// **************************************************************
//					ElectrophyData_Init
// **************************************************************
void ElectrophyData_Init(Output_device_t  Output_dev)
{
	uint16_t i,j,k;
	Output_device = Output_dev;
	
	//**********************************
	// Initialization of the NRF buffer
	for(i=0; i<SIZE_BUFFER; i++) 
		for(j=2; j< BYTES_PER_FRAME; j++) 
			ElectrophyDataNRF.Data[i][j] = 0x0000;	
	
	ElectrophyDataNRF.ReadIndex  = 0;
	ElectrophyDataNRF.WriteIndex = 0;
	
	//**********************************
	// Initialization of the USB buffer
	for(i=0; i<SIZE_BUFFER; i++) 
		for(j=2; j<(1 + USB_FRAME); j++)   
			for(k=2; k<(1 + BYTES_PER_FRAME); k++)
				ElectrophyDataUSB.Data[i][j][k] = 0x0000;
	
	for(i=0, j=0; i<BYTES_PER_FRAME; i++)
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
		for(j=2; j<(1 + NRF_CHANNEL_FRAME); j++) 
			for(k=2; k<(1 + DAC_FRAME); k++)
				ElectrophyDataDAC.Data[i][j][k] = 0x0000;	
	
	ElectrophyDataDAC.ReadIndexDac  = 0;
	ElectrophyDataDAC.ReadIndexNrf  = 0;
	ElectrophyDataDAC.WriteIndexNrf = 0;
}

// *************************************************************************
// 									CheckFill	Functions
// *************************************************************************

// **************************************************************
//					ElectrophyData_CheckFill_NRF
// **************************************************************
uint16_t ElectrophyData_Checkfill_NRF(void)
{
	if (ElectrophyDataNRF.WriteIndex == ElectrophyDataNRF.ReadIndex)
		return 0;
	else
		return 1;
}

// **************************************************************
//					ElectrophyData_Checkfill_USB 
// **************************************************************
uint16_t ElectrophyData_Checkfill_USB(void)
{
	if (ElectrophyDataUSB.WriteIndexUsb == ElectrophyDataUSB.ReadIndexUsb)
		return 0;
	else
		return 1;
}

// **************************************************************
//					ElectrophyData_Checkfill_DAC 
// **************************************************************
uint16_t ElectrophyData_Checkfill_DAC(void)
{
	if (ElectrophyDataDAC.WriteIndexNrf == ElectrophyDataDAC.ReadIndexNrf)
		return 0;
	else
		return 1;
}

// *************************************************************************
// 									Write	Functions
// *************************************************************************

// **************************************************************
//					ElectrophyData_Write_NRF 
// **************************************************************
uint8_t * ElectrophyData_Write_NRF(void)
{
	ElectrophyDataNRF.WriteIndex++;
	
	if (ElectrophyDataNRF.WriteIndex >= SIZE_BUFFER)
		ElectrophyDataNRF.WriteIndex = 0;
	
	return ElectrophyDataNRF.Data[ElectrophyDataNRF.WriteIndex];
}

// **************************************************************
//					ElectrophyData_Write_USB 
// **************************************************************
static uint16_t * ElectrophyData_Write_USB(void)
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
//					ElectrophyData_Write_DAC 
// **************************************************************
static uint16_t * ElectrophyData_Write_DAC(void)
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

// *************************************************************************
// 									Read Functions
// *************************************************************************

// **************************************************************
//					ElectrophyData_Read_NRF
// **************************************************************
uint8_t * ElectrophyData_Read_NRF(void)
{
	static uint16_t previousReadNRF;
	previousReadNRF = ElectrophyDataNRF.ReadIndex;
	
	ElectrophyDataNRF.ReadIndex++;
	
	if(ElectrophyDataNRF.ReadIndex >= SIZE_BUFFER)
		ElectrophyDataNRF.ReadIndex = 0;
	
	return ElectrophyDataNRF.Data[ElectrophyDataNRF.ReadIndex];
}

// **************************************************************
//					ElectrophyData_Read_USB
// **************************************************************
uint16_t * ElectrophyData_Read_USB(void)
{
	static uint16_t previousReadUsb;
	previousReadUsb = ElectrophyDataUSB.ReadIndexUsb;
	
	ElectrophyDataUSB.ReadIndexUsb++;
	
	if(ElectrophyDataUSB.ReadIndexUsb >= SIZE_BUFFER) 
		ElectrophyDataUSB.ReadIndexUsb = 0;	
		
	return ElectrophyDataUSB.Data[previousReadUsb][0];
}

// **************************************************************
//					ElectrophyData_Read_DAC
// **************************************************************
uint16_t * ElectrophyData_Read_DAC(void)
{
	static uint16_t previousReadIndexDac, previousReadIndexNrf;
	
	previousReadIndexNrf = ElectrophyDataDAC.ReadIndexNrf;
	previousReadIndexDac = ElectrophyDataDAC.ReadIndexDac;
	 
	
	ElectrophyDataDAC.ReadIndexDac++;
	if(ElectrophyDataDAC.ReadIndexDac >= NRF_CHANNEL_FRAME)
	{
		ElectrophyDataDAC.ReadIndexDac = 0;
		
		ElectrophyDataDAC.ReadIndexNrf++;
		if(ElectrophyDataDAC.ReadIndexNrf >= SIZE_BUFFER) 
			ElectrophyDataDAC.ReadIndexNrf = 0;	
	}	
	return ElectrophyDataDAC.Data[previousReadIndexNrf][previousReadIndexDac];
}
	
// *************************************************************************
// 									Data Process Functions
// *************************************************************************
void ElectrophyData_Process(void)
{
	if (ElectrophyData_Checkfill_NRF())
	{
		uint16_t i,j;
		if (COMPRESS)
		{	
			if(Output_device == Usb)
				FBAR_Process(ElectrophyData_Read_NRF(), ElectrophyData_Write_USB());
			else
				FBAR_Process(ElectrophyData_Read_NRF(), ElectrophyData_Write_DAC());
		}
		else
		{
			if(Output_device == Usb)
				FBAR_Assemble(ElectrophyData_Read_NRF(), ElectrophyData_Write_USB());
			else
				FBAR_Assemble(ElectrophyData_Read_NRF(), ElectrophyData_Write_DAC());
		}
	}
}



