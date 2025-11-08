#include "servo.h"

ServoControl::ServoControl(int servoPin1, int servoPin2) {
  pin1 = servoPin1;
  pin2 = servoPin2;
}

void ServoControl::begin() {
  // Allocate timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  
  // Attach servos
  servox.attach(pin1, 500, 2400);
  servoy.attach(pin2, 956, 1988); // ±50° from center

  
  Serial.println("Servo control initialized");
}

void ServoControl::setServoxAngle(int angle) {
  angle = constrain(angle, 0, 180);
  servox.write(angle);
}

void ServoControl::setServoyAngle(int angle) {
  angle = constrain(angle, 0, 180);
  servoy.write(angle);
}

void ServoControl::sweep(int servoNumber) {
  Servo* targetServo = (servoNumber == 1) ? &servox : &servoy;
  
  // Sweep forward
  for (int pos = 0; pos <= 180; pos += 1) {
    targetServo->write(pos);
    delay(15);
  }
}
