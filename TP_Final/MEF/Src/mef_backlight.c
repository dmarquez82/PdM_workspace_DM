#include "mef_backlight.h"
#include "API_ldr.h"
#include "API_delay.h"

#define LOW_THRESHOLD    1500U
#define HIGH_THRESHOLD   2000U
#define READ_PERIOD_MS   100U

typedef enum
{
    STATE_BACKLIGHT_OFF,
    STATE_BACKLIGHT_ON,
} backlightState_t;

static backlightState_t currentState;
static delay_t readDelay;

// Declaraciones internas: las implementa mef_backlight_port_stm32f4xx.c
extern void backlightPort_TurnOnLed(void);
extern void backlightPort_TurnOffLed(void);

/**
 * @brief  Inicializa la MEF de backlight. Ver descripción en el header.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void mef_backlight_init(void)
{
    currentState = STATE_BACKLIGHT_OFF;
    delayInit(&readDelay, READ_PERIOD_MS);
}

/**
 * @brief  Actualiza la MEF de backlight. Ver descripción en el header.
 * @param  Ninguno.
 * @retval Ninguno.
 */
void mef_backlight_update(void)
{
    uint16_t ldrValue;

    if (delayRead(&readDelay) == false)
    {
        return;
    }

    delayInit(&readDelay, READ_PERIOD_MS);

    ldrValue = ldr_ReadValue();

    switch (currentState)
    {
        case STATE_BACKLIGHT_OFF:
            if (ldrValue < LOW_THRESHOLD)
            {
                backlightPort_TurnOnLed();
                currentState = STATE_BACKLIGHT_ON;
            }
            break;

        case STATE_BACKLIGHT_ON:
            if (ldrValue > HIGH_THRESHOLD)
            {
                backlightPort_TurnOffLed();
                currentState = STATE_BACKLIGHT_OFF;
            }
            break;

        default:
            currentState = STATE_BACKLIGHT_OFF;
            break;
    }
}
