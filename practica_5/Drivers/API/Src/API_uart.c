#include "API_uart.h"
#include "main.h"     /* para huart2, UART_HandleTypeDef */
#include <string.h>   /* para strlen buscá en las carpetas include del sistema/estándar */

#define UART_MAX_SIZE      256U
#define UART_TIMEOUT_MS     100U

extern UART_HandleTypeDef huart2;

/**
 * @brief  Verifica que la inicialización de la UART (realizada por
 *         MX_USART2_UART_Init(), generada por CubeMX) haya sido
 *         exitosa, y envía por la terminal serie un mensaje con los
 *         parámetros de configuración utilizados.
 * @param  Ninguno.
 * @retval bool_t true si la UART está correctamente inicializada y
 *         el mensaje se envió con éxito; false en caso contrario.
 */
bool_t uartInit(void)
{
  uint8_t msg[] = "UART inicializada: 115200 baudios, 8N1\r\n";

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;

  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    return false;
  }

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
