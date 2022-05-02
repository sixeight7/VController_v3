#include <MIDI.h>
#include "debug.h"

#define VCTOUCH_WL_FIRMWARE_VERSION_MAJOR 1
#define VCTOUCH_WL_FIRMWARE_VERSION_MINOR 0
#define VCTOUCH_WL_FIRMWARE_VERSION_BUILD 0

#define MIDI2_SPEED 500000

#define LED_BUILTIN 2

#define LED_TIME 100
unsigned long led_timer = 0xFFFFFFFF;
bool led_on = false;

// -----------------------------------------------------------------------------
// When BLE connected, LED will turn on (indication that connection was successful)
// When receiving a NoteOn, LED will go out, on NoteOff, light comes back on.
// This is an easy and conveniant way to show that the connection is alive and working.
// -----------------------------------------------------------------------------
void setup()
{
  //Serial.begin(115200);

  setup_debug();
  setup_eeprom();
  setup_MIDI();
  setup_BLE();
  setup_WIFI();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void loop()
{
  run_MIDI();
  run_BLE();
  run_MIDI();
  run_WIFI();
  run_server();
  check_flash_LED();
}

void flash_LED() {
  digitalWrite(LED_BUILTIN, LOW);
  led_timer = millis() + LED_TIME;
  led_on = false;
}

void check_flash_LED() {
  if (led_on) return;
  if (millis() > led_timer) {
    digitalWrite(LED_BUILTIN, HIGH);
    led_on = true;
  }
}
