#include "servo.h"

ServoControl::ServoControl(int servoPin1, int servoPin2)
{
  pin1 = servoPin1;
  pin2 = servoPin2;
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
  servoy.attach(pin2, 956, 1988); // ±50° from center
  delay(3000);
  Serial.println("Servo control initialized");
}

// void ServoControl::setServoxRead() {
//   const int angle = servox.read();
//   Serial.println(angle);
// }

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

// void ServoControl::servoxOneDegree() {
//   servox.writeMicroseconds(1460);
//   delay(30);
//   servox.writeMicroseconds(1500);
// }

// void ServoControl::servoyOneDegree() {
//   servoy.writeMicroseconds(1480);
//   delay(30);
//   servoy.writeMicroseconds(1500);
// }

void ServoControl::findXSpeed(int angle)
{
  //targetSpeedX = constrain(1510 - angle / 0.06, 1300, 1700);
  servox.writeMicroseconds(1515 - angle/0.06);
}