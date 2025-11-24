#include <Arduino.h>
#include "config_pins.h"
#include "config_params.h"
#include "io.h"

// ================= UI LEDs =================
static void setRGB(bool r, bool g, bool b) {
  digitalWrite(LED_PIN_R, r ? HIGH : LOW);
  digitalWrite(LED_PIN_G, g ? HIGH : LOW);
  digitalWrite(LED_PIN_B, b ? HIGH : LOW);
}

static void setColor(UiColor color) {
  switch (color) {
    case UiColor::Red:      setRGB(true,  false, false); break;
    case UiColor::Green:    setRGB(false, true,  false); break;
    case UiColor::Blue:     setRGB(false, false, true ); break;
    case UiColor::Yellow:   setRGB(true,  true,  false); break;
    case UiColor::Cyan:     setRGB(false, true,  true ); break;
    case UiColor::Magenta:  setRGB(true,  false, true ); break;
    case UiColor::White:    setRGB(true,  true,  true ); break;
    case UiColor::Off:
    default:                setRGB(false, false, false); break;
  }
}

void UI_init() {
  pinMode(LED_PIN_R, OUTPUT);
  pinMode(LED_PIN_G, OUTPUT);
  pinMode(LED_PIN_B, OUTPUT);
  setColor(UiColor::Off);
}

void UI_setColor(UiColor color) { setColor(color); }
void UI_setRGB(bool r, bool g, bool b) { setRGB(r, g, b); }

static void blinkColor(UiColor color, uint8_t n, unsigned onMs, unsigned offMs) {
  for (uint8_t i = 0; i < n; i++) {
    setColor(color);
    delay(onMs);
    setColor(UiColor::Off);
    delay(offMs);
  }
}

void UI_previewMode(uint8_t mode) {
  // Preview: blink count equals mode number (green channel)
  setColor(UiColor::Off);
  blinkColor(UiColor::Green, mode, 120, 80);
}

void UI_confirmMode(uint8_t mode) {
  // Confirmation: blink count equals mode number (red channel, slightly slower)
  setColor(UiColor::Off);
  blinkColor(UiColor::Red, mode, 150, 200);
}

void UI_showRun(bool running) {
  setColor(running ? UiColor::Blue : UiColor::Off);
}

// Non-blocking strobe on blue channel during run
void UI_runStrobeUpdate(bool running) {
  static bool led_state = true;
  static unsigned long next_ms = 0;
  if (!running) {
    led_state = false;
    setColor(UiColor::Off);
    next_ms = millis();
    return;
  }
  unsigned long now = millis();
  if ((long)(now - next_ms) >= 0) {
    led_state = !led_state;
    setColor(led_state ? UiColor::Blue : UiColor::Off);
    next_ms = now + RUN_STROBE_MS;
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
    // soft feedback while button stays pressed
    setColor(UiColor::Blue);

    while (digitalRead(PIN_MODE_BTN) == LOW) {
      if (millis() - t0 >= LONG_MS) {
        // long press
        setColor(UiColor::White);
        delay(120);
        setColor(UiColor::Off);
        return 2;
      }
    }
    setColor(UiColor::Green);
    delay(80);
    setColor(UiColor::Off);
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
