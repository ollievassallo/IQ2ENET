/*
 * app.c
 *
 *  Created on: 6 Nov 2022
 *      Author: Oliver Vassallo
 */

#include <app_main.h>

uint8_t flag_1ms = 0;
uint8_t flag_1s = 0;

// Timer Variables
extern TIM_HandleTypeDef htim6;

// USART3 Variables
extern UART_HandleTypeDef huart3;
uint16_t counter = 0;

// Ethernet Variables
extern struct netif 	 gnetif;

// UDP Variables
extern struct udp_pcb 	*upcb;
extern IQ_CMD_Type		 flag_reg;
extern char 		 	 payload_buffer[100];
struct pbuf *txBuf;

// IQ Data Management data
uint8_t logging_buffer[MEM_BUFFER_SIZE + 2] = {0};
IQ_DoubleBuffer_TypeDef iq_data;

IQ_CTRL_State iq_ctrl_fsm_wait()
{
	if(flag_reg == IQ_CMD_Config)
	{
		// CONFIG was received
		char buf[100];
		int len = sprintf (buf,"[ STATE - Idle ] Waiting for Client's Start Command \n");

		txBuf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
		pbuf_take(txBuf, buf, len);

		udp_connect(upcb, &upcb->remote_ip, upcb->remote_port);
		udp_send(upcb, txBuf);
		pbuf_free(txBuf);

		HAL_GPIO_WritePin(LED_BLUE, GPIO_PIN_SET);
		return IQ_CTRL_FSM_Idle;
	}
	else
	{
		return IQ_CTRL_FSM_Wait;
	}
}

IQ_CTRL_State iq_ctrl_fsm_idle()
{
	if(flag_reg == IQ_CMD_Start)
	{
		char buf[100];
		int len = sprintf (buf,"[ STATE - Logging ] Data-Logging Procedure ... Started\n");

#if defined(DATA_SOURCE_SPI)
		IQ_SystemReset(&iq_data);
		IQ_Hardware_SetState(&iq_data, IQ_STATE_ENABLE);
#endif

		txBuf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
		pbuf_take(txBuf, buf, len);

		udp_send(upcb, txBuf);
		pbuf_free(txBuf);


		txBuf = pbuf_alloc(PBUF_TRANSPORT, 5, PBUF_RAM);

		HAL_GPIO_WritePin(LED_GREEN, GPIO_PIN_SET);
		counter = 0;
		return IQ_CTRL_FSM_Log;
	}
	else if(flag_reg == IQ_CMD_Reset)
	{
		char buf[100];
		int len = sprintf (buf,"[ STATE - Waiting ] Client Disconnected\n");

		txBuf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
		pbuf_take(txBuf, buf, len);
		udp_send(upcb, txBuf);
		pbuf_free(txBuf);

		udp_disconnect(upcb);

		HAL_GPIO_WritePin(LED_BLUE, GPIO_PIN_RESET);
		return IQ_CTRL_FSM_Wait;
	}
	else
	{
		return IQ_CTRL_FSM_Idle;
	}

}

IQ_CTRL_State iq_ctrl_fsm_log()
{
	if(flag_reg == IQ_CMD_Stop)
	{
#ifdef DATA_SOURCE_SPI
		IQ_Hardware_SetState(&iq_data, IQ_STATE_DISABLE);
#endif

		pbuf_free(txBuf);

		char buf[100];
		int len = sprintf (buf,"[ STATE - Idle ] Data-Logging Procedure Stopped ... Stopped\n");

		txBuf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
		pbuf_take(txBuf, buf, len);
		udp_send(upcb, txBuf);
		pbuf_free(txBuf);

		HAL_GPIO_WritePin(LED_GREEN, GPIO_PIN_RESET);

		return IQ_CTRL_FSM_Idle;
	}
	else
	{
#if defined(DATA_SOURCE_SPI)
		if(iq_data.flag_process == 0b1111)
#else
		if(flag_1s)
#endif
		{
			HAL_GPIO_WritePin(LED_RED, GPIO_PIN_SET);

			// Data is readily processed
			flag_1s = 0;
			counter++;
			// Swap the data
			IQ_DoubleBuffer_Swap(&iq_data);
			iq_data.transmit.data_array[0] = counter;
			// Transmit it
			txBuf = pbuf_alloc(PBUF_TRANSPORT, PAYLOAD_SIZE, PBUF_RAM);
			pbuf_take(txBuf, iq_data.transmit.data_array, PAYLOAD_SIZE);
			udp_send(upcb, txBuf);
			pbuf_free(txBuf);
			HAL_GPIO_WritePin(LED_RED, GPIO_PIN_RESET);
		}
		return IQ_CTRL_FSM_Log;
	}
}

