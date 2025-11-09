#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <ESP32Servo.h>

class ServoControl {
private:
  Servo servox;
  Servo servoy;
  int pin1;
  int pin2;
  
  int currentSpeedX = 1510;   // current servo X speed
  int targetSpeedX = 1510;    // target speed based on angle
  unsigned long lastMoveTimeX = 0; // last time we updated servo

public:
  ServoControl(int servoPin1, int servoPin2);
  void begin();
  void setServoxSpeed(int micro);
  void setServoySpeed(int micro);
  void setServoxRead();
  void servoxOneDegree();
  void servoyOneDegree();
  void findXSpeed(int angle);
  void updateServos();
};

#endif
