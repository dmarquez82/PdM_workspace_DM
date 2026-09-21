#ifndef MEF_KEYPAD_H
#define MEF_KEYPAD_H

/**
 * @brief  Inicializa la MEF del teclado (pulsador + CAN) y el
 *         driver de antirrebote del que depende.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void mef_keypad_init(void);

/**
 * @brief  Actualiza la MEF del teclado: procesa el antirrebote,
 *         envía comandos por CAN al detectar una pulsación
 *         confirmada, y resuelve las transiciones de estado según
 *         la llegada (o no) del ACK correspondiente. Debe llamarse
 *         periódicamente dentro del loop principal.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void mef_keypad_update(void);

#endif
