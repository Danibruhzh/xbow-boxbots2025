#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <ESP32Servo.h>

class ServoControl {
private:
  Servo servo1;
  Servo servo2;
  int pin1;
  int pin2;

public:
  ServoControl(int servoPin1, int servoPin2);
  void begin();
  void setServo1Angle(int angle);
  void setServo2Angle(int angle);
  void sweep(int servoNumber);
};

#endif
