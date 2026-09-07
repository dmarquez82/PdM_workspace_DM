# Práctica 5 - Punto 2.2: Recepción de línea completa

**Materia:** Programación de Microcontroladores
**Estudiante:** Prof. Ing. Daniel Márquez
**Placa:** NUCLEO-F446RE

## Objetivo

Sobre la base del módulo `API_uart` (Punto 1), acumular los caracteres recibidos
por UART en un buffer hasta detectar un terminador de línea (`\r`, `\n` o `\r\n`),
ignorando líneas que comiencen con `#` o `//` (comentarios), y delegar el
procesamiento de la línea completa a una función privada `cmdProcessLine()`.

## Archivos

```
Drivers/API/Inc/API_cmdparser.h
Drivers/API/Src/API_cmdparser.c
```

## `API_cmdparser.h` — definiciones

```c
#define CMD_MAX_LINE     64U   /* incluye '\0' */
#define CMD_MAX_TOKENS   3U    /* COMANDO + máximo 2 argumentos */

typedef enum {
    CMD_OK = 0,
    CMD_ERR_OVERFLOW,
    CMD_ERR_SYNTAX,
    CMD_ERR_UNKNOWN,
    CMD_ERR_ARG
} cmd_status_t;

void cmdParserInit(void);
void cmdPoll(void);
void cmdPrintHelp(void);
```

`CMD_MAX_LINE` incluye el carácter `'\0'` que marca el final de la línea, por lo
que la cantidad máxima de caracteres útiles de texto es `CMD_MAX_LINE - 1`.

## Variables privadas en `API_cmdparser.c`

```c
static bool_t recibiendo;
static uint8_t buffer[CMD_MAX_LINE];
static uint16_t bufferIndex;
```

- `recibiendo`: indica si se está en medio de la acumulación de una línea.
- `buffer`: acumula los caracteres recibidos.
- `bufferIndex`: posición actual de escritura dentro de `buffer`.

## Funciones

### `void cmdParserInit(void)`
Resetea el estado de recepción (`recibiendo = false`) y el índice del buffer.

### `void cmdPoll(void)`
Lee un byte por invocación mediante `uartReceiveStringSize(&c, 1)`. Si no llegó
ningún byte nuevo (se identifica porque `uartReceiveStringSize` deja el buffer en
`'\0'` ante timeout de la HAL), no hace nada en esa vuelta. Si el byte recibido es
un terminador (`\r` o `\n`) y se estaba recibiendo una línea, agrega el `'\0'`
final al buffer y llama a `cmdProcessLine()`. Si se llena el buffer antes de
encontrar el terminador, informa el error correspondiente y reinicia la
recepción.

### `static void cmdProcessLine(void)` *(privada)*
Si la línea comienza con `#` o `//`, se ignora (no se procesa). En esta etapa
(2.2), realiza un eco de la línea completa por UART, únicamente para validar que
el mecanismo de acumulación funciona correctamente. La interpretación real de
comandos se implementa en la etapa 2.3.

## Por qué se agrega `'\0'` al completar la línea

El buffer es un arreglo de tamaño fijo que puede contener contenido de usos
anteriores más allá de la línea actual. Al colocar `'\0'` justo después del
último carácter recibido, el buffer se convierte en un string válido según la
convención de C, permitiendo que funciones como `uartSendString` (que usa
`strlen` internamente) sepan exactamente dónde termina el contenido útil.

## Pruebas realizadas

- Línea normal (ej. `hola` + Enter): se refleja completa por UART.
- Línea que comienza con `#` o `//`: no genera ninguna respuesta.
- Línea de más de `CMD_MAX_LINE - 1` caracteres sin terminador: se informa el
  error de overflow y se reinicia la recepción.
