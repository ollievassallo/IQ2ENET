/*
 * app.h
 *
 *  Created on: 6 Nov 2022
 *      Author: Oliver Vassallo
 */

#ifndef APP_MAIN_H_
#define APP_MAIN_H_

#include <string.h>

#include "lwip.h"
#include "udpServerRAW.h"
#include "iq_memory.h"


#define DATA_SOURCE_SPI

typedef enum
{
	IQ_CMD_Config = 1,
	IQ_CMD_Start,
	IQ_CMD_Stop,
	IQ_CMD_Reset
}IQ_CMD_Type;

typedef enum
{
	IQ_CTRL_FSM_Wait = 0,	// No Ethernet Connection is setup between the MCU and a PC
	IQ_CTRL_FSM_Idle,		// Ethernet Connection Configured between MCU and PC but System is not Logging Data
	IQ_CTRL_FSM_Log			// MCU Starts logging data to PC
}IQ_CTRL_State;

void App_Init();
void App_Runtime();

void App_Testing();

#endif /* APP_MAIN_H_ */
