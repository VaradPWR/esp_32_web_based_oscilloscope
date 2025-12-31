#include "joker.h"

#define CS_PIN 15  // MCP3008 Chip Select (change if needed)
#define ADC_MAX 1023  // MCP3008 is 10-bit (0-1023)
#define V_REF 3.3  // MCP3008 Reference Voltage
#define SAMPLE_TIME 20  // Sampling time in milliseconds

int formtype = 0;  // Variable to store waveform type (1: Sine, 2: Square, 3: Triangle)
float Vpp = 0.0;   // Variable to store peak-to-peak voltage

void initWaveformAnalyzer() {
    SPI.begin(18, 19, 23, CS_PIN);
    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);
    delay(100);
}

// Function to read from MCP3008
uint16_t readMCP3008(uint8_t channel) {
    if (channel > 7) return 0;  // MCP3008 has 8 channels (0-7)

    digitalWrite(CS_PIN, LOW);
    SPI.beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0));

    uint8_t command = 0b00000001;  // Start bit
    uint8_t config = (0b1000 | channel) << 4;

    SPI.transfer(command);
    uint8_t highByte = SPI.transfer(config) & 0x03;  // Get 2 MSB bits
    uint8_t lowByte = SPI.transfer(0x00);  // Get 8 LSB bits

    SPI.endTransaction();
    digitalWrite(CS_PIN, HIGH);

    return (highByte << 8) | lowByte;
}


void updateWaveformData() {
    int Vmin = ADC_MAX, Vmax = 0;
    float sumSquares = 0.0;
    float sumVoltage = 0.0;
    int sampleCount = 0;
    bool isClipping = false;

    // First pass: Find raw measurements
    unsigned long startTime = millis();
    while (millis() - startTime < SAMPLE_TIME) {
        int adcValue = readMCP3008(0);
        float voltage = (adcValue * V_REF) / ADC_MAX;

        // Detect clipping at boundaries
        if (adcValue <= 5 || adcValue >= ADC_MAX-5) isClipping = true;

        if (adcValue > Vmax) Vmax = adcValue;
        if (adcValue < Vmin) Vmin = adcValue;

        sumVoltage += voltage;
        sampleCount++;
    }

    // Calculate apparent DC offset
    float Vavg = sumVoltage / sampleCount;
    
    // If clipping detected at lower end, assume pure AC signal
    if (isClipping && Vmin <= 5) {
        // Calculate estimated Vpp based on clipped waveform
        Vpp = (Vmax * V_REF) / ADC_MAX * 2;  // Assume symmetric clipping
        Vavg = Vpp / 2;  // Set virtual DC offset to center waveform
    } else {
        // Normal Vpp calculation
        Vpp = ((Vmax - Vmin) * V_REF) / ADC_MAX;
    }

    sumSquares = 0.0;
    sampleCount = 0;
    startTime = millis();
    
    while (millis() - startTime < SAMPLE_TIME) {
        int adcValue = readMCP3008(0);
        float voltage = (adcValue * V_REF) / ADC_MAX;
        
        // Apply dynamic offset correction
        voltage -= Vavg;
        
        sumSquares += voltage * voltage;
        sampleCount++;
    }

    // Calculate RMS and crest factor
    float Vrms = sqrt(sumSquares / sampleCount);
    float Vpeak = Vpp / 2;
    float crestFactor = (Vrms > 0.1) ? Vpeak / Vrms : 0; 

    // Enhanced waveform detection
    if (crestFactor >= 1.3 && crestFactor <= 1.45) {
        formtype = 1;  // Sine wave
    } else if (crestFactor >= 0.95 && crestFactor <= 1.05) {
        formtype = 2;  // Square wave
    } else if (crestFactor >= 1.5 && crestFactor <= 1.7) {
        formtype = 3;  // Triangle wave
    } else {
        formtype = 0;  // Unknown
    }
}

