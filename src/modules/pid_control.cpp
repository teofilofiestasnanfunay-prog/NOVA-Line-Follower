#include <Arduino.h>
#include "pid_control.h"

static float Kp = 0.02f, Kd = 0.05f;
static int   lastErr = 0;

void PID_init(float kp, float kd) {
  Kp = kp; Kd = kd;
  lastErr = 0;
}

int PID_computePosition(const int *cal, uint8_t N) {
  const int maxPos = (int)(N - 1) * 1000;   // e.g., 11000 for 12 sensors
  const int midPos = maxPos / 2;
  long sum = 0, weight = 0;
  static int  lastPos = midPos;
  static bool lastWasLeft = true;           // keep side for zero-sum fallback
  for (uint8_t i = 0; i < N; i++) {
    int val = cal[i];         // 0..1000
    sum    += val;
    weight += (long)val * i * 1000;
  }
  // If all sensors report 0, return an extreme (0 or max) based on last known side.
  if (sum == 0) {
    lastPos = lastWasLeft ? 0 : maxPos;
    return lastPos;
  }
  lastPos = (int)(weight / sum);
  // Track which side was last dominant to inform the zero-sum fallback.
  if (lastPos < midPos)      lastWasLeft = true;
  else if (lastPos > midPos) lastWasLeft = false;
  return lastPos;
}

int PID_computeOutput(int pos, int setpoint) {
  int e = pos - setpoint;
  int u = (int)(Kp * e + Kd * (e - lastErr));
  lastErr = e;
  return u;
}
