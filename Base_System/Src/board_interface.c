#include "board_interface.h"

static DataStateTypeDef   DataState = FIRST_STATE;
static Output_device_t    Output_device = FIRST_OUTPUT;
static uint8_t            FlagUpdate = 0;
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
	Board_Leds(DataState);
  TIM3Init(500, 300);
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
  
    /*##-2- Configure peripheral GPIO ##########################################*/ 
  /* ADC3 Channel8 GPIO pin configuration */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
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


uint8_t changeOutput = 0, indexState = 0;
DataStateTypeDef stateSystem[4] = 
{__8ch_16bit_20kHz__C__, __4ch_16bit_20kHz_NC__, __8ch_16bit_10kHz_NC__, __8ch_8bit__20kHz_NC__};
/**************************************************************/
//					EXTI0_IRQHandler
/**************************************************************/
void EXTI0_IRQHandler(void)
{
  static uint32_t ticksIn;
  
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
        changeOutput = 1;
        break;
      }
    }
    
    if (!changeOutput)
    {
      indexState = (indexState >= 3) ? 0 : indexState+1;
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
    
    if (DataState == __8ch_16bit_20kHz__C__)
      HAL_NVIC_EnableIRQ(TIM3_IRQn);      // enable timer (adc potentiometer) interrupt
    else
		 HAL_NVIC_DisableIRQ(TIM3_IRQn);     // or disable it
      
        
    FlagUpdate = 1;
    
    while((GPIOA->IDR & GPIO_PIN_0))
    {;} 
      
  }
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
//					Board_GetUpdate
/**************************************************************/
uint8_t Board_GetUpdate(void)
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

// *************************************************************
// 	 				Board_ExtiInterruptEnable 
// *************************************************************
void Board_InterruptEnable(uint8_t state)
{
  if (state) 
  {  
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
  }
  else
  {                               
    HAL_NVIC_DisableIRQ(TIM3_IRQn);
    HAL_NVIC_DisableIRQ(EXTI0_IRQn);
  }
}

static uint8_t toggle;
volatile static uint32_t adcVal, AdcResult = 0;
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
          AdcResult =  adcVal;         
          FlagUpdate = 1;
        }
        toggle = 1;
      }
      __HAL_TIM_CLEAR_IT(&TimHandle, TIM_IT_UPDATE); // Remove TIMx update interrupt flag 
			__HAL_TIM_CLEAR_FLAG(&TimHandle, TIM_IT_UPDATE);
		}	
	}
}  

/**************************************************************/
//					Board_GetEtaIndex
/**************************************************************/
uint8_t Board_GetEtaIndex(void)
{ 
  static uint16_t EtaIndex;
  
  EtaIndex = AdcResult;
  if (EtaIndex > 188)
    EtaIndex = 188;
  
  EtaIndex = EtaIndex * 100; 
  
  EtaIndex = EtaIndex / 188; 
  
  return (uint8_t) EtaIndex;
}

















