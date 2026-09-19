#include "API_debounce.h"
#include "main.h" /* para HAL_GPIO_ReadPin, B1_GPIO_Port, B1_Pin */

/**
 * @brief  Lee el estado físico del pulsador B1 y lo traduce a un
 *         booleano genérico. El botón es activo en bajo (pull-up
 *         interno), por lo que GPIO_PIN_RESET significa "presionado" —
 *         esa inversión queda oculta acá, no en la MEF.
 * @param  Ninguno.
 * @retval bool_t true si el botón está físicamente presionado,
 *         false en caso contrario.
 */
bool_t debounce_ReadButton(void)
{
    if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET)
    {
        return true;
    }
    else
    {
        return false;
    }
}
