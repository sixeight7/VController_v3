// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: LCD Hardware Initialization
// Section 2: Update of main LCD
// Section 3: Update of individual LCDs
// Section 4: LCD Functions
// Section 5: Virtual LED functions
// Section 6: TFT functions
// Section 7: TFT On-screen Keyboard
// Section 8: TFT pong game

// Functions for LCD control
// Main LCD is a large 16x2 LCD display with a serial i2c module attached
// Futhermore we have 12 16x2 LCD displays for the bottom 12 switches

// ********************************* Section 1: LCD Hardware Initialization ********************************************
// Hardware setting have been moved to hardware.h

//#include <Wire.h>
//#include <i2c_t3.h>
#include <LCD.h>
//#include <LiquidCrystal_I2C.h>
#include "lcdlib1.h" // i2c expander board with PCF8754
#include "lcd_lib.h" // For the VController display boards 

// Main display:
#ifdef PRIMARY_MAIN_DISPLAY_ADDRESS
LiquidCrystal_PCF8745	Main_lcd(PRIMARY_MAIN_DISPLAY_ADDRESS, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN); //First number is the i2c address of the display
#endif

#ifdef MAIN_DISPLAY_TEENSY_PINS
#include <LiquidCrystal.h>
LiquidCrystal  Main_lcd(MAIN_DISPLAY_TEENSY_PINS);
#endif

#ifdef IS_VCTOUCH
#ifdef USE_TFT_USER_FONT
#include "fonts.h"
#endif
#include "pics.h"
#include <ER-TFTM0784-1.h>
ER_TFTM0784 tft = ER_TFTM0784(SPI_SPEED, SDI_PIN, SDO_PIN, SCLK_PIN, CS_PIN, RST_SER_PIN);
#include <2828.h>
SSD2828 Master_bridge = SSD2828(SDI_2828_PIN, SCLK_2828_PIN, CS_2828_PIN, RST_2828_PIN);

Goodix touch = Goodix(INT_911_PIN, RST_911_PIN);

#define layer1_start_addr 0
#define layer2_start_addr 1024000   //400*1280*2 
#define layer3_start_addr 2048000   //400*1280*2*2
#define layer4_start_addr 3072000   //400*1280*2*3
#define layer5_start_addr 4096000   //400*1280*2*4
#define layer6_start_addr 5120000   //400*1280*2*5
#define layer7_start_addr 6144000   //400*1280*2*5

struct TFT_layout_struct { // Combines all the data we need for controlling a parameter in a device
  uint16_t y_pos;
  uint16_t x_pos;
  uint16_t y_size;
  uint16_t x_size;
};

const TFT_layout_struct TFT_layout[15] = {
  { 323,    0, 71, 255}, // Switch 1
  { 323,  256, 71, 255}, // Switch 2
  { 323,  512, 71, 255}, // Switch 3
  { 323,  768, 71, 255}, // Switch 4
  { 323, 1024, 71, 255}, // Switch 5
  { 251,    0, 71, 255}, // Switch 6
  { 251,  256, 71, 255}, // Switch 7
  { 251,  512, 71, 255}, // Switch 8
  { 251,  768, 71, 255}, // Switch 9
  { 251, 1024, 71, 255}, // Switch 10
  {   0,    0, 71, 255}, // Switch 11
  {   0,  256, 71, 255}, // Switch 12
  {   0,  512, 71, 255}, // Switch 13
  {   0,  768, 71, 255}, // Switch 14
  {   0, 1024, 71, 255}, // Switch 15
};

bool touch_drag_active = false;
uint8_t last_touch_switch_pressed = 0;
uint16_t last_touch_x_pos;
uint16_t last_touch_y_pos;
uint16_t TFT_label_colour[15] = { Black };
DMAMEM char TFT_title_text[NUMBER_OF_TOUCH_SWITCHES][LCD_DISPLAY_SIZE + 1] = { ' ' };
DMAMEM char TFT_label_text[NUMBER_OF_TOUCH_SWITCHES][LCD_DISPLAY_SIZE + 1] = { ' ' };
bool TFT_bar_showing = false;
uint8_t TFT_current_brightness = 0;
uint8_t TFT_current_device_for_pic = 255;
String TFT_main_spaces = ""; // To clear the display
String TFT_spaces = ""; // To clear the display
#endif

bool touch_active = false;
uint8_t show_ble_state = 0;
uint8_t show_wifi_state = 0;
uint8_t show_rtpmidi_state = 0;

// Main display modes - for top or bottom line
#define MD_SHOW_PAGE_NAME 0
#define MD_SHOW_CURRENT_PATCH 1
#define MD_SHOW_PATCHES_COMBINED 2
#define MD_SHOW_3_SWITCH_LABELS 3 // Made for VC mini to display switch labels
#define MD_SHOW_CURRENT_SCENE 4
#define MD_SHOW_ALL_PATCH_NUMBERS 5
#define MD_SHOW_PAGE_NUMBER 6
#define MD_SHOW_PAGE_NUMBER_AND_NAME 7
#define MD_SHOW_SONG_NUMBER_AND_NAME 8
#define MD_SHOW_PART_NUMBER_AND_NAME 9
#define MD_SHOW_SETLIST_NUMBER_AND_NAME 10
#define MD_SHOW_SETLIST_TARGET_NAME 11

// Main display_modes - added to top line
#define MDT_OFF 0
#define MDT_CURRENT_DEVICE 1
#define MDT_CURRENT_TEMPO 2
#define MDT_SCENE_NAME 3
#define MDT_PATCH_NUMBER 4
#define MDT_SCENE_NUMBER 5

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

// Method 2: declare displays using regular i2C backpacks if defined in hardware.h
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

unsigned long messageTimer = 0;
unsigned long mainLedbarTimer = 0;
unsigned long ledbarTimer = 0;
bool popup_title_showing = false;
bool popup_label_showing = false;
bool main_ledbar_showing = false;
bool main_looper_ledbar_showing = false;
uint8_t ledbar_showing = 0;

String Current_patch_number_string = "";
String Current_page_name = "";
String Current_device_name = "";
//String Main_menu_line1 = ""; // Text that is show on the main display from the menu on line 1
//String Main_menu_line2 = ""; // Text that is show on the main display from the menu on line 2
//String topline;
DMAMEM char main_lcd_title[MAIN_LCD_DISPLAY_SIZE + 1] = { ' ' }; // Text that is show on the main display from the menu on line 1
DMAMEM char main_lcd_label[MAIN_LCD_DISPLAY_SIZE + 1] = { ' ' }; // Text that is show on the main display from the menu on line 2
uint8_t Main_menu_cursor = 0; // Position of main menu cursor. Zero = off.
DMAMEM char lcd_title[LCD_DISPLAY_SIZE + 1] = { ' ' }; // char array reserved for individual display titles
DMAMEM char lcd_label[LCD_DISPLAY_SIZE + 1] = { ' ' }; // char array reserved for individual display labels
DMAMEM char lcd_mem[NUMBER_OF_DISPLAYS + 1][2][LCD_DISPLAY_SIZE + 1] = { ' ' }; // Memory for individual displays

String Combined_patch_name; // Patchname displayed in the main display
uint16_t Current_patch_number = 0;              // Patchnumber displayed in the main display

String Display_number_string; // Placeholder for patchnumbers on display
uint8_t my_looper_lcd = 0; // The LCD that will show the looper progress bar (0 is no display)
bool backlight_on[NUMBER_OF_DISPLAYS] = {false}; // initialize all backlights off
bool pong_active = false;

byte arrow_up[8] = { // The character for the UP arrow
  0b00100,
  0b01110,
  0b11111,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00000
};

byte arrow_down[8] = { // The character for DOWN arrow
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b11111,
  0b01110,
  0b00100,
  0b00000
};

// Custom characters for the expression pedal bar
byte block1[8] = {16, 16, 16, 16, 16, 16, 16, 0};
byte block2[8] = {24, 24, 24, 24, 24, 24, 24, 0};
byte block3[8] = {28, 28, 28, 28, 28, 28, 28, 0};
byte block4[8] = {30, 30, 30, 30, 30, 30, 30, 0};
byte block5[8] = {31, 31, 31, 31, 31, 31, 31, 0};

// Custom quarter note character
byte qnote[8] = {B00010, B00010, B00010, B00010, B01110, B11110, B01100, 0};

// Custom character numbers
#define CHAR_BLOCK1 0
#define CHAR_BLOCK2 1
#define CHAR_BLOCK3 2
#define CHAR_BLOCK4 3
#define CHAR_BLOCK5 4
#ifndef MAIN_TFT_DISPLAY
#define CHAR_ARROW_UP 5
#define CHAR_ARROW_DOWN 6
#define CHAR_QUARTER_NOTE 7
#else
#define CHAR_ARROW_DOWN 127
#define CHAR_ARROW_UP 128
#define CHAR_QUARTER_NOTE 127
#endif
#define CHAR_VLED 7

void setup_LCD_control()
{
  DEBUGMAIN("Starting LCD control");

  // Initialize main LCD
#ifndef MAIN_TFT_DISPLAY
  DEBUGMAIN("Starting main LCD display");
  Main_lcd.begin (16, 2); // Main LCD is 16x2
#endif
#ifdef SECONDARY_MAIN_DISPLAY_ADDRESS
  if (!Main_lcd.check_initialized()) { // Check if i2c address is found on primary address
    Main_lcd.set_addr(SECONDARY_MAIN_DISPLAY_ADDRESS); // If not try on secondary address
    Main_lcd.begin (16, 2); // Main LCD is 16x2
  }
  Main_lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  Main_lcd.setBacklight(true); // Backlight state is the same as initial on or off state...
#endif

#ifdef IS_VCTOUCH
  DEBUGMAIN("Starting main TFT display");
  Master_bridge.Reset(); // SD2828 reset
  tft.RA8876_IO_Init(); // RA8876 IO init
  tft.RA8876_HW_Reset(); // RA8876 HW reset
  Master_bridge.Initial(); // SSD2828 init
  tft.RA8876_initial();
  tft.Display_ON();
  DEBUGMAIN("Starting backlight");
#ifdef MAIN_TFT_BACKLIGHT_PIN
  analogWriteResolution(12);
  pinMode(MAIN_TFT_BACKLIGHT_PIN, OUTPUT);
  //digitalWrite(MAIN_TFT_BACKLIGHT_PIN, HIGH);
  analogWriteFrequency(MAIN_TFT_BACKLIGHT_PIN, 36621.09); // Change the frequency to avoid noise - ideal frequency for 12 bit resolution (https://www.pjrc.com/teensy/td_pulse.html)
  TFT_current_brightness = EEPROM_read_brightness();
  analogWrite(MAIN_TFT_BACKLIGHT_PIN, 256); // Enough to start up the display on low brightness
  for (uint8_t c = 0; c < MAIN_LCD_DISPLAY_SIZE; c++) TFT_main_spaces += ' ';
  for (uint8_t c = 0; c < LCD_DISPLAY_SIZE - 1; c++) TFT_spaces += ' ';
#else
  tft.Backlight_ON();
#endif
  TFT_fillScreen(Black);
  DEBUGMAIN("Starting touch module");
  touch.setHandler(TFT_handleTouch);
  touch.begin();
#endif


  // Show startup screen
  LCD_show_startup_message();

#ifdef IS_VCTOUCH
  TFT_draw_main_grid();
#endif

#ifndef MAIN_TFT_DISPLAY
  Main_lcd.createChar(CHAR_BLOCK1, block1);
  Main_lcd.createChar(CHAR_BLOCK2, block2);
  Main_lcd.createChar(CHAR_BLOCK3, block3);
  Main_lcd.createChar(CHAR_BLOCK4, block4);
  Main_lcd.createChar(CHAR_BLOCK5, block5);
  Main_lcd.createChar(CHAR_ARROW_UP, arrow_up);
  Main_lcd.createChar(CHAR_ARROW_DOWN, arrow_down);
  Main_lcd.createChar(CHAR_QUARTER_NOTE, qnote);
#endif

  memset(main_lcd_title, ' ', sizeof(main_lcd_title)); // Clear topline of main lcd

#ifdef INDIVIDUAL_DISPLAYS

  // Initialize individual LCDs - same for both methods
  DEBUGMAIN("Starting individual displays");
  for (uint8_t i = 0; i < NUMBER_OF_DISPLAYS; i++) {
    lcd[i].begin (16, 2);
    lcd[i].setBacklightPin(BACKLIGHT_PIN, POSITIVE);
    LCD_switch_on_backlight(i);
    lcd[i].createChar(CHAR_BLOCK1, block1);
    lcd[i].createChar(CHAR_BLOCK2, block2);
    lcd[i].createChar(CHAR_BLOCK3, block3);
    lcd[i].createChar(CHAR_BLOCK4, block4);
    lcd[i].createChar(CHAR_BLOCK5, block5);
    lcd[i].createChar(CHAR_ARROW_UP, arrow_up);
    lcd[i].createChar(CHAR_ARROW_DOWN, arrow_down);
    LCD_init_virtual_LED(i); // Initialize the virtual LEDs
  }
#endif

  // Reserve space for Strings - this should avoid memory defragemntation
  Current_patch_number_string.reserve(17);
  Current_page_name.reserve(17);
  Current_device_name.reserve(17);
  Combined_patch_name.reserve(17);
  //Main_menu_line1.reserve(17);
  //Main_menu_line2.reserve(17);
  LCD_clear_string(Combined_patch_name);
  LCD_clear_memory();
}

// ********************************* Section 2: Update of main LCD ********************************************

void main_LCD_control()
{
  if ((!on_screen_keyboard_active) && (!pong_active)) {

    if (update_main_lcd) {
      DEBUGMSG("Refreshing main display");
      update_main_lcd = false;
      LCD_update_top_line();
      LCD_update_bottom_line();
      LCD_print_main_lcd_txt();
      LCD_show_text_entry_cursor(); // Must be last to avoid incorrect cursor position on LCD displays
    }

    if ((popup_label_showing) || (popup_title_showing)) {
      if (millis() > messageTimer) { // Check if message is still showing
        popup_title_showing = false;
        popup_label_showing = false;
        update_main_lcd = true; // Now update the main display, so the status message will be overwritten
      }
    }

    if (main_ledbar_showing) {
      if (millis() > mainLedbarTimer) { // Check if message is still showing
        main_ledbar_showing = false;
        main_looper_ledbar_showing = false;
        update_main_lcd = true; // Now update the main display, so the status message will be overwritten
      }
    }

    if ((ledbar_showing > 0) && (millis() > ledbarTimer)) {
      uint8_t ledbar_lcd = ledbar_showing;
      ledbar_showing = 0;
      LCD_update(ledbar_lcd, true); // Now update the ledbar display, so the ledbar will be overwritten
    }

#ifdef IS_VCTOUCH
    TFT_update_device_pic();
    if ((TFT_bar_showing) && (millis() > ledbarTimer)) {
      TFT_hide_bar();
      TFT_bar_showing = false;
    }
    LCD_update_TFT_brightness();
#endif

    if (update_lcd > 0) {
      LCD_update(update_lcd, true);
      update_lcd = 0;
    }
  } // (!on_screen_keyboard_active)

#ifdef IS_VCTOUCH
  touch.loop();
  if (pong_active) TFT_pong_update();
#endif
}

#define PATCH_NUMBER_SEPERATOR "+"

void LCD_show_text_entry_cursor() {
#ifndef MAIN_TFT_DISPLAY
  if (Current_page == PAGE_MENU) {
    // Show cursor
    if (Main_menu_cursor > 0) {
      Main_lcd.setCursor (Main_menu_cursor - 1, 1);
      Main_lcd.cursor();
    }
    else Main_lcd.noCursor();
  }
#endif
}

void LCD_update_top_line() {

  if (popup_title_showing) return;

  for (uint8_t i = 0; i < MAIN_LCD_DISPLAY_SIZE; i++) {
    main_lcd_title[i] = ' ';
  }

  if (global_tuner_active) { // Show tuner message if active
    LCD_main_set_title("Tuner active");
    return;
  }

  if (Current_page == PAGE_MENU) { // Show menu content if active
    menu_set_main_title();

    return;
  }

  // Show patchnumbers and device name on top line
  String topline = "";
  String msg = "";

  uint8_t toplinemode = Setting.Main_display_top_line_mode;
  if (Current_mode == SONG_MODE) toplinemode = MD_SHOW_SONG_NUMBER_AND_NAME;
  if (Current_mode == PAGE_MODE) {
#ifdef IS_VCMINI
    if ((Current_page != PAGE_FOR_DEVICE_MODE) && (Current_page != PAGE_FOR_PAGE_MODE)) toplinemode = MD_SHOW_PAGE_NUMBER_AND_NAME;
    else toplinemode = MD_SHOW_PAGE_NAME;
#else
    toplinemode = MD_SHOW_PAGE_NUMBER;
#endif
  }
  if (Current_page == PAGE_SETLIST_SELECT) toplinemode = MD_SHOW_SETLIST_NUMBER_AND_NAME;
  LCD_fill_line_main_display(toplinemode, topline);
  topline.trim();

  if (Setting.Main_display_show_top_right == MDT_PATCH_NUMBER) {
    if (Current_device < NUMBER_OF_DEVICES) {
      uint16_t patch_no = Device[Current_device]->patch_number;
      Device[Current_device]->number_format(patch_no, msg);
      msg += ':';
      uint8_t spaces = (MAIN_LCD_DISPLAY_SIZE - topline.length() + 1) / 2;
      if (spaces > msg.length()) spaces -= msg.length();
      else spaces = 0;
      String spacestring = "";
      for (uint8_t i = 0; i < spaces; i++) spacestring += ' ';
      topline = msg + spacestring + topline;
    }
  }

  if (Setting.Main_display_show_top_right == MDT_SCENE_NUMBER) {
    if (Current_device < NUMBER_OF_DEVICES) {
      uint8_t scene_no = Device[Current_device]->current_snapscene;
      if (scene_no > 0) {
        topline = String(scene_no) + ": " + topline;
      }
    }
  }

  uint8_t topline_length = topline.length();
  if (topline_length > MAIN_LCD_DISPLAY_SIZE) topline_length = MAIN_LCD_DISPLAY_SIZE;
  for (uint8_t i = 0; i < topline_length; i++) {
    main_lcd_title[i] = topline[i];
  }

  String top_right = "";

  if (Setting.Main_display_show_top_right == MDT_OFF) {
    LCD_main_set_title(main_lcd_title); // To center the title
  }

  if (Setting.Main_display_show_top_right == MDT_CURRENT_DEVICE) {
    if (Current_device < NUMBER_OF_DEVICES) {
      Current_device_name = Device[Current_device]->device_name;
      top_right = Current_device_name;
    }
  }

  if (Setting.Main_display_show_top_right == MDT_CURRENT_TEMPO) {
    top_right = char(CHAR_QUARTER_NOTE);
    top_right += "=";
    top_right += String(Setting.Bpm);
  }

  if (Setting.Main_display_show_top_right == MDT_SCENE_NAME) {
    uint8_t sc = Device[Current_device]->current_snapscene;
    if (sc > 0) Device[Current_device]->get_snapscene_label(sc, top_right);
    else top_right = "";
    top_right.trim();
    if (top_right.length() > 8) top_right.substring(0, 8);
  }

  uint8_t topright_length = top_right.length();
  if ((topline_length < (MAIN_LCD_DISPLAY_SIZE - topright_length)) && (topright_length > 0)) { // Only show top right string if it fits
    for (uint8_t i = 0; i < topright_length; i++) {
      main_lcd_title[MAIN_LCD_DISPLAY_SIZE - topright_length + i] = top_right[i];
    }
  }
}


