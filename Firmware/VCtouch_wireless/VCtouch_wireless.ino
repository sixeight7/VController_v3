/****************************************************************************
**
** Copyright (C) 2022 Catrinus Feddema
** All rights reserved.
** This file is part of "VCtouch_wireless" ESP32 software.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License along
** with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
**
** If you use this code in a homebuild project, please support me by sending
** a donation via https://www.paypal.me/sixeight
** If you want to build VControllers, VC-mini's or VC-touches commercially, 
** please contact me on SixEightSoundControl|@gmail.com.
****************************************************************************/

// This page has the following parts:
// Section 1: Library declaration and main setup() and loop()
// Section 2: VCtouch_wireless documentation overview
// Section 3: VCtouch_wireless project log

// Section 1: Library declaration and main setup() and loop()

// Used libraries:
// * MIDI library 5.0.2
// * NIM-BLE Arduino 1.2.0
// * https://github.com/sixeight7/Arduino-AppleMIDI-Library
// * https://github.com/sixeight7/Arduino-BLE-MIDI
// * preferences - should be part of ESP32 installation
// * WIFI - should be part of ESP32 installation


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

// Section 2: VCtouch_wireless documentation overview
// This sketch is used for the ESP32 module in the VC-touch. It hass the following features:
// * BLE midi support - so far only in client mode.
// * RTP or applemidi support - also only in client mode.
// * A 500 kHz MIDI connection to the main Teensy of the VC-touch
// * OTA firmware upgrades
// * Active scanning of SSID's.
// * All settings can be made from the VC-touch wireless menu

// Section 3: VCtouch_wireless project log
// 21-07-2021 Started development of the wireless module 
// 23-07-2021 BLE midi working
// 20-08-2021 Added AppleMIDI/RTP MIDI
// 08-01-2022 Made WIFI programmable from the VC-touch. Added scanning of ssid's
// 29-04-2022 Disabled sysex debugging as it seems to slow down communication
