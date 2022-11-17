/*
 * app.c
 *
 *  Created on: 6 Nov 2022
 *      Author: Oliver Vassallo
 */

#include "app.h"

#define PAYLOAD_SIZE 1024

uint8_t flag_1ms 	= 0;
uint8_t flag_1s 	= 0;
extern struct udp_pcb 	*upcb;
extern IQ_CMD_Type		 flag_reg;
extern char 		 	 payload_buffer[100];
extern struct netif 	 gnetif;

uint8_t  dma_flag = 0;
uint8_t  logging_buffer[PAYLOAD_SIZE] = {0};
uint8_t* pData_I0 = &logging_buffer[0*(PAYLOAD_SIZE >> 2)];
uint8_t* pData_I1 = &logging_buffer[1*(PAYLOAD_SIZE >> 2)];
uint8_t* pData_Q0 = &logging_buffer[2*(PAYLOAD_SIZE >> 2)];
uint8_t* pData_Q1 = &logging_buffer[3*(PAYLOAD_SIZE >> 2)];

struct pbuf *txBuf;

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

		HAL_GPIO_WritePin(LED_YELLOW, GPIO_PIN_SET);

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

		txBuf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
		pbuf_take(txBuf, buf, len);

		udp_send(upcb, txBuf);
		pbuf_free(txBuf);


		txBuf = pbuf_alloc(PBUF_TRANSPORT, 5, PBUF_RAM);

		HAL_GPIO_WritePin(LED_GREEN, GPIO_PIN_SET);

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

		return IQ_CTRL_FSM_Wait;
	}
	else
	{
		return IQ_CTRL_FSM_Idle;
	}

}

IQ_CTRL_State iq_ctrl_fsm_log()
{
	static uint16_t tx_counter = 0;
	if(flag_reg == IQ_CMD_Stop)
	{
		pbuf_free(txBuf);

		char buf[100];
		int len = sprintf (buf,"[ STATE - Idle ] Data-Logging Procedure Stopped ... Stopped\n");

		txBuf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
		pbuf_take(txBuf, buf, len);
		udp_send(upcb, txBuf);
		pbuf_free(txBuf);

		tx_counter = 0;

		HAL_GPIO_WritePin(LED_GREEN, GPIO_PIN_RESET);

		return IQ_CTRL_FSM_Idle;
	}
	else
	{
		if(flag_1s)
		{
			flag_1s = 0;
/*			char buf[100];
			int len = sprintf (buf,"TX Counter: %4d\n", tx_counter);*/

			HAL_GPIO_WritePin(LED_GREEN, GPIO_PIN_SET);
			txBuf = pbuf_alloc(PBUF_TRANSPORT, PAYLOAD_SIZE, PBUF_RAM);
			pbuf_take(txBuf, logging_buffer, PAYLOAD_SIZE);
			udp_send(upcb, txBuf);
			pbuf_free(txBuf);
			HAL_GPIO_WritePin(LED_GREEN, GPIO_PIN_RESET);

		}
		return IQ_CTRL_FSM_Log;
	}
}

void App_Init()
{
	/*
	 * Configure SPI DMAs
	 */
	// Channel I-0


	// Channel I-1

	// Channel Q-0

	// Channel Q-1


	udpServer_init();

	//txBuf = pbuf_alloc(PBUF_TRANSPORT, 100, PBUF_RAM);
}

void App_Runtime()
{
	IQ_CTRL_State fsm_state = IQ_CTRL_FSM_Wait;
	uint16_t ms_counter = 0;

	while(1)
	{
		ethernetif_input(&gnetif);
		sys_check_timeouts();

		if(flag_1ms)
		{
			// Divide the Systick by 500
			flag_1ms = 0;
			if(ms_counter++ == 500)
			{
				ms_counter = 0;
				HAL_GPIO_TogglePin(LED_RED);
				flag_1s = 1;
			}
		}

		switch(fsm_state)
		{
			case IQ_CTRL_FSM_Wait: fsm_state = iq_ctrl_fsm_wait(); break;
			case IQ_CTRL_FSM_Idle: fsm_state = iq_ctrl_fsm_idle(); break;
			case IQ_CTRL_FSM_Log:  fsm_state = iq_ctrl_fsm_log();  break;
		}
	}
}

