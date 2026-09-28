/*
  Self-balancing two-wheel robot
  Arduino Uno + MPU6050 (I2C) + Adafruit TB6612 motor driver

  Tilt angle from accelerometer + gyro (complementary filter).
  Balancing with PID, D-term taken from the gyro rate.

  Power: battery (2x 18650, approx. 7.2-8.4V) -> fuse -> switch -> Arduino VIN
         and TB6612 VM. Arduino's 5V pin feeds TB6612 VCC and MPU6050 VCC.
         The motors run directly off the battery, hence MAX_PWM < 255.
*/

#include <Wire.h>

//Pins 
const uint8_t PWMA = 5, AIN1 = 9, AIN2 = 8, STBY = 10;
const uint8_t BIN1 = 11, BIN2 = 12, PWMB = 6;
const uint8_t MPU = 0x68;

//Tuned parameters
const float Kp = 30.0;
const float Ki = 2.0;
const float Kd = 1.6;
const float SETPOINT = 2.0;   // degrees, balance point (measured with the robot on edge)
const int   MIN_PWM = 40;     // deadband, lowest PWM that gets the wheels rolling
const int   MAX_PWM = 180;    // downscaling volts from battery to match motor voltage
const int   DIR_A = 1, DIR_B = 1;

//Measured, fixed
const float ANGLE_SIGN = 1.0;
const float GYRO_SIGN  = -1.0;
const float ALPHA = 0.98;
const float FALL_ANGLE = 30.0;
const float ACTIVATE_ANGLE = 3.0;
const float DEADZONE = 0.5;

float gyroBias = 0, angle = SETPOINT, integral = 0;
bool active = false;
unsigned long lastUs, lastTx;

int16_t rd() { uint8_t h = Wire.read(); uint8_t l = Wire.read(); return (h << 8) | l; }

void readMPU(int16_t &ax, int16_t &az, int16_t &gy) {
  Wire.beginTransmission(MPU); 
  Wire.write(0x3B); 
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, (uint8_t)14);
  ax = rd(); rd(); az = rd(); rd(); rd(); gy = rd(); rd();
}

void wr(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MPU); Wire.write(reg); Wire.write(val); Wire.endTransmission();
}

void setMotor(uint8_t in1, uint8_t in2, uint8_t pwm, int u) {
  digitalWrite(in1, u >= 0);
  digitalWrite(in2, u < 0);
  analogWrite(pwm, abs(u));
}

void drive(float u) {
  int p = constrain((int)fabs(u), 0, 255);
  if (p > 0) p = MIN_PWM + (long)p * (MAX_PWM - MIN_PWM) / 255;
  int s = (u >= 0) ? p : -p;
  setMotor(AIN1, AIN2, PWMA, s * DIR_A);
  setMotor(BIN1, BIN2, PWMB, s * DIR_B);
}

void setup() {
  Serial.begin(115200);
  pinMode(STBY, OUTPUT);
  pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);
  pinMode(PWMA, OUTPUT); pinMode(PWMB, OUTPUT);
  drive(0);
  digitalWrite(STBY, HIGH);

  Wire.begin(); Wire.setClock(400000);
  wr(0x6B, 0x00);   // wake up
  wr(0x1A, 0x03);   // digital low-pass filter, approx. 42 Hz
  wr(0x1B, 0x08);   // gyro +-500 deg/s
  wr(0x1C, 0x00);   // accelerometer +-2g
  delay(100);

  // Keep the robot still for about 3 seconds here (any position) to calibrate gyro bias.
  long sum = 0; int16_t ax, az, gy;
  for (int i = 0; i < 1000; i++) {
    readMPU(ax, az, gy);
    sum += gy;
    delay(3);
  }
  gyroBias = sum / 1000.0;
  Serial.println(F("Ready."));
  lastUs = micros();
}

void loop() {
  unsigned long now = micros();
  if (now - lastUs < 5000) return;
  float dt = (now - lastUs) * 1e-6; lastUs = now;

  int16_t ax, az, gy; readMPU(ax, az, gy);
  float rate = GYRO_SIGN * (gy - gyroBias) / 65.5;
  float accAngle = ANGLE_SIGN * atan2((float)ax, (float)az) * RAD_TO_DEG;
  angle = ALPHA * (angle + rate * dt) + (1.0 - ALPHA) * accAngle;

  float err = angle - SETPOINT;

  if (fabs(err) > FALL_ANGLE)          active = false;
  else if (fabs(err) < ACTIVATE_ANGLE) active = true;

  float u = 0;
  if (active) {
    integral = constrain(integral + err * dt, -15, 15);
    u = Kp * err + Ki * integral + Kd * rate;
    if (fabs(err) < DEADZONE) u = 0;
    drive(u);
  } else {
    integral = 0;
    drive(0);
  }

  if (millis() - lastTx > 100) {
    lastTx = millis();
    Serial.print(F("err:")); Serial.print(err, 1);
    Serial.print(F(",u:")); Serial.println((int)u);
  }
}