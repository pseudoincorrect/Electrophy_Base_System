#include "DAC.h"

// *************************************************************************
// *************************************************************************
// 						static function declaration, see .h file	
// *************************************************************************
// *************************************************************************
static void GPIODeInit(const DAC_Conf * dac);
static void GPIOInit(const DAC_Conf * dac);
static void SpiInit(const DAC_Conf * dac);
static void CsnDigitalWrite(const DAC_Conf * dac, uint8_t state);
static void SpiSend(const DAC_Conf * dac, uint32_t * data);
static void RegisterInit(const DAC_Conf * dac);
static void TIM2Init(uint32_t reloadValue, uint16_t prescalerValue);
static void SPI_IRQ_Handler(const DAC_Conf * dac);
static void DAC_Refresh(const DAC_Conf * dac);
static void DAC_SendSample(const DAC_Conf * dac, uint16_t * buffer);

// *************************************************************************
// *************************************************************************
// 						static variables	
// *************************************************************************
// *************************************************************************
//static uint16_t DAC_Channel[CHANNEL_SIZE] = {0, 1, 2, 3, 4, 5, 6, 7};

static uint16_t DAC_ChannelCommand[CHANNEL_SIZE] = { 0, 1, 2, 3, 4, 5, 6, 7 + UPDATE_ALL};

static uint32_t DAC_Empty[CHANNEL_SIZE] 	= {0, 0, 0, 0, 0, 0, 0, 0};

//dac info handler
const DAC_Conf dac1 = {	
		GPIOC, GPIO_PIN_10, GPIO_PIN_11, GPIO_PIN_12, // SCK, MISO, MOSI
		GPIOD, GPIO_PIN_0, // CSN
		GPIO_AF6_SPI3, SPI3, SPI3_IRQn,	//alternate function, spi SPI IRQ
	};

	
// *************************************************************************
// *************************************************************************
// 										Function definitions																 
// *************************************************************************
// *************************************************************************
// **************************************************************
// 	 				DAC_Init 
// **************************************************************
void DAC_Init(void) 
{	
	GPIODeInit(&dac1);
	GPIOInit(&dac1);		
	SpiInit(&dac1);						
	RegisterInit(&dac1); 
	TIM2Init(264, 20); // (230,20) =  20 kHz sample	
}

// **************************************************************
// 	 				DAC_Enable 
// **************************************************************
void DAC_Enable(uint8_t state) 
{	
	if (state)
		 HAL_NVIC_EnableIRQ(TIM2_IRQn);
	else
		 HAL_NVIC_DisableIRQ(TIM2_IRQn);
}

// **************************************************************
//					GPIODeInit 
// **************************************************************
static void GPIODeInit(const DAC_Conf * dac) 
{
	HAL_GPIO_DeInit(dac->PORT_SPI, dac->PIN_SCK);
	HAL_GPIO_DeInit(dac->PORT_SPI, dac->PIN_MOSI);
	HAL_GPIO_DeInit(dac->PORT_SPI, dac->PIN_MISO);
	HAL_GPIO_DeInit(dac->PORT_CSN, dac->PIN_CSN);
}

// **************************************************************
//					GPIOInit 
// **************************************************************
static void GPIOInit(const DAC_Conf * dac) 
{
	//init structures for the config
  GPIO_InitTypeDef GPIO_InitStructure;
	
	// configure pins used by SPI : SCK, MISO, MOSI	
	GPIO_InitStructure.Mode 		 = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Speed 		 = GPIO_SPEED_FAST ;
	GPIO_InitStructure.Pull 		 = GPIO_NOPULL;
	GPIO_InitStructure.Alternate = dac->SPI_FUNCTION; 
	
	GPIO_InitStructure.Pin 	= dac->PIN_SCK;
	HAL_GPIO_Init(dac->PORT_SPI, &GPIO_InitStructure);	
	GPIO_InitStructure.Pin 	= dac->PIN_MISO;
	HAL_GPIO_Init(dac->PORT_SPI, &GPIO_InitStructure);
	GPIO_InitStructure.Pin 	= dac->PIN_MOSI;
	HAL_GPIO_Init(dac->PORT_SPI, &GPIO_InitStructure);
	
	// Configure the chip SELECT pin : CSN
	GPIO_InitStructure.Pin 	= dac->PIN_CSN;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(dac->PORT_CSN, &GPIO_InitStructure);	
	CsnDigitalWrite(dac, HIGH); // set PAD6 HIGH
}
	
//init structures for the config 
static SPI_HandleTypeDef SpiHandle;
// **************************************************************
//					SpiInit
// **************************************************************
static void SpiInit(const DAC_Conf * dac)
{			
	// Set the SPI parameters 
  SpiHandle.Instance               = dac->SPI_INSTANCE;
  SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
  SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
  SpiHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
  SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
  SpiHandle.Init.CRCPolynomial     = 7;
  SpiHandle.Init.DataSize          = SPI_DATASIZE_16BIT;
  SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
  SpiHandle.Init.NSS               = SPI_NSS_SOFT;
  SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLED;
	SpiHandle.Init.Mode 						 = SPI_MODE_MASTER;
	
	HAL_SPI_Init(&SpiHandle);
	
	// enable SPI2 global interrupt
  HAL_NVIC_SetPriority(dac->SPI_IRQN, 2, 0);
	HAL_NVIC_DisableIRQ(dac->SPI_IRQN);
	
	//Enable SPI	 
	dac->SPI_INSTANCE->CR1 |= SPI_CR1_SPE;
}	

