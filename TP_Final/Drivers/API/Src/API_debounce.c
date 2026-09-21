#include "API_debounce.h"

#define DEBOUNCE_TIME_MS 40

typedef enum
{
    BUTTON_UP,
    BUTTON_FALLING,
    BUTTON_DOWN,
    BUTTON_RISING,
} debounceState_t;

static debounceState_t currentState;
static delay_t debounceDelay;
static bool_t keyPressed;

static void buttonPressed(void);
static void buttonReleased(void);

extern bool_t debounce_ReadButton(void);

/**
 * @brief  Inicializa la MEF de antirrebote en su estado inicial
 *         (BUTTON_UP) y prepara el retardo no bloqueante interno
 *         que usa la MEF para confirmar los flancos.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void debounceFSM_init(void)
{
    currentState = BUTTON_UP;
    keyPressed = false;
    delayInit(&debounceDelay, DEBOUNCE_TIME_MS);
}

/**
 * @brief  Actualiza la MEF de antirrebote: consulta si el botón
 *         está presionado a través de debounce_ReadButton(),
 *         resuelve las transiciones de estado correspondientes,
 *         y dispara los eventos internos buttonPressed/buttonReleased
 *         cuando corresponde. Debe llamarse periódicamente dentro
 *         del loop principal.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void debounceFSM_update(void)
{
    switch (currentState)
    {
        case BUTTON_UP:
            if (debounce_ReadButton() == true)
            {
                currentState = BUTTON_FALLING;
            }
            break;

        case BUTTON_FALLING:
            if (delayRead(&debounceDelay))
            {
                if (debounce_ReadButton() == true)
                {
                    currentState = BUTTON_DOWN;
                    buttonPressed();
                }
                else
                {
                    currentState = BUTTON_UP;
                }
            }
            break;

        case BUTTON_DOWN:
            if (debounce_ReadButton() == false)
            {
                currentState = BUTTON_RISING;
            }
            break;

        case BUTTON_RISING:
            if (delayRead(&debounceDelay))
            {
                if (debounce_ReadButton() == false)
                {
                    currentState = BUTTON_UP;
                    buttonReleased();
                }
                else
                {
                    currentState = BUTTON_DOWN;
                }
            }
            break;

        default:
            debounceFSM_init();
            break;
    }
}

/**
 * @brief  Lee y resetea la bandera interna que indica si ocurrió
 *         una pulsación confirmada (flanco descendente) desde la
 *         última vez que se llamó a esta función.
 * @param  Ninguno.
 * @retval bool_t true si hubo una pulsación pendiente de leer
 *         (y la resetea a false); false en caso contrario.
 */
bool_t readKey(void)
{
    bool_t value = keyPressed;
    keyPressed = false;
    return value;
}

static void buttonPressed(void)
{
    keyPressed = true;
}

static void buttonReleased(void)
{
    /* Sin acción por ahora */
}
