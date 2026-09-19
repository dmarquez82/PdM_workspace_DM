#include "mef_tablero.h"
#include "API_can.h"
#include "API_debounce.h"
#include "API_delay.h"

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

static void enviarComando(uint8_t dato);

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
 *         resetea la bandera de ACK antes de esperarlo.
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
    can_write_msg(&mensajeTx);
}

/**
 * @brief  Redefinición (sin __weak) del callback de recepción CAN.
 *         Se ejecuta en contexto de interrupción, por eso se limita
 *         a guardar el dato recibido y levantar una bandera —
 *         nada de lógica pesada acá (buena práctica de interrupciones
 *         cortas, vista en la materia).
 * @param  mensaje: puntero al mensaje CAN recibido.
 * @retval Ninguno.
 */
void can_read_msg_callback(can_msg_t *mensaje)
{
    if (mensaje->id == CAN_ID_ACK)
    {
        datoAckRecibido = mensaje->dato[0];
        ackRecibido = true;
    }
}
