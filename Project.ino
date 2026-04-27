#include <Wire.h>
#include "rgb_lcd.h"
#include <Adafruit_TCS34725.h>
#include <ESP32Servo.h>

#define SENSOR_LED_PIN 16 
#define MOTOR_E1  18  // PWM speed control
#define MOTOR_M1  19  // Direction
// Thresholds
#define GLASS_THRESHOLD     800   // Clear channel drop = glass blocking light
#define MAX_SATURATION      60000
// Transmission mode: longer integration, higher gain for light-through-glass
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Servo ClearServo;  // create a servo object
Servo GreenServo;  // create a servo object
Servo BrownServo;  // create a servo object
Servo BlueServo;  // create a servo object
rgb_lcd lcd;

uint16_t r, g, b, c;
uint16_t baselineC = 5541;  // No-glass reading (full LED light)
uint32_t count = 0;

struct ColorProfile {
  const char* name;
  float rRatio;
  float gRatio;
  float bRatio;
  float tolerance;
};

// UPDATE THESE with your calibration values
const uint8_t NUM_PROFILES = 3;

ColorProfile profiles[] = {
  {"GREEN",  0.3163, 0.3836, 0.3001, 0.06},
  {"BROWN",  0.3440, 0.3753, 0.2808, 0.08},
  {"CLEAR",  0.3031, 0.3781, 0.3188, 0.02},
};




byte Glass[8] = {
    0b01110,  // Top: neck (3 pixels)
    0b01110,  // Neck continues
    0b11111,  // Shoulder (widens)
    0b10001,  // Body left side    
    0b10001,  // Body middle
    0b10001,  // Body middle
    0b10001,  // Body bottom
    0b11111   // Base
};

void setup() {
  pinMode(SENSOR_LED_PIN, OUTPUT);
  digitalWrite(SENSOR_LED_PIN, LOW); // Turn on LED
  pinMode(MOTOR_E1, OUTPUT);
  pinMode(MOTOR_M1, OUTPUT);

  ClearServo.attach(12); 
  GreenServo.attach(13); 
  BrownServo.attach(14); 
  BlueServo.attach(27);

  Serial.begin(115200);
  delay(1000);
  // pinMode(SENSOR_LED_PIN, OUTPUT);
  // digitalWrite(SENSOR_LED_PIN, LOW); // Turn on LED
  
  Wire.begin();
  lcd.begin(16, 2);
  
  // Create custom characters in CGRAM (locations 0-7)
  lcd.createChar(0, Glass);
  
  // Clear display
  lcd.clear();
  lcd.print("Advanced Bottle");
  lcd.setCursor(0, 1);
  lcd.print("Bank");
  lcd.setCursor(15, 1);
  lcd.write(byte(0));
  
  if (!tcs.begin()) {
    Serial.println("TCS34725 not found!");
    while (1) delay(100);
  }
  
  // Calibrate baseline (no glass, full LED light)
  delay(500); // Let LED stabilize
  tcs.getRawData(&r, &g, &b, &baselineC);
  Serial.print("Baseline (no glass): C=");
  Serial.println(baselineC);
  
  Serial.println("\n=== Glass Color Detector - Transmission Mode ===");
  Serial.println("Send 'c' for calibration, 'b' for new baseline\n");
}

void loop() {
  GreenServo.write(150);
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'c' || cmd == 'C') calibrationMode();
    if (cmd == 'b' || cmd == 'B') {
      tcs.getRawData(&r, &g, &b, &baselineC);
      Serial.print("New baseline: C="); Serial.println(baselineC);
    }
  }
  
  if (detectGlass()) {
    String result = classifyColor();
    Serial.print(">>> DETECTED: ");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(result);
    lcd.setCursor(0, 1);
    lcd.print("Count:");
    count++;
    lcd.print(count);
    lcd.setCursor(15, 1);
    lcd.write(byte(0));  // Show Glass
    Serial.print(result);
    Serial.println(" <<<\n");
    
    Serial.println("Remove glass to continue...");
    while (detectGlass()) delay(100);
    delay(300);
  }
  
  delay(50);
}

bool detectGlass() {
  tcs.getRawData(&r, &g, &b, &c);
  
  // In transmission mode: glass BLOCKS light, so c drops below baseline
  if (c > baselineC - GLASS_THRESHOLD) {
    return false; // No glass (light passing through unobstructed)
  }
  
  // // Glass detected - light is being filtered
  // if (r > MAX_SATURATION || g > MAX_SATURATION || b > MAX_SATURATION) {
  //   Serial.println("WARNING: Saturated - reduce LED brightness");
  // }
  
  return true;
}

String classifyColor() {

  // Take multiple samples for stability
  uint32_t rSum = 0, gSum = 0, bSum = 0;
  const int samples = 5;

  for (int i = 0; i < samples; i++) {
    tcs.getRawData(&r, &g, &b, &c);
    rSum += r;
    gSum += g;
    bSum += b;
    delay(20);
  }

  r = rSum / samples;
  g = gSum / samples;
  b = bSum / samples;

  float total = (float)r + g + b;
  if (total < 200) return "UNKNOWN";

  float rRatio = r / total;
  float gRatio = g / total;
  float bRatio = b / total;

  float bestDist = 999;
  int bestIndex = -1;

  // Compare to calibration profiles
  for (int i = 0; i < NUM_PROFILES; i++) {
    float dR = rRatio - profiles[i].rRatio;
    float dG = gRatio - profiles[i].gRatio;
    float dB = bRatio - profiles[i].bRatio;

    float dist = sqrt(dR*dR + dG*dG + dB*dB);

    if (dist < profiles[i].tolerance && dist < bestDist) {
      bestDist = dist;
      bestIndex = i;
    }
  }

  if (bestIndex >= 0) {
    return profiles[bestIndex].name;
  }

  return "UNKNOWN";
}




void calibrationMode() {
  Serial.println("\n=== CALIBRATION ===");
  Serial.println("Place glass between LED and sensor");
  Serial.println("Type: 1=GREEN  2=BLUE  3=BROWN  4=CLEAR  0=skip  q=quit");
  
  while (true) {
    tcs.getRawData(&r, &g, &b, &c);
    // Serial.print("R="); Serial.print(r);
    // Serial.print(" G="); Serial.print(g);
    // Serial.print(" B="); Serial.print(b);
    // Serial.print(" C="); Serial.println(c);
    
    if (Serial.available()) {
      char input = Serial.read();
      if (input == 'q') break;
      if (input >= '1' && input <= '4') {
        int idx = input - '1';
        float total = (float)r + g + b;
        Serial.print("\n>>> ");
        Serial.print(profiles[idx].name);
        Serial.print(" profile: ");
        Serial.print(r/total, 4); Serial.print(", ");
        Serial.print(g/total, 4); Serial.print(", ");
        Serial.println(b/total, 4);
        Serial.println("Update ColorProfile array with these values.\n");
      }
    }
    delay(400);
  }
  Serial.println("Exiting calibration.\n");
}
void motorForward() {
  digitalWrite(MOTOR_M1, HIGH);
  analogWrite(MOTOR_M1, 255);  // 0-255
}

void motorStop() {
  analogWrite(MOTOR_M1, 0);
}