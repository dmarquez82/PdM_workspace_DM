# Práctica 2: Delays no bloqueantes

**Materia:** Programación de Microcontroladores

**Estudiante:** Prof. Ing. Daniel Márquez

**Placa:** NUCLEO-F446RE

## Objetivo

Implementar un módulo de software para el manejo de retardos no bloqueantes basado en
`HAL_GetTick()`, y utilizarlo para generar patrones de parpadeo del LED de usuario (LD2)
sin bloquear la ejecución del programa principal.

## Estructura de datos

```c
typedef uint32_t tick_t;
typedef bool bool_t;

typedef struct {
    tick_t startTime;   // marca de tiempo en la que arrancó el conteo
    tick_t duration;    // duración configurada del retardo, en ms
    bool_t running;     // indica si el retardo está corriendo actualmente
} delay_t;
```

## Detalle de funciones

### `void delayInit(delay_t * delay, tick_t duration)`

```c
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
```

Inicializa una estructura `delay_t`: carga la duración solicitada y pone `running` en
`false`. **No** inicia el conteo del tiempo — eso ocurre recién en la primera llamada a
`delayRead`.

### `bool_t delayRead(delay_t * delay)`

```c
/**
 * @brief  Verifica si se cumplió el tiempo configurado en un retardo
 *         no bloqueante. Debe llamarse repetidamente (polling) dentro
 *         del loop principal; no bloquea la ejecución en ningún caso.
 *
 * @param  delay Puntero a la estructura delay_t a evaluar.
 *
 * @retval bool_t  true  si se cumplió el tiempo configurado (y reinicia
 *                        el ciclo para la próxima llamada).
 *                 false si el tiempo todavía no se cumplió, si el
 *                        retardo recién arranca a correr, o si delay
 *                        es NULL (puntero inválido).
 */
```

Función central del módulo, pensada para llamarse repetidamente dentro de un loop
(polling), sin bloquear:

- Si `running == false`: toma la marca de tiempo actual (`HAL_GetTick()`) como
  `startTime`, pone `running = true`, y devuelve `false` (recién arranca el conteo).
- Si `running == true`: compara `HAL_GetTick() - startTime` contra `duration`.
  - Si todavía no se cumplió el tiempo, devuelve `false`.
  - Si ya se cumplió, pone `running = false` (reinicia el ciclo para la próxima
    llamada) y devuelve `true`.

### `void delayWrite(delay_t * delay, tick_t duration)`

```c
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
```

Permite cambiar la duración de un delay ya existente, sin afectar su estado `running`
actual.

## Validación de parámetros

Las tres funciones controlan los parámetros recibidos antes de operar sobre ellos:

- **Puntero `delay` nulo:** todas verifican `delay == NULL` antes de acceder a sus
  campos. Si el puntero es inválido, `delayInit` y `delayWrite` no hacen nada
  (`return` inmediato), y `delayRead` devuelve `false`. Esto evita un acceso a
  memoria inválido (posible HardFault) si alguna función se llama por error sin
  una estructura válida.
- **`duration == 0`:** no tiene sentido físico un retardo de 0 ms, por lo que tanto
  `delayInit` como `delayWrite` ignoran ese valor y no lo cargan en la estructura.
  En `delayInit`, el flag `running` igualmente se deja en `false` para que la
  estructura no quede en un estado indefinido, aunque la duración no se haya
  podido cargar.

*Nota: esta validación se implementó y compiló correctamente, pero no se probó aún
de forma exhaustiva con parámetros inválidos (puntero NULL o duración 0) en la
placa física.*

## Explicación breve del código (`main.c`)

**Punto 2 — Blink 100 ms ON / 100 ms OFF:**

```c
delay_t estructura_delay;
delayInit(&estructura_delay, 100);

while (1)
{
  if (delayRead(&estructura_delay))
  {
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
  }
}
```



El `while(1)` da vueltas libremente sin ningún `HAL_Delay` bloqueante. `delayRead`
devuelve `true` únicamente en el instante puntual en que se cumplen los 100 ms
configurados; el `if` aprovecha esa señal para togglear el LED solo en ese momento,
evitando que se togglee en cada vuelta del loop.

**Punto 3 (opcional) — Patrón de 3 tandas:**

```c
uint32_t periodos[]     = {1000, 200, 100};
uint8_t  repeticiones[] = {5, 5, 5};
uint8_t  n_patrones = sizeof(periodos) / sizeof(periodos[0]);

delay_t estructura_delay;
uint8_t patronIdx = 0;
uint8_t cuenta_toggle = 0;

delayInit(&estructura_delay, periodos[patronIdx] / 2);

while (1)
{
  if (delayRead(&estructura_delay))
  {
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    cuenta_toggle++;

    /* Se multiplica por 2 ya que un blink son dos toggleos */
    if (cuenta_toggle >= repeticiones[patronIdx] * 2)
    {
      cuenta_toggle = 0;

      patronIdx++;
      if (patronIdx == n_patrones)
      {
        patronIdx = 0;
      }

      delayWrite(&estructura_delay, periodos[patronIdx] / 2);
    }
  }
}
```

- Se togglea el LED cada **medio período**, lo que genera automáticamente un duty
  cycle del 50% sin necesidad de manejar el tiempo ON y OFF por separado.
- `cuenta_toggle` cuenta los toggles dentro de la tanda actual. Como cada blink
  completo equivale a 2 toggles, se compara contra `repeticiones[patronIdx] * 2`.
- Al completarse la tanda, se avanza `patronIdx` a la siguiente posición del
  patrón, reiniciando a 0 si se llegó al final del arreglo (patrón cíclico), y se
  reconfigura el delay con `delayWrite` según el nuevo período.

## Patrón implementado (Punto 3)

| Tanda | Período | Ciclo de trabajo | Repeticiones |
|-------|---------|------------------|--------------|
| 1     | 1000 ms | 50%              | 5            |
| 2     | 200 ms  | 50%              | 5            |
| 3     | 100 ms  | 50%              | 5            |

El patrón se repite en forma indefinida.
