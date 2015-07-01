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
static ElectrophyData_NRF ElectrophyDataNRF;
static ElectrophyData_USB ElectrophyDataUSB;
static ElectrophyData_DAC ElectrophyDataDAC;

static DataStateTypeDef ElectrophyData_State  = FIRST_STATE; 
static Output_device_t  ElectrophyData_Output = FIRST_OUTPUT; // Set which output device we use (DAC or USB)

// debug ptr to the beginning of each buffer
volatile uint8_t  * NRFptr;
volatile uint16_t * USBptr;
volatile uint16_t * DACptr;
// *************************************************************************
// *************************************************************************
// 										Function definitions	
// *************************************************************************
// *************************************************************************

// **************************************************************
//					ElectrophyData_Init
// **************************************************************
void ElectrophyData_Init(uint16_t EtaIndex)
{
	uint16_t i,j,k;
	
	NRFptr = ElectrophyDataNRF.Data[0];
	USBptr = ElectrophyDataUSB.Data[0][0];
	DACptr = ElectrophyDataDAC.Data[0][0];
	
	//**********************************
	// Initialization of the NRF buffer
	for(i=0; i<SIZE_BUFFER_NRF; i++) 
		for(j=0; j< BYTES_PER_FRAME; j++) 
			ElectrophyDataNRF.Data[i][j] = 0x0000;	
	
	ElectrophyDataNRF.ReadIndex  = 0;
	ElectrophyDataNRF.WriteIndex = 0;
	
	//**********************************
	// Initialization of the USB buffer
	for(i=0; i < SIZE_BUFFER_USB; i++) 
		for(j=0; j< USB_FRAME; j++)   
			for(k=0; k < BYTES_PER_FRAME; k++)
				ElectrophyDataUSB.Data[i][j][k] = 0x0000;
	
	ElectrophyDataUSB.ReadIndexUsb  = 0;
	ElectrophyDataUSB.WriteIndexNrf = 0;
	ElectrophyDataUSB.WriteIndexUsb = 0;

	//**********************************	
	// Initialization of the DAC buffer
	for(i=0; i<SIZE_BUFFER_DAC; i++) 
		for(j=2; j<(1 + NRF_CHANNEL_FRAME); j++) 
			for(k=2; k<(1 + DAC_FRAME); k++)
				ElectrophyDataDAC.Data[i][j][k] = 0x0000;	
	
	ElectrophyDataDAC.ReadIndexDac  = 0;
	ElectrophyDataDAC.ReadIndexNrf  = 0;
	ElectrophyDataDAC.WriteIndexNrf = 0;
	
	FBAR_Initialize(EtaIndex);
}

// *************************************************************************
// 								  NRF	Functions
// *************************************************************************

//		uint8_t Data[SIZE_BUFFER_NRF][BYTES_PER_FRAME]

// **************************************************************
//					ElectrophyData_CheckFill_NRF
// **************************************************************
uint16_t ElectrophyData_Checkfill_NRF(void)
{
	if (ElectrophyDataNRF.WriteIndex == ElectrophyDataNRF.ReadIndex ||
     ElectrophyDataNRF.WriteIndex - ElectrophyDataNRF.ReadIndex  == 1 ||
     ElectrophyDataNRF.WriteIndex - ElectrophyDataNRF.ReadIndex  == 2 ||
     ElectrophyDataNRF.ReadIndex  - ElectrophyDataNRF.WriteIndex >= SIZE_BUFFER_NRF - 3 )
		return 0;
	else
		return 1;
}

// **************************************************************
//					ElectrophyData_Write_NRF 
// **************************************************************
uint8_t * ElectrophyData_Write_NRF(void)
{
	ElectrophyDataNRF.WriteIndex++;
	
	if (ElectrophyDataNRF.WriteIndex >= SIZE_BUFFER_NRF)
		ElectrophyDataNRF.WriteIndex = 0;
	
	return ElectrophyDataNRF.Data[ElectrophyDataNRF.WriteIndex];
}

// **************************************************************
//					ElectrophyData_Read_NRF
// **************************************************************
uint8_t * ElectrophyData_Read_NRF(void)
{
	static uint16_t previousReadNRF;
	previousReadNRF = ElectrophyDataNRF.ReadIndex;
	
	ElectrophyDataNRF.ReadIndex++;
	
	if(ElectrophyDataNRF.ReadIndex >= SIZE_BUFFER_NRF)
		ElectrophyDataNRF.ReadIndex = 0;
	
	return ElectrophyDataNRF.Data[previousReadNRF];
}

// *************************************************************************
// 									USB	Functions
// *************************************************************************

// 			uint16_t Data[SIZE_BUFFER_USB][USB_FRAME][BYTES_PER_FRAME]

// **************************************************************
//					ElectrophyData_Checkfill_USB 
// **************************************************************
uint16_t ElectrophyData_Checkfill_USB(void)
{
	if (ElectrophyDataUSB.WriteIndexUsb == ElectrophyDataUSB.ReadIndexUsb ||
      ElectrophyDataUSB.WriteIndexUsb - ElectrophyDataUSB.ReadIndexUsb == 1 ||
      ElectrophyDataUSB.ReadIndexUsb - ElectrophyDataUSB.WriteIndexUsb >= SIZE_BUFFER_USB - 3 )
		return 0;
	else
		return 1;
}

