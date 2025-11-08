#include <WiFi.h>
#include <WebServer.h>
#include <Servo.h>
#include <Arduino.h>
#include <ESP32Servo.h>
#include "servo.h"

// Create servo controller object
ServoControl servoController(18, 19); // pins 18 and 19

void setup()
{
  Serial.begin(115200);
  servoController.begin();
  Serial.println("ESP32 Ready");
}

void loop()
{
  // Example usage
  servoController.setServo1Angle(90);
  delay(1000);

  servoController.setServo2Angle(45);
  delay(1000);

  servoController.sweep(1); // Sweep servo 1
  delay(500);
}
