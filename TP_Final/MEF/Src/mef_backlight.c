#include "mef_backlight.h"
#include "API_ldr.h"
#include "API_delay.h"

/* Umbrales de histéresis (0 a 4095, según divisor resistivo del LDR).
   UMBRAL_ALTO > UMBRAL_BAJO a propósito: valores de ejemplo, hay que
   calibrarlos con el sensor real y el ambiente donde se pruebe. */
#define UMBRAL_BAJO         1500U
#define UMBRAL_ALTO         2000U
#define PERIODO_LECTURA_MS  100U

typedef enum
{
    ESTADO_BACKLIGHT_APAGADO,
    ESTADO_BACKLIGHT_ENCENDIDO,
} estadoBacklight_t;

static estadoBacklight_t estadoActual;
static delay_t delayLectura;

// Declaraciones internas: las implementa mef_backlight_port_stm32f4xx.c
extern void backlightPort_EncenderLed(void);
extern void backlightPort_ApagarLed(void);

/**
 * @brief  Inicializa la MEF de backlight. Ver descripción en el header.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void backlightMef_init(void)
{
    estadoActual = ESTADO_BACKLIGHT_APAGADO;
    delayInit(&delayLectura, PERIODO_LECTURA_MS);
}

/**
 * @brief  Actualiza la MEF de backlight. Ver descripción en el header.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void backlightMef_update(void)
{
    uint16_t valorLdr;

    if (delayRead(&delayLectura) == false)
    {
        return;
    }

    /* Rearma el período apenas se cumple, sin importar si delayRead
       sigue devolviendo true en llamadas siguientes o no. */
    delayInit(&delayLectura, PERIODO_LECTURA_MS);

    valorLdr = ldr_LeerValor();

    switch (estadoActual)
    {
        case ESTADO_BACKLIGHT_APAGADO:
            if (valorLdr < UMBRAL_BAJO)
            {
                backlightPort_EncenderLed();
                estadoActual = ESTADO_BACKLIGHT_ENCENDIDO;
            }
            break;

        case ESTADO_BACKLIGHT_ENCENDIDO:
            if (valorLdr > UMBRAL_ALTO)
            {
                backlightPort_ApagarLed();
                estadoActual = ESTADO_BACKLIGHT_APAGADO;
            }
            break;

        default:
            estadoActual = ESTADO_BACKLIGHT_APAGADO;
            break;
    }
}
