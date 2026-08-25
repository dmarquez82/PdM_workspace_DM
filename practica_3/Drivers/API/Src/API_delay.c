#include "API_delay.h"
#include "stm32f4xx_hal.h"   /* para HAL_GetTick() */


/**
 * @brief  Inicializa una estructura de retardo no bloqueante.
 *         Carga la duración solicitada y deja el retardo detenido
 *         (running = false); el conteo del tiempo recién arranca
 *         en la primera llamada a delayRead.
 *
 * @param  delay    Puntero a la estructura delay_t a inicializar.
 * @param  duration Duración del retardo, en milisegundos.
 *
 * @retval Ninguno (void). Si delay es NULL, la función no hace nada.
 *         Si duration es 0, se ignora ese valor (no se carga) pero
 *         igual se deja el retardo inicializado con running = false.
 */
void delayInit(delay_t * delay, tick_t duration)
{
    if (delay == NULL)
    {
        return;   /* puntero inválido: no hacemos nada, evitamos crash */
    }

    delay->running = false;

    if (duration == 0)
    {
        return;   /* duración inválida: no tiene sentido un retardo de 0 ms */
    }

    delay->duration = duration;
}

/**
 * @brief  Verifica si se cumplió el tiempo configurado en un retardo
 *         no bloqueante. Debe llamarse repetidamente (polling) dentro
 *         del loop principal; no bloquea la ejecución en ningún caso.
 *
 * @param  delay Puntero a la estructura delay_t a evaluar.
 *
 * @retval bool_t  true  si se cumplió el tiempo configurado (y reinicia
 *                        el ciclo para la próxima llamada).
 *                  false si el tiempo todavía no se cumplió, si el
 *                        retardo recién arranca a correr, o si delay
 *                        es NULL (puntero inválido).
 */
bool_t delayRead(delay_t * delay)
{
    if (delay == NULL)
    {
        return false;   /* no hay retardo válido que reportar */
    }

    if (!delay->running)
    {
        delay->startTime = HAL_GetTick();
        delay->running = true;
        return false;
    }

    if ((HAL_GetTick() - delay->startTime) >= delay->duration)
    {
        delay->running = false;
        return true;
    }

    return false;
}

/**
 * @brief  Cambia la duración de un retardo ya existente, sin alterar
 *         su estado running actual (si estaba corriendo, sigue
 *         corriendo con la nueva duración).
 *
 * @param  delay    Puntero a la estructura delay_t a modificar.
 * @param  duration Nueva duración del retardo, en milisegundos.
 *
 * @retval Ninguno (void). Si delay es NULL, o si duration es 0,
 *         la función no hace nada y se conserva la duración anterior.
 */
void delayWrite(delay_t * delay, tick_t duration)
{
    if (delay == NULL)
    {
        return;
    }

    if (duration == 0)
    {
        return;   /* duración inválida: se conserva la anterior */
    }

    delay->duration = duration;
}



