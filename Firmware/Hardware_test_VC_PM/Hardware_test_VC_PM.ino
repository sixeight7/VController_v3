#include <Print.h>
#include <i2c_t3.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include "LiquidCrystal_MCP23017.h"

// Test below here
#define POWER_PIN 4
#define POWER_SWITCH_PIN 5
#define INTA_PIN 2 // Digital Pin 2 of the Teensy is connected to INTA of the MCP23017 on the display boards
#define NUMBER_OF_DISPLAYS 12

#define BACKLIGHT_PIN     3
#define EN_PIN  2
#define RW_PIN  1
#define RS_PIN  0
#define D4_PIN  4
#define D5_PIN  5
#define D6_PIN  6
#define D7_PIN  7

// Main display:
LiquidCrystal_I2C	Main_lcd(0x27, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN); //First number is the i2c address of the display

// Display boards:
LiquidCrystal_MCP23017 lcd[NUMBER_OF_DISPLAYS] = {
  LiquidCrystal_MCP23017 (0x20, DISPLAY1),
  LiquidCrystal_MCP23017 (0x20, DISPLAY2),
  LiquidCrystal_MCP23017 (0x20, DISPLAY3),
  LiquidCrystal_MCP23017 (0x21, DISPLAY1),
  LiquidCrystal_MCP23017 (0x21, DISPLAY2),
  LiquidCrystal_MCP23017 (0x21, DISPLAY3),
  LiquidCrystal_MCP23017 (0x22, DISPLAY1),
  LiquidCrystal_MCP23017 (0x22, DISPLAY2),
  LiquidCrystal_MCP23017 (0x22, DISPLAY3),
  LiquidCrystal_MCP23017 (0x23, DISPLAY1),
  LiquidCrystal_MCP23017 (0x23, DISPLAY2),
  LiquidCrystal_MCP23017 (0x23, DISPLAY3)
};

uint8_t prev_state = 255;

void setup() {
  // Switch power on
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, HIGH);

 // switch LEDs off:
  setup_LED_control();

  // Setup interrupt pin for switches:
  pinMode(INTA_PIN, INPUT_PULLUP);

  Serial.begin(9600);

  // Initialize and test main LCD
  Main_lcd.begin (16, 2); // Main LCD is 16x2
  Main_lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  Main_lcd.setBacklight(true);
  Main_lcd.home();
  Main_lcd.print("Testing Main LCD");
  Main_lcd.setCursor(0, 1);
  Main_lcd.print("Succes!!!");

  // Initialize and test seperate displays
  for (uint8_t i = 0; i < NUMBER_OF_DISPLAYS; i++) {
    lcd[i].begin (16, 2);
    lcd[i].home();
    lcd[i].print("Testing no." + String(i + 1));
    ///delay(1000);
    lcd[i].setCursor(0, 1);
    lcd[i].print("Succes!!!");
    ///delay(1000);
  }
  
  // Put LEDs on
  show_on_all_LEDs(1);
}

void loop() {
  // Check for switch being pressed
  if (digitalRead(INTA_PIN) == LOW) {
    Serial.println("Interrupt!");

    for (uint8_t i = 0; i < 4; i++) {
      bool updated = lcd[i * 3].updateButtonState();

      if (updated) {
        lcd[i * 3].setCursor(0, 1);
        lcd[i * 3].print("ButtonState:" + String (lcd[i * 3].readButtonState()));
        Serial.println("ButtonState: " + String (lcd[i * 3].readButtonState()));
      }
    }
  }
}

