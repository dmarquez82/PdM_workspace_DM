#ifndef API_CAN_H
#define API_CAN_H

#include <stdint.h>
#include "API_delay.h" /* para bool_t */

/**
 * @brief Estructura genérica de un mensaje CAN, independiente del
 *        microcontrolador. La usan tanto la aplicación (para armar
 *        mensajes a enviar) como el driver (para entregar mensajes
 *        recibidos).
 */
typedef struct
{
    uint32_t id;       /*!< Identificador del mensaje (ID estándar, 11 bits) */
    uint8_t dato[8];    /*!< Datos del mensaje, hasta 8 bytes */
    uint8_t longitud;   /*!< Cantidad de bytes válidos en 'dato' (0 a 8) */
} can_msg_t;

/**
 * @brief  Inicializa el driver CAN.
 * @param  Ninguno.
 * @retval bool_t: true si toda la inicialización de hardware fue
 *         exitosa, false si alguna etapa falló.
 */
bool_t can_Init(void);

/**
 * @brief  Envía un mensaje por el bus CAN.
 * @param  mensaje: puntero a la estructura can_msg_t a transmitir.
 * @retval bool_t: true si el mensaje se encoló correctamente para
 *         transmitir, false si ocurrió un error.
 */
bool_t can_write_msg(can_msg_t *mensaje);

/**
 * @brief  Callback de recepción. Implementación __weak (vacía) en
 *         API_can.c: la aplicación debe redefinirla (sin __weak)
 *         para procesar los mensajes que llegan por el bus.
 * @param  mensaje: puntero a la estructura can_msg_t recibida.
 * @retval Ninguno.
 */
void can_read_msg_callback(can_msg_t *mensaje);

#endif
