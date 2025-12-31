#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "waveform_analysis.h"
#include "FrequencyCorrector.h"

// WiFi Credentials 
const char* ssid = "enter ssid";
const char* password = "enter password";

// Web server on port 80 
AsyncWebServer server(80);

// Simulated data () 
float frequency = 1000;  // Hz 
float amplitude = 1.5;   // Vpp 
int wavetype = 1;        // 1 = Sine, 2 = Square, 3 = Triangle 

// Serve HTML page 
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Digital Oscilloscope</title>
  <style>
    body { 
      font-family: Arial, sans-serif; 
      background-color: #121212; 
      color: #ffffff; 
      text-align: center; 
      margin: 0; 
      padding: 20px; 
    }

    h1 { 
      font-size: 2rem; 
      text-transform: uppercase; 
      letter-spacing: 2px; 
      text-shadow: 0px 0px 10px #00ffcc; 
    }

    #waveform { 
      display: block; 
      margin: 20px auto; 
      background-color: #000; 
      border: 2px solid #00ffcc; 
      box-shadow: 0px 0px 20px #00ffcc; 
      border-radius: 10px; 
    }

    .slider-container { 
      max-width: 600px; 
      margin: 20px auto; 
      padding: 15px; 
      background: rgba(255, 255, 255, 0.1); 
      border-radius: 10px; 
      box-shadow: 0px 0px 15px rgba(0, 255, 204, 0.5); 
    }

    .slider { 
      display: flex; 
      align-items: center; 
      margin-bottom: 15px; 
      padding: 10px; 
      border-radius: 5px; 
    }

    .slider label { 
      width: 160px; 
      font-size: 1rem; 
      font-weight: bold; 
      color: #00ffcc; 
    }

    .slider input { 
      flex: 1; 
      max-width: 200px; 
      appearance: none; 
      height: 6px; 
      background: #00ffcc; 
      border-radius: 5px; 
      outline: none; 
      transition: 0.2s; 
    }

    .slider input::-webkit-slider-thumb { 
      appearance: none; 
      width: 15px; 
      height: 15px; 
      background: #ffffff; 
      border-radius: 50%; 
      cursor: pointer; 
      box-shadow: 0px 0px 10px #00ffcc; 
    }

    .slider span { 
      margin-left: 10px; 
      font-size: 1rem; 
      font-weight: bold; 
      color: #ffffff; 
    }

    #info-box {
      position: absolute;
      top: 20px;
      right: 20px;
      padding: 12px 18px;
      background-color: rgba(0, 255, 204, 0.15);
      border: 1px solid #00ffcc;
      border-radius: 10px;
      color: #ffffff;
      font-size: 0.95rem;
      box-shadow: 0px 0px 10px #00ffcc;
      text-align: left;
    }

    #info-box p {
      margin: 4px 0;
    }
  </style>
</head>
<body>
  <h1>Digital Oscilloscope</h1>

  <canvas id="waveform" width="800" height="400"></canvas>

  <!-- Info Box (Top Right Corner) -->
  <div id="info-box">
    <p><strong>Amplitude:</strong> <span id="amplitude-val">--</span> V</p>
    <p><strong>Frequency:</strong> <span id="frequency-val">--</span> Hz</p>
  </div>

  <div class="slider-container">
    <div class="slider">
      <label for="voltage-slider">Volts/Div:</label>
      <input id="voltage-slider" type="range" min="0.1" max="5.0" step="0.1" value="1.0">
      <span id="voltage-value">1.0</span>
    </div>
    <div class="slider">
      <label for="time-slider">Time/Div:</label>
      <input id="time-slider" type="range" min="-4" max="0.301" step="0.01" value="0">
      <span id="time-value">1.0 s</span>
    </div>
  </div>

  <script>
    let waveType = 1;
    let inputFrequency = 1000;
    let amplitude = 1.5;
    
    const canvas = document.getElementById('waveform');
    const ctx = canvas.getContext('2d');

    let voltsPerDivision = 1.0;
    let timePerDivision = 1.0;
    const divisions = 10;

    async function fetchWaveformData() {
      try {
        const response = await fetch('/get_data');
        if (!response.ok) throw new Error("Failed to fetch data");
        const data = await response.json();

        inputFrequency = parseFloat(data.frequency);
        amplitude = parseFloat(data.amplitude);
        waveType = parseInt(data.wavetype);

        // Update UI
        document.getElementById("amplitude-val").innerText = amplitude.toFixed(2);
        document.getElementById("frequency-val").innerText = inputFrequency.toFixed(1);
      } catch (error) {
        console.error("Error fetching data:", error);
      }
    }

    document.getElementById('voltage-slider').addEventListener('input', (e) => {
      voltsPerDivision = parseFloat(e.target.value);
      document.getElementById('voltage-value').innerText = voltsPerDivision.toFixed(1);
    });

    document.getElementById('time-slider').addEventListener('input', (e) => {
      const logValue = parseFloat(e.target.value);
      timePerDivision = Math.pow(10, logValue);
      const displayValue = timePerDivision < 1 ? 
        `${(timePerDivision * 1000).toFixed(1)} ms` : 
        `${timePerDivision.toFixed(1)} s`;
      document.getElementById('time-value').innerText = displayValue;
    });

    function drawGrid() {
      ctx.strokeStyle = "white";
      ctx.lineWidth = 0.5;
      
      for (let x = 0; x <= canvas.width; x += canvas.width / divisions) {
        ctx.beginPath();
        ctx.moveTo(x, 0);
        ctx.lineTo(x, canvas.height);
        ctx.stroke();
      }
      
      for (let y = 0; y <= canvas.height; y += canvas.height / divisions) {
        ctx.beginPath();
        ctx.moveTo(0, y);
        ctx.lineTo(canvas.width, y);
        ctx.stroke();
      }
    }

    function generateWaveform() {
      const waveform = [];
      const totalTime = timePerDivision * divisions;
      const period = 1 / inputFrequency;

      for (let x = 0; x < canvas.width; x++) {
        const t = (x / canvas.width) * totalTime;
        let value;
        
        switch (waveType) {
          case 1: value = amplitude * Math.sin(2 * Math.PI * inputFrequency * t); break;
          case 2: value = ((t / period) % 1) < 0.5 ? amplitude : -amplitude; break;
          case 3: value = 4 * amplitude * (0.5 - Math.abs((t / period) % 1 - 0.5)) - amplitude; break;
        }
        
        waveform.push(value);
      }
      return waveform;
    }

    function drawWaveform(waveformData) {
      ctx.clearRect(0, 0, canvas.width, canvas.height);
      drawGrid();

      ctx.beginPath();
      ctx.strokeStyle = 'hotpink';
      ctx.lineWidth = 2;

      for (let x = 0; x < waveformData.length; x++) {
        const y = canvas.height / 2 - (waveformData[x] * (canvas.height / 2)) / voltsPerDivision;
        if (x === 0) ctx.moveTo(x, y);
        else ctx.lineTo(x, y);
      }
      ctx.stroke();
    }

    async function update() {
      await fetchWaveformData();
      const waveform = generateWaveform();
      drawWaveform(waveform);
      requestAnimationFrame(update);
    }

    update();
  </script>
</body>
</html>

)rawliteral";

void handleDataRequest(AsyncWebServerRequest *request) {
    String json = "{\"frequency\":" + String(frequency) +
                  ",\"amplitude\":" + String(amplitude) +
                  ",\"wavetype\":" + String(wavetype) + "}";
    request->send(200, "application/json", json);
}

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { delay(1000); Serial.println("Connecting..."); }
    Serial.println("Connected!");
    Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html);
    });

    server.on("/get_data", HTTP_GET, handleDataRequest);
    server.begin();

    initWaveformAnalyzer();
    initFrequencyCorrector();
}

void loop() {
    updateWaveformData();
    static unsigned long lastFreqUpdate = 0;
    if (millis() - lastFreqUpdate > 500) {
        lastFreqUpdate = millis();
        updateCorrectFrequencyLoop();
    }
  
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 2000) {
        lastUpdate = millis();
        frequency = CorrectFrequency;
        amplitude = Vpp;
        wavetype = formtype;
    }
}
