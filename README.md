# NOVA 5 - Firmware Seguidor de Linea (ATmega328P)

Proyecto PlatformIO para un seguidor de linea con dos builds: carrera y diagnostico. Incluye calibracion, control PD y control de motores. Esta version elimina la turbina y arranca la rampa de motores de inmediato (sin tiempo muerto de espera).

**Requisitos**
- PlatformIO (desde VS Code o `pio` en terminal)
- Placa `miniatmega328` (ATmega328P) y cable USB

**Entornos de Build**
- `env:miniatmega328` (race): firmware minimo para competir
- `env:miniatmega328_diag` (diag): anade modos de diagnostico y salida por Serial

**Comandos**
- Compilar race: `pio run -e miniatmega328`
- Compilar diag: `pio run -e miniatmega328_diag`
- Subir (race): `pio run -e miniatmega328 -t upload`
- Subir (diag): `pio run -e miniatmega328_diag -t upload`
- Monitor serie: `pio device monitor -b 115200`

**Modos de Operacion**
- Race (por defecto):
  - Modo 1: Calibracion (4 capturas con pulsacion larga)
  - Modo 2: Carrera
- Diag (`-DENABLE_DIAG`):
  - Modo 1: Calibracion
  - Modo 2: Lecturas RAW (sensores sin calibrar) cada 100 ms
  - Modo 3: Lecturas CAL (0..1000) + posicion `POS` cada 100 ms
  - Modo 4: Carrera

Seleccion con el boton de modo: pulsacion corta avanza, pulsacion larga confirma. Los LEDs indican el modo seleccionado.

### Modo 1: Calibracion (detalle del algoritmo)
- Se piden cuatro capturas (Black Near/Far, White Near/Far) con pulsacion larga; los LED guian cada paso (rojo para negro, cyan para blanco).
- Por sensor se toma el minimo entre los dos negros y el maximo entre los dos blancos; se calcula un margen de blanco `CAL_WHITE_MARGIN_PCT = 10%` del rango (al menos `CAL_WHITE_MARGIN_MIN = 2` unidades) y se resta a `mx` para que el blanco sature en `1000`.
- Cada lectura RAW se clampa a `[mn, mx]`, se mapea a `0..1000` (blanco alto, negro bajo) y cualquier valor calibrado menor a `CAL_WHITE_CLAMP_BELOW = 750` se fuerza a `0`.
- Los vectores de captura se guardan en EEPROM comprimidos (`>>4`) tras anunciar `Calibration saved to EEPROM`, antes de reiniciar o pasar a carrera.

**Parada de Emergencia**
- Si la posicion calibrada permanece en `0` o `11000` durante >= 500 ms, se ejecuta parada de emergencia: motores a 0.
- Implementacion principal en `src/main.cpp` (buscar "EMERGENCY STOP").

**Estructura del Codigo**
- IO/UI/boton/GO: `src/modules/io.h`, `src/modules/io.cpp`
- Sensores y calibracion: `src/modules/sensors.h`, `src/modules/sensors.cpp`
- Motores: `src/modules/drive.h`, `src/modules/drive.cpp`
- Control PD: `src/modules/pid_control.h`, `src/modules/pid_control.cpp`
- Parametros: `include/config_params.h`
- Pines: `include/config_pins.h`
- Entrada principal: `src/main.cpp`

**Ajustes Rapidos**
- Ganancias PD: `include/config_params.h` (`Kp_default`, `Kd_default`)
- Velocidad base y rampa: `include/config_params.h` (`BASE_PWM_DEFAULT`, `MOTOR_RAMP_*`)
- Setpoint de linea: `include/config_params.h` (`LINE_SETPOINT`)
- Retraso inicial opcional tras GO: `include/config_params.h` (`RUN_START_DELAY_ENABLE`, `RUN_START_DELAY_MS`)
- Numero de sensores y MUX: `include/config_pins.h` (`NUM_SENSORS`, `FIRST_MUX_CHANNEL`)

**Consejos de Diagnostico**
- La salida Serial es frecuente; evita abrir el monitor con baudios bajos.
- En modos 2/3 (diag) los motores quedan parados para no interferir.
- Usa strings con `F("...")` para no consumir SRAM.

**Notas de PlatformIO**
- Configuracion de builds en `platformio.ini`.
- Si necesitas puerto fijo: anade `upload_port = COMx` (reemplaza `COMx`) en el `env` correspondiente.
