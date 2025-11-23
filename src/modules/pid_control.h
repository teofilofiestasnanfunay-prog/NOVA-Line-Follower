#pragma once
#include <stdint.h>

void PID_init(float kp, float kd);
int  PID_computePosition(const int *cal, uint8_t N);  // 0..(N-1)*1000
int  PID_computeOutput(int pos, int setpoint);        // PD output
