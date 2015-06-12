#include "NRF.h"

// *************************************************************************
// *************************************************************************
// 						static function declaration, see .h file	
// *************************************************************************
// *************************************************************************
static void GPIODeInit(const NRF_Conf * nrf);
static void GPIOInit(const NRF_Conf * nrf);
static void SpiInit(const NRF_Conf * nrf);
static void DmaInit(const NRF_Conf * nrf);
static void CeDigitalWrite(const NRF_Conf * nrf, uint8_t state);
static void CsnDigitalWrite(const NRF_Conf * nrf, uint8_t state);
static void SpiSend(const NRF_Conf * nrf, uint8_t * data, uint8_t length);
static uint8_t SpiSendThenDma(const NRF_Conf * nrf, uint8_t * data, uint8_t length);
static void ExtiHandler(const NRF_Conf * nrf, const NRF_Conf * nrfBackup);
static void DmaHandler(const NRF_Conf * nrf, const NRF_Conf * nrfBackup);
static void RegisterInit(const NRF_Conf * nrf);

// *************************************************************************
// *************************************************************************
// 						static variables	
// *************************************************************************
// *************************************************************************
static uint8_t   Spi1TxBuffer[BYTES_PER_FRAME + 1] = {0}; // buffer for Dma TX
static uint16_t  Spi1RxBuffer[BYTES_PER_FRAME + 1] = {0}; // buffer for Dma RX

static uint8_t   Spi2TxBuffer[BYTES_PER_FRAME + 1] = {0}; // buffer for Dma TX
static uint16_t  Spi2RxBuffer[BYTES_PER_FRAME + 1] = {0}; // buffer for Dma RX

volatile static uint8_t TRANSFERT_FLAG = 0;
volatile static uint8_t FLAG_PACKET = 0;  //Flag is set when a transmission packet is alread in process

// Right NRF
const NRF_Conf nrf1 = {	
		GPIOB, GPIO_PIN_3, GPIO_PIN_4, GPIO_PIN_5, // SCK, MISO, MOSI
		GPIOD, GPIO_PIN_6, // CSN
		GPIOD, GPIO_PIN_4, // CE
		GPIOD, GPIO_PIN_2, EXTI2_IRQn,	// IRQ
		GPIO_AF5_SPI1, SPI1,	//alternate function, spi
		DMA2, DMA2_Stream3, DMA2_Stream2, DMA_CHANNEL_3, 			 // DMA, TX, RX, channel
		DMA2_Stream2_IRQn, DMA_FLAG_TCIF3_7, DMA_FLAG_TCIF2_6, // DMA Irqnumber, mask IRQ TX, RX
		Spi1TxBuffer, Spi1RxBuffer
	};

// LeftNRF
const NRF_Conf nrf2 = {	
		GPIOB, GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15, // SCK, MISO, MOSI
		GPIOB, GPIO_PIN_12,	// CSN
		GPIOE, GPIO_PIN_15, // CE
		GPIOE, GPIO_PIN_13, EXTI15_10_IRQn,	// IRQ
		GPIO_AF5_SPI2, SPI2,	//alternate function, spi
		DMA1, DMA1_Stream4, DMA1_Stream3, DMA_CHANNEL_0, 			 // DMA TX, RX, channel
		DMA1_Stream3_IRQn, DMA_FLAG_TCIF0_4, DMA_FLAG_TCIF3_7, // DMA Irqnumber, mask IRQ TX, RX
		Spi2TxBuffer, Spi2RxBuffer
	};

// *************************************************************************
// *************************************************************************
// 										Function definitions																 
// *************************************************************************
// *************************************************************************	
// **************************************************************
// 	 				NRF_Init 
// **************************************************************
void NRF_Init(void) 
{	
	GPIODeInit(&nrf1);
	GPIODeInit(&nrf2);
	
	GPIOInit(&nrf1);		
	SpiInit(&nrf1);				
	DmaInit(&nrf1); 					
	RegisterInit(&nrf1); 
	
	GPIOInit(&nrf2);
	SpiInit(&nrf2);
	DmaInit(&nrf2);
	RegisterInit(&nrf2);
}

