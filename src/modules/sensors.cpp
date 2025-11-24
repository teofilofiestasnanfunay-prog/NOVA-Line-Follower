#include <Arduino.h>
#include <EEPROM.h>
#include "config_pins.h"
#include "config_params.h"
#include "io.h"
#include "sensors.h"

// =============== MUX raw reading ===============
void MUX_init() {
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  pinMode(MUX_S3, OUTPUT);
  pinMode(MUX_C0_ADC, INPUT);
  // Position MUX at the first valid channel (e.g., 2)
  pinmux_write(FIRST_MUX_CHANNEL & 0x0F);
}

static inline void selectMux(uint8_t channel) {
  pinmux_write(channel & 0x0F);
}

#ifdef ENABLE_DIAG
// Optional: print sensor->channel mapping for verification
static void printBinary4(uint8_t v) {
  Serial.print((v & 0x08) ? '1' : '0');
  Serial.print((v & 0x04) ? '1' : '0');
  Serial.print((v & 0x02) ? '1' : '0');
  Serial.print((v & 0x01) ? '1' : '0');
}

void MUX_debugPrintMapping() {
  Serial.println(F("Sensor -> Channel (S3 S2 S1 S0)"));
  for (uint8_t i = 0; i < NUM_SENSORS; i++) {
    uint8_t ch = FIRST_MUX_CHANNEL + i;  // expected 2..13
    Serial.print(i);
    Serial.print(F(" -> "));
    Serial.print(ch);
    Serial.print(F("  ("));
    printBinary4(ch & 0x0F);
    Serial.println(F(")"));
  }
}
#endif

void Sensors_readRaw(int *raw, uint8_t N) {
  for (uint8_t i = 0; i < N; i++) {
    selectMux(FIRST_MUX_CHANNEL + i);
    delayMicroseconds(5);
    raw[i] = analogRead(MUX_C0_ADC);
  }
}

// =============== Calibration + calibrated read ===============
static void captureWaitLong(const char *label) {
  Serial.print(F("Hold LONG press to capture: ")); Serial.println(label);
  while (true) {
    uint8_t v = ModeBtn_poll();
    if (v == 2) { delay(200); break; }
    delay(10);
  }
}

static void captureArray(int *dst, uint8_t N) {
  int raw[NUM_SENSORS];
  Sensors_readRaw(raw, N);
  for (uint8_t i = 0; i < N; i++) dst[i] = raw[i];
}

void Calib_captureBlackNear(int *bn, uint8_t N) {
  UI_setColor(UiColor::Red);
  captureWaitLong("Black Near");
  captureArray(bn, N);
  UI_setColor(UiColor::Off);
}

void Calib_captureBlackFar (int *bf, uint8_t N) {
  UI_setColor(UiColor::Red);
  captureWaitLong("Black Far");
  captureArray(bf, N);
  UI_setColor(UiColor::Off);
}

void Calib_captureWhiteNear(int *wn, uint8_t N) {
  UI_setColor(UiColor::Cyan);
  captureWaitLong("White Near");
  captureArray(wn, N);
  UI_setColor(UiColor::Off);
}

void Calib_captureWhiteFar (int *wf, uint8_t N) {
  UI_setColor(UiColor::Cyan);
  captureWaitLong("White Far");
  captureArray(wf, N);
  UI_setColor(UiColor::Off);
}

void Calib_load(int *bn, int *bf, int *wn, int *wf, uint8_t N) {
  for (uint8_t i = 0; i < N; i++) {
    bn[i] = ((int)EEPROM.read(EE_OFF_BLACK_NEAR + i)) << 4;
    bf[i] = ((int)EEPROM.read(EE_OFF_BLACK_FAR  + i)) << 4;
    wn[i] = ((int)EEPROM.read(EE_OFF_WHITE_NEAR + i)) << 4;
    wf[i] = ((int)EEPROM.read(EE_OFF_WHITE_FAR  + i)) << 4;
  }
}

void Calib_save(const int *bn,const int *bf,const int *wn,const int *wf, uint8_t N) {
  for (uint8_t i = 0; i < N; i++) {
    EEPROM.write(EE_OFF_BLACK_NEAR + i, (uint8_t)(bn[i] >> 4));
    EEPROM.write(EE_OFF_BLACK_FAR  + i, (uint8_t)(bf[i] >> 4));
    EEPROM.write(EE_OFF_WHITE_NEAR + i, (uint8_t)(wn[i] >> 4));
    EEPROM.write(EE_OFF_WHITE_FAR  + i, (uint8_t)(wf[i] >> 4));
  }
  UI_setColor(UiColor::Green);
  Serial.println(F("Calibration saved to EEPROM."));
}

void Sensors_readCalibrated(const int *bn,const int *bf,
                            const int *wn,const int *wf,
                            int *raw,int *cal,uint8_t N) {
  Sensors_readRaw(raw, N);
  for (uint8_t i = 0; i < N; i++) {
    int mn = (bn[i] < bf[i]) ? bn[i] : bf[i];
    int mx = (wn[i] > wf[i]) ? wn[i] : wf[i];
    if (mx < mn) { int t = mx; mx = mn; mn = t; }
    int v = raw[i];
    if (v < mn) v = mn;
    if (v > mx) v = mx;
    cal[i] = static_cast<int>(map(static_cast<long>(v),
                                  static_cast<long>(mn),
                                  static_cast<long>(mx),
                                  1000L, 0L));
  }
}
