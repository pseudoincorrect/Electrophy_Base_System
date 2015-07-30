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
static void Process_Test (uint8_t * ptrBuffer);

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

static uint8_t RxAdressP0[6] 			= {W_REGISTER | RX_ADDR_P0, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7};  // set RX ADDR P0
static uint8_t TxAdress[6]  			= {W_REGISTER | TX_ADDR 	, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7}; // set RX ADDR P0
static uint8_t DisableAutoAck[2] 	= {W_REGISTER | EN_AA 		, 0x00};         // disable auto acknowledgement
static uint8_t RxPayloadSize[2] 	= {W_REGISTER | RX_PW_P0 	, BYTES_PER_FRAME};        // set size payload 32 byte
static uint8_t RxPipe[2] 					= {W_REGISTER | EN_RXADDR , 0x01};       // set pipe enabled (pipe 0)   0b 0000 0001 
static uint8_t RxTxAdressSize[2] 	= {W_REGISTER | SETUP_AW  , 0x01};      // set size adresse RX 3 byte   0b 0000 0001
static uint8_t RfChanel[2] 				= {W_REGISTER | RF_CH 		, 0x02};     // set RF chanel (2.4GHz+2MHz)	  0b 0000 0010  
static uint8_t RfParameter[2] 		= {W_REGISTER | RF_SETUP  , 0x0E};    // set RF parameters (2Mbps, 0dB) 0b 0000 1110
static uint8_t ClearIrqFlag[2]    = {W_REGISTER | STATUS    , 0x70}; // clear IRQ                         0b 1110 0000

//config without CRC
static uint8_t ReceiveMode[2] 		= {W_REGISTER | CONFIG		, 0x33};   // set Receive mode                0b 0011 0011 
static uint8_t TransmitMode[2]    = {W_REGISTER | CONFIG    , 0x52};  // set Transmit mode                0b 0101 0010
//config with CRC
//static uint8_t ReceiveMode[2] 	= {W_REGISTER | CONFIG		, 0x3B};   // set Receive mode                0b 0011 0011 
//static uint8_t TransmitMode[2] = {W_REGISTER | CONFIG    , 0x5A};  // set Transmit mode                0b 0101 0010

static uint8_t FlushRxFifo 				= FLUSH_RX;                       // flush Rx fifo
static uint8_t FlushTxFifo				= FLUSH_TX;                      // flush Tx fifo


// *************************************************************************
// *************************************************************************
// 						Const variables	
// *************************************************************************
// *************************************************************************
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

static uint8_t BufferTest[BYTES_PER_FRAME];
static uint8_t * NrfWritePtr;
static uint8_t Receive = R_RX_PAYLOAD;
static uint8_t Dummy 	 = 0x00;
// **************************************************************
//					ExtiHandler
// **************************************************************
void ExtiHandler(const NRF_Conf * nrf, const NRF_Conf * nrfBackup)
{
  TRANSFERT_FLAG = 1;
  
  //Get the adresse in the buffer where to send the datas
	//NrfWritePtr = ElectrophyData_Write_NRF();
  
  NrfWritePtr = BufferTest;
  
	//load the destination adress in the DMA controler for the next transfert
	nrf->DMA_RX_INSTANCE->M0AR = (uint32_t) (NrfWritePtr+1);
	
	SpiSend(nrfBackup, &FlushRxFifo, 1);
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
  
  Process_Test(NrfWritePtr);
}		

