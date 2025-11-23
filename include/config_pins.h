#pragma once
#include <Arduino.h>

// ===================== PINOUT (ATmega328P) =====================
// Run/Stop from external module: HIGH = run, LOW = stop
#define PIN_GO           2    // GO/D2

// 74HCT4067 (S0..S3) and common output C0→ADC
#define MUX_S0           A0    // S0/A0
#define MUX_S1           A1    // S1/A1
#define MUX_S2           A2    // S2/A2
#define MUX_S3           A3    // S3/A3
#define MUX_C0_ADC       A4    // C0/A4

// Motors (IFX9201)
#define PIN_DIR_L        3     // DIR_L/D3
#define PIN_DIR_R        4     // DIR_R/D4

// PWM pin mapping (fijo, sin mover hardware)
// Motores: D5 (Timer0B) y D6 (Timer0A)
// Nota: El modo 20 kHz "same pins" mantiene exactamente estos pines.
#define PIN_PWM_L        5     // PWM_L/D5
#define PIN_PWM_R        6     // PWM_R/D6

// Mode button (free pushbutton wired to D11)
#define PIN_MODE_BTN     11     // B1/D11 INPUT_PULLUP (LOW = pressed)

// LEDs
#define LED1_PIN         7     // LED1/D7
#define LED2_PIN         A5     // LED2/A5

// Sensors
#define NUM_SENSORS          12
#define FIRST_MUX_CHANNEL     2   // set 2 if your harness starts at Y2

// Small helper for S0..S3 write
inline void pinmux_write(uint8_t ch) {
  digitalWrite(MUX_S0, (ch >> 0) & 1);
  digitalWrite(MUX_S1, (ch >> 1) & 1);
  digitalWrite(MUX_S2, (ch >> 2) & 1);
  digitalWrite(MUX_S3, (ch >> 3) & 1);
}
