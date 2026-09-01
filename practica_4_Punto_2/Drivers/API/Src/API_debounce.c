#include "API_debounce.h"
#include "main.h"   /* para HAL_GPIO_ReadPin, LD2_GPIO_Port, B1_GPIO_Port, etc. */

#define DEBOUNCE_TIME_MS  40U

/* --- Declaraciones privadas --- */

typedef enum {
  BUTTON_UP,
  BUTTON_FALLING,
  BUTTON_DOWN,
  BUTTON_RISING,
} debounceState_t;

static debounceState_t estadoActual;
static delay_t debounceDelay;
static bool_t teclaPresionada;

static void buttonPressed(void);
static void buttonReleased(void);

/* --- Implementación pública --- */

/**
 * @brief  Inicializa la MEF de antirrebote en su estado inicial
 *         (BUTTON_UP) y prepara el retardo no bloqueante interno
 *         que usa la MEF para confirmar los flancos.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void debounceFSM_init(void)
{
  estadoActual = BUTTON_UP;
  teclaPresionada = false;
  delayInit(&debounceDelay, DEBOUNCE_TIME_MS);
}

/**
 * @brief  Actualiza la MEF de antirrebote: lee el estado del
 *         pulsador B1, resuelve las transiciones de estado
 *         correspondientes, y dispara los eventos internos
 *         buttonPressed/buttonReleased cuando corresponde. Debe
 *         llamarse periódicamente dentro del loop principal.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void debounceFSM_update(void)
{
  switch (estadoActual)
  {
    case BUTTON_UP:
      if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET)
      {
        estadoActual = BUTTON_FALLING;
      }
      break;

    case BUTTON_FALLING:
      if (delayRead(&debounceDelay))
      {
        if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET)
        {
          estadoActual = BUTTON_DOWN;
          buttonPressed();
        }
        else
        {
          estadoActual = BUTTON_UP;
        }
      }
      break;

    case BUTTON_DOWN:
      if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_SET)
      {
        estadoActual = BUTTON_RISING;
      }
      break;

    case BUTTON_RISING:
      if (delayRead(&debounceDelay))
      {
        if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_SET)
        {
          estadoActual = BUTTON_UP;
          buttonReleased();
        }
        else
        {
          estadoActual = BUTTON_DOWN;
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
  bool_t valor = teclaPresionada;
  teclaPresionada = false;
  return valor;
}

/* --- Implementación privada --- */

/**
 * @brief  Evento interno disparado al confirmarse una pulsación
 *         real del botón. Marca la bandera interna que readKey()
 *         reporta y resetea.
 * @param  Ninguno.
 * @retval Ninguno.
 */
static void buttonPressed(void)
{
  teclaPresionada = true;
}

/**
 * @brief  Evento interno disparado al confirmarse la liberación
 *         real del botón. Sin acción por el momento: readKey()
 *         solo reporta el flanco descendente.
 * @param  Ninguno.
 * @retval Ninguno.
 */
static void buttonReleased(void)
{
  /* Sin acción por ahora */
}
