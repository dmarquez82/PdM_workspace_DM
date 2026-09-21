#include "mef_keypad.h"
#include "API_can.h"
#include "API_debounce.h"
#include "API_delay.h"
#include "API_uart.h"
#include <stdio.h>

#define CAN_ID_COMMAND   0x100
#define CAN_ID_ACK       0x200
#define DATA_ON          0x01
#define DATA_OFF         0x00
#define ACK_TIMEOUT_MS   1000

typedef enum
{
    STATE_IDLE,
    STATE_WAIT_ACK_ON,
    STATE_OUTPUT_ON,
    STATE_WAIT_ACK_OFF,
} keypadState_t;

static keypadState_t currentState;
static delay_t ackTimeoutDelay;

/* Variables compartidas con el callback de recepción CAN (interrupción):
   deben ser volatile porque cambian fuera del flujo normal del programa. */
static volatile bool_t ackReceived;
static volatile uint8_t ackReceivedData;

/* Copia del último mensaje recibido, para loguearlo fuera de la
   interrupción. La interrupción solo copia datos y levanta una
   bandera; nunca transmite por UART directamente. */
static volatile bool_t rxLogPending;
static volatile uint32_t rxLogId;
static volatile uint8_t rxLogData[8];
static volatile uint8_t rxLogLength;

static void sendCommand(uint8_t data);
static void logMessage(const char *prefix, uint32_t id, uint8_t *data, uint8_t length);
static void logTimeout(const char *action);

// Declaraciones internas: las implementa mef_keypad_port_stm32f4xx.c
extern void keypadPort_TurnOnLed(void);
extern void keypadPort_TurnOffLed(void);

/**
 * @brief  Inicializa la MEF del teclado en su estado inicial
 *         (Idle) e inicializa el driver de antirrebote.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void mef_keypad_init(void)
{
    currentState = STATE_IDLE;
    ackReceived = false;
    rxLogPending = false;
    debounceFSM_init();
}

/**
 * @brief  Actualiza la MEF del teclado. Ver descripción en el header.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void mef_keypad_update(void)
{
    debounceFSM_update();

    if (rxLogPending == true)
    {
        rxLogPending = false;
        logMessage("RX", rxLogId, (uint8_t *)rxLogData, rxLogLength);
    }

    switch (currentState)
    {
        case STATE_IDLE:
            if (readKey() == true)
            {
                sendCommand(DATA_ON);
                delayInit(&ackTimeoutDelay, ACK_TIMEOUT_MS);
                currentState = STATE_WAIT_ACK_ON;
            }
            break;

        case STATE_WAIT_ACK_ON:
            if (ackReceived == true && ackReceivedData == DATA_ON)
            {
                keypadPort_TurnOnLed();
                currentState = STATE_OUTPUT_ON;
            }
            else if (delayRead(&ackTimeoutDelay) == true)
            {
                logTimeout("on");
                currentState = STATE_IDLE;
            }
            break;

        case STATE_OUTPUT_ON:
            if (readKey() == true)
            {
                sendCommand(DATA_OFF);
                delayInit(&ackTimeoutDelay, ACK_TIMEOUT_MS);
                currentState = STATE_WAIT_ACK_OFF;
            }
            break;

        case STATE_WAIT_ACK_OFF:
            if (ackReceived == true && ackReceivedData == DATA_OFF)
            {
                keypadPort_TurnOffLed();
                currentState = STATE_IDLE;
            }
            else if (delayRead(&ackTimeoutDelay) == true)
            {
                logTimeout("off");
                currentState = STATE_OUTPUT_ON;
            }
            break;

        default:
            currentState = STATE_IDLE;
            break;
    }
}

/**
 * @brief  Arma y transmite el mensaje de comando por CAN, y
 *         resetea la bandera de ACK antes de esperarlo. Si el
 *         envío fue exitoso, lo loguea por UART.
 * @param  data: DATA_ON o DATA_OFF.
 * @retval Ninguno.
 */
static void sendCommand(uint8_t data)
{
    can_msg_t txMessage;

    txMessage.id = CAN_ID_COMMAND;
    txMessage.longitud = 1;
    txMessage.dato[0] = data;

    ackReceived = false;

    if (can_write_msg(&txMessage) == true)
    {
        logMessage("TX", txMessage.id, txMessage.dato, txMessage.longitud);
    }
}

/**
 * @brief  Arma una línea de texto con el prefijo (TX/RX), el ID,
 *         el DLC (longitud) y los datos del mensaje CAN en
 *         formato hexadecimal, y la envía por UART.
 * @param  prefix: "TX" o "RX", para distinguir el sentido del mensaje.
 * @param  id: identificador del mensaje CAN.
 * @param  data: puntero al arreglo de datos del mensaje.
 * @param  length: cantidad de bytes válidos en 'data' (0 a 8).
 * @retval Ninguno.
 */
static void logMessage(const char *prefix, uint32_t id, uint8_t *data, uint8_t length)
{
    char buffer[96];
    uint16_t position;
    uint8_t i;

    position = (uint16_t)sprintf(buffer, "%s - ID: 0x%03lX - DLC: %d - Datos:", prefix, id, length);

    for (i = 0; i < length; i++)
    {
        position = position + (uint16_t)sprintf(&buffer[position], " 0x%02X", data[i]);
    }

    buffer[position] = '\r';
    buffer[position + 1] = '\n';
    position = position + 2;

    uartSendStringSize((uint8_t *)buffer, position);
}

/**
 * @brief  Informa por UART que se venció el tiempo de espera del
 *         ACK sin recibir respuesta.
 * @param  action: texto descriptivo ("on" u "off"), para indicar
 *         de qué comando era la espera.
 * @retval Ninguno.
 */
static void logTimeout(const char *action)
{
    char buffer[64];
    uint16_t length;

    length = (uint16_t)sprintf(buffer, "TIMEOUT - No hubo respuesta (ACK %s)\r\n", action);
    uartSendStringSize((uint8_t *)buffer, length);
}

/**
 * @brief  Redefinición (sin __weak) del callback de recepción CAN.
 *         Se ejecuta en contexto de interrupción: se limita a
 *         copiar el dato recibido y levantar banderas — nada de
 *         transmisión UART acá (interrupciones cortas).
 * @param  mensaje: puntero al mensaje CAN recibido.
 * @retval Ninguno.
 */
void can_read_msg_callback(can_msg_t *mensaje)
{
    uint8_t i;

    if (mensaje->id == CAN_ID_ACK)
    {
        ackReceivedData = mensaje->dato[0];
        ackReceived = true;
    }

    rxLogId = mensaje->id;
    rxLogLength = mensaje->longitud;

    for (i = 0; i < mensaje->longitud; i++)
    {
        rxLogData[i] = mensaje->dato[i];
    }

    rxLogPending = true;
}