// **************************************************************
//					ElectrophyData_Write_USB 
// **************************************************************
static uint16_t * ElectrophyData_Write_USB(void)
{
	// We increment the buffer indexes fo the nex Nrf write
	ElectrophyDataUSB.WriteIndexNrf++;
	if ( ElectrophyDataUSB.WriteIndexNrf >= USB_FRAME)
	{
		ElectrophyDataUSB.WriteIndexNrf = 0;
		ElectrophyDataUSB.WriteIndexUsb++;
		
		if (ElectrophyDataUSB.WriteIndexUsb >=  SIZE_BUFFER_USB)
			ElectrophyDataUSB.WriteIndexUsb = 1;
	}			
	// return a pointer to the buffer to be written by the DMA 
	return ElectrophyDataUSB.Data[ElectrophyDataUSB.WriteIndexUsb][ElectrophyDataUSB.WriteIndexNrf];
}

// **************************************************************
//					ElectrophyData_Read_USB
// **************************************************************
uint16_t * ElectrophyData_Read_USB(void)
{
	static uint16_t previousReadUsb;
	 previousReadUsb = ElectrophyDataUSB.ReadIndexUsb;
	
	ElectrophyDataUSB.ReadIndexUsb++;
	
	if(ElectrophyDataUSB.ReadIndexUsb >= SIZE_BUFFER_USB) 
		ElectrophyDataUSB.ReadIndexUsb = 1;	
		
	return ElectrophyDataUSB.Data[previousReadUsb][0];
}

// *************************************************************************
// 									DAC	Functions
// *************************************************************************

//				uint16_t Data[SIZE_BUFFER_DAC][DAC_FRAME][CHANNEL_NUMBER] 

// **************************************************************
//					ElectrophyData_Checkfill_DAC 
// **************************************************************
uint16_t ElectrophyData_Checkfill_DAC(void)
{   
	if (ElectrophyDataDAC.WriteIndexNrf == ElectrophyDataDAC.ReadIndexNrf ||
      ElectrophyDataDAC.WriteIndexNrf - ElectrophyDataDAC.ReadIndexNrf == 1 ||
      ElectrophyDataDAC.WriteIndexNrf - ElectrophyDataDAC.ReadIndexNrf == SIZE_BUFFER_DAC - 1)	
    return 0;
	else
		return 1;
}

// **************************************************************
//					ElectrophyData_Write_DAC 
// **************************************************************
static uint16_t * ElectrophyData_Write_DAC(void)
{
	// We increment the buffer indexes fo the next Nrf write
	ElectrophyDataDAC.WriteIndexNrf++;
	
	if (ElectrophyDataDAC.WriteIndexNrf >= SIZE_BUFFER_DAC)
		ElectrophyDataDAC.WriteIndexNrf = 0;
				
	// return a pointer to the buffer to be written to by uncompression function 
	return ElectrophyDataDAC.Data[ElectrophyDataDAC.WriteIndexNrf][0];
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
	
	if(ElectrophyDataDAC.ReadIndexDac >= DAC_FRAME)
	{
		ElectrophyDataDAC.ReadIndexDac = 0;
		
		ElectrophyDataDAC.ReadIndexNrf++;
		
		if(ElectrophyDataDAC.ReadIndexNrf >= SIZE_BUFFER_DAC) 
			ElectrophyDataDAC.ReadIndexNrf = 0;	
	}	
	return ElectrophyDataDAC.Data[previousReadIndexNrf][previousReadIndexDac];
}
	
// *************************************************************************
// 									Data Process Functions
// *************************************************************************

// **************************************************************
//					ElectrophyData_Process
// **************************************************************
uint8_t ElectrophyData_Process(void)
{
	if (ElectrophyData_Checkfill_NRF())
	{
		uint8_t *  FbarReadPtr, * Assemble8Ptr;
		uint16_t * FbarWritePtr,* Assemble16Ptr;
       
		if (ElectrophyData_State == __8ch_3bit__20kHz__C__)  // if compress
		{	
			FbarReadPtr = ElectrophyData_Read_NRF();
			
			//if cutvalues reinitialisation balise
			if (*FbarReadPtr == 0xFF && *(FbarReadPtr + 1) == 0xFF )
        ;//FBAR_Reinitialize((FbarReadPtr + 2));		
			// if no reinitialisation
			else 
			{
				if(ElectrophyData_Output == Usb)
          FbarWritePtr = ElectrophyData_Write_USB();
				else 
					FbarWritePtr = ElectrophyData_Write_DAC();
				
				FBAR_Uncompress(FbarReadPtr, FbarWritePtr);
			}			
		}
		else // !(Compress)
		{
      Assemble8Ptr = ElectrophyData_Read_NRF();
              
      if(ElectrophyData_Output == Usb)
        Assemble16Ptr = ElectrophyData_Write_USB();
			else
        Assemble16Ptr = ElectrophyData_Write_DAC();
      
      FBAR_Assemble(Assemble8Ptr, Assemble16Ptr, ElectrophyData_State);     
           
      if (ElectrophyData_State == __8ch_16bit_10kHz_NC__)
      {
        Assemble8Ptr = ElectrophyData_Read_NRF();
        Assemble16Ptr += (NRF_CHANNEL_FRAME/2) * CHANNEL_SIZE;
        
        FBAR_Assemble(Assemble8Ptr, Assemble16Ptr, ElectrophyData_State);     
      }
    }
	}
	return ElectrophyData_Checkfill_NRF();
}

// **************************************************************
//					ElectrophyData_Reset
// **************************************************************
void ElectrophyData_Reset(Output_device_t Output, DataStateTypeDef State, uint16_t eta)
{
  ElectrophyData_State = State;
  ElectrophyData_Init(eta);
  ElectrophyData_Output = Output;
}  


