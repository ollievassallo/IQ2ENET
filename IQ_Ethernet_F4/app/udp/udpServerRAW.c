/*
  ***************************************************************************************************************
  ***************************************************************************************************************
  ***************************************************************************************************************

  File:		  udpServerRAW.c
  Author:     ControllersTech.com
  Updated:    Jul 23, 2021

  ***************************************************************************************************************
  Copyright (C) 2017 ControllersTech.com

  This is a free software under the GNU license, you can redistribute it and/or modify it under the terms
  of the GNU General Public License version 3 as published by the Free Software Foundation.
  This software library is shared with public for educational purposes, without WARRANTY and Author is not liable for any damages caused directly
  or indirectly by this software, read more about this on the GNU General Public License.

  ***************************************************************************************************************
*/


#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"

#include <string.h>
#include "stdio.h"
#include "udpServerRAW.h"


struct udp_pcb 	*upcb;
IQ_CMD_Type		 flag_reg;
char 			 payload_buffer[100];
void udp_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);


/* IMPLEMENTATION FOR UDP Server :   source:https://www.geeksforgeeks.org/udp-server-client-implementation-c/

1. Create UDP socket.
2. Bind the socket to server address.
3. Wait until datagram packet arrives from client.
4. Process the datagram packet and send a reply to client.
5. Go back to Step 3.
*/

void udpServer_init(void)
{
	// UDP Control Block structure
   err_t err;

   /* 1. Create a new UDP control block  */
   upcb = udp_new();

   /* 2. Bind the upcb to the local port */
   ip_addr_t myIPADDR;
   IP_ADDR4(&myIPADDR, 192, 168, 1, 195);

   err = udp_bind(upcb, &myIPADDR, 7);  // 7 is the server UDP port


   /* 3. Set a receive callback for the upcb */
   if(err == ERR_OK)
   {
	   udp_recv(upcb, udp_receive_callback, NULL);
   }
   else
   {
	   udp_remove(upcb);
   }
}

// udp_receive_callback will be called, when the client sends some data to the server
/* 4. Process the datagram packet and send a reply to client. */

void udp_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
//	struct pbuf *txBuf;


	// Set the IP Address and Port to the ones currenlty in use
	upcb->remote_ip.addr = addr->addr;
	upcb->remote_port = port;
	strncpy(payload_buffer, (char *)p->payload, p->len);

	if(strncmp(payload_buffer, "CONFIG", 6)==0)
	{
		flag_reg = IQ_CMD_Config;
	}
	else if(strncmp(payload_buffer, "START", 5)==0)
	{
		flag_reg = IQ_CMD_Start;
	}
	else if(strncmp(payload_buffer, "STOP", 4)==0)
	{
		flag_reg = IQ_CMD_Stop;
	}
	else if(strncmp(payload_buffer, "RESET", 5)==0)
	{
		flag_reg = IQ_CMD_Reset;
	}

	/* Get the IP of the Client */
//	char *remoteIP 	= ipaddr_ntoa(&(upcb->remote_ip));
//	char *localIP 	= ipaddr_ntoa(&(upcb->local_ip));

//	char buf[100];


//	int len = sprintf (buf,"Hello %s:%d from %s:%d\n", remoteIP, upcb->remote_port, localIP, upcb->local_port);
//
//	/* allocate pbuf from RAM*/
//	txBuf = pbuf_alloc(PBUF_TRANSPORT,len, PBUF_RAM);
//
//	/* copy the data into the buffer  */
//	pbuf_take(txBuf, buf, len);
//
//	/* Connect to the remote client */
//	udp_connect(upcb, addr, port);
//
//	/* Send a Reply to the Client */
//	udp_send(upcb, txBuf);
//
//	/* free the UDP connection, so we can accept new clients */
//	udp_disconnect(upcb);
//
//	/* Free the p_tx buffer */
//	pbuf_free(txBuf);
//
//	/* Free the p buffer */
//	pbuf_free(p);
}

