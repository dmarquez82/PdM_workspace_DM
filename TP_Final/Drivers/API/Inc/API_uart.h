#ifndef API_INC_API_UART_H_
#define API_INC_API_UART_H_

#include <stdint.h>
#include "API_delay.h" /* para bool_t */

/**
 * @brief Verifica que la inicialización de la UART (realizada por
 *        MX_USART2_UART_Init(), generada por CubeMX) haya sido
 *        exitosa, y envía por la terminal serie un mensaje con los
 *        parámetros de configuración utilizados.
 * @param Ninguno.
 * @retval bool_t true si la UART está correctamente inicializada y
 *         el mensaje se envió con éxito; false en caso contrario.
 */
bool_t uartInit(void);

/**
 * @brief Envía por UART un string completo, hasta encontrar el
 *        carácter '\0'.
 * @param pstring Puntero al string a enviar (terminado en '\0').
 * @retval Ninguno.
 */
void uartSendString(uint8_t * pstring);

/**
 * @brief Envía por UART una cantidad fija de caracteres, sin
 *        depender del terminador '\0'.
 * @param pstring Puntero al buffer a enviar.
 * @param size Cantidad de caracteres a enviar (1 a 256).
 * @retval Ninguno.
 */
void uartSendStringSize(uint8_t * pstring, uint16_t size);

/**
 * @brief Recibe por UART una cantidad fija de caracteres, en modo
 *        polling (bloqueante hasta completar o hasta timeout).
 * @param pstring Puntero al buffer donde almacenar lo recibido.
 * @param size Cantidad de caracteres a recibir (1 a 256).
 * @retval Ninguno.
 */
void uartReceiveStringSize(uint8_t * pstring, uint16_t size);

#endif /* API_INC_API_UART_H_ */

