#include <Arduino.h>
#include "config_pins.h"
#include "config_params.h"
#include "io.h"

// ================= UI LEDs =================
static void setRaw(bool l1, bool l2) {
  digitalWrite(LED1_PIN, l1 ? HIGH : LOW);
  digitalWrite(LED2_PIN, l2 ? HIGH : LOW);
}

void UI_init() {
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  setRaw(false,false);
}

void UI_set(bool l1, bool l2) { setRaw(l1,l2); }

static void blinkNTimes(uint8_t pin, uint8_t n, unsigned onMs, unsigned offMs) {
  for (uint8_t i = 0; i < n; i++) {
    digitalWrite(pin, HIGH);
    delay(onMs);
    digitalWrite(pin, LOW);
    delay(offMs);
  }
}

void UI_previewMode(uint8_t mode) {
  // Preview with LED1: blink count equals mode number
  setRaw(false,false);
  blinkNTimes(LED1_PIN, mode, 120, 80);
}

void UI_confirmMode(uint8_t mode) {
  // Confirmation with LED2: blink count equals mode number (slightly slower)
  setRaw(false,false);
  blinkNTimes(LED2_PIN, mode, 150, 200);
}

void UI_showRun(bool running) {
  digitalWrite(LED2_PIN, running ? HIGH : LOW);
}

// Non-blocking strobe on LED2 during run
void UI_runStrobeUpdate(bool running) {
  static bool led2_state = true;
  static unsigned long next_ms = 0;
  if (!running) {
    led2_state = false;
    digitalWrite(LED2_PIN, LOW);
    next_ms = millis();
    return;
  }
  unsigned long now = millis();
  if ((long)(now - next_ms) >= 0) {
    led2_state = !led2_state;
    digitalWrite(LED2_PIN, led2_state ? HIGH : LOW);
    next_ms = now + LED2_STROBE_MS;
  }
}

// ================= Mode Button =================
static const unsigned long LONG_MS   = 1000UL;
static const unsigned long DEBOUNCE  = 30UL;

void ModeBtn_init() {
  pinMode(PIN_MODE_BTN, INPUT_PULLUP);
}

uint8_t ModeBtn_poll() {
  if (digitalRead(PIN_MODE_BTN) == LOW) {
    delay(DEBOUNCE);
    if (digitalRead(PIN_MODE_BTN) != LOW) return 0;

    unsigned long t0 = millis();
    // soft feedback (optional): brief LED1 hint
    digitalWrite(LED2_PIN, HIGH);

    while (digitalRead(PIN_MODE_BTN) == LOW) {
      if (millis() - t0 >= LONG_MS) {
        // long press
        digitalWrite(LED1_PIN, LOW);
        // confirmation blink (LED1 + LED2)
        digitalWrite(LED1_PIN, HIGH);
        digitalWrite(LED2_PIN, HIGH);
        delay(100);
        digitalWrite(LED1_PIN, LOW);
        digitalWrite(LED2_PIN, LOW);
        return 2;
      }
    }
    digitalWrite(LED1_PIN, LOW);
    return 1; // short press
  }
  return 0;
}

// Non-blocking falling-edge (press) detector for runtime use
static bool rt_prev = false;                 // false = not pressed, true = pressed (LOW)
static unsigned long rt_last_change = 0;
static const unsigned long RT_DB = 30UL;     // debounce

bool ModeBtn_edgePressed() {
  bool rawPressed = (digitalRead(PIN_MODE_BTN) == LOW);
  unsigned long now = millis();
  bool edge = false;
  if (rawPressed != rt_prev) {
    if (now - rt_last_change >= RT_DB) {
      // state change accepted
      edge = (!rt_prev && rawPressed); // LOW just pressed
      rt_prev = rawPressed;
      rt_last_change = now;
    }
  }
  return edge;
}

// ================= GO Signal =================
static bool prevGO = false;

void GO_init() {
  pinMode(PIN_GO, INPUT);   // external module drives 0V/5V
  prevGO = (digitalRead(PIN_GO) == HIGH);
}

bool GO_read() {
  return digitalRead(PIN_GO) == HIGH;
}

bool GO_edgeStart() {
  bool now = GO_read();
  bool edge = (!prevGO && now);
  prevGO = now;
  return edge;
}

bool GO_edgeStop() {
  bool now = GO_read();
  bool edge = (prevGO && !now);
  prevGO = now;
  return edge;
}
