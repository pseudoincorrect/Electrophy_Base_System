/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "usb_device.h"
#include "NRF.h"
#include "DAC.h"
#include "ElectrophyData.h"
#include "board_interface.h"

// *************************************************************************
// *************************************************************************
// 						Private functions	
// *************************************************************************
// *************************************************************************
// initialize all the hardware clocks
static void             ClockInit   (void);
//select the output (Dac or Usb)
static void             SetOutput   (Output_device_t  Output_device);
// apply the change considering the new state
static void             ChangeState (void);
// set the next state of the state machine (Moore state machine)
static DataStateTypeDef NextState   (void);

// *************************************************************************
// *************************************************************************
// 						static variables	
// *************************************************************************
// *************************************************************************
const DataStateTypeDef stateSystem[4] = { 
  __8ch_16bit_20kHz__C__, 
  __4ch_16bit_20kHz_NC__, 
  __8ch_16bit_10kHz_NC__, 
  __8ch_8bit__20kHz_NC__
};

static Output_device_t  Output_device;
static DataStateTypeDef State;
static     uint8_t      EtaIndex;

// *************************************************************************
// *************************************************************************
// 								               MAIN
// *************************************************************************
// *************************************************************************
int main(void)
{	
  // STM32 CORE initialization
  ClockInit(); 					// Initialize all configured peripherals clocks
  HAL_Init();						// Reset of all peripherals, Initializes the Flash interface and the Systick
  SystemClock_Config(); // Configure the system clock
	
  // HARDWARE initialization
  DAC_Init();
  MX_USB_DEVICE_Init(); 
  NRF_Init();
  Board_Init();
	  
  // SOFTWARE initialization
  Output_device = FIRST_OUTPUT;
  EtaIndex      = ETA_INDEX_INIT;
  State         = NextState();
  
  SetOutput(Output_device);	
  ElectrophyData_Reset(Output_device, State, EtaIndex);
  Board_Interrupt(HIGH, State); 
  DAC_SetNewState(State);
  Board_Leds(State, Output_device);
  //Board_LedsBlink(State);
  
  // INFINITE LOOP 
	while (1)
  { 
    if (Board_CheckUpdate())
    {
      Board_Interrupt(LOW, State);
      ChangeState();
      Board_Interrupt(HIGH, State);
    }  
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
  __PWR_CLK_ENABLE();
  
  // GPIO Ports Clock Enable 
  __GPIOA_CLK_ENABLE();	
	__GPIOB_CLK_ENABLE();
	__GPIOC_CLK_ENABLE();
	__GPIOD_CLK_ENABLE();
	__GPIOE_CLK_ENABLE();
  __GPIOH_CLK_ENABLE();
	
	__SPI1_CLK_ENABLE();
	__SPI2_CLK_ENABLE();
	__SPI3_CLK_ENABLE();

	__DMA1_CLK_ENABLE();
	__DMA2_CLK_ENABLE();
	
	__TIM2_CLK_ENABLE();
	__TIM3_CLK_ENABLE();
  
  __ADC1_CLK_ENABLE();
  
	__USB_OTG_FS_CLK_ENABLE();
}

// **************************************************************
// 	 				SetOutput 
// **************************************************************
static void SetOutput(Output_device_t  Output_device)
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
  switch (Board_GetUpdate())
  {
    //Change and send the State of the system
    case (FLAG_STATE) :
    {
      State = NextState();  
      if (State == __8ch_16bit_20kHz__C__)
      {
        EtaIndex = Board_GetEtaIndex();
        NRF_SendNewState(EtaIndex + 100);
      }
      else
        NRF_SendNewState((uint8_t) State);      
      ElectrophyData_Reset(Output_device, State, EtaIndex);
      DAC_SetNewState(State); 
      Board_Leds(State, Output_device);  
      break;
    }
    
    //Change the Output : Usb or dac
    case (FLAG_OUTPUT) :
    {           
      Output_device = (Output_device == Usb) ? Dac : Usb; 
      SetOutput(Output_device);
      ElectrophyData_Reset(Output_device, State, EtaIndex);
      Board_LedsBlink(State, Output_device);      
      break;
    }
    
    //Change and send the Eta used by FBAR
    case (FLAG_ETA) :
    {
      if (State == __8ch_16bit_20kHz__C__)
      {
        Board_Leds(NO_LED, Output_device);
        EtaIndex = Board_GetEtaIndex();
        NRF_SendNewState(EtaIndex + 100);
        ElectrophyData_Reset(Output_device, State, EtaIndex);
        Board_Leds(State, Output_device);
      }
      break;
    }
    
    default :
      break;
  }
}  

static uint8_t indexState = 3;
// **************************************************************
// 	 				            NextState 
// **************************************************************
static DataStateTypeDef NextState(void)
{
  indexState = (indexState >= 3) ? 0 : indexState+1;
  return stateSystem[indexState];     
}





