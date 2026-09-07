# Práctica 5 - Punto 2.3: MEF completa con comandos mínimos

**Materia:** Programación de Microcontroladores
**Estudiante:** Prof. Ing. Daniel Márquez
**Placa:** NUCLEO-F446RE

## Objetivo

Completar el módulo `API_cmdparser` implementando la Máquina de Estados Finitos
(MEF) completa de 5 estados para recibir, tokenizar y ejecutar comandos por UART
en modo polling, sin bloquear el programa principal.

## Diagrama de estados

| Estado | Qué hace | Transición a |
|---|---|---|
| `CMD_IDLE` | Espera primer carácter no-terminador | `CMD_RECEIVING` |
| `CMD_RECEIVING` | Acumula caracteres en buffer | `CMD_PROCESS` (al recibir `\r`/`\n`) o `CMD_ERROR` (overflow) |
| `CMD_PROCESS` | Tokeniza, valida comando y argumentos | `CMD_EXEC` o `CMD_ERROR` |
| `CMD_EXEC` | Ejecuta la acción y vuelve a `CMD_IDLE` | `CMD_IDLE` |
| `CMD_ERROR` | Imprime mensaje de error y vuelve a `CMD_IDLE` | `CMD_IDLE` |

`cmd_state_t` se declara privada (`static`) en `API_cmdparser.c`, no se expone en
el `.h`.

## Variables privadas

```c
static cmd_state_t state;
static uint8_t buffer[CMD_MAX_LINE];
static uint16_t bufferIndex;
static cmd_status_t lastStatus;
static uint8_t * tokens[CMD_MAX_TOKENS];
static uint8_t tokenCount;
```

`tokens` y `tokenCount` se calculan en el estado `CMD_PROCESS` (una invocación de
`cmdPoll()`) y se usan recién en el estado `CMD_EXEC` (la invocación siguiente),
por lo que deben persistir como variables de archivo, igual que `state`.
`lastStatus` guarda el tipo de error ocurrido para que `CMD_ERROR` sepa qué
mensaje reportar.

## Comandos reconocidos (case-insensitive)

- `HELP` → llama a `cmdPrintHelp()`, que imprime la lista de comandos.
- `LED ON` / `LED OFF` / `LED TOGGLE` → controla el LED de usuario (LD2) con
  `HAL_GPIO_WritePin`/`HAL_GPIO_TogglePin` directamente (no existía en prácticas
  anteriores una función de más alto nivel para el LED que reutilizar).
- `STATUS` → responde `LED is ON` o `LED is OFF`, leyendo el estado real del pin
  con `HAL_GPIO_ReadPin` (en lugar de llevar una variable de estado separada, para
  evitar que se desincronice del estado real del hardware).

La comparación case-insensitive se realiza con `strcasecmp` (`<strings.h>`),
disponible en el toolchain del proyecto.

## Tokenizado

```c
static uint8_t cmdTokenize(uint8_t * line, uint8_t * tokens[]);
```

Recorre la línea recibida, ignorando espacios y tabs múltiples consecutivos, y va
guardando en `tokens[]` un puntero al inicio de cada palabra encontrada dentro
del mismo buffer original (sin copiar memoria), insertando un `'\0'` donde
estaba cada espacio separador. Devuelve la cantidad de tokens encontrados (hasta
`CMD_MAX_TOKENS`).

## Manejo de errores

En el estado `CMD_ERROR`, un `switch` sobre `lastStatus` (de tipo `cmd_status_t`)
decide el mensaje a enviar:

- `CMD_ERR_OVERFLOW` → `ERROR: line too long\r\n`
- `CMD_ERR_UNKNOWN` → `ERROR: unknown command\r\n`
- `CMD_ERR_ARG` / `CMD_ERR_SYNTAX` → `ERROR: bad arguments\r\n`

Todas las respuestas (éxito o error) terminan en `\r\n`.

## Por qué no bloquea el programa principal

`cmdPoll()` se llama repetidamente desde el `while(1)` de `main()`. En los
estados `CMD_IDLE`/`CMD_RECEIVING`, cada invocación intenta leer un solo byte
con un timeout muy corto en `API_uart` (`UART_TIMEOUT_MS = 5`); si no hay dato
disponible, la función retorna casi de inmediato sin bloquear el resto del
sistema. Los estados `CMD_PROCESS`, `CMD_EXEC` y `CMD_ERROR` no involucran
espera alguna: se resuelven por completo en una sola invocación una vez que la
línea ya está completa.

## Pruebas realizadas

- `HELP`, `LED ON`, `LED OFF`, `LED TOGGLE`, `STATUS` (mayúsculas y minúsculas):
  funcionan correctamente.
- `LED` sin argumento, o con argumento inválido (ej. `LED FOO`): responde
  `ERROR: bad arguments`.
- Comando inexistente (ej. `XYZ`): responde `ERROR: unknown command`.
- Línea que comienza con `#` o `//`: no genera respuesta.
- Línea de más de `CMD_MAX_LINE - 1` caracteres: responde `ERROR: line too long`.
