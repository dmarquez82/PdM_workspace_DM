#include "API_cmdparser.h"
#include "API_uart.h"
#include "main.h"
#include <strings.h>

typedef enum {
  CMD_IDLE,
  CMD_RECEIVING,
  CMD_PROCESS,
  CMD_EXEC,
  CMD_ERROR
} cmd_state_t;

static cmd_state_t state;
static uint8_t buffer[CMD_MAX_LINE];
static uint16_t bufferIndex;
static cmd_status_t lastStatus;
static uint8_t * tokens[CMD_MAX_TOKENS];
static uint8_t tokenCount;

static uint8_t cmdTokenize(uint8_t * line, uint8_t * tokens[]);

/**
 * @brief  Inicializa el módulo parser de comandos: carga el estado
 *         inicial de la MEF (CMD_IDLE) y resetea el índice del
 *         buffer de línea.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void cmdParserInit(void)
{
  state = CMD_IDLE;
  bufferIndex = 0U;
}

/**
 * @brief  Máquina de estados del parser de comandos. Lee un byte de
 *         la UART (si hay alguno disponible) y resuelve la
 *         transición de estados correspondiente: acumula caracteres
 *         hasta un terminador, tokeniza y valida el comando,
 *         ejecuta la acción, o informa un error. Debe llamarse
 *         periódicamente desde el bucle principal.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void cmdPoll(void)
{
  uint8_t c;

  switch (state)
  {
    case CMD_IDLE:
      uartReceiveStringSize(&c, 1U);

      if (c == '\0')
      {
        break;
      }

      if (c != '\r' && c != '\n')
      {
        bufferIndex = 0U;
        buffer[bufferIndex] = c;
        bufferIndex++;
        state = CMD_RECEIVING;
      }
      break;

    case CMD_RECEIVING:
      uartReceiveStringSize(&c, 1U);

      if (c == '\0')
      {
        break;
      }

      if (c == '\r' || c == '\n')
      {
        buffer[bufferIndex] = '\0';
        state = CMD_PROCESS;
      }
      else if (bufferIndex >= (CMD_MAX_LINE - 1U))
      {
        lastStatus = CMD_ERR_OVERFLOW;
        state = CMD_ERROR;
      }
      else
      {
        buffer[bufferIndex] = c;
        bufferIndex++;
      }
      break;

    case CMD_PROCESS:
      if (buffer[0] == '#' || (buffer[0] == '/' && buffer[1] == '/'))
      {
        state = CMD_IDLE;
        break;
      }

      tokenCount = cmdTokenize(buffer, tokens);

      if (tokenCount == 0U)
      {
        state = CMD_IDLE;
        break;
      }

      if (strcasecmp((char *)tokens[0], "HELP") == 0)
      {
        lastStatus = CMD_OK;
        state = CMD_EXEC;
      }
      else if (strcasecmp((char *)tokens[0], "STATUS") == 0)
      {
        lastStatus = CMD_OK;
        state = CMD_EXEC;
      }
      else if (strcasecmp((char *)tokens[0], "LED") == 0)
      {
        if (tokenCount != 2U)
        {
          lastStatus = CMD_ERR_ARG;
          state = CMD_ERROR;
        }
        else if (strcasecmp((char *)tokens[1], "ON") != 0 &&
                 strcasecmp((char *)tokens[1], "OFF") != 0 &&
                 strcasecmp((char *)tokens[1], "TOGGLE") != 0)
        {
          lastStatus = CMD_ERR_ARG;
          state = CMD_ERROR;
        }
        else
        {
          lastStatus = CMD_OK;
          state = CMD_EXEC;
        }
      }
      else
      {
        lastStatus = CMD_ERR_UNKNOWN;
        state = CMD_ERROR;
      }
      break;

    case CMD_EXEC:
      if (strcasecmp((char *)tokens[0], "HELP") == 0)
      {
        cmdPrintHelp();
      }
      else if (strcasecmp((char *)tokens[0], "STATUS") == 0)
      {
        if (HAL_GPIO_ReadPin(LD2_GPIO_Port, LD2_Pin) == GPIO_PIN_SET)
        {
          uartSendString((uint8_t *)"LED is ON\r\n");
        }
        else
        {
          uartSendString((uint8_t *)"LED is OFF\r\n");
        }
      }
      else if (strcasecmp((char *)tokens[0], "LED") == 0)
      {
        if (strcasecmp((char *)tokens[1], "ON") == 0)
        {
          HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
        }
        else if (strcasecmp((char *)tokens[1], "OFF") == 0)
        {
          HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
        }
        else
        {
          HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
        }
        uartSendString((uint8_t *)"OK\r\n");
      }
      state = CMD_IDLE;
      break;

    case CMD_ERROR:
      switch (lastStatus)
      {
        case CMD_ERR_OVERFLOW:
          uartSendString((uint8_t *)"ERROR: line too long\r\n");
          break;

        case CMD_ERR_UNKNOWN:
          uartSendString((uint8_t *)"ERROR: unknown command\r\n");
          break;

        case CMD_ERR_ARG:
        case CMD_ERR_SYNTAX:
          uartSendString((uint8_t *)"ERROR: bad arguments\r\n");
          break;

        default:
          break;
      }
      state = CMD_IDLE;
      break;

    default:
      cmdParserInit();
      break;
  }
}

/**
 * @brief  Separa una línea en hasta CMD_MAX_TOKENS tokens, usando
 *         espacios y tabs como separadores, ignorando espacios
 *         múltiples consecutivos. Modifica la línea original
 *         insertando terminadores '\0' entre tokens.
 * @param  line   Línea a tokenizar (se modifica in-place).
 * @param  tokens Arreglo de punteros donde se guarda el inicio de
 *         cada token encontrado.
 * @retval uint8_t Cantidad de tokens encontrados (0 a CMD_MAX_TOKENS).
 */
static uint8_t cmdTokenize(uint8_t * line, uint8_t * tokens[])
{
  uint8_t count = 0U;
  uint8_t * p = line;

  while (*p != '\0' && count < CMD_MAX_TOKENS)
  {
    while (*p == ' ' || *p == '\t')
    {
      p++;
    }

    if (*p == '\0')
    {
      break;
    }

    tokens[count] = p;
    count++;

    while (*p != '\0' && *p != ' ' && *p != '\t')
    {
      p++;
    }

    if (*p != '\0')
    {
      *p = '\0';
      p++;
    }
  }

  return count;
}

/**
 * @brief  Imprime por UART la lista de comandos disponibles.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void cmdPrintHelp(void)
{
  uartSendString((uint8_t *)"Comandos disponibles:\r\n");
  uartSendString((uint8_t *)"HELP           - muestra esta ayuda\r\n");
  uartSendString((uint8_t *)"LED ON         - enciende el LED\r\n");
  uartSendString((uint8_t *)"LED OFF        - apaga el LED\r\n");
  uartSendString((uint8_t *)"LED TOGGLE     - conmuta el LED\r\n");
  uartSendString((uint8_t *)"STATUS         - muestra el estado del LED\r\n");
}
