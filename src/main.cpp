#include <Arduino.h>
#include <ESP32Servo.h>
#include "servo.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

const char *ssid = "DanielZh";
const char *password = "googoogaga1";

WebServer server(80);

// Store gyro data
float alpha = 0;
float beta = 0;
float xangle_prev = 0;
float yangle_prev = 0;
float xchange = 0;
float ychange = 0;
unsigned long lastUpdate = 0;
int updateInterval = 120; // start with 120 ms
const int minInterval = 100;
const int maxInterval = 200;

ServoControl servoController(18, 19, 13); // pins 18, 19, and 13

void handleOrientationData()
{
  if (server.hasArg("plain"))
  {
    String body = server.arg("plain");

    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, body);

    if (error)
    {
      Serial.print("JSON parse error: ");
      Serial.println(error.c_str());
      server.send(400, "text/plain", "Invalid JSON");
      return;
    }

    if (doc.containsKey("alpha") && doc.containsKey("beta"))
    {
      alpha = doc["alpha"];
      beta = doc["beta"];
      lastUpdate = millis();

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

void handleTrigger()
{
  if (server.method() == HTTP_POST)
  {
    servoController.fireTrigger();
    server.send(200, "text/plain", "Bullet fired!");
  }
  else
  {
    server.send(200, "text/plain", "POST required");
  }
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
  server.on("/servo", handleTrigger);

  server.begin();
  Serial.println("Server started!");
}

void loop()
{
  server.handleClient();
  static float xchange = 0; // how much the x is going to move
  static float xrefAngleOld = 0;
  static float ychange = 0;
  static float yrefAngleOld = 0;
  float xrefAngleNew = alpha <= 178 ? alpha : alpha - 360;
  float yrefAngleNew = beta <= 178 ? beta : beta - 360;
  static unsigned long lastPrint = 0;
  unsigned long timediff = millis() - lastPrint;
  if (timediff >= updateInterval && millis() - lastUpdate < 2000)
  {
    xchange = xrefAngleNew - xrefAngleOld;
    xrefAngleOld = xrefAngleNew;
    ychange = yrefAngleNew - yrefAngleOld;
    yrefAngleOld = yrefAngleNew;
    Serial.printf("📊 Current - Alpha: %.2f°, Beta: %.2f°, Xchange: %.2f°, Ychange: %.2f°\n", xrefAngleNew, yrefAngleNew, xchange, ychange);
    servoController.findXSpeed(xchange);
    servoController.findYSpeed(ychange);
    lastPrint = millis();

    // dynamic interval adjustment
    if (abs(xchange) > 2 || abs(ychange) > 2)
    {
      // decrease interval if movement is fast
      updateInterval = max(minInterval, updateInterval - 10);
    }
    else
    {
      // increase interval if Movement is slow
      updateInterval = min(maxInterval, updateInterval + 5);
    }
  }

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
}
