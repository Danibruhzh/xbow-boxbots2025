#include <WiFi.h>
#include <WebServer.h>
#include <Servo.h>
#include <Arduino.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>
#include "servo.h"

const char *ssid = "DanielZh";
const char *password = "googoogaga1";

WebServer server(80);

// Store latest gyro data
float alpha = 0;
float beta = 0;
float xangle_prev = 0;
float yangle_prev = 0;
float xchange = 0;
float ychange = 0;
unsigned long lastUpdate = 0;

// Create servo controller object
ServoControl servoController(18, 19); // pins 18 and 19

void handleOrientationData()
{
  if (server.hasArg("plain"))
  {
    String body = server.arg("plain");
    Serial.println("Received: " + body);

    // Parse JSON properly
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, body);

    if (error)
    {
      Serial.print("JSON parse error: ");
      Serial.println(error.c_str());
      server.send(400, "text/plain", "Invalid JSON");
      return;
    }

    // Extract alpha and beta from JSON
    if (doc.containsKey("alpha") && doc.containsKey("beta"))
    {
      alpha = doc["alpha"];
      beta = doc["beta"];
      float xrefAngle = alpha <= 90 ? alpha : 360 - alpha;

      xchange = xrefAngle - xangle_prev;
      ychange = beta - yangle_prev;
      // reset the angle
      xangle_prev = xrefAngle;
      yangle_prev = beta;
      lastUpdate = millis();

      Serial.printf("✅ Orientation - Alpha: %.2f°, Beta: %.2f°\n", alpha, beta);

      // Add CORS headers
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(200, "text/plain", "OK");
    }
    else
    {
      Serial.println("Missing alpha or beta fields");
      server.send(400, "text/plain", "Missing fields");
    }
  }
  else
  {
    server.send(400, "text/plain", "No data");
  }
}

void handleRoot()
{
  server.send(200, "text/plain", "ESP32 Orientation Server Running");
}

void handleOptions()
{
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(204);
}

void setup()
{
  Serial.begin(115200);
  servoController.begin();

  Serial.println("ESP32 Ready");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ Connected to WiFi!");
  Serial.print("📱 Open this URL on your phone: http://");
  Serial.println(WiFi.localIP());

  // Setup HTTP endpoints
  server.on("/", HTTP_GET, handleRoot);
  server.on("/gyro", HTTP_POST, handleOrientationData);
  server.on("/gyro", HTTP_OPTIONS, handleOptions);

  server.begin();
  Serial.println("Server started!");
}

void loop()
{
  server.handleClient();
  static float xalpha = 0; //how much the x is going to move
  static float xrefAngleOld = 0;
  static float ybeta = 0;
  float xrefAngleNew = alpha <= 170 ? alpha : alpha-360;
  // Example: Print data every second if it's being updated
  static unsigned long lastPrint = 0;
  unsigned long timediff = millis() - lastPrint;
  if (timediff >= 100 && millis() - lastUpdate < 2000)
  {
    xalpha = xrefAngleNew - xrefAngleOld;
    xrefAngleOld = xrefAngleNew;
    ybeta = ychange;
    Serial.printf("📊 Current - Alpha: %.2f°, Beta: %.2f°, XAlpha: %.2f°\n", xrefAngleNew, ybeta, xalpha);
    servoController.findXSpeed(xalpha);
    lastPrint = millis();
  }

  
  //servoController.updateServos(); 

  // Timeout indicator
  if (millis() - lastUpdate > 5000 && lastUpdate > 0)
  {
    static unsigned long lastWarning = 0;
    if (millis() - lastWarning > 5000)
    {
      Serial.println("⚠️ No data received for 5 seconds");
      lastWarning = millis();
    }
  }

  // HERE: Use alpha and beta to control your servos
  // Example:
  // if (millis() - lastUpdate < 2000) { // Only if receiving recent data
  //     int servoXAngle = map(alpha, 0, 360, 0, 180);
  //     int servoYAngle = map(beta, -90, 90, 40, 140);

  //     servoController.setServoxAngle(servoXAngle);
  //     servoController.setServoyAngle(servoYAngle);
  // }

  // servoController.sweep();
}
