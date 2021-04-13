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
#define LED_MASK 0x7F // The colour stripped of flashing bit

// Defining FX colours types - the colours are set in the menu.
// Colour numbers 241 - 255 are reserved for colour sublists.
#define FX_TYPE_OFF 0
#define FX_DEFAULT_TYPE 240 // For parameters that fit no other category
#define FX_GTR_TYPE 239 // For COSM/guitar settings
#define FX_PITCH_TYPE 238 // For pitch FX
#define FX_FILTER_TYPE 237 // For filter FX
#define FX_DIST_TYPE 236 // For distortion FX
#define FX_AMP_TYPE 235 // For amp FX and amp solo
#define FX_MODULATE_TYPE 234 //B For modulation FX
#define FX_DELAY_TYPE 233 // For delays
#define FX_REVERB_TYPE 232// For reverb FX
#define FX_LOOPER_TYPE 231 // For looper
#define FX_WAH_TYPE 230 // For wahs
#define FX_DYNAMICS_TYPE 229 // For gates/etc
#define FX_SHOW_TAP_TEMPO 228

//Lets make some colours (R,G,B)


#define NUMBER_OF_COLOURS 32
#define NUMBER_OF_SELECTABLE_COLOURS 12
colour colours[NUMBER_OF_COLOURS] = {
  {0, 0, 0} ,   // Colour 0 is LED OFF
  {0, 255, 0} ,  // Colour 1 is Green
  {255, 0, 0} ,  //  Colour 2 is Red
  {0, 0, 255} ,  // Colour 3 is Blue
  {255, 92, 0} ,  // Colour 4 is Orange
  {0, 204, 128} ,  // Colour 5 is Cyan
  {204, 255, 204} ,  // Colour 6 is White
  {204, 204, 0} ,   // Colour 7 is Yellow
  {128, 0, 255} ,   // Colour 8 is Purple
  {255, 0, 128} ,   // Colour 9 is Pink
  {92, 255, 114} ,   // Colour 10 is Soft Green
  {0, 92, 204} ,   // Colour 11 is Light Blue
  {0, 0, 0} ,   // Colour 12 is spare
  {0, 0, 0} ,   // Colour 13 is spare
  {0, 0, 0} ,   // Colour 14 is spare
  {0, 0, 0} ,   // Colour 15 is spare
  {0, 0, 0} ,   // Colour 16 is LED OFF dimmed
  {0, 51, 0} ,  // Colour 17 is Green dimmed
  {51, 0, 0} ,  //  Colour 18 is Red dimmed
  {0, 0, 51} ,  // Colour 19 is Blue dimmed
  {51, 26, 0} ,  // Colour 20 is Orange dimmed
  {0, 41, 25} ,  // Colour 21 is Cyan dimmed
  {51, 51, 51} ,  // Colour 22 is White dimmed
  {51, 51, 0} ,   // Colour 23 is Yellow dimmed
  {36, 0, 72} ,   // Colour 24 is Purple dimmed
  {36, 0, 36} ,   // Colour 25 is Pink dimmed
  {18, 40, 23} ,   // Colour 26 is Soft green dimmed
  {0, 22, 41} ,   // Colour 27 is Light Blue dimmed
  {0, 0, 0} ,   // Colour 28 is spare dimmed
  {0, 0, 0} ,   // Colour 29 is spare dimmed
  {0, 0, 0} ,   // Colour 30 is spare dimmed
  {0, 0, 0} ,   // Colour 31 is spare dimmed
};

colour Backlight_colours_Adafruit[NUMBER_OF_COLOURS] = {
  {0, 0, 0} ,   // Colour 0 is LED OFF
  {0, 255, 0} ,  // Colour 1 is Green
  {255, 50, 50} ,  //  Colour 2 is Red
  {50, 50, 255} ,  // Colour 3 is Blue
  {255, 178, 50} ,  // Colour 4 is Orange
  {0, 204, 128} ,  // Colour 5 is Cyan
  {204, 255, 204} ,  // Colour 6 is White
  {204, 204, 0} ,   // Colour 7 is Yellow
  {128, 50, 204} ,   // Colour 8 is Purple
  {255, 50, 128} ,   // Colour 9 is Pink
  {92, 255, 114} ,   // Colour 10 is Soft Green
  {0, 92, 204} ,   // Colour 11 is Light Blue
  {0, 0, 0} ,   // Colour 12 is spare
  {0, 0, 0} ,   // Colour 13 is spare
  {0, 0, 0} ,   // Colour 14 is spare
  {0, 0, 0} ,   // Colour 15 is spare
};

