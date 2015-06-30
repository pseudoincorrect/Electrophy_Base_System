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
#define  DELTA_ADC 5

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

// call back the Timer interrupt
void TIM3_IRQHandler(void);

// call back the Push Button interrupt
void EXTI0_IRQHandler(void);

// Control respectively the top bottom left rigth leds 
static void Leds(uint8_t haut, uint8_t bas, uint8_t gauche, uint8_t droite);

// ligth the led in function of state
void Board_Leds(uint8_t  state, Output_device_t output);

// blink all the leds
void Board_LedsBlink(DataStateTypeDef CurrentState, Output_device_t output);

// check if the push button was push or the pot turned
uint8_t Board_CheckUpdate(void);

// get which updtade is set (output, state, or eta)
uint8_t Board_GetUpdate(void);

// enable timer and EXTI interrupt in fuction of the state 
void Board_Interrupt(uint8_t input, DataStateTypeDef CurrentState);

// get a value of eta between 0 and  100 in function of the Adc_pot
uint8_t Board_GetEtaIndex(void);

#endif





