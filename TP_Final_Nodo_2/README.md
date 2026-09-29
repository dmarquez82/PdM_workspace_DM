# TP_Final_Nodo_2 — Nodo CAN receptor sobre FRDM-MCXC444

Segundo nodo del Trabajo Final de las asignaturas **Programación de Microcontroladores** y **Protocolos de Comunicación en Sistemas Embebidos** (CESE).

Este nodo recibe comandos por bus CAN desde el **Nodo 1** ([`TP_Final`](../TP_Final), NUCLEO-F446RE), acciona una salida (LED rojo) según el comando y responde con un mensaje de ACK.

## Hardware

| Elemento | Detalle |
|---|---|
| Placa | FRDM-MCXC444 (NXP MCXC444, Cortex-M0+) + Shield SD2 |
| Controlador CAN | MCP2515 (cristal de 8 MHz) vía SPI |
| Transceptor CAN | Asociado al módulo MCP2515 |
| Consola de depuración | LPUART0, 115200 baudios |

### Conexión MCP2515 ↔ MCXC444

| Señal MCP2515 | Pin MCXC444 | Función |
|---|---|---|
| SCK | PTC5 | SPI0_SCK |
| SI | PTC6 | SPI0_MOSI |
| SO | PTC7 | SPI0_MISO |
| CS | PTC4 | GPIO (CS manual) |
| VCC / GND | 5 V / GND | Alimentación del módulo |

SPI0 configurado como master, modo 3 (CPOL = 1, CPHA = 1), 8 bits, MSB primero, 4 MHz.

### Salidas utilizadas

| LED | Pin | Uso |
|---|---|---|
| Rojo | PTE31 | Salida comandada por CAN (activo en bajo) |
| Azul | PTE29 | Se apaga al inicio |

## Protocolo CAN

- Bitrate: **125 kbps**
- Formato: tramas estándar (ID de 11 bits)

| ID | Sentido | DLC | Data[0] | Significado |
|---|---|---|---|---|
| `0x100` | Nodo 1 → Nodo 2 | 1 | `0x01` | Encender LED rojo |
| `0x100` | Nodo 1 → Nodo 2 | 1 | `0x00` | Apagar LED rojo |
| `0x200` | Nodo 2 → Nodo 1 | 1 | eco de Data[0] recibido | ACK |

El ACK se envía ante cualquier trama `0x100` recibida, repitiendo el byte de dato para que el Nodo 1 confirme qué comando fue procesado.

## Funcionamiento

1. `main()` inicializa pines, reloj, consola de depuración, placa y SPI0, y configura SysTick a 1 ms.
2. `Nodo2()` inicializa el MCP2515 (reset → 125 kbps con cristal de 8 MHz → modo normal) y apaga los LEDs.
3. En el lazo principal:
   - Lee el MCP2515 por **polling** (`mcp2515_readMessage`).
   - Si llega una trama con ID `0x100`: envía el ACK con ID `0x200`, actualiza el LED rojo según `data[0]` y resetea el ID recibido para no reprocesar la trama.

Mensajes por consola:

```
NODO2: Iniciado.
CAN: Iniciado correctamente.
CAN SW : Mensaje enviado - ID: 0x200, DLC: 1, Data[0]: 0x1, ...
```

## Estructura del proyecto

```
TP_Final_Nodo_2/
├── source/
│   ├── main.c          # Inicialización y SysTick
│   ├── Nodo2.c/.h      # Lógica del nodo: recepción, salida y ACK
│   ├── mcp2515.c/.h    # Driver del controlador CAN MCP2515
│   ├── can.h           # Estructura can_frame (estilo SocketCAN)
│   ├── spi_can.c/.h    # Capa de adaptación SPI para el driver MCP2515
│   └── SD2_board.c/.h  # Soporte de placa SD2: LEDs, pulsadores, SPI0, CS
├── board/              # Pines, clocks y periféricos (MCUXpresso Config Tools)
├── drivers/            # Drivers SDK NXP (fsl_spi, fsl_gpio, fsl_lpuart, ...)
├── CMSIS/, device/, startup/, utilities/, component/
└── SD2_MCXC444_CAN LinkServer Debug.launch
```

### Capas de software

```
Nodo2.c  →  mcp2515.c  →  spi_can.c  →  SD2_board.c  →  SDK NXP (fsl_spi / fsl_gpio)
```

## Compilación y carga

1. Importar el proyecto en **MCUXpresso IDE** (*File → Import → Existing Projects into Workspace*).
2. Verificar que esté instalado el SDK de MCXC444.
3. Compilar (*Build*).
4. Depurar/cargar con la configuración `SD2_MCXC444_CAN LinkServer Debug`.
5. Abrir una terminal serie a 115200 8N1 sobre el puerto de la placa.

## Prueba con el Nodo 1

1. Conectar CANH/CANL entre ambos nodos, con terminación de 120 Ω en cada extremo y GND común.
2. Encender ambos nodos y verificar `CAN: Iniciado correctamente.` en la consola.
3. Enviar un comando desde el Nodo 1: el LED rojo debe cambiar de estado y en la consola del Nodo 2 debe aparecer el envío del ACK.

## Créditos

- Driver MCP2515 y soporte de placa SD2: Agustín M. Zuliani — Cátedra Sistemas Digitales 2, DSI, FCEIA, UNR.
- Adaptación y lógica del Nodo 2: Prof. Ing. Daniel Márquez (dmarquez@fceia.unr.edu.ar).

Licencia: BSD 3-Clause (ver encabezados de los archivos fuente).