static TIM_HandleTypeDef    TimHandle;
/**************************************************************/
//					TIM2Init
/**************************************************************/
static void TIM2Init(uint32_t reloadValue, uint16_t prescalerValue)
{	
	TimHandle.Instance = TIM2;

	TimHandle.Init.Period            = reloadValue;
  TimHandle.Init.Prescaler         = prescalerValue;
  TimHandle.Init.ClockDivision     = 0;
  TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  TimHandle.Init.RepetitionCounter = 0;
 	HAL_TIM_Base_Init(&TimHandle);
  
	// Set the TIMx priority 
	HAL_NVIC_SetPriority(TIM2_IRQn, 2, 0);	
	// Enable the TIMx global Interrupt 
  HAL_NVIC_DisableIRQ(TIM2_IRQn);
	
	__HAL_TIM_ENABLE_IT(&TimHandle, TIM_IT_UPDATE); //The specified TIM3 interrupt : update
  __HAL_TIM_ENABLE(&TimHandle);
}

// **************************************************************
// 					CsnDigitalWrite 
// **************************************************************
static void CsnDigitalWrite(const DAC_Conf * dac, uint8_t state)
{
	if (state)  dac->PORT_CSN->BSRRL |= dac->PIN_CSN; 
	else 			  dac->PORT_CSN->BSRRH |= dac->PIN_CSN;
}

// **************************************************************
//					SpiSend 
// **************************************************************
static void SpiSend(const DAC_Conf * dac, uint32_t * data)
{
  static uint8_t i;
  
	SPI_TypeDef * spi = dac->SPI_INSTANCE;
	
	CsnDigitalWrite(dac, LOW); //N_slave select low
  
  dac->SPI_INSTANCE->DR = *data >> 16;  // write data to be transmitted to the SPI data register
  
  while( !(spi->SR & SPI_FLAG_TXE)  ); 	// wait until transmit complete
   
  dac->SPI_INSTANCE->DR = *data & 0xFFFF; 
	
  while( spi->SR & SPI_FLAG_BSY ); 		// wait until SPI is not busy anymore
	
	CsnDigitalWrite(dac, HIGH);
}

// **************************************************************
// 					RegisterInit 
// **************************************************************
static void RegisterInit(const DAC_Conf * dac)
{
	// disable auto acknowledgement
	static uint32_t POWER_REF = 0x00000000 | (1<<27) | (1<<24) | (1<<19) | (1<<17);  // ref always powered On
    
	// sending of the instructions to the DAC
	SpiSend(dac, &POWER_REF);
  
}

/**************************************************************/
//					TIM2_IRQHandler
/**************************************************************/
void TIM2_IRQHandler(void)
{	
	if(__HAL_TIM_GET_FLAG(&TimHandle, TIM_FLAG_UPDATE) != RESET)
	{
		if(__HAL_TIM_GET_ITSTATUS(&TimHandle, TIM_IT_UPDATE) !=RESET)
		{
			DAC_Refresh(&dac1);
			__HAL_TIM_CLEAR_IT(&TimHandle, TIM_IT_UPDATE); // Remove TIMx update interrupt flag 
			__HAL_TIM_CLEAR_FLAG(&TimHandle, TIM_IT_UPDATE);
		}	
	}
}

static uint16_t * SampleData;
// **************************************************************
// 					DAC_Refresh
// **************************************************************
static void DAC_Refresh(const DAC_Conf * dac)
{
	if (ElectrophyData_Checkfill_DAC())
	{
		SampleData = ElectrophyData_Read_DAC();
		DAC_SendSample(&dac1, SampleData);
	}
	else 
		SpiSend(&dac1, DAC_Empty);
}

static uint8_t dataCnt;
static uint32_t dataSpi;
static uint16_t * ptrChannelCommand;
static uint16_t * bufferSample;
/**************************************************************/
//	 				DAC_SendSample
/**************************************************************/
static void DAC_SendSample(const DAC_Conf * dac, uint16_t * buffer)
{
	//get the pointer to the data to send
	bufferSample = buffer;
	//get the pointer to the channel number array
	ptrChannelCommand = DAC_ChannelCommand;
	//set the ammount of data to send
	dataCnt = CHANNEL_SIZE;
	// enable interrupt : the sampling is done in the interrupt handler
	dac->SPI_INSTANCE->CR2 |= SPI_IT_TXE;
	HAL_NVIC_EnableIRQ(dac->SPI_IRQN);
}

// *************************************************************
// 	 				SPI3_IRQHandler
// *************************************************************
void SPI3_IRQHandler()
{
	SPI_IRQ_Handler(&dac1);
}

/**************************************************************/
//					SPI_IRQ_Handler
/**************************************************************/
static void SPI_IRQ_Handler(const DAC_Conf * dac)
{
	while(dac->SPI_INSTANCE->SR & SPI_FLAG_BSY);
	
	// If buffer TX empty, load next data
	if (dac->SPI_INSTANCE->SR & SPI_FLAG_TXE)
	{	
		if (dataCnt)
		{	
			CsnDigitalWrite(dac, HIGH); 
			dataSpi = (*ptrChannelCommand++ << 20) + (*bufferSample++ << 4) ;
			CsnDigitalWrite(dac, LOW); 
			
      dac->SPI_INSTANCE->DR = dataSpi >> 16;
      while( !(dac->SPI_INSTANCE->SR & SPI_FLAG_TXE)  ); 	// wait until transmit complete
      dac->SPI_INSTANCE->DR = dataSpi & 0x0000FFFF;
      
			dataCnt--;
		}
		else
		{
			CsnDigitalWrite(dac, HIGH); 
			HAL_NVIC_DisableIRQ(dac->SPI_IRQN);
		}
	}
	dac->SPI_INSTANCE->SR &=  ~(SPI_IT_TXE); // clear flag won't trigger directly an other interrupt
}

