void LCD_update_bottom_line() {
  if (popup_label_showing) return;
  for (uint8_t i = 0; i < MAIN_LCD_DISPLAY_SIZE; i++) {
    main_lcd_label[i] = ' ';
  }

  if (Current_page == PAGE_MENU) {
    menu_set_main_label();
    return;
  }

  String bottomline = "";
  uint8_t bottomlinemode = Setting.Main_display_bottom_line_mode;
  if (Current_mode == SONG_MODE) bottomlinemode = MD_SHOW_PART_NUMBER_AND_NAME;
  if (Current_mode == PAGE_MODE) bottomlinemode = MD_SHOW_PAGE_NAME;
  if (Current_page == PAGE_SETLIST_SELECT) bottomlinemode = MD_SHOW_SETLIST_TARGET_NAME;
#ifdef IS_VCMINI
  if ((Current_mode == PAGE_MODE) || (Current_page <= PAGE_SELECT) || (Current_page == PAGE_SETLIST_SELECT)) bottomlinemode = MD_SHOW_3_SWITCH_LABELS; // Always show labels on VC-mini for menu and page select
#endif
  LCD_fill_line_main_display(bottomlinemode, bottomline);
  LCD_main_set_label(bottomline);
}

void LCD_fill_line_main_display(uint8_t setting, String &line) {
  String msg;
  uint8_t l1, l2, l3;
  switch (setting) {
    case MD_SHOW_PAGE_NAME:
      EEPROM_read_title(Current_page, 0, Current_page_name); // Read page name from EEPROM
      line += Current_page_name;
      break;
    case MD_SHOW_CURRENT_PATCH:
      if (Current_device < NUMBER_OF_DEVICES) line += Device[Current_device]->current_patch_name;
      break;
    case MD_SHOW_PATCHES_COMBINED:
      LCD_set_combined_patch_name();
      line += Combined_patch_name; // Show the combined patchname
      break;
    case MD_SHOW_3_SWITCH_LABELS:
      l3 = (MAIN_LCD_DISPLAY_SIZE - 2) / 3;
      l2 = (MAIN_LCD_DISPLAY_SIZE - l3 - 2) / 2;
      l1 = MAIN_LCD_DISPLAY_SIZE - l3 - l2 - 2;
      LCD_load_short_message(1, msg);
      LCD_set_length(msg, l1);
      line += msg;
      line += '|';
      LCD_load_short_message(2, msg);
      LCD_set_length(msg, l2);
      line += msg;
      line += '|';
      LCD_load_short_message(3, msg);
      LCD_set_length(msg, l3);
      line += msg;
      break;
    case MD_SHOW_CURRENT_SCENE:
      l1 = Device[Current_device]->current_snapscene;
      if (l1 > 0) Device[Current_device]->get_snapscene_label(l1, line);
      break;
    case MD_SHOW_ALL_PATCH_NUMBERS:
      LCD_set_combined_patch_number();
      line = Current_patch_number_string;
      break;
    case MD_SHOW_PAGE_NUMBER:
      line += "PG " + String(Current_page);
      break;
    case MD_SHOW_PAGE_NUMBER_AND_NAME:
      line += "P" + String(Current_page) + ' ';
      EEPROM_read_title(Current_page, 0, Current_page_name); // Read page name from EEPROM
      line += Current_page_name;
      break;
    case MD_SHOW_SONG_NUMBER_AND_NAME:
      SCO_get_song_number_name(Current_song, line);
      line += ": ";
      SCO_get_song_name(Current_song, line);
      break;
    case MD_SHOW_PART_NUMBER_AND_NAME:
      SCO_get_part_number_name(Current_part, line);
      line += ": ";
      SCO_get_part_name(Current_part, line);
      break;
    case MD_SHOW_SETLIST_NUMBER_AND_NAME:
      line += "SETLIST " + String(Current_setlist) + ": ";
      SCO_get_setlist_name(Current_setlist, line);
      break;
    case MD_SHOW_SETLIST_TARGET_NAME:
      SCO_get_setlist_target_name(Current_setlist_target, line);
      break;
  }
}

void LCD_update_TFT_brightness() {
#ifdef IS_VCTOUCH
  if (TFT_current_brightness != Setting.Backlight_brightness) {
    TFT_current_brightness = Setting.Backlight_brightness;
    LCD_set_TFT_brightness(TFT_current_brightness);
  }
#endif
}

#ifdef IS_VCTOUCH
void LCD_set_TFT_brightness(uint8_t val) { // Scale the brightness logarithmically from 0 - 255 to 0 - 4095 (16 times larger)
  val++;
  uint16_t b = val * val / 16 + 1;
  analogWrite(MAIN_TFT_BACKLIGHT_PIN, b);
}
#endif

void LCD_show_page_name() { // Will temporary show the name of the current page
  if ((Setting.Main_display_top_line_mode == MD_SHOW_PAGE_NAME) || (Setting.Main_display_bottom_line_mode == MD_SHOW_PAGE_NAME)) return; // Do not show when display already shows the page name.
  //LCD_update_main_display();
  EEPROM_read_title(Current_page, 0, Current_page_name); // Read page name from EEPROM
  LCD_show_popup_label(Current_page_name, ACTION_TIMER_LENGTH);
}

bool LCD_check_main_display_show_patchname() {
  if (Setting.Main_display_top_line_mode == MD_SHOW_CURRENT_PATCH) return true;
  if (Setting.Main_display_bottom_line_mode == MD_SHOW_CURRENT_PATCH) return true;
  return false;
}

void LCD_set_combined_patch_number() {
  // Here we determine what to show on the main display, based on what devices are active
  uint8_t number_of_active_devices = 0;
  Current_patch_number_string = "";

  for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
    if (Device[d]->is_on) {
      Device[d]->display_patch_number_string(); // Adds the device name to patch number string
      Current_patch_number_string += PATCH_NUMBER_SEPERATOR; //Add a seperation sign in between
      number_of_active_devices++;
    }
  }

  if (number_of_active_devices == 0) { // So we see the patch number before the first device connects
    Device[Current_device]->display_patch_number_string();
    Current_patch_number_string += PATCH_NUMBER_SEPERATOR;
  }

  // Cut last character of patch number string
  uint8_t l = Current_patch_number_string.length();
  Current_patch_number_string.remove(l - 1);

  // Check if device is in bank selection
  if ((device_in_bank_selection > 0) && (device_in_bank_selection <= NUMBER_OF_DEVICES)) {
    Current_patch_number_string = "";
    Device[device_in_bank_selection - 1]->display_patch_number_string();
  }
}


void LCD_set_combined_patch_name() {
  // Here we determine what to show on the main display, based on what devices are active
  uint8_t number_of_active_devices = 0;
  String patch_names[NUMBER_OF_DEVICES]; //Array of strings for the patchnames

  for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
    if (Device[d]->is_on) {
      patch_names[number_of_active_devices] = Device[d]->current_patch_name; //Add patchname to string
      number_of_active_devices++;
    }
  }

  // Set combined patchname
  for (uint8_t i = 0; i < patch_names[0].length(); i++) Serial.print(String((uint8_t)patch_names[0][i]) + " ");
  switch (number_of_active_devices) {
    case 0:
      Combined_patch_name = "";
      break;
    case 1: // Only one device active
      Combined_patch_name = patch_names[0];
      break;
    case 2: // Show 7 bytes of both names
      LCD_shorten_string(7, patch_names[0]);
      LCD_shorten_string(7, patch_names[1]);
      Combined_patch_name = patch_names[0] + "  " + patch_names[1];
      break;
    case 3: // Show 4 bytes of both names
      LCD_shorten_string(4, patch_names[0]);
      LCD_shorten_string(4, patch_names[1]);
      LCD_shorten_string(4, patch_names[2]);
      Combined_patch_name = patch_names[0] + "  " + patch_names[1] + "  " + patch_names[2];
      break;
    default: // More then 3 devices
      Combined_patch_name = String(number_of_active_devices) + " devices on";
      break;
  }
}

void LCD_shorten_string(uint8_t len, String &txt) {
  if (txt.length() > len) txt = txt.substring(0, len);
}


void LCD_load_short_message(uint8_t sw, String & msg) {
  if (EEPROM_check4label(Current_page, sw)) {
    //String lbl;
    EEPROM_read_title(Current_page, sw, msg); // Override the label if a custom label exists
    LCD_set_SP_label(sw, msg); // So full label will popup on switch activation
    return;
  }

  msg = "";
  uint8_t Dev = SP[sw].Device;
  if (Dev == CURRENT) Dev = Current_device;
  if (Dev < NUMBER_OF_DEVICES) {
    switch (SP[sw].Type) {
      case PATCH:
        if ((SP[sw].Sel_type == SELECT) || (SP[sw].Sel_type == BANKSELECT)) {
          msg = SP[sw].Label;
          if (msg.trim() == "") { // Check if label is empty - then there is no patch name, so we will display the patch number
            msg = "";
            if (SP[sw].PP_number != NO_RESULT) Device[Dev]->number_format(SCO_get_patchnumber(Dev, SP[sw].PP_number), msg);
          }
        }
        if (SP[sw].Sel_type == NEXT) {
          msg = "NEXT  ";
        }
        if (SP[sw].Sel_type == PREV) {
          msg = "PREV ";
        }
        if (SP[sw].Sel_type == BANKUP) {
          msg = "BNK+ ";
        }
        if (SP[sw].Sel_type == BANKDOWN) {
          msg = "BNK- ";
        }
        break;
      case PAR_BANK_CATEGORY:
        msg = SP[sw].Label;
        if (msg.trim() == "") { // Check if label is empty
          msg = "";
          Device[Dev]->number_format(SP[sw].PP_number, msg);
        }
        break;
      case PAR_BANK_DOWN:
        msg = "BNK- ";
        break;
      case PAR_BANK_UP:
        msg = "BNK+ ";
        break;
      case DIRECT_SELECT:
        msg = "---  ";
        break;
      case PARAMETER:
      case PAR_BANK:
        //msg = SP[sw].Label;
        LCD_parameter_label_short(sw, Dev, msg);
        break;
      case ASSIGN:
        //msg = SP[sw].Label;
        if ((SP[sw].Sel_type == SELECT) || (SP[sw].Sel_type == BANKSELECT)) {
          Device[Dev]->read_assign_short_name(SP[sw].Assign_number, msg);
        }
        if (SP[sw].Sel_type == NEXT) {
          msg = "PG+  ";
        }
        if (SP[sw].Sel_type == PREV) {
          msg = "PG-  ";
        }
        if (SP[sw].Sel_type == BANKUP) {
          msg = "BNK+ ";
        }
        if (SP[sw].Sel_type == BANKDOWN) {
          msg = "BNK- ";
        }
        break;
      case OPEN_PAGE_DEVICE:
        //msg = Device[Dev]->device_name;
        msg = SP[sw].Label;
        break;
      case OPEN_NEXT_PAGE_OF_DEVICE:
        msg = "PGE+ ";
        break;
      case MUTE:
        msg = "MUTE ";
        break;
      case TOGGLE_EXP_PEDAL:
        msg = "M EXP";
        break;
      case SNAPSCENE:
        msg = '[';
        if (Dev < NUMBER_OF_DEVICES) Device[Dev]->get_snapscene_title_short(SP[sw].PP_number, msg);
        msg += "] ";
        break;
      case LOOPER:
        msg = SP[sw].Label;
        break;
      case SAVE_PATCH:
        msg = "SAVE ";
        break;
      default:
        //LCD_print_lcd_txt(sw);
        break;
    }
  }
  if (Dev == COMMON) {
    switch (SP[sw].Type) {
      case MENU:
        LCD_add_title(SP[sw].Title);
        LCD_add_label(SP[sw].Label);
        //LCD_print_lcd_txt(sw);
        break;
      case PAGE:
        if ((SP[sw].Sel_type == SELECT) || (SP[sw].Sel_type == BANKSELECT)) {
          msg = SP[sw].Label;
        }
        if (SP[sw].Sel_type == NEXT) {
          msg = "PG+  ";
        }
        if (SP[sw].Sel_type == PREV) {
          msg = "PG-  ";
        }
        if (SP[sw].Sel_type == BANKUP) {
          msg = "BNK+ ";
        }
        if (SP[sw].Sel_type == BANKDOWN) {
          msg = "BNK- ";
        }
        break;
      case GLOBAL_TUNER:
        msg = "TUNER";
        break;
      case TAP_TEMPO:
        msg = char(CHAR_QUARTER_NOTE) + String(Setting.Bpm);
        break;
      case SET_TEMPO:
        msg = char(CHAR_QUARTER_NOTE) + String(SP[sw].PP_number);
        break;
      case MIDI_PC:
        if ((SP[sw].Sel_type == SELECT) || (SP[sw].Sel_type == BANKSELECT)) {
          msg = 'P';
          LCD_add_3digit_number(SP[sw].PP_number, msg);
        }
        if (SP[sw].Sel_type == NEXT) {
          msg = "PC+  ";
        }
        if (SP[sw].Sel_type == PREV) {
          msg = "PC-  ";
        }
        break;
      case MIDI_CC:
        msg = 'C';
        LCD_add_3digit_number(SP[sw].PP_number, msg);
        break;
      case MIDI_NOTE:
        msg = 'N';
        LCD_add_3digit_number(SP[sw].PP_number, msg);
        break;
      case SELECT_NEXT_DEVICE:
        msg = Device[SCO_get_number_of_next_device()]->device_name;
        break;
      case SETLIST:
        if ((SP[sw].Sel_type == SL_SELECT) || (SP[sw].Sel_type == SL_BANKSELECT)) {
          msg = SP[sw].Label;
        }
        if (SP[sw].Sel_type == SL_NEXT) {
          msg = "SL+  ";
        }
        if (SP[sw].Sel_type == SL_PREV) {
          msg = "SL-  ";
        }
        if (SP[sw].Sel_type == SL_BANKUP) {
          msg = "BNK+ ";
        }
        if (SP[sw].Sel_type == SL_BANKDOWN) {
          msg = "BNK- ";
        }
        if (SP[sw].Sel_type == SL_EDIT) {
          msg = "EDIT ";
        }
        break;
      case SONG:
        if ((SP[sw].Sel_type == SONG_SELECT) || (SP[sw].Sel_type == SONG_BANKSELECT)) {
          msg = SP[sw].Label;
        }
        if (SP[sw].Sel_type == SONG_NEXT) {
          if (SP[sw].Trigger == SONG_PREVNEXT_SONG) msg = "SNG+  ";
          if (SP[sw].Trigger == SONG_PREVNEXT_PART) msg = "PRT+  ";
          if (SP[sw].Trigger == SONG_PREVNEXT_SONGPART) msg = "SP+  ";
        }
        if (SP[sw].Sel_type == SONG_PREV) {
          if (SP[sw].Trigger == SONG_PREVNEXT_SONG) msg = "SNG-  ";
          if (SP[sw].Trigger == SONG_PREVNEXT_PART) msg = "PRT-  ";
          if (SP[sw].Trigger == SONG_PREVNEXT_SONGPART) msg = "SP-  ";
        }
        if (SP[sw].Sel_type == SONG_BANKUP) {
          msg = "BNK+ ";
        }
        if (SP[sw].Sel_type == SONG_BANKDOWN) {
          msg = "BNK- ";
        }
        if (SP[sw].Sel_type == SONG_PARTSEL) {
          msg = SP[sw].Label;
        }
        if (SP[sw].Sel_type == SONG_EDIT) {
          msg = "EDIT ";
        }
        break;
      case MODE:
        if (SP[sw].PP_number == SONG_MODE) msg = "SONG";
        if (SP[sw].PP_number == PAGE_MODE) msg = "PAGE";
        if (SP[sw].PP_number == DEVICE_MODE) msg = "DEV";
        break;
      case MIDI_MORE:
        if (SP[sw].PP_number == MIDI_START) msg = "START";
        if (SP[sw].PP_number == MIDI_STOP) msg = "STOP";
        if (SP[sw].PP_number == MIDI_START_STOP) {
          if (MIDI_get_start_stop_state(SP[sw].Value1)) msg = "STOP";
          else msg = "START";
        }
        break;
    }
  }
}

void LCD_set_length(String & msg, uint8_t length) {
  uint8_t strlength = msg.trim().length();
  if (strlength < length) { // Add spaces if it is too short
    for (uint8_t i = strlength; i < length; i++) msg += ' ';
  }
  else { // Cut if not
    msg = msg.substring(0, length);
  }
}

void LCD_show_popup_label(String message, uint16_t time)
// Will display a status message on the main display
{
  if (on_screen_keyboard_active) return;
  if (pong_active) return;
  LCD_main_set_label(message);
  LCD_print_main_lcd_txt();
  if (SC_switch_is_expr_pedal()) messageTimer = millis() + LEDBAR_TIMER_LENGTH;
  else messageTimer = millis() + time;
  popup_label_showing = true;
}

void LCD_show_popup_title(String message, uint16_t time)
// Will display a status message on the main display
{
  if (on_screen_keyboard_active) return;
  if (pong_active) return;
  LCD_main_set_title(message);
  LCD_print_main_lcd_txt();
  if (SC_switch_is_expr_pedal()) messageTimer = millis() + LEDBAR_TIMER_LENGTH;
  else messageTimer = millis() + time;
  popup_title_showing = true;
}

