#ifndef LCD_H
#define LCD_H

#include <i2c_t3.h>
#include <LCD.h>
//#include <LiquidCrystal_I2C.h>
#include "lcdlib1.h" // i2c expander board with PCF8754
#include "lcd_lib.h" // For the VController display boards 

// ***************************** Hardware settings *****************************
// Moved to hardware.h

// Main displays:
LiquidCrystal_PCF8745	Main_lcd1(DISPLAY_MAIN_ADDRESS1, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN); //First number is the i2c address of the display
LiquidCrystal_PCF8745	Main_lcd2(DISPLAY_MAIN_ADDRESS2, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN); //First number is the i2c address of the display

// Displays above the switches

// Method 1: declare displays on VController display boards:
#ifdef DISPLAY_01_ADDRESS_AND_NUMBER
LiquidCrystal_MCP23017 lcd[NUMBER_OF_DISPLAYS] = {
  LiquidCrystal_MCP23017 (DISPLAY_01_ADDRESS_AND_NUMBER),
#endif
#ifdef DISPLAY_02_ADDRESS_AND_NUMBER
  LiquidCrystal_MCP23017 (DISPLAY_02_ADDRESS_AND_NUMBER),
#endif
#ifdef DISPLAY_03_ADDRESS_AND_NUMBER
  LiquidCrystal_MCP23017 (DISPLAY_03_ADDRESS_AND_NUMBER),
#endif
#ifdef DISPLAY_04_ADDRESS_AND_NUMBER
  LiquidCrystal_MCP23017 (DISPLAY_04_ADDRESS_AND_NUMBER),
#endif
#ifdef DISPLAY_05_ADDRESS_AND_NUMBER
  LiquidCrystal_MCP23017 (DISPLAY_05_ADDRESS_AND_NUMBER),
#endif
#ifdef DISPLAY_06_ADDRESS_AND_NUMBER
  LiquidCrystal_MCP23017 (DISPLAY_06_ADDRESS_AND_NUMBER),
#endif
#ifdef DISPLAY_07_ADDRESS_AND_NUMBER
  LiquidCrystal_MCP23017 (DISPLAY_07_ADDRESS_AND_NUMBER),
#endif
#ifdef DISPLAY_08_ADDRESS_AND_NUMBER
  LiquidCrystal_MCP23017 (DISPLAY_08_ADDRESS_AND_NUMBER),
#endif
#ifdef DISPLAY_09_ADDRESS_AND_NUMBER
  LiquidCrystal_MCP23017 (DISPLAY_09_ADDRESS_AND_NUMBER),
#endif
#ifdef DISPLAY_10_ADDRESS_AND_NUMBER
  LiquidCrystal_MCP23017 (DISPLAY_10_ADDRESS_AND_NUMBER),
#endif
#ifdef DISPLAY_11_ADDRESS_AND_NUMBER
  LiquidCrystal_MCP23017 (DISPLAY_11_ADDRESS_AND_NUMBER),
#endif
#ifdef DISPLAY_12_ADDRESS_AND_NUMBER
  LiquidCrystal_MCP23017 (DISPLAY_12_ADDRESS_AND_NUMBER),
#endif
#ifdef DISPLAY_13_ADDRESS_AND_NUMBER
  LiquidCrystal_MCP23017 (DISPLAY_13_ADDRESS_AND_NUMBER),
#endif
#ifdef DISPLAY_14_ADDRESS_AND_NUMBER
  LiquidCrystal_MCP23017 (DISPLAY_14_ADDRESS_AND_NUMBER),
#endif
#ifdef DISPLAY_15_ADDRESS_AND_NUMBER
  LiquidCrystal_MCP23017 (DISPLAY_15_ADDRESS_AND_NUMBER),
#endif
#ifdef DISPLAY_01_ADDRESS_AND_NUMBER
};
#endif

// Method 2: devclare displays using regular i2C backpacks if defined in hardware.h
#ifdef DISPLAY_01_ADDRESS
LiquidCrystal_PCF8745 lcd[NUMBER_OF_DISPLAYS] = {
  LiquidCrystal_PCF8745 (DISPLAY_01_ADDRESS, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN),
#endif
#ifdef DISPLAY_02_ADDRESS
  LiquidCrystal_PCF8745 (DISPLAY_02_ADDRESS, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN),
#endif
#ifdef DISPLAY_03_ADDRESS
  LiquidCrystal_PCF8745 (DISPLAY_03_ADDRESS, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN),
#endif
#ifdef DISPLAY_04_ADDRESS
  LiquidCrystal_PCF8745 (DISPLAY_04_ADDRESS, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN),
#endif
#ifdef DISPLAY_05_ADDRESS
  LiquidCrystal_PCF8745 (DISPLAY_05_ADDRESS, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN),
#endif
#ifdef DISPLAY_06_ADDRESS
  LiquidCrystal_PCF8745 (DISPLAY_06_ADDRESS, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN),
#endif
#ifdef DISPLAY_07_ADDRESS
  LiquidCrystal_PCF8745 (DISPLAY_07_ADDRESS, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN),
#endif
#ifdef DISPLAY_08_ADDRESS
  LiquidCrystal_PCF8745 (DISPLAY_08_ADDRESS, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN),
#endif
#ifdef DISPLAY_09_ADDRESS
  LiquidCrystal_PCF8745 (DISPLAY_09_ADDRESS, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN),
#endif
#ifdef DISPLAY_10_ADDRESS
  LiquidCrystal_PCF8745 (DISPLAY_10_ADDRESS, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN),
#endif
#ifdef DISPLAY_11_ADDRESS
  LiquidCrystal_PCF8745 (DISPLAY_11_ADDRESS, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN),
#endif
#ifdef DISPLAY_12_ADDRESS
  LiquidCrystal_PCF8745 (DISPLAY_12_ADDRESS, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN),
#endif
#ifdef DISPLAY_13_ADDRESS
  LiquidCrystal_PCF8745 (DISPLAY_13_ADDRESS, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN),
#endif
#ifdef DISPLAY_14_ADDRESS
  LiquidCrystal_PCF8745 (DISPLAY_14_ADDRESS, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN),
#endif
#ifdef DISPLAY_15_ADDRESS
  LiquidCrystal_PCF8745 (DISPLAY_15_ADDRESS, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN),
#endif
#ifdef DISPLAY_01_ADDRESS
};
#endif

#define MESSAGE_TIMER_LENGTH 1500 // time that status messages are shown (in msec)
unsigned long messageTimer = 0;
bool status_message_showing = false;

void setup_LCD_control()
{
  // Set i2C speed

  //Wire.begin();

  // Initialize main LCD
  Main_lcd1.begin (16, 2); // Main LCD is 16x2
  Main_lcd1.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  Main_lcd1.setBacklight(true); // Backlight state is the same as initial on or off state...
  Main_lcd2.begin (16, 2); // Main LCD is 16x2
  Main_lcd2.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  Main_lcd2.setBacklight(true); // Backlight state is the same as initial on or off state...

  // Initialize individual LCDs - same for both methods
  for (uint8_t i = 0; i < NUMBER_OF_DISPLAYS; i++) {
    lcd[i].begin (16, 2);
    lcd[i].setBacklightPin(BACKLIGHT_PIN, POSITIVE);
    lcd[i].setBacklight(true); // Backlight state is the same as initial on or off state...
  }
}

#endif
