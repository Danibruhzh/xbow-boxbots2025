#include <Arduino.h>
#include <ESP32Servo.h>
#include "servo.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

const char* ssid = "DanielZh";
const char* password = "googoogaga1";

WebServer server(80);

// Store latest gyro data
float gyroX = 0, gyroY = 0, gyroZ = 0;
unsigned long lastUpdate = 0;

// Create servo controller object
ServoControl servoController(18, 19);  // pins 18 and 19

void setup() {
  Serial.begin(115200);
  servoController.begin();
  Serial.println("ESP32 Ready");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");


  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ Connected to WiFi!");
  Serial.print("📱 Open this URL on your phone: http://");
  Serial.println(WiFi.localIP());

  // Setup HTTP endpoints
  server.on("/", HTTP_GET, handleRoot);
  server.on("/gyro", HTTP_POST, handleGyroData);
  
  server.begin();
  Serial.println("Server started!");

  
}

void loop() {
  // Example usage
  servoController.setServo1Angle(90);
  delay(1000);
  
  servoController.setServo2Angle(45);
  delay(1000);
  
  servoController.sweep(1);  // Sweep servo 1
  delay(500);

  server.handleClient();
  
  // HERE: Use gyroX, gyroY, gyroZ to control your project!
  // Examples:
  // - Control servos: servo.write(map(gyroX, -180, 180, 0, 180));
  // - Control LEDs based on tilt
  // - Drive a robot based on phone orientation
  
  // Example: Print data every second if it's being updated
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000 && millis() - lastUpdate < 2000) {
    Serial.printf("Current Gyro - X: %.2f, Y: %.2f, Z: %.2f\n", gyroX, gyroY, gyroZ);
    lastPrint = millis();
  }
}

void handleGyroData() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    
    // Manual JSON parsing (no library needed!)
    int xPos = body.indexOf("\"x\":");
    int yPos = body.indexOf("\"y\":");
    int zPos = body.indexOf("\"z\":");
    
    if (xPos != -1 && yPos != -1 && zPos != -1) {
      // Extract values
      gyroX = body.substring(xPos + 4, body.indexOf(',', xPos)).toFloat();
      gyroY = body.substring(yPos + 4, body.indexOf(',', yPos)).toFloat();
      gyroZ = body.substring(zPos + 4, body.indexOf('}', zPos)).toFloat();
      
      lastUpdate = millis();
      
      Serial.printf("Gyro - X: %.2f, Y: %.2f, Z: %.2f\n", gyroX, gyroY, gyroZ);
      
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Invalid format");
    }
  } else {
    server.send(400, "text/plain", "No data");
  }
}

void handleRoot() {
  // Serve the control page
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 Gyro Control</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      padding: 20px;
      text-align: center;
      background: #1a1a1a;
      color: white;
      margin: 0;
    }
    h1 { color: #4CAF50; }
    .status {
      padding: 20px;
      margin: 20px 0;
      border-radius: 10px;
      font-size: 18px;
    }
    .connected { background: #2d5016; }
    .disconnected { background: #501616; }
    .tracking { background: #16355a; }
    .data {
      background: #2a2a2a;
      padding: 20px;
      border-radius: 10px;
      margin: 20px 0;
      font-family: monospace;
      font-size: 20px;
    }
    button {
      padding: 20px 40px;
      font-size: 20px;
      border: none;
      border-radius: 10px;
      background: #4CAF50;
      color: white;
      cursor: pointer;
      margin: 10px;
    }
    button.stop { background: #f44336; }
    .value { color: #4CAF50; font-weight: bold; }
  </style>
</head>
<body>
  <h1>📱 ESP32 Gyro Control</h1>
  
  <div id="status" class="status disconnected">
    Ready to start
  </div>
  
  <button id="startBtn" onclick="start()">Start Tracking</button>
  
  <div class="data">
    <h3>Gyroscope Data</h3>
    <div id="gyroData">
      X: <span class="value">0.00</span> °/s<br>
      Y: <span class="value">0.00</span> °/s<br>
      Z: <span class="value">0.00</span> °/s
    </div>
  </div>

  <script>
    let isTracking = false;

    async function start() {
      const btn = document.getElementById('startBtn');
      const status = document.getElementById('status');
      
      if (isTracking) {
        isTracking = false;
        btn.textContent = 'Start Tracking';
        btn.classList.remove('stop');
        status.textContent = 'Stopped';
        status.className = 'status disconnected';
        window.removeEventListener('devicemotion', handleMotion);
        return;
      }

      // Request permissions for iOS
      if (typeof DeviceMotionEvent !== 'undefined' && 
          typeof DeviceMotionEvent.requestPermission === 'function') {
        try {
          const permission = await DeviceMotionEvent.requestPermission();
          if (permission !== 'granted') {
            alert('Permission denied for motion sensors');
            return;
          }
        } catch (error) {
          alert('Error: ' + error);
          return;
        }
      }

      isTracking = true;
      btn.textContent = 'Stop Tracking';
      btn.classList.add('stop');
      status.textContent = '✅ Tracking Active';
      status.className = 'status tracking';
      
      window.addEventListener('devicemotion', handleMotion);
    }

    let lastSendTime = 0;
    const sendThrottle = 50; // Send every 50ms max

    function handleMotion(event) {
      if (!isTracking) return;

      const now = Date.now();
      if (now - lastSendTime < sendThrottle) return;
      lastSendTime = now;

      const gyro = event.rotationRate;
      if (!gyro) return;

      const data = {
        x: parseFloat((gyro.alpha || 0).toFixed(2)),
        y: parseFloat((gyro.beta || 0).toFixed(2)),
        z: parseFloat((gyro.gamma || 0).toFixed(2))
      };

      // Update display
      document.getElementById('gyroData').innerHTML = 
        'X: <span class="value">' + data.x + '</span> °/s<br>' +
        'Y: <span class="value">' + data.y + '</span> °/s<br>' +
        'Z: <span class="value">' + data.z + '</span> °/s';

      // Send to ESP32
      fetch('/gyro', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(data)
      }).catch(err => console.error('Send error:', err));
    }
  </script>
</body>
</html>
  )rawliteral";
  
  server.send(200, "text/html", html);
}