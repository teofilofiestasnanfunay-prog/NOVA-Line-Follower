#pragma once
#include <Arduino.h>
#include <stdint.h>

// UI LEDs
void UI_init();
void UI_set(bool l1, bool l2);
void UI_previewMode(uint8_t mode);   // Preview: LED1 blinks N times (mode)
void UI_confirmMode(uint8_t mode);   // Confirm: LED2 blinks N times (mode)
void UI_showRun(bool running);       // LED2 on if running
void UI_runStrobeUpdate(bool running); // LED2 strobe (non-blocking) if running

// Mode button
void    ModeBtn_init();
uint8_t ModeBtn_poll();            // 0 = none, 1 = short, 2 = long
bool    ModeBtn_edgePressed();     // non-blocking, true once on press (LOW active)

// GO (run/stop) signal
void GO_init();
bool GO_read();        // true = RUN, false = STOP
bool GO_edgeStart();   // rising edge since last call
bool GO_edgeStop();    // falling edge since last call
