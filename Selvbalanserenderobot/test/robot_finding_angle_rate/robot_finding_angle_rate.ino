#include <Adafruit_MPU6050.h>
#include <Wire.h>
Adafruit_MPU6050 mpu;

void setup() {
  Serial.begin(115200);
  mpu.begin();
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
}

void loop() {
  sensors_event_t a, g, t;
  mpu.getEvent(&a, &g, &t);
  Serial.print("angle:");
  Serial.print(atan2(a.acceleration.x, a.acceleration.z) * RAD_TO_DEG);
  Serial.print(",rate:");
  Serial.println(g.gyro.y * RAD_TO_DEG);
  delay(50);
}