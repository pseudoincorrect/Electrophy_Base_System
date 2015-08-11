#include "FBAR.h"

// *************************************************************************
// *************************************************************************
// 						static function declaration, see .h file	
// *************************************************************************
// *************************************************************************
static void FBAR_AdaptCutValues(uint16_t channel, uint16_t winner);

// *************************************************************************
// *************************************************************************
// 						static variables	
// *************************************************************************
// *************************************************************************
static uint16_t Eta;
static uint16_t cutValue[CHANNEL_SIZE][CUT_VAL_SIZE + 1] = {0}; // (CUT_VAL_SIZE + 1) : we add 1 to avoid the warning out of range line 145
static uint16_t etaAdd[CUT_VAL_SIZE + 1] ={0};  // (CUT_VAL_SIZE + 1) is for the case NBIT == 4
static uint16_t etaSous[CUT_VAL_SIZE + 1]={0}; //  (CUT_VAL_SIZE + 1) is for the case NBIT == 4

// *************************************************************************
// *************************************************************************
// 						Example 
// *************************************************************************
// *************************************************************************
/*	 N = 2 ==> 2 bits ==> 3 cuts values ==> 4 winners
						  CUT_VAL_SIZE = 3   ==> (2^2 - 1)

cut val i :							0					1					2 	
							|---------|---------|---------|---------|
winner :					00				01				10				11
Delta	 :			|---------|

etaAdd  [0] = ETA/1			[1] =	ETA/2			[2] =	ETA/3		
etaSous [0] = ETA/3			[1] =	ETA/2			[2] =	ETA/1	
*/

//*************************************************************************
//*************************************************************************
// 										Function definitions																 
//*************************************************************************
//*************************************************************************	
/**************************************************************/
//					FBAR_Initialize
/**************************************************************/
void FBAR_Initialize(uint16_t EtaIndex)
{
	volatile  uint16_t i,j,range, delta;
	
  Eta = EtaIndex * 50;
	range = 65535;
  
	delta = range / (CUT_VAL_SIZE + 1);
	
	for(i=0; i < CHANNEL_SIZE; i++)
	{
    for(j=0; j < CUT_VAL_SIZE; j++)
		{
      #if ((NBIT == 1) || (NBIT == 4))
      cutValue[i][0] = 30000;
      #else
      cutValue[i][j] = (j+1) * delta;
      #endif
    }
  }  
  #if ((NBIT == 2) || (NBIT == 3))    
	for (i=0; i < CUT_VAL_SIZE; i++)
	{
		etaSous[i] = Eta / (i+1);
		etaAdd[i]  = Eta / (CUT_VAL_SIZE-i);
	}
  #elif (NBIT == 4)
  for (i=0; i < 4; i++)
		etaAdd[i] = (Eta / 3) * i;   
  #endif
  __nop();
}

uint8_t reinitializeBit = 0;
/**************************************************************/
//					FBAR_Reinitialize
/**************************************************************/
void FBAR_Reinitialize(uint8_t * bufferFrom)
{
	volatile uint16_t i, j, value, delta;
	
  #pragma unroll_completely 
	for(i=0; i < CHANNEL_SIZE; i++)
	{
		value = ((*bufferFrom) << 8) + (*(bufferFrom+1)); 
	
    #if (!((NBIT == 1) || (NBIT == 4)))
		// quand je prend un delta non constant, le système à la réception ne fonctionne pas très bien
		delta = (cutValue[i][CUT_VAL_SIZE - 1]-cutValue[i][0]) / (CUT_VAL_SIZE - 1);      
    #endif 	
	
    #pragma unroll_completely 
		for(j=0; j < CUT_VAL_SIZE; j++)
    {
      #if ((NBIT == 1) || (NBIT == 4))
      cutValue[i][0] = value;
      #else
			cutValue[i][j] = value + (j-NBIT) * delta;
      #endif 
		}
		bufferFrom+= 2;
	}
  reinitializeBit = 1;
}

