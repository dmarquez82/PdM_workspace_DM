#include "mef_keypad.h"
#include "main.h" /* para HAL_GPIO_WritePin, LD2_GPIO_Port, LD2_Pin */

/**
 * @brief  Enciende el LED de confirmación del teclado (LD2), al
 *         recibirse el ACK de activación desde la caja de relés.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void keypadPort_TurnOnLed(void)
{
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
}

/**
 * @brief  Apaga el LED de confirmación del teclado (LD2), al
 *         recibirse el ACK de desactivación desde la caja de relés.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void keypadPort_TurnOffLed(void)
{
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
}