// **************************************************************
//					GPIODeInit 
// **************************************************************
static void GPIODeInit(const NRF_Conf * nrf) 
{
	HAL_GPIO_DeInit(nrf->PORT_SPI, nrf->PIN_SCK);
	HAL_GPIO_DeInit(nrf->PORT_SPI, nrf->PIN_MOSI);
	HAL_GPIO_DeInit(nrf->PORT_SPI, nrf->PIN_MISO);
	HAL_GPIO_DeInit(nrf->PORT_CSN, nrf->PIN_CSN);
	HAL_GPIO_DeInit(nrf->PORT_CE, nrf->PIN_CE);
	HAL_GPIO_DeInit(nrf->PORT_IRQ, nrf->PIN_IRQ);
}

// **************************************************************
//					GPIOInit 
// **************************************************************
static void GPIOInit(const NRF_Conf * nrf) 
{
	//init structures for the config
  GPIO_InitTypeDef GPIO_InitStructure;
	
	// configure pins used by SPI1 : SCK, MISO, MOSI	
	GPIO_InitStructure.Mode 		 = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Speed 		 = GPIO_SPEED_FAST ;
	GPIO_InitStructure.Pull 		 = GPIO_NOPULL;
	GPIO_InitStructure.Alternate = nrf->SPI_FUNCTION; 
	
	GPIO_InitStructure.Pin 	= nrf->PIN_SCK;
	HAL_GPIO_Init(nrf->PORT_SPI, &GPIO_InitStructure);	
	GPIO_InitStructure.Pin 	= nrf->PIN_MISO;
	HAL_GPIO_Init(nrf->PORT_SPI, &GPIO_InitStructure);
	GPIO_InitStructure.Pin 	= nrf->PIN_MOSI;
	HAL_GPIO_Init(nrf->PORT_SPI, &GPIO_InitStructure);
	
	// Configure the chip SELECT pin : CSN
	GPIO_InitStructure.Pin 	= nrf->PIN_CSN;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(nrf->PORT_CSN, &GPIO_InitStructure);	
	nrf->PORT_CSN->BSRRL |= nrf->PIN_CSN; // set PAD6 HIGH
	
	// Configure the chip ENABLE pin : CE
	GPIO_InitStructure.Pin  = nrf->PIN_CE;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;;
	HAL_GPIO_Init(nrf->PORT_CE, &GPIO_InitStructure);	
	nrf->PORT_CE->BSRRH |= nrf->PIN_CE; // set PD4 LOW
	
	// Configure the chip IRQ pin EXTI : IRQ
	GPIO_InitStructure.Pin   = nrf->PIN_IRQ;
  GPIO_InitStructure.Mode  = GPIO_MODE_IT_FALLING;
  GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
  GPIO_InitStructure.Pull  = GPIO_NOPULL;
  HAL_GPIO_Init(nrf->PORT_IRQ, &GPIO_InitStructure);
  // Enable and set IRQ_Pin to the highest priority 
  HAL_NVIC_SetPriority(nrf->IRQ_EXTI_LINE, 0x00, 0x00);
  HAL_NVIC_EnableIRQ(nrf->IRQ_EXTI_LINE);
}
	
//init structures for the config 
static SPI_HandleTypeDef SpiHandle;
// **************************************************************
//					SpiInit
// **************************************************************
static void SpiInit(const NRF_Conf * nrf)
{			
	// Set the SPI parameters 
  SpiHandle.Instance               = nrf->SPI_INSTANCE;
  SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
  SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
  SpiHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
  SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
  SpiHandle.Init.CRCPolynomial     = 7;
  SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
  SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
  SpiHandle.Init.NSS               = SPI_NSS_SOFT;
  SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLED;
	SpiHandle.Init.Mode 						 = SPI_MODE_MASTER;
	
	HAL_SPI_Init(&SpiHandle);
	
	//Disable SPI	 
	nrf->SPI_INSTANCE->CR1 &= ~(SPI_CR1_SPE);
}	

