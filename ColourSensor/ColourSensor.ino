#define S2 19
#define S3 21
#define sensorOut 22
#define S0 18
#define S1 5
int redFrequency = 0;
int greenFrequency = 0;
int blueFrequency = 0;
void setup() {
 pinMode(S2, OUTPUT);
 pinMode(S3, OUTPUT);
 pinMode(sensorOut, INPUT);
 Serial.begin(115200);
 pinMode(S0, OUTPUT);
 pinMode(S1, OUTPUT);
 digitalWrite(S0, HIGH);
 digitalWrite(S1, LOW);

}
void loop() {
 // Read Red
 digitalWrite(S2, LOW);
 digitalWrite(S3, LOW);
 redFrequency = pulseIn(sensorOut, LOW);
 // Read Green
 digitalWrite(S2, HIGH);
 digitalWrite(S3, HIGH);
 greenFrequency = pulseIn(sensorOut, LOW);
 // Read Blue
 digitalWrite(S2, LOW);
 digitalWrite(S3, HIGH);
 blueFrequency = pulseIn(sensorOut, LOW);
 // Print RGB values
 Serial.print("Red: ");
 Serial.print(redFrequency);
 Serial.print(" Green: ");
 Serial.print(greenFrequency);
 Serial.print(" Blue: ");
 Serial.println(blueFrequency);
 delay(1000); // Wait for a second
}
