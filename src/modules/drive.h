#pragma once

// Motors
void Motors_init();
void Motors_cmdPct(int left_pwm, int right_pwm); // -255..+255
void Motors_brake();
