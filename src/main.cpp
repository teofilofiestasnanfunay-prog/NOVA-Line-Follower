#include <Arduino.h>
#include <EEPROM.h>
#include "config_pins.h"
#include "config_params.h"

// Modules
#include "modules/utils.h"
#include "modules/io.h"
#include "modules/sensors.h"
#include "modules/pid_control.h"
#include "modules/drive.h"

// ======== GLOBAL STATE ========
static int  rawSensor[NUM_SENSORS], calSensor[NUM_SENSORS];
static int  blackNear[NUM_SENSORS], blackFar[NUM_SENSORS];
static int  whiteNear[NUM_SENSORS], whiteFar[NUM_SENSORS];

static int  basePWM_current = 0;
static bool rampActive = false;
static unsigned long lastRampMs = 0;
static bool sessionActive = false;
static unsigned long sessionStartMs = 0;
static bool startDelayActive = false;
static unsigned long startDelayStartMs = 0;
static bool startDelayFromGo = false;
static bool sessionFromGo = false; // tracks if current session started from GO
static int  BASE_PWM = BASE_PWM_DEFAULT;
static bool emgActive = false;
static bool posAtExtreme = false;
static unsigned long extremeStartMs = 0;
static uint8_t g_mode = 2; // selected mode persisted for loop

// ======== HELPERS ========
static void rampBase_update() {
  if (!rampActive) return;
  unsigned long now = millis();
  if (now - lastRampMs >= (unsigned long)MOTOR_RAMP_INTERVAL_MS) {
    lastRampMs = now;
    basePWM_current += MOTOR_RAMP_STEP;
    if (basePWM_current >= BASE_PWM) {
      basePWM_current = BASE_PWM;
      rampActive = false;
    }
  }
}

static void startRaceNow(bool fromGo) {
  basePWM_current = 0;
  lastRampMs = millis();
  rampActive = true;
  sessionActive = true;
  sessionStartMs = millis(); // run session starts counting when we actually move
  sessionFromGo = fromGo;    // Track source to decide if GO lines can stop the run
  Serial.println(F(">> START (GO)"));
}

static void startRace(bool fromGo) {
  Motors_cmdPct(0,0);
  if (RUN_START_DELAY_ENABLE && RUN_START_DELAY_MS > 0) {
    startDelayActive = true;
    startDelayStartMs = millis();
    startDelayFromGo = fromGo;
    Serial.print(F(">> START DELAY ")); Serial.print(RUN_START_DELAY_MS); Serial.println(F(" ms"));
    return;
  }
  startRaceNow(fromGo);
}

static void stopRace() {
  startDelayActive = false;
  rampActive = false;
  basePWM_current = 0;
  Motors_cmdPct(0,0);
  sessionActive = false;
  Serial.println(F(">> STOP (GO)"));
}

static uint8_t menu_select_mode() {
  Serial.println(F("Menu: 1 = Calibrate, 2 = Run. Short = next, Long = confirm."));
  UI_setColor(UiColor::Off);
  uint8_t sel = 1;
  #ifdef ENABLE_DIAG
    const uint8_t MAX_MODES = 4; // 1=Calib, 2=Raw, 3=Cal+Pos, 4=Run
    Serial.println(F("DIAG build: Modes -> 1:Calib  2:Raw  3:Cal+Pos  4:Run"));
  #else
    const uint8_t MAX_MODES = 2; // 1=Calib, 2=Run
  #endif
  while (true) {
    uint8_t v = ModeBtn_poll();
    if (v == 2) { UI_confirmMode(sel); delay(200); break; }     // long press confirms
    if (v == 1) {                          // short cycles
      sel++; if (sel > MAX_MODES) sel = 1;
      UI_previewMode(sel);
    }
  }
  return sel;
}

