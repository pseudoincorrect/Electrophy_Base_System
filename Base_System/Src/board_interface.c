#include "board_interface.h"

volatile static uint8_t  FlagUpdate, FlagState, FlagOutput, FlagEtaBeta;
static ADC_HandleTypeDef  AdcHandle;
static TIM_HandleTypeDef  TimHandle;

// **************************************************************
// 	 				SystemClock_Config 
// **************************************************************
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

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
  AdcInit();
  TIM3Init(500, 300);
  
  FlagUpdate  = 0;
  FlagState   = 0;
  FlagOutput  = 0;
  FlagEtaBeta = 0;
}

/**************************************************************/
//					GpioInit
/**************************************************************/
static void GpioInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
  
  //****************************************************** LED's
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Pin   = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  
  /* ADC1 Channel1 GPIO pin configuration */
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); 
  
//  //****************************************************** debug pin
//	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//  GPIO_InitStruct.Pull = GPIO_NOPULL;
//  GPIO_InitStruct.Pin =  GPIO_PIN_15 | GPIO_PIN_10 | GPIO_PIN_8;
//  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  //****************************************************** Push button	
  /* Configure PA0 pin as input floating */
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Enable and set EXTI Line0 Interrupt to the lowest priority */
  HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
  
    //****************************************************** Push button BETA	
  /* Configure PA0 pin as input floating */
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* Enable and set EXTI Line0 Interrupt to the lowest priority */
  HAL_NVIC_SetPriority(EXTI1_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);
}

// *************************************************************
// 	 				AdcInit 
// *************************************************************
static void AdcInit(void)
{  
  ADC_ChannelConfTypeDef    sConfig;
  
/*##-1- Configure the ADC peripheral #######################################*/
  AdcHandle.Instance = ADC1;
  
  AdcHandle.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV2;
  AdcHandle.Init.Resolution            = ADC_RESOLUTION8b;
  AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  AdcHandle.Init.ScanConvMode          = DISABLE;                       /* Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1) */
  AdcHandle.Init.EOCSelection          = EOC_SINGLE_CONV;
  AdcHandle.Init.ContinuousConvMode    = DISABLE;                       /* Continuous mode disabled to have only 1 conversion at each conversion trig */
  AdcHandle.Init.DiscontinuousConvMode = DISABLE;                       /* Parameter discarded because sequencer is disabled */
  AdcHandle.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T1_CC1;   /* Software start to trig the 1st conversion manually, without external event */
  AdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
  AdcHandle.Init.DMAContinuousRequests = ENABLE;
      
  HAL_ADC_Init(&AdcHandle);

  /*##-2- Configure ADC regular channel ######################################*/  
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  sConfig.Offset = 0;
  
  HAL_ADC_ConfigChannel(&AdcHandle, &sConfig);
}

/**************************************************************/
//					TIM3Init
/**************************************************************/
static void TIM3Init(uint32_t reloadValue, uint16_t prescalerValue)
{	
	TimHandle.Instance = TIM3;

	TimHandle.Init.Period            = reloadValue;
  TimHandle.Init.Prescaler         = prescalerValue;
  TimHandle.Init.ClockDivision     = 0;
  TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  TimHandle.Init.RepetitionCounter = 0;
 	HAL_TIM_Base_Init(&TimHandle);
  
	// Set the TIMx priority 
	HAL_NVIC_SetPriority(TIM3_IRQn, 3, 0);	
	// Enable the TIMx global Interrupt 
  HAL_NVIC_EnableIRQ(TIM3_IRQn);
	
	__HAL_TIM_ENABLE_IT(&TimHandle, TIM_IT_UPDATE); //The specified TIM3 interrupt : update
  __HAL_TIM_ENABLE(&TimHandle);
}

/**************************************************************/
//					EXTI0_IRQHandler
/**************************************************************/
void EXTI0_IRQHandler(void)
{
  uint32_t ticksIn;
  
	if (EXTI->PR & EXTI_PR_PR0)
	{
    // disable interrupt so it won't triger before the change of state in "main.c"
    HAL_NVIC_DisableIRQ(EXTI0_IRQn);
    
    ticksIn = It_getTicks();   
    while(It_getTicks() - ticksIn < 250){;} 
    
    ticksIn = It_getTicks();  
    while((GPIOA->IDR & GPIO_PIN_0))
    {
     if (It_getTicks() - ticksIn > 850)
      {
        FlagOutput = 1;
        break;
      }
    }
    
    if (!FlagOutput)
      FlagState = 1;

    FlagUpdate = 1;
  }
  EXTI->PR = EXTI_PR_PR0;
}

uint16_t BetaIndex = 1;
/**************************************************************/
//					EXTI1_IRQHandler
/**************************************************************/
void EXTI1_IRQHandler(void)
{
  uint32_t ticksIn;
  
	if (EXTI->PR & EXTI_PR_PR1)
	{
    // disable interrupt so it won't triger before the change of state in "main.c"
    HAL_NVIC_DisableIRQ(EXTI1_IRQn);
    
    ticksIn = It_getTicks();   
    while(It_getTicks() - ticksIn < 250){;} 
    
    BetaIndex++;
      
    if(BetaIndex > 8)
      BetaIndex = 1;  

    FlagUpdate = 1;
    FlagEtaBeta = 1;
  }
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);
  EXTI->PR = EXTI1_IRQn;
}

