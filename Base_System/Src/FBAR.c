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
static uint16_t Beta  = 1;
static uint16_t etaAdd[CUT_VAL_SIZE + 1] ={0};  // (CUT_VAL_SIZE + 1) is for the case NBIT == 4
static uint16_t etaSous[CUT_VAL_SIZE + 1]={0}; //  (CUT_VAL_SIZE + 1) is for the case NBIT == 4
static uint16_t Prediction[CHANNEL_SIZE]     = {0};
static int16_t PredictorError  = {0};
static int16_t cutValue[CHANNEL_SIZE][CUT_VAL_SIZE + 1]     = {0}; // (CUT_VAL_SIZE + 1) : we add 1 to avoid the warning out of range
static int16_t cutValueSave[CHANNEL_SIZE][CUT_VAL_SIZE + 1] = {0}; // (CUT_VAL_SIZE + 1) : we add 1 to avoid the warning out of range

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
	volatile  int16_t i,j,range, Delta;
	
  Eta = 20; //EtaIndex * 10;
  
	Delta = 250;
	
	// initialize the first cutvalues
	for(i=0; i < CHANNEL_SIZE; i++) 
	{
    for(j=0; j < CUT_VAL_SIZE; j++)  
    {
      cutValue[i][j] = (j-NBIT) * Delta;
      cutValueSave[i][j] = (j-NBIT) * Delta;
    }
  }
    
	for (i=0; i < CUT_VAL_SIZE; i++)
	{
		etaSous[i] = Eta / (i + 1);
		etaAdd[i]  = Eta / (CUT_VAL_SIZE - i);
	}
}

/**************************************************************/
//					FBAR_Reinitialize
/**************************************************************/
void FBAR_Reinitialize(uint8_t * bufferFrom)
{
	static volatile uint16_t i, j, delta;
	static volatile int16_t value;
  
  #pragma unroll_completely 
	for(i=0; i < CHANNEL_SIZE; i++)
	{
		value = ((*bufferFrom) << 8) + (*(bufferFrom+1));    
   
    Prediction[i] = value;
    
    delta = Eta;
    
    #pragma unroll_completely 
		for(j=0; j < CUT_VAL_SIZE; j++)
      cutValue[i][j] = (j-1) * delta;
    
		bufferFrom+= 2;
	}
}

volatile int8_t winner;
/**************************************************************/
//					FBAR_Uncompress
/**************************************************************/
void FBAR_Uncompress(uint8_t * bufferFrom, uint16_t * bufferTo)
{
	uint16_t i, j;
  uint32_t temp, tempPred;
  
	//loop on an NRF frame : NRF_CHANNEL_FRAME * CHANNEL_SIZE channels
	for(i=0; i < NRF_CHANNEL_FRAME; i++)
	{ 
    //#pragma unroll_completely 
		for(j=0; j < CHANNEL_SIZE; j++)  // loop on all the CHANNEL_SIZE channels
		{      
      
//      PredictorError = (*bufferFrom) << 8;
//      bufferFrom++;

//      if (Prediction[j] + PredictorError - 0x8000 < 0)
//        Prediction[j] = 0;     
//      else if (Prediction[j] + PredictorError - 0x8000 >= 0xFFFF - 10)
//        Prediction[j] = 0xFFFF;
//      else
//      Prediction[j] = Prediction[j] + PredictorError - 0x8000;
//      
//      *bufferTo++ = (Prediction[j] >> 1 ) & 0x7FFF; 

			winner = (*bufferFrom++);
      
      FBAR_AdaptCutValues(j, winner);
      
      if (winner == CUT_VAL_SIZE)
        PredictorError = cutValue[j][CUT_VAL_SIZE-1];    
      else if (!winner) 
        PredictorError = cutValue[j][0];               
      else 
        PredictorError = (cutValue[j][winner-1] + cutValue[j][winner]) /2;
      
      if (PredictorError >= 0)
      {  
        if (Prediction[j] + PredictorError < 0xFFFF)
          Prediction[j] = (Prediction[j] * 120 / 128) + PredictorError;
        else
          Prediction[j] = 0xFFFF * 120 / 128;
      }
      else //(PredictorError < 0)
      {
        if (Prediction[j] - (uint16_t)(-PredictorError) > 0)
          Prediction[j] = (Prediction[j] * 120 / 128) - (uint16_t)(-PredictorError);
        else
          Prediction[j] = 0;
      }    
      
       *bufferTo++ = (Prediction[j]) & 0x7FFF; 
       
//      if (Prediction[j] > 50)
//       Prediction[j] -= 3; 
        
//      if (winner == CUT_VAL_SIZE-1)
//        temp = cutValue[j][CUT_VAL_SIZE-1];    
//      else if (!winner) 
//        temp = cutValue[j][0];               
//      else
//        temp = (cutValue[j][winner-1] + cutValue[j][winner]) /2;
//      *bufferTo++ = temp;
    } 
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
				if (cutValue[channel][0] >  (-32767) + Eta) 
					cutValue[channel][0] -= Eta;
			}
			else if ((cutValue[channel][i] - cutValue[channel][i-1]) >= etaSous[i])
				cutValue[channel][i] -= etaSous[i];	
		}
		else 
		{
			if (i == CUT_VAL_SIZE-1) 
			{
        if (cutValue[channel][CUT_VAL_SIZE-1] <  32767 - Eta) 
					cutValue[channel][CUT_VAL_SIZE-1] += Eta;
			}
			else if ((cutValue[channel][i+1] - cutValue[channel][i]) >= etaAdd[i])
				cutValue[channel][i] += etaAdd[i];
		}
    cutValue[channel][i] -= (cutValue[channel][i] - cutValueSave[channel][i]) / 256;
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
          
          bufferTo++;
          bufferFrom++;
        }
      }
      break;
      
    default :
      break;
  }
}



















