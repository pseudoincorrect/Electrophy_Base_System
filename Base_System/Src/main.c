/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "usb_device.h"
#include "NRF.h"
#include "DAC.h"
#include "ElectrophyData.h"
#include "board_interface.h"
#include "stm32f4xx_hal_gpio.h"

// *************************************************************************
// *************************************************************************
// 						Private functions	
// *************************************************************************
// *************************************************************************
static void ClockInit(void);
static void ChooseOutput(Output_device_t  Output_device);
static void ChangeState(void);

// *************************************************************************
// *************************************************************************
// 						static variables	
// *************************************************************************
// *************************************************************************
static Output_device_t Output_device = Usb;
static Board_StateTypeDef BoardState = __8ch_16bit_20kHz__C__;
static GPIO_InitTypeDef GPIO_InitStruct;

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
  Board_Init();
	
	//****************************************************** debug pin
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pin =  GPIO_PIN_15 | GPIO_PIN_10 | GPIO_PIN_8;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
 

	while (1)
  { 
    if (Board_GetStatus())
       ChangeState();
    ElectrophyData_Process();
	}
}

// *************************************************************************
// *************************************************************************
// 						static function definition	
// *************************************************************************
// *************************************************************************
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

// **************************************************************
// 	 				ChangeState 
// **************************************************************
static void ChangeState(void)
{
    if (Output_device != Board_GetOutput()) 
      ChooseOutput(Board_GetOutput());

}







