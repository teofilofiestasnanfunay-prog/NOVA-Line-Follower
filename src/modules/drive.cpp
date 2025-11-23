#include <Arduino.h>
#include "config_pins.h"
#include "modules/utils.h"
#include "drive.h"

// ================= Motors =================
void Motors_init() {
  pinMode(PIN_DIR_L, OUTPUT);
  pinMode(PIN_DIR_R, OUTPUT);
  pinMode(PIN_PWM_L, OUTPUT);
  pinMode(PIN_PWM_R, OUTPUT);
  analogWrite(PIN_PWM_L, 0);
  analogWrite(PIN_PWM_R, 0);
}

void Motors_cmdPct(int left_pwm, int right_pwm) {
  left_pwm  = clampPWM(left_pwm);
  right_pwm = clampPWM(right_pwm);

  if (left_pwm >= 0) {
    digitalWrite(PIN_DIR_L, HIGH);
    analogWrite(PIN_PWM_L, (uint8_t)left_pwm);
  } else {
    digitalWrite(PIN_DIR_L, LOW);
    analogWrite(PIN_PWM_L, (uint8_t)(-left_pwm));
  }

  if (right_pwm >= 0) {
    digitalWrite(PIN_DIR_R, HIGH);
    analogWrite(PIN_PWM_R, (uint8_t)right_pwm);
  } else {
    digitalWrite(PIN_DIR_R, LOW);
    analogWrite(PIN_PWM_R, (uint8_t)(-right_pwm));
  }
}

void Motors_brake() {
  const int BRAKE_MAX = 80;   // 40
  const int BRAKE_STEP_MS = 1; // 2
  for (int p = 0; p <= BRAKE_MAX; ++p) {
    digitalWrite(PIN_DIR_L, LOW);
    digitalWrite(PIN_DIR_R, LOW);
    analogWrite(PIN_PWM_L, (uint8_t)p);
    analogWrite(PIN_PWM_R, (uint8_t)p);
    delay(BRAKE_STEP_MS);
  }
  analogWrite(PIN_PWM_L, 0);
  analogWrite(PIN_PWM_R, 0);
}
