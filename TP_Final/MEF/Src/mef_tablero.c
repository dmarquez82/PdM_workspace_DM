#include "mef_tablero.h"
#include "API_can.h"
#include "API_debounce.h"
#include "API_delay.h"
#include "API_uart.h"
#include <stdio.h>

#define CAN_ID_COMANDO   0x100
#define CAN_ID_ACK       0x200
#define DATO_ACTIVAR     0x01
#define DATO_DESACTIVAR  0x00
#define TIMEOUT_ACK_MS   1000

typedef enum
{
    ESTADO_REPOSO,
    ESTADO_ESPERA_ACK_ACTIVACION,
    ESTADO_SALIDA_ACTIVA,
    ESTADO_ESPERA_ACK_DESACTIVACION,
} estadoTablero_t;

static estadoTablero_t estadoActual;
static delay_t delayTimeoutAck;

/* Variables compartidas con el callback de recepción CAN (interrupción):
   deben ser volatile porque cambian fuera del flujo normal del programa. */
static volatile bool_t ackRecibido;
static volatile uint8_t datoAckRecibido;

/* Copia del último mensaje recibido, para loguearlo fuera de la
   interrupción. La interrupción solo copia datos y levanta una
   bandera; nunca transmite por UART directamente. */
static volatile bool_t rxLogPendiente;
static volatile uint32_t rxLogId;
static volatile uint8_t rxLogDato[8];
static volatile uint8_t rxLogLongitud;

static void enviarComando(uint8_t dato);
static void loguearMensaje(const char *prefijo, uint32_t id, uint8_t *dato, uint8_t longitud);
static void loguearTimeout(const char *accion);

// Declaraciones internas: las implementa mef_tablero_port_stm32f4xx.c
extern void tableroPort_EncenderLed(void);
extern void tableroPort_ApagarLed(void);

/**
 * @brief  Inicializa la MEF del tablero en su estado inicial
 *         (Reposo) e inicializa el driver de antirrebote.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void tableroMef_init(void)
{
    estadoActual = ESTADO_REPOSO;
    ackRecibido = false;
    rxLogPendiente = false;
    debounceFSM_init();
}

/**
 * @brief  Actualiza la MEF del tablero. Ver descripción en el header.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void tableroMef_update(void)
{
    debounceFSM_update();

    /* Log del último mensaje CAN recibido, si hay uno pendiente.
       Se procesa acá (fuera de la interrupción) para no bloquear
       la ISR con una transmisión UART. */
    if (rxLogPendiente == true)
    {
        rxLogPendiente = false;
        loguearMensaje("RX", rxLogId, (uint8_t *)rxLogDato, rxLogLongitud);
    }

    switch (estadoActual)
    {
        case ESTADO_REPOSO:
            if (readKey() == true)
            {
                enviarComando(DATO_ACTIVAR);
                delayInit(&delayTimeoutAck, TIMEOUT_ACK_MS);
                estadoActual = ESTADO_ESPERA_ACK_ACTIVACION;
            }
            break;

        case ESTADO_ESPERA_ACK_ACTIVACION:
            if (ackRecibido == true && datoAckRecibido == DATO_ACTIVAR)
            {
                tableroPort_EncenderLed();
                estadoActual = ESTADO_SALIDA_ACTIVA;
            }
            else if (delayRead(&delayTimeoutAck) == true)
            {
                loguearTimeout("activacion");
                estadoActual = ESTADO_REPOSO;
            }
            break;

        case ESTADO_SALIDA_ACTIVA:
            if (readKey() == true)
            {
                enviarComando(DATO_DESACTIVAR);
                delayInit(&delayTimeoutAck, TIMEOUT_ACK_MS);
                estadoActual = ESTADO_ESPERA_ACK_DESACTIVACION;
            }
            break;

        case ESTADO_ESPERA_ACK_DESACTIVACION:
            if (ackRecibido == true && datoAckRecibido == DATO_DESACTIVAR)
            {
                tableroPort_ApagarLed();
                estadoActual = ESTADO_REPOSO;
            }
            else if (delayRead(&delayTimeoutAck) == true)
            {
                loguearTimeout("desactivacion");
                estadoActual = ESTADO_SALIDA_ACTIVA;
            }
            break;

        default:
            estadoActual = ESTADO_REPOSO;
            break;
    }
}

/**
 * @brief  Arma y transmite el mensaje de comando por CAN, y
 *         resetea la bandera de ACK antes de esperarlo. Si el
 *         envío fue exitoso, lo loguea por UART.
 * @param  dato: DATO_ACTIVAR o DATO_DESACTIVAR.
 * @retval Ninguno.
 */
static void enviarComando(uint8_t dato)
{
    can_msg_t mensajeTx;

    mensajeTx.id = CAN_ID_COMANDO;
    mensajeTx.longitud = 1;
    mensajeTx.dato[0] = dato;

    ackRecibido = false;

    if (can_write_msg(&mensajeTx) == 1)
    {
        loguearMensaje("TX", mensajeTx.id, mensajeTx.dato, mensajeTx.longitud);
    }
}

/**
 * @brief  Arma una línea de texto con el prefijo (TX/RX), el ID,
 *         el DLC (longitud) y los datos del mensaje CAN en
 *         formato hexadecimal, y la envía por UART.
 * @param  prefijo: "TX" o "RX", para distinguir el sentido del mensaje.
 * @param  id: identificador del mensaje CAN.
 * @param  dato: puntero al arreglo de datos del mensaje.
 * @param  longitud: cantidad de bytes válidos en 'dato' (0 a 8).
 * @retval Ninguno.
 */
static void loguearMensaje(const char *prefijo, uint32_t id, uint8_t *dato, uint8_t longitud)
{
    char buffer[96];
    uint16_t posicion;
    uint8_t i;

    posicion = (uint16_t)sprintf(buffer, "%s - ID: 0x%03lX - DLC: %d - Datos:", prefijo, id, longitud);

    for (i = 0; i < longitud; i++)
    {
        posicion = posicion + (uint16_t)sprintf(&buffer[posicion], " 0x%02X", dato[i]);
    }

    buffer[posicion] = '\r';
    buffer[posicion + 1] = '\n';
    posicion = posicion + 2;

    uartSendStringSize((uint8_t *)buffer, posicion);
}

/**
 * @brief  Informa por UART que se venció el tiempo de espera del
 *         ACK sin recibir respuesta.
 * @param  accion: texto descriptivo ("activacion" o "desactivacion"),
 *         para indicar de qué comando era la espera.
 * @retval Ninguno.
 */
static void loguearTimeout(const char *accion)
{
    char buffer[64];
    uint16_t posicion;

    posicion = (uint16_t)sprintf(buffer, "TIMEOUT - No hubo respuesta (ACK %s)\r\n", accion);
    uartSendStringSize((uint8_t *)buffer, posicion);
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
        datoAckRecibido = mensaje->dato[0];
        ackRecibido = true;
    }

    rxLogId = mensaje->id;
    rxLogLongitud = mensaje->longitud;

    for (i = 0; i < mensaje->longitud; i++)
    {
        rxLogDato[i] = mensaje->dato[i];
    }

    rxLogPendiente = true;
}
