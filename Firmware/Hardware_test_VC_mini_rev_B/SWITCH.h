#ifndef SWITCH_H
#define SWITCH_H

#include <Bounce.h>
#include <Encoder.h>

// Test below here
#define SWITCH1_PIN 16
#define SWITCH2_PIN 15
#define SWITCH3_PIN 14

#define ENCODER1_A_PIN 11
#define ENCODER1_B_PIN 12
#define ENCODER1_SWITCH_PIN 24

#define ENCODER2_A_PIN 20
#define ENCODER2_B_PIN 21
#define ENCODER2_SWITCH_PIN 25

Bounce switch1 = Bounce(SWITCH1_PIN, 50);
Bounce switch2 = Bounce(SWITCH2_PIN, 50);
Bounce switch3 = Bounce(SWITCH3_PIN, 50);

Encoder enc1(ENCODER1_B_PIN, ENCODER1_A_PIN);
Bounce enc1_switch = Bounce(ENCODER1_SWITCH_PIN, 50);

Encoder enc2(ENCODER2_B_PIN, ENCODER2_A_PIN);
Bounce enc2_switch = Bounce(ENCODER2_SWITCH_PIN, 50);

uint8_t switch_pressed = 0;
uint8_t switch_released = 0;

uint8_t enc1_value = 0;
uint8_t enc2_value = 0;


void setup_switch_control() {
  // Setup switch pins:
  pinMode(SWITCH1_PIN, INPUT_PULLUP);
  pinMode(SWITCH2_PIN, INPUT_PULLUP);
  pinMode(SWITCH3_PIN, INPUT_PULLUP);

  //pinMode(ENCODER1_A_PIN, INPUT_PULLUP);
  //pinMode(ENCODER1_B_PIN, INPUT_PULLUP);
  pinMode(ENCODER1_SWITCH_PIN, INPUT_PULLUP);

  //pinMode(ENCODER2_A_PIN, INPUT_PULLUP);
  //pinMode(ENCODER2_B_PIN, INPUT_PULLUP);
  pinMode(ENCODER2_SWITCH_PIN, INPUT_PULLUP);

  switch1.update();
  switch2.update();
  switch3.update();

  enc1_switch.update();
  enc2_switch.update();

  enc1.write(0);
  enc2.write(0);
}

void main_switch_control() {
  // Check for switch being pressed
  switch1.update();
  if (switch1.fallingEdge()) { // Check if power switch is pressed
    switch_pressed = 1;
  }
  if (switch1.risingEdge()) { // Check if power switch is released
    switch_released = 1;
  }

  switch2.update();
  if (switch2.fallingEdge()) { // Check if power switch is pressed
    switch_pressed = 2;
  }
  if (switch2.risingEdge()) { // Check if power switch is released
    switch_released = 2;
  }

  switch3.update();
  if (switch3.fallingEdge()) { // Check if power switch is pressed
    switch_pressed = 3;
  }
  if (switch3.risingEdge()) { // Check if power switch is released
    switch_released = 3;
  }

  enc1_switch.update();
  if (enc1_switch.fallingEdge()) { // Check if power switch is pressed
    Main_lcd.setCursor(0, 1);
    Main_lcd.print("Pressed enc. 1  ");
  }
  if (enc1_switch.risingEdge()) { // Check if power switch is released
    Main_lcd.setCursor(0, 1);
    Main_lcd.print("Released enc. 1 ");
  }

  enc2_switch.update();
  if (enc2_switch.fallingEdge()) { // Check if power switch is pressed
    Main_lcd.setCursor(0, 1);
    Main_lcd.print("Pressed enc. 2  ");
  }
  if (enc2_switch.risingEdge()) { // Check if power switch is released
    Main_lcd.setCursor(0, 1);
    Main_lcd.print("Released enc. 2 ");
  }

  if (switch_pressed > 0) {
    Main_lcd.setCursor(0, 1);
    Main_lcd.print("Pressed switch " + String(switch_pressed));
    show_on_all_LEDs(switch_pressed); // Show this colour on all LEDs
    switch_pressed = 0;
  }

  if (switch_released > 0) {
    Main_lcd.setCursor(0, 1);
    Main_lcd.print("Rleased switch " + String(switch_released));
    show_red_green_blue();
    switch_released = 0;
  }

  uint8_t new_reading = enc1.read() / 2;
  if (new_reading != enc1_value) {
    enc1_value = new_reading;
    Main_lcd.setCursor(0, 1);
    Main_lcd.print("Enc1 value: " + String(enc1_value) + "  ");
  }

  new_reading = enc2.read() / 2;
  if (new_reading != enc2_value) {
    enc2_value = new_reading;
    Main_lcd.setCursor(0, 1);
    Main_lcd.print("Enc2 value: " + String(enc2_value) + "  ");
  }
}

#endif


