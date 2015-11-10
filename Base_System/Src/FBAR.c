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
#define BETA   BETA_INIT
#define ETA_   ETA_INIT
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
static int16_t Eta, Beta;
static int16_t etaAdd[CUT_VAL_SIZE]     ={0};
static int16_t etaSous[CUT_VAL_SIZE]    ={0};
static int16_t Prediction[CHANNEL_SIZE] = {0};
static int16_t PredictorError           = {0};
static int16_t cutValue[CHANNEL_SIZE][CUT_VAL_SIZE] = {0}; 

//*************************************************************************
//*************************************************************************
// 										Function definitions																 
//*************************************************************************
//*************************************************************************	
/**************************************************************/
//					FBAR_Initialize
/**************************************************************/
void FBAR_Initialize(int16_t EtaIndex, int16_t BetaIndex)
{
	uint8_t i;
  
  if (BetaIndex > 0 && BetaIndex <= 128)
    Beta  = BetaIndex;
  
  #ifdef PARAMETER_SELECTION
  Eta = EtaIndex;
  #else
  Eta = ETA_FIXED;
  #endif
  
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
  static uint8_t ReinitIndex;
  
  ReinitIndex = *bufferFrom++;
  
  switch (ReinitIndex)
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
		#ifdef COMPARISON
		for(j=0; j < 2; j++)
		#else
		for(j=0; j < CHANNEL_SIZE; j++)
		#endif
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
	  #ifdef COMPARISON
		*bufferTo = ((*bufferFrom << 7) + (*(bufferFrom + 1) >> 1)) & 0x7FFF;
		bufferTo++; bufferFrom+= 2;
		*bufferTo = ((*bufferFrom << 7) + (*(bufferFrom + 1) >> 1)) & 0x7FFF;
		bufferTo++; bufferFrom+= 2;
		*bufferTo++ = 0; 
		*bufferTo++ = 0; 
		*bufferTo++ = 0; 
		*bufferTo++ = 0; 
		bufferFrom += 2;
		#endif
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
      #ifndef PARAMETER_SELECTION
      TmpCut = -etaSous[i]-(cutValue[channel][i]) / BETA_FIXED;
      #else
      //TmpCut = -etaSous[i]-(cutValue[channel][i]) / Beta;
      switch (Beta)
      {
        case (1)  : TmpCut = -etaSous[i]-(cutValue[channel][i]) / 1  ; break;
        case (2)  : TmpCut = -etaSous[i]-(cutValue[channel][i]) / 2  ; break;
        case (4)  : TmpCut = -etaSous[i]-(cutValue[channel][i]) / 4  ; break;
        case (8)  : TmpCut = -etaSous[i]-(cutValue[channel][i]) / 8  ; break;
        case (16) : TmpCut = -etaSous[i]-(cutValue[channel][i]) / 16 ; break;
        case (32) : TmpCut = -etaSous[i]-(cutValue[channel][i]) / 32 ; break;                    
        case (64) : TmpCut = -etaSous[i]-(cutValue[channel][i]) / 64 ; break; 
        case (128): TmpCut = -etaSous[i]-(cutValue[channel][i]) / 128; break; 
        default :   TmpCut = -etaSous[i]-(cutValue[channel][i]) / 256; break;
      } 
      #endif
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
      #ifndef PARAMETER_SELECTION
      TmpCut = etaAdd[i]-(cutValue[channel][i]) / BETA_FIXED;
      #else
      //TmpCut = etaAdd[i]-(cutValue[channel][i]) / Beta;
      switch (Beta)
      {
        case (1)  : TmpCut = etaAdd[i]-(cutValue[channel][i]) / 1  ; break;
        case (2)  : TmpCut = etaAdd[i]-(cutValue[channel][i]) / 2  ; break;
        case (4)  : TmpCut = etaAdd[i]-(cutValue[channel][i]) / 4  ; break;
        case (8)  : TmpCut = etaAdd[i]-(cutValue[channel][i]) / 8  ; break;
        case (16) : TmpCut = etaAdd[i]-(cutValue[channel][i]) / 16 ; break;
        case (32) : TmpCut = etaAdd[i]-(cutValue[channel][i]) / 32 ; break;                    
        case (64) : TmpCut = etaAdd[i]-(cutValue[channel][i]) / 64 ; break; 
        case (128): TmpCut = etaAdd[i]-(cutValue[channel][i]) / 128; break; 
        default   : TmpCut = etaAdd[i]-(cutValue[channel][i]) / 256; break;
      }
      #endif      
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
					*bufferTo = CurrentValue;
					
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



















