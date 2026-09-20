#ifndef API_UART_H
#define API_UART_H

#include <stdint.h>
#include "API_delay.h" /* para bool_t */

bool_t uartInit(void);
void uartSendString(uint8_t * pstring);
void uartSendStringSize(uint8_t * pstring, uint16_t size);
void uartReceiveStringSize(uint8_t * pstring, uint16_t size);

#endif

