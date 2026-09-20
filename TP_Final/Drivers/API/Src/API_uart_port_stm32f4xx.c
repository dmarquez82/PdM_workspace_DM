#include "API_uart.h"
#include "main.h" /* para huart2, UART_HandleTypeDef y stm32f4xx_hal.h */

#define UART_TIMEOUT_MS 10U

extern UART_HandleTypeDef huart2;

/**
 * @brief  Verifica que huart2 (inicializada por MX_USART2_UART_Init(),
 *         generada por CubeMX) quedó correctamente configurada.
 *         No reinicializa el periférico: esa responsabilidad es de
 *         MX_USART2_UART_Init(), que ya se ejecutó antes.
 * @param  Ninguno.
 * @retval bool_t true si huart2 apunta a USART2, false en caso contrario.
 */
bool_t uart_port_Init(void)
{
    if (huart2.Instance != USART2)
    {
        return false;
    }

    return true;
}

/**
 * @brief  Transmite por UART2 en modo polling.
 * @param  pstring: puntero al buffer a enviar.
 * @param  size: cantidad de bytes a enviar.
 * @retval bool_t: true si HAL_OK, false en caso contrario.
 */
bool_t uart_port_Transmit(uint8_t *pstring, uint16_t size)
{
    if (HAL_UART_Transmit(&huart2, pstring, size, UART_TIMEOUT_MS) != HAL_OK)
    {
        return false;
    }

    return true;
}

/**
 * @brief  Recibe por UART2 en modo polling.
 * @param  pstring: puntero al buffer donde almacenar lo recibido.
 * @param  size: cantidad de bytes a recibir.
 * @retval bool_t: true si HAL_OK, false en caso contrario.
 */
bool_t uart_port_Receive(uint8_t *pstring, uint16_t size)
{
    if (HAL_UART_Receive(&huart2, pstring, size, UART_TIMEOUT_MS) != HAL_OK)
    {
        return false;
    }

    return true;
}
