# Futuras iteraciones — organización por grafo de estados

Este documento recoge ideas para seguir evolucionando la arquitectura hacia un
grafo de estados más completo, sin que sea necesario implementarlas ahora.

## Hecho (2026-08-25)
- `src/main.cpp` reemplazó las banderas `sessionActive` / `startDelayActive` por
  un único `enum class RunState { Idle, StartDelay, Running }` con transiciones
  explícitas (`setRunState`). Mismo comportamiento, más fácil de leer y de dibujar
  como grafo.

## Ideas para siguientes iteraciones

1. **FSM de alto nivel unificada.** Hoy la selección de modo (`g_mode`: Calibrar,
   Diag Raw, Diag Cal+Pos, Run) vive en `setup()` como bloques `if/else` y
   `#ifdef ENABLE_DIAG`. Podría modelarse como un segundo grafo de estados
   (`Menu -> Calibrate | DiagRaw | DiagCalPos | Run`) del mismo estilo que
   `RunState`, para que todo el flujo del programa sea un solo grafo visible.

2. **Emergencia como estado propio.** Actualmente "line lost" dispara
   `stopRace()` y vuelve a `Idle` en el mismo ciclo (es una transición, no un
   estado persistente). Si en el futuro quieres, por ejemplo, un cooldown antes
   de permitir reiniciar, o un color de LED distinto mientras dura la
   emergencia, conviene promoverlo a un estado real: `EmergencyStop -> Idle`
   tras N ms.

3. **Extraer el grafo a su propio módulo.** `RunState` y sus transiciones podrían
   vivir en `src/modules/run_fsm.h/.cpp` en vez de `main.cpp`, dejando que
   `main.cpp` solo orqueste (lee sensores, llama a la FSM, llama a motores).
   Se pospuso ahora para minimizar el diff y el riesgo en código de carrera.

4. **Jerarquía Running -> SpeedPhase.** Hoy `RunState::Running` y `SpeedPhase`
   (Normal/Decelerating/Slow/Accelerating) son dos enums paralelos. Formalmente
   `SpeedPhase` es un sub-grafo *dentro* de `Running` (estado jerárquico). Se
   podría anidar explícitamente si se necesitan más perfiles de velocidad.

5. **Config en runtime en vez de `#if START_MODE`.** El modo de arranque
   (botón vs. señal GO) es hoy una rama de compilación. Convertirlo en dos
   grafos seleccionables en runtime (o un flag guardado en EEPROM) permitiría
   probar ambos modos sin recompilar.

6. **Diagrama vivo.** Si el grafo crece, vale la pena mantener el diagrama
   (Mermaid) como comentario en el propio código fuente para que no se
   desincronice del comportamiento real.
