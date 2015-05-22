#ifndef __COMMONINCLUDE_H__
#define __COMMONINCLUDE_H__

// *************************************************************************
// 									Common Include
// *************************************************************************

#define COMPRESS	1

/* GPIO command */
#define LOW		0
#define HIGH	1

#define CHANNEL_BYTE		1
#define CHANNEL_NUMBER	8

#define CHANNEL_SIZE 		(CHANNEL_BYTE * CHANNEL_NUMBER) // 8
#define BYTES_PER_FRAME	32

#define	SIZE_BUFFER_NRF	 		200
#define	SIZE_BUFFER_USB	 		200
#define	SIZE_BUFFER_DAC	 		100

#define USB_FRAME	 	 		4 
#define DAC_FRAME	    	4
#define NRF_CHANNEL_FRAME		4 // 32/8 = 4

#define NBIT 				 	 3								 // resolution of the compression
#define POW_2_NBIT  	(1 << NBIT) 			// 2^NBIT
#define CUT_VAL_SIZE 	(POW_2_NBIT - 1) // number of cut value
#define ETA					 	3000						// adaptation parameter
#define	RANGE					1000
//#define	DELTA 				(RANGE/ (CUT_VAL_SIZE + 1))

#define DEBUG_HIGH 	(GPIOA->BSRRL |= GPIO_PIN_15)
#define DEBUG_LOW		(GPIOA->BSRRH |= GPIO_PIN_15)


#endif