void LCD_show_startup_message() {
  LCD_clear_main_lcd_txt();
  String msg = VC_NAME;
#ifndef IS_VCTOUCH
  LCD_center_string(msg);
#endif
  LCD_main_set_title(msg);  // Show startup message
  String msg1 = "version " + String(VCONTROLLER_FIRMWARE_VERSION_MAJOR) + "." + String(VCONTROLLER_FIRMWARE_VERSION_MINOR) + "." + String(VCONTROLLER_FIRMWARE_VERSION_BUILD);
#ifndef IS_VCTOUCH
  LCD_center_string(msg1);
#endif
  LCD_show_popup_label(msg1, MESSAGE_TIMER_LENGTH);
#ifdef IS_VCTOUCH
  /*for (uint8_t b = 0; b <= TFT_current_brightness; b++) {
    LCD_set_TFT_brightness(b);
    delay((255 - b) >> 4);
    }*/
  LCD_set_TFT_brightness(TFT_current_brightness);
#endif
  delay(800);
  String msg2 = "by SixEight"; //Please give me the credits :-)
#ifndef IS_VCTOUCH
  LCD_center_string(msg2);
#endif
  LCD_show_popup_label(msg2, MESSAGE_TIMER_LENGTH);
  delay(800);
}

void LCD_clear_main_lcd_txt() {
  for (uint8_t i = 0; i < MAIN_LCD_DISPLAY_SIZE; i++) {
    main_lcd_title[i] = ' ';
    main_lcd_label[i] = ' ';
  }
}

void LCD_center_string(String & msg) {
  uint8_t msg_length = msg.length();
  if (msg_length >= MAIN_LCD_DISPLAY_SIZE) return;
  uint8_t spaces_right = (MAIN_LCD_DISPLAY_SIZE - msg_length) / 2;
  uint8_t spaces_left = MAIN_LCD_DISPLAY_SIZE - spaces_right - msg_length;
  String newmsg = "";
  for (uint8_t s = 0; s < spaces_left; s++) newmsg += ' ';
  newmsg += msg;
  for (uint8_t s = 0; s < spaces_right; s++) newmsg += ' ';
  msg = newmsg;
}

void LCD_main_set_title(const String & msg) { // Will set the Title string in the SP array
  // Check length does not exceed LABEL_SIZE
  uint8_t msg_length = msg.length();
  if (msg_length > MAIN_LCD_DISPLAY_SIZE) msg_length = MAIN_LCD_DISPLAY_SIZE;
  while ((msg_length > 0) && (msg[msg_length - 1] == ' ')) msg_length--; // Remove spaces at the end
#ifdef IS_VCTOUCH // Center the string
  uint8_t leftlen = (MAIN_LCD_DISPLAY_SIZE - msg_length) / 2;
  for (uint8_t i = 0; i < leftlen; i++) { // Fill the remaining chars with spaces
    main_lcd_title[i] = ' ';
  }
  for (uint8_t i = 0; i < msg_length; i++) {
    main_lcd_title[i + leftlen] = msg[i];
  }
  for (uint8_t i = (msg_length + leftlen); i < MAIN_LCD_DISPLAY_SIZE; i++) { // Fill the remaining chars with spaces
    main_lcd_title[i] = ' ';
  }
#else
  for (uint8_t i = 0; i < msg_length; i++) {
    main_lcd_title[i] = msg[i];
  }
  for (uint8_t i = msg_length; i < MAIN_LCD_DISPLAY_SIZE; i++) { // Fill the remaining chars with spaces
    main_lcd_title[i] = ' ';
  }
#endif
}

void LCD_main_set_label(const String & msg) { // Will set the Title string in the SP array
  // Check length does not exceed LABEL_SIZE
  uint8_t msg_length = msg.length();
  if (msg_length > MAIN_LCD_DISPLAY_SIZE) msg_length = MAIN_LCD_DISPLAY_SIZE;
  while ((msg_length > 0) && (msg[msg_length - 1] == ' ')) msg_length--; // Remove spaces at the end
#ifdef IS_VCTOUCH // Center the string
  uint8_t leftlen = (MAIN_LCD_DISPLAY_SIZE - msg_length) / 2;
  for (uint8_t i = 0; i < leftlen; i++) { // Fill the remaining chars with spaces
    main_lcd_label[i] = ' ';
  }
  for (uint8_t i = 0; i < msg_length; i++) {
    main_lcd_label[i + leftlen] = msg[i];
  }
  for (uint8_t i = (msg_length + leftlen); i < MAIN_LCD_DISPLAY_SIZE; i++) { // Fill the remaining chars with spaces
    main_lcd_label[i] = ' ';
  }
#else
  for (uint8_t i = 0; i < msg_length; i++) {
    main_lcd_label[i] = msg[i];
  }
  for (uint8_t i = msg_length; i < MAIN_LCD_DISPLAY_SIZE; i++) { // Fill the remaining chars with spaces
    main_lcd_label[i] = ' ';
  }
#endif
}

void LCD_main_set_label_right(const String & msg) { // Will set the Title string in the SP array
  // Check length does not exceed LABEL_SIZE
  //String msg = msgi;
  //msg.trim();
  uint8_t msg_length = msg.length();
  if (msg_length > MAIN_LCD_DISPLAY_SIZE) msg_length = MAIN_LCD_DISPLAY_SIZE;
  if (msg_length > 1) while (msg[msg_length - 1] == ' ') msg_length--; // Do not count any spaces on the right of the string.
  for (uint8_t i = 0; i < (MAIN_LCD_DISPLAY_SIZE - msg_length); i++) { // Fill the left chars with spaces
    main_lcd_label[i] = ' ';
  }
  for (uint8_t i = 0; i < msg_length; i++) { // Put the string on the right
    main_lcd_label[(MAIN_LCD_DISPLAY_SIZE - msg_length) + i] = msg[i];
  }
}

void LCD_print_main_lcd_txt() {
  bool line1_changed = false;
  bool line2_changed = false;
  if (!main_ledbar_showing) { // otherwise ledbar will not show
    line1_changed = LCD_print_delta(0, 0, main_lcd_title);
  }
  line2_changed = LCD_print_delta(0, 1, main_lcd_label);
  if (line1_changed || line2_changed)
    MIDI_remote_update_display(0, main_lcd_title, main_lcd_label); // Show the data on the remote display
}

// ********************************* Section 3: Update of individual LCDs ********************************************

// Define my strings
const char LCD_Bank_Down[] = "<BANK DOWN>";
const char LCD_Bank_Up[] = "<BANK UP>";
const char LCD_Mute[] = "[MUTE]";
const char LCD_Tuner[] = "[GLOBAL TUNER]";
const char LCD_Tap_Tempo[] = "<TAP TEMPO>";
const char LCD_Set_Tempo[] = "<SET TEMPO>";
const char LCD_Page_Down[] = "<PAGE DOWN>";
const char LCD_Page_Up[] = "<PAGE UP>";
const char LCD_Looper[] = "<LOOPER>";
const char LCD_Page[] = VC_NAME;
const char LCD_Unknown[] = "Unknown";
const char LCD_Next_Device[] = "<NEXT DEVICE>";
const char LCD_Next_Setlist[] = "<NEXT SETLIST>";
const char LCD_Prev_Setlist[] = "<PREV SETLIST>";
const char LCD_Next_Song[] = "<NEXT SONG>";
const char LCD_Prev_Song[] = "<PREV SONG>";
const char LCD_Next_Part[] = "<NEXT PART>";
const char LCD_Prev_Part[] = "<PREV PART>";
const char LCD_Next_SongPart[] = "<NEXT PART/SONG>";
const char LCD_Prev_SongPart[] = "<PREV PART/SONG>";
const char LCD_Setlist[] = "<SETLIST>";
const char LCD_Song[] = "<SONG>";
const char LCD_Edit[] = "EDIT";
const char LCD_Song_Mode[] = "SONG MODE";
const char LCD_Page_Mode[] = "PAGE MODE";
const char LCD_Device_Mode[] = "DEVICE MODE";
const char LCD_Start[] = "START";
const char LCD_End[] = "END";


void LCD_update(uint8_t sw, bool do_show) {
  if (on_screen_keyboard_active) return;
  if (pong_active) return;

  // Will update a specific display based on the data in the SP array
  if (sw == 0) return;
#ifdef VCMINI_DISPLAYS
  if (update_lcd <= NUMBER_OF_DISPLAYS) update_main_lcd = true;
  return;
#endif
  if (sw > NUMBER_OF_DISPLAYS) return; // Check if there is a display for this switch

  //Determine what to display from the Switch type
  DEBUGMSG("Update display no:" + String(sw));

  LCD_clear_lcd_txt();
  Display_number_string = "";

  if (EEPROM_check4label(Current_page, sw)) {
    String msg;
    EEPROM_read_title(Current_page, sw, msg); // Override the label if a custom label exists
    LCD_set_SP_label(sw, msg);
  }

  uint8_t Dev = SP[sw].Device;
  if (Dev == CURRENT) Dev = Current_device;

  if (Dev < NUMBER_OF_DEVICES) {
    switch (SP[sw].Type) {
      case PATCH:
        if ((SP[sw].Sel_type == SELECT) || (SP[sw].Sel_type == BANKSELECT)) {
          Display_number_string = "";
          if (SP[sw].PP_number != NO_RESULT) {
            if ((SCO_setlist_active(Dev + SETLIST_TARGET_FIRST_DEVICE)) && (SP[sw].Sel_type == BANKSELECT)) {
              LCD_add_2digit_number(SP[sw].PP_number + 1, Display_number_string);
              Display_number_string += ':';
              if (SP[sw].PP_number == NEW_PATCH) Display_number_string += "NEW PATCH";
              else Device[Dev]->setlist_song_short_item_format(SCO_read_setlist_item(SP[sw].PP_number), Display_number_string);
            }
            else {
              Device[Dev]->number_format(SP[sw].PP_number, Display_number_string);
            }
          }
          if (Device[Dev]->patch_number_in_current_setlist(SP[sw].PP_number) == Device[Dev]->patch_number) {
            if (Setting.CURNUM_action == CN_PREV_PATCH) {
              Display_number_string += " (";
              Device[Dev]->number_format(Device[Dev]->prev_patch_number, Display_number_string);
              Display_number_string += ')';
            }
            if (Setting.CURNUM_action == CN_TAP_TEMPO) {
              Display_number_string += " (Tap)";
            }
            if (Setting.CURNUM_action == CN_GLOBAL_TUNER) {
              Display_number_string += " (Tune)";
            }
            if (Setting.CURNUM_action == CN_DIRECT_SELECT) {
              Display_number_string += " (Sel)";
            }
          }
          LCD_add_vled(3);
          LCD_add_title(Display_number_string);
          LCD_add_label(SP[sw].Label);
        }
        if (SP[sw].Sel_type == NEXT) {
          Display_number_string = "NEXT (";
          if (SCO_setlist_active(Dev + SETLIST_TARGET_FIRST_DEVICE)) {
            if (SP[sw].PP_number == Device[Dev]->get_patch_max()) Display_number_string = LCD_End;
            else Device[Dev]->setlist_song_short_item_format(SP[sw].PP_number, Display_number_string);
          }
          else Device[Dev]->number_format(SP[sw].PP_number, Display_number_string);
          Display_number_string += ')';
          LCD_add_vled(2);
          LCD_add_title(Display_number_string);
          LCD_add_label(SP[sw].Label);
        }
        if (SP[sw].Sel_type == PREV) {
          Display_number_string = "PREV (";
          if (SCO_setlist_active(Dev + SETLIST_TARGET_FIRST_DEVICE)) {
            if (SP[sw].PP_number == Device[Dev]->get_patch_min()) Display_number_string = LCD_Start;
            else Device[Dev]->setlist_song_short_item_format(SP[sw].PP_number, Display_number_string);
          }
          else Device[Dev]->number_format(SP[sw].PP_number, Display_number_string);
          Display_number_string += ')';
          LCD_add_vled(2);
          LCD_add_title(Display_number_string);
          LCD_add_label(SP[sw].Label);
        }
        if (SP[sw].Sel_type == BANKUP) {
          LCD_add_vled(2);
          LCD_add_title(LCD_Bank_Up);
          LCD_add_label(Device[Dev]->device_name);
        }
        if (SP[sw].Sel_type == BANKDOWN) {
          LCD_add_vled(2);
          LCD_add_title(LCD_Bank_Down);
          LCD_add_label(Device[Dev]->device_name);
        }
        break;
      case PAR_BANK_DOWN:
        LCD_add_vled(2);
        LCD_add_title(LCD_Bank_Down);
        Display_number_string = "PAR ";
        Display_number_string += Device[Dev]->device_name;
        LCD_add_label(Display_number_string);
        //LCD_print_lcd_txt(sw);
        break;
      case PAR_BANK_UP:
        LCD_add_vled(2);
        LCD_add_title(LCD_Bank_Up);
        Display_number_string = "PAR ";
        Display_number_string += Device[Dev]->device_name;
        LCD_add_label(Display_number_string);
        //LCD_print_lcd_txt(sw);
        break;
      case DIRECT_SELECT:
        if (Device[Dev]->valid_direct_select_switch(SP[sw].PP_number)) {
          Display_number_string = ""; //Clear the display_number_string
          Device[Dev]->direct_select_format(SP[sw].PP_number, Display_number_string);
          LCD_add_vled(3);
          LCD_add_title(Display_number_string);
          LCD_add_label(SP[sw].Label);
        }
        //LCD_print_lcd_txt(sw);
        break;
      case PAR_BANK_CATEGORY:
        Display_number_string = '<';
        Display_number_string += Device[Dev]->device_name;
        Display_number_string += " edit>";
        LCD_add_vled(3);
        LCD_add_title(Display_number_string);
        LCD_add_label(SP[sw].Label);
        //LCD_print_lcd_txt(sw);
        break;
      case PARAMETER:
      case PAR_BANK:
        Display_number_string = "";
        LCD_parameter_title(sw, Dev, Display_number_string);
        LCD_add_vled(3);
        LCD_add_title(Display_number_string);
        Display_number_string = "";
        LCD_parameter_label(sw, Dev, Display_number_string);
        LCD_add_label(Display_number_string);
        //LCD_print_lcd_txt(sw);
        break;
      case ASSIGN:
        if ((SP[sw].Sel_type == SELECT) || (SP[sw].Sel_type == BANKSELECT)) {
          if (SP[sw].Latch == TOGGLE) Display_number_string = '[';
          else Display_number_string = '<';
          Device[Dev]->read_assign_name(SP[sw].Assign_number, Display_number_string);
          if (SP[sw].Latch == TOGGLE) Display_number_string += ']';
          else Display_number_string += '>';
          LCD_add_vled(3);
          LCD_add_title(Display_number_string);
          LCD_add_label(SP[sw].Label);
        }
        if (SP[sw].Sel_type == NEXT) {
          LCD_add_vled(1);
          LCD_add_title(LCD_Unknown);
          LCD_add_label(Device[Dev]->device_name);
        }
        if (SP[sw].Sel_type == PREV) {
          LCD_add_vled(1);
          LCD_add_title(LCD_Unknown);
          LCD_add_label(Device[Dev]->device_name);
        }
        if (SP[sw].Sel_type == BANKUP) {
          LCD_add_vled(2);
          LCD_add_title(LCD_Bank_Up);
          LCD_add_label(Device[Dev]->device_name);
        }
        if (SP[sw].Sel_type == BANKDOWN) {
          LCD_add_vled(2);
          LCD_add_title(LCD_Bank_Down);
          LCD_add_label(Device[Dev]->device_name);
        }
        break;
      case OPEN_PAGE_DEVICE:
      case OPEN_NEXT_PAGE_OF_DEVICE:
        LCD_add_vled(3);
        LCD_add_title(Device[Dev]->device_name);
        LCD_add_label(SP[sw].Label);
        //LCD_print_lcd_txt(sw);
        break;
      case MUTE:
        LCD_add_vled(3);
        LCD_add_title(LCD_Mute);
        LCD_add_label(Device[Dev]->device_name);
        //LCD_print_lcd_txt(sw);
        break;
      case TOGGLE_EXP_PEDAL:
        LCD_add_vled(3);
        LCD_add_title(SP[sw].Title);
        LCD_add_label(SP[sw].Label);
        //LCD_print_lcd_txt(sw);
        break;
      case SNAPSCENE:
        LCD_add_vled(3);
        Display_number_string = "";
        if (SP[sw].Value2 == 0) { // Single snapshot in view
          Display_number_string += "    ";
          Device[Dev]->get_snapscene_title(SP[sw].PP_number, Display_number_string);
          Display_number_string += "     ";
        }
        else if (SP[sw].Value3 == 0) { // Two snapshots in view
          Display_number_string += "  ";
          LCD_add_snapshot_number(Dev, SP[sw].Value1, SP[sw].PP_number, Device[Dev]->current_snapscene, Display_number_string);
          Display_number_string += "    ";
          LCD_add_snapshot_number(Dev, SP[sw].Value2, SP[sw].PP_number, Device[Dev]->current_snapscene, Display_number_string);
          Display_number_string += "  ";
        }
        else { // Three snapshots in view
          LCD_add_snapshot_number(Dev, SP[sw].Value1, SP[sw].PP_number, Device[Dev]->current_snapscene, Display_number_string);
          Display_number_string += "  ";
          LCD_add_snapshot_number(Dev, SP[sw].Value2, SP[sw].PP_number, Device[Dev]->current_snapscene, Display_number_string);
          Display_number_string += "  ";
          LCD_add_snapshot_number(Dev, SP[sw].Value3, SP[sw].PP_number, Device[Dev]->current_snapscene, Display_number_string);
        }
        LCD_set_title(Display_number_string);
        strcpy(lcd_label, SP[sw].Label);
        break;
      case LOOPER:
        LCD_set_looper_title();
        LCD_add_label(SP[sw].Label);
        //LCD_print_lcd_txt(sw);
        break;
      case SAVE_PATCH:
        if ((Dev == SY1000) || (Dev == GR55)) LCD_add_title("<SCENE MENU>");
        else LCD_add_title("<SAVE PATCH>");
        LCD_add_label(Device[Dev]->device_name);
        //LCD_print_lcd_txt(sw);
        break;
      default:
        //LCD_print_lcd_txt(sw);
        break;
    }
  }
  if (Dev == COMMON) {
    switch (SP[sw].Type) {
      case MENU:
        LCD_add_title(SP[sw].Title);
        LCD_add_label(SP[sw].Label);
        //LCD_print_lcd_txt(sw);
        break;
      case PAGE:
        if (SP[sw].Sel_type == SELECT) {
          LCD_add_vled(3);
          LCD_add_title(SP[sw].Label);
        }
        if (SP[sw].Sel_type == NEXT) {
          LCD_add_vled(1);
          LCD_add_title(LCD_Page_Up);
          if (Current_page_setlist_item == SCO_get_page_max()) LCD_add_label(LCD_End);
          else LCD_add_label(SP[sw].Label);
        }
        if (SP[sw].Sel_type == PREV) {
          LCD_add_vled(1);
          LCD_add_title(LCD_Page_Down);
          if (Current_page_setlist_item == SCO_get_page_min()) LCD_add_label(LCD_Start);
          else LCD_add_label(SP[sw].Label);
        }
        if (SP[sw].Sel_type == BANKSELECT) {
          Display_number_string = "PG ";
          if (SCO_get_page_number(SP[sw].PP_number) > 0) {
            LCD_add_3digit_number(SCO_get_page_number(SP[sw].PP_number), Display_number_string);
            LCD_add_vled(3);
            LCD_add_title(Display_number_string);
            LCD_add_label(SP[sw].Label);
          }
        }
        if (SP[sw].Sel_type == BANKUP) {
          LCD_add_vled(2);
          LCD_add_title(LCD_Bank_Up);
          LCD_add_label(LCD_Page);
        }
        if (SP[sw].Sel_type == BANKDOWN) {
          LCD_add_vled(2);
          LCD_add_title(LCD_Bank_Down);
          LCD_add_label(LCD_Page);
        }
        break;
      case GLOBAL_TUNER:
        LCD_add_vled(1);
        LCD_add_title(LCD_Tuner);
        //LCD_print_lcd_txt(sw);
        break;
      case TAP_TEMPO:
        LCD_add_vled(1);
        LCD_add_title(LCD_Tap_Tempo);
        Display_number_string = String(Setting.Bpm);
        Display_number_string += " BPM";
        LCD_add_label(Display_number_string);
        //LCD_print_lcd_txt(sw);
        break;
      case SET_TEMPO:
        LCD_add_vled(1);
        LCD_add_title(LCD_Set_Tempo);
        Display_number_string = String(SP[sw].PP_number);
        Display_number_string += " BPM";
        LCD_add_label(Display_number_string);
        //LCD_print_lcd_txt(sw);
        break;
      case MIDI_PC:
        if ((SP[sw].Sel_type == SELECT) || (SP[sw].Sel_type == BANKSELECT)) {
          Display_number_string = "<PC ";
          LCD_add_3digit_number(SP[sw].PP_number, Display_number_string);
          Display_number_string += '>';
          LCD_add_vled(3);
          LCD_add_title(Display_number_string);
          LCD_add_label(SP[sw].Label);
        }
        if (SP[sw].Sel_type == NEXT) {
          Display_number_string = "NEXT (PC ";
          LCD_add_3digit_number(SP[sw].PP_number, Display_number_string);
          Display_number_string += ')';
          LCD_add_vled(2);
          LCD_add_title(Display_number_string);
          LCD_add_label(SP[sw].Label);
        }
        if (SP[sw].Sel_type == PREV) {
          Display_number_string = "PREV (PC ";
          LCD_add_3digit_number(SP[sw].PP_number, Display_number_string);
          Display_number_string += ')';
          LCD_add_vled(2);
          LCD_add_title(Display_number_string);
          LCD_add_label(SP[sw].Label);
        }
        if (SP[sw].Sel_type == BANKUP) {
          LCD_add_vled(2);
          LCD_add_title(LCD_Bank_Up);
          //LCD_add_label(Device[Dev]->device_name);
        }
        if (SP[sw].Sel_type == BANKDOWN) {
          LCD_add_vled(2);
          LCD_add_title(LCD_Bank_Down);
          //LCD_add_label(Device[Dev]->device_name);
        }
        break;
      case MIDI_CC:
        Display_number_string = "";
        LCD_CC_title(sw, Display_number_string);
        LCD_add_vled(3);
        LCD_add_title(Display_number_string);
        if (SP[sw].Latch == CC_UPDOWN) {
          Display_number_string = "";
          LCD_add_3digit_number(SP[sw].Target_byte1, Display_number_string);
          LCD_add_label(Display_number_string);
        }
        else LCD_add_label(SP[sw].Label);
        //LCD_print_lcd_txt(sw);
        break;
      case MIDI_NOTE:
        Display_number_string = "<NOTE ";
        LCD_add_3digit_number(SP[sw].PP_number, Display_number_string);
        Display_number_string += '>';
        LCD_add_vled(3);
        LCD_add_title(Display_number_string);
        LCD_add_label(SP[sw].Label);
        //LCD_print_lcd_txt(sw);
        break;
      case SELECT_NEXT_DEVICE:
        LCD_add_vled(1);
        LCD_add_title(LCD_Next_Device);
        LCD_add_label(Device[SCO_get_number_of_next_device()]->device_name);
        break;
      case SETLIST:
        if ((SP[sw].Sel_type == SL_SELECT) || (SP[sw].Sel_type == SL_BANKSELECT)) {
          Display_number_string = "SL ";
          LCD_add_3digit_number(SP[sw].PP_number, Display_number_string);
          LCD_add_vled(3);
          LCD_add_title(Display_number_string);
          LCD_add_label(SP[sw].Label);
        }
        if (SP[sw].Sel_type == SL_NEXT) {
          LCD_add_vled(1);
          LCD_add_title(LCD_Next_Setlist);
          LCD_add_label(SP[sw].Label);
        }
        if (SP[sw].Sel_type == SL_PREV) {
          LCD_add_vled(1);
          LCD_add_title(LCD_Prev_Setlist);
          LCD_add_label(SP[sw].Label);
        }
        if (SP[sw].Sel_type == SL_BANKUP) {
          LCD_add_vled(2);
          LCD_add_title(LCD_Bank_Up);
          LCD_add_label(LCD_Setlist);
        }
        if (SP[sw].Sel_type == SL_BANKDOWN) {
          LCD_add_vled(2);
          LCD_add_title(LCD_Bank_Down);
          LCD_add_label(LCD_Setlist);
        }
        if (SP[sw].Sel_type == SL_EDIT) {
          LCD_add_vled(2);
          LCD_add_title(LCD_Edit);
          LCD_add_label(LCD_Setlist);
        }
        break;
      case SONG:
        if ((SP[sw].Sel_type == SONG_SELECT) || (SP[sw].Sel_type == SONG_BANKSELECT)) {
          if ((SCO_setlist_active(SETLIST_TARGET_SONG)) && (SP[sw].Sel_type == BANKSELECT)) {
            LCD_add_2digit_number(SP[sw].PP_number + 1, Display_number_string);
            Display_number_string += ':';
          }
          SCO_get_song_number_name(SCO_get_song_number(SP[sw].PP_number), Display_number_string);
          LCD_add_vled(3);
          LCD_add_title(Display_number_string);
          LCD_add_label(SP[sw].Label);
        }
        if (SP[sw].Sel_type == SONG_NEXT) {
          LCD_add_vled(1);
          if (SP[sw].Trigger == SONG_PREVNEXT_SONG) LCD_add_title(LCD_Next_Song);
          if (SP[sw].Trigger == SONG_PREVNEXT_PART) LCD_add_title(LCD_Next_Part);
          if (SP[sw].Trigger == SONG_PREVNEXT_SONGPART) LCD_add_title(LCD_Next_SongPart);
          LCD_add_label(SP[sw].Label);
        }
        if (SP[sw].Sel_type == SONG_PREV) {
          LCD_add_vled(1);
          if (SP[sw].Trigger == SONG_PREVNEXT_SONG) LCD_add_title(LCD_Prev_Song);
          if (SP[sw].Trigger == SONG_PREVNEXT_PART) LCD_add_title(LCD_Prev_Part);
          if (SP[sw].Trigger == SONG_PREVNEXT_SONGPART) LCD_add_title(LCD_Prev_SongPart);
          LCD_add_label(SP[sw].Label);
        }
        if (SP[sw].Sel_type == SONG_BANKUP) {
          LCD_add_vled(2);
          LCD_add_title(LCD_Bank_Up);
          LCD_add_label(LCD_Song);
        }
        if (SP[sw].Sel_type == SONG_BANKDOWN) {
          LCD_add_vled(2);
          LCD_add_title(LCD_Bank_Down);
          LCD_add_label(LCD_Song);
        }
        if (SP[sw].Sel_type == SONG_PARTSEL) {
          SCO_get_part_number_name(SP[sw].PP_number, Display_number_string);
          LCD_add_vled(2);
          LCD_add_title(Display_number_string);
          LCD_add_label(SP[sw].Label);
        }
        if (SP[sw].Sel_type == SONG_EDIT) {
          LCD_add_vled(2);
          LCD_add_title(LCD_Edit);
          LCD_add_label(LCD_Song);
        }
        break;
      case MODE:
        if (SP[sw].PP_number == SONG_MODE) {
          LCD_add_title(LCD_Song_Mode);
          if (Current_mode == SONG_MODE) LCD_add_label(LCD_Edit);
        }
        if (SP[sw].PP_number == PAGE_MODE) LCD_add_title(LCD_Page_Mode);
        if (SP[sw].PP_number == DEVICE_MODE) LCD_add_title(LCD_Device_Mode);
        break;
      case MIDI_MORE:
        if (SP[sw].PP_number == MIDI_START) LCD_add_title("START");
        if (SP[sw].PP_number == MIDI_STOP) LCD_add_title("STOP");
        if (SP[sw].PP_number == MIDI_START_STOP) {
          if (MIDI_get_start_stop_state(SP[sw].Value1) == true) LCD_add_title("[START] STOP ");
          else LCD_add_title(" START [STOP]");
        }
        LCD_add_label(SP[sw].Label);

        break;
      default:
        //LCD_print_lcd_txt(sw);
        break;
    }
  }
  if (do_show) LCD_print_lcd_txt(sw);
}

