int MOTOR_E1 = 16;
int MOTOR_M1 = 17;

void setup() {
  pinMode(MOTOR_E1, OUTPUT);
  pinMode(MOTOR_M1, OUTPUT);
  //Serial.begin(115200);
}

void loop() {
  //Serial.println("Motor ON forward");
  analogWrite(MOTOR_E1, 200);
  digitalWrite(MOTOR_M1, LOW);
}