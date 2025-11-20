#include <LiquidCrystal.h>
LiquidCrystal lcd(22, 21, 19, 18, 5, 17);


void setup() {
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Colour: ");
  lcd.setCursor(0, 1);
  lcd.print("Green");
}

void loop() {

}
