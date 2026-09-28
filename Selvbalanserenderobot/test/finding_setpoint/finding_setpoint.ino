#include <Wire.h>
const uint8_t MPU = 0x68;

int16_t rd() { uint8_t h = Wire.read(); uint8_t l = Wire.read(); return (h << 8) | l; }

void setup() {
  Serial.begin(115200);
  Wire.begin(); Wire.setClock(400000);
  Wire.beginTransmission(MPU); Wire.write(0x6B); Wire.write(0x00); Wire.endTransmission();
  Wire.beginTransmission(MPU); Wire.write(0x1A); Wire.write(0x03); Wire.endTransmission();
}

void loop() {
  float sum = 0;
  for (int i = 0; i < 300; i++) {         
    Wire.beginTransmission(MPU); Wire.write(0x3B); Wire.endTransmission(false);
    Wire.requestFrom(MPU, (uint8_t)6);
    int16_t ax = rd(); rd(); int16_t az = rd();
    sum += atan2((float)ax, (float)az) * RAD_TO_DEG;
    delay(10);
  }
  Serial.print("Snitt vinkel: ");
  Serial.println(sum / 300.0, 2);
}