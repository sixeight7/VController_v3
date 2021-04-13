// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: LCD Hardware Initialization
// Section 2: Update of main LCD
// Section 3: Update of individual LCDs
// Section 4: LCD Functions
// Section 5: Virtual LED functions

// Functions for LCD control
// Main LCD is a large 16x2 LCD display with a serial i2c module attached
// Futhermore we have 12 16x2 LCD displays for the bottom 12 switches

// ********************************* Section 1: LCD Hardware Initialization ********************************************
// Hardware setting have been moved to hardware.h

//#include <Wire.h>
#include <i2c_t3.h>
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

// Main display modes - stored in Setting.Main_display_mode
#define MD_SHOW_PAGE_NAME 0
#define MD_SHOW_CURRENT_PATCH 1
#define MD_SHOW_PATCHES_COMBINED 2
#define MD_SHOW_3_SWITCH_LABELS 3 // Made for VC mini to display switch labels

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
unsigned long ledbarTimer = 0;
bool popup_title_showing = false;
bool popup_label_showing = false;
bool main_ledbar_showing = false;
uint8_t ledbar_showing = 0;

String Current_patch_number_string = "";
String Current_page_name = "";
String Current_device_name = "";
//String Main_menu_line1 = ""; // Text that is show on the main display from the menu on line 1
//String Main_menu_line2 = ""; // Text that is show on the main display from the menu on line 2
//String topline;
char main_lcd_title[MAIN_LCD_DISPLAY_SIZE + 1]; // Text that is show on the main display from the menu on line 1
char main_lcd_label[MAIN_LCD_DISPLAY_SIZE + 1]; // Text that is show on the main display from the menu on line 2
uint8_t Main_menu_cursor = 0; // Position of main menu cursor. Zero = off.
char lcd_title[LCD_DISPLAY_SIZE + 1]; // char array reserved for individual display titles
char lcd_label[LCD_DISPLAY_SIZE + 1]; // char array reserved for individual display labels
char lcd_mem[NUMBER_OF_DISPLAYS + 1][2][LCD_DISPLAY_SIZE + 1]; // Memory for individual displays

String Combined_patch_name; // Patchname displayed in the main display
uint16_t Current_patch_number = 0;              // Patchnumber displayed in the main display

String Display_number_string; // Placeholder for patchnumbers on display
uint8_t my_looper_lcd = 0; // The LCD that will show the looper progress bar (0 is no display)
bool backlight_on[NUMBER_OF_DISPLAYS] = {false}; // initialize all backlights off

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
#define CHAR_ARROW_UP 5
#define CHAR_ARROW_DOWN 6
#define CHAR_VLED 7
#define CHAR_QUARTER_NOTE 7

void setup_LCD_control()
{
  // Set i2C speed

  //Wire.begin();

  // Initialize main LCD
  Main_lcd.begin (16, 2); // Main LCD is 16x2
#ifdef PRIMARY_MAIN_DISPLAY_ADDRESS
  if (!Main_lcd.check_initialized()) { // Check if i2c address is found on primary address
    Main_lcd.set_addr(SECONDARY_MAIN_DISPLAY_ADDRESS); // If not try on secondary address
    Main_lcd.begin (16, 2); // Main LCD is 16x2
  }
  Main_lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  Main_lcd.setBacklight(true); // Backlight state is the same as initial on or off state...
#endif


  // Show startup screen
  LCD_show_startup_message();

  Main_lcd.createChar(CHAR_BLOCK1, block1);
  Main_lcd.createChar(CHAR_BLOCK2, block2);
  Main_lcd.createChar(CHAR_BLOCK3, block3);
  Main_lcd.createChar(CHAR_BLOCK4, block4);
  Main_lcd.createChar(CHAR_BLOCK5, block5);
  Main_lcd.createChar(CHAR_ARROW_UP, arrow_up);
  Main_lcd.createChar(CHAR_ARROW_DOWN, arrow_down);
  Main_lcd.createChar(CHAR_QUARTER_NOTE, qnote);

  memset(main_lcd_title, ' ', sizeof(main_lcd_title)); // Clear topline of main lcd

#ifndef VCMINI_DISPLAYS
  // Initialize individual LCDs - same for both methods
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

  // Show startup screen
  LCD_show_startup_credits();
}

// ********************************* Section 2: Update of main LCD ********************************************

void main_LCD_control()
{
  if (update_main_lcd == true) {
    update_main_lcd = false;
    LCD_update_main_display();
  }

  if ((popup_label_showing) || (popup_title_showing) || (main_ledbar_showing)) {
    if (millis() > messageTimer) { // Check if message is still showing
      popup_title_showing = false;
      popup_label_showing = false;
      main_ledbar_showing = false;
      update_main_lcd = true; // Now update the main display, so the status message will be overwritten
    }
  }

  if ((ledbar_showing > 0) && (millis() > ledbarTimer)) {
    uint8_t ledbar_lcd = ledbar_showing;
    ledbar_showing = 0;
    LCD_update(ledbar_lcd, true); // Now update the ledbar display, so the ledbar will be overwritten
  }

  if (update_lcd > 0) {
    LCD_update(update_lcd, true);
    update_lcd = 0;
  }
}

#define PATCH_NUMBER_SEPERATOR "+"

void LCD_update_main_display() {

  if (!popup_title_showing) { // Clear title_string
    for (uint8_t i = 0; i < MAIN_LCD_DISPLAY_SIZE; i++) {
      main_lcd_title[i] = ' ';
    }
    
    if (global_tuner_active) { // Show tuner message if active
      LCD_main_set_title("  Tuner active  ");
      LCD_print_main_lcd_txt();
    }

    else if (Current_page == PAGE_MENU) { // Show menu content if active
      menu_set_main_title();
      LCD_print_main_lcd_txt();
      // Show cursor
      if (Main_menu_cursor > 0) {
        Main_lcd.setCursor (Main_menu_cursor - 1, 1);
        Main_lcd.cursor();
      }
      else Main_lcd.noCursor();
    }

    else { // Show patchnumers and device name on top line
      LCD_set_combined_patch_number_and_name();

      uint8_t ps_length = Current_patch_number_string.length();
      if (ps_length > MAIN_LCD_DISPLAY_SIZE) ps_length = MAIN_LCD_DISPLAY_SIZE;
      for (uint8_t i = 0; i < ps_length; i++) {
        main_lcd_title[i] = Current_patch_number_string[i];
      }

      String top_right = "";
      if (Setting.Main_display_show_top_right == 0) {
        if (Current_device < NUMBER_OF_DEVICES) {
          Current_device_name = Device[Current_device]->device_name;
          top_right = Current_device_name;
        }
      }

      if (Setting.Main_display_show_top_right == 1) {
        top_right = char(CHAR_QUARTER_NOTE);
        top_right += "=";
        top_right += String(Setting.Bpm);
      }

      if (Setting.Main_display_show_top_right == 2) {
        uint8_t sc = Device[Current_device]->current_snapscene;
        if (sc > 0) Device[Current_device]->get_snapscene_label(sc, top_right);
        else top_right = "";
        top_right.trim();
      }

      uint8_t name_length = top_right.length();
      if (ps_length < (MAIN_LCD_DISPLAY_SIZE - name_length)) { // Only show top right string if it fits
        for (uint8_t i = 0; i < name_length; i++) {
          main_lcd_title[MAIN_LCD_DISPLAY_SIZE - name_length + i] = top_right[i];
        }
      }
    }
  }

  // Show current page name, current patch or patches combined on second line
  if (!popup_label_showing) {
    if (Current_page == PAGE_MENU) return;
    
    for (uint8_t i = 0; i < MAIN_LCD_DISPLAY_SIZE; i++) {
      main_lcd_label[i] = ' ';
    }
    uint8_t main_display_mode = Setting.Main_display_mode;
#ifdef IS_VCMINI
    if (Current_page <= PAGE_SELECT) main_display_mode = MD_SHOW_3_SWITCH_LABELS; // Always show labels on VC-mini for menu and page select
#endif
    switch (main_display_mode) {
      case MD_SHOW_PAGE_NAME:
        EEPROM_read_title(Current_page, 0, Current_page_name); // Read page name from EEPROM
        LCD_main_set_label(Current_page_name);
        break;
      case MD_SHOW_CURRENT_PATCH:
        if (Current_device < NUMBER_OF_DEVICES) LCD_main_set_label(Device[Current_device]->current_patch_name);
        break;
      case MD_SHOW_PATCHES_COMBINED:
        LCD_main_set_label(Combined_patch_name); // Show the combined patchname
        break;
      case MD_SHOW_3_SWITCH_LABELS:
        LCD_show_3_switch_labels();
        break;
    }
  }
  LCD_print_main_lcd_txt();
}

void LCD_show_page_name() { // Will temporary show the name of the current page
  if (Setting.Main_display_mode == MD_SHOW_PAGE_NAME) return; // Do not show when display already shows the page name.
  LCD_update_main_display();
  EEPROM_read_title(Current_page, 0, Current_page_name); // Read page name from EEPROM
  LCD_show_popup_label(Current_page_name, ACTION_TIMER_LENGTH);
}

void LCD_set_combined_patch_number_and_name() {
  // Here we determine what to show on the main display, based on what devices are active
  uint8_t number_of_active_devices = 0;
  Current_patch_number_string = "";
  String patch_names[NUMBER_OF_DEVICES]; //Array of strings for the patchnames

  for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
    if (Device[d]->is_on) {
      Device[d]->display_patch_number_string(); // Adds the device name to patch number string
      Current_patch_number_string += PATCH_NUMBER_SEPERATOR; //Add a seperation sign in between
      patch_names[number_of_active_devices] = Device[d]->current_patch_name; //Add patchname to string
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

  // Set combined patchname
  switch (number_of_active_devices) {
    case 0:
      Combined_patch_name = "";
      break;
    case 1: // Only one device active
      Combined_patch_name = patch_names[0];
      break;
    case 2: // Show 7 bytes of both names
      Combined_patch_name = patch_names[0].substring(0, 7) + "  " + patch_names[1].substring(0, 7);
      break;
    case 3: // Show 4 bytes of both names
      Combined_patch_name = patch_names[0].substring(0, 4) + "  " + patch_names[1].substring(0, 4) + "  " + patch_names[2].substring(0, 4);
      break;
    default: // More then 3 devices
      Combined_patch_name = String(number_of_active_devices) + " devices on";
      break;
  }
}

void LCD_show_3_switch_labels() {
  String msg, outmsg;
  uint8_t l3 = (MAIN_LCD_DISPLAY_SIZE - 2) / 3;
  uint8_t l2 = (MAIN_LCD_DISPLAY_SIZE - l3 - 2) / 2;
  uint8_t l1 = MAIN_LCD_DISPLAY_SIZE - l3 - l2 - 2;
  LCD_load_short_message(1, outmsg);
  LCD_set_length(outmsg, l1);
  outmsg += '|';
  LCD_load_short_message(2, msg);
  LCD_set_length(msg, l2);
  outmsg += msg;
  outmsg += '|';
  LCD_load_short_message(3, msg);
  LCD_set_length(msg, l3);
  outmsg += msg;
  LCD_main_set_label(outmsg);
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
            Device[Dev]->number_format(SP[sw].PP_number, msg);
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
        msg = "[S";
        msg += String(SP[sw].PP_number);
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
        msg = 'P';
        LCD_add_3digit_number(SP[sw].PP_number, msg);
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
  LCD_main_set_label(message);
  LCD_print_main_lcd_txt();
  if (SC_switch_is_expr_pedal()) messageTimer = millis() + LEDBAR_TIMER_LENGTH;
  else messageTimer = millis() + time;
  popup_label_showing = true;
}

void LCD_show_popup_title(String message, uint16_t time)
// Will display a status message on the main display
{
  LCD_main_set_title(message);
  LCD_print_main_lcd_txt();
  if (SC_switch_is_expr_pedal()) messageTimer = millis() + LEDBAR_TIMER_LENGTH;
  else messageTimer = millis() + time;
  popup_title_showing = true;
}

void LCD_show_startup_message() {
  LCD_clear_main_lcd_txt();
  LCD_main_set_title("  V-controller  ");  // Show startup message
  LCD_show_popup_label(" version " + String(VCONTROLLER_FIRMWARE_VERSION_MAJOR) + "." + String(VCONTROLLER_FIRMWARE_VERSION_MINOR) + "." + String(VCONTROLLER_FIRMWARE_VERSION_BUILD) + "  ", MESSAGE_TIMER_LENGTH);  //Please give me the credits :-)
}

void LCD_show_startup_credits() {
  delay(800);
  LCD_show_popup_label("   by SixEight  ", MESSAGE_TIMER_LENGTH);  //Please give me the credits :-)
}

void LCD_clear_main_lcd_txt() {
  for (uint8_t i = 0; i < MAIN_LCD_DISPLAY_SIZE; i++) {
    main_lcd_title[i] = ' ';
    main_lcd_label[i] = ' ';
  }
}

void LCD_main_set_title(const String & msg) { // Will set the Title string in the SP array
  // Check length does not exceed LABEL_SIZE
  uint8_t msg_length = msg.length();
  if (msg_length > MAIN_LCD_DISPLAY_SIZE) msg_length = MAIN_LCD_DISPLAY_SIZE;
  for (uint8_t i = 0; i < msg_length; i++) {
    main_lcd_title[i] = msg[i];
  }
  for (uint8_t i = msg_length; i < MAIN_LCD_DISPLAY_SIZE; i++) { // Fill the remaining chars with spaces
    main_lcd_title[i] = ' ';
  }
}

void LCD_main_set_label(const String & msg) { // Will set the Title string in the SP array
  // Check length does not exceed LABEL_SIZE
  uint8_t msg_length = msg.length();
  if (msg_length > MAIN_LCD_DISPLAY_SIZE) msg_length = MAIN_LCD_DISPLAY_SIZE;
  for (uint8_t i = 0; i < msg_length; i++) {
    main_lcd_label[i] = msg[i];
  }
  for (uint8_t i = msg_length; i < MAIN_LCD_DISPLAY_SIZE; i++) { // Fill the remaining chars with spaces
    main_lcd_label[i] = ' ';
  }
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
const char LCD_Bank_Down[] = "BANK DOWN";
const char LCD_Bank_Up[] = "BANK UP";
const char LCD_Mute[] = "MUTE";
const char LCD_Tuner[] = "[GLOBAL TUNER]";
const char LCD_Tap_Tempo[] = "<TAP TEMPO>";
const char LCD_Set_Tempo[] = "<SET TEMPO>";
const char LCD_Page_Down[] = "<PAGE DOWN>";
const char LCD_Page_Up[] = "<PAGE UP>";
const char LCD_Looper[] = "<LOOPER>";
const char LCD_Page[] = "V-Controller";
const char LCD_Unknown[] = "Unknown";

void LCD_update(uint8_t sw, bool do_show) {
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
          Device[Dev]->number_format(SP[sw].PP_number, Display_number_string);
          if (SP[sw].PP_number == Device[Dev]->patch_number) {
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
          Device[Dev]->number_format(SP[sw].PP_number, Display_number_string);
          Display_number_string += ')';
          LCD_add_vled(2);
          LCD_add_title(Display_number_string);
          LCD_add_label(SP[sw].Label);
        }
        if (SP[sw].Sel_type == PREV) {
          Display_number_string = "PREV (";
          Device[Dev]->number_format(SP[sw].PP_number, Display_number_string);
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
        LCD_add_label(Device[Dev]->device_name);
        //LCD_print_lcd_txt(sw);
        break;
      case PAR_BANK_UP:
        LCD_add_vled(2);
        LCD_add_title(LCD_Bank_Up);
        LCD_add_label(Device[Dev]->device_name);
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
        if (SP[sw].Value2 == 0) {
          Device[Dev]->get_snapscene_title(SP[sw].PP_number, Display_number_string);
        }
        else if (SP[sw].Value3 == 0) { // Two snapshots in view
          LCD_add_snapshot_number(SP[sw].Value1, SP[sw].PP_number, Device[Dev]->current_snapscene, Display_number_string);
          LCD_add_snapshot_number(SP[sw].Value2, SP[sw].PP_number, Device[Dev]->current_snapscene, Display_number_string);
        }
        else { // Three snapshots in view
          LCD_add_snapshot_number(SP[sw].Value1, SP[sw].PP_number, Device[Dev]->current_snapscene, Display_number_string);
          LCD_add_snapshot_number(SP[sw].Value2, SP[sw].PP_number, Device[Dev]->current_snapscene, Display_number_string);
          LCD_add_snapshot_number(SP[sw].Value3, SP[sw].PP_number, Device[Dev]->current_snapscene, Display_number_string);
        }
        LCD_add_title(Display_number_string);
        LCD_add_label(SP[sw].Label);
        //LCD_print_lcd_txt(sw);
        break;
      case LOOPER:
        LCD_set_looper_title();
        LCD_add_label(SP[sw].Label);
        //LCD_print_lcd_txt(sw);
        break;
      case SAVE_PATCH:
        if (Dev == SY1000) LCD_add_title("<SCENE MENU>");
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
          LCD_add_label(SP[sw].Label);
        }
        if (SP[sw].Sel_type == NEXT) {
          LCD_add_vled(1);
          LCD_add_title(LCD_Page_Up);
          LCD_add_label(SP[sw].Label);
        }
        if (SP[sw].Sel_type == PREV) {
          LCD_add_vled(1);
          LCD_add_title(LCD_Page_Down);
          LCD_add_label(SP[sw].Label);
        }
        if (SP[sw].Sel_type == BANKSELECT) {
          Display_number_string = "PG ";
          LCD_add_3digit_number(SP[sw].PP_number, Display_number_string);
          LCD_add_vled(3);
          LCD_add_title(Display_number_string);
          LCD_add_label(SP[sw].Label);
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
        Display_number_string = "<PC ";
        LCD_add_3digit_number(SP[sw].PP_number, Display_number_string);
        Display_number_string += '>';
        LCD_add_vled(3);
        LCD_add_title(Display_number_string);
        LCD_add_label(SP[sw].Label);
        //LCD_print_lcd_txt(sw);
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

void LCD_add_snapshot_number(uint8_t number, uint8_t current_number, uint8_t current_snap, String & msg) {
  if (number == current_snap) msg += '[';
  else if (number == current_number) msg += '<';
  msg += "S";
  msg += String(number);
  if (number == current_snap) msg += ']';
  else if (number == current_number) msg += '>';
  msg += ' ';
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
  uint8_t len = lbl.length();
  if (len > LCD_DISPLAY_SIZE) len = LCD_DISPLAY_SIZE;
  while ((len > 1) && (lbl[len - 1] == ' ')) len--; //Find last character that is not a space
  uint8_t start_point = (LCD_DISPLAY_SIZE - len) >> 1;
  for (uint8_t i = 0; i < len; i++) { // Copy the lbl
    lcd_label[start_point + i] = lbl[i];
  }
}

void LCD_add_label(const char* lbl) {
  uint8_t len = strlen(lbl);
  if (len > LCD_DISPLAY_SIZE) len = LCD_DISPLAY_SIZE;
  while ((len > 1) && (lbl[len - 1] == ' ')) len--; //Find last character that is not a space
  uint8_t start_point = (LCD_DISPLAY_SIZE - len) >> 1;
  for (uint8_t i = 0; i < len; i++) { // Copy the title
    lcd_label[start_point + i] = lbl[i];
  }
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

  for (uint8_t i = 0; i < LCD_DISPLAY_SIZE; i++) { // Compare each character of source to the lcd_mem. Find first_char and last_char
    if (source[i] != lcd_mem[number][line][i]) {
      if (first_char == 255) first_char = i;
      last_char = i;
      lcd_mem[number][line][i] = source[i];
    }
  }

  if (first_char != 255) {
    if (number == 0) { // Update main display
      Main_lcd.setCursor (first_char, line); // Move cursor to first character that has changed on line 1
      for (uint8_t i = first_char; i <= last_char; i++) {  // Print the characters that have changed (one by one)
        Main_lcd.print(source[i]);
      }
    }
#ifndef VCMINI_DISPLAYS
    else { // Update individual display
      uint8_t lcd_no = number - 1;
      lcd[lcd_no].setCursor (first_char, line); // Move cursor to first character that has changed on line 1
      for (uint8_t i = first_char; i <= last_char; i++) {  // Print the characters that have changed (one by one)
        lcd[lcd_no].print(source[i]);
      }
    }
#endif
    return true;
  }
  return false;
}

void LCD_clear_memory() {
  memset(lcd_mem, 0, sizeof(lcd_mem)); // Fill lcd_line arrays with zeros
}

void LCD_show_bar(uint8_t lcd_no, uint8_t value) { // Will show the bar for the expression pedal on the top line of the main display
  char ledbar[17];
  value &= 0x7F; // Keep value below 0x80
  uint8_t full_blocks = (value >> 3); // Calculate the number of full blocks to display - value is between 0 and 127
  uint8_t part_block = (value & 7) * 6 >> 3; // Calculate which part block to display
  if (lcd_no == 0) {
    for (uint8_t i = 0; i < full_blocks; i++) ledbar[i] = char(4); // Display the full blocks
    if (part_block > 0) ledbar[full_blocks] = char(part_block - 1); // Display the correct part block
    else ledbar[full_blocks] = main_lcd_title[full_blocks];
    for (uint8_t i = full_blocks + 1; i < LCD_DISPLAY_SIZE; i++) ledbar[i] = main_lcd_title[i]; // Fill the rest with remainder of topline
    main_ledbar_showing = true;
    messageTimer = millis() + LEDBAR_TIMER_LENGTH;
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

void LCD_switch_on_backlight(uint8_t number) {
#ifndef VCMINI_DISPLAYS
  if (number < NUMBER_OF_DISPLAYS) {
    if (!backlight_on[number]) {
      lcd[number].setBacklight(HIGH);
      backlight_on[number] = true;
    }
  }
#endif
}

void LCD_switch_off_backlight(uint8_t number) {
#ifndef VCMINI_DISPLAYS
  if (number < NUMBER_OF_DISPLAYS) {
    if (backlight_on[number]) {
      lcd[number].setBacklight(LOW);
      backlight_on[number] = false;
    }
  }
#endif
}

void LCD_backlight_on() { // Will switch all backlights on
#ifndef VCMINI_DISPLAYS
  for (uint8_t i = 0; i < NUMBER_OF_DISPLAYS; i++) {
    LCD_switch_on_backlight(i);
  }
#endif
  Main_lcd.setBacklight(HIGH);
}

void LCD_backlight_off() { // Will switch all backlights off
#ifndef VCMINI_DISPLAYS
  for (uint8_t i = 0; i < NUMBER_OF_DISPLAYS; i++) {
    LCD_switch_off_backlight(i); //switch on the backlight
  }
#endif
  Main_lcd.setBacklight(LOW);
  Main_lcd.clear(); // Clear main display
}

void LCD_clear_all_displays() {
  LCD_clear_main_lcd_txt();
  LCD_clear_lcd_txt();
#ifndef VCMINI_DISPLAYS
  for (uint8_t i = 0; i < NUMBER_OF_DISPLAYS + 1; i++) {
    LCD_print_lcd_txt(i);
  }
#endif
}

void LCD_show_are_you_sure(String line1, String line2)
{

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
  LCD_print_lcd_txt(10);
  LCD_clear_lcd_txt();
  LCD_add_label(LBL_NO);
  LCD_print_lcd_txt(11);
#endif
}

void LCD_show_program_mode() {
  LCD_clear_all_displays();
  LCD_main_set_title("* PROGRAM MODE *");
  LCD_print_main_lcd_txt();
  LCD_add_title("Upload firmware ");
  LCD_add_label("from TeensyDuino");
  LCD_print_lcd_txt(10);
  LCD_clear_lcd_txt();
  LCD_add_title(" or power cycle ");
  LCD_add_label("the VController ");
  LCD_print_lcd_txt(11);
}

void LCD_number_to_note(uint8_t number, String & msg) {
  uint8_t note = number % 12;
  uint8_t octave = number / 12;
  const PROGMEM char notename[12][3] = {"C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B" };

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
#ifndef VCMINI_DISPLAYS
  if (number < NUMBER_OF_DISPLAYS) lcd[number].createChar(CHAR_VLED, vLED_off);
#endif
}

void LCD_set_virtual_LED(uint8_t number, uint8_t state) { // Will set the state of a virtual LED
  if (Display_LED[number] != state) { // Check if state is new
    Display_LED[number] = state; // Update state

#ifndef VCMINI_DISPLAYS
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
