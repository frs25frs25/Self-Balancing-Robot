const uint8_t PWMA = 5, AIN1 = 9, AIN2 = 8, STBY = 10;
const uint8_t BIN1 = 11, BIN2 = 12, PWMB = 6;

const int DIR_A = 1;      
const int DIR_B = 1;      
const int TEST_PWM = 50;  

void setMotor(uint8_t in1, uint8_t in2, uint8_t pwm, int u) {
  digitalWrite(in1, u >= 0);
  digitalWrite(in2, u < 0);
  analogWrite(pwm, abs(u));
}

void drive(int s) {
  setMotor(AIN1, AIN2, PWMA, s * DIR_A);
  setMotor(BIN1, BIN2, PWMB, s * DIR_B);
}

void setup() {
  pinMode(STBY, OUTPUT);
  pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);
  pinMode(PWMA, OUTPUT); pinMode(PWMB, OUTPUT);
  digitalWrite(STBY, HIGH);
}

void loop() {
  drive(TEST_PWM);   delay(2000);   
  drive(0);          delay(1000);
  drive(-TEST_PWM);  delay(2000);  
  drive(0);          delay(1000);
}