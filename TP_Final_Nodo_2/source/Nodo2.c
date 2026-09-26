/* Copyright 2026, DSI FCEIA UNR - Sistemas Digitales 2
 *    DSI: http://www.dsi.fceia.unr.edu.ar/
 * Copyright 2026, Agustin M. Zuliani (amzuliani02@gmail.com)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*==================[inclusions]=====================================*/
#include "Nodo2.h"
#include <stdint.h>
#include <stdbool.h>
#include "fsl_debug_console.h"
#include "mcp2515.h"
#include "can.h"
#include "SD2_board.h"

/*==================[macros and typedefs]============================*/
#define BASE_DE_TIEMPO_SYSTICK	1		// 1ms
#define MSG_CAN_NODO2_ACK_ID	0X200


/*==================[private declarations]===========================*/
static uint32_t BaseDeTiempo = 0;
static struct can_frame canMsg_Tx;
static struct can_frame canMsg_Rx;

/*==================[private functions]==============================*/
static void Nodo2_Led_Init();
static void Nodo2_LecturaBusCan();
static void Nodo2_EnviarAck();



static void Nodo2_Print_Init()
{
	PRINTF("NODO2: Iniciado.\n\r");
}

static void Nodo2_CAN_Init()
{
	ERROR_t error = ERROR_OK;
	uint8_t contadorErrores = 0;

	error = mcp2515_reset();
	if (error != ERROR_OK)	contadorErrores++;
	error = mcp2515_setBitrate(CAN_125KBPS, MCP_8MHZ);
	if (error != ERROR_OK)	contadorErrores++;
	error = mcp2515_setNormalMode();
	if (error != ERROR_OK)	contadorErrores++;

	if (!contadorErrores)
	{
		PRINTF("CAN: Iniciado correctamente.\r\n");
	}
	else
	{
		PRINTF("CAN: Error la iniciar el modulo can bus.\n\r");
	}
}

static void Nodo2_Led_Init()
{
	board_setLed(BOARD_LED_ID_AZUL, BOARD_LED_MSG_OFF);
	board_setLed(BOARD_LED_ID_ROJO, BOARD_LED_MSG_OFF);
}



static void Nodo2_EnviarAck()
{
	ERROR_t error = ERROR_OK;

	// Prepara el msg
	canMsg_Tx.can_id = MSG_CAN_NODO2_ACK_ID;
	canMsg_Tx.can_dlc = 1;
	canMsg_Tx.data[0] = canMsg_Rx.data[0];


	// Envia el msg
	error = mcp2515_sendMessage(&canMsg_Tx);

	if (error == ERROR_OK)
	{
		PRINTF("CAN SW : Mensaje enviado - ID: 0x%X, DLC: %d, Data[0]: 0x%X, "
				"Data[1]: 0x%X\r\n",
				canMsg_Tx.can_id,
				canMsg_Tx.can_dlc,
				canMsg_Tx.data[0],
				canMsg_Tx.data[1]);
	}
}


// Lectura por polling de mensajes CAN
static void Nodo2_LecturaBusCan()
{
	ERROR_t error = ERROR_OK;

	// Lee por polling los mensajes del modulo can
	error = mcp2515_readMessage(&canMsg_Rx);

	if (error != ERROR_OK)
	{
		return;
	}

}


/*==================[public functions]==============================*/
// Función principal de lectura de mensajes, acción de salidas y envío de mensaje de ACK
void Nodo2()
{
	Nodo2_Print_Init();
	Nodo2_CAN_Init();
	Nodo2_Led_Init();

	board_setLed(BOARD_LED_ID_ROJO, BOARD_LED_MSG_OFF);


	while(1)
	{
		Nodo2_LecturaBusCan();

		if (canMsg_Rx.can_id == 0x100)
		{

			Nodo2_EnviarAck();


			if (canMsg_Rx.data[0] == 0x01)
			{
				board_setLed(BOARD_LED_ID_ROJO, BOARD_LED_MSG_ON);
			}
			else if (canMsg_Rx.data[0] == 0x00)
			{
				board_setLed(BOARD_LED_ID_ROJO, BOARD_LED_MSG_OFF);
			}

			// Reseteo de ID recibido
			canMsg_Rx.can_id = 0x000;
		}


	}


	return;
}

void Nodo2_BaseDeTiempo()
{
	BaseDeTiempo++;

	return;
}
