#ifndef MEF_TABLERO_H
#define MEF_TABLERO_H

/**
 * @brief  Inicializa la MEF del tablero (pulsador + CAN) y el
 *         driver de antirrebote del que depende.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void tableroMef_init(void);

/**
 * @brief  Actualiza la MEF del tablero: procesa el antirrebote,
 *         envía comandos por CAN al detectar una pulsación
 *         confirmada, y resuelve las transiciones de estado según
 *         la llegada (o no) del ACK correspondiente. Debe llamarse
 *         periódicamente dentro del loop principal.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void tableroMef_update(void);

#endif
