#include "API_can.h"

extern bool_t can_port_Init(void);
extern bool_t can_port_write(can_msg_t *message);

/**
 * @brief  Inicializa el driver CAN.
 * @param  Ninguno.
 * @retval bool_t: true si toda la inicialización de hardware fue
 *         exitosa, false si alguna etapa falló.
 */
bool_t can_Init(void)
{
    return can_port_Init();
}

/**
 * @brief  Envía un mensaje por el bus CAN.
 * @param  message: puntero a la estructura can_msg_t a transmitir.
 * @retval bool_t: true si el mensaje se encoló correctamente para
 *         transmitir, false si ocurrió un error.
 */
bool_t can_write_msg(can_msg_t *message)
{
    return can_port_write(message);
}

/**
 * @brief  Callback de recepción de mensajes CAN. Implementación
 *         __weak (vacía) por defecto: la aplicación debe redefinir
 *         esta misma función, sin __weak, para procesar el mensaje
 *         recibido según su lógica.
 * @param  message: puntero a la estructura can_msg_t recibida.
 * @retval Ninguno.
 */
__attribute__((weak)) void can_read_msg_callback(can_msg_t *message)
{
    // Implementación vacía por defecto.
}

/**
 * @brief  Uso interno del driver. La invoca API_can_port_stm32f4xx.c
 *         cuando el hardware recibe un mensaje, y esta a su vez
 *         invoca al callback de aplicación (can_read_msg_callback).
 * @param  message: puntero a la estructura can_msg_t recibida.
 * @retval Ninguno.
 */
void can_NotifyReception(can_msg_t *message)
{
    can_read_msg_callback(message);
}
