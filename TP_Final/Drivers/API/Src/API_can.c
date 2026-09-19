#include "API_can.h"

// Declaraciones internas del driver: no forman parte de API_can.h,
// por lo tanto no son visibles para main.c ni para otros módulos.
extern uint8_t can_port_Init(void);
extern uint8_t can_port_write(can_msg_t *mensaje);

/**
 * @brief  Inicializa el driver CAN.
 *         Delega la configuración específica de hardware (filtro,
 *         arranque del periférico y activación de la interrupción
 *         de recepción) en la capa de port.
 * @param  Ninguno.
 * @retval uint8_t: 1 si toda la inicialización de hardware fue
 *         exitosa, 0 si alguna etapa falló.
 */
uint8_t can_Init(void)
{
    return can_port_Init();
}

/**
 * @brief  Envía un mensaje por el bus CAN.
 * @param  mensaje: puntero a la estructura can_msg_t con el ID,
 *         los datos y la longitud (en bytes) a transmitir.
 * @retval uint8_t: 1 si el mensaje se encoló correctamente para
 *         transmitir, 0 si ocurrió un error.
 */
uint8_t can_write_msg(can_msg_t *mensaje)
{
    return can_port_write(mensaje);
}

/**
 * @brief  Callback de recepción de mensajes CAN.
 *         Se ejecuta automáticamente cada vez que llega un mensaje
 *         nuevo al bus. Implementación __weak (vacía) por defecto:
 *         la aplicación debe redefinir esta misma función, sin
 *         __weak, para procesar el mensaje recibido según su lógica.
 * @param  mensaje: puntero a la estructura can_msg_t con el ID,
 *         los datos y la longitud del mensaje recibido.
 * @retval Ninguno.
 */
__attribute__((weak)) void can_read_msg_callback(can_msg_t *mensaje)
{
    // Implementación vacía por defecto.
}

/**
 * @brief  Uso interno del driver. La invoca API_can_port_stm32f4xx.c
 *         cuando el hardware recibe un mensaje, y esta a su vez
 *         invoca al callback de aplicación (can_read_msg_callback).
 *         Existe para que la capa de port no dependa directamente
 *         del nombre del callback público.
 * @param  mensaje: puntero a la estructura can_msg_t recibida.
 * @retval Ninguno.
 */
void can_NotificarRecepcion(can_msg_t *mensaje)
{
    can_read_msg_callback(mensaje);
}
