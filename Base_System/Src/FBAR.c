#include "FBAR.h"

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

#define H_     120/128
#define BETA   8
#define ETA_   512
#define DELTA_ 400

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
static int16_t Eta;
static int16_t etaAdd[CUT_VAL_SIZE + 1] ={0};  // (CUT_VAL_SIZE + 1) is for the case NBIT == 4
static int16_t etaSous[CUT_VAL_SIZE + 1]={0}; //  (CUT_VAL_SIZE + 1) is for the case NBIT == 4
static int16_t Prediction[CHANNEL_SIZE]     = {0};
static int16_t PredictorError  = {0};
static int16_t cutValue[CHANNEL_SIZE][CUT_VAL_SIZE]     = {0}; 

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
	volatile  int16_t i;
	
  Eta = ETA_; //EtaIndex * 10;
	
	// initialize the first cutvalues
	for(i=0; i < CHANNEL_SIZE; i++) 
	{
    Prediction[i] = 0;
    
    cutValue[i][0] = - DELTA_;
    cutValue[i][1] = 0;
    cutValue[i][2] = DELTA_;
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
	static uint16_t i, j;
	static int16_t value;
  static uint8_t ReinitCoice;
  
  ReinitCoice = *bufferFrom++;
  
  switch (ReinitCoice)
  {
    case 0x01 :
    {
      #pragma unroll_completely 
      for(i=0; i < CHANNEL_SIZE; i++)
      {
        value = ((*bufferFrom) << 8) + (*(bufferFrom+1));
        Prediction[i] = value;
        bufferFrom+= 2;
      }
      __nop(); __nop(); __nop(); 
      break;
    }
    
    case 0x02 :
    {
      #pragma unroll_completely
      for(i=0; i < CHANNEL_SIZE/2; i++)
      {		
        #pragma unroll_completely
        for(j=0; j < CUT_VAL_SIZE; j++)
        {
          cutValue[i][j] = ((*bufferFrom) << 8) + (*(bufferFrom+1)); 
          bufferFrom+= 2;          
        }
      }
      __nop(); __nop(); __nop(); 
      break;
    }
    
    case 0x03 :
    {
      #pragma unroll_completely
      for(i=CHANNEL_SIZE/2; i < CHANNEL_SIZE; i++)
      {		
        #pragma unroll_completely
        for(j=0; j < CUT_VAL_SIZE; j++)
        {
          cutValue[i][j] = ((*bufferFrom) << 8) + (*(bufferFrom+1));
          bufferFrom+= 2;
        }
      }
      __nop(); __nop(); __nop(); 
      break;
    }
    
    default :
      break; 
  }
}

volatile int8_t winner;
/**************************************************************/
//					FBAR_Uncompress
/**************************************************************/
void FBAR_Uncompress(uint8_t * bufferFrom, uint16_t * bufferTo)
{
	uint16_t i, j;
  
	//loop on an NRF frame : NRF_CHANNEL_FRAME * CHANNEL_SIZE channels
	for(i=0; i < NRF_CHANNEL_FRAME; i++)
	{ 
    #pragma unroll_completely 
		for(j=0; j < CHANNEL_SIZE; j++)  // loop on all the CHANNEL_SIZE channels
		{      
			winner = (*bufferFrom++);
      
      FBAR_AdaptCutValues(j, winner);
                 
      //prediction error
      if (winner == CUT_VAL_SIZE)
        PredictorError = cutValue[j][CUT_VAL_SIZE-1];    
      else if (!winner) 
        PredictorError = cutValue[j][0];               
      else 
        PredictorError = (cutValue[j][winner-1] + cutValue[j][winner]) /2;
                 
      Prediction[j] = (Prediction[j] * H_) + PredictorError;
      
      *bufferTo++ = Prediction[j]; 
    } 
	}	
}   
	
/**************************************************************/
//					FBAR_AdaptCutValues
/**************************************************************/
static void FBAR_AdaptCutValues(uint16_t channel, uint16_t winner)
{
	uint16_t i;
  static int16_t TmpCut;
	
	#pragma unroll_completely 
	for(i=0; i < CUT_VAL_SIZE; i++)
	{
		if (winner <= i)
		{
      TmpCut = -etaSous[i]-(cutValue[channel][i]) / BETA;
      if (!i)
      {
        // on controle que les cutvalues ne dépassent pas 0 en négatif
        if (cutValue[channel][0] > (-32767) + TmpCut)  
          cutValue[channel][0] += TmpCut;        
      }
      // anti chevauchement (et compilation warning i-1, depassement negatif range tableau, secu à prévoir)
      else 
      {
        if (((cutValue[channel][i] - cutValue[channel][i-1]) >= TmpCut))  
          cutValue[channel][i] +=  TmpCut;	
      }
		}
		else 
		{
      TmpCut = etaAdd[i]-(cutValue[channel][i]) / BETA;
      if(i == (CUT_VAL_SIZE-1))
      {        
        // on controle que les cutvalues ne dépassent pas l2^15 - 1 
        if (cutValue[channel][CUT_VAL_SIZE-1] < 32767 - TmpCut) 
          cutValue[channel][CUT_VAL_SIZE-1] += TmpCut;
      }
      // anti chevauchement (et compilation warning i+1, depassement range tableau, secu à prévoir)
      else 
      {  
        if ((cutValue[channel][i+1] - cutValue[channel][i]) >= TmpCut) 
          cutValue[channel][i] += TmpCut;	
      }
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



















