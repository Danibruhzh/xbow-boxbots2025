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
  servo1.attach(pin1, 500, 2400);
  servo2.attach(pin2, 500, 2400);
  
  Serial.println("Servo control initialized");
}

void ServoControl::setServo1Angle(int angle) {
  angle = constrain(angle, 0, 180);
  servo1.write(angle);
}

void ServoControl::setServo2Angle(int angle) {
  angle = constrain(angle, 0, 180);
  servo2.write(angle);
}

void ServoControl::sweep(int servoNumber) {
  Servo* targetServo = (servoNumber == 1) ? &servo1 : &servo2;
  
  // Sweep forward
  for (int pos = 0; pos <= 180; pos += 1) {
    targetServo->write(pos);
    delay(15);
  }
  
  // Sweep back
  for (int pos = 180; pos >= 0; pos -= 1) {
    targetServo->write(pos);
    delay(15);
  }
}
