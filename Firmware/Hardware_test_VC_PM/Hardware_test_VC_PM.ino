#include <Print.h>
#include <i2c_t3.h>
//#include <Wire.h>
#include <LCD.h>
#include <Adafruit_NeoPixel.h>
#include <Bounce.h>
#include <EEPROM.h>
#include "hardware.h" // Hardware of production model

#include "lcdlib1.h" // i2c expander board with PCF8754
#include "lcd_lib.h"

#include "LCD.h"
#include "LEDs.h"
#include "switches.h"
#include "eeprom.h"

uint8_t prev_state = 255;

void setup() {
  // Switch power on
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, HIGH);

  // switch LEDs off:
  setup_LED_control();
  LEDs_turn_all_on_white();

  //delay(2000);

  setup_backlight_control();
  Backlight_turn_all_on_white();

  //delay(2000);

  // Start i2c
  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, WIRE_SPEED);
  Wire1.begin(I2C_MASTER, 0x00, I2C_PINS_29_30, I2C_PULLUP_EXT, WIRE1_SPEED);

  setup_LCD_control();
  setup_switch_check();

  Serial.begin(9600);

  // Initialize and test main LCD
  Main_lcd1.home();
  Main_lcd1.print("Main LCD @ 0x27");
  Main_lcd1.setCursor(0, 1);
  Main_lcd1.print("Succes!!!");

  Main_lcd2.home();
  Main_lcd2.print("Main LCD @ 0x3F");
  Main_lcd2.setCursor(0, 1);
  Main_lcd2.print("Succes!!!");


  // Initialize and test seperate displays
  for (uint8_t i = 0; i < NUMBER_OF_DISPLAYS; i++) {
    lcd[i].home();
    lcd[i].print("Testing no." + String(i + 1));
    lcd[i].setCursor(0, 1);
    lcd[i].print("Succes!!!");
  }

  // Put LEDs on
  LEDs_turn_all_on_white();

  // Test external memory chip
  write_ext_EEPROM(10, 77); // Write byte 77 to address 10
  write_ext_EEPROM(67999, 255); // Write byte 255 to address 67999
  if ((read_ext_EEPROM(10) == 77) && (read_ext_EEPROM(67999) == 255)) {
    Main_lcd1.setCursor(0, 1);
    Main_lcd1.print("24LC512 test OK");
    Main_lcd2.setCursor(0, 1);
    Main_lcd2.print("24LC512 test OK");
  }
  else {
    Main_lcd1.setCursor(0, 1);
    Main_lcd1.print("24LC512 ERROR!");
    Main_lcd2.setCursor(0, 1);
    Main_lcd2.print("24LC512 ERROR!");
  }

  // Clear memory
  write_ext_EEPROM(10, 0);
  write_ext_EEPROM(67999, 0);
  EEPROM.write(0, 0); // Will overwrite the internal EEPROM version number, so the VController sketch will rewrite memory
  EEPROM.write(1, 0); // Will overwrite the external EEPROM version number, so the VController sketch will rewrite memory
}

void loop() {
  // Check for switch being pressed
  main_switch_check();
  if (switch_pressed > 0) {
    if (switch_pressed <= NUMBER_OF_LEDS) LEDs_show_green(switch_pressed - 1);
    Main_lcd1.setCursor(0, 1);
    Main_lcd1.print("Pressed  SW #" + String(switch_pressed) + "   ");
    Main_lcd2.setCursor(0, 1);
    Main_lcd2.print("Pressed  SW #" + String(switch_pressed) + "   ");
  }

  if (switch_released > 0) {
    if (switch_released <= NUMBER_OF_LEDS) LEDs_show_blue(switch_released - 1);
    Main_lcd1.setCursor(0, 1);
    Main_lcd1.print("Released SW #" + String(switch_released) + "   ");
    Main_lcd2.setCursor(0, 1);
    Main_lcd2.print("Released SW #" + String(switch_released) + "   ");
  }
}

