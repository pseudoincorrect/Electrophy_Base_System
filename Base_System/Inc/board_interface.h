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

#define  ALL_LEDS 0xFF
#define  NO_LED   0xFE
// Configure the clock for the periphérald
void SystemClock_Config(void);

// Initialise the Leds and push button USR
void Board_Init(void);

// Initialize the GPIO
static void GpioInit(void);

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

uint8_t Board_GetStateUpdate(void);

uint8_t Board_GetStateUpdate(void);

DataStateTypeDef Board_GetState(void);

Output_device_t Board_GetOutput(void);

void Board_ExtiInterruptEnable(uint8_t state);

#endif





