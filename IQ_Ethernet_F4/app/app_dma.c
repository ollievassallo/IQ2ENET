/*
 * app_dma.c
 *
 *  Created on: Nov 18, 2022
 *      Author: Oliver Vassallo
 */
#include "app_dma.h"

extern IQ_DoubleBuffer_TypeDef iq_data;

void DMA2_Stream2_IRQHandler()
{
	// I0
	if(DMA2->LISR & DMA_LISR_TCIF2)
	{
		CLEAR_BIT(DMA2->LIFCR, DMA_LIFCR_CTCIF2);
		SET_BIT(iq_data.flag_process, 0);
	}
}

void DMA1_Stream3_IRQHandler()
{
	// I1
	if(DMA1->LISR & DMA_LISR_TCIF3)
	{
		CLEAR_BIT(DMA1->LIFCR, DMA_LIFCR_CTCIF3);
		SET_BIT(iq_data.flag_process, 1);
	}
}


void DMA2_Stream0_IRQHandler()
{
	// Q0
	if(DMA2->LISR & DMA_LISR_TCIF0)
	{
		CLEAR_BIT(DMA2->LIFCR, DMA_LIFCR_CTCIF0);
		SET_BIT(iq_data.flag_process, 2);
	}
}


void DMA2_Stream3_IRQHandler()
{
	// Q1
	if(DMA2->LISR & DMA_LISR_TCIF3)
	{
		CLEAR_BIT(DMA2->LIFCR, DMA_LIFCR_CTCIF3);
		SET_BIT(iq_data.flag_process, 3);
	}
}