void LCD_parameter_label_short(uint8_t sw, uint8_t Dev, String & msg) { // Will print the right parameter message depending on the TOGGLE state
  if (Dev < NUMBER_OF_DEVICES) {
    switch (SP[sw].Latch) {
      case UPDOWN:
        if (SP[sw].Direction) msg += char(CHAR_ARROW_UP);
        else msg += char(CHAR_ARROW_DOWN);
        Device[Dev]->read_parameter_title(SP[sw].PP_number, msg);
        break;
      case MOMENTARY:
        msg += '<';
        Device[Dev]->read_parameter_title(SP[sw].PP_number, msg);
        break;
      case TGL_OFF:
        msg += " --  ";
        break;
      default:
        Device[Dev]->read_parameter_title_short(SP[sw].PP_number, msg);
        break;
    }
  }
}

void LCD_add_snapshot_number(uint8_t Dev, uint8_t number, uint8_t current_number, uint8_t current_snap, String & msg) {
  if (number == current_snap) msg += '[';
  else if (number == current_number) msg += '<';
  else msg += ' ';
  if (Dev < NUMBER_OF_DEVICES) Device[Dev]->get_snapscene_title_short(number, msg);
  if (number == current_snap) msg += ']';
  else if (number == current_number) msg += '>';
  else msg += ' ';
}

void LCD_parameter_title(uint8_t sw, uint8_t Dev, String & msg) { // Will print the right parameter message depending on the TOGGLE state
  if (Dev < NUMBER_OF_DEVICES) {
    switch (SP[sw].Latch) {
      case TOGGLE:
        msg += '[';
        Device[Dev]->read_parameter_title(SP[sw].PP_number, msg);
        msg += ']';
        break;
      case MOMENTARY:
      case ONE_SHOT:
        msg += '<';
        Device[Dev]->read_parameter_title(SP[sw].PP_number, msg);
        msg += '>';
        break;
      case TGL_OFF:
        msg += "--";
        break;
      default:
        Device[Dev]->read_parameter_title(SP[sw].PP_number, msg);
        break;
    }
  }
}

void LCD_parameter_label(uint8_t sw, uint8_t Dev, String & msg) { // Will print the right parameter message depending on the TOGGLE state
  if (Dev < NUMBER_OF_DEVICES) {
    String lbl_trimmed = SP[sw].Label;
    uint8_t step;
    lbl_trimmed.trim();
    switch (SP[sw].Latch) {
      case TRISTATE:
        msg = lbl_trimmed;
        msg += " (";
        msg += String(SP[sw].State);
        msg += "/3)";
        break;
      case FOURSTATE:
        msg = lbl_trimmed;
        msg += " (";
        msg += String(SP[sw].State);
        msg += "/4)";
        break;
      case TGL_OFF:
        break;
      case UPDOWN:
        if (SP[sw].Direction) msg += char(CHAR_ARROW_UP);
        else msg += char(CHAR_ARROW_DOWN);
        msg += ' ';
        msg += lbl_trimmed;
        msg += ' ';
        if (SP[sw].Direction) msg += char(CHAR_ARROW_UP);
        else msg += char(CHAR_ARROW_DOWN);
        break;
      case STEP:
        step = SP[sw].Value3;
        if (step == 0) step = 1;
        msg = lbl_trimmed;
        msg += " (";
        msg += String((SP[sw].Target_byte1 / step) + 1);
        msg += '/';
        msg += String(((SP[sw].Assign_max - 1) / step) + 1);
        msg += ')';
        break;
      case RANGE:
        msg = lbl_trimmed;
        msg += " (";
        msg += String(SP[sw].Target_byte1 + 1);
        msg += '/';
        msg += String(SP[sw].Assign_max);
        msg += ')';
        break;
      default:
        msg = lbl_trimmed;
        break;
    }
  }
}

void LCD_CC_title(uint8_t sw, String & msg) { // Will print the right parameter message depending on the TOGGLE state
  switch (SP[sw].Latch) {
    case CC_ONE_SHOT:
    case CC_MOMENTARY:
      msg += "<CC #";
      LCD_add_3digit_number(SP[sw].PP_number, msg);
      msg += '>';
      break;
    case CC_TOGGLE:
    case CC_TOGGLE_ON:
      msg += "[CC #";
      LCD_add_3digit_number(SP[sw].PP_number, msg);
      msg += ']';
      break;
    case CC_STEP:
      msg += "CC #";
      LCD_add_3digit_number(SP[sw].PP_number, msg);
      msg += " (";
      msg += String(SP[sw].Target_byte1);
      msg += '/';
      msg += String(SP[sw].Assign_max);
      msg += ')';
      break;
    case CC_UPDOWN:
      if (SP[sw].Direction) msg += char(CHAR_ARROW_UP);
      else msg += char(CHAR_ARROW_DOWN);
      msg += " CC #";
      LCD_add_3digit_number(SP[sw].PP_number, msg);
      msg += ' ';
      if (SP[sw].Direction) msg += char(CHAR_ARROW_UP);
      else msg += char(CHAR_ARROW_DOWN);
      //DEBUGMSG("LCD check - direction: " + String(SP[sw].Direction));
      break;
  }
}

void LCD_set_looper_title() {
  LCD_clear_lcd_title();
  LCD_add_vled(3);
  LCD_add_title(LCD_Looper);
}

// ********************************* Section 4: LCD Functions ********************************************

void LCD_add_char_to_string(String ch, String &str, uint8_t len) {
  uint8_t my_length = ch.length();
  if (my_length > len) my_length = len;
  while ((my_length > 1) && (ch[my_length - 1] == ' ')) my_length--; //Find last character that is not a space
  uint8_t left_spaces = (len - my_length) / 2;
  for (uint8_t i = 0; i < left_spaces; i++) str += ' ';
  for (uint8_t i = 0; i < my_length; i++) str += ch[i];
  for (uint8_t i = left_spaces + my_length; i < len; i++) str += ' ';

}

void LCD_clear_string(String & msg) {
  for (uint8_t i = 0; i < LCD_DISPLAY_SIZE; i++) {
    msg[i] = ' ';
  }
}

void LCD_clear_SP_title(uint8_t no) { // Will clear the Label string in the SP array
  for (uint8_t i = 0; i < LCD_DISPLAY_SIZE; i++) {
    SP[no].Title[i] = ' ';
  }
}

void LCD_clear_SP_label(uint8_t no) { // Will clear the Label string in the SP array
  for (uint8_t i = 0; i < LCD_DISPLAY_SIZE; i++) {
    SP[no].Label[i] = ' ';
  }
}

void LCD_set_SP_title(uint8_t sw, String & msg) { // Will set the Title string in the SP array
  // Check length does not exceed LABEL_SIZE
  uint8_t msg_length = msg.length();
  if (msg_length > LCD_DISPLAY_SIZE) msg_length = LCD_DISPLAY_SIZE;
  for (uint8_t i = 0; i < msg_length; i++) {
    SP[sw].Title[i] = msg[i];
  }
  for (uint8_t i = msg_length; i < LCD_DISPLAY_SIZE; i++) { // Fill the remaining chars with spaces
    SP[sw].Title[i] = ' ';
  }
}

void LCD_set_SP_title(uint8_t sw, const char* label) { // Will set the Label string in the SP array
  // Check length does not exceed LABEL_SIZE
  uint8_t len = strlen(label);
  if (len > LCD_DISPLAY_SIZE) len = LCD_DISPLAY_SIZE;
  for (uint8_t i = 0; i < len; i++) {
    SP[sw].Title[i] = label[i];
  }
  for (uint8_t i = len; i < LCD_DISPLAY_SIZE; i++) { // Fill the remaining chars with spaces
    SP[sw].Title[i] = ' ';
  }
}

void LCD_set_SP_label(uint8_t sw, String & lbl) { // Will set the Label string in the SP array
  // Check length does not exceed LABEL_SIZE
  uint8_t len = lbl.length();
  if (len > LCD_DISPLAY_SIZE) len = LCD_DISPLAY_SIZE;
  for (uint8_t i = 0; i < len; i++) {
    SP[sw].Label[i] = lbl[i];
  }
  for (uint8_t i = len; i < LCD_DISPLAY_SIZE; i++) { // Fill the remaining chars with spaces
    SP[sw].Label[i] = ' ';
  }
}

void LCD_set_SP_label(uint8_t sw, const char* label) { // Will set the Label string in the SP array
  // Check length does not exceed LABEL_SIZE
  uint8_t len = strlen(label);
  if (len > LCD_DISPLAY_SIZE) len = LCD_DISPLAY_SIZE;
  for (uint8_t i = 0; i < len; i++) {
    SP[sw].Label[i] = label[i];
  }
  for (uint8_t i = len; i < LCD_DISPLAY_SIZE; i++) { // Fill the remaining chars with spaces
    SP[sw].Label[i] = ' ';
  }
}

void LCD_clear_lcd_txt() {
  for (uint8_t i = 0; i < LCD_DISPLAY_SIZE; i++) {
    lcd_title[i] = ' ';
    lcd_label[i] = ' ';
  }
}

void LCD_clear_lcd_title() {
  for (uint8_t i = 0; i < LCD_DISPLAY_SIZE; i++) {
    lcd_title[i] = ' ';
  }
}

void LCD_add_vled(uint8_t number) { // Will add the VLEDs to both ends of the top line of the display
  for (uint8_t i = 0; i < number; i++) {
    lcd_title[i] = char(CHAR_VLED);
    lcd_title[(LCD_DISPLAY_SIZE - 1) - i] = char(CHAR_VLED);
  }
}

