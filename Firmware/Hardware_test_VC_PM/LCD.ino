// Please read VController_v2.ino for information about the license and authors

// Functions for LED control for which I use 12 5mm Neopixel RGB LEDs

#include <Adafruit_NeoPixel.h>
#include <avr/power.h>

// ***************************** Hardware settings *****************************
// Which pin on the Arduino is connected to the NeoPixels LEDs?
// On a Trinket or Gemma we suggest changing this to 1
#define NEOPIXELLEDPIN            17

// HARDWARE settings
#define NUMBER_OF_LEDS      12 // Number of neopixel LEDs connected
uint8_t LED_order[NUMBER_OF_LEDS] = { 0, 3, 6, 9, 1, 4, 7, 10, 2, 5, 8, 11}; //Order in which the LEDs are connected. First LED = 0

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

#define NUMBER_OF_COLOURS 21
colour colours[NUMBER_OF_COLOURS] = {
  {0, 0, 0} ,   // Colour 0 is LED OFF
  {0, 10, 0} ,  // Colour 1 is Green
  {10, 0, 0} ,  //  Colour 2 is Red
  {0, 0, 10} ,  // Colour 3 is Blue
  {10, 5, 0} ,  // Colour 4 is Orange
  {0, 8, 5} ,  // Colour 5 is Turquoise
  {8, 10, 8} ,  // Colour 6 is White
  {8, 8, 0} ,   // Colour 7 is Yellow
  {4, 0, 8} ,   // Colour 8 is Purple
  {10, 0, 5} ,   // Colour 9 is Pink
  {0, 0, 0} ,   // Colour 10 is LED OFF (dimmed)
  {0, 1, 0} ,   // Colour 11 is Green dimmed
  {1, 0, 0} ,   // Colour 12 is Red dimmed
  {0, 0, 1} ,   // Colour 13 is Blue dimmed
  {1, 1, 0} ,   // Colour 14 is Orange dimmed
  {0, 1, 1} ,   // Colour 15 is Turquoise dimmed
  {1, 1, 1} ,   // Colour 16 is White dimmed
  {1, 1, 0} ,   // Colour 17 is Yellow dimmed
  {1, 0, 2} ,   // Colour 18 is Purple dimmed
  {1, 0, 1} ,   // Colour 19 is Pink dimmed
  {0, 0, 0}   // Colour 20 is available
};

#define LEDFLASH_TIMER_LENGTH 500 // Sets the speed with which the LEDs flash (500 means 500 ms on, 500 msec off)
unsigned long LEDflashTimer = 0;
boolean LED_flashing_state_on = true;

#define STARTUP_TIMER_LENGTH 500 // NeoPixel LED switchoff timer set to 100 ms
unsigned long startupTimer = 0;

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel LEDs = Adafruit_NeoPixel(NUMBER_OF_LEDS, NEOPIXELLEDPIN, NEO_GRB + NEO_KHZ800);

boolean update_LEDS = true;
uint8_t global_tap_tempo_LED;
uint8_t prev_page_shown = 255;

void setup_LED_control()
{
  LEDs.begin(); // This initializes the NeoPixel library.

  //Turn the LEDs off repeatedly for 100 ms to reduce startup flash of LEDs
  unsigned int startupTimer = millis();
  while (millis() - startupTimer <= STARTUP_TIMER_LENGTH) {
    LEDs.show();
  }
}

void show_on_all_LEDs(uint8_t colour) {
  for (uint8_t count = 0; count < NUMBER_OF_LEDS; count++) {
    LEDs.setPixelColor(count, LEDs.Color(colours[colour].green, colours[colour].red, colours[colour].blue));
  }
  LEDs.show();
}
