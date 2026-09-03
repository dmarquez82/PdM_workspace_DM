# Práctica 3: Modularización

**Materia:** Programación de Microcontroladores
**Estudiante:** Prof. Ing. Daniel Márquez
**Placa:** NUCLEO-F446RE

## Objetivo

Modularizar el código de retardos no bloqueantes desarrollado en la Práctica 2,
encapsulándolo en una API propia (`API_delay.c` / `API_delay.h`), e implementar una
secuencia de parpadeo con tiempos variables a partir de una tabla.

## Estructura del proyecto

```
Practica_3/
├── Core/
│   ├── Inc/        (main.h, stm32f4xx_hal_conf.h, etc.)
│   └── Src/        (main.c, stm32f4xx_it.c, etc.)
├── Drivers/
│   ├── API/
│   │   ├── Inc/
│   │   │   └── API_delay.h
│   │   └── Src/
│   │       └── API_delay.c
│   ├── BSP/
│   ├── CMSIS/
│   └── STM32F4xx_HAL_Driver/
```

*Nota:* la carpeta `Drivers/API/Inc` debe agregarse al **include path** del proyecto
(clic derecho sobre la carpeta → `Add/remove include path...`) para que el compilador
encuentre `API_delay.h` al incluirlo desde `main.c`.

## `API_delay.h` — Estructura de datos

```c
typedef uint32_t tick_t;
typedef bool bool_t;

typedef struct {
    tick_t startTime;
    tick_t duration;
    bool_t running;
} delay_t;
```

## Detalle de funciones

### `void delayInit(delay_t * delay, tick_t duration)`
Inicializa una estructura `delay_t`: carga la duración solicitada y pone `running` en
`false`. No inicia el conteo del tiempo — eso ocurre recién en la primera llamada a
`delayRead`.

### `bool_t delayRead(delay_t * delay)`
Función central del módulo, pensada para llamarse repetidamente dentro de un loop
(polling), sin bloquear:
- Si `running == false`: toma la marca de tiempo actual (`HAL_GetTick()`) como
  `startTime`, pone `running = true`, y devuelve `false`.
- Si `running == true`: compara `HAL_GetTick() - startTime` contra `duration`. Si se
  cumplió, pone `running = false` y devuelve `true`; si no, devuelve `false`.

### `void delayWrite(delay_t * delay, tick_t duration)`
Cambia la duración de un delay ya existente, sin alterar su `running` actual. Se
recomienda verificar con `delayIsRunning` que el delay no esté corriendo antes de
llamarla, para evitar alterar un conteo en curso con una duración inconsistente.

### `bool_t delayIsRunning(delay_t * delay)` *(nueva, Punto 3)*
Devuelve una copia del campo `running`, sin modificar la estructura. Permite consultar
desde afuera si un delay está corriendo antes de decidir reconfigurarlo con
`delayWrite`.

## Validación de parámetros

Las cuatro funciones controlan los parámetros recibidos antes de operar sobre ellos:

- **Puntero `delay` nulo:** se verifica `delay == NULL` antes de acceder a sus campos.
  Si es inválido, `delayInit` y `delayWrite` no hacen nada; `delayRead` y
  `delayIsRunning` devuelven `false`.
- **`duration == 0`:** no tiene sentido físico un retardo de 0 ms, por lo que
  `delayInit` y `delayWrite` ignoran ese valor y no lo cargan en la estructura.
  En `delayInit`, el flag `running` igual se deja en `false` para no dejar la
  estructura en un estado indefinido.

## Explicación del código (`main.c`) — Punto 2 y 3

Secuencia de tiempos de encendido, con duty 50%:

```c
const uint32_t TIEMPOS[] = {500, 100, 100, 1000};
uint8_t n_tiempos = sizeof(TIEMPOS) / sizeof(TIEMPOS[0]);
uint8_t idx = 0;
uint8_t cuenta_toggle = 0;

delay_t estructura_delay;
delayInit(&estructura_delay, TIEMPOS[idx]);

while (1)
{
  if (delayRead(&estructura_delay))
  {
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    cuenta_toggle++;

    if (cuenta_toggle >= 2)
    {
      cuenta_toggle = 0;

      idx++;
      if (idx == n_tiempos)
      {
        idx = 0;
      }

      if (!delayIsRunning(&estructura_delay))
      {
        delayWrite(&estructura_delay, TIEMPOS[idx]);
      }
    }
  }
}
```

- Se usa una **única variable `delay_t`** (`estructura_delay`) para toda la secuencia, tal como
  pide la consigna, reconfigurando su duración con `delayWrite` en lugar de crear un
  delay distinto por cada tiempo.
- `TIEMPOS[]` contiene el tiempo de **encendido** de cada tramo; como se pide duty
  50%, cada valor debe usarse dos veces seguidas (una para el ON, otra para el OFF)
  antes de pasar al siguiente. Por eso se cuenta con `cuenta_toggle` y solo se avanza
  `idx` cuando se completaron los 2 toggles (ON + OFF) del tramo actual.
- Antes de reconfigurar el tiempo con `delayWrite`, se verifica con
  `delayIsRunning(&estructura_delay)` que el delay no esté corriendo — en este punto del
  código nunca lo está (porque `delayRead` ya puso `running = false` al devolver
  `true`), pero se deja el chequeo como buena práctica defensiva, tal como pide el
  Punto 3.

## Nota sobre el uso de `static` en API_delay.c

No es necesario declarar nada como `static` en `API_delay.c`: no hay
variables ni funciones privadas, todos los tipos y funciones son públicos.

## Puntos para pensar (reflexión de la consigna)

- **Claridad de la consigna del Punto 2:** el enunciado indica que los tiempos son
  "de encendido" y que el duty debe ser 50%, pero no aclara explícitamente que cada
  tiempo deba repetirse para el ON y el OFF antes de avanzar al siguiente. Una lectura
  apresurada podría llevar a consumir la tabla linealmente (un valor por semiciclo),
  lo cual **no** cumple el 50% de duty para cada tramo — es el error que se detectó y
  corrigió durante la implementación.
- **Números mágicos:** el `2` usado para comparar `cuenta_toggle >= 2` representa "un
  blink completo son 2 toggles" — podría nombrarse con una constante para mayor
  claridad, aunque en este caso el comentario en el código cumple una función similar.
  El array `TIEMPOS[]` en sí es la forma correcta de evitar hardcodear los tiempos:
  cambiarlos o agregar un tramo nuevo solo requiere modificar esa única línea.
- **Bibliotecas estándar:** `<stdint.h>` (para `uint32_t`/`tick_t`) y `<stdbool.h>`
  (para `bool`/`bool_t`), incluidas en `API_delay.h`. Si la API creciera con más
  módulos que reutilicen estos mismos typedefs, convendría moverlos a un header común
  (por ejemplo `tipos.h`) para no obligar a otros módulos a depender de
  `API_delay.h` únicamente para acceder a `tick_t`/`bool_t`.
- **Control de parámetros:** las cuatro funciones de la API validan puntero `NULL` y
  `duration == 0` (ver sección de validación arriba). No se agregó validación sobre
  `idx` en `main.c` más allá del propio manejo cíclico del índice, dado que
  `n_tiempos` se calcula automáticamente a partir del tamaño del array.
