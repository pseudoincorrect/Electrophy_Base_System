/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "usb_device.h"
#include "NRF.h"
#include "DAC.h"
#include "ElectrophyData.h"

// *************************************************************************
// *************************************************************************
// 						Private functions	
// *************************************************************************
// *************************************************************************
static void SystemClock_Config(void);
static void ClockInit(void);
static void ChooseOutput(Output_device_t  Output_device);

// *************************************************************************
// *************************************************************************
// 						static variables	
// *************************************************************************
// *************************************************************************
static Output_device_t Output_device = Usb;

// *************************************************************************
// *************************************************************************
// 								 MAIN
// *************************************************************************
// *************************************************************************
int main(void)
{	
  HAL_Init();						// Reset of all peripherals, Initializes the Flash interface and the Systick
  SystemClock_Config(); // Configure the system clock
	ClockInit(); 					// Initialize all configured peripherals clocks
	DAC_Init();
	MX_USB_DEVICE_Init();
	ChooseOutput(Output_device);
	ElectrophyData_Init(Output_device);
	NRF_Init();
	
	while (1)
  {
		if (Output_device == Usb)
			ElectrophyData_ApplyMask();
	}
}

// *************************************************************************
// *************************************************************************
// 						static function definition	
// *************************************************************************
// *************************************************************************
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

// **************************************************************
// 	 				ClockInit 
// **************************************************************
static void ClockInit(void)
{
// GPIO Ports Clock Enable 
  __GPIOH_CLK_ENABLE();
  __GPIOA_CLK_ENABLE();	
	__GPIOB_CLK_ENABLE();
	__GPIOC_CLK_ENABLE();
	__GPIOD_CLK_ENABLE();

	__GPIOE_CLK_ENABLE();
	
	__SPI1_CLK_ENABLE();
	__SPI2_CLK_ENABLE();
	__SPI3_CLK_ENABLE();

	__DMA1_CLK_ENABLE();
	__DMA2_CLK_ENABLE();
	
	__TIM2_CLK_ENABLE();
	
	__USB_OTG_FS_CLK_ENABLE();
}

// **************************************************************
// 	 				ChooseOutput 
// **************************************************************
static void ChooseOutput(Output_device_t  Output_device)
{
	if (Output_device == Usb)
	{
		DAC_Enable(LOW);
		MX_USB_DEVICE_Enable(HIGH);
	}
	else
	{
		DAC_Enable(HIGH);
		MX_USB_DEVICE_Enable(LOW);
	}
}