// ======== SETUP ========
void setup() {
  Serial.begin(115200);
  Motors_init();
  UI_init();
  ModeBtn_init();
  GO_init();
  MUX_init();
  #ifdef ENABLE_DIAG
    MUX_debugPrintMapping();
  #endif

  g_mode = menu_select_mode();
  // Show chosen mode once more for clarity
  UI_confirmMode(g_mode);

  if (g_mode == 1) {
    Serial.println(F("Mode 1: Calibration (use LONG press for each capture)"));
    Calib_captureBlackNear(blackNear, NUM_SENSORS);
    Calib_captureBlackFar (blackFar , NUM_SENSORS);
    Calib_captureWhiteNear(whiteNear, NUM_SENSORS);
    Calib_captureWhiteFar (whiteFar , NUM_SENSORS);
    Calib_save(blackNear, blackFar, whiteNear, whiteFar, NUM_SENSORS);
    Serial.println(F("Calibration saved to EEPROM. Reboot or choose Run (Mode 2)."));
    while (1) { Motors_cmdPct(0,0); delay(100); }
  }
  #ifdef ENABLE_DIAG
  else if (g_mode == 2) {
    Serial.println(F("Mode 2: DIAG Raw sensors"));
    // No need to load calibration
  } else if (g_mode == 3) {
    Serial.println(F("Mode 3: DIAG Calibrated + Position"));
    Calib_load(blackNear, blackFar, whiteNear, whiteFar, NUM_SENSORS);
  } else { // 4: Run
    Serial.println(F("Mode 4: Run"));
    Calib_load(blackNear, blackFar, whiteNear, whiteFar, NUM_SENSORS);
  }
  #else
  else {
    Serial.println(F("Mode 2: Run"));
    Calib_load(blackNear, blackFar, whiteNear, whiteFar, NUM_SENSORS);
  }
  #endif

  PID_init(Kp_default(), Kd_default());

  // Initial GO state
  #ifdef ENABLE_DIAG
    if (g_mode >= 4) {
      if (GO_read()) startRace(true); else stopRace();
    } else {
      stopRace();
    }
  #else
    if (GO_read()) startRace(true); else stopRace();
  #endif
}

// ======== LOOP ========
void loop() {
  // Start delay handler: only arm ramp/session after delay expires
  if (startDelayActive && (millis() - startDelayStartMs >= (unsigned long)RUN_START_DELAY_MS)) {
    startDelayActive = false;
    startRaceNow(startDelayFromGo);
  }

  // Start/Stop triggers
  if (!sessionActive && !startDelayActive) {
    if (GO_edgeStart())  { startRace(true);  }
    if (ModeBtn_edgePressed()) { startRace(false); }
  }
  // Only GO-based sessions listen to GO falling edge for stop
  if (sessionActive && sessionFromGo && GO_edgeStop())  { Motors_brake(); stopRace(); }

  // Auto-stop after configured session window if still running
  if (sessionActive && (millis() - sessionStartMs >= (unsigned long)RUN_SESSION_MS)) {
    Motors_brake();
    stopRace();
  }

  rampBase_update();

  #ifdef ENABLE_DIAG
  // Diagnostic modes: 2=raw, 3=cal+pos. Keep motors stopped, print at intervals.
  static unsigned long lastPrintMs = 0;
  if (g_mode == 2 || g_mode == 3) {
    Motors_cmdPct(0,0);
    UI_runStrobeUpdate(false);
    unsigned long now = millis();
    if (now - lastPrintMs >= 100UL) {
      lastPrintMs = now;
      if (g_mode == 2) {
        Sensors_readRaw(rawSensor, NUM_SENSORS);
        Serial.print(F("RAW:"));
        for (uint8_t i=0;i<NUM_SENSORS;i++){ Serial.print(rawSensor[i]); if(i<NUM_SENSORS-1) Serial.print(','); }
        Serial.println();
      } else {
        Sensors_readCalibrated(blackNear, blackFar, whiteNear, whiteFar,
                               rawSensor, calSensor, NUM_SENSORS);
        int pos = PID_computePosition(calSensor, NUM_SENSORS);
        Serial.print(F("CAL:"));
        for (uint8_t i=0;i<NUM_SENSORS;i++){ Serial.print(calSensor[i]); if(i<NUM_SENSORS-1) Serial.print(','); }
        Serial.print(F(";POS:"));
        Serial.println(pos);
      }
    }
    return;
  }
  #endif

  if (sessionActive) {
    Sensors_readCalibrated(blackNear, blackFar, whiteNear, whiteFar,
                           rawSensor, calSensor, NUM_SENSORS);
    int pos = PID_computePosition(calSensor, NUM_SENSORS);
    int pid = PID_computeOutput(pos, LINE_SETPOINT);

    bool extremeNow = (pos == 0 || pos == 11000);
    if (extremeNow) {
      if (!posAtExtreme) {
        posAtExtreme = true;
        extremeStartMs = millis();
      } else if (!emgActive && (millis() - extremeStartMs >= 100UL)) {// tiempo de espera para apagado
        emgActive = true;
        stopRace();
        Serial.println(F(">> EMERGENCY STOP: line lost"));
      }
    } else {
      posAtExtreme = false;
      emgActive = false;
    }

    if (!emgActive) {
      int L = clampPWM(basePWM_current + pid);
      int R = clampPWM(basePWM_current - pid);
      Motors_cmdPct(L, R);
      UI_runStrobeUpdate(true);
    } else {
      Motors_cmdPct(0,0);
      UI_runStrobeUpdate(false);
    }
  } else {
    Motors_cmdPct(0,0);
    UI_runStrobeUpdate(false);
  }
}