static DMA_HandleTypeDef hdma_tx;
static DMA_HandleTypeDef hdma_rx;
// **************************************************************
//					DmaInit
// **************************************************************
static void DmaInit(const NRF_Conf * nrf)
{	
  // Configure the DMA handler for SPI1 TX 
  hdma_tx.Instance                 = nrf->DMA_TX_INSTANCE;
	
	hdma_tx.Init.Channel             = nrf->DMA_CHANNEL;
  hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
  hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
  hdma_tx.Init.Mode                = DMA_NORMAL;
  hdma_tx.Init.Priority            = DMA_PRIORITY_HIGH;
  hdma_tx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;         
  hdma_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
  hdma_tx.Init.MemBurst            = DMA_MBURST_SINGLE;
  hdma_tx.Init.PeriphBurst         = DMA_MBURST_SINGLE;
  
	HAL_DMA_Init(&hdma_tx);   
  		
  // Configure the DMA handler for SPI1 RX 
  hdma_rx.Instance                 = nrf->DMA_RX_INSTANCE;
  
	hdma_rx.Init.Channel             = nrf->DMA_CHANNEL;
  hdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  hdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_rx.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_rx.Init.MemDataAlignment    = DMA_PDATAALIGN_BYTE;
  hdma_rx.Init.Mode                = DMA_NORMAL;
  hdma_rx.Init.Priority            = DMA_PRIORITY_HIGH;
  hdma_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;         
  hdma_rx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
  hdma_rx.Init.MemBurst            = DMA_MBURST_SINGLE;
  hdma_rx.Init.PeriphBurst         = DMA_MBURST_SINGLE; 

	HAL_DMA_Init(&hdma_rx);

	// DMA SPI TX
	nrf->DMA_TX_INSTANCE->NDTR = BYTES_PER_FRAME - 1; 
	nrf->DMA_TX_INSTANCE->M0AR = (uint32_t) nrf->TX_BUFFER; 					// src
	nrf->DMA_TX_INSTANCE->PAR  = (uint32_t) &(nrf->SPI_INSTANCE->DR); // dest
	
	// DMA SPI RX
	nrf->DMA_RX_INSTANCE->NDTR = BYTES_PER_FRAME - 1;
	nrf->DMA_RX_INSTANCE->M0AR = (uint32_t) nrf->RX_BUFFER;					  // dest
	nrf->DMA_RX_INSTANCE->PAR  = (uint32_t) &(nrf->SPI_INSTANCE->DR); // src
	   
	// Set the DMA interupt in the vector interrupt (general) register	 
	HAL_NVIC_SetPriority(nrf->DMA_IRQ_VEC, 0, 0);
	HAL_NVIC_EnableIRQ(nrf->DMA_IRQ_VEC);	 
		 
	// Enable SPI	 
	nrf->SPI_INSTANCE->CR1 |= SPI_CR1_SPE;
	
	// Enable the transfer complete interrupt
	nrf->DMA_RX_INSTANCE->CR |= DMA_IT_TC;
	
	// Clear Dma interrupt
	nrf->DMA->LIFCR  |= (nrf->DMA_MASK_IRQ_TX | nrf->DMA_MASK_IRQ_RX);
	nrf->DMA->HIFCR  |= (nrf->DMA_MASK_IRQ_TX | nrf->DMA_MASK_IRQ_RX); 
}

static uint8_t * NrfWritePtr;
static uint8_t flushRxFifo 				= FLUSH_RX;
static uint8_t ClearIrqFlag[2] = {W_REGISTER | STATUS, 0x70};
static uint8_t Receive = R_RX_PAYLOAD;
static uint8_t Dummy 	 = 0x00;
// **************************************************************
//					ExtiHandler
// **************************************************************
void ExtiHandler(const NRF_Conf * nrf, const NRF_Conf * nrfBackup)
{
  TRANSFERT_FLAG = 1;
  
  //Get the adresse in the buffer where to send the datas
	NrfWritePtr = ElectrophyData_Write_NRF();
  
	//load the destination adress in the DMA controler for the next transfert
	nrf->DMA_RX_INSTANCE->M0AR = (uint32_t) (NrfWritePtr+1);
	
	SpiSend(nrfBackup, &flushRxFifo, 1);
	SpiSend(nrfBackup, ClearIrqFlag, sizeof(ClearIrqFlag) );
	
	// Send read command to the NRF before read through the DMA and keep CSN low
	SpiSendThenDma(nrf, &Receive, 1 );		
	*NrfWritePtr = SpiSendThenDma(nrf, &Dummy, 	1 );	
	
	// Clear Dma interrupt
	nrf->DMA->LIFCR  |= (nrf->DMA_MASK_IRQ_TX | nrf->DMA_MASK_IRQ_RX);
	nrf->DMA->HIFCR  |= (nrf->DMA_MASK_IRQ_TX | nrf->DMA_MASK_IRQ_RX);

	//Enable DMAs, then SPI_DMA, which will start the transfert 
	nrf->DMA_TX_INSTANCE->CR |= DMA_SxCR_EN; 
	nrf->DMA_RX_INSTANCE->CR |= DMA_SxCR_EN;  
	nrf->SPI_INSTANCE->CR2 	 |= (SPI_CR2_RXDMAEN | SPI_CR2_TXDMAEN);  
}

