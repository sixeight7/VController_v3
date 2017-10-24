// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: LED Hardware Initialization
// Section 2: LED Update Code
// Section 3: LED Functions

// Functions for LED control for which I use 12 5mm Neopixel RGB LEDs
// LEDs need three colour numbers (RGB), so we define colours below, to be able to store it in one byte.

// ********************************* Section 1: LED Hardware Initialization ********************************************

#include <Adafruit_NeoPixel.h>
#include <avr/power.h>

// ***************************** Hardware settings *****************************
// Hardware settings have been moved to hardware.h.

struct colour {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

#define LED_FLASHING 0x80 // Enabling bit 8 in a colour number will make the LEDs flash
#define LED_DIMMED 0x10 // Enabling bit 5 in a colour number will dim the LED.
#define LED_MASK 0x7F // The colour stripped of flashingbit

// Defining FX colours types - the colours are set in the menu:
#define FX_TYPE_OFF 0
#define FX_DEFAULT_TYPE 1 // For parameters that fit no other category
#define FX_GTR_TYPE 2 // For COSM/guitar settings
#define FX_PITCH_TYPE 3 // For pitch FX
#define FX_FILTER_TYPE 4 // For filter FX
#define FX_DIST_TYPE 5 // For distortion FX
#define FX_AMP_TYPE 6 // For amp FX and amp solo
#define FX_MODULATE_TYPE 7 //B For modulation FX
#define FX_DELAY_TYPE 8 // For delays
#define FX_REVERB_TYPE 9// For reverb FX

//Lets make some colours (R,G,B)

#define CURRENT_DEVICE_COLOUR 255 // Special variable to get the LED to show the colour of the current device

#define NUMBER_OF_COLOURS 32
#define NUMBER_OF_SELECTABLE_COLOURS 10
colour colours[NUMBER_OF_COLOURS] = {
  {0, 0, 0} ,   // Colour 0 is LED OFF
  {0, 255, 0} ,  // Colour 1 is Green
  {255, 0, 0} ,  //  Colour 2 is Red
  {0, 0, 255} ,  // Colour 3 is Blue
  {255, 128, 0} ,  // Colour 4 is Orange
  {0, 204, 128} ,  // Colour 5 is Turquoise
  {204, 255, 204} ,  // Colour 6 is White
  {204, 204, 0} ,   // Colour 7 is Yellow
  {128, 0, 204} ,   // Colour 8 is Purple
  {255, 0, 128} ,   // Colour 9 is Pink
  {0, 0, 0} ,   // Colour 10 is spare
  {0, 0, 0} ,   // Colour 11 is spare
  {0, 0, 0} ,   // Colour 12 is spare
  {0, 0, 0} ,   // Colour 13 is spare
  {0, 0, 0} ,   // Colour 14 is spare
  {0, 0, 0} ,   // Colour 15 is spare
  {0, 0, 0} ,   // Colour 0 is LED OFF dimmed
  {0, 51, 0} ,  // Colour 1 is Green dimmed
  {51, 0, 0} ,  //  Colour 2 is Red dimmed
  {0, 0, 51} ,  // Colour 3 is Blue dimmed
  {51, 51, 0} ,  // Colour 4 is Orange dimmed
  {0, 41, 25} ,  // Colour 5 is Turquoise dimmed
  {51, 51, 51} ,  // Colour 6 is White dimmed
  {51, 51, 0} ,   // Colour 7 is Yellow dimmed
  {51, 0, 102} ,   // Colour 8 is Purple dimmed
  {51, 0, 51} ,   // Colour 9 is Pink dimmed
  {0, 0, 0} ,   // Colour 10 is spare dimmed
  {0, 0, 0} ,   // Colour 11 is spare dimmed
  {0, 0, 0} ,   // Colour 12 is spare dimmed
  {0, 0, 0} ,   // Colour 13 is spare dimmed
  {0, 0, 0} ,   // Colour 14 is spare dimmed
  {0, 0, 0} ,   // Colour 15 is spare dimmed
};

colour Backlight_colours[NUMBER_OF_COLOURS] = {
  {0, 0, 0} ,   // Colour 0 is LED OFF
  {0, 255, 0} ,  // Colour 1 is Green
  {255, 50, 50} ,  //  Colour 2 is Red
  {50, 50, 255} ,  // Colour 3 is Blue
  {255, 178, 50} ,  // Colour 4 is Orange
  {0, 204, 128} ,  // Colour 5 is Turquoise
  {204, 255, 204} ,  // Colour 6 is White
  {204, 204, 0} ,   // Colour 7 is Yellow
  {128, 50, 204} ,   // Colour 8 is Purple
  {255, 50, 128} ,   // Colour 9 is Pink
  {0, 0, 0} ,   // Colour 10 is spare
  {0, 0, 0} ,   // Colour 11 is spare
  {0, 0, 0} ,   // Colour 12 is spare
  {0, 0, 0} ,   // Colour 13 is spare
  {0, 0, 0} ,   // Colour 14 is spare
  {0, 0, 0} ,   // Colour 15 is spare
};

#define LEDFLASH_TIMER_LENGTH 500 // Sets the speed with which the LEDs flash (500 means 500 ms on, 500 msec off)
unsigned long LEDflashTimer = 0;
boolean LED_flashing_state_on = true;

#define STARTUP_TIMER_LENGTH 100 // NeoPixel LED switchoff timer set to 100 ms
unsigned long startupTimer = 0;

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel LEDs = Adafruit_NeoPixel(NUMBER_OF_LEDS, NEOPIXELLEDPIN, NEO_RGB + NEO_KHZ800);
uint8_t LED_order[NUMBER_OF_LEDS] = {LED_ORDER};

#ifdef BACKLIGHTNEOPIXELPIN
Adafruit_NeoPixel Backlights = Adafruit_NeoPixel(NUMBER_OF_BACKLIGHTS, BACKLIGHTNEOPIXELPIN, NEO_RGB + NEO_KHZ400);
uint8_t Backlight_order[NUMBER_OF_BACKLIGHTS] = {BACKLIGHT_ORDER};
#endif

boolean update_LEDS = true;
uint8_t global_tap_tempo_LED;
uint8_t prev_page_shown = 255;

void setup_LED_control()
{
  LEDs.begin(); // This initializes the NeoPixel library.

  //Turn the LEDs off repeatedly for 100 ms to reduce startup flash of LEDs
  //When neopixel LEDs are powered on, they burn at 100%.
  unsigned int startupTimer = millis();
  while (millis() - startupTimer <= STARTUP_TIMER_LENGTH) {
    LEDs.show();
  }
  LEDflashTimer = millis(); // Set the timer

  // Startup backlights
#ifdef BACKLIGHTNEOPIXELPIN
  Backlights.begin();
  //Backlights.setPixelColor(0,255,255,255);
  Backlights.show();
#endif
}

void main_LED_control()
{
  if (update_LEDS) {
    update_LEDS = false;
    LED_update();
  }

  // Check here if LEDs need to flash and make them do it
  LED_flash();
}

// ********************************* Section 2: LED Update Code ********************************************

void LED_update() {
  // Set brightness
  LEDs.setBrightness(Setting.LED_brightness);

#ifdef BACKLIGHTNEOPIXELPIN
  // Set brightness backlights
  Backlights.setBrightness(Setting.Backlight_brightness);
#endif

  //Check the switch_states on the current page
  for (uint8_t s = 0; s < NUMBER_OF_LEDS; s++) {

    //Copy the LED state from the switch state
    uint8_t sw = s + 1;
    uint8_t Dev = SP[sw].Device;
    uint8_t colour;
    if (Dev == CURRENT) Dev = Current_device;

    if (Dev < NUMBER_OF_DEVICES) {
      switch (SP[sw].Type) {
        case PATCH_SEL:
          if (Device[Dev]->patch_number == SP[sw].PP_number) {
            if (Device[Dev]->is_on) LED_show_colour(s, SP[sw].Colour);
            else LED_show_colour(s, SP[sw].Colour | LED_DIMMED); // Show off colour
          }
          else LED_show_colour(s, 0);
          Backlight_show_colour(s, SP[sw].Colour);
          break;
        case PATCH_BANK:
          if (Device[Dev]->bank_selection_active()) LED_show_colour(s, SP[sw].Colour | LED_FLASHING); //Flash the devices PATCH LEDs
          else {
            if (Device[Dev]->patch_number == SP[sw].PP_number) {
              if (Device[Dev]->is_on) {
                LED_show_colour(s, SP[sw].Colour);
              }
              else LED_show_colour(s, SP[sw].Colour | LED_DIMMED); // Show off colour
            }
            else {
              LED_show_colour(s, 0);
            }
          }
          Backlight_show_colour(s, SP[sw].Colour);
          break;
        case BANK_UP:
        case BANK_DOWN:
        case PAR_BANK_UP:
        case PAR_BANK_DOWN:
        //case ASG_BANK_UP:
        //case ASG_BANK_DOWN:
        case PREV_PATCH:
        case NEXT_PATCH:
          if (SP[sw].Colour == CURRENT_DEVICE_COLOUR) colour = Device[Current_device]->my_LED_colour;
          else colour = SP[sw].Colour;
          if (SP[sw].Pressed) LED_show_colour(s, colour);
          else LED_show_colour(s, colour | LED_DIMMED);
          Backlight_show_colour(s, colour);
          break;
        case DIRECT_SELECT:
          LED_show_colour(s, SP[sw].Colour | LED_FLASHING); //Flash the devices PATCH LEDs
          Backlight_show_colour(s, SP[sw].Colour);
          break;
        case PARAMETER:
        case PAR_BANK:
        case ASSIGN:
          if ((SP[sw].Latch == MOMENTARY) || (SP[sw].Latch == TOGGLE)) {
            //DEBUGMSG("State pedal " + String(s) + ": " + String(SP[sw].State));
            if (SP[sw].State == 1) LED_show_colour(s, LED_FX_type_colour(SP[sw].Colour));  // LED on
            if (SP[sw].State == 2) LED_show_dimmed(s, LED_FX_type_colour(SP[sw].Colour)); // LED dimmed
            if (SP[sw].State == 0) LED_show_colour(s, 0); // LED off
          }
          else { // For the TRI/FOUR/STEP/RANGE/UPDOWN only light up when pressed.
            if (SP[sw].Pressed) LED_show_colour(s, LED_FX_type_colour(SP[sw].Colour));
            else LED_show_dimmed(s, LED_FX_type_colour(SP[sw].Colour));
          }
          Backlight_show_colour(s, LED_FX_type_colour(SP[sw].Colour));
          break;
        case OPEN_PAGE_DEVICE:
        case OPEN_PAGE_PATCH:
        case OPEN_PAGE_PARAMETER:
        case OPEN_PAGE_ASSIGN:
          if ((SP[sw].Pressed) || ((SP[sw].PP_number == prev_page_shown) && (SP[sw].Device == Current_device))) LED_show_colour(s, Setting.LED_global_colour);
          else LED_show_colour(s, 0);
          Backlight_show_colour(s, Setting.LED_global_colour);
          break;
        case MUTE:
          if (Device[Dev]->is_on) LED_show_colour(s, 0);
          else LED_show_colour(s, SP[sw].Colour);
          Backlight_show_colour(s, SP[sw].Colour);
          break;
        default:
          LED_show_colour(s, 0); // Show nothing with undefined LED
          Backlight_show_colour(s, 0);
      }
    }
    if (Dev == COMMON) {
      switch (SP[sw].Type) {
        case TAP_TEMPO:
          LED_show_colour(s, global_tap_tempo_LED); // The state of the tap tempo LED is controlled from SCO_update_tap_tempo_LED()
          Backlight_show_colour(s, Setting.LED_bpm_colour);
          break;
        case SET_TEMPO:
          if (SP[sw].Pressed) LED_show_colour(s, Setting.LED_bpm_colour);
          else LED_show_colour(s, 0);
          Backlight_show_colour(s, Setting.LED_bpm_colour);
          break;
        case MIDI_PC:
          if (SP[sw].PP_number == MIDI_recall_PC(SP[sw].Value1, SP[sw].Value2)) LED_show_colour(s, SP[sw].Colour); // Value1 stores MIDI channel, value2 stores MIDI port
          else LED_show_colour(s, 0);
          Backlight_show_colour(s, SP[sw].Colour);
          break;
        case MIDI_NOTE:
          if (SP[sw].Pressed) LED_show_colour(s, SP[sw].Colour);
          else LED_show_colour(s, 0);
          Backlight_show_colour(s, SP[sw].Colour);
          break;
        case MIDI_CC:
          //DEBUGMAIN("CC latch:" + String(SP[sw].Latch));
          if ((SP[sw].Latch == CC_TOGGLE) || (SP[sw].Latch == CC_TOGGLE_ON)) {
            //DEBUGMAIN("State pedal " + String(s) + ": " + String(SP[sw].State));
            if (SP[sw].State == 0) LED_show_colour(s, SP[sw].Colour);  // LED on
            if (SP[sw].State == 1) LED_show_dimmed(s, SP[sw].Colour); // LED off
          }
          else { // For the ONE_SHOT/MOMENTARY/RANGE/UPDOWN only light up when pressed.
            if (SP[sw].Pressed) LED_show_colour(s, SP[sw].Colour);
            else LED_show_dimmed(s, SP[sw].Colour);
          }
          Backlight_show_colour(s, SP[sw].Colour);
          break;
        case OPEN_PAGE:
        case MENU:
        case SELECT_NEXT_DEVICE:
          if ((SP[sw].Pressed) || (SP[sw].PP_number == prev_page_shown)) LED_show_colour(s, SP[sw].Colour);
          else LED_show_colour(s, 0);
          Backlight_show_colour(s, SP[sw].Colour);
          break;
        case GLOBAL_TUNER:
          if (global_tuner_active) LED_show_colour(s, Setting.LED_global_colour);
          else LED_show_colour(s, 0);
          Backlight_show_colour(s, Setting.LED_global_colour);
          break;
        default:
          LED_show_colour(s, 0); // Show nothing with undefined LED
          Backlight_show_colour(s, 0);
      }
    }
  }
  LEDs.show();
#ifdef BACKLIGHTNEOPIXELPIN
  Backlights.show();
#endif
  //DEBUGMSG("LEDs updated");
}

void LED_show_dimmed(uint8_t no, uint8_t colour) {
  if (Setting.LED_FX_off_is_dimmed) LED_show_colour(no, colour | LED_DIMMED);
  else LED_show_colour(no, 0);
}

uint8_t LED_FX_type_colour(uint8_t type) {
  switch (type) {
    case FX_DEFAULT_TYPE: return Setting.FX_default_colour;
    case FX_GTR_TYPE: return Setting.FX_GTR_colour;
    case FX_PITCH_TYPE: return Setting.FX_PITCH_colour;
    case FX_FILTER_TYPE: return Setting.FX_FILTER_colour;
    case FX_DIST_TYPE: return Setting.FX_DIST_colour;
    case FX_AMP_TYPE: return Setting.FX_AMP_colour;
    case FX_MODULATE_TYPE: return Setting.FX_MODULATION_colour;
    case FX_DELAY_TYPE: return Setting.FX_DELAY_colour;
    case FX_REVERB_TYPE: return Setting.FX_REVERB_colour;
    default: return 0;
  }
}

// ********************************* Section 3: LED Functions ********************************************

void LED_show_colour(uint8_t LED_number, uint8_t colour_number) { // Sets the specified LED to the specified colour

  uint8_t number_fixed = colour_number & LED_MASK;
  if (number_fixed < NUMBER_OF_COLOURS) {
    if (colour_number & LED_FLASHING) { // Check if it is not a flashing LED
      // Update flashing LED
      if (LED_flashing_state_on == true) {
        // Turn the LED on
        if (Setting.Physical_LEDs) LEDs.setPixelColor(LED_order[LED_number], LEDs.Color(colours[number_fixed].red, colours[number_fixed].green, colours[number_fixed].blue));
        if (Setting.Virtual_LEDs) Set_virtual_LED_colour(LED_number, number_fixed); // Update the virtual LEDs on the LCD as well
      }
      else {
        // Turn the LED off
        if (Setting.Physical_LEDs) LEDs.setPixelColor(LED_order[LED_number], 0, 0, 0);
        if (Setting.Virtual_LEDs) Set_virtual_LED_colour(LED_number, 0); // Update the virtual LEDs on the LCD as well
      }
    }
    else {
      // Turn the LED on
      if (Setting.Physical_LEDs) LEDs.setPixelColor(LED_order[LED_number], LEDs.Color(colours[number_fixed].red, colours[number_fixed].green, colours[number_fixed].blue));
      if (Setting.Virtual_LEDs) Set_virtual_LED_colour(LED_number, number_fixed); // Update the virtual LEDs on the LCD as well
    }
  }
  else {
    // Invalid colour: show message and give error
    LEDs.setPixelColor(LED_order[LED_number], LEDs.Color(colours[10].red, colours[10].green, colours[10].blue));
    DEBUGMSG("Invalid colour (number " + String(number_fixed) + ") on LED " + String(LED_number));
  }

}

void Backlight_show_colour(uint8_t LED_number, uint8_t colour_number) { // Sets the specified LED to the specified colour
#ifdef BACKLIGHTNEOPIXELPIN
  if ((colour_number < NUMBER_OF_COLOURS) && (LED_number < NUMBER_OF_BACKLIGHTS)) {
    //DEBUGMSG("Backlight #" + String(LED_number) + " gets colour #" + String(colour_number));
    Backlights.setPixelColor(Backlight_order[LED_number], LEDs.Color(Backlight_colours[colour_number].red, Backlight_colours[colour_number].green, Backlight_colours[colour_number].blue));
  }
#else
  // try to switch backlights
  if (colour_number == 0) LCD_switch_off_backlight(LED_number);
  else LCD_switch_on_backlight(LED_number);
#endif
}

void LED_turn_all_off() {
  for (uint8_t l = 0; l < NUMBER_OF_LEDS; l++) {
    LEDs.setPixelColor(l, LEDs.Color(0, 0, 0));
  }
  LEDs.show();
#ifdef BACKLIGHTNEOPIXELPIN
  for (uint8_t l = 0; l < NUMBER_OF_BACKLIGHTS; l++) {
    Backlights.setPixelColor(l, LEDs.Color(0, 0, 0));
  }
  Backlights.show();
#endif
}

void LED_show_middle_four() { // To indicate EEPROM memory being written.
  LED_show_colour(5, Setting.LED_global_colour);
  LED_show_colour(6, Setting.LED_global_colour);
  LED_show_colour(9, Setting.LED_global_colour);
  LED_show_colour(10, Setting.LED_global_colour);
  LEDs.show();
}

void LED_flash() {
  // Check if timer runs out
  if (millis() - LEDflashTimer > LEDFLASH_TIMER_LENGTH) {
    LEDflashTimer = millis(); // Reset the timer
    LED_flashing_state_on = !LED_flashing_state_on;
    update_LEDS = true; // Get the LEDs to update
  }
}

void LED_update_pressed_state_only() { // Show LED when button is pressed. Used for the MENU "are you sure" loops
  for (uint8_t s = 0; s < NUMBER_OF_LEDS; s++) {
    if (SP[s].Pressed) LED_show_colour(s, Setting.LED_global_colour);
    else LED_show_colour(s, 0);
  }
  update_LEDS = true;
}

void LED_show_are_you_sure() {
  LED_turn_all_off();
#ifdef BACKLIGHTNEOPIXELPIN
  Backlight_show_colour(9, Setting.LED_global_colour);
  Backlight_show_colour(10, Setting.LED_global_colour);
  Backlights.show();
#else
  for (uint8_t l = 0; l < NUMBER_OF_DISPLAYS; l++) LCD_switch_off_backlight(l);
  LCD_switch_on_backlight(9);
  LCD_switch_on_backlight(10);
#endif
}

