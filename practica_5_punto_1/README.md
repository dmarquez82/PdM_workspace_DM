# Práctica 5 - Punto 1: Módulo API_uart

**Materia:** Programación de Microcontroladores
**Estudiante:** Prof. Ing. Daniel Márquez
**Placa:** NUCLEO-F446RE

## Objetivo

Implementar un módulo de software para el acceso a la UART (USART2, conectada al
puerto virtual COM del ST-Link) en modo polling, con funciones para inicializar el
periférico, enviar y recibir strings.

## Estructura del proyecto

```
Practica_5/
├── Core/
├── Drivers/
│   ├── API/
│   │   ├── Inc/
│   │   │   ├── API_delay.h
│   │   │   ├── API_debounce.h
│   │   │   └── API_uart.h
│   │   └── Src/
│   │       ├── API_delay.c
│   │       ├── API_debounce.c
│   │       └── API_uart.c
│   ├── BSP/
│   ├── CMSIS/
│   └── STM32F4xx_HAL_Driver/
```

Proyecto armado como copia de la Práctica 4 - Punto 2 (ya contaba con la estructura
`Drivers/API/Inc`/`Src` y el include path configurado).

## `API_uart.h` — Interfaz pública

```c
bool_t uartInit(void);
void uartSendString(uint8_t * pstring);
void uartSendStringSize(uint8_t * pstring, uint16_t size);
void uartReceiveStringSize(uint8_t * pstring, uint16_t size);
```

`bool_t` se reutiliza desde `API_delay.h` (incluido en `API_uart.h`), evitando
redefinir el typedef en cada módulo nuevo.

## Encapsulamiento en `API_uart.c`

```c
static UART_HandleTypeDef API_UART2;
```

El handle de la UART se declara como variable **privada** (`static`) del módulo,
en vez de depender de la `huart2` generada por CubeMX. De esta forma, ningún otro
archivo del proyecto (incluyendo `main.c`) necesita conocer ni manipular
directamente el handle de UART — toda la interacción se hace exclusivamente a
través de las 4 funciones públicas del módulo.

*Nota: al declarar un handle propio, es responsabilidad de `uartInit()` cargar
todos los parámetros de configuración (`Instance`, `BaudRate`, `WordLength`, etc.)
y llamar a `HAL_UART_Init()`, en lugar de depender de que
`MX_USART2_UART_Init()` (generado por CubeMX) lo haga automáticamente.*

## Detalle de funciones

### `bool_t uartInit(void)`
Configura los parámetros de `API_UART2` (115200 baudios, 8N1) y llama a
`HAL_UART_Init()`. Si la inicialización es exitosa, envía por la terminal serie un
mensaje informando la configuración utilizada. Devuelve `true` si tanto la
inicialización como el envío del mensaje fueron exitosos; `false` en caso
contrario.

### `void uartSendString(uint8_t * pstring)`
Envía por UART un string completo, calculando su longitud con `strlen` (hasta el
carácter `'\0'`). Valida que el puntero no sea `NULL` y que la longitud esté en el
rango permitido (1 a `UART_MAX_SIZE`). Verifica el retorno de `HAL_UART_Transmit`;
al ser una función `void`, no existe forma de reportar el error hacia quien la
llama.

### `void uartSendStringSize(uint8_t * pstring, uint16_t size)`
Envía por UART una cantidad fija de caracteres (`size`), sin depender del
terminador `'\0'`. Misma validación de parámetros que `uartSendString`, y misma
verificación (sin acción posible) del retorno de `HAL_UART_Transmit`.

### `void uartReceiveStringSize(uint8_t * pstring, uint16_t size)`
Recibe por UART una cantidad fija de caracteres, en modo polling (bloqueante hasta
completar o hasta cumplirse el timeout). Valida los mismos parámetros. Si
`HAL_UART_Receive` no devuelve `HAL_OK`, fuerza `pstring[0] = '\0'` para dejar el
buffer en un estado conocido, en lugar de dejarlo con contenido parcial o
indefinido.

## Validación de parámetros

Las tres funciones de envío/recepción verifican:
- **Puntero `pstring` nulo:** si es `NULL`, la función retorna sin hacer nada.
- **`size`/longitud fuera de rango:** debe estar entre 1 y `UART_MAX_SIZE` (256,
  valor ajustable definido como macro). Fuera de ese rango, la función retorna sin
  hacer nada.

## Verificación de retorno de funciones HAL

Todas las llamadas a `HAL_UART_Transmit`/`HAL_UART_Receive` verifican su valor de
retorno contra `HAL_OK`. Como `uartSendString`/`uartSendStringSize`/
`uartReceiveStringSize` son funciones `void` (por especificación de la consigna),
no existe mecanismo para propagar el error hacia quien las llama; se deja
documentado en el código dónde se verifica el retorno y, cuando es posible
(`uartReceiveStringSize`), se toma una acción concreta ante el fallo.

## Constantes utilizadas

```c
#define UART_MAX_SIZE      256U
#define UART_TIMEOUT_MS     100U
```

Evitan hardcodear los valores de tamaño máximo y timeout en el cuerpo de las
funciones.

## Pendiente / a confirmar

Queda pendiente confirmar en el `.ioc` del proyecto que no exista una doble
inicialización del periférico USART2 (una por parte de `MX_USART2_UART_Init()`
generado por CubeMX sobre una eventual `huart2`, y otra por parte de `uartInit()`
sobre `API_UART2`), dado que ambos handles apuntarían al mismo periférico físico.