// *************************************************************
// 	 				DmaHandler 
// *************************************************************
void DmaHandler(const NRF_Conf * nrf, const NRF_Conf * nrfBackup)
{
	SPI_TypeDef * spi = nrf->SPI_INSTANCE;
	
	//disable DMA TX/RX SPI1
	spi->CR2 &= ~((uint32_t)(SPI_CR2_RXDMAEN | SPI_CR2_TXDMAEN));
	
	// wait until SPI is not busy anymore	
	while( spi->SR & SPI_FLAG_BSY ); 
	
	CsnDigitalWrite(nrf, HIGH);		 //	Csn => HIGH
		
	//clear IRQ and Transfert complete flag
	SpiSend(nrf, ClearIrqFlag, sizeof(ClearIrqFlag) );	
	
	// Enable the transfer complete interrupt
	nrf->DMA_RX_INSTANCE->CR |= DMA_IT_TC;
	
	// Clear Dma interrupt
	nrf->DMA->LIFCR  |= (nrf->DMA_MASK_IRQ_TX | nrf->DMA_MASK_IRQ_RX);
	nrf->DMA->HIFCR  |= (nrf->DMA_MASK_IRQ_TX | nrf->DMA_MASK_IRQ_RX);
	
  TRANSFERT_FLAG = 0;
	FLAG_PACKET = 0;
}		

// **************************************************************
// 					CeDigitalWrite 
// **************************************************************
void CeDigitalWrite(const NRF_Conf * nrf, uint8_t state)
{
	if (state) 	nrf->PORT_CE->BSRRL |= nrf->PIN_CE;  
	else 				nrf->PORT_CE->BSRRH |= nrf->PIN_CE;	 
}

// **************************************************************
// 					CsnDigitalWrite 
// **************************************************************
void CsnDigitalWrite(const NRF_Conf * nrf, uint8_t state)
{
	if (state)  nrf->PORT_CSN->BSRRL |= nrf->PIN_CSN; 
	else 			  nrf->PORT_CSN->BSRRH |= nrf->PIN_CSN;
}

static uint8_t indexSpi;
// **************************************************************
//					SpiSend 
// **************************************************************
static void SpiSend(const NRF_Conf * nrf, uint8_t * data, uint8_t length)
{
	SPI_TypeDef * spi = nrf->SPI_INSTANCE;
	
	CsnDigitalWrite(nrf, LOW); //N_slave select lox
	
	for (indexSpi=0; indexSpi<length; indexSpi++) 
	{
		spi->DR = data[indexSpi]; 						 // write data to be transmitted to the SPI data register
		while( !(spi->SR & SPI_FLAG_TXE)  ); 	// wait until transmit complete
		while( !(spi->SR & SPI_FLAG_RXNE) ); // wait until receive complete		
	}
  
  while( spi->SR & SPI_FLAG_BSY ); 		// wait until SPI is not busy anymore
	
  CsnDigitalWrite(nrf, HIGH);
}

// **************************************************************
//					SpiSendThenDma 
// **************************************************************
static uint8_t SpiSendThenDma(const NRF_Conf * nrf, uint8_t * data, uint8_t length)
{
	SPI_TypeDef * spi =  nrf->SPI_INSTANCE;
	
	CsnDigitalWrite(nrf, LOW); //N_slave select lox
	
	for (indexSpi=0; indexSpi<length; indexSpi++) 
	{
		spi->DR = data[indexSpi]; 						 // write data to be transmitted to the SPI data register
		while( !(spi->SR & SPI_FLAG_TXE)  ); 	// wait until transmit complete
		while( !(spi->SR & SPI_FLAG_RXNE) ); // wait until receive complete		
	}
  
  while( spi->SR & SPI_FLAG_BSY ); 		// wait until SPI is not busy anymore
	
  return spi->DR;
}

