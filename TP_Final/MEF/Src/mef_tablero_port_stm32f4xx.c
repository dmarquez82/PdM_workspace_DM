#include "mef_tablero.h"
#include "main.h" /* para HAL_GPIO_WritePin, LD2_GPIO_Port, LD2_Pin */

/**
 * @brief  Enciende el LED de confirmación del tablero (LD2), al
 *         recibirse el ACK de activación desde la caja de relés.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void tableroPort_EncenderLed(void)
{
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
}

/**
 * @brief  Apaga el LED de confirmación del tablero (LD2), al
 *         recibirse el ACK de desactivación desde la caja de relés.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void tableroPort_ApagarLed(void)
{
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
}
