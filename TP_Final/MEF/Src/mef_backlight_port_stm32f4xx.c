#include "mef_backlight.h"
#include "main.h" /* para HAL_GPIO_WritePin, LED_BACKLIGHT_GPIO_Port, LED_BACKLIGHT_Pin */

/**
 * @brief  Enciende el LED de backlight (ambiente oscuro detectado).
 * @param  Ninguno.
 * @retval Ninguno.
 */
void backlightPort_EncenderLed(void)
{
    HAL_GPIO_WritePin(LED_BACKLIGHT_GPIO_Port, LED_BACKLIGHT_Pin, GPIO_PIN_SET);
}

/**
 * @brief  Apaga el LED de backlight (ambiente claro detectado).
 * @param  Ninguno.
 * @retval Ninguno.
 */
void backlightPort_ApagarLed(void)
{
    HAL_GPIO_WritePin(LED_BACKLIGHT_GPIO_Port, LED_BACKLIGHT_Pin, GPIO_PIN_RESET);
}