static uint8_t toggle;
volatile uint32_t adcVal, AdcResult = 0;
/**************************************************************/
//					TIM3_IRQHandler
/**************************************************************/
void TIM3_IRQHandler(void)
{
  if(__HAL_TIM_GET_FLAG(&TimHandle, TIM_FLAG_UPDATE) != RESET)
	{
		if(__HAL_TIM_GET_ITSTATUS(&TimHandle, TIM_IT_UPDATE) !=RESET)
		{
      if (toggle)
      {
        HAL_ADC_Start(&AdcHandle);
        toggle = 0;    
      }
      else
      {       
        /*##-5- Get the converted value of regular channel  ########################*/
        adcVal =  HAL_ADC_GetValue(&AdcHandle);
        if (adcVal - DELTA_ADC >  AdcResult || adcVal + DELTA_ADC <  AdcResult)
        {
          FlagEtaBeta = 1;
          FlagUpdate = 1;
          AdcResult =  adcVal;
        }
        toggle = 1;
      }
      __HAL_TIM_CLEAR_IT(&TimHandle, TIM_IT_UPDATE); // Remove TIMx update interrupt flag 
			__HAL_TIM_CLEAR_FLAG(&TimHandle, TIM_IT_UPDATE);
		}	
	}
}  

/**************************************************************/
//					Leds
/**************************************************************/	
static void Leds(uint8_t haut, uint8_t bas, uint8_t gauche, uint8_t droite)
{ 
  if (haut)   GPIOD->BSRRL |= GPIO_PIN_13; 
	else 			  GPIOD->BSRRH |= GPIO_PIN_13;
  
  if (bas)    GPIOD->BSRRL |= GPIO_PIN_15; 
	else 			  GPIOD->BSRRH |= GPIO_PIN_15;
  
  if (gauche) GPIOD->BSRRL |= GPIO_PIN_12; 
	else 			  GPIOD->BSRRH |= GPIO_PIN_12;
  
  if (droite) GPIOD->BSRRL |= GPIO_PIN_14; 
	else 			  GPIOD->BSRRH |= GPIO_PIN_14;  
}

/**************************************************************/
//					Board_Leds
/**************************************************************/
void Board_Leds(uint8_t  state, Output_device_t output)
{
	switch(state)
	{
		case ((uint8_t) __8ch_2bit__20kHz__C__) :
			if (output == Usb)
        Leds(1,0,0,0);
      else 
        Leds(0,1,1,1);
			break;    
		case ((uint8_t) __4ch_16bit_20kHz_NC__) :
			if (output == Usb)
        Leds(0,0,0,1);
      else 
        Leds(1,1,1,0);
			break;
		case ((uint8_t) __8ch_16bit_10kHz_NC__) :
			if (output == Usb)
        Leds(0,1,0,0);
      else 
        Leds(1,0,1,1);
			break;
		case ((uint8_t) __8ch_8bit__20kHz_NC__ ) :
			if (output == Usb)
        Leds(0,0,1,0);
      else 
        Leds(1,1,0,1);
			break;
    case NO_LED :
			Leds(0,0,0,0);
			break;
    case ALL_LEDS :
			Leds(1,1,1,1);
			break;
    default :
      break;
	}		
}

 /**************************************************************/
//					Leds_Blink
/**************************************************************/
void Board_LedsBlink(DataStateTypeDef CurrentState, Output_device_t output)
{
  static uint8_t i;
  static uint32_t ticksIn;
  
  Board_Leds(ALL_LEDS, output);
  
  for(i = 0; i < 4; i++)
  {
     ticksIn = It_getTicks();
     Board_Leds(ALL_LEDS, Usb);
     while (( It_getTicks() - ticksIn) < 100)
     {;}
       
     ticksIn = It_getTicks();
     Board_Leds(NO_LED, output);
     while ((It_getTicks() - ticksIn) < 200)
     {;}  
  }
  Board_Leds((uint8_t) CurrentState, output); 
}  

/**************************************************************/
//					Board_CheckUpdate
/**************************************************************/
uint8_t Board_CheckUpdate(void)
{
  if (FlagUpdate)
  {
    FlagUpdate = 0;
    return FLAG_UPDATE;
  } 
  else
    return FLAG_NO_UPDATE;
}
  
/**************************************************************/
//					Board_GetUpdate
/**************************************************************/
uint8_t Board_GetUpdate(void)
{
  if (FlagState)
  {
    FlagState = 0;
    return FLAG_STATE;
  }
  else  if (FlagOutput)
  {
    FlagOutput = 0;
    return FLAG_OUTPUT;
  }
  else  if (FlagEtaBeta)
  {
    FlagEtaBeta = 0;
    return FLAG_ETA_BETA;
  }
  else 
    return FLAG_NO_UPDATE;
}

// *************************************************************
// 	 				Board_ExtiInterruptEnable 
// *************************************************************
void Board_Interrupt(uint8_t input, DataStateTypeDef CurrentState)
{
  if (input) 
  {  
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
    if (CurrentState == __8ch_2bit__20kHz__C__)
      HAL_NVIC_EnableIRQ(TIM3_IRQn);
  }
  else
  { 
    HAL_NVIC_DisableIRQ(EXTI0_IRQn);   
    HAL_NVIC_DisableIRQ(TIM3_IRQn); 
  }
} 

uint16_t EtaIndex;
/**************************************************************/
//					Board_GetEtaIndex
/**************************************************************/
uint8_t Board_GetEtaIndex(void)
{   
  EtaIndex = AdcResult;
  if (EtaIndex > 188)
    EtaIndex = 188;
  
  EtaIndex = EtaIndex * 32; 
  
  EtaIndex = EtaIndex / 188;
    
  if (EtaIndex < 5)
    EtaIndex = 5;
    
  if (EtaIndex > 32)
    EtaIndex = 32;
  
  return (uint8_t) EtaIndex;
}

/**************************************************************/
//					Board_GetEtaIndex
/**************************************************************/
uint8_t Board_GetBetaIndex(void)
{ 
  return (uint8_t) BetaIndex;
}
















