#include "board_interface.h"

static DataStateTypeDef DataState = FIRST_STATE;
static Output_device_t Output_device = FIRST_OUTPUT;
static uint8_t FlagUpdate = 0;

// **************************************************************
// 	 				SystemClock_Config 
// **************************************************************
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  __PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);
 
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

}

/**************************************************************/
//					Board_Init
/**************************************************************/
void Board_Init(void)
{
	GpioInit();
	Board_Leds(DataState);
}

/**************************************************************/
//					GpioInit
/**************************************************************/
static void GpioInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

//****************************************************** Push button	
	 /* Configure PA.15 pin as input floating */
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Enable and set EXTI line 0 Interrupt to the lowest priority */
  HAL_NVIC_SetPriority(EXTI0_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
	
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;

//****************************************************** LED's
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}


uint8_t changeOutput = 0, indexState = 0;
DataStateTypeDef stateSystem[4] = {	__8ch_16bit_20kHz__C__, __4ch_16bit_20kHz_NC__ ,
                                    __8ch_16bit_10kHz_NC__, __8ch_8bit__20kHz_NC__};
/**************************************************************/
//					EXTI0_IRQHandler
/**************************************************************/
void EXTI0_IRQHandler(void)
{
  static uint32_t ticksIn;
  
	if (EXTI->PR & EXTI_PR_PR0)
	{
    
    DEBUG2_HIGH;
    
    ticksIn = It_getTicks();   
    while(It_getTicks() - ticksIn < 250){;} 
    
    ticksIn = It_getTicks();  
    while((GPIOA->IDR & GPIO_PIN_0))
    {
     if (It_getTicks() - ticksIn > 850)
      {
        changeOutput = 1;
        break;
      }
    }
    
    if (!changeOutput)
    {
      indexState = (indexState >= 2) ? 0 : indexState+1;
      Board_Leds((uint8_t) stateSystem[indexState]);   
      DataState = stateSystem[indexState];     
	  }
    else
    {
      LedsBlink();
      if(Output_device == Dac)
        Output_device = Usb;
      else
        Output_device = Dac;
    }
    changeOutput = 0;
    FlagUpdate = 1;
    
    while((GPIOA->IDR & GPIO_PIN_0))
    {;}
  }
  
  DEBUG2_LOW;
	
  EXTI->PR = EXTI_PR_PR0;
}

/**************************************************************/
//					WestLed
/**************************************************************/	
static void WestLed(uint8_t state)
{
  if (state)  GPIOD->BSRRL |= GPIO_PIN_12; 
	else 			  GPIOD->BSRRH |= GPIO_PIN_12;
}

/**************************************************************/
//					NorthLed
/**************************************************************/	
static void NorthLed(uint8_t state)
{
  if (state)  GPIOD->BSRRL |= GPIO_PIN_13; 
	else 			  GPIOD->BSRRH |= GPIO_PIN_13;
}

/**************************************************************/
//					EstLed
/**************************************************************/	
static void EstLed(uint8_t state)
{
  if (state)  GPIOD->BSRRL |= GPIO_PIN_14; 
	else 			  GPIOD->BSRRH |= GPIO_PIN_14;
}

/**************************************************************/
//					SouthLed
/**************************************************************/	
static void SouthLed(uint8_t state)
{
  if (state)  GPIOD->BSRRL |= GPIO_PIN_15; 
	else 			  GPIOD->BSRRH |= GPIO_PIN_15;
}

/**************************************************************/
//					Board_Leds
/**************************************************************/
static void Board_Leds(uint8_t  state)
{
	switch(state)
	{
		case ((uint8_t) __8ch_16bit_20kHz__C__) :
			WestLed(LOW);
			NorthLed(HIGH);
      EstLed(LOW);
      SouthLed(LOW);
			break;
		case ((uint8_t) __4ch_16bit_20kHz_NC__) :
			WestLed(LOW);
			NorthLed(LOW);
      EstLed(HIGH);
      SouthLed(LOW);
			break;
		case ((uint8_t) __8ch_16bit_10kHz_NC__) :
			WestLed(LOW);
			NorthLed(LOW);
      EstLed(LOW);
      SouthLed(HIGH);
			break;
		case ((uint8_t) __8ch_8bit__20kHz_NC__ ) :
      WestLed(HIGH);
			NorthLed(LOW);
      EstLed(LOW);
      SouthLed(LOW);
			break;
    case NO_LED :
      WestLed(LOW);
			NorthLed(LOW);
      EstLed(LOW);
      SouthLed(LOW);
			break;
    case ALL_LEDS :
      WestLed(HIGH);
			NorthLed(HIGH);
      EstLed(HIGH);
      SouthLed(HIGH);
			break;
    default :
      break;
	}		
}

 /**************************************************************/
//					Leds_Blink
/**************************************************************/
static void LedsBlink(void)
{
  static uint8_t i;
  static uint32_t ticksIn;
  
  Board_Leds(ALL_LEDS);
  
  for(i = 0; i < 4; i++)
  {
     ticksIn = It_getTicks();
     Board_Leds(ALL_LEDS);
     while (( It_getTicks() - ticksIn) < 100)
     {;}
       
     ticksIn = It_getTicks();
     Board_Leds(NO_LED);
     while ((It_getTicks() - ticksIn) < 200)
     {;}  
  }
  Board_Leds((uint8_t) DataState); 
}  

/**************************************************************/
//					Board_GetStateUpdate
/**************************************************************/
uint8_t Board_GetStateUpdate(void)
{
  if (FlagUpdate)
  {
    FlagUpdate = 0;
    return HIGH;
  }
  else 
    return LOW;
}

/**************************************************************/
//					Board_GetState
/**************************************************************/
DataStateTypeDef Board_GetState(void)
{
  return DataState;
}

/**************************************************************/
//					Board_GetOutput
/**************************************************************/
Output_device_t Board_GetOutput(void)
{
  return Output_device;
}






