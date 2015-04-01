#ifndef __NRF_H__
#define __NRF_H__

#include <stdint.h>
#include "stdint.h"
#include "nRF24L01.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_spi.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_cortex.h"
#include "ElectrophyData.h"

#define BYTES_PER_FRAME	   32
#define	DATA_FRAMES				 21

/* GPIO command */
#define LOW		0
#define HIGH	1

#define DEBUG 1

typedef struct{
	//SPI port/pin config
	GPIO_TypeDef * PORT_SPI;
	uint16_t PIN_SCK;
	uint16_t PIN_MISO;
	uint16_t PIN_MOSI;
	
	// CSN port/pin config
	GPIO_TypeDef * PORT_CSN;
	uint16_t PIN_CSN;
	
	// CE port/pin config
	GPIO_TypeDef * PORT_CE;
	uint16_t PIN_CE;
	
	// IRQ port/pin config
	GPIO_TypeDef * PORT_IRQ;
	uint16_t PIN_IRQ;
	IRQn_Type IRQ_EXTI_LINE;
	
	// SPI config
	uint8_t SPI_FUNCTION;
	SPI_TypeDef * SPI_INSTANCE;
	
	// DMA config
	DMA_TypeDef * DMA;
	DMA_Stream_TypeDef * DMA_TX_INSTANCE;
	DMA_Stream_TypeDef * DMA_RX_INSTANCE;
	uint32_t DMA_CHANNEL;
	IRQn_Type DMA_IRQ_VEC;
	uint32_t DMA_MASK_IRQ_TX;
	uint32_t DMA_MASK_IRQ_RX;
	
	// Buffers
	uint8_t * TX_BUFFER;
	uint16_t * RX_BUFFER;
	
}NRF_Conf;

/**************************************************************/
// Private functions
/**************************************************************/
// DeInitialize GPIO
static void GPIODeInit(const NRF_Conf * nrf);
// Initialize GPIO
static void GPIOInit(const NRF_Conf * nrf);
// initialization SPI1 
static void SpiInit(const NRF_Conf * nrf);
// initialization DMA1Stream3 
static void DmaInit(const NRF_Conf * nrf);
// set the ce pin state
static void CeDigitalWrite(const NRF_Conf * nrf, uint8_t state);
// set the csn pin state
static void CsnDigitalWrite(const NRF_Conf * nrf, uint8_t state);
// transmit data with SPI1 
static void SpiSend(const NRF_Conf * nrf, uint8_t * data, uint8_t length);
// transmit a command before a DMA transfer
static void SpiSendThenDma(const NRF_Conf * nrf, uint8_t * data, uint8_t length);
// manage the handlers for the external interupt
static void ExtiHandler(const NRF_Conf * nrf, const NRF_Conf * nrfBackup);
//manage the handlers for the end of a DMA transfert
static void DmaHandler(const NRF_Conf * nrf, const NRF_Conf * nrfBackup);
// function used to test the spi of the NRF
static void RegisterInit(const NRF_Conf * nrf);

/**************************************************************/
// public functions
/**************************************************************/
//initialize the NRF
void NRF_Init(void);
//handlers 
void DMA2_Stream2_IRQHandler(void);
void EXTI2_IRQHandler(void);
void DMA1_Stream3_IRQHandler(void);
void EXTI15_10_IRQHandler(void);
// test the nrf (ask for the adress pipe 2
void NRF_Test(const NRF_Conf * nrf);

#endif









