#pragma once

// ===================== PARAMETERS =====================
// PID
inline float Kp_default() { return 0.0405f; } // Kp = 0.0235 -> 255*0.0235 ~ 6
inline float Kd_default() { return 0.665f; }  // Kd = 0.23529 -> 255*0.23529 ~ 60
#define LINE_SETPOINT     5500  // with 12 sensors: 0..11000

#define BASE_PWM_DEFAULT    80   // [0..255]

// Motor ramp (left and right share the same ramp)
#define MOTOR_RAMP_STEP         20    // [0..255 per step]
#define MOTOR_RAMP_INTERVAL_MS  1     // ms between steps

// Run session management
#define RUN_SESSION_MS      4500   // auto-stop window (ms) after a start (GO or button)

// LED patterns
#define LED2_STROBE_MS       500   // non-blocking toggle interval during run

// EEPROM layout (same as your previous code)
#define EE_OFF_BLACK_NEAR   100
#define EE_OFF_BLACK_FAR    200
#define EE_OFF_WHITE_NEAR   300
#define EE_OFF_WHITE_FAR    400
