#include "API_ldr.h"

extern bool_t ldr_port_Init(void);
extern uint16_t ldr_port_ReadValue(void);

/**
 * @brief  Inicializa el driver del LDR.
 * @param  Ninguno.
 * @retval bool_t: true si la inicialización de hardware fue exitosa.
 */
bool_t ldr_Init(void)
{
    return ldr_port_Init();
}

/**
 * @brief  Lee el valor actual del LDR.
 * @param  Ninguno.
 * @retval uint16_t: valor crudo de la conversión ADC (0 a 4095).
 */
uint16_t ldr_ReadValue(void)
{
    return ldr_port_ReadValue();
}
