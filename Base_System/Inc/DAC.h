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
#include "ElectrophyData.h"
#include "CommonInclude.h"

#define UPDATE_ALL  32

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
// Transmit data with SPI1 
//static void SpiSend(const DAC_Conf * dac, uint32_t * data);
// Function used to set the register of the DAC 
//static void RegisterInit(const DAC_Conf * dac);
// Initialize the timer used to send regularely datas to the DAC
//static void TIM2Init(uint32_t reloadValue, uint16_t prescalerValue);
// Interrupt handler for spi irq handler
//static void SPI_IRQ_Handler(const DAC_Conf * dac);
// called by the SPI IRQ
//static void SPI_IRQ_Handler(const DAC_Conf * dac);
// Manage the refreshing of the DAC output
//static void DAC_Refresh(const DAC_Conf * dac, uint8_t * buffer);
// send By SPI and interrupt data to the DAC output register
//static void DAC_SendSample(const DAC_Conf * dac, uint16_t * buffer);

/**************************************************************/
//Public functions
/**************************************************************/
// Initialize the DAC
void DAC_Init(void);
// enable DAC refreshing
void DAC_Enable(uint8_t state);
// handler for Timer and Spi interrupts
void TIM2_IRQHandler(void);
void SPI3_IRQHandler(void);

#endif


