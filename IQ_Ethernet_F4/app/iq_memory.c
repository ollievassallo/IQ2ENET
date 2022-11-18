/*
 * iq_memory.c
 *
 *  Created on: Nov 17, 2022
 *      Author: Oliver Vassallo
 */
#include "iq_memory.h"

#define BIT_SET(REG, BIT)	((REG) |= (BIT))
#define BIT_CLR(REG, BIT)   ((REG) &= ~(BIT))

void IQ_DoubleBuffer_Init(IQ_DoubleBuffer_TypeDef* dbuff, uint8_t* data_array)
{
	dbuff->flag_process = 0;
	dbuff->flag_transmit = 0;

	dbuff->process.data_array 	= &data_array[0];
	dbuff->transmit.data_array 	= &data_array[PAYLOAD_SIZE];

	dbuff->process.i0 	= dbuff->process.data_array  + 0 * MEM_DMA_SIZE;
	dbuff->process.i1 	= dbuff->process.data_array  + 1 * MEM_DMA_SIZE;
	dbuff->process.q0 	= dbuff->process.data_array  + 2 * MEM_DMA_SIZE;
	dbuff->process.q1 	= dbuff->process.data_array  + 3 * MEM_DMA_SIZE;

	dbuff->transmit.i0 	= dbuff->transmit.data_array + 0 * MEM_DMA_SIZE;
	dbuff->transmit.i1 	= dbuff->transmit.data_array + 1 * MEM_DMA_SIZE;
	dbuff->transmit.q0 	= dbuff->transmit.data_array + 2 * MEM_DMA_SIZE;
	dbuff->transmit.q1 	= dbuff->transmit.data_array + 3 * MEM_DMA_SIZE;

	dbuff->buffer_state = IQ_SWAPPED_FALSE;
}

void IQ_Hardware_Init(IQ_Hardware_TypeDef* hw, uint8_t* process, uint8_t* tx)
{
	hw->dma->CR = 	((hw->channel  << DMA_SxCR_CHSEL_Pos)& DMA_SxCR_CHSEL 	)| \
					((DMA_SxCR_DBM) 										)| \
					((hw->priority << DMA_SxCR_PL_Pos) 	& DMA_SxCR_PL 		)| \
					((0b00 << DMA_SxCR_MSIZE_Pos) 		& DMA_SxCR_MSIZE 	)| \
					((0b00 << DMA_SxCR_PSIZE_Pos) 		& DMA_SxCR_PSIZE 	)| \
					((DMA_SxCR_MINC) 										)| \
					((DMA_SxCR_CIRC) 										)| \
					((0b00 << DMA_SxCR_DIR_Pos) 		& DMA_SxCR_DIR 		)| \
					(DMA_SxCR_TCIE);

	hw->dma->NDTR = MEM_DMA_SIZE;
	hw->dma->M0AR = (uint32_t)process;
	hw->dma->M1AR = (uint32_t)tx;
	hw->dma->PAR  = hw->spi->DR;

	NVIC_EnableIRQ(hw->irq);
}

void IQ_DoubleBuffer_Swap(IQ_DoubleBuffer_TypeDef* dbuff)
{
	// Swap the buffers
	uint8_t* temp_ptr = dbuff->process.data_array;
	dbuff->process.data_array 	= dbuff->transmit.data_array;
	dbuff->transmit.data_array = temp_ptr;

	// Reconfigure the pointers to point to the correct locations
	dbuff->process.i0 	= dbuff->process.data_array  + 0 * MEM_DMA_SIZE;
	dbuff->process.i1 	= dbuff->process.data_array  + 1 * MEM_DMA_SIZE;
	dbuff->process.q0 	= dbuff->process.data_array  + 2 * MEM_DMA_SIZE;
	dbuff->process.q1 	= dbuff->process.data_array  + 3 * MEM_DMA_SIZE;

	dbuff->transmit.i0 = dbuff->transmit.data_array + 0 * MEM_DMA_SIZE;
	dbuff->transmit.i1 = dbuff->transmit.data_array + 1 * MEM_DMA_SIZE;
	dbuff->transmit.q0 = dbuff->transmit.data_array + 2 * MEM_DMA_SIZE;
	dbuff->transmit.q1 = dbuff->transmit.data_array + 3 * MEM_DMA_SIZE;

	// Did we swap the buffers before?
	dbuff->buffer_state = (dbuff->buffer_state == IQ_SWAPPED_TRUE) ? IQ_SWAPPED_FALSE: IQ_SWAPPED_TRUE;
}

void IQ_SystemReset(IQ_DoubleBuffer_TypeDef* dbuff)
{
	IQ_Hardware_SetState(dbuff, IQ_STATE_DISABLE);

	dbuff->flag_process = 0;

	// Clear any pending Interrupt
	DMA1->LIFCR |= DMA_LIFCR_CTCIF3;
	DMA2->LIFCR |= DMA_LIFCR_CTCIF0 | DMA_LIFCR_CTCIF2 | DMA_LIFCR_CTCIF3;

	// Make sure buffers are now correctly setup
	if(dbuff->buffer_state == IQ_SWAPPED_TRUE)
	{
		IQ_DoubleBuffer_Swap(dbuff);
	}
}

void IQ_Hardware_SetState(IQ_DoubleBuffer_TypeDef* dbuff, IQ_State state)
{
	if(state == IQ_STATE_ENABLE)
	{
		// Enable All Hardware
		BIT_SET(dbuff->hw_i0.spi->CR1, SPI_CR1_SPE);
		BIT_SET(dbuff->hw_i1.spi->CR1, SPI_CR1_SPE);
		BIT_SET(dbuff->hw_q0.spi->CR1, SPI_CR1_SPE);
		BIT_SET(dbuff->hw_q1.spi->CR1, SPI_CR1_SPE);

		BIT_SET(dbuff->hw_i0.dma->CR, DMA_SxCR_EN);
		BIT_SET(dbuff->hw_i1.dma->CR, DMA_SxCR_EN);
		BIT_SET(dbuff->hw_q0.dma->CR, DMA_SxCR_EN);
		BIT_SET(dbuff->hw_q1.dma->CR, DMA_SxCR_EN);
	}
	else
	{
		// Disable All Hardware
		BIT_CLR(dbuff->hw_i0.spi->CR1, SPI_CR1_SPE);
		BIT_CLR(dbuff->hw_i1.spi->CR1, SPI_CR1_SPE);
		BIT_CLR(dbuff->hw_q0.spi->CR1, SPI_CR1_SPE);
		BIT_CLR(dbuff->hw_q1.spi->CR1, SPI_CR1_SPE);

		BIT_CLR(dbuff->hw_i0.dma->CR, DMA_SxCR_EN);
		BIT_CLR(dbuff->hw_i1.dma->CR, DMA_SxCR_EN);
		BIT_CLR(dbuff->hw_q0.dma->CR, DMA_SxCR_EN);
		BIT_CLR(dbuff->hw_q1.dma->CR, DMA_SxCR_EN);
	}
}