volatile uint8_t winner;
/**************************************************************/
//					FBAR_Uncompress
/**************************************************************/
void FBAR_Uncompress(uint8_t * bufferFrom, uint16_t * bufferTo)
{
	uint16_t i, j;
	volatile int32_t tempValue;
  
	// loop on an NRF frame : NRF_CHANNEL_FRAME * CHANNEL_SIZE channels
	//#pragma unroll_completely 
	for(i=0; i < NRF_CHANNEL_FRAME; i++)
	{ 
    #pragma unroll_completely 
		for(j=0; j < CHANNEL_SIZE; j++)  // loop on all the CHANNEL_SIZE channels
		{
			winner = (*bufferFrom) & 0x07;
      bufferFrom++;
      
      //************************************************** N == 1
      #if (NBIT == 1)
      tempValue = cutValue[j][0]; 
          
      if (winner)
      { 
       if (cutValue[j][0] < 65535 - Eta)
          cutValue[j][0] += Eta;
      }      
      else 
      {
        if (cutValue[j][0] > Eta)
          cutValue[j][0] -= Eta;
      }
      
      *bufferTo++ = ( tempValue >> 1) & 0x7FFF;
      
      //************************************************** N == 4
      #elif (NBIT == 4)
      
      tempValue = cutValue[j][0];
      
      if (winner & 0x04)
      {
        if (cutValue[j][0] > Eta)
          cutValue[j][0] -= etaAdd[(winner & 0x03)];
      }
      else
      {
        if (cutValue[j][0] < 65535 - Eta)
          cutValue[j][0] += etaAdd[(winner & 0x03)];   
      }
      
//      if (reinitializeBit)
//        *bufferTo++ =  60000 ;
//      else
      *bufferTo++ = ( tempValue >> 1) & 0x7FFF; 
      
      //************************************************** N == 2 or 3
      #elif ((NBIT == 2) || (NBIT == 3)) 
      if (!winner)
      { 
        tempValue = cutValue[j][0]-(cutValue[j][1]-cutValue[j][0])/2;
        if ( tempValue < 0)
          tempValue = cutValue[j][0];
        *bufferTo++ = ( tempValue >> 1)  & 0x7FFF;
      }      
      else if (winner == CUT_VAL_SIZE)
      {
        tempValue = ((cutValue[j][CUT_VAL_SIZE-1] + Eta/2)); //((cutValue[j][CUT_VAL_SIZE-1]-cutValue[j][CUT_VAL_SIZE-2])/2)));
        if (tempValue > 65535 )
          tempValue = cutValue[j][CUT_VAL_SIZE-1];
        *bufferTo++ = ( tempValue >> 1)  & 0x7FFF;
          
      }  
      else
        *bufferTo++ = ((cutValue[j][winner] + cutValue[j][winner-1]) >> 2) & 0x7FFF; 
      // set the new the cut values
      FBAR_AdaptCutValues(j, winner);
      #endif  
		} 
    reinitializeBit = 0;
	}	
}	
	
/**************************************************************/
//					FBAR_AdaptCutValues
/**************************************************************/
static void FBAR_AdaptCutValues(uint16_t channel, uint16_t winner)
{
	uint16_t i;
	
	#pragma unroll_completely 
	for(i=0; i < CUT_VAL_SIZE; i++)
	{
		if (winner <= i)
		{
			if (!i)
			{
				if (cutValue[channel][0] >  Eta) 
					cutValue[channel][0] -= Eta;
			}
			else if ((cutValue[channel][i] - cutValue[channel][i-1]) >= etaSous[i])
				cutValue[channel][i] -= etaSous[i];	
		}
		else 
		{
			if (i == CUT_VAL_SIZE-1) 
			{
        if (cutValue[channel][CUT_VAL_SIZE-1] <  65535 - Eta) 
					cutValue[channel][CUT_VAL_SIZE-1] += Eta;
			}
			else if ((cutValue[channel][i+1] - cutValue[channel][i]) >= etaAdd[i])
				cutValue[channel][i] += etaAdd[i];
		}
	}
}

