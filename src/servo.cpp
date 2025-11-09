#include "servo.h"

ServoControl::ServoControl(int servoPin1, int servoPin2, int servoPin3)
{
  pin1 = servoPin1;
  pin2 = servoPin2;
  pin3 = servoPin3;
}

void ServoControl::begin()
{
  // Allocate timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  // Attach servos
  servox.attach(pin1);
  servoy.attach(pin2);
  servotrig.attach(pin3);
  Serial.println("Servo control initialized");
  delay(3000);
}

// servoX stops at 1490 - 1540!!!
void ServoControl::setServoxSpeed(int speed)
{
  speed = constrain(speed, 1300, 1700);
  servox.writeMicroseconds(speed);
}

// servoY stops at 1470 - 1540
void ServoControl::setServoySpeed(int speed)
{
  speed = constrain(speed, 1300, 1700);
  servoy.writeMicroseconds(speed);
}

void ServoControl::findXSpeed(int angle)
{
  if (angle > 0)
    servox.writeMicroseconds(1535 + angle / 0.10);
  else
    servox.writeMicroseconds(1495 + angle / 0.10);
}

void ServoControl::findYSpeed(int angle)
{
  if (angle > 0)
    servoy.writeMicroseconds(1535 + angle / 0.20);
  else
    servoy.writeMicroseconds(1475 + angle / 0.20);
}

void ServoControl::fireTrigger()
{
  servotrig.writeMicroseconds(2000);
  delay(250);
  servotrig.writeMicroseconds(1500);
}