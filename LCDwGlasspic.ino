#include <Wire.h>
#include "rgb_lcd.h"

int count = 0;
rgb_lcd lcd;

// Define custom characters (5x8 pixels each)

// Example 1: Speaker icon
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
  // Initialize LCD (16 columns, 2 rows)
  lcd.begin(16, 2);
  
  // Create custom characters in CGRAM (locations 0-7)
  lcd.createChar(0, Glass);
  
  // Clear display
  lcd.clear();
  
  // Display text with custom characters
  lcd.setCursor(0, 0);
  lcd.print("Green");
  lcd.setCursor(0, 1);
  lcd.print("Count:");
  lcd.print(count);
  lcd.setCursor(15, 1);
  lcd.write(byte(0));  // Show speaker
}

void loop() {
  for(int i = 0; i <= 999999999; i++){
  count = i;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Colour: Green");
  lcd.setCursor(0, 1);
  lcd.print("Count:");
  lcd.print(count); // displays count variable
  lcd.setCursor(15, 1); // puts cursor at the last spot on the LCD
  lcd.write(byte(0));  // Show glass picture
  delay(10000);
  }
  
}