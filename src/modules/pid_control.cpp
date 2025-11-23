#include <Arduino.h>
#include "pid_control.h"

static float Kp = 0.02f, Kd = 0.05f;
static int   lastErr = 0;

void PID_init(float kp, float kd) {
  Kp = kp; Kd = kd;
  lastErr = 0;
}

int PID_computePosition(const int *cal, uint8_t N) {
  long sum = 0, weight = 0;
  for (uint8_t i = 0; i < N; i++) {
    int val = cal[i];         // 0..1000
    sum    += val;
    weight += (long)val * i * 1000;
  }
  static int lastPos = 0;
  if (sum == 0) return lastPos;
  lastPos = (int)(weight / sum);
  return lastPos;
}

int PID_computeOutput(int pos, int setpoint) {
  int e = pos - setpoint;
  int u = (int)(Kp * e + Kd * (e - lastErr));
  lastErr = e;
  return u;
}
