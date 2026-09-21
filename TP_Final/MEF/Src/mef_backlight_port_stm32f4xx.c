#include "mef_backlight.h"
#include "main.h" /* para HAL_GPIO_WritePin, LED_BACKLIGHT_GPIO_Port/Pin, LD2 (espejo de prueba) */

/**
 * @brief  Enciende el LED de backlight (ambiente oscuro detectado).
 *         Incluye espejo temporal en LD2 para probar sin el LED
 *         externo conectado.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void backlightPort_TurnOnLed(void)
{
    HAL_GPIO_WritePin(LED_BACKLIGHT_GPIO_Port, LED_BACKLIGHT_Pin, GPIO_PIN_SET);
    //HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
}

/**
 * @brief  Apaga el LED de backlight (ambiente claro detectado).
 *         Incluye espejo temporal en LD2 para probar sin el LED
 *         externo conectado.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void backlightPort_TurnOffLed(void)
{
    HAL_GPIO_WritePin(LED_BACKLIGHT_GPIO_Port, LED_BACKLIGHT_Pin, GPIO_PIN_RESET);
    //HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
}
