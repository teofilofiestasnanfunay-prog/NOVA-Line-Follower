#pragma once
#include <Arduino.h>

inline int clampInt(int x, int a, int b) {
  if (x < a) return a;
  if (x > b) return b;
  return x;
}

// Clamp for signed 8-bit PWM values in range [-255..255]
inline int clampPWM(int x) {            // [-255..255]
  return clampInt(x, -255, 255);
}
