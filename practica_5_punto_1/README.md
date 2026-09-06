# Práctica 4 - Punto 2: Modularización del antirrebote (API_debounce)

**Materia:** Programación de Microcontroladores

**Estudiante:** Prof. Ing. Daniel Márquez

**Placa:** NUCLEO-F446RE

## Objetivo

Encapsular la MEF de antirrebote desarrollada en el Punto 1 en un módulo propio
(`API_debounce.c` / `API_debounce.h`), y utilizarlo junto con `API_delay` para
implementar una aplicación que cambia la frecuencia de parpadeo del LED entre 100 ms
y 500 ms cada vez que se presiona el pulsador B1.

## Estructura del proyecto

```
Practica_4_Punto_2/
├── Core/
│   ├── Inc/        (main.h, stm32f4xx_hal_conf.h, etc.)
│   └── Src/        (main.c, stm32f4xx_it.c, etc.)
├── Drivers/
│   ├── API/
│   │   ├── Inc/
│   │   │   ├── API_delay.h
│   │   │   └── API_debounce.h
│   │   └── Src/
│   │       ├── API_delay.c
│   │       └── API_debounce.c
│   ├── BSP/
│   ├── CMSIS/
│   └── STM32F4xx_HAL_Driver/
```

*Nota:* `Drivers/API/Inc` debe estar agregada al **include path** del proyecto para
que el compilador encuentre los headers al incluirlos desde `main.c`.

## `API_debounce.h` — Interfaz pública

```c
void debounceFSM_init(void);
void debounceFSM_update(void);
bool_t readKey(void);
```

Solo se exponen estas tres funciones. El tipo `debounceState_t` y todo el estado
interno de la MEF permanecen ocultos en `API_debounce.c`.

## Encapsulamiento en `API_debounce.c`

```c
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
```

- `debounceState_t` se declara **privada al archivo `.c`** (no está en el `.h`).
- `estadoActual`, `debounceDelay` y `teclaPresionada` son variables **globales
  privadas** (`static`): necesitan persistir entre llamadas sucesivas a
  `debounceFSM_update()` (por eso son globales de archivo, no locales a una
  función), y se marcan `static` para que ningún otro archivo del proyecto
  (`main.c`) pueda acceder a ellas directamente.
- `buttonPressed`/`buttonReleased` pasan a ser **funciones privadas** (`static`):
  solo tienen sentido llamadas internamente desde `debounceFSM_update`, y ya no
  manejan el LED directamente (a diferencia del Punto 1) — su única
  responsabilidad ahora es actualizar el estado interno que `readKey()` reporta.

## Detalle de funciones

### `void debounceFSM_init(void)`
Inicializa la MEF en su estado inicial (`BUTTON_UP`), resetea la bandera interna
`teclaPresionada`, y prepara el retardo no bloqueante interno (`debounceDelay`) con
la duración `DEBOUNCE_TIME_MS`. Al ser toda la MEF un detalle interno del módulo, es
esta función la que debe encargarse de inicializar también sus propios recursos
(el delay), ya que `main.c` no tiene acceso a ellos.

### `void debounceFSM_update(void)`
Debe llamarse periódicamente. Implementa el mismo `switch-case` sobre
`estadoActual` del Punto 1 (los 4 estados más el `default` de recuperación), leyendo
B1 y usando el retardo no bloqueante para confirmar los flancos. Al confirmarse una
pulsación o liberación, dispara los eventos internos `buttonPressed()` /
`buttonReleased()`.

### `bool_t readKey(void)`
Lee la bandera interna `teclaPresionada`: si estaba en `true` (hubo una pulsación
confirmada desde la última lectura), la resetea a `false` y devuelve `true`. Si no
hubo pulsación pendiente, devuelve `false`. Cada pulsación se "consume" una sola vez.

### `static void buttonPressed(void)` *(privada)*
Evento disparado al confirmarse el flanco descendente. Marca `teclaPresionada` en
`true` para que `readKey()` la reporte en la próxima consulta.

### `static void buttonReleased(void)` *(privada)*
Evento disparado al confirmarse el flanco ascendente. Sin acción por el momento:
`readKey()` solo reporta pulsaciones (flanco descendente), no liberaciones.

## Aplicación (`main.c`) — cambio de frecuencia del LED

```c
#include "API_debounce.h"
#include "API_delay.h"

#define LED_FREQ_FAST_MS  100U
#define LED_FREQ_SLOW_MS  500U

debounceFSM_init();

delay_t delayLed;
delayInit(&delayLed, LED_FREQ_FAST_MS);
bool_t frecuenciaRapida = true;

while (1)
{
  debounceFSM_update();

  if (readKey())
  {
    frecuenciaRapida = !frecuenciaRapida;

    if (frecuenciaRapida)
    {
      delayWrite(&delayLed, LED_FREQ_FAST_MS);
    }
    else
    {
      delayWrite(&delayLed, LED_FREQ_SLOW_MS);
    }
  }

  if (delayRead(&delayLed))
  {
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
  }
}
```

`main.c` no conoce ningún detalle interno de la MEF de antirrebote (ni el enum de
estados, ni el delay interno, ni las funciones privadas) — solo utiliza
`debounceFSM_update()` y `readKey()` como una caja negra, quedando la lógica de
parpadeo del LED **desacoplada** de la lógica de detección de la tecla. El módulo
`API_delay` se usa directamente y sin relación con `API_debounce`, mostrando que
ambas API son independientes entre sí.

## Puntos para pensar

- **Control de parámetros:** `API_delay` valida puntero `NULL` y `duration == 0` en
  sus tres funciones (ver README de la Práctica 2/3). `API_debounce` no recibe
  parámetros de usuario en ninguna de sus funciones públicas (todas son `void` o sin
  argumentos), por lo que no aplica validación de ese tipo en este módulo.
- **Constantes:** `DEBOUNCE_TIME_MS`, `LED_FREQ_FAST_MS` y `LED_FREQ_SLOW_MS` se
  definen como macros, evitando números hardcodeados en el código.
- **Tipos estándar:** se utilizan `uint32_t`/`tick_t` (definidos vía `stdint.h` en
  `API_delay.h`) y `bool_t` (vía `stdbool.h`) en toda la implementación.
