#include "API_cmdparser.h"
#include "API_uart.h"

static bool_t recibiendo;
static uint8_t buffer[CMD_MAX_LINE];
static uint16_t bufferIndex;

static void cmdProcessLine(void);

/**
 * @brief  Inicializa el módulo parser de comandos: resetea el estado
 *         de recepción y el índice del buffer de línea.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void cmdParserInit(void)
{
  recibiendo = false;
  bufferIndex = 0U;
}

/**
 * @brief  Lee un byte de la UART (si hay alguno disponible) y lo
 *         acumula en el buffer de línea, hasta detectar un
 *         terminador (\r o \n), momento en el que llama a
 *         cmdProcessLine(). Si el buffer se llena antes de encontrar
 *         el terminador, informa el error CMD_ERR_OVERFLOW.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void cmdPoll(void)
{
  uint8_t c;
  cmd_status_t status;

  uartReceiveStringSize(&c, 1U);

  if (c == '\0')
  {
	  /* no llegó ningún byte nuevo en esta invocación */
	  return;
  }

  if (c == '\r' || c == '\n')
  {
    if (recibiendo)
    {
      buffer[bufferIndex] = '\0';
      cmdProcessLine();
      recibiendo = false;
      bufferIndex = 0U;
    }
    return;
  }

  if (!recibiendo)
  {
    recibiendo = true;
    bufferIndex = 0U;
  }

  if (bufferIndex >= (CMD_MAX_LINE - 1U))
  {
    status = CMD_ERR_OVERFLOW;
    uartSendString((uint8_t *)"ERROR: linea muy extensa\r\n");
    recibiendo = false;
    bufferIndex = 0U;
    return;
  }

  buffer[bufferIndex] = c;
  bufferIndex++;
}

/**
 * @brief  Procesa una línea completa recibida por UART. Ignora las
 *         líneas que comienzan con '#' o '//' (comentarios). En esta
 *         etapa (2.2) realiza un eco de la línea completa para
 *         validar la recepción; la interpretación de comandos se
 *         implementa en la etapa 2.3.
 * @param  Ninguno.
 * @retval Ninguno.
 */
static void cmdProcessLine(void)
{
  if (buffer[0] == '#')
  {
    return;   /* línea de comentario, se ignora */
  }

  if (buffer[0] == '/' && buffer[1] == '/')
  {
    return;   /* línea de comentario, se ignora */
  }

  uartSendString(buffer);
  uartSendString((uint8_t *)"\r\n");
}

/**
 * @brief  Imprime por UART la lista de comandos disponibles.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void cmdPrintHelp(void)
{
  /* Se completa en el punto 2.3 */
}
