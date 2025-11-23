#pragma once
#include <stdint.h>

// MUX + raw sensors
void MUX_init();
void Sensors_readRaw(int *raw, uint8_t N);  // fills raw[0..N-1], 0..1023
#ifdef ENABLE_DIAG
void MUX_debugPrintMapping();
#endif

// Calibration capture/load/save and calibrated read (0..1000)
void Calib_load(int *bn, int *bf, int *wn, int *wf, uint8_t N);
void Calib_save(const int *bn,const int *bf,const int *wn,const int *wf, uint8_t N);

void Calib_captureBlackNear(int *bn, uint8_t N);
void Calib_captureBlackFar (int *bf, uint8_t N);
void Calib_captureWhiteNear(int *wn, uint8_t N);
void Calib_captureWhiteFar (int *wf, uint8_t N);

void Sensors_readCalibrated(const int *bn,const int *bf,
                            const int *wn,const int *wf,
                            int *raw,int *cal,uint8_t N);
