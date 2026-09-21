#include "API_uart.h"
#include <string.h> /* para strlen */

#define UART_MAX_SIZE 256U

extern bool_t uart_port_Init(void);
extern bool_t uart_port_Transmit(uint8_t *pstring, uint16_t size);
extern bool_t uart_port_Receive(uint8_t *pstring, uint16_t size);

/**
 * @brief Verifica que la inicialización de la UART (realizada por
 *        MX_USART2_UART_Init(), generada por CubeMX) haya sido
 *        exitosa, y envía por la terminal serie un mensaje con los
 *        parámetros de configuración utilizados.
 * @param Ninguno.
 * @retval bool_t true si la UART está correctamente inicializada y
 *         el mensaje se envió con éxito; false en caso contrario.
 */
bool_t uartInit(void)
{
    uint8_t msg[] = "UART inicializada: 115200 bps - 8N1\r\n";

    if (uart_port_Init() == false)
    {
        return false;
    }

    if (uart_port_Transmit(msg, (uint16_t)strlen((char *)msg)) == false)
    {
        return false;
    }

    return true;
}

/**
 * @brief Envía por UART un string completo, hasta encontrar el
 *        carácter '\0'.
 * @param pstring Puntero al string a enviar (terminado en '\0').
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

    uart_port_Transmit(pstring, len);
}

/**
 * @brief Envía por UART una cantidad fija de caracteres, sin
 *        depender del terminador '\0'.
 * @param pstring Puntero al buffer a enviar.
 * @param size Cantidad de caracteres a enviar (1 a 256).
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

    uart_port_Transmit(pstring, size);
}

/**
 * @brief Recibe por UART una cantidad fija de caracteres, en modo
 *        polling (bloqueante hasta completar o hasta timeout).
 * @param pstring Puntero al buffer donde almacenar lo recibido.
 * @param size Cantidad de caracteres a recibir (1 a 256).
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

    if (uart_port_Receive(pstring, size) == false)
    {
        pstring[0] = '\0';
    }
}
