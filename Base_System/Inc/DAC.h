#ifndef __DAC_H__
#define __DAC_H__

#include <stdint.h>
#include "stdint.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_spi.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_cortex.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_hal_tim_ex.h"
#include "AudioBuffer.h"

#define BYTES_PER_FRAME	   32
#define	DATA_FRAMES				 21
#define CHANNEL_SIZE 			 8

/* GPIO command */
#define LOW		0
#define HIGH	1

typedef struct{
	//SPI port/pin config
	GPIO_TypeDef * PORT_SPI;
	uint16_t PIN_SCK;
	uint16_t PIN_MISO;
	uint16_t PIN_MOSI;
	
	// CSN port/pin config
	GPIO_TypeDef * PORT_CSN;
	uint16_t PIN_CSN;
	
	// SPI config
	uint8_t SPI_FUNCTION;
	SPI_TypeDef * SPI_INSTANCE;
	IRQn_Type SPI_IRQN;
	
	// Buffers
	uint8_t * TX_BUFFER;
	
}DAC_Conf;

/**************************************************************/
// Static functions
/**************************************************************/
// DeInitialize GPIO
//static void GPIODeInit(const DAC_Conf * dac);
// Initialize GPIO
//static void GPIOInit(const DAC_Conf * dac);
// Initialization SPI1 
//static void SpiInit(const DAC_Conf * dac);
// Set the csn pin state
//static void CsnDigitalWrite(const DAC_Conf * dac, uint8_t state);
// Function used to test the spi of the DAC
//static void RegisterInit(const DAC_Conf * dac);
// Transmit data with SPI1 
//static void SpiSend(const DAC_Conf * dac, uint8_t * data, uint8_t length);
// Initialize the timer used to send regularely datas to the DAC
//static void TIM2Init(uint32_t reloadValue, uint16_t prescalerValue);

/**************************************************************/
//Public functions
/**************************************************************/
// Initialize the DAC
void DAC_Init(void);
// Interrupt handler for spi irq handler
void SPI_IRQ_Handler(const DAC_Conf * dac);
// Test the dac (ask for the adress pipe 2
void DAC_Test(const DAC_Conf * dac);


#endif



