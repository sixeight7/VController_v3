// Please read VController_v2.ino for information about the license and authors

#ifndef LEDS_H
#define LEDS_H

// Functions for LED control for which I use 12 5mm Neopixel RGB LEDs

#include <Adafruit_NeoPixel.h>
#include <avr/power.h>

// ***************************** Hardware settings *****************************
// Which pin on the Arduino is connected to the NeoPixels LEDs?
// On a Trinket or Gemma we suggest changing this to 1
#define NEOPIXELLEDPIN            17
#define NEO2_PIN 2
#define LED_PIN 13

// HARDWARE settings
#define NUMBER_OF_LEDS      3 // Number of neopixel LEDs connected
uint8_t LED_order[NUMBER_OF_LEDS] = { 0, 1, 2}; //Order in which the LEDs are connected. First LED = 0

struct colour {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

// ***************************** Settings you may want to change *****************************

// Which colours are used for which mode?
// pixels.Color takes GRB (Green - Red - Blue) values, from 0,0,0 up to 255,255,255
// The neopixels are very bright. I find setting them to 10 is just fine for an indicator light.

#define GP10_PATCH_COLOUR 4 //Yellow
#define GR55_PATCH_COLOUR 3 //Blue
#define VG99_PATCH_COLOUR 2 //Red
#define ZG3_PATCH_COLOUR 1 //Green
#define ZMS70_PATCH_COLOUR 1 //Green
#define GP10_STOMP_COLOUR 4 //Yellow
#define GR55_STOMP_COLOUR 3 //Blue
#define VG99_STOMP_COLOUR 2 //Red
#define ZG3_STOMP_COLOUR 1 //Green
#define GLOBAL_STOMP_COLOUR 2 //White
#define BPM_COLOUR_ON 2 // Red flasing LED
#define BPM_COLOUR_OFF 0

// Defining FX colours:
#define FX_GTR_COLOUR 6 //White for guitar settings
#define FX_DELAY_COLOUR 1 //Green for delays
#define FX_MODULATE_COLOUR 3 //Blue for modulation FX
#define FX_FILTER_COLOUR 8 //Purple for filter FX
#define FX_PITCH_COLOUR 5 //Turquoise for pitch FX
#define FX_AMP_COLOUR 2 //Red for amp FX and amp solo
#define FX_DIST_COLOUR 9 //Pink for distortion FX
#define FX_REVERB_COLOUR 7 //Yellow for reverb FX

//Lets make some colours (R,G,B)
// Adding 100 to a colour number makes the LED flash!
// To make new colours, set the mode in B_Settings to MODE_COLOUR_MAKER (mode 17)

#define NUMBER_OF_COLOURS 32
colour colours[NUMBER_OF_COLOURS] = {
  {0, 0, 0} ,   // Colour 0 is LED OFF
  {0, 255, 0} ,  // Colour 1 is Green
  {255, 0, 0} ,  //  Colour 2 is Red
  {0, 0, 255} ,  // Colour 3 is Blue
  {255, 128, 0} ,  // Colour 4 is Orange
  {0, 204, 128} ,  // Colour 5 is Cyan
  {204, 255, 204} ,  // Colour 6 is White
  {204, 204, 0} ,   // Colour 7 is Yellow
  {128, 0, 204} ,   // Colour 8 is Purple
  {255, 0, 128} ,   // Colour 9 is Pink
  {92, 255, 114} ,   // Colour 10 is Soft green
  {0, 0, 0} ,   // Colour 11 is spare
  {0, 0, 0} ,   // Colour 12 is spare
  {0, 0, 0} ,   // Colour 13 is spare
  {0, 0, 0} ,   // Colour 14 is spare
  {0, 0, 0} ,   // Colour 15 is spare
  {0, 0, 0} ,   // Colour 16 is LED OFF dimmed
  {51, 0, 0} ,  //  Colour 17 is Red dimmed
  {0, 51, 0} ,  // Colour 18 is Green dimmed
  {0, 0, 51} ,  // Colour 19 is Blue dimmed
  {51, 51, 0} ,  // Colour 20 is Orange dimmed
  {0, 41, 25} ,  // Colour 21 is Cyan dimmed
  {51, 51, 51} ,  // Colour 22 is White dimmed
  {51, 51, 0} ,   // Colour 23 is Yellow dimmed
  {51, 0, 102} ,   // Colour 24 is Purple dimmed
  {51, 0, 51} ,   // Colour 25 is Pink dimmed
  {18, 40, 23} ,   // Colour 26 is Soft green dimmed
  {0, 0, 0} ,   // Colour 27 is spare dimmed
  {0, 0, 0} ,   // Colour 28 is spare dimmed
  {0, 0, 0} ,   // Colour 29 is spare dimmed
  {0, 0, 0} ,   // Colour 30 is spare dimmed
  {0, 0, 0} ,   // Colour 31 is spare dimmed
};

#define LEDFLASH_TIMER_LENGTH 500 // Sets the speed with which the LEDs flash (500 means 500 ms on, 500 msec off)
unsigned long LEDflashTimer = 0;
boolean LED_flashing_state_on = true;

#define STARTUP_TIMER_LENGTH 500 // NeoPixel LED switchoff timer set to 100 ms
unsigned long startupTimer = 0;

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel LEDs = Adafruit_NeoPixel(NUMBER_OF_LEDS, NEOPIXELLEDPIN, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel Backlight = Adafruit_NeoPixel(1, NEO2_PIN, NEO_RGB + NEO_KHZ400);

boolean update_LEDS = true;
uint8_t global_tap_tempo_LED;
uint8_t prev_page_shown = 255;

void setup_LED_control()
{
  LEDs.begin(); // This initializes the NeoPixel library.
  delay(100);
  Backlight.begin();
  Backlight.show();
  pinMode(LED_PIN, OUTPUT);

  //Turn the LEDs off repeatedly for 100 ms to reduce startup flash of LEDs
  unsigned int startupTimer = millis();
  while (millis() - startupTimer <= STARTUP_TIMER_LENGTH) {
    LEDs.show();
  }
  digitalWrite(LED_PIN, HIGH);
  // Set brightness
  LEDs.setBrightness(10);
  delay(100);
  Backlight.setBrightness(255);
}

void show_on_all_LEDs(uint8_t colour) {
  for (uint8_t count = 0; count < NUMBER_OF_LEDS; count++) {
    LEDs.setPixelColor(count, LEDs.Color(colours[colour].green, colours[colour].red, colours[colour].blue));
  }
  LEDs.show();
  delay(100);
  Backlight.setPixelColor(0, Backlight.Color(colours[colour].green, colours[colour].red, colours[colour].blue));
  Backlight.show();
}

void show_red_green_blue() {
  for (uint8_t count = 0; count < NUMBER_OF_LEDS; count++) {
    LEDs.setPixelColor(count, LEDs.Color(colours[count + 1].green, colours[count + 1].red, colours[count + 1].blue));
  }
  LEDs.show();
  delay(100);
  Backlight.setPixelColor(0, 255, 255, 255); // Backlight back to white
  Backlight.show();
}

void backlight_fade_in() {
  for (uint8_t i = 0; i < 255; i++) { // Slowly switch on backlight
    Backlight.setPixelColor(0, i, i, i);
    Backlight.show();
    delay(8);
  }
}

bool led_state = false;
uint32_t timer = 0;
#define BLINK_TIME 600;

void LED_blink() {
  if (millis() > timer) {
    timer = millis() + BLINK_TIME;
    led_state ^= 1;
    digitalWrite(LED_PIN, led_state);
  }
}

#endif
