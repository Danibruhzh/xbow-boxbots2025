#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <ESP32Servo.h>

class ServoControl {
private:
  Servo servox;
  Servo servoy;
  int pin1;
  int pin2;

public:
  ServoControl(int servoPin1, int servoPin2);
  void begin();
  void setServoxAngle(int angle);
  void setServoyAngle(int angle);
  void sweep(int servoNumber);
};

#endif
