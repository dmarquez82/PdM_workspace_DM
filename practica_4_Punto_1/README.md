# Práctica 4 - Punto 1: MEF Antirrebote (main.c)

**Materia:** Programación de Microcontroladores
**Estudiante:** Prof. Ing. Daniel Márquez
**Placa:** NUCLEO-F446RE

## Objetivo

Implementar una Máquina de Estados Finitos (MEF) tipo Mealy para realizar antirrebote
por software sobre el pulsador B1 de la placa, generando eventos ante flancos
descendentes y ascendentes confirmados, sin bloquear el programa principal.

## Diagrama de estados

```
BUTTON\_UP ──(pulsador presionado)──> BUTTON\_FALLING
BUTTON\_FALLING ──(40ms, sigue presionado)──> BUTTON\_DOWN  \[evento: buttonPressed()]
BUTTON\_FALLING ──(40ms, ya no está presionado)──> BUTTON\_UP
BUTTON\_DOWN ──(pulsador liberado)──> BUTTON\_RISING
BUTTON\_RISING ──(40ms, sigue liberado)──> BUTTON\_UP  \[evento: buttonReleased()]
BUTTON\_RISING ──(40ms, ya no está liberado)──> BUTTON\_DOWN
```

* **BUTTON\_UP:** botón liberado, en reposo.
* **BUTTON\_FALLING:** posible flanco descendente detectado; se espera el tiempo de
antirrebote y se relee el pin para confirmar.
* **BUTTON\_DOWN:** botón confirmado como presionado.
* **BUTTON\_RISING:** posible flanco ascendente detectado; se espera el tiempo de
antirrebote y se relee el pin para confirmar.

El estado inicial de la MEF es `BUTTON\_UP`.

## Estructura del código (`main.c`)

```c
#define DEBOUNCE\_TIME\_MS  40U

typedef enum {
  BUTTON\_UP,
  BUTTON\_FALLING,
  BUTTON\_DOWN,
  BUTTON\_RISING,
} debounceState\_t;

debounceState\_t estadoActual;
delay\_t debounceDelay;
```

La constante `DEBOUNCE\_TIME\_MS` evita hardcodear el valor de 40 ms en el código.
`estadoActual` se declara como variable global de archivo (no local a una función).

## Funciones implementadas

### `void debounceFSM\_init(void)`

Inicializa la MEF en su estado inicial (`BUTTON\_UP`) y prepara el retardo no
bloqueante interno (`debounceDelay`) que la MEF usa para confirmar los flancos, con
la duración `DEBOUNCE\_TIME\_MS`.

### `void debounceFSM\_update(void)`

Debe llamarse periódicamente dentro del loop principal. Implementa un `switch-case`
sobre `estadoActual` que cubre los 4 estados definidos más un caso `default` de
recuperación (si `estadoActual` tomara un valor inválido por algún motivo anómalo,
se reinicializa la MEF a un estado seguro). Lee el pulsador B1, resuelve las
transiciones de estado, y usa un retardo no bloqueante (`delayRead`) para diferir la
segunda lectura de confirmación en los estados `BUTTON\_FALLING`/`BUTTON\_RISING`.

### `void buttonPressed(void)`

Evento disparado al confirmarse una pulsación real (flanco descendente confirmado).
Enciende el LED de usuario (LD2).

### `void buttonReleased(void)`

Evento disparado al confirmarse la liberación real del botón (flanco ascendente
confirmado). Apaga el LED de usuario (LD2).

## Por qué un retardo no bloqueante y no un `HAL\_Delay`

Un `HAL\_Delay(40)` bloqueante detendría la ejecución completa del programa durante
esos 40 ms cada vez que se detecta un posible cambio de nivel del pulsador —
incluyendo los rebotes, que pueden ocurrir varias veces seguidas. Con la MEF y el
retardo no bloqueante (`API\_delay`), el `while(1)` sigue corriendo libremente: al
entrar a `BUTTON\_FALLING`/`BUTTON\_RISING` se arma el retardo y se continúa de largo;
en las vueltas siguientes se consulta con `delayRead` si ya se cumplió el tiempo,
sin bloquear nada mientras tanto. Esto permite que cualquier otra tarea del sistema
(otros delays, lecturas, etc.) siga ejecutándose en paralelo.

## `main()`

```c
debounceFSM\_init();

while (1)
{
  debounceFSM\_update();
}
```

## Puntos para pensar

* **¿Se nota una mejora respecto a lecturas sin antirrebote?** Sí: al confirmar el
flanco recién después de los 40 ms, se descartan los rebotes mecánicos del
pulsador (que suelen durar unos pocos ms), evitando disparos múltiples de
`buttonPressed()`/`buttonReleased()` por una sola pulsación real.
* **¿Es adecuada la temporización de `debounceFSM\_update()`?** Se llama en cada
vuelta del `while(1)`, sin ningún delay entre llamadas — esto maximiza la
resolución de detección de flancos. Si se llamara con un período mucho más
grande, se correría el riesgo de no detectar pulsaciones muy cortas; con un
período mucho más chico no habría inconveniente funcional, salvo el consumo de
ciclos de CPU en llamadas redundantes.

