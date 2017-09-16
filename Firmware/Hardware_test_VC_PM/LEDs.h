#ifndef LEDS_H
#define LEDS_H

#include <Adafruit_NeoPixel.h>
#include <avr/power.h>

// ***************************** Hardware settings *****************************

uint8_t LED_order[NUMBER_OF_LEDS] = {LED_ORDER};

Adafruit_NeoPixel LEDs = Adafruit_NeoPixel(NUMBER_OF_LEDS, NEOPIXELLEDPIN, NEO_RGB + NEO_KHZ800);

#ifdef BACKLIGHTNEOPIXELPIN
Adafruit_NeoPixel Backlights = Adafruit_NeoPixel(NUMBER_OF_BACKLIGHTS, BACKLIGHTNEOPIXELPIN, NEO_RGB + NEO_KHZ400);
#endif

#define STARTUP_TIMER_LENGTH 500 // NeoPixel LED switchoff timer set to 100 ms
unsigned long startupTimer = 0;

void setup_LED_control()
{
  LEDs.begin(); // This initializes the NeoPixel library.

  //Turn the LEDs off repeatedly for 100 ms to reduce startup flash of LEDs
  //When neopixel LEDs are powered on, they burn at 100%.
  unsigned int startupTimer = millis();
  while (millis() - startupTimer <= STARTUP_TIMER_LENGTH) {
    LEDs.show();
  }
}

void setup_backlight_control() {
  // Startup backlights
#ifdef BACKLIGHTNEOPIXELPIN
  Backlights.begin();
  Backlights.show();
#endif
}

void LEDs_turn_all_on_red() {
  for (uint8_t l = 0; l < NUMBER_OF_LEDS; l++) {
    LEDs.setPixelColor(l, LEDs.Color(10, 0, 0));
#ifdef BACKLIGHTNEOPIXELPIN
    Backlights.setPixelColor(l, LEDs.Color(255, 0, 0));
#endif
  }
  LEDs.show();
#ifdef BACKLIGHTNEOPIXELPIN
  Backlights.show();
#endif
}

void LEDs_turn_all_on_white() {
  for (uint8_t l = 0; l < NUMBER_OF_LEDS; l++) {
    LEDs.setPixelColor(l, LEDs.Color(10, 10, 10));
    LEDs.show();
  }
  LEDs.show();

}

void Backlight_turn_all_on_white() {
#ifdef BACKLIGHTNEOPIXELPIN
  for (uint8_t l = 0; l < NUMBER_OF_BACKLIGHTS; l++) {
    Backlights.setPixelColor(l, LEDs.Color(255, 255, 255));
  }
  Backlights.show();
#endif
}

void LEDs_show_green(uint8_t number) {
  LEDs.setPixelColor(LED_order[number], LEDs.Color(0, 10, 0));
  LEDs.show();
#ifdef BACKLIGHTNEOPIXELPIN
  Backlights.setPixelColor(LED_order[number], LEDs.Color(0, 255, 0));
  Backlights.show();
#endif

}

void LEDs_show_blue(uint8_t number) {
  LEDs.setPixelColor(LED_order[number], LEDs.Color(0, 0, 10));
  LEDs.show();
#ifdef BACKLIGHTNEOPIXELPIN
  Backlights.setPixelColor(LED_order[number], LEDs.Color(0, 0, 255));
  Backlights.show();
#endif
}

#endif
