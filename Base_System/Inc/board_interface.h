/********************************************************************************
* @file    board_interface.h
* @author  Maxime Clement
* @version V1.0
* @date    01-jun-2015
* @brief   Header file of the custom board user modules.
*******************************************************************************/	

#ifndef __BOARD_INTERFACE_H__
#define __BOARD_INTERFACE_H__

#include "stm32f4xx.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_cortex.h"
#include "CommonInclude.h"
#include "stm32f4xx_it.h"
#include "stm32f4xx_hal_adc.h"

#define  ALL_LEDS 0xFF
#define  NO_LED   0xFE
#define DELTA_ADC 2

// Configure the clock for the periphérald
void SystemClock_Config(void);

// Initialise the Leds and push button USR
void Board_Init(void);

// Initialize the GPIO
static void GpioInit(void);

//Initialize the ADC for the potentiometer
static void AdcInit(void);

//Initialize a periodic interript to check the potentiometer adc
static void TIM3Init(uint32_t reloadValue, uint16_t prescalerValue);

// Control the West/left led 
static void WestLed(uint8_t state);

// Control the North/Top led 
static void NorthLed(uint8_t state);

// Control the Est/Right led 
static void EstLed(uint8_t state);

// Control the South/bottom led 
static void SouthLed(uint8_t state);
	
static void Board_Leds(uint8_t  state);

static void LedsBlink(void);

uint8_t Board_GetUpdate(void);

uint8_t Board_GetUpdate(void);

DataStateTypeDef Board_GetState(void);

Output_device_t Board_GetOutput(void);

void Board_InterruptEnable(uint8_t state);

void TIM3_IRQHandler(void);

uint8_t Board_GetEtaIndex(void);

#endif





