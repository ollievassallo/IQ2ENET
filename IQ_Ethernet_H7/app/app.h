/*
 * app.h
 *
 *  Created on: 6 Nov 2022
 *      Author: Oliver Vassallo
 */

#ifndef APP_H_
#define APP_H_

#include <string.h>

#include "lwip.h"
#include "udpServerRAW.h"

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

typedef struct
{

};

#define LED_GREEN 	LD1_GPIO_Port, LD1_Pin
#define LED_YELLOW 	LD2_GPIO_Port, LD2_Pin
#define LED_RED 	LD3_GPIO_Port, LD3_Pin

void App_Init();
void App_Runtime();

#endif /* APP_H_ */
