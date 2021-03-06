#ifndef __COMMONINCLUDE_H__
#define __COMMONINCLUDE_H__

// *************************************************************************
// 									Common Include
// *************************************************************************

/********************************************************************/
//CHANGEABLE PARAMETER 

// This mode will generate an artifiacial signal (a sort of replacement of te RHD acquired signal) to test the system
//#define TESTBUFFER

//enable the change of Eta and Beta via receiver inputs
//#define PARAMETER_SELECTION 

// On compressing mode, compress only 4 chanel and send at the same time their non-compressed value to compare
#define COMPARISON 

#define ETA_      512					    // adaptation parameter
#define BETA_     8					     // error parameter
#define H_        120/128

/********************************************************************/
// FIXED PARAMETERS 

/* GPIO command */
#define LOW		0
#define HIGH	1

#define CHANNEL_BYTE		  1
#define CHANNEL_NUMBER	  8

#define CHANNEL_SIZE 		  (CHANNEL_BYTE * CHANNEL_NUMBER) // 8
#define BYTES_PER_FRAME	  32

#define	SIZE_BUFFER_NRF	  100
#define	SIZE_BUFFER_USB	  200
#define	SIZE_BUFFER_DAC	  100

#define USB_FRAME	 	 		  4 
#define DAC_FRAME	    	  4
#define NRF_CHANNEL_FRAME	4 // 32/8 = 4

#define FBAR               1
#define NBIT 				 	     2                 // resolution of the compression
#define POW_2_NBIT  	    (1 << NBIT) 			// 2^NBIT
#define CUT_VAL_SIZE 	    (POW_2_NBIT - 1) // number of cut value

#define FLAG_NO_UPDATE 0
#define FLAG_UPDATE    1

#define FLAG_STATE    1
#define FLAG_OUTPUT   2
#define FLAG_ETA_BETA 3

#define DEBUG_HIGH 	(GPIOA->BSRRL |= GPIO_PIN_15)
#define DEBUG_LOW		(GPIOA->BSRRH |= GPIO_PIN_15)

#define STATE_INIT	 __8ch_2bit__20kHz__C__
#define OUTPUT_INIT  Usb

/**************************************************************/
//					Enum
/**************************************************************/
typedef enum{
  Dac, 
  Usb

} Output_device_t;

typedef enum
{
	__8ch_2bit__20kHz__C__ = 0x01,
	__4ch_16bit_20kHz_NC__ = 0x02,
	__8ch_16bit_10kHz_NC__ = 0x03,
  __8ch_8bit__20kHz_NC__ = 0x04,
  
} DataStateTypeDef;

#endif



