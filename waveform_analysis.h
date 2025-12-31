#ifndef waveform_analysis
#define waveform_analysis

#include <Arduino.h>
#include <SPI.h>

extern int formtype;  // External variable for waveform type
extern float Vpp;     // External variable for peak-to-peak voltage

void initWaveformAnalyzer();
void updateWaveformData();

#endif // WAVEFORM_ANALYZER_H
