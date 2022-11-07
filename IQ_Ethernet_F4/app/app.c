/*
 * app.c
 *
 *  Created on: 6 Nov 2022
 *      Author: Oliver Vassallo
 */

#include "app.h"


uint8_t flag_1ms = 0;
uint8_t flag_1s = 0;
extern struct udp_pcb 	*upcb;
extern IQ_CMD_Type		 flag_reg;
extern char 		 	 payload_buffer[100];
extern struct netif 	 gnetif;


struct pbuf *txBuf;

IQ_CTRL_State iq_ctrl_fsm_wait()
{
	if(flag_reg == 1)
	{
		// CONFIG was received

		char *remoteIP 	= ipaddr_ntoa(&(upcb->remote_ip));
		char *localIP 	= ipaddr_ntoa(&(upcb->local_ip));

		char buf[100];
		int len = sprintf (buf,"Remote Device -> \t %s:%04d \nLocal  Device -> \t %s:%04d ... Connected\n", remoteIP, upcb->remote_port, localIP, upcb->local_port);

		txBuf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
		pbuf_take(txBuf, buf, len);

		udp_connect(upcb, &upcb->remote_ip, upcb->remote_port);
		udp_send(upcb, txBuf);
		pbuf_free(txBuf);


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
		int len = sprintf (buf,"Start data Logging\n");

		txBuf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
		pbuf_take(txBuf, buf, len);

		udp_send(upcb, txBuf);
		pbuf_free(txBuf);

		return IQ_CTRL_FSM_Log;
	}
	else if(flag_reg == IQ_CMD_Reset)
	{
		char *remoteIP 	= ipaddr_ntoa(&(upcb->remote_ip));
		char *localIP 	= ipaddr_ntoa(&(upcb->local_ip));

		char buf[100];
		int len = sprintf (buf,"Remote Device -> \t %s:%04d \nLocal  Device -> \t %s:%04d ... Disconnected\n", remoteIP, upcb->remote_port, localIP, upcb->local_port);

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
		char *remoteIP 	= ipaddr_ntoa(&(upcb->remote_ip));
		char buf[100];
		int len = sprintf (buf,"Logging Stopped by %s:%04d after %d Transmit cycle(s)\n", remoteIP, upcb->remote_port, tx_counter);

		txBuf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
		pbuf_take(txBuf, buf, len);
		udp_send(upcb, txBuf);
		pbuf_free(txBuf);


		tx_counter = 0;
		return IQ_CTRL_FSM_Idle;
	}
	else
	{
		if(flag_1s)
		{
			flag_1s = 0;
			if(++tx_counter > 9999)
			{
				tx_counter = 0;
			}
			char buf[100];
			int len = sprintf (buf,"TX Counter: %4d\n", tx_counter);

			txBuf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
			pbuf_take(txBuf, buf, len);
			udp_send(upcb, txBuf);
			pbuf_free(txBuf);

		}
		return IQ_CTRL_FSM_Log;
	}
}

void App_Init()
{
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
			// Divide the Systick by 1000
			flag_1ms = 0;
			if(ms_counter++ == 1000)
			{
				ms_counter = 0;
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

