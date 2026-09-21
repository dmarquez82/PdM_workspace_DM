#include "API_can.h"
#include "stm32f4xx_hal.h"

extern void can_NotifyReception(can_msg_t *message);
extern CAN_HandleTypeDef hcan1;

static CAN_RxHeaderTypeDef rxHeader;
static uint8_t rxData[8];

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
    CAN_FilterTypeDef filterConfig;

    filterConfig.FilterBank = 0;
    filterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    filterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    filterConfig.FilterIdHigh = 0x0000;
    filterConfig.FilterIdLow = 0x0000;
    filterConfig.FilterMaskIdHigh = 0x0000;
    filterConfig.FilterMaskIdLow = 0x0000;
    filterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    filterConfig.FilterActivation = ENABLE;
    filterConfig.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(&hcan1, &filterConfig) != HAL_OK)
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
 * @param  message: puntero a la estructura can_msg_t a transmitir.
 * @retval bool_t: true si HAL_CAN_AddTxMessage devolvió HAL_OK, false
 *         si no.
 */
bool_t can_port_write(can_msg_t *message)
{
    CAN_TxHeaderTypeDef txHeader;
    uint32_t txMailbox;

    txHeader.StdId = message->id;
    txHeader.IDE = CAN_ID_STD;
    txHeader.RTR = CAN_RTR_DATA;
    txHeader.DLC = message->length;

    if (HAL_CAN_AddTxMessage(&hcan1, &txHeader, message->data, &txMailbox) != HAL_OK)
    {
        return false;
    }

    return true;
}

/**
 * @brief  Callback de HAL invocado automáticamente cuando llega un
 *         mensaje al FIFO0. Lee el mensaje, lo traduce a can_msg_t
 *         y notifica al driver genérico mediante can_NotifyReception.
 * @param  hcan: puntero al handle del periférico CAN que generó la
 *         interrupción (lo pasa HAL internamente).
 * @retval Ninguno.
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    can_msg_t receivedMessage;

    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxData) == HAL_OK)
    {
        receivedMessage.id = rxHeader.StdId;
        receivedMessage.length = rxHeader.DLC;

        for (uint8_t i = 0; i < rxHeader.DLC; i++)
        {
            receivedMessage.data[i] = rxData[i];
        }

        can_NotifyReception(&receivedMessage);
    }
}
