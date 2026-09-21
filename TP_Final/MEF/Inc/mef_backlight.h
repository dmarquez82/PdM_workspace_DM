#ifndef MEF_BACKLIGHT_H
#define MEF_BACKLIGHT_H

/**
 * @brief  Inicializa la MEF de backlight en su estado inicial
 *         (apagado) e inicializa el driver del LDR.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void mef_backlight_init(void);

/**
 * @brief  Actualiza la MEF de backlight: cada READ_PERIOD_MS,
 *         lee el LDR y resuelve la transición de estado según los
 *         umbrales de histéresis. Debe llamarse periódicamente
 *         dentro del loop principal.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void mef_backlight_update(void);

#endif