// **************************************************************
// 					RegisterInit 
// **************************************************************
static void RegisterInit(const NRF_Conf * nrf)
{
	//declaration of instruction to send to the NRF
	// disable auto acknowledgement
	uint8_t disableAutoAck[2] 	= {W_REGISTER | EN_AA 		 , 0x00};
	// set size payload 32 byte
	uint8_t rxPayloadSize[2] 	= {W_REGISTER | RX_PW_P0 	 , 32  };
	// set pipe enabled  0b 0000 0001 (pipe 0 enabled)
	uint8_t rxPipe[2] 					= {W_REGISTER | EN_RXADDR  , 0x01};
	// set size adresse RX 3 byte
	uint8_t rxTxAdressSize[2] 	= {W_REGISTER | SETUP_AW   , 0x01};
	// set RX ADDR P0
	uint8_t rxAdressP0[6] 			= {W_REGISTER | RX_ADDR_P0 , 0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
		// set RX ADDR P0
	uint8_t txAdress[6]  			= {W_REGISTER | TX_ADDR 	 , 0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
	// set RF chanel 		 0b 0000 0010 (2400 MHz + 2 MHz) 
	uint8_t rfChanel[2] 				= {W_REGISTER | RF_CH 		 , 0x02};
	// set RF parameters 0b 0000 1110 (2Mbps, 0dB)
	uint8_t rfParameter[2] 		= {W_REGISTER | RF_SETUP   , 0x0E};
	// set Receive mode  0b 0011 0011 
	uint8_t receiveMode[2] 		= {W_REGISTER | CONFIG		 , 0x33};
	// clear IRQ
	uint8_t clearIrq[2] 		  	= {W_REGISTER | STATUS		 , 0x60};
	// flush Rx fifo
	uint8_t flushRxFifo 				= FLUSH_RX;
	// flush Tx fifo
	uint8_t flushTxFifo				= FLUSH_TX;
	
	CeDigitalWrite(nrf, LOW);
	
	// sending of the instructions to the NRF
	SpiSend(nrf,	disableAutoAck, sizeof(disableAutoAck));
	SpiSend(nrf,	rxPayloadSize, 	sizeof(rxPayloadSize)	);
	SpiSend(nrf,	rxPipe, 				sizeof(rxPipe)				);
	SpiSend(nrf,	rxTxAdressSize,	sizeof(rxTxAdressSize));
	SpiSend(nrf,	rxAdressP0,   	sizeof(rxAdressP0)		);
	SpiSend(nrf,	txAdress,   		sizeof(txAdress)			);
	SpiSend(nrf,	rfChanel, 			sizeof(rfChanel)			);
	SpiSend(nrf,	rfParameter, 		sizeof(rfParameter)		);
	SpiSend(nrf,	&flushRxFifo, 	1											);
	SpiSend(nrf,	&flushTxFifo, 	1											);
	SpiSend(nrf,	receiveMode, 	  sizeof(receiveMode)	  );
	SpiSend(nrf, clearIrq, 			sizeof(clearIrq) 			);
	CeDigitalWrite(nrf, HIGH);
}

static uint8_t DataTransmit[BYTES_PER_FRAME + 1] = {0};
// **************************************************************
// 					NRF_SendNewState
// **************************************************************
void NRF_SendNewState(DataStateTypeDef DataState)
{  
  uint8_t i;
  
  uint8_t transmitMode[2] = {W_REGISTER | CONFIG, 0x52};
  uint8_t clearIrqFlag[2] = {W_REGISTER | STATUS, 0x60};
  uint8_t flushTxFifo		 	= FLUSH_TX;
  
  ExtiInterruptEnable(LOW);
  
  while(FLAG_PACKET)
  {;}
  
  DataTransmit[0] = W_TX_PAYLOAD;
  for(i=1; i < BYTES_PER_FRAME+1; i++)
  {
     DataTransmit[i] =  (uint8_t)  DataState;
  }
 
  while(TRANSFERT_FLAG)
  {;}
  
  CeDigitalWrite(&nrf1, LOW);
  
  SpiSend(&nrf1,	transmitMode, sizeof(transmitMode));
 
  SpiSend(&nrf1,	&flushTxFifo, 1);
 
  SpiSend(&nrf1,	clearIrqFlag, sizeof(clearIrqFlag));
    
  for(i=0; i < 20; i++)	
  {  
    SpiSend(&nrf1,	DataTransmit, sizeof(DataTransmit));
    CeDigitalWrite(&nrf1, HIGH);
    while ((HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_2) == 1))
    {;}
    CeDigitalWrite(&nrf1, LOW);    
    SpiSend(&nrf1,	clearIrqFlag, sizeof(clearIrqFlag));
    SpiSend(&nrf1,	&flushTxFifo, 1);
    CeDigitalWrite(&nrf1, HIGH);
  }
   
  CeDigitalWrite(&nrf1, LOW);
  RegisterInit(&nrf1);

  ExtiInterruptEnable(HIGH);
}

// **************************************************************
// 					NRF_Test
// **************************************************************
void NRF_Test(const NRF_Conf * nrf)
{
	uint8_t rxAdressP0[4] = {R_REGISTER | RX_ADDR_P0, 0x00, 0x00, 0x00};
 	// Read RX ADDR P0
	SpiSend(nrf, rxAdressP0, sizeof(rxAdressP0)); 
	
	// Send the commande : read buffer
	SpiSendThenDma(nrf, &Receive, 1 );
	
	// load the destination adress in the DMA controler for the next transfert
	nrf->DMA_RX_INSTANCE->M0AR = (uint32_t) ElectrophyData_Write_NRF(); 

	// Enable the transfer complete interrupt
	nrf->DMA_RX_INSTANCE->CR |= DMA_IT_TC;
	
	// Clear Dma interrupt
	nrf->DMA->LIFCR  |= (nrf->DMA_MASK_IRQ_TX | nrf->DMA_MASK_IRQ_RX);
	nrf->DMA->HIFCR  |= (nrf->DMA_MASK_IRQ_TX | nrf->DMA_MASK_IRQ_RX);
	
	//Enable DMAs, then SPI_DMA, which will start the transfert 
	nrf->DMA_RX_INSTANCE->CR |= DMA_SxCR_EN;  
	nrf->DMA_TX_INSTANCE->CR |= DMA_SxCR_EN;  
	nrf->SPI_INSTANCE->CR2 	 |= (SPI_CR2_RXDMAEN | SPI_CR2_TXDMAEN);
  
	FLAG_PACKET = 1;
}

// **************************************************************
//					EXTI15_10_IRQHandler
// **************************************************************
void EXTI15_10_IRQHandler(void)
{
	if (!FLAG_PACKET)
	{	
		FLAG_PACKET = 1;
		ExtiHandler(&nrf2, &nrf1);
	}
	__HAL_GPIO_EXTI_CLEAR_IT((&nrf2)->PIN_IRQ); //clear exti interrupt
}

// **************************************************************
//					EXTI2_IRQHandler
// **************************************************************
void EXTI2_IRQHandler(void)
{
	if (!FLAG_PACKET)
	{	
		FLAG_PACKET = 1;
		ExtiHandler(&nrf1, &nrf2);
	}
	__HAL_GPIO_EXTI_CLEAR_IT((&nrf1)->PIN_IRQ); //clear exti interrupt
}

// *************************************************************
// 	 				DMA1_Stream3_IRQHandler 
// *************************************************************
void DMA1_Stream3_IRQHandler(void)
{
	DmaHandler(&nrf2, &nrf1);
}		

// *************************************************************
// 	 				DMA2_Stream2_IRQHandler 
// *************************************************************
void DMA2_Stream2_IRQHandler(void)
{
	DmaHandler(&nrf1, &nrf2);	
}		

// *************************************************************
// 	 				ExtiInterruptEnable 
// *************************************************************
void ExtiInterruptEnable(uint8_t state)
{
  if (state) 
  {  
    __HAL_GPIO_EXTI_CLEAR_IT((&nrf1)->PIN_IRQ); //clear exti interrupt
    __HAL_GPIO_EXTI_CLEAR_IT((&nrf2)->PIN_IRQ); //clear exti interrupt
    HAL_NVIC_EnableIRQ(nrf1.IRQ_EXTI_LINE);
    HAL_NVIC_EnableIRQ(nrf2.IRQ_EXTI_LINE);
  }
  else
  {
    HAL_NVIC_DisableIRQ(nrf1.IRQ_EXTI_LINE);
    HAL_NVIC_DisableIRQ(nrf2.IRQ_EXTI_LINE);
  }
}





