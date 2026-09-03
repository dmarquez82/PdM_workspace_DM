#include "API_uart.h"
#include "main.h"     /* para huart2, UART_HandleTypeDef */
#include <string.h>   /* para strlen buscá en las carpetas include del sistema/estándar */

#define UART_MAX_SIZE      256U
#define UART_TIMEOUT_MS     100U

extern UART_HandleTypeDef huart2;

/**
 * @brief  Inicializa la UART (ya configurada por CubeMX en huart2) e
 *         imprime por la terminal serie un mensaje con sus parámetros
 *         de configuración.
 * @param  Ninguno.
 * @retval bool_t true si la inicialización fue exitosa, false en
 *         caso contrario.
 */
bool_t uartInit(void)
{
  uint8_t msg[] = "UART inicializada: 115200 baudios, 8N1\r\n";

  if (HAL_UART_Transmit(&huart2, msg, strlen((char *)msg), UART_TIMEOUT_MS) != HAL_OK)
  {
    return false;
  }

  return true;
}

/**
 * @brief  Envía por UART un string completo, hasta encontrar el
 *         carácter '\0'.
 * @param  pstring Puntero al string a enviar (terminado en '\0').
 * @retval Ninguno.
 */
void uartSendString(uint8_t * pstring)
{
  uint16_t len;

  if (pstring == NULL)
  {
    return;
  }

  len = (uint16_t)strlen((char *)pstring);

  if (len == 0U || len > UART_MAX_SIZE)
  {
    return;
  }

  HAL_UART_Transmit(&huart2, pstring, len, UART_TIMEOUT_MS);
}

/**
 * @brief  Envía por UART una cantidad fija de caracteres, sin
 *         depender del terminador '\0'.
 * @param  pstring Puntero al buffer a enviar.
 * @param  size    Cantidad de caracteres a enviar (1 a 256).
 * @retval Ninguno.
 */
void uartSendStringSize(uint8_t * pstring, uint16_t size)
{
  if (pstring == NULL)
  {
    return;
  }

  if (size == 0U || size > UART_MAX_SIZE)
  {
    return;
  }

  HAL_UART_Transmit(&huart2, pstring, size, UART_TIMEOUT_MS);
}

/**
 * @brief  Recibe por UART una cantidad fija de caracteres, en modo
 *         polling (bloqueante hasta completar o hasta timeout).
 * @param  pstring Puntero al buffer donde almacenar lo recibido.
 * @param  size    Cantidad de caracteres a recibir (1 a 256).
 * @retval Ninguno.
 */
void uartReceiveStringSize(uint8_t * pstring, uint16_t size)
{
  if (pstring == NULL)
  {
    return;
  }

  if (size == 0U || size > UART_MAX_SIZE)
  {
    return;
  }

  HAL_UART_Receive(&huart2, pstring, size, UART_TIMEOUT_MS);
}
