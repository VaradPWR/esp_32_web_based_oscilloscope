## Project Overview

A single-channel web-based oscilloscope built using the ESP32 and the Arduino framework, designed to visualize periodic waveforms and extract key signal parameters such as frequency and peak-to-peak voltage (Vpp) through a browser interface.

This project focuses on real-time measurement accuracy and bandwidth-efficient visualization rather than raw ADC streaming.

---

## Features

- Waveform visualization in a web browser
- Sampling frequency up to 3 kHz
- Frequency measurement using dedicated measurement logic with interpolation-based correction
- Peak-to-peak voltage (Vpp) calculation
- Supports sine, square, and triangular waveforms
- Modular C++ design beyond a single Arduino sketch

---

## Working

- Signal input is applied to a channel of the MCP3008 ADC
- Frequency is measured using dedicated measurement logic and corrected using mathematical interpolation
- Peak-to-peak voltage (Vpp) is calculated from ADC readings
- Crest factor is computed to estimate the type of waveform supplied
- Instead of streaming raw ADC samples (bandwidth-intensive), waveform parameters
  (Vpp, frequency, crest factor) are sent to the browser
- The waveform is reconstructed and rendered using canvas on HTML.

---

## Hardware Used

- ESP32 DevKit V1
- External ADC (MCP3008)
