#include "API_can.h"
#include "stm32f4xx_hal.h"

// Declaración interna: solo la usa este archivo para notificar
// a API_can.c que llegó un mensaje. No forma parte de API_can.h.
extern void can_NotificarRecepcion(can_msg_t *mensaje);

extern CAN_HandleTypeDef hcan1;

static CAN_RxHeaderTypeDef rxHeader;
static uint8_t rxDatos[8];

/**
 * @brief  Configuración específica de hardware del periférico CAN1:
 *         filtro de recepción (acepta todos los ID), arranque del
 *         periférico y activación de la interrupción RX0.
 * @param  Ninguno.
 * @retval bool_t: true si las tres etapas devolvieron HAL_OK, false
 *         si alguna falló.
 */
bool_t can_port_Init(void)
{
    CAN_FilterTypeDef configFiltro;

    configFiltro.FilterBank = 0;
    configFiltro.FilterMode = CAN_FILTERMODE_IDMASK;
    configFiltro.FilterScale = CAN_FILTERSCALE_32BIT;
    configFiltro.FilterIdHigh = 0x0000;
    configFiltro.FilterIdLow = 0x0000;
    configFiltro.FilterMaskIdHigh = 0x0000;
    configFiltro.FilterMaskIdLow = 0x0000;
    configFiltro.FilterFIFOAssignment = CAN_RX_FIFO0;
    configFiltro.FilterActivation = ENABLE;
    configFiltro.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(&hcan1, &configFiltro) != HAL_OK)
    {
        return false;
    }

    if (HAL_CAN_Start(&hcan1) != HAL_OK)
    {
        return false;
    }

    if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
    {
        return false;
    }

    return true;
}

/**
 * @brief  Transmite un mensaje usando HAL_CAN_AddTxMessage, traduciendo
 *         la estructura genérica can_msg_t a los tipos que espera HAL.
 * @param  mensaje: puntero a la estructura can_msg_t a transmitir.
 * @retval bool_t: true si HAL_CAN_AddTxMessage devolvió HAL_OK, false
 *         si no.
 */
bool_t can_port_write(can_msg_t *mensaje)
{
    CAN_TxHeaderTypeDef txHeader;
    uint32_t txMailbox;

    txHeader.StdId = mensaje->id;
    txHeader.IDE = CAN_ID_STD;
    txHeader.RTR = CAN_RTR_DATA;
    txHeader.DLC = mensaje->longitud;

    if (HAL_CAN_AddTxMessage(&hcan1, &txHeader, mensaje->dato, &txMailbox) != HAL_OK)
    {
        return false;
    }

    return true;
}

/**
 * @brief  Callback de HAL invocado automáticamente cuando llega un
 *         mensaje al FIFO0. Lee el mensaje, lo traduce a can_msg_t
 *         y notifica al driver genérico mediante can_NotificarRecepcion.
 * @param  hcan: puntero al handle del periférico CAN que generó la
 *         interrupción (lo pasa HAL internamente).
 * @retval Ninguno.
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    can_msg_t mensajeRecibido;

    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxDatos) == HAL_OK)
    {
        mensajeRecibido.id = rxHeader.StdId;
        mensajeRecibido.longitud = rxHeader.DLC;

        for (uint8_t i = 0; i < rxHeader.DLC; i++)
        {
            mensajeRecibido.dato[i] = rxDatos[i];
        }

        can_NotificarRecepcion(&mensajeRecibido);
    }
}
