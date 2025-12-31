#include <Arduino.h>
#include <arduinoFFT.h>
#include "FrequencyCorrector.h"

#define ADC_INPUT_PIN 34
#define SAMPLES 2048
#define SAMPLING_FREQUENCY 8192

double vReal[SAMPLES];
double vImag[SAMPLES];

arduinoFFT FFT = arduinoFFT(vReal, vImag, SAMPLES, SAMPLING_FREQUENCY);

const int tableSize = 10;
const float detectedTable[tableSize] = {
  24.77, 64.7, 491.2, 776.8, 870, 916.1, 962.5, 1191.4, 1238, 1326.6
};
const float actualTable[tableSize] = {
  25,    65,   500,   800,   900,  950,   1000,  1250, 1300, 1400
};

float CorrectFrequency = 0;

void initFrequencyCorrector() {
  // Any setup needed (if any), currently nothing
}

void updateCorrectFrequencyLoop() {
  unsigned long lastSampleTime = micros();
  noInterrupts();
  for (int i = 0; i < SAMPLES; i++) {
    while (micros() - lastSampleTime < (1000000 / SAMPLING_FREQUENCY)) {}
    lastSampleTime = micros();
    vReal[i] = analogRead(ADC_INPUT_PIN) * (3.3 / 4095.0);
    vImag[i] = 0;
  }
  interrupts();

  FFT.Windowing(FFT_WIN_TYP_HANN, FFT_FORWARD);
  FFT.Compute(FFT_FORWARD);
  FFT.ComplexToMagnitude();

  double rawFrequency = FFT.MajorPeak();

  for (int i = 0; i < tableSize - 1; i++) {
    if (rawFrequency >= detectedTable[i] && rawFrequency <= detectedTable[i + 1]) {
      float x0 = detectedTable[i];
      float x1 = detectedTable[i + 1];
      float y0 = actualTable[i];
      float y1 = actualTable[i + 1];
      CorrectFrequency = y0 + (rawFrequency - x0) * (y1 - y0) / (x1 - x0);
      return;
    }
  }
  CorrectFrequency = rawFrequency;
}

