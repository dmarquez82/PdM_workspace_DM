#include "API_ldr.h"
#include "main.h" /* para hadc1, ADC_HandleTypeDef y stm32f4xx_hal.h */

#define ADC_TIMEOUT_MS 10U

extern ADC_HandleTypeDef hadc1;

/**
 * @brief  Verifica que hadc1 (inicializada por MX_ADC1_Init(),
 *         generada por CubeMX) quedó correctamente configurada.
 * @param  Ninguno.
 * @retval bool_t: true si hadc1 apunta a ADC1, false en caso contrario.
 */
bool_t ldr_port_Init(void)
{
    if (hadc1.Instance != ADC1)
    {
        return false;
    }

    return true;
}

/**
 * @brief  Dispara una conversión, espera (polling) su finalización,
 *         y devuelve el resultado.
 * @param  Ninguno.
 * @retval uint16_t: valor de la conversión (0 a 4095), o 0 si
 *         ocurrió un error de arranque o timeout.
 */
uint16_t ldr_port_ReadValue(void)
{
    uint16_t value;

    if (HAL_ADC_Start(&hadc1) != HAL_OK)
    {
        return 0;
    }

    if (HAL_ADC_PollForConversion(&hadc1, ADC_TIMEOUT_MS) != HAL_OK)
    {
        HAL_ADC_Stop(&hadc1);
        return 0;
    }

    value = (uint16_t)HAL_ADC_GetValue(&hadc1);

    HAL_ADC_Stop(&hadc1);

    return value;
}
