#include <WiFi.h>
#include <WebServer.h>
#include <Servo.h>
#include <Arduino.h>

const char *ssid = "Catalyst-Commons Guest";
const char *password = "";

Servo servo;
WebServer server(80);

void handleServo()
{
  if (server.hasArg("angle"))
  {
    int angle = server.arg("angle").toInt();
    servo.write(angle);
    server.send(200, "text/plain", "Servo moved to " + String(angle));
  }
  else
  {
    server.send(400, "text/plain", "Missing angle parameter");
  }
}

void setup()
{
  Serial.begin(115200);
  servo.attach(13); // connect servo signal to pin 13
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
    delay(1000);
  Serial.println(WiFi.localIP());

  server.on("/servo", handleServo);
  server.begin();
}

void loop()
{
  server.handleClient();
}