colour Backlight_colours_Buydisplay[NUMBER_OF_COLOURS] = {
  {0, 0, 0} ,   // Colour 0 is LED OFF
  {0, 255, 0} ,  // Colour 1 is Green
  {255, 50, 50} ,  //  Colour 2 is Red
  {50, 50, 255} ,  // Colour 3 is Blue
  {255, 178, 50} ,  // Colour 4 is Orange
  {0, 255, 128} ,  // Colour 5 is Cyan
  {192, 255, 240} ,  // Colour 6 is White
  {255, 255, 0} ,   // Colour 7 is Yellow
  {128, 50, 255} ,   // Colour 8 is Purple
  {255, 70, 150} ,   // Colour 9 is Pink
  {92, 255, 114} ,   // Colour 10 is Soft Green
  {0, 92, 204} ,   // Colour 11 is Light Blue
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

#ifdef MAINDISPLAYBACKLIGHTPIN
Adafruit_NeoPixel Main_backlight = Adafruit_NeoPixel(1, MAINDISPLAYBACKLIGHTPIN, NEO_RGB + NEO_KHZ800);
#endif

boolean update_LEDS = true;
uint8_t global_tap_tempo_LED;
//uint8_t prev_page_shown = 255;
uint8_t MIDI_LEDs[NUMBER_OF_LEDS];
uint8_t main_backlight_colour = 0;
bool LED_state_changed = false;

class LED_init_class
{
  public:
    LED_init_class(); // Constructor
};

LED_init_class::LED_init_class() {
  LEDs.begin(); // This initializes the NeoPixel library.

  //Turn the LEDs off repeatedly for 100 ms to reduce startup flash of LEDs
  //When neopixel LEDs are powered on, they burn at 100%.
  unsigned int startupTimer = millis();
  while (millis() - startupTimer <= STARTUP_TIMER_LENGTH) {
    LEDs.show();
  }
}

LED_init_class LED_init; // This will dim the Neopixel LEDs earlier in the code...

void setup_LED_control()
{
  LEDs.begin(); // This initializes the NeoPixel library.

  //Turn the LEDs off repeatedly for 100 ms to reduce startup flash of LEDs
  //When neopixel LEDs are powered on, they burn at 100%.
  unsigned int startupTimer = millis();
  while (millis() - startupTimer <= STARTUP_TIMER_LENGTH) {
    LEDs.show();
  }
  LEDflashTimer = millis(); // Initialize timer for flashing LEDs

  // Startup backlights
#ifdef BACKLIGHTNEOPIXELPIN
  Backlights.begin();
  //Backlights.setPixelColor(0,255,255,255);
  Backlights.show();
#endif

#ifdef MAINDISPLAYBACKLIGHTPIN
  Main_backlight.begin();
  //Main_backlight_show_colour(6);
  Main_backlight.show();
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

#ifdef MAINDISPLAYBACKLIGHTPIN
  Main_backlight.setBrightness(Setting.Backlight_brightness);
#endif

  LED_state_changed = false;

  //Check the switch_states on the current page
  for (uint8_t s = 0; s < NUMBER_OF_LEDS; s++) {

    //Copy the LED state from the switch state
    uint8_t sw = s + 1;
    uint8_t colour;
    uint8_t Dev = SP[sw].Device;
    uint8_t cs;
    if (Dev == CURRENT) Dev = Current_device;

    if (Dev < NUMBER_OF_DEVICES) {
      switch (SP[sw].Type) {
        case PATCH:
          if ((SP[sw].Sel_type == SELECT) || (SP[sw].Sel_type == BANKSELECT)) {
            if ((SP[sw].Sel_type == BANKSELECT) && (Device[Dev]->flash_LEDs_for_patch_bank_switch(sw))) {
              LED_show_colour(s, SP[sw].Colour | LED_FLASHING); //Flash the devices PATCH LEDs
            }
            else {
              if (Device[Dev]->patch_number == SP[sw].PP_number) {
                if (Device[Dev]->is_on) LED_show_colour(s, SP[sw].Colour);
                else LED_show_colour(s, SP[sw].Colour | LED_DIMMED); // Show dimmed colour for devices that are not connected
              }
              else LED_show_colour(s, 0);
            }
          }
          else {
            if (SP[sw].Pressed) LED_show_colour(s, SP[sw].Colour);
            else LED_show_colour(s, SP[sw].Colour | LED_DIMMED);
          }
          Backlight_show_colour(s, SP[sw].Colour);
          break;
        case PAR_BANK_CATEGORY:
        case PAR_BANK_UP:
        case PAR_BANK_DOWN:
        case TOGGLE_EXP_PEDAL:
        case SAVE_PATCH:
          colour = SP[sw].Colour;
          if (colour >= FX_LOOPER_TYPE) colour = LED_FX_type_colour(SP[sw].Colour); // Check for FX colours
          if (SP[sw].Pressed) LED_show_colour(s, colour);
          else LED_show_colour(s, colour | LED_DIMMED);
          Backlight_show_colour(s, colour);
          break;
        case DIRECT_SELECT:
          if (Device[Dev]->valid_direct_select_switch(SP[sw].PP_number)) {
            LED_show_colour(s, SP[sw].Colour | LED_FLASHING); //Flash the devices PATCH LEDs
            Backlight_show_colour(s, SP[sw].Colour);
          }
          else {
            LED_show_colour(s, 0);
            Backlight_show_colour(s, 0);
          }
          break;
        case PARAMETER:
        case PAR_BANK:
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
          if (Device[Dev]->connected) Backlight_show_colour(s, LED_FX_type_colour(SP[sw].Colour));
          else Backlight_show_colour(s, Device[Dev]->my_LED_colour);
          break;
        case ASSIGN:
          if ((SP[sw].Sel_type == SELECT) || (SP[sw].Sel_type == BANKSELECT)) {
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
            if (Device[Dev]->connected) Backlight_show_colour(s, LED_FX_type_colour(SP[sw].Colour));
            else Backlight_show_colour(s, Device[Dev]->my_LED_colour);
          }
          else {
            colour = SP[sw].Colour;
            if (SP[sw].Pressed) LED_show_colour(s, colour);
            else LED_show_colour(s, colour | LED_DIMMED);
            Backlight_show_colour(s, colour);
          }
          break;
        case OPEN_PAGE_DEVICE:
        case OPEN_NEXT_PAGE_OF_DEVICE:
          //if ((SP[sw].Pressed) || ((SP[sw].PP_number == prev_page_shown) && (SP[sw].Device == Current_device))) LED_show_colour(s, Setting.LED_global_colour);
          if (SP[sw].Pressed) LED_show_colour(s, Setting.LED_global_colour);
          else LED_show_colour(s, 0);
          Backlight_show_colour(s, Setting.LED_global_colour);
          break;
        case MUTE:
          if (Device[Dev]->is_on) LED_show_colour(s, 0);
          else LED_show_colour(s, SP[sw].Colour);
          Backlight_show_colour(s, SP[sw].Colour);
          break;
        case SNAPSCENE:
          cs = Device[Dev]->current_snapscene;
          if ((cs == SP[sw].PP_number) || (SP[sw].Pressed)) LED_show_colour(s, SP[sw].Colour);
          else if ((cs != 0) && ((cs == SP[sw].Value1) || (cs == SP[sw].Value2) || (cs == SP[sw].Value3))) LED_show_colour(s, SP[sw].Colour | LED_DIMMED);
          else if (Device[Dev]->check_snapscene_active(SP[sw].PP_number)) LED_show_colour(s, 3 | LED_DIMMED); // Show blue LEDs for active scenes SY1000
          else LED_show_colour(s, 0);
          Backlight_show_colour(s, SP[sw].Colour);
          break;
        case LOOPER:
          LED_show_colour(s, Device[Dev]->show_looper_LED(sw));
          Backlight_show_colour(s, Device[Dev]->show_looper_LED(sw) & 0x0F);
          break;
        default:
          LED_show_colour(s, 0); // Show nothing with undefined LED
          Backlight_show_colour(s, 0);
          break;
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
        case MENU:
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
        case PAGE:
          if (SP[sw].Pressed) LED_show_colour(s, SP[sw].Colour);
          else LED_show_colour(s, 0);
          if (SP[sw].Latch == BANKSELECT) { // Override colour if bank is in selection
            if ((device_in_bank_selection == PAGE_BANK_SELECTION_IN_PROGRESS) && (SCO_valid_page(SP[sw].PP_number))) LED_show_colour(s, SP[sw].Colour | LED_FLASHING);
          }
          Backlight_show_colour(s, SP[sw].Colour);
          break;
        case SELECT_NEXT_DEVICE:
          colour = Device[SCO_get_number_of_next_device()]->my_LED_colour;
          if (SP[sw].Pressed) LED_show_colour(s, colour);
          else LED_show_dimmed(s, colour);
          Backlight_show_colour(s, colour);
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

  if (LED_state_changed) {
    LEDs.show();
#ifdef BACKLIGHTNEOPIXELPIN
    Backlights.show();
#endif
    MIDI_update_LEDs(MIDI_LEDs, NUMBER_OF_LEDS);
    //DEBUGMSG("LEDs updated");
  }
}

void LED_show_dimmed(uint8_t no, uint8_t colour) {
  if (Setting.LED_FX_off_is_dimmed) LED_show_colour(no, colour | LED_DIMMED);
  else LED_show_colour(no, 0);
}

uint8_t LED_FX_type_colour(uint8_t type) { // Read the FX colour from the settings
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
    case FX_LOOPER_TYPE: return Setting.FX_LOOPER_colour;
    case FX_WAH_TYPE: return Setting.FX_WAH_colour;
    case FX_DYNAMICS_TYPE: return Setting.FX_DYNAMICS_colour;
    case FX_SHOW_TAP_TEMPO: return global_tap_tempo_LED;
    default: return type;
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
        if (MIDI_LEDs[LED_number] != number_fixed) {
          MIDI_LEDs[LED_number] = number_fixed;
          LED_state_changed = true;
        }
      }
      else {
        // Turn the LED off
        if (Setting.Physical_LEDs) LEDs.setPixelColor(LED_order[LED_number], 0, 0, 0);
        if (Setting.Virtual_LEDs) Set_virtual_LED_colour(LED_number, 0); // Update the virtual LEDs on the LCD as well
        if (MIDI_LEDs[LED_number] != 0) {
          MIDI_LEDs[LED_number] = 0;
          LED_state_changed = true;
        }
      }
    }
    else {
      // Turn the LED on
      if (Setting.Physical_LEDs) LEDs.setPixelColor(LED_order[LED_number], LEDs.Color(colours[number_fixed].red, colours[number_fixed].green, colours[number_fixed].blue));
      if (Setting.Virtual_LEDs) Set_virtual_LED_colour(LED_number, number_fixed); // Update the virtual LEDs on the LCD as well
      if (MIDI_LEDs[LED_number] != number_fixed) {
        MIDI_LEDs[LED_number] = number_fixed;
        LED_state_changed = true;
      }
    }
  }
  else {
    // Invalid colour: show message and give error
    LEDs.setPixelColor(LED_order[LED_number], LEDs.Color(colours[10].red, colours[10].green, colours[10].blue));
    DEBUGMSG("Invalid colour (number " + String(number_fixed) + ") on LED " + String(LED_number));
    if (MIDI_LEDs[LED_number] != 0) {
      MIDI_LEDs[LED_number] = 0;
      LED_state_changed = true;
    }
  }
}

void Backlight_show_colour(uint8_t LED_number, uint8_t colour_number) { // Sets the specified LED to the specified colour
#ifdef BACKLIGHTNEOPIXELPIN
  if ((colour_number < NUMBER_OF_COLOURS) && (LED_number < NUMBER_OF_BACKLIGHTS)) {
    if (Setting.RGB_Backlight_scheme != 1) {
      Backlights.setPixelColor(Backlight_order[LED_number], LEDs.Color(Backlight_colours_Adafruit[colour_number].red, Backlight_colours_Adafruit[colour_number].green, Backlight_colours_Adafruit[colour_number].blue));
    }
    else {
      Backlights.setPixelColor(Backlight_order[LED_number], LEDs.Color(Backlight_colours_Buydisplay[colour_number].red, Backlight_colours_Buydisplay[colour_number].green, Backlight_colours_Buydisplay[colour_number].blue));
    }
  }
#else
  // try to switch backlights
  if (colour_number == 0) LCD_switch_off_backlight(LED_number);
  else LCD_switch_on_backlight(LED_number);
#endif
}


void Main_backlight_show_colour(uint8_t colour_number) { // Sets the specified LED to the specified colour
#ifdef MAINDISPLAYBACKLIGHTPIN
  if ((colour_number != main_backlight_colour) && (colour_number < NUMBER_OF_COLOURS)) {
    DEBUGMSG("Main Backlight gets colour #" + String(colour_number));
    if (Setting.RGB_Backlight_scheme != 1) {
      Main_backlight.setPixelColor(0, LEDs.Color(Backlight_colours_Adafruit[colour_number].red, Backlight_colours_Adafruit[colour_number].green, Backlight_colours_Adafruit[colour_number].blue));
    }
    else {
      Main_backlight.setPixelColor(0, LEDs.Color(Backlight_colours_Buydisplay[colour_number].red, Backlight_colours_Buydisplay[colour_number].green, Backlight_colours_Buydisplay[colour_number].blue));
    }
    Main_backlight.show();
    main_backlight_colour = colour_number;
  }
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

void LED_show_initializing_data() { // To indicate EEPROM memory being written.
#ifdef VCMINI
  if (NUMBER_OF_LEDS < 3) return;
  LED_show_colour(0, Setting.LED_global_colour);
  LED_show_colour(1, Setting.LED_global_colour);
  LED_show_colour(2, Setting.LED_global_colour);
  LEDs.show();
#else
  if (NUMBER_OF_LEDS < 10) return;
  LED_show_colour(5, Setting.LED_global_colour);
  LED_show_colour(6, Setting.LED_global_colour);
  LED_show_colour(9, Setting.LED_global_colour);
  LED_show_colour(10, Setting.LED_global_colour);
  LEDs.show();
#endif
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

void LED_start_remote_control() {
  MIDI_update_LEDs(MIDI_LEDs, NUMBER_OF_LEDS);
}