void LCD_set_title(String & msg) { // Will set the Title string in the SP array
  // Check length does not exceed LABEL_SIZE
  uint8_t msg_length = msg.length();
  if (msg_length > LCD_DISPLAY_SIZE) msg_length = LCD_DISPLAY_SIZE;
  for (uint8_t i = 0; i < msg_length; i++) {
    lcd_title[i] = msg[i];
  }
  for (uint8_t i = msg_length; i < LCD_DISPLAY_SIZE; i++) { // Fill the remaining chars with spaces
    lcd_title[i] = ' ';
  }
}

void LCD_add_title(String & title) {
  uint8_t len = title.length();
  if (len > LCD_DISPLAY_SIZE) len = LCD_DISPLAY_SIZE;
  while ((len > 1) && (title[len - 1] == ' ')) len--; //Find last character that is not a space
  uint8_t start_point = (LCD_DISPLAY_SIZE - len) >> 1;
  for (uint8_t i = 0; i < len; i++) { // Copy the title
    lcd_title[start_point + i] = title[i];
  }
}

void LCD_add_title(const char* title) {
  uint8_t len = strlen(title);
  if (len > LCD_DISPLAY_SIZE) len = LCD_DISPLAY_SIZE;
  while ((len > 1) && (title[len - 1] == ' ')) len--; //Find last character that is not a space
  uint8_t start_point = (LCD_DISPLAY_SIZE - len) >> 1;
  for (uint8_t i = 0; i < len; i++) { // Copy the title
    lcd_title[start_point + i] = title[i];
  }
}

void LCD_set_label(String & msg) { // Will set the Title string in the SP array
  // Check length does not exceed LABEL_SIZE
  uint8_t msg_length = msg.length();
  if (msg_length > LCD_DISPLAY_SIZE) msg_length = LCD_DISPLAY_SIZE;
  for (uint8_t i = 0; i < msg_length; i++) {
    lcd_label[i] = msg[i];
  }
  for (uint8_t i = msg_length; i < LCD_DISPLAY_SIZE; i++) { // Fill the remaining chars with spaces
    lcd_label[i] = ' ';
  }
}

void LCD_add_label(String & lbl) {
  uint8_t last_char = lbl.length();
  if (last_char > LCD_DISPLAY_SIZE) last_char = LCD_DISPLAY_SIZE;
  while ((last_char > 1) && (lbl[last_char - 1] == ' ')) last_char--; //Find last character that is not a space
  uint8_t start_point = (LCD_DISPLAY_SIZE - last_char) >> 1;
  for (uint8_t i = 0; i < last_char; i++) { // Copy the lbl
    lcd_label[start_point + i] = lbl[i];
  }
}

void LCD_add_label(const char* lbl) {
  uint8_t last_char = strlen(lbl);
  if (last_char > LCD_DISPLAY_SIZE) last_char = LCD_DISPLAY_SIZE;
  while ((last_char > 1) && (lbl[last_char - 1] == ' ')) last_char--; //Find last character that is not a space
  uint8_t start_point = (LCD_DISPLAY_SIZE - last_char) >> 1;
  for (uint8_t i = 0; i < last_char; i++) { // Copy the title
    lcd_label[start_point + i] = lbl[i];
  }
}

void LCD_add_2digit_number(uint16_t number, String & msg) {
  msg += String(number / 10);
  msg += String(number % 10);
}

void LCD_add_3digit_number(uint16_t number, String & msg) {
  msg += String(number / 100);
  msg += String((number % 100) / 10);
  msg += String(number % 10);
}

void LCD_print_lcd_txt(uint8_t number) {
  bool line1_changed = false;
  bool line2_changed = false;
  if (ledbar_showing != (number)) { // Find first and last character on line 1 that have changed
    line1_changed = LCD_print_delta(number, 0, lcd_title);
  }
  line2_changed = LCD_print_delta(number, 1, lcd_label);
  if (line1_changed | line2_changed)
    MIDI_remote_update_display(number, lcd_title, lcd_label); // Show the data on the remote display
}

bool LCD_print_delta(uint8_t number, uint8_t line, const char * source) {
  if ((number > NUMBER_OF_DISPLAYS) || (line > 1)) return false;
  // We will only update the characters on the displays that have changed.
  uint8_t first_char = 255;
  uint8_t last_char = 0;
  uint8_t lcd_size;
  if (number == 0) lcd_size = MAIN_LCD_DISPLAY_SIZE;
  else lcd_size = LCD_DISPLAY_SIZE;

  for (uint8_t i = 0; i < lcd_size; i++) { // Compare each character of source to the lcd_mem. Find first_char and last_char
    if (source[i] != lcd_mem[number][line][i]) {
      if (first_char == 255) first_char = i;
      last_char = i;
      lcd_mem[number][line][i] = source[i];
    }
  }

  if ((first_char < 255) || (last_char > 0)) {
    if (number == 0) { // Update main display
#ifndef MAIN_TFT_DISPLAY
      Main_lcd.setCursor (first_char, line); // Move cursor to first character that has changed on line 1
      for (uint8_t i = first_char; i <= last_char; i++) {  // Print the characters that have changed (one by one)
        Main_lcd.print(source[i]);
      }
#else
      if (line == 0) TFT_show_main_title();
      if (line == 1) TFT_show_main_label();
#endif
    }
    else { // Update individual display
#ifdef INDIVIDUAL_DISPLAYS
      lcd[number - 1].setCursor (first_char, line); // Move cursor to first character that has changed on line 1
      for (uint8_t i = first_char; i <= last_char; i++) {  // Print the characters that have changed (one by one)
        lcd[number - 1].print(source[i]);
      }
#endif
#ifdef IS_VCTOUCH
      if (line == 0) TFT_set_display_title(number - 1);
      if (line == 1) TFT_set_display_label(number - 1);
#endif
    }
    return true;
  }
  return false;
}

void LCD_clear_memory() {
  memset(lcd_mem, 0, sizeof(lcd_mem)); // Fill lcd_line arrays with zeros
}

void LCD_display_bar(uint8_t lcd_no, uint8_t value, uint16_t colour) { // Will show the bar for the expression pedal on the top line of the main display
#ifdef IS_VCTOUCH
  TFT_show_bar(value, colour);
  ledbarTimer = millis() + LEDBAR_TIMER_LENGTH;
  return;
#endif

  char ledbar[17];
  value &= 0x7F; // Keep value below 0x80
  uint8_t full_blocks = (value >> 3); // Calculate the number of full blocks to display - value is between 0 and 127
  uint8_t part_block = (value & 7) * 6 >> 3; // Calculate which part block to display
  if (lcd_no == 0) {
    for (uint8_t i = 0; i < full_blocks; i++) ledbar[i] = char(4); // Display the full blocks
    if (part_block > 0) ledbar[full_blocks] = char(part_block - 1); // Display the correct part block
    else ledbar[full_blocks] = main_lcd_title[full_blocks];
    for (uint8_t i = full_blocks + 1; i < LCD_DISPLAY_SIZE; i++) ledbar[i] = main_lcd_title[i]; // Fill the rest with remainder of topline
    mainLedbarTimer = millis() + LEDBAR_TIMER_LENGTH;
  }
  else if (lcd_no <= NUMBER_OF_DISPLAYS) {
    // Start the timer to make sure nothing gets printed on this display for awhile, but not if it is my_looper_lcd
    if (lcd_no != my_looper_lcd) ledbar_showing = lcd_no;
    ledbarTimer = millis() + LEDBAR_TIMER_LENGTH;
    LCD_update(lcd_no, false); // Will update lcd_title with the correct data for this switch.
    for (uint8_t i = 0; i < full_blocks; i++) ledbar[i] = char(4); // Display the full blocks
    if (part_block > 0) ledbar[full_blocks] = char(part_block - 1); // Display the correct part block
    else ledbar[full_blocks] = lcd_title[full_blocks];
    for (uint8_t i = full_blocks + 1; i < LCD_DISPLAY_SIZE; i++) ledbar[i] = lcd_title[i]; // Fill the rest with remainder of title
  }
  LCD_print_delta(lcd_no, 0, ledbar);
}

void LCD_show_bar(uint8_t lcd_no, uint8_t value, uint16_t colour) {
  if (lcd_no == 0) {
    main_ledbar_showing = true;
  }
  LCD_display_bar(lcd_no, value, colour);
}

void LCD_show_looper_bar(uint8_t lcd_no, uint8_t value, uint16_t colour) {
  if (lcd_no == 0) {
    if (main_ledbar_showing) return;
    main_looper_ledbar_showing = true;
  }
  LCD_display_bar(lcd_no, value, colour);
}

void LCD_switch_on_backlight(uint8_t number) {
#ifdef INDIVIDUAL_DISPLAYS
  if (number < NUMBER_OF_DISPLAYS) {
    if (!backlight_on[number]) {
      lcd[number].setBacklight(HIGH);
      backlight_on[number] = true;
    }
  }
#endif
}

void LCD_switch_off_backlight(uint8_t number) {
#ifdef INDIVIDUAL_DISPLAYS
  if (number < NUMBER_OF_DISPLAYS) {
    if (backlight_on[number]) {
      lcd[number].setBacklight(LOW);
      backlight_on[number] = false;
    }
  }
#endif
}

void LCD_backlight_on() { // Will switch all backlights on
#ifdef INDIVIDUAL_DISPLAYS
  for (uint8_t i = 0; i < NUMBER_OF_DISPLAYS; i++) {
    LCD_switch_on_backlight(i);
  }
#endif
#ifndef MAIN_TFT_DISPLAY
  Main_lcd.setBacklight(HIGH);
#endif
}

void LCD_backlight_off() { // Will switch all backlights off
#ifdef INDIVIDUAL_DISPLAYS
  for (uint8_t i = 0; i < NUMBER_OF_DISPLAYS; i++) {
    LCD_switch_off_backlight(i); //switch on the backlight
  }
#endif
#ifndef MAIN_TFT_DISPLAY
  Main_lcd.setBacklight(LOW);
  Main_lcd.clear(); // Clear main display
#endif
}

void LCD_clear_all_displays() {
  LCD_clear_main_lcd_txt();
  LCD_clear_lcd_txt();
#ifndef IS_VCMINI
  for (uint8_t i = 0; i < NUMBER_OF_DISPLAYS + 1; i++) {
    LCD_print_lcd_txt(i);
  }
#endif
}

void LCD_show_are_you_sure(String line1, String line2)
{
  LCD_clear_all_displays();
  LCD_main_set_title(line1); //Print message on main display
  LCD_main_set_label(line2);
  LCD_print_main_lcd_txt();

#ifdef VCMINI_DISPLAYS
  const char LBL_SURE[] = "No   Sure?   Yes";
  LCD_main_set_label(LBL_SURE);
  LCD_print_main_lcd_txt();
#else
  const char LBL_YES[] = "YES";
  const char LBL_NO[] = "NO";
  LCD_clear_lcd_txt();
  LCD_add_label(LBL_YES);
  LCD_print_lcd_txt(YES_SWITCH);
  LCD_clear_lcd_txt();
  LCD_add_label(LBL_NO);
  LCD_print_lcd_txt(NO_SWITCH);
#endif
}

void LCD_show_program_mode() {
  LCD_clear_all_displays();
  LCD_main_set_title("* PROGRAM MODE *");
  LCD_print_main_lcd_txt();
  LCD_add_title("Upload firmware ");
  LCD_add_label("from TeensyDuino");
  LCD_print_lcd_txt(YES_SWITCH);
  LCD_clear_lcd_txt();
  LCD_add_title(" or power cycle ");
  LCD_add_label("the VController ");
  LCD_print_lcd_txt(NO_SWITCH);
}

void LCD_number_to_note(uint8_t number, String & msg) {
  uint8_t note = number % 12;
  uint8_t octave = number / 12;
  const char notename[12][3] = {"C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B" };

  msg = notename[note];
  if (octave == 0) msg += "(-2)"; // Following "C3 convention" - see http://computermusicresource.com/midikeys.html
  if (octave == 1) msg += "(-1)";
  if (octave >= 2) msg += String(octave - 2);
}

// ********************************* Section 5: Virtual LED functions ********************************************

// A virtual LED is user defined character 0 on the display. The virtual LED is changed by changing the user defined character.
// To have one or more virtual LEDS in a display message, user defined character 0 has to be included on the position where the LEDs have to appear
// The state of these virtual LEDs is set in the LEDs section.

uint8_t Display_LED[NUMBER_OF_SWITCHES]; // For showing state of the LEDs on the display. Can have state on (1), off (0) and dimmed (2)
// Custom characters for virtual LEDs
byte vLED_on[8] = { // The character for virtual LED on
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b00000,
  0b00000,
  0b00000
};
byte vLED_off[8] = { // The character for virtual LED off
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};
byte vLED_dimmed[8] = { // The character for LED dimmed
  0b11111,
  0b10001,
  0b10001,
  0b10001,
  0b11111,
  0b00000,
  0b00000,
  0b00000
};

void LCD_init_virtual_LED(uint8_t number) { // Initialize virtual LED
  Display_LED[number] = 0;
#ifdef INDIVIDUAL_DISPLAYS
  if (number < NUMBER_OF_DISPLAYS) lcd[number].createChar(CHAR_VLED, vLED_off);
#endif
}

void LCD_set_virtual_LED(uint8_t number, uint8_t state) { // Will set the state of a virtual LED
  if (Display_LED[number] != state) { // Check if state is new
    Display_LED[number] = state; // Update state

#ifdef INDIVIDUAL_DISPLAYS
    // Update virtual LED
    if (number < NUMBER_OF_DISPLAYS) {
      if (state == 0) lcd[number].createChar(CHAR_VLED, vLED_off);
      if (state == 1) lcd[number].createChar(CHAR_VLED, vLED_on);
      if (state == 2) lcd[number].createChar(CHAR_VLED, vLED_dimmed);
    }
#endif
  }
}

void Set_virtual_LED_colour(uint8_t number, uint8_t colour) { // Called from LEDs.ino-show_colour()
  if ((colour == 0) | (colour > 9)) LCD_set_virtual_LED(number, 0); //Virtual LED off
  else LCD_set_virtual_LED(number, 1); //Virtual LED on
}

// ********************************* Section 6: TFT functions ********************************************
#ifdef IS_VCTOUCH
#define DRAG_MIN_POSITION 25 // Bottom of screen
#define DRAG_MAX_POSITION 325 // Top of screen

void TFT_handleTouch(int8_t contacts, GTPoint *points) {
  DEBUGMSG("Touch contacts: " + String(contacts));
  if (contacts > 0) {
    if ((!touch_active) && (contacts == 1)) {
      touch_active = true;
      if ((!on_screen_keyboard_active) && (!pong_active)) {
        last_touch_x_pos = points[0].x_lsb + (points[0].x_msb << 8);
        last_touch_y_pos = points[0].y_lsb + (points[0].y_msb << 8);
        uint8_t sw = TFT_checkTouchButton(last_touch_x_pos, last_touch_y_pos);
        if (sw > 0) {
          last_touch_switch_pressed = sw;
          SC_remote_switch_pressed(last_touch_switch_pressed, true);
        }
        if (TFT_check_menu_pressed(last_touch_x_pos, last_touch_y_pos)) {
          if (Current_page != PAGE_MENU) SCO_select_page(PAGE_MENU);
          else menu_exit();
        }
      }
      else { // Pressing key on on-screen keyboard
        TFT_check_on_screen_key_press(points[0].x_lsb + (points[0].x_msb << 8), points[0].y_lsb + (points[0].y_msb << 8));
      }
    }
    else { // Check for dragging
      //uint16_t new_x_pos = points[0].x_lsb + (points[0].x_msb << 8);
      uint16_t new_y_pos = points[0].y_lsb + (points[0].y_msb << 8);
      if (!touch_drag_active) { // Check if drag can be enabled
        if ((new_y_pos > (last_touch_y_pos + 10)) || ((new_y_pos + 10) < last_touch_y_pos)) {
          touch_drag_active = true;
          SC_skip_release_and_hold_until_next_press(SKIP_RELEASE | SKIP_LONG_PRESS | SKIP_HOLD);
        }
      }
      else { // Drag is active
        DEBUGMSG("Drag y position:" + String(new_y_pos));
        if (last_touch_switch_pressed > 0) {
          uint16_t val = 399 - new_y_pos;
          if (val < DRAG_MIN_POSITION) val = DRAG_MIN_POSITION;
          if (val > DRAG_MAX_POSITION) val = DRAG_MAX_POSITION;
          DEBUGMSG("Drag value:" + String(map(val, DRAG_MIN_POSITION, DRAG_MAX_POSITION, 0, 127)));
          SCO_move_touch_screen_parameter(last_touch_switch_pressed, map(val, DRAG_MIN_POSITION, DRAG_MAX_POSITION, 0, 127));
        }
      }
    }
  }
  else {
    if ((!on_screen_keyboard_active) && (!pong_active)) {
      SC_remote_switch_released(last_touch_switch_pressed, true);
    }
    touch_active = false;
    touch_drag_active = false;
    last_touch_switch_pressed = 0;
    SC_reset_multipress();
  }

}

uint8_t TFT_checkTouchButton(uint16_t x, uint16_t y) {
  for (uint8_t s = 0; s < 15; s++) {
    if ((x >= TFT_layout[s].x_pos) && (x < TFT_layout[s].x_pos + TFT_layout[s].x_size)
        && (y >= TFT_layout[s].y_pos) && (y < TFT_layout[s].y_pos + TFT_layout[s].y_size)) {
      // Switch pressed
      return s + 1;
    }
  }
  return 0;
}

void TFT_fillScreen(uint16_t colour) {
  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(layer1_start_addr);
  tft.Main_Image_Width(400);
  tft.Main_Window_Start_XY(0, 0);
  tft.Foreground_color_65k(colour);
  tft.Line_Start_XY(0, 0);
  tft.Line_End_XY(399, 1279);
  tft.Start_Square_Fill();
}

void TFT_draw_main_grid() {
  tft.Canvas_Image_Start_address(layer1_start_addr);
  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(layer1_start_addr);
  tft.Main_Image_Width(400);
  tft.Main_Window_Start_XY(0, 0);
  tft.Foreground_color_65k(Grey);

  for (uint8_t i = 0; i < 15; i++) {
    tft.Line_Start_XY(TFT_layout[i].y_pos, TFT_layout[i].x_pos);
    tft.Line_End_XY(TFT_layout[i].y_pos + TFT_layout[i].y_size, TFT_layout[i].x_pos + TFT_layout[i].x_size);
    tft.Start_Square();
  }

  TFT_draw_menu_icon();
}

void TFT_switch_off_main_window() {
#ifdef MAIN_TFT_BACKLIGHT_PIN
  for (uint8_t b = TFT_current_brightness; b -- > 0 ;) {
    LCD_set_TFT_brightness(b);
    delay((255 - b) >> 5);
  }
#else
  tft.Backlight_OFF();
#endif
  TFT_fillScreen(Black);
}


