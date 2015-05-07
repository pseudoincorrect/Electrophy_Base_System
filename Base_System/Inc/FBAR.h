#ifndef __FBAR_H__
#define __FBAR_H__

#include <stdint.h>
#include <math.h>
#include <CommonInclude.h>

void FBAR_Initialize(void);

void FBAR_Reinitialize(uint8_t * bufferFrom);

void FBAR_Uncompress(uint8_t * bufferFrom, uint16_t * bufferTo);

//void FBAR_AdaptCutValue(uint8_t channel, uint8_t winner);

void FBAR_Assemble(uint8_t * bufferFrom, uint16_t * bufferTo);

#endif
