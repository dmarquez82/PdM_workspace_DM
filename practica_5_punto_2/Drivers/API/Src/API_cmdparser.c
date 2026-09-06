#include "API_cmdparser.h"
#include "API_uart.h"


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


static void cmdProcessLine(void);




/**
 * @brief  Inicializa el módulo parser de comandos: carga el estado
 *         inicial de la MEF (CMD_IDLE) y resetea el índice del buffer
 *         de recepción de línea.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void cmdParserInit(void)
{
  state = CMD_IDLE;
  bufferIndex = 0U;
}
