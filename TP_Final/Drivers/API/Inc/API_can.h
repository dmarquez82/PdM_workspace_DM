#ifndef CAN_H
#define CAN_H

#include <stdint.h>

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
 * @retval Ninguno.
 */
void can_Init(void);

/**
 * @brief  Envía un mensaje por el bus CAN.
 * @param  mensaje: puntero a la estructura can_msg_t a transmitir.
 * @retval uint8_t: 1 si se encoló correctamente, 0 si hubo error.
 */
uint8_t can_write_msg(can_msg_t *mensaje);

/**
 * @brief  Callback de recepción. Implementación __weak (vacía) en
 *         can.c: la aplicación debe redefinirla (sin __weak) para
 *         procesar los mensajes que llegan por el bus.
 * @param  mensaje: puntero a la estructura can_msg_t recibida.
 * @retval Ninguno.
 */
void can_read_msg_callback(can_msg_t *mensaje);

#endif