static uint16_t PreviousValue[CHANNEL_SIZE] = {0};
/**************************************************************/
//					FBAR_Assemble
/**************************************************************/
void FBAR_Assemble(uint8_t * bufferFrom, uint16_t * bufferTo, DataStateTypeDef state)
{
	uint16_t i,j, CurrentValue;
	
  switch (state)
  {      
    case __4ch_16bit_20kHz_NC__ :
      #pragma unroll_completely 
      for(i=0; i < NRF_CHANNEL_FRAME; i++)
      {
        #pragma unroll_completely 
        for(j=0; j < (CHANNEL_SIZE/2); j++)
        {
          CurrentValue = ( (*bufferFrom) << 7) + (*(bufferFrom + 1) >> 1) & 0x7FFF;
          *bufferTo = CurrentValue;
//          
//          if (CurrentValue > PreviousValue[j] + SECU)
//          {
//            *bufferTo = PreviousValue[j];
//            PreviousValue[j] = PreviousValue[j] + SECU;
//            
//          }
//          else if (CurrentValue < PreviousValue[j] - SECU) 
//          {
//            *bufferTo = PreviousValue[j];
//            PreviousValue[j] = PreviousValue[j] - SECU;
//          }
//          else
//          {
//            *bufferTo = CurrentValue;
//            PreviousValue[j] = CurrentValue;
//          }
          
          bufferTo++;
          bufferFrom += 2;
        }
        #pragma unroll_completely 
        for(j=0; j < (CHANNEL_SIZE/2); j++)
        {
          *bufferTo = 0;
          bufferTo++;
        }
      }
      break;
    
    case __8ch_16bit_10kHz_NC__ :
      #pragma unroll_completely 
      for(i=0; i < NRF_CHANNEL_FRAME/2; i++)
      {
        #pragma unroll_completely 
        for(j=0; j < CHANNEL_SIZE; j++)
        {
          CurrentValue = ( (*bufferFrom) << 7) + (*(bufferFrom + 1) >> 1) & 0x7FFF;
          
          if (CurrentValue > PreviousValue[j] + SECU)
          {
            *bufferTo = PreviousValue[j];
            PreviousValue[j] = PreviousValue[j] + SECU;
            
          }
          else if (CurrentValue < PreviousValue[j] - SECU) 
          {
            *bufferTo = PreviousValue[j];
            PreviousValue[j] = PreviousValue[j] - SECU;
          }
          else
          {
            *bufferTo = CurrentValue;
            PreviousValue[j] = CurrentValue;
          }
       
          bufferTo++;
          bufferFrom += 2;
        }
      }
      break;
    
    case __8ch_8bit__20kHz_NC__ :
      #pragma unroll_completely 
      for(i=0; i < NRF_CHANNEL_FRAME; i++)
      {
        #pragma unroll_completely 
        for(j=0; j < CHANNEL_SIZE; j++)
        {
          CurrentValue = ((*bufferFrom) << 7) & 0x7FFF;
          *bufferTo = CurrentValue;
//          if (CurrentValue > PreviousValue[j] + SECU)
//          {
//            *bufferTo = PreviousValue[j];
//            PreviousValue[j] = PreviousValue[j] + SECU;
//            
//          }
//          else if (CurrentValue < PreviousValue[j] - SECU) 
//          {
//            *bufferTo = PreviousValue[j];
//            PreviousValue[j] = PreviousValue[j] - SECU;
//          }
//          else
//          {
//            *bufferTo = CurrentValue;
//            PreviousValue[j] = CurrentValue;
//          }
          
          bufferTo++;
          bufferFrom++;
        }
      }
      break;
      
    default :
      break;
  }
}



