void App_Init()
{
	// Configure the Double Buffer System that will be used
	IQ_DoubleBuffer_Init(&iq_data, logging_buffer);

#if defined(DATA_SOURCE_SPI)
	/*
	 * Configure SPI DMAs
	 */
	__HAL_RCC_DMA1_CLK_ENABLE();
	__HAL_RCC_DMA2_CLK_ENABLE();

	iq_data.hw_i0.spi 		= SPI1;
	iq_data.hw_i0.dma 		= DMA2_Stream2;
	iq_data.hw_i0.channel  	= 3;
	iq_data.hw_i0.priority 	= 3;
	iq_data.hw_i0.irq 		= DMA2_Stream2_IRQn;

	iq_data.hw_i1.spi 		= SPI2;
	iq_data.hw_i1.dma 		= DMA1_Stream3;
	iq_data.hw_i1.channel  	= 0;
	iq_data.hw_i1.priority 	= 1;
	iq_data.hw_i1.irq 		= DMA1_Stream3_IRQn;

	iq_data.hw_q0.spi 		= SPI4;
	iq_data.hw_q0.dma 		= DMA2_Stream0;
	iq_data.hw_q0.channel  	= 4;
	iq_data.hw_q0.priority 	= 2;
	iq_data.hw_q0.irq 		= DMA2_Stream0_IRQn;

	iq_data.hw_q1.spi 		= SPI5;
	iq_data.hw_q1.dma 		= DMA2_Stream3;
	iq_data.hw_q1.channel  	= 4;
	iq_data.hw_q1.priority 	= 0;
	iq_data.hw_q1.irq 		= DMA2_Stream3_IRQn;

	IQ_Hardware_Init(&iq_data.hw_i0, iq_data.process.i0, iq_data.transmit.i0);
	IQ_Hardware_Init(&iq_data.hw_i1, iq_data.process.i1, iq_data.transmit.i1);
	IQ_Hardware_Init(&iq_data.hw_q0, iq_data.process.q0, iq_data.transmit.q0);
	IQ_Hardware_Init(&iq_data.hw_q1, iq_data.process.q1, iq_data.transmit.q1);

#else
	// Set up the Memory location instead of the DMA writing data
	for(uint16_t i = 0; i < MEM_BUFFER_SIZE; i++)
	{
		if(i < MEM_DMA_SIZE)
		{
			logging_buffer[i] = 49;
		}
		else if(i < MEM_DMA_SIZE*2)
		{
			logging_buffer[i] = 50;
		}
		else if(i < MEM_DMA_SIZE*3)
		{
			logging_buffer[i] = 51;
		}
		else if(i < MEM_DMA_SIZE*4)
		{
			logging_buffer[i] = 52;
		}
		else if(i < MEM_DMA_SIZE*5)
		{
			logging_buffer[i] = 53;
		}
		else if(i < MEM_DMA_SIZE*6)
		{
			logging_buffer[i] = 54;
		}
		else if(i < MEM_DMA_SIZE*7)
		{
			logging_buffer[i] = 55;
		}
		else if(i < MEM_DMA_SIZE*8)
		{
			logging_buffer[i] = 56;
		}
	}
#endif

	// Initialise and start the UDP Server
	udpServer_init();

	htim6.Instance->PSC = 9000;	// Divide the clock by 90. 90MHz / 90 = 1MHz
	htim6.Instance->ARR = 1000;	// Divide the clock by 100. So 1MHz /100 = 100kHz

#ifndef DATA_SOURCE_SPI
	HAL_TIM_Base_Start(&htim6);
#endif
}

void App_Runtime()
{
	IQ_CTRL_State fsm_state = IQ_CTRL_FSM_Wait;

	while(1)
	{
		ethernetif_input(&gnetif);
		sys_check_timeouts();

#ifndef DATA_SOURCE_SPI
		if(READ_BIT(htim6.Instance->SR,TIM_SR_UIF))
		{
			CLEAR_BIT(htim6.Instance->SR, TIM_SR_UIF);
			HAL_GPIO_TogglePin(LED_RED);
			flag_1s = 1;
		}
#endif
		switch(fsm_state)
		{
			case IQ_CTRL_FSM_Wait: fsm_state = iq_ctrl_fsm_wait(); break;
			case IQ_CTRL_FSM_Idle: fsm_state = iq_ctrl_fsm_idle(); break;
			case IQ_CTRL_FSM_Log:  fsm_state = iq_ctrl_fsm_log();  break;
		}
	}
}

void App_Testing()
{
	IQ_SystemReset(&iq_data);
	IQ_Hardware_SetState(&iq_data, IQ_STATE_ENABLE);

	while(1)
	{
		if(iq_data.flag_process == 0b1111)
			HAL_GPIO_TogglePin(LED_RED)
	}
}