volatile uint16_t Loss, Error, WrongFrame; 
static uint16_t CurrentLoss, CurrentError, CurrentWrongFrame, PreviousIndice = 0;
volatile static uint8_t * ptrTmp;
static uint32_t Count;
// **************************************************************
// 					Process_Test 
// **************************************************************
static void Process_Test(uint8_t * ptrBuffer)
{
  uint8_t i = 0;
  
  ptrTmp = ptrBuffer;
  
  // check if we receive datas frome some other device
  if ((ptrBuffer[0] != 0x0E && ptrBuffer[0] != 0xE0) || (ptrBuffer[1] != 0x0F && ptrBuffer[1] != 0xF0))
  {
    CurrentWrongFrame++;
  }
  else
  {
    // check for corrupted datas
    for(i=0; i < BYTES_PER_FRAME; i++)
    {
      if ((i == 0) && (ptrBuffer[i] != 0x0E) && (ptrBuffer[i] != 0xE0))
          CurrentError++;
      
      __nop();
      
      if ((i == 1) && (ptrBuffer[i] != 0x0F) && (ptrBuffer[i] != 0xF0))
          CurrentError++;
      
      __nop();
      
      if ((i == 2) && (ptrBuffer[i] >= NUMBER_OF_PACKETS))
          CurrentError++;
      
      __nop();
      
      if ((i >= 3) && (ptrTmp[i] != (i + 100)))
          CurrentError++;
    }
    
    // check if we miss one or several packets
    if((ptrBuffer[2] != PreviousIndice+1) && (ptrBuffer[2] != 0))
      CurrentLoss++;;
  }
  
  // update the current Control frame index 
  if(ptrBuffer[2] < NUMBER_OF_PACKETS)
    PreviousIndice = ptrBuffer[2];
  else
  {
    PreviousIndice++;
    if(PreviousIndice >= NUMBER_OF_PACKETS)     
      PreviousIndice = 0;
  }
  
  
  //reset and update loss statistics after the reception of 100 frames
  Count++;
  if (Count >= 50000)
  {
    Loss       = CurrentLoss;
    Error      = CurrentError;
    WrongFrame = CurrentWrongFrame;
    
    CurrentLoss       = 0;
    CurrentError      = 0;
    CurrentWrongFrame = 0;
    Count             = 0;
  }
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


// **************************************************************
//					SpiSend 
// **************************************************************
static void SpiSend(const NRF_Conf * nrf, uint8_t * data, uint8_t length)
{
  uint8_t indexSpi;
	SPI_TypeDef * spi = nrf->SPI_INSTANCE;
	
	CsnDigitalWrite(nrf, LOW); //N_slave select lox
	
	for (indexSpi=0; indexSpi<length; indexSpi++) 
	{
		spi->DR = data[indexSpi]; 						 // write data to be transmitted to the SPI data register
		while( !(spi->SR & SPI_FLAG_TXE)){;} 	// wait until transmit complete
		while( !(spi->SR & SPI_FLAG_RXNE)){;} // wait until receive complete	      
	}
  while( spi->SR & SPI_FLAG_BSY ){;} 		// wait until SPI is not busy anymore  
  CsnDigitalWrite(nrf, HIGH);
}

// **************************************************************
//					SpiSendThenDma 
// **************************************************************
static uint8_t SpiSendThenDma(const NRF_Conf * nrf, uint8_t * data, uint8_t length)
{
  uint8_t indexSpi;
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
	CeDigitalWrite(nrf, LOW);
	
	// sending of the instructions to the NRF
	SpiSend(nrf,	DisableAutoAck, sizeof(DisableAutoAck));
	SpiSend(nrf,	RxPayloadSize, 	sizeof(RxPayloadSize)	);
	SpiSend(nrf,	RxPipe, 				sizeof(RxPipe)				);
	SpiSend(nrf,	RxTxAdressSize,	sizeof(RxTxAdressSize));
	SpiSend(nrf,	RxAdressP0,   	sizeof(RxAdressP0)		);
	SpiSend(nrf,	TxAdress,   		sizeof(TxAdress)			);
	SpiSend(nrf,	RfChanel, 			sizeof(RfChanel)			);
	SpiSend(nrf,	RfParameter, 		sizeof(RfParameter)		);
	SpiSend(nrf,	&FlushRxFifo, 	1											);
	SpiSend(nrf,	&FlushTxFifo, 	1											);
	SpiSend(nrf,	ReceiveMode, 	  sizeof(ReceiveMode)	  );
	SpiSend(nrf,  ClearIrqFlag, 	sizeof(ClearIrqFlag) 	);
	CeDigitalWrite(nrf, HIGH);
}


static uint8_t DataTransmit[BYTES_PER_FRAME + 1];
// **************************************************************
// 					NRF_SendNewState
// **************************************************************
void NRF_SendNewState(uint8_t DataState)
{  
  volatile uint16_t i,j;
  
  ExtiInterruptEnable(LOW);
  
  while(FLAG_PACKET)
  {;}
   
  for(i=1; i < BYTES_PER_FRAME+1; i++)
  {
     DataTransmit[i] = ((uint8_t)  DataState) + (i-1);
  }
  DataTransmit[0] = W_TX_PAYLOAD;
  
  while(TRANSFERT_FLAG) {;}
  
  CeDigitalWrite(&nrf1, LOW);
  
  SpiSend(&nrf1, TransmitMode, sizeof(TransmitMode));
  SpiSend(&nrf1, ClearIrqFlag, sizeof(ClearIrqFlag));
    
  for(i=1; i < 75; i++)
  {
    SpiSend(&nrf1, &FlushTxFifo, 1);
    SpiSend(&nrf1, ClearIrqFlag, sizeof(ClearIrqFlag));
    
    SpiSend(&nrf1,	DataTransmit, sizeof(DataTransmit));
    CeDigitalWrite(&nrf1, HIGH);
    
    while ((HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_2) == 1)) {;}
    
   for(j=1; j < 5000; j++)  // small delay to let the embedded system listening the chanel
      __nop();
  }
  
   SpiSend(&nrf1, &FlushTxFifo, 1);
   SpiSend(&nrf1, ClearIrqFlag, sizeof(ClearIrqFlag));
  
  CeDigitalWrite(&nrf1, LOW);
  RegisterInit(&nrf1);

  ExtiInterruptEnable(HIGH);
}


// **************************************************************
// 					NRF_Test
// **************************************************************
void NRF_Test(const NRF_Conf * nrf)
{
	uint8_t RxAdressP0[4] = {R_REGISTER | RX_ADDR_P0, 0x00, 0x00, 0x00};
 	// Read RX ADDR P0
	SpiSend(nrf, RxAdressP0, sizeof(RxAdressP0)); 
	
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





