/*
 * iq_memory.h
 *
 *  Created on: Nov 17, 2022
 *      Author: Oliver Vassallo
 */

#ifndef IQ_MEMORY_H_
#define IQ_MEMORY_H_

#include "stm32f429xx.h"
#include <stdio.h>

#define PAYLOAD_SIZE 		1024
#define MEM_BUFFER_SIZE 	(PAYLOAD_SIZE << 1) // BUFFER Length is twice the PAYLOAD Length due to double buffering
#define MEM_DMA_SIZE		(PAYLOAD_SIZE >> 2) // Each DMA has a quarter of the PAYLOAD size assigned to it

typedef struct
{
	uint8_t* data_array;
	uint8_t* i0;
	uint8_t* i1;
	uint8_t* q0;
	uint8_t* q1;
}IQ_Data_TypeDef;

typedef struct
{
	SPI_TypeDef* 		spi;
	DMA_Stream_TypeDef* dma;
	uint8_t 			channel;
	uint8_t 			priority;
	IRQn_Type 			irq;
}IQ_Hardware_TypeDef;

typedef enum
{
	IQ_STATE_DISABLE = 0,
	IQ_STATE_ENABLE = 1
}IQ_State;

typedef enum
{
	IQ_SWAPPED_FALSE = 0,
	IQ_SWAPPED_TRUE
}IQ_Swapped_State;

typedef struct
{
	uint8_t 			flag_process;
	uint8_t 			flag_transmit;

	IQ_Data_TypeDef 	process;
	IQ_Data_TypeDef 	transmit;
	IQ_Swapped_State 	buffer_state;

	IQ_Hardware_TypeDef hw_i0;
	IQ_Hardware_TypeDef hw_i1;
	IQ_Hardware_TypeDef hw_q0;
	IQ_Hardware_TypeDef hw_q1;
}IQ_DoubleBuffer_TypeDef;

void IQ_DoubleBuffer_Init(IQ_DoubleBuffer_TypeDef* dbuff, uint8_t* data_array);
void IQ_Hardware_Init(IQ_Hardware_TypeDef* hw, uint8_t* process, uint8_t* tx);

void IQ_SystemReset(IQ_DoubleBuffer_TypeDef* dbuff);

void IQ_DoubleBuffer_Swap(IQ_DoubleBuffer_TypeDef* dbuff);
void IQ_Hardware_SetState(IQ_DoubleBuffer_TypeDef* dbuff, IQ_State state);
#endif /* IQ_MEMORY_H_ */
