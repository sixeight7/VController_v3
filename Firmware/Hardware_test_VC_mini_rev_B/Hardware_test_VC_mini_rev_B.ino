#include "EEPROM.h"
#include "LCD.h"
#include "LEDs.h"
#include "MIDI.h"
#include "SWITCH.h"

void setup() {

  // setup hardware
  setup_LED_control();
  setup_LCD_control();
  setup_EEPROM_control();
  setup_switch_control();

  backlight_fade_in();

  // Setup debug procedures.
#define SERIAL_STARTUP_TIME 3000 // Will wait max three seconds max before serial starts
  uint32_t serial_timer;

  Serial.begin(115200);
  serial_timer = millis();
  while ((!Serial) && (serial_timer - millis() < SERIAL_STARTUP_TIME)) {}; // Wait while the serial communication is not ready or while the SERIAL_START_UP time has not elapsed.
  Serial.println("VController MINI debugging started...");
  Serial.println("Arduino version: " + String(ARDUINO));

  // Put LEDs on
  show_red_green_blue();

  // Test external memory chip
  write_ext_EEPROM(10, 77); // Write byte 77 to address 10
  write_ext_EEPROM(67999, 255); // Write byte 255 to address 67999
  if ((read_ext_EEPROM(10) == 77) && (read_ext_EEPROM(67999) == 255)) {
    Main_lcd.setCursor(0, 1);
    Main_lcd.print("24LC512 test OK");
  }
  else {
    Main_lcd.setCursor(0, 1);
    Main_lcd.print("24LC512 ERROR!");
  }

  setup_MIDI_common();
}

void loop() {
  main_switch_control();
  main_MIDI_common();
  LED_blink();
}