void TFT_update_LED(uint8_t number, uint16_t colour) {
  if (on_screen_keyboard_active) return;
  if (pong_active) return;
  if (TFT_label_colour[number] == colour) return;
  tft.Canvas_Image_Start_address(layer1_start_addr);
  tft.Main_Window_Start_XY(0, 0);
  tft.Foreground_color_65k(colour);
  tft.Line_Start_XY(TFT_layout[number].y_pos + 1, TFT_layout[number].x_pos + 1);
  tft.Line_End_XY(TFT_layout[number].y_pos + TFT_layout[number].y_size - 2, TFT_layout[number].x_pos + TFT_layout[number].x_size - 2);
  tft.Start_Square_Fill();
  TFT_label_colour[number] = colour;
  TFT_show_display_title(number);
  TFT_show_display_label(number);
}

void TFT_set_display_title(uint8_t number) {
  if (number < NUMBER_OF_TOUCH_SWITCHES) {
    for (uint8_t c = 0; c < LCD_DISPLAY_SIZE; c++) {
      if (lcd_title[c] < 32) TFT_title_text[number][c] = ' '; // Remove custom characters
      else TFT_title_text[number][c] = lcd_title[c];
    }
    TFT_show_display_title(number);
  }
}

void TFT_show_display_title(uint8_t number) {
  if (number < NUMBER_OF_TOUCH_SWITCHES) {
    uint16_t front_colour;
    if (TFT_check_dark_colour(number)) front_colour = White;
    else front_colour = Black;
#ifdef USE_TFT_USER_FONT
    if (TFT_title_text[number][LCD_DISPLAY_SIZE - 1] == ' ') {
      String lbl = TFT_title_text[number];
      lbl = lbl.trim();
      uint8_t whitespace = (LCD_DISPLAY_SIZE - lbl.length() - 1) * 8;
      TFT_show_user_font(Grotesk16x32, TFT_spaces, TFT_layout[number].x_pos + 8, TFT_layout[number].y_pos + 4, front_colour, TFT_label_colour[number], false);
      TFT_show_user_font(Grotesk16x32, lbl, TFT_layout[number].x_pos + whitespace + 8, TFT_layout[number].y_pos + 4, front_colour, TFT_label_colour[number], false);
    }
    else TFT_show_user_font(GroteskBold16x32, TFT_title_text[number], TFT_layout[number].x_pos + 8, TFT_layout[number].y_pos + 4, front_colour, TFT_label_colour[number], true);
#else
    tft.Canvas_Image_Start_address(layer1_start_addr);
    tft.Foreground_color_65k(front_colour);
    tft.Background_color_65k(TFT_label_colour[number]);
    tft.CGROM_Select_Internal_CGROM();
    tft.Font_Select_16x32_32x32();
    tft.Font_Width_X1();
    tft.Font_Height_X1();
    tft.Goto_Text_XY(TFT_layout[number].y_pos + 5, TFT_layout[number].x_pos + 16);
    tft.Show_String(TFT_title_text[number]);
#endif
  }
}

void TFT_set_display_label(uint8_t number) {
  if (number < NUMBER_OF_TOUCH_SWITCHES) {
    for (uint8_t c = 0; c < LCD_DISPLAY_SIZE; c++) {
      if (lcd_label[c] < 32) TFT_label_text[number][c] = ' '; // Remove custom characters
      else TFT_label_text[number][c] = lcd_label[c];
    }
    TFT_show_display_label(number);
  }
}

void TFT_show_display_label(uint8_t number) {
  if (number < NUMBER_OF_TOUCH_SWITCHES) {
    uint16_t front_colour;
    if (TFT_check_dark_colour(number)) front_colour = Yellow;
    else front_colour = 0x2115; // Dark blue
#ifdef USE_TFT_USER_FONT
    if (TFT_label_text[number][LCD_DISPLAY_SIZE - 1] == ' ') {
      String lbl = TFT_label_text[number];
      lbl = lbl.trim();
      uint8_t whitespace = (LCD_DISPLAY_SIZE - lbl.length() - 1) * 8;
      TFT_show_user_font(Grotesk16x32, TFT_spaces, TFT_layout[number].x_pos + 8, TFT_layout[number].y_pos + 35, front_colour, TFT_label_colour[number], false);
      TFT_show_user_font(Grotesk16x32, lbl, TFT_layout[number].x_pos + whitespace + 8, TFT_layout[number].y_pos + 35, front_colour, TFT_label_colour[number], false);
    }
    else TFT_show_user_font(Grotesk16x32, TFT_label_text[number], TFT_layout[number].x_pos + 8, TFT_layout[number].y_pos + 35, front_colour, TFT_label_colour[number], true);
#else
    tft.Canvas_Image_Start_address(layer1_start_addr);
    tft.Foreground_color_65k(front_colour);
    tft.Background_color_65k(TFT_label_colour[number]);
    tft.CGROM_Select_Internal_CGROM();
    tft.Font_Select_16x32_32x32();
    tft.Font_Width_X1();
    tft.Font_Height_X1();
    tft.Goto_Text_XY(TFT_layout[number].y_pos + 35, TFT_layout[number].x_pos + 16);
    tft.Show_String(TFT_label_text[number]);
#endif
  }
}

bool TFT_check_dark_colour(uint8_t number) {
  //uint16_t cr = (TFT_label_colour[number] & 0xF800) >> 8;
  uint16_t cg = (TFT_label_colour[number] & 0x07E0) >> 3;
  //uint16_t cb = (TFT_label_colour[number] & 0x001F) << 3;
  if (cg > 0xBF) return false;
  else return true;
}

#define MAIN_LCD_X_POS (640 - (MAIN_LCD_DISPLAY_SIZE * 16)) // Was 380
#define MAIN_LCD_Y_POS 100

void TFT_show_main_title() {
#ifdef USE_TFT_USER_FONT
  if (Setting.Main_display_show_top_right == MDT_OFF) {
    String lbl = main_lcd_title;
    lbl = lbl.trim();
    uint16_t whitespace = (MAIN_LCD_DISPLAY_SIZE - lbl.length()) * 16;
    TFT_show_user_font(Grotesk32x64, TFT_main_spaces, MAIN_LCD_X_POS, MAIN_LCD_Y_POS, White, Black, false);
    TFT_show_user_font(Grotesk32x64, lbl, MAIN_LCD_X_POS + whitespace, MAIN_LCD_Y_POS, White, Black, false);
  }
  else
    TFT_show_user_font(GroteskBold32x64, main_lcd_title, MAIN_LCD_X_POS, MAIN_LCD_Y_POS, White, Black, false);
#else
  tft.Canvas_Image_Start_address(layer1_start_addr);
  tft.Foreground_color_65k(White);
  tft.Background_color_65k(Black);
  tft.CGROM_Select_Internal_CGROM();
  tft.Font_Select_16x32_32x32();
  tft.Font_Width_X2();
  tft.Font_Height_X2();
  tft.Goto_Text_XY(MAIN_LCD_Y_POS, MAIN_LCD_X_POS);
  tft.Show_String(main_lcd_title);
#endif
}

void TFT_show_main_label() {
#ifdef USE_TFT_USER_FONT
  if (Setting.Main_display_show_top_right == MDT_OFF) {
    String lbl = main_lcd_label;
    lbl = lbl.trim();
    uint16_t whitespace = (MAIN_LCD_DISPLAY_SIZE - lbl.length()) * 16;
    TFT_show_user_font(Grotesk32x64, TFT_main_spaces, MAIN_LCD_X_POS, MAIN_LCD_Y_POS + 60, Yellow, Black, false);
    TFT_show_user_font(Grotesk32x64, lbl, MAIN_LCD_X_POS + whitespace, MAIN_LCD_Y_POS + 60, Yellow, Black, false);
  }
  else
    TFT_show_user_font(Grotesk32x64, main_lcd_label, MAIN_LCD_X_POS, MAIN_LCD_Y_POS + 60, Yellow, Black, false);
#else
  tft.Canvas_Image_Start_address(layer1_start_addr);
  tft.Foreground_color_65k(Yellow);
  tft.Background_color_65k(Black);
  tft.CGROM_Select_Internal_CGROM();
  tft.Font_Select_16x32_32x32();
  tft.Font_Width_X2();
  tft.Font_Height_X2();
  tft.Goto_Text_XY(MAIN_LCD_Y_POS + 60, MAIN_LCD_X_POS);
  tft.Show_String(main_lcd_label);
#endif
}

#define TFT_BAR_START_X MAIN_LCD_X_POS
#define TFT_BAR_START_Y 228
#define TFT_BAR_END_X (1280 - MAIN_LCD_X_POS)
#define TFT_BAR_END_Y 238

void TFT_show_bar(uint8_t value, uint16_t colour) {
  tft.Canvas_Image_Start_address(layer1_start_addr);
  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(layer1_start_addr);
  tft.Main_Image_Width(400);
  tft.Main_Window_Start_XY(0, 0);

  uint16_t length = map(value, 0, 127, 0, TFT_BAR_END_X - TFT_BAR_START_X);
  if (colour == 0) tft.Foreground_color_65k(White);
  else tft.Foreground_color_65k(colour);
  tft.Line_Start_XY(TFT_BAR_START_Y, TFT_BAR_START_X);
  tft.Line_End_XY(TFT_BAR_END_Y, TFT_BAR_START_X + length);
  tft.Start_Square_Fill();

  tft.Foreground_color_65k(0x39E7);
  tft.Line_Start_XY(TFT_BAR_START_Y, TFT_BAR_START_X + length);
  tft.Line_End_XY(TFT_BAR_END_Y, TFT_BAR_END_X);
  tft.Start_Square_Fill();
  TFT_bar_showing = true;
}

void TFT_hide_bar() {
  tft.Canvas_Image_Start_address(layer1_start_addr);
  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(layer1_start_addr);
  tft.Main_Image_Width(400);
  tft.Main_Window_Start_XY(0, 0);

  tft.Foreground_color_65k(Black);
  tft.Line_Start_XY(TFT_BAR_START_Y, TFT_BAR_START_X);
  tft.Line_End_XY(TFT_BAR_END_Y, TFT_BAR_END_X);
  tft.Start_Square_Fill();
  TFT_bar_showing = true;
}

void TFT_draw_Big_Point(uint16_t x, uint16_t y, uint16_t colour) {
  if (x < 1) x = 1;
  if (x > 1278) x = 1278;
  if (y < 1) y = 1;
  if (y > 398) y = 398;
  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(layer1_start_addr);
  tft.Main_Window_Start_XY(0, 0);
  tft.Foreground_color_65k(colour);
  tft.Line_Start_XY(y - 1, x - 1);
  tft.Line_End_XY(y + 1, x + 1);
  tft.Start_Square_Fill();
}

#define TFT_SYMBOL_ON_COLOUR White
#define TFT_SYMBOL_OFF_COLOUR 0x8471
#define TFT_MENU_X_POS 1245
#define TFT_MENU_Y_POS 80
#define TFT_MENU_ICON_COLOUR TFT_SYMBOL_OFF_COLOUR

void TFT_draw_menu_icon() {
  tft.Canvas_Image_Start_address(layer1_start_addr);
  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(layer1_start_addr);
  tft.Main_Image_Width(400);
  tft.Main_Window_Start_XY(0, 0);
  tft.Foreground_color_65k(TFT_MENU_ICON_COLOUR);

  // Three lines
  for (uint8_t i = 0; i < 3; i++) {
    tft.Line_Start_XY(TFT_MENU_Y_POS + (i * 10), TFT_MENU_X_POS + 4);
    tft.Line_End_XY(TFT_MENU_Y_POS + 5 + (i * 10), TFT_MENU_X_POS + 28);
    //tft.Circle_Square_Radius_RxRy(2, 2);
    //tft.Start_Circle_Square_Fill();
    tft.Start_Square_Fill();
  }

  // Text
  //const String menu_txt = "MENU";
  //TFT_show_user_font(Grotesk16x32, menu_txt, TFT_MENU_X_POS + 36, TFT_MENU_Y_POS - 3, TFT_MENU_ICON_COLOUR, Black, false);
}

bool TFT_check_menu_pressed(uint16_t x_pos, uint16_t y_pos) {
  if ((x_pos > TFT_MENU_X_POS - 90) && (y_pos > TFT_MENU_Y_POS) && (y_pos < TFT_MENU_Y_POS + 35)) return true;
  return false;
}

void TFT_show_bluetooth_state() {
  if (on_screen_keyboard_active) return;
  if (pong_active) return;
  char bt_char;
  uint16_t bt_colour;
  if (Setting.BLE_mode != 0) bt_char = '"';
  else bt_char = ' ';
  if (show_ble_state != 0) bt_colour = TFT_SYMBOL_ON_COLOUR;
  else bt_colour = TFT_SYMBOL_OFF_COLOUR;
  TFT_show_user_font(Symbols24x24, bt_char, 1155, 80, bt_colour, Black, false);
}

void TFT_show_wifi_state() {
  if (on_screen_keyboard_active) return;
  if (pong_active) return;
  char wifi_char;
  uint16_t wifi_colour;
  if (Setting.WIFI_mode == 1) wifi_char = '!';
  else if (Setting.WIFI_mode == 2) wifi_char = '$';
  else wifi_char = ' ';
  if (show_wifi_state != 0) wifi_colour = TFT_SYMBOL_ON_COLOUR;
  else wifi_colour = TFT_SYMBOL_OFF_COLOUR;
  TFT_show_user_font(Symbols24x24, wifi_char, 1185, 80, wifi_colour, Black, false);
}

void TFT_show_rtpmidi_state() {
  if (on_screen_keyboard_active) return;
  if (pong_active) return;
  char rtpmidi_char;
  if (Setting.WIFI_mode > 0) rtpmidi_char = '#';
  else rtpmidi_char = ' ';
  uint16_t rtpmidi_colour;
  if (show_rtpmidi_state != 0) rtpmidi_colour = TFT_SYMBOL_ON_COLOUR;
  else rtpmidi_colour = TFT_SYMBOL_OFF_COLOUR;
  TFT_show_user_font(Symbols24x24, rtpmidi_char, 1215, 80, rtpmidi_colour, Black, false);
}

// ********************************* Section 7: TFT On-screen Keyboard ********************************************

#define KEYBOARD_CAPS 0
#define KEYBOARD_NO_CAPS 1
#define KEYBOARD_NUM1 2
#define KEYBOARD_NUM2 3

uint8_t current_keyboard = KEYBOARD_CAPS;
uint8_t prev_keyboard = KEYBOARD_CAPS;

#define KEY_HEIGHT 67
#define KEY_MARGIN 4
#define KEY_FOREGROUND_COLOUR White
#define KEY_BACKGROUND_COLOUR Black
#define UTILITY_KEY_FOREGROUND_COLOUR White
#define UTILITY_KEY_BACKGROUND_COLOUR 0x3186

#define Y_POS_ROW0  3
#define Y_POS_ROW1 11
#define Y_POS_ROW2 18
#define Y_POS_ROW3 25
#define Y_POS_ROW4 32

#define TOP_KEYBOARD_EDGE 20
#define BOTTOM_KEYBOARD_EDGE 394
#define LEFT_KEYBOARD_EDGE 80
#define RIGHT_KEYBOARD_EDGE 1200

#define OSK_KEY_BACKSPACE (char) 1
#define OSK_KEY_ENTER (char) 2
#define OSK_KEY_SHIFT (char) 3
#define OSK_KEY_NUM (char) 4
#define OSK_KEY_ABC (char) 5
#define OSK_KEY_LEFT (char) 6
#define OSK_KEY_RIGHT (char) 7
#define OSK_KEY_ESCAPE (char) 8
#define OSK_KEY_CLEAR (char) 9

struct TFT_keyboard_struct { // Combines all the data we need for controlling a parameter in a device
  uint8_t x_pos;
  uint8_t y_pos;
  uint8_t x_width;
  char key1;
  char key2;
  char key3;
  char key4;
};

const TFT_keyboard_struct TFT_keyboard[] = {
  {   9,  Y_POS_ROW1, 10, 'Q', 'q', '1', '1' },
  {  19,  Y_POS_ROW1, 10, 'W', 'w', '2', '2' },
  {  29,  Y_POS_ROW1, 10, 'E', 'e', '3', '3' },
  {  39,  Y_POS_ROW1, 10, 'R', 'r', '4', '4' },
  {  49,  Y_POS_ROW1, 10, 'T', 't', '5', '5' },
  {  59,  Y_POS_ROW1, 10, 'Y', 'y', '6', '6' },
  {  69,  Y_POS_ROW1, 10, 'U', 'u', '7', '7' },
  {  79,  Y_POS_ROW1, 10, 'I', 'i', '8', '8' },
  {  89,  Y_POS_ROW1, 10, 'O', 'o', '9', '9' },
  {  99,  Y_POS_ROW1, 10, 'P', 'p', '0', '0' },
  { 109,  Y_POS_ROW1, 10, OSK_KEY_BACKSPACE, OSK_KEY_BACKSPACE, OSK_KEY_BACKSPACE, OSK_KEY_BACKSPACE},

  {  14,  Y_POS_ROW2, 10, 'A', 'a', '@', '$' },
  {  24,  Y_POS_ROW2, 10, 'S', 's', '#', '#' },
  {  34,  Y_POS_ROW2, 10, 'D', 'd', '$', '@' },
  {  44,  Y_POS_ROW2, 10, 'F', 'f', '&', '_' },
  {  54,  Y_POS_ROW2, 10, 'G', 'g', '*', '^' },
  {  64,  Y_POS_ROW2, 10, 'H', 'h', '(', '[' },
  {  74,  Y_POS_ROW2, 10, 'J', 'j', ')', ']' },
  {  84,  Y_POS_ROW2, 10, 'K', 'k', '\'', '{' },
  {  94,  Y_POS_ROW2, 10, 'L', 'l', '"', '}' },
  { 104,  Y_POS_ROW2, 15, OSK_KEY_ENTER, OSK_KEY_ENTER, OSK_KEY_ENTER, OSK_KEY_ENTER},

  {   9,  Y_POS_ROW3, 10, OSK_KEY_SHIFT, OSK_KEY_SHIFT, OSK_KEY_SHIFT, OSK_KEY_SHIFT},
  {  19,  Y_POS_ROW3, 10, 'Z', 'z', '%', '`' },
  {  29,  Y_POS_ROW3, 10, 'X', 'x', '-', '|' },
  {  39,  Y_POS_ROW3, 10, 'C', 'c', '+', '~' },
  {  49,  Y_POS_ROW3, 10, 'V', 'v', '=', '=' },
  {  59,  Y_POS_ROW3, 10, 'B', 'b', '/', '\\' },
  {  69,  Y_POS_ROW3, 10, 'N', 'n', ';', '<' },
  {  79,  Y_POS_ROW3, 10, 'M', 'm', ':', '>' },
  {  89,  Y_POS_ROW3, 10, '!', ',', ',', '!' },
  {  99,  Y_POS_ROW3, 10, '?', '.', '.', '?' },
  { 109,  Y_POS_ROW3, 10, OSK_KEY_SHIFT, OSK_KEY_SHIFT, OSK_KEY_SHIFT, OSK_KEY_SHIFT},

  {   9,  Y_POS_ROW4, 20, OSK_KEY_NUM, OSK_KEY_NUM, OSK_KEY_ABC, OSK_KEY_ABC},
  {  29,  Y_POS_ROW4, 70, ' ', ' ', ' ', ' ' },
  {  99,  Y_POS_ROW4, 10, OSK_KEY_LEFT, OSK_KEY_LEFT, OSK_KEY_LEFT, OSK_KEY_LEFT},
  { 109,  Y_POS_ROW4, 10, OSK_KEY_RIGHT, OSK_KEY_RIGHT, OSK_KEY_RIGHT, OSK_KEY_RIGHT},

  {   9,  Y_POS_ROW0, 10, OSK_KEY_ESCAPE, OSK_KEY_ESCAPE, OSK_KEY_ESCAPE, OSK_KEY_ESCAPE},
  { 109,  Y_POS_ROW0, 10, OSK_KEY_CLEAR,  OSK_KEY_CLEAR,  OSK_KEY_CLEAR,  OSK_KEY_CLEAR},
};

const uint8_t NUMBER_OF_KEYS = sizeof(TFT_keyboard) / sizeof(TFT_keyboard[0]);

void TFT_draw_keyboard(bool text_only) {
  // We always draw the outline of the keyboard, also when text_only = false, otherwise keyboard may show weird random characters
  tft.Foreground_color_65k(KEY_BACKGROUND_COLOUR);
  tft.Line_Start_XY(TOP_KEYBOARD_EDGE, LEFT_KEYBOARD_EDGE);
  tft.Line_End_XY(BOTTOM_KEYBOARD_EDGE, RIGHT_KEYBOARD_EDGE);
  tft.Circle_Square_Radius_RxRy(10, 10);
  if (!text_only) tft.Start_Circle_Square_Fill();
  tft.Foreground_color_65k(KEY_FOREGROUND_COLOUR);
  tft.Start_Circle_Square();

  for (uint8_t k = 0; k < NUMBER_OF_KEYS; k++) {
    TFT_draw_key(TFT_keyboard[k].x_pos, TFT_keyboard[k].y_pos, TFT_keyboard[k].x_width, TFT_key_char(k), text_only);
  }
  TFT_print_text_entry();
  on_screen_keyboard_active = true;
}

void TFT_exit_on_screen_keyboard(bool save_and_exit) {
  if (save_and_exit) save_and_close_keyboard();
  else close_keyboard();
  TFT_fillScreen(Black);
  TFT_draw_main_grid();
  LCD_clear_memory();
  on_screen_keyboard_active = false;
  TFT_current_device_for_pic = 255; // Triggers a refresh of the device pic
  update_page = RELOAD_PAGE;
  update_main_lcd = true;
}

void TFT_draw_key(uint8_t x_pos, uint8_t y_pos, uint8_t width, char key, bool text_only) {
  uint16_t x = x_pos * 10;
  uint16_t y = y_pos * 10;
  uint16_t w = width * 10;
  char key_txt[2] = {key, ' '};
  tft.Canvas_Image_Start_address(layer1_start_addr);
  tft.Main_Window_Start_XY(0, 0);
  if (!text_only) {
    if (key > (char)10) {
      tft.Foreground_color_65k(KEY_BACKGROUND_COLOUR);
    }
    else {
      tft.Foreground_color_65k(UTILITY_KEY_BACKGROUND_COLOUR);
    }
    tft.Line_Start_XY(y + KEY_MARGIN, x + KEY_MARGIN);
    tft.Line_End_XY(y + KEY_HEIGHT, x + w - (2 * KEY_MARGIN));
    tft.Circle_Square_Radius_RxRy(10, 10);
    tft.Start_Circle_Square_Fill();
  }
  if (key > (char)10) {
    tft.Foreground_color_65k(KEY_FOREGROUND_COLOUR);
    tft.Background_color_65k(KEY_BACKGROUND_COLOUR);
  }
  else {
    tft.Foreground_color_65k(UTILITY_KEY_FOREGROUND_COLOUR);
    tft.Background_color_65k(UTILITY_KEY_BACKGROUND_COLOUR);
  }
  if (!text_only) tft.Start_Circle_Square();

  uint16_t x_char_pos;
  String text;
  switch (key) {
    case OSK_KEY_BACKSPACE:
      x_char_pos = x + (w / 2) - 20;
      text = "<=";
      break;
    case OSK_KEY_ENTER:
      x_char_pos = x + (w / 2) - 40;
      text = "enter";
      break;
    case OSK_KEY_SHIFT:
      x_char_pos = x + (w / 2) - 10;
      text = '^';
      break;
    case OSK_KEY_NUM:
      x_char_pos = x + (w / 2) - 30;
      text = "#123";
      break;
    case OSK_KEY_ABC:
      x_char_pos = x + (w / 2) - 35;
      text = " ABC ";
      break;
    case OSK_KEY_LEFT:
      x_char_pos = x + (w / 2) - 10;
      text = '<';
      break;
    case OSK_KEY_RIGHT:
      x_char_pos = x + (w / 2) - 10;
      text = '>';
      break;
    case OSK_KEY_ESCAPE:
      x_char_pos = x + (w / 2) - 25;
      text = "esc";
      break;
    case OSK_KEY_CLEAR:
      x_char_pos = x + (w / 2) - 25;
      text = "clr";
      break;
    default:
      x_char_pos = x + (w / 2) - 10;
      text = String(key_txt);
      break;
  }
#ifdef USE_TFT_USER_FONT
  if (key > (char)10) {
    TFT_show_user_font(GroteskBold16x32, text, x_char_pos, y + (KEY_MARGIN * 2) + 12, KEY_FOREGROUND_COLOUR, KEY_BACKGROUND_COLOUR, false);
  }
  else {
    TFT_show_user_font(Grotesk16x32, text, x_char_pos, y + (KEY_MARGIN * 2) + 12, UTILITY_KEY_FOREGROUND_COLOUR, UTILITY_KEY_BACKGROUND_COLOUR, false);
  }
#else
  tft.CGROM_Select_Internal_CGROM();
  tft.Font_Select_16x32_32x32();
  tft.Font_Width_X1();
  tft.Font_Height_X1();
  tft.Goto_Text_XY(y + (KEY_MARGIN * 2) + 12, x_char_pos);
  //if (text.length() == 1) text += " "; // Workaround for not properly showing single characters
  char ch_text[text.length()];
  text.toCharArray(ch_text, text.length() + 1);
  tft.Show_String(ch_text);
#endif
}

void TFT_print_text_entry() {
#ifdef USE_TFT_USER_FONT
  TFT_show_user_font(Grotesk32x64, Text_entry, 640 - (Text_entry_length * 16), 25, UTILITY_KEY_FOREGROUND_COLOUR, UTILITY_KEY_BACKGROUND_COLOUR, false);
#else
  char text[Text_entry_length];
  Text_entry.toCharArray(text, Text_entry_length + 1);
  tft.Canvas_Image_Start_address(layer1_start_addr);
  tft.Foreground_color_65k(UTILITY_KEY_FOREGROUND_COLOUR);
  tft.Background_color_65k(UTILITY_KEY_BACKGROUND_COLOUR);
  tft.CGROM_Select_Internal_CGROM();
  tft.Font_Select_16x32_32x32();
  tft.Font_Width_X2();
  tft.Font_Height_X2();
  tft.Goto_Text_XY(25, 640 - (Text_entry_length * 16));
  tft.Show_String(text);
#endif

  // Show cursor
  tft.Line_Start_XY(85, 640 - (Text_entry_length * 16) + ((Main_menu_cursor - 1) * 32));
  tft.Line_End_XY(86, 639 - (Text_entry_length * 16) + (Main_menu_cursor * 32));
  tft.Start_Square();
}

void TFT_check_on_screen_key_press(uint16_t x, uint16_t y) {
  Serial.println("y:" + String(y) + ", x:" + String(x));
  for (uint8_t k = 0; k < NUMBER_OF_KEYS; k++) {
    uint16_t x_pos = TFT_keyboard[k].x_pos * 10;
    uint16_t y_pos = TFT_keyboard[k].y_pos * 10;
    uint16_t x_width = TFT_keyboard[k].x_width * 10;
    if ((x > x_pos) && (x < (x_pos + x_width)) && (y > y_pos) && (y < y_pos + KEY_HEIGHT)) {
      // Key_pressed
      bool update_text_entry = true;
      switch (TFT_key_char(k)) {
        case OSK_KEY_BACKSPACE:
          if ((Main_menu_cursor < Text_entry_length) || (Text_entry[Text_entry_length - 1] == ' ')) {
            cursor_left_page_name();
            for (uint8_t c = Main_menu_cursor - 1; c < Text_entry_length - 1; c++) Text_entry[c] = Text_entry[c + 1];
          }
          Text_entry[Text_entry_length - 1] = ' ';
          break;
        case OSK_KEY_ENTER:
          TFT_exit_on_screen_keyboard(true);
          update_text_entry = false;
          break;
        case OSK_KEY_SHIFT:
          if (current_keyboard == KEYBOARD_CAPS) current_keyboard = KEYBOARD_NO_CAPS;
          else if (current_keyboard == KEYBOARD_NO_CAPS) current_keyboard = KEYBOARD_CAPS;
          else if (current_keyboard == KEYBOARD_NUM1) current_keyboard = KEYBOARD_NUM2;
          else current_keyboard = KEYBOARD_NUM1;

          TFT_draw_keyboard(true);
          break;
        case OSK_KEY_NUM:
          prev_keyboard = current_keyboard;
          current_keyboard = KEYBOARD_NUM1;
          TFT_draw_keyboard(true);
          break;
        case OSK_KEY_ABC:
          current_keyboard = prev_keyboard;
          TFT_draw_keyboard(true);
          break;
        case OSK_KEY_LEFT:
          cursor_left_page_name();
          break;
        case OSK_KEY_RIGHT:
          cursor_right_page_name();
          break;
        case OSK_KEY_ESCAPE:
          TFT_exit_on_screen_keyboard(false);
          update_text_entry = false;
          break;
        case OSK_KEY_CLEAR:
          for (uint8_t c = 0; c < Text_entry_length; c++) Text_entry[c] = ' ';
          Main_menu_cursor = 1;
          break;
        default:
          for (uint8_t c = Text_entry_length - 1; c -- > Main_menu_cursor - 1;) Text_entry[c + 1] = Text_entry [c];
          Text_entry[Main_menu_cursor - 1] = TFT_key_char(k);
          cursor_right_page_name();
          break;
      }
      if (update_text_entry) TFT_print_text_entry();
    }
  }

  // Select cursor position in text entry line
  if ((y > 25) && (y < 90)) {
    int pos = ((x - 640) / 32) + (Text_entry_length / 2);
    Serial.println("pos:" + String(pos));
    if ((pos > 0) && (pos < (Text_entry_length + 1))) {
      Main_menu_cursor = pos;
      TFT_print_text_entry();
    }
  }

  // Exit when pressing outside keyboard
  if ((x < LEFT_KEYBOARD_EDGE) || (x > RIGHT_KEYBOARD_EDGE) || (y < TOP_KEYBOARD_EDGE)) TFT_exit_on_screen_keyboard(false);
}

char TFT_key_char(uint8_t key) {
  switch (current_keyboard) {
    case KEYBOARD_CAPS: return TFT_keyboard[key].key1;
    case KEYBOARD_NO_CAPS: return TFT_keyboard[key].key2;
    case KEYBOARD_NUM1: return TFT_keyboard[key].key3;
    case KEYBOARD_NUM2: return TFT_keyboard[key].key4;
    default: return ' ';
  }
}


#define PIC_HEIGHT 100
#define PIC_WIDTH 160
#define PIC_X 50
#define PIC_Y 110
void TFT_update_device_pic() {
  uint8_t device_for_pic = Current_device;
  if ((Current_page == PAGE_MENU) && (!device_menu_active())) device_for_pic = 255;
  if (TFT_current_device_for_pic == device_for_pic) return;
  if (Current_device > + NUMBER_OF_DEVICES) return;

  TFT_current_device_for_pic = device_for_pic;

  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(layer1_start_addr);
  tft.Main_Image_Width(400);

  tft.Main_Window_Start_XY(0, 0);

  tft.Canvas_Image_Start_address(layer1_start_addr);//
  tft.Main_Image_Width(400);
  tft.Active_Window_XY(PIC_Y, PIC_X);
  tft.Active_Window_WH(PIC_HEIGHT, PIC_WIDTH);
  tft.Goto_Pixel_XY(PIC_Y, PIC_X);
  if (device_for_pic < 255) tft.Show_picture(PIC_HEIGHT * PIC_WIDTH, Device[Current_device]->device_pic);
  else tft.Show_picture(PIC_HEIGHT * PIC_WIDTH, img_VC_touch);

  tft.Active_Window_XY(0, 0);
  tft.Active_Window_WH(400, 1280);
}

#ifdef USE_TFT_USER_FONT
void TFT_show_user_font(const uint8_t * font, String text, uint16_t x, uint16_t y, uint16_t front_colour, uint16_t back_colour, bool compress) {
  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(0);
  tft.Main_Image_Width(400);
  tft.Main_Window_Start_XY(0, 0);

  uint8_t len = text.length();

  uint8_t font_height = font[0];
  uint8_t font_width = font[1];
  uint8_t first_char = font[2];
  uint8_t number_of_char = font[3];

  for (uint8_t c = 0; c < len; c++) {
    char my_char = text[c];
    if (((uint8_t) my_char >= first_char) && ((uint8_t) my_char <= (first_char + number_of_char))) {
      // Show character
      tft.BTE_Destination_Color_16bpp();
      tft.BTE_Destination_Memory_Start_Address(layer1_start_addr);
      tft.BTE_Destination_Image_Width(400);
      tft.BTE_Destination_Window_Start_XY(y, x);
      tft.BTE_Window_Size(font_height, font_width);
      tft.Foreground_color_65k(front_colour);
      tft.Background_color_65k(back_colour);
      tft.BTE_ROP_Code(15);
      tft.BTE_Operation_Code(8); //BTE color expansion
      //tft.BTE_Operation_Code(9); //BTE color expansion with chroma key

      tft.BTE_Enable();

      uint16_t font_index = (((uint8_t) my_char - first_char) * font_height * font_width / 8) + 4;

      tft.Show_picture(font_width * font_height / 8, &font[font_index]);
      tft.Check_Mem_WR_FIFO_Empty();
      tft.Check_BTE_Busy();

      x += font_width;
      if (compress) x--;
    }
  }
}

/*bool TFT_check_mem_chip() {
  bool TFT_mem_check = true;
  uint8_t data = 0;
  TFT_read_write_sdram(&data, true);
  data = 0xFF;
  TFT_read_write_sdram(&data, false);
  DEBUGMSG("MEMCHECK - data #1: " + String(data));
  if (data != 0x00) TFT_mem_check = false;
  data = 0xFF;
  TFT_read_write_sdram(&data, true);
  data = 0;
  TFT_read_write_sdram(&data, false);
  DEBUGMSG("MEMCHECK - data #2: " + String(data));
  if (data != 222) TFT_mem_check = false;
  return TFT_mem_check;
  }

  void TFT_read_write_sdram(uint8_t *data, bool do_write) {
  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(layer6_start_addr);
  tft.Main_Image_Width(1);

  tft.Main_Window_Start_XY(0, 0);

  tft.Canvas_Image_Start_address(layer1_start_addr);//
  tft.Main_Image_Width(1);
  tft.Active_Window_XY(0, 0);
  tft.Active_Window_WH(1, 1);
  tft.Goto_Pixel_XY(0, 0);
  delay(1);
  if (do_write) tft.Show_picture(1, data);
  else tft.Read_picture(1, data);
  delay(1);
  }*/

// Optional code to diagonally mirror the fonts from http://www.rinkydinkelectronics.com/r_fonts.php
void TFT_fix_rinkydink_font(const uint8_t * font) {
  uint8_t font_width = font[0];
  uint8_t font_height = font[1];
  uint8_t first_char = font[2];
  uint8_t number_of_char = font[3];

  // Write header
  Serial.println(" ");
  Serial.println("const uint8_t Font" + String(font_width) + "x" + String(font_height) + "[" + String(font_width * font_height * number_of_char / 8 + 4) + "] PROGMEM = {");
  write_hex_code(font_height);
  write_hex_code(font_width);
  write_hex_code(first_char);
  write_hex_code(number_of_char);
  Serial.println(" ");

  for (uint8_t c = 0; c < number_of_char; c++) {
    uint16_t font_index = (c * font_height * font_width / 8) + 4;

    uint8_t font_mirror[font_width * font_height / 8] = { 0 };
    for (uint16_t b = 0; b < font_width * font_height; b++) {
      uint8_t line = b % font_height;
      uint8_t pixel = b / font_height;
      uint16_t my_byte = (line * font_width + pixel) / 8;
      uint16_t my_bit = (line * font_width + pixel) % 8;
      if (my_byte < (font_width * font_height / 8)) {
        bool font_bit = bitRead(font[font_index + my_byte], 7 - my_bit);
        if (font_bit == true) bitSet(font_mirror[b / 8], 7 - (b % 8));
      }
    }

    for (uint16_t i = 0; i < (font_height * font_width / 8); i++) {
      write_hex_code(font_mirror[i]);
    }
    Serial.print("// ");
    char ch = (char) (c + first_char);
    switch (ch) {
      case ' ':
        Serial.println("space");
        break;
      case '/':
        Serial.println("slash");
        break;
      case '\\':
        Serial.println("backslash");
        break;
      default:
        Serial.println(ch);
        break;
    }
  }

  Serial.println("};");
  Serial.println(" ");
}

void write_hex_code(uint8_t number) {
  Serial.print("0x");
  if (number >= 16) Serial.print(String(number >> 4, HEX));
  else Serial.print("0");
  Serial.print(String(number & 0xF, HEX));
  Serial.print(", ");
}


// ********************************* Section 8: TFT Pong game ********************************************

// Thanks to Chinzin Nazar, https://hackernoon.com/how-i-developed-the-classic-pong-game-on-an-arduino-board
// GAME vars:

#define PONG_FIELD_X_TOP 256
#define PONG_FIELD_Y_TOP 0
#define PONG_FIELD_X_BOTTOM 1023
#define PONG_FIELD_Y_BOTTOM 398
#define PONG_BAT_WIDTH 8
#define PONG_BAT_HEIGHT 40
#define PONG_BAT_X_OFFSET 50
#define PONG_BALL_RADIUS 5
#define PONG_SCORE_MARGIN 25

#define TFT_PONG_CLEAR_HINT 10000

// scores:
int player_score;
int enemy_score;

// player:
const int player_position_X = PONG_FIELD_X_TOP + PONG_BAT_X_OFFSET; // static
int player_position_Y, player_new_position_Y;
uint32_t player_last_move_time;
uint32_t player_speed_of_moving; // update time in ms

// enemy:
const int enemy_position_X = PONG_FIELD_X_BOTTOM - PONG_BAT_X_OFFSET - PONG_BAT_WIDTH; // static
int enemy_position_Y;
uint8_t enemy_skill;
int enemy_random_deviation;

uint32_t enemy_last_move_time;
uint32_t enemy_speed_of_moving; // update time in ms

// ball:
int ball_position_X;
float ball_position_Y;
int ball_direction_X;
float ball_direction_Y;
int pong_speed;
uint32_t ball_last_move_time;
uint32_t ball_speed_of_moving;
float ball_acceleration;
int hint_position;
bool hint_as_note;

void TFT_pong_startup() {
  pong_active = true;
  player_score = 0;
  player_speed_of_moving = 2000;
  enemy_score = 0;
  enemy_speed_of_moving = 20000; //update time in ms
  pong_speed = 1;
  enemy_skill = 5;
  enemy_random_deviation = 0;
  ball_speed_of_moving = 20000;
  player_position_Y = PONG_FIELD_Y_TOP + 1;
  player_new_position_Y = player_position_Y;
  player_last_move_time = 0;
  enemy_position_Y = PONG_FIELD_Y_BOTTOM - PONG_BAT_HEIGHT - 1;
  enemy_last_move_time = 0;
  ball_position_X = PONG_FIELD_X_BOTTOM - PONG_BAT_X_OFFSET - PONG_BAT_WIDTH - (PONG_BALL_RADIUS * 2);
  ball_position_Y = (PONG_FIELD_Y_BOTTOM - PONG_FIELD_Y_TOP) / 2.0 + PONG_FIELD_Y_TOP;
  ball_direction_X = -3;
  TFT_pong_restart_game();
  hint_position = TFT_pong_predict_position();
  hint_as_note = true;
  //TFT_pong_draw_bat(hint_position, player_position_X - 40, Blue);
  TFT_pong_show_hint(hint_position);
}

void TFT_pong_restart_game() {
  // ball:

  ball_direction_Y = map(generate_random_number(), 0, 127, -50, 50);
  ball_last_move_time = micros() + 3000000; // Wait three seconds before starting
  ball_acceleration = 1;
  //enemy_last_move_time = ball_last_move_time;

  TFT_pong_draw_field();
  TFT_pong_draw_center_line();
  TFT_pong_show_score(player_score, PONG_FIELD_X_TOP - 128 - PONG_SCORE_MARGIN);
  TFT_pong_show_score(enemy_score, PONG_FIELD_X_BOTTOM + PONG_SCORE_MARGIN);
  TFT_pong_show_speed();
  TFT_pong_move_player_bat();
  TFT_pong_move_enemy_bat();
}

void TFT_pong_close() {
  TFT_fillScreen(Black);
  TFT_draw_main_grid();
  LCD_clear_memory();
  pong_active = false;
  TFT_current_device_for_pic = 255; // Triggers a refresh of the device pic
  update_page = RELOAD_PAGE;
  update_main_lcd = true;
}

void TFT_pong_update() {
  TFT_pong_move_player_bat();
  TFT_pong_move_enemy_bat();
  TFT_pong_move_ball();
}

void TFT_pong_switch_pressed(uint8_t sw) {
  if (SC_switch_is_expr_pedal()) {
    player_new_position_Y = map(Expr_ped_value, 127, 0, PONG_FIELD_Y_TOP + 1, PONG_FIELD_Y_BOTTOM - PONG_BAT_HEIGHT - 1);
    hint_as_note = false;
  }
  if ((sw == 1) && (pong_speed > 1)) {
    pong_speed--;
    player_speed_of_moving /= 0.8;
    enemy_speed_of_moving /= 0.8;
    ball_speed_of_moving /= 0.8;
    TFT_pong_show_speed();
  }
  if ((sw == 6) && (pong_speed < 9)) {
    pong_speed++;
    player_speed_of_moving *= 0.8;
    enemy_speed_of_moving *= 0.8;
    ball_speed_of_moving *= 0.8;
    TFT_pong_show_speed();
  }
  if ((sw >= 11) && (sw <= 15)) TFT_pong_close();
}

#define PONG_NOTE_MIN 40
#define PONG_NOTE_MAX 76

void TFT_pong_receive_note_on(byte channel, byte note, byte velocity) {
  if (velocity > 0) {
    if (note < PONG_NOTE_MIN) note = PONG_NOTE_MIN;
    if (note > PONG_NOTE_MAX) note = PONG_NOTE_MAX;
    player_new_position_Y = map(note, PONG_NOTE_MAX, PONG_NOTE_MIN, PONG_FIELD_Y_TOP + 1, PONG_FIELD_Y_BOTTOM - PONG_BAT_HEIGHT - 1);
    hint_as_note = true;
  }
}

void TFT_pong_move_player_bat() {
  if (micros() > player_speed_of_moving + player_last_move_time) {
    TFT_pong_draw_bat(player_position_Y, player_position_X, Black);
    if (player_new_position_Y < player_position_Y) {
      player_position_Y --;
    }
    if (player_new_position_Y > player_position_Y) {
      player_position_Y ++;
    }
    TFT_pong_draw_bat(player_position_Y, player_position_X, White);
    player_last_move_time = micros();
  }
}

void TFT_pong_move_enemy_bat() {
  if (micros() > enemy_speed_of_moving + enemy_last_move_time) {
    TFT_pong_draw_bat(enemy_position_Y, enemy_position_X, Black);

    int pos;
    pos = ball_position_Y - PONG_FIELD_Y_TOP;
    if (ball_direction_X > 0) { 
      pos = abs(ball_position_Y + (enemy_position_X - ball_position_X) / ball_direction_X * (ball_direction_Y / 10.0));
      if (pos < (PONG_FIELD_Y_BOTTOM - PONG_FIELD_Y_TOP)) pos %= (PONG_FIELD_Y_BOTTOM - PONG_FIELD_Y_TOP);
      else {
        pos %= (PONG_FIELD_Y_BOTTOM - PONG_FIELD_Y_TOP) ;
        pos = PONG_FIELD_Y_BOTTOM - pos;
      }
      pos += enemy_random_deviation * (9 - enemy_skill);
    }
    else { // Follow the ball
      pos = ball_position_Y;
    }
    if (pos < enemy_position_Y + PONG_BAT_HEIGHT / 2) {
      enemy_position_Y --;
    }
    if (pos > enemy_position_Y + PONG_BAT_HEIGHT / 2) {
      enemy_position_Y ++;
    }

    //checking if enemy is within the wall:
    if (enemy_position_Y > PONG_FIELD_Y_BOTTOM - PONG_BAT_HEIGHT) enemy_position_Y = PONG_FIELD_Y_BOTTOM - PONG_BAT_HEIGHT;
    if (enemy_position_Y < PONG_FIELD_Y_TOP + 1) enemy_position_Y = PONG_FIELD_Y_TOP + 1;

    TFT_pong_draw_bat(enemy_position_Y, enemy_position_X, White);
    enemy_last_move_time = micros();
  }
}

void TFT_pong_move_ball() {
  if (micros() > ((ball_speed_of_moving * ball_acceleration * (5 + abs(ball_direction_Y / 10)) / 5) + ball_last_move_time)) {
    //erase ball on old position:
    TFT_pong_draw_ball(ball_position_Y, ball_position_X, Black);
    TFT_pong_draw_center_line();

    //set new posion of the ball:
    ball_position_X += ball_direction_X;

    if (ball_position_Y + (ball_direction_Y / 10.0) <= PONG_FIELD_Y_TOP) ball_direction_Y *= -1;
    if (ball_position_Y + (ball_direction_Y / 10.0) >= PONG_FIELD_Y_BOTTOM - (2 * PONG_BALL_RADIUS)) ball_direction_Y *= -1;

    ball_position_Y += (ball_direction_Y / 10.0);

    //draw ball on new position:
    TFT_pong_draw_ball(ball_position_Y, ball_position_X, White);
    ball_last_move_time = micros();

    //Check for player loose:
    if (ball_position_X <= PONG_FIELD_X_TOP) {
      TFT_pong_new_round(1); // enemy wins
    }
    //check for collision of the ball and the player:
    if ((ball_position_X < player_position_X + PONG_BAT_WIDTH) && (ball_position_X > player_position_X)
        && (ball_position_Y + (2 * PONG_BALL_RADIUS) > player_position_Y) && (ball_position_Y < player_position_Y + PONG_BAT_HEIGHT)) {
      //send the ball to enemy with random values:
      ball_direction_X = 3;
      ball_direction_Y = map(generate_random_number(), 0, 127, -50, 50);
      ball_acceleration *= 0.9;
      TFT_pong_show_hint(TFT_PONG_CLEAR_HINT);
    }
    //check for enemy loose:
    if (ball_position_X >= PONG_FIELD_X_BOTTOM - (2 * PONG_BALL_RADIUS)) {
      TFT_pong_new_round(0); // player wins
    }
    //check for collision of the ball and the enemy:
    if ((ball_position_X + (2 * PONG_BALL_RADIUS) >= enemy_position_X) && (ball_position_X + (2 * PONG_BALL_RADIUS) <= enemy_position_X + PONG_BAT_WIDTH)
        && (ball_position_Y + (2 * PONG_BALL_RADIUS) > enemy_position_Y) && (ball_position_Y < enemy_position_Y + PONG_BAT_HEIGHT)) {
      //send the ball to player with random values:
      ball_direction_X = -3;
      ball_direction_Y = map(generate_random_number(), 0, 127, -50, 50);
      enemy_random_deviation = map(generate_random_number(), 0, 127, -10, 10);
      //TFT_pong_draw_bat(hint_position, player_position_X - 40, Black);
      hint_position = TFT_pong_predict_position();
      //TFT_pong_draw_bat(hint_position, player_position_X - 40, Blue);
      if (micros() > ball_last_move_time - 2000000) TFT_pong_show_hint(hint_position);
    }
  }
}

uint16_t TFT_pong_predict_position() {
  float y = (float) ball_position_Y;
  float y_dir = (float) ball_direction_Y / 10.0;
  for (uint16_t x = player_position_X + PONG_BAT_WIDTH; x < enemy_position_X - (2 * PONG_BALL_RADIUS); x += 3) {
    y += y_dir;
    if (y + y_dir <= PONG_FIELD_Y_TOP) y_dir *= -1.0;
    if (y + y_dir >= PONG_FIELD_Y_BOTTOM - (2 * PONG_BALL_RADIUS)) y_dir *= -1.0;
  }
  if (y > (PONG_BAT_HEIGHT / 2)) y -= PONG_BAT_HEIGHT / 2;
  return y;
}

void TFT_pong_new_round(uint8_t winner) {
  //Update scores:
  if (winner == 1) { // Enemy wins
    enemy_score++;
    ball_position_X = PONG_FIELD_X_BOTTOM - PONG_BAT_X_OFFSET - (PONG_BALL_RADIUS * 2); // Ball comes from enemy
    ball_position_Y = enemy_position_Y + (PONG_BAT_HEIGHT / 2);
    ball_direction_X = -3;
    if (enemy_skill > 0) enemy_skill--;
  } else {
    player_score++;
    ball_position_X = PONG_FIELD_X_TOP + PONG_BAT_X_OFFSET + PONG_BAT_WIDTH; // Ball comes from player
    ball_position_Y = player_position_Y + (PONG_BAT_HEIGHT / 2);
    ball_direction_X = 3;
    if (enemy_skill < 9) enemy_skill++;
  }
  TFT_pong_show_score(player_score, PONG_FIELD_X_TOP - 153);
  TFT_pong_show_score(enemy_score, PONG_FIELD_X_BOTTOM + 25);
  TFT_pong_restart_game();

  //checking for if we need to update enemy_speed_of_moving and ball_speed
  if (((player_score + enemy_score) % 5 == 0) && (pong_speed < 9)) {
    pong_speed++;
    player_speed_of_moving *= 0.8;
    enemy_speed_of_moving *= 0.8;
    ball_speed_of_moving *= 0.8;
    TFT_pong_show_speed();
  }
  //if (winner == 0) enemy_speed_of_moving *= 0.95;
  //else enemy_speed_of_moving /= 0.93;
}

void TFT_pong_draw_field() {
  TFT_fillScreen(Black);
  tft.Canvas_Image_Start_address(layer1_start_addr);
  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(layer1_start_addr);
  tft.Main_Image_Width(400);
  tft.Main_Window_Start_XY(0, 0);
  tft.Foreground_color_65k(Grey);
  tft.Line_Start_XY(PONG_FIELD_Y_TOP, PONG_FIELD_X_TOP);
  tft.Line_End_XY(PONG_FIELD_Y_BOTTOM, PONG_FIELD_X_BOTTOM);
  tft.Start_Square();
}

void TFT_pong_draw_center_line() {
  tft.Canvas_Image_Start_address(layer1_start_addr);
  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(layer1_start_addr);
  tft.Main_Image_Width(400);
  tft.Main_Window_Start_XY(0, 0);
  tft.Foreground_color_65k(Grey);
  tft.Line_Start_XY(PONG_FIELD_Y_TOP, (PONG_FIELD_X_BOTTOM + PONG_FIELD_X_TOP) / 2);
  tft.Line_End_XY(PONG_FIELD_Y_BOTTOM, (PONG_FIELD_X_BOTTOM + PONG_FIELD_X_TOP) / 2);
  tft.Start_Line();
}

void TFT_pong_draw_bat(uint16_t y, uint16_t x, uint16_t colour) {
  tft.Canvas_Image_Start_address(layer1_start_addr);
  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(layer1_start_addr);
  tft.Main_Image_Width(400);
  tft.Main_Window_Start_XY(0, 0);
  tft.Foreground_color_65k(colour);
  tft.Line_Start_XY(y, x);
  tft.Line_End_XY(y + PONG_BAT_HEIGHT, x + PONG_BAT_WIDTH);
  tft.Start_Square_Fill();
}

void TFT_pong_draw_ball(uint16_t y, uint16_t x, int16_t colour) {
  tft.Canvas_Image_Start_address(layer1_start_addr);
  tft.Select_Main_Window_16bpp();
  tft.Main_Image_Start_Address(layer1_start_addr);
  tft.Main_Image_Width(400);
  tft.Main_Window_Start_XY(0, 0);
  tft.Foreground_color_65k(colour);
  tft.Circle_Center_XY(PONG_BALL_RADIUS + y, PONG_BALL_RADIUS + x);
  tft.Circle_Radius_R(PONG_BALL_RADIUS);
  tft.Start_Circle_or_Ellipse_Fill();
}

void TFT_pong_show_score(int value, int x) {
  String score = "";
  LCD_add_3digit_number(value, score);
#ifdef USE_TFT_USER_FONT
  TFT_show_user_font(Grotesk32x64, score, x, PONG_FIELD_Y_TOP + 10, Yellow, Black, false);
#else
  char text[3];
  score.toCharArray(text, 4);
  tft.Canvas_Image_Start_address(layer1_start_addr);
  tft.Foreground_color_65k(Yellow);
  tft.Background_color_65k(Black);
  tft.CGROM_Select_Internal_CGROM();
  tft.Font_Select_16x32_32x32();
  tft.Font_Width_X2();
  tft.Font_Height_X2();
  tft.Goto_Text_XY(PONG_FIELD_Y_TOP + 10, x);
  tft.Show_String(text);
#endif
}

void TFT_pong_show_speed() {
  String score = "SPEED: " + String(pong_speed);
#ifdef USE_TFT_USER_FONT
  TFT_show_user_font(Grotesk16x32, score, PONG_FIELD_X_TOP - 138, PONG_FIELD_Y_BOTTOM - 42, Yellow, Black, false);
#else
  char text[9];
  score.toCharArray(text, 9);
  tft.Canvas_Image_Start_address(layer1_start_addr);
  tft.Foreground_color_65k(Yellow);
  tft.Background_color_65k(Black);
  tft.CGROM_Select_Internal_CGROM();
  tft.Font_Select_16x32_32x32();
  tft.Font_Width_X2();
  tft.Font_Height_X2();
  tft.Goto_Text_XY(PONG_FIELD_Y_BOTTOM - 42, PONG_FIELD_X_TOP - 128 - PONG_SCORE_MARGIN);
  tft.Show_String(text);
#endif
}

void TFT_pong_show_hint(int value) {
  String hint;
  if (hint_as_note) {
    uint8_t note_val = map(value, PONG_FIELD_Y_TOP, PONG_FIELD_Y_BOTTOM - PONG_BAT_HEIGHT, PONG_NOTE_MAX, PONG_NOTE_MIN) - 24;
    const char note_name[12][3] = { "C ", "C#", "D ", "Eb", "E ", "F ", "F#", "G ", "G#", "A ", "Bb", "B " };
    hint = note_name[note_val % 12] + String(note_val / 12);
  }
  else {
    uint8_t percentage = map(value, PONG_FIELD_Y_TOP, PONG_FIELD_Y_BOTTOM - PONG_BAT_HEIGHT, 99, 0);
    //if (percentage < 100) 
    hint = String(percentage / 10) + String(percentage % 10) + "%";
  }
  if (value == TFT_PONG_CLEAR_HINT) hint = "   ";
#ifdef USE_TFT_USER_FONT
  TFT_show_user_font(Grotesk32x64, hint, PONG_FIELD_X_BOTTOM + PONG_SCORE_MARGIN, PONG_FIELD_Y_BOTTOM - 74, Yellow, Black, false);
#else
  char text[3];
  hint.toCharArray(text, 4);
  tft.Canvas_Image_Start_address(layer1_start_addr);
  tft.Foreground_color_65k(Yellow);
  tft.Background_color_65k(Black);
  tft.CGROM_Select_Internal_CGROM();
  tft.Font_Select_16x32_32x32();
  tft.Font_Width_X2();
  tft.Font_Height_X2();
  tft.Goto_Text_XY(PONG_FIELD_Y_BOTTOM - 74, PONG_FIELD_X_BOTTOM + PONG_SCORE_MARGIN);
  tft.Show_String(text);
#endif
}

#endif

#endif
