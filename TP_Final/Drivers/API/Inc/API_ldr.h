#ifndef API_LDR_H
#define API_LDR_H

#include <stdint.h>
#include "API_delay.h" /* para bool_t */

/**
 * @brief  Inicializa el driver del LDR (verifica que el ADC1,
 *         inicializado por MX_ADC1_Init(), esté listo).
 * @param  Ninguno.
 * @retval bool_t: true si la inicialización fue exitosa.
 */
bool_t ldr_Init(void);

/**
 * @brief  Lee el valor actual del LDR mediante el ADC1, en modo
 *         polling (bloqueante, de muy corta duración).
 * @param  Ninguno.
 * @retval uint16_t: valor crudo de la conversión (0 a 4095, 12 bits).
 */
uint16_t ldr_LeerValor(void);

#endif
