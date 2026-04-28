// ========== Libraries ==========
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include "rgb_lcd.h"
#include <Adafruit_TCS34725.h>
#include <ESP32Servo.h>
#include "Html.h"

// ========== WiFi Credentials ==========
const char* ssid = "AndroidAP";
const char* password = "EtHaN#@$26";

// ========== Hardware Pins ==========
int SENSOR_LED_PIN = 4;
int MOTOR_E1 = 16;
int MOTOR_M1 = 17;

#define GLASS_THRESHOLD 800

// ========== Objects ==========
// Color sensor object with 50ms integration time and 4x gain
// Integration time = how long the sensor collects light per reading
// Gain = amplifier for low-light conditions
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Servo ClearServo, GreenServo, BrownServo;
rgb_lcd lcd;
WebServer server(80);

// ========== Variables ==========
uint16_t r, g, b, c;
uint16_t baselineC = 5541;

// Counters for web display
uint32_t countClear = 0;
uint32_t countGreen = 0;
uint32_t countBrown = 0;
uint32_t countOther = 0;
String lastDetected = "None";

struct ColorProfile {
  const char* name; // "GREEN", "BROWN", "CLEAR"
  float rRatio, gRatio, bRatio, tolerance; // Expected RGB proportions and tolerance is to check  for matches slightly different to my calibrated numbers
};

const uint8_t NUM_PROFILES = 3;
ColorProfile profiles[] = {
  {"GREEN", 0.3163, 0.3836, 0.3001, 0.06}, // Green bottles: higher green ratio, lower red
  {"BROWN", 0.3440, 0.3753, 0.2808, 0.08}, // Brown/amber bottles: higher red, lower blue
  {"CLEAR", 0.3031, 0.3781, 0.3188, 0.02}, // Clear bottles: most balanced RGB ratios
};

// 5x8 pixel bitmap for a bottle/glass icon
// Used on the LCD display to indicate sorting mode
byte Glass[8] = {
  0b01110,  // Top of bottle (narrow neck)
  0b01110,  // Neck continues
  0b11111,  // Shoulder of bottle
  0b10001,  // Body left side (empty)
  0b10001,  // Body middle
  0b10001,  // Body middle
  0b10001,  // Body right side
  0b11111   // Base of bottle
};


// ========== Web Server Handlers ==========
void handleRoot() {
  server.send(200, "text/html", index_html);
}

void handleData() {
  uint32_t total = countClear + countGreen + countBrown + countOther;
  String json = "{"; //creating a json object
  json += "\"clear\":" + String(countClear) + ",";
  json += "\"green\":" + String(countGreen) + ",";
  json += "\"brown\":" + String(countBrown) + ",";
  json += "\"other\":" + String(countOther) + ",";
  json += "\"total\":" + String(total) + ",";
  json += "\"last\":\"" + lastDetected + "\"";
  json += "}"; //ending the json object
  server.send(200, "application/json", json); //sends object to the webpage
}

// ========== Bottle Detection Functions ==========
bool detectGlass() {
  tcs.getRawData(&r, &g, &b, &c);
  return (c <= baselineC - GLASS_THRESHOLD); // Bottle detected if clear value is much lower than baseline (empty)
}

String classifyColor() {
  uint32_t rSum = 0, gSum = 0, bSum = 0;
  const int samples = 5;

  for (int i = 0; i < samples; i++) {
    tcs.getRawData(&r, &g, &b, &c);
    rSum += r; gSum += g; bSum += b;
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

  // Find closest matching color profile using Euclidean distance
  float bestDist = 999; // Start with large distance
  int bestIndex = -1; // -1 means no match found

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

  return (bestIndex >= 0) ? profiles[bestIndex].name : "UNKNOWN";
}

void processBottle(String color) {
  lastDetected = color;
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(color);
  lcd.setCursor(0, 1);
  lcd.print("Count:");

  if (color == "GREEN") {
    countGreen++;
    lcd.print(countGreen);
    GreenServo.write(15);
    delay(1000);
    GreenServo.write(90);
  } 
  else if (color == "CLEAR") {
    countClear++;
    lcd.print(countClear);
    ClearServo.write(160);
    delay(1000);
    ClearServo.write(90);
  } 
  else if (color == "BROWN") {
    countBrown++;
    lcd.print(countBrown);
    BrownServo.write(30);
    delay(1000);
    BrownServo.write(90);
  } 
  else {
    countOther++;
    lcd.print(countOther);
  }

  lcd.setCursor(15, 1);
  lcd.write(byte(0));
  
  delay(500);
  DisplayLCD();
}

void DisplayLCD() {
  uint32_t total = countClear + countGreen + countBrown + countOther;
  lcd.clear();
  lcd.print("Advanced Bottle");
  lcd.setCursor(0, 1);
  lcd.print("Bank.Count:");
  lcd.print(total);
  lcd.setCursor(15, 1);
  lcd.write(byte(0));
}

// ========== Setup ==========
void setup() {
  Serial.begin(115200);
  
  // Pin setup
  pinMode(SENSOR_LED_PIN, OUTPUT);
  digitalWrite(SENSOR_LED_PIN, LOW);
  pinMode(MOTOR_E1, OUTPUT);
  pinMode(MOTOR_M1, OUTPUT);

  // Servos
  ClearServo.attach(12);
  GreenServo.attach(13);
  BrownServo.attach(14);
  ClearServo.write(90);
  GreenServo.write(90);
  BrownServo.write(90);

  // LCD
  Wire.begin();
  lcd.begin(16, 2);
  lcd.createChar(0, Glass);
  lcd.clear();
  lcd.print("Connecting WiFi");

  // Color sensor
  if (!tcs.begin()) {
    Serial.println("TCS34725 not found!");
    lcd.clear();
    lcd.print("Sensor Error!");
    while (1) delay(100);
  }

  // WiFi connection
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP());

  // Show IP on LCD
  lcd.clear();
  lcd.print("IP Address:");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(3000);

  // Web server routes
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.println("Web server started");

  DisplayLCD();
}

// ========== Loop ==========
void loop() {
  server.handleClient();  // Handle web requests
  
  if (detectGlass()) {
    String result = classifyColor();
    processBottle(result);
    
    // Wait for bottle to pass
    while (detectGlass()) {
      server.handleClient();  // Keep serving during wait
      delay(100);
    }
    delay(300);
  }
  
  delay(50);
}
