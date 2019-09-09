#ifndef LCD_H
#define LCD_H

#include <LiquidCrystal.h>

#define RS_PIN  3
#define EN_PIN  4
#define D4_PIN  5
#define D5_PIN  6
#define D6_PIN  7
#define D7_PIN  8

// Main display:
LiquidCrystal  Main_lcd(RS_PIN, EN_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN);

void setup_LCD_control() {
  // Initialize and test main LCD
  Main_lcd.begin (16, 2); // Main LCD is 16x2
  Main_lcd.home();
  Main_lcd.print("Testing Main LCD");
  Main_lcd.setCursor(0, 1);
  Main_lcd.print("Succes!!!");
}

void main_LCD_control() {
  
}

#endif
