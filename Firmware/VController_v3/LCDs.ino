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
LiquidCrystal_PCF8745	Main_lcd(PRIMARY_MAIN_DISPLAY_ADDRESS, EN_PIN, RW_PIN, RS_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN); //First number is the i2c address of the display

// Main display modes - stored in Setting.Main_display_mode
#define MD_SHOW_PAGE_NAME 0
#define MD_SHOW_CURRENT_PATCH 1
#define MD_SHOW_PATCHES_COMBINED 2

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

// Method 2: devclare displays using regular i2C backpacks if defined in hardware.h
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

#define MESSAGE_TIMER_LENGTH 1500 // time that status messages are shown (in msec)
unsigned long messageTimer = 0;
bool status_message_showing = false;

String Current_patch_number_string = "";
String Current_page_name = "";
String Current_device_name = "";
String Main_menu_line1 = ""; // Text that is show on the main display from the menu on line 1
String Main_menu_line2 = ""; // Text that is show on the main display from the menu on line 2
uint8_t Main_menu_cursor = 0; // Position of main menu cursor. Zero = off.

String Current_patch_name; // Patchname displayed in the main display
uint16_t Current_patch_number = 0;              // Patchnumber displayed in the main display

String Display_number_string[NUMBER_OF_DISPLAYS]; // Placeholder for patchnumbers on display
String Blank_line = "                ";
bool backlight_on[NUMBER_OF_DISPLAYS] = {false}; // initialize all backlights off

byte arrow_up[8] = { // The character for LED off
  0b00100,
  0b01110,
  0b11111,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00000
};

byte arrow_down[8] = { // The character for LED off
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

void setup_LCD_control()
{
  // Set i2C speed

  //Wire.begin();

  // Initialize main LCD
  Main_lcd.begin (16, 2); // Main LCD is 16x2
  if (!Main_lcd.check_initialized()) { // Check if i2c address is found on primary address
    Main_lcd.set_addr(SECONDARY_MAIN_DISPLAY_ADDRESS); // If not try on secondary address
    Main_lcd.begin (16, 2); // Main LCD is 16x2
  }
  Main_lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  Main_lcd.setBacklight(true); // Backlight state is the same as initial on or off state...

  // Create the characters for the expression pedal bar
  Main_lcd.createChar(0, block1);
  Main_lcd.createChar(1, block2);
  Main_lcd.createChar(2, block3);
  Main_lcd.createChar(3, block4);
  Main_lcd.createChar(4, block5);
  // Show startup screen
  LCD_show_startup_message();

  // Initialize individual LCDs - same for both methods
  for (uint8_t i = 0; i < NUMBER_OF_DISPLAYS; i++) {
    lcd[i].begin (16, 2);
    lcd[i].setBacklightPin(BACKLIGHT_PIN, POSITIVE);
    LCD_switch_on_backlight(i);
    LCD_init_virtual_LED(i); // Initialize the virtual LEDs
    lcd[i].createChar(1, arrow_up);
    lcd[i].createChar(2, arrow_down);
  }

  // Reserve space for Strings - this should avoid memory defragemntation
  Current_patch_number_string.reserve(17);
  Current_page_name.reserve(17);
  Current_device_name.reserve(17);
  Current_patch_name.reserve(17);
  Main_menu_line1.reserve(17);
  Main_menu_line2.reserve(17);
  Current_patch_name = "                ";
}

// ********************************* Section 2: Update of main LCD ********************************************

void main_LCD_control()
{
  if  ((update_main_lcd == true) && (status_message_showing == false)) {
    update_main_lcd = false;

    if (global_tuner_active) {
      Main_lcd.setCursor (0, 0);
      Main_lcd.print("  Tuner active  ");
      Main_lcd.setCursor (0, 1);
      Main_lcd.print("                ");
    }

    else if (Current_page == PAGE_MENU) {
      menu_set_main_title();
      Main_lcd.setCursor (0, 0);
      Main_lcd.print(Main_menu_line1);
      Main_lcd.setCursor (0, 1);
      Main_lcd.print(Main_menu_line2);
      // Show cursor
      if (Main_menu_cursor > 0) {
        Main_lcd.setCursor (Main_menu_cursor - 1, 1);
        Main_lcd.cursor();
      }
      else Main_lcd.noCursor();
    }

    else LCD_update_main_display();

  }

  if ((status_message_showing) && (millis() - messageTimer >= MESSAGE_TIMER_LENGTH)) {
    status_message_showing = false;
    update_main_lcd = true; // Now update the main display, so the status message will be overwritten
  }

  if (update_lcd > 0) {
    LCD_update(update_lcd);
    update_lcd = 0;
  }
}

#define PATCH_NUMBER_SEPERATOR "+"

void LCD_update_main_display() {
  Main_lcd.home();
  /*Current_page_name = ""; //Page[Current_page].Title;
  //PAGE_lookup_title(Current_page, Current_page_name);
  EEPROM_read_title(Current_page, 0, Current_page_name); // Page name from EEPROM

  Current_page_name.trim();
  Main_lcd.print(Blank_line.substring(0, 16 - Current_page_name.length()) + Current_page_name);*/

  if (Current_device < NUMBER_OF_DEVICES) {
    Current_device_name = Device[Current_device]->device_name;
    Main_lcd.print(Blank_line.substring(0, 16 - Current_device_name.length()) + Current_device_name);
  }

  LCD_set_combined_patch_number_and_name();
  Main_lcd.setCursor (0, 0);
  Main_lcd.print(Current_patch_number_string);

  Main_lcd.setCursor (0, 1);       // go to start of 2nd line

  switch (Setting.Main_display_mode) {
    case MD_SHOW_PAGE_NAME:
      EEPROM_read_title(Current_page, 0, Current_page_name); // Page name from EEPROM
      Main_lcd.print(Current_page_name);
      break;
    case MD_SHOW_CURRENT_PATCH:
      if (Current_device < NUMBER_OF_DEVICES) Main_lcd.print(Device[Current_device]->current_patch_name);
      break;
    case MD_SHOW_PATCHES_COMBINED:
      Main_lcd.print(Current_patch_name); // Show the combined patchname
      break;
  }
}

void LCD_set_combined_patch_number_and_name() {
  // Here we determine what to show on the main display, based on what devices are active
  uint8_t number_of_active_devices = 0;
  Current_patch_number_string = "";
  String patch_names[NUMBER_OF_DEVICES]; //Array of strings for the patchnames

  for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
    if (Device[d]->is_on) {
      Device[d]->display_patch_number_string(); // Adds the device name to patch number string
      Current_patch_number_string = Current_patch_number_string + PATCH_NUMBER_SEPERATOR; //Add a seperation sign in between
      patch_names[number_of_active_devices] = Device[d]->current_patch_name; //Add patchname to string
      number_of_active_devices++;
    }
  }

  // Cut last character of patch number string
  uint8_t l = Current_patch_number_string.length();
  Current_patch_number_string.remove(l - 1);

  if (number_of_active_devices > 2) Current_patch_number_string = Current_patch_number_string + "     "; //Add spaces at the end to clear any remaining leters

  // Show patchname
  switch (number_of_active_devices) {
    case 0:
      Current_patch_name = "                ";
      break;
    case 1: // Only one device active
      Current_patch_name = patch_names[0];
      break;
    case 2: // Show 7 bytes of both names
      Current_patch_name = patch_names[0].substring(0, 7) + "  " + patch_names[1].substring(0, 7);
      break;
    case 3: // Show 4 bytes of both names
      Current_patch_name = patch_names[0].substring(0, 4) + "  " + patch_names[1].substring(0, 4) + "  " + patch_names[2].substring(0, 4);
      break;
    default: // More then 3 devices
      Current_patch_name = String(number_of_active_devices) + " devices on    ";
      break;
  }
}

void LCD_show_status_message(String message)
// Will display a status message on the main display
{
  //Main_lcd.home();
  Main_lcd.setCursor (0, 1);       // go to start of 2nd line
  Main_lcd.print(message + Blank_line.substring(0, 16 - message.length())); //Print message plus remaining spaces
  messageTimer = millis();
  status_message_showing = true;
}

void LCD_show_are_you_sure(String line1, String line2)
{
  //Main_lcd.home();
  Main_lcd.setCursor (0, 0);       // go to start of 2nd line
  Main_lcd.print(line1 + Blank_line.substring(0, 16 - line1.length())); //Print message plus remaining spaces
  Main_lcd.setCursor (0, 1);       // go to start of 2nd line
  Main_lcd.print(line2 + Blank_line.substring(0, 16 - line2.length())); //Print message plus remaining spaces

  lcd[9].setCursor (0, 0);
  lcd[9].print(Blank_line);
  lcd[9].setCursor (0, 1);
  lcd[9].print("      YES       ");
  lcd[10].setCursor (0, 0);
  lcd[10].print(Blank_line);
  lcd[10].setCursor (0, 1);
  lcd[10].print("       NO       ");
}

void LCD_show_startup_message() {
  Main_lcd.home(); // go home
  //Main_lcd.setCursor(0, 0); // Need double command - otherwise I get garbage
  Main_lcd.print("V-controller " + String(VCONTROLLER_FIRMWARE_VERSION_MAJOR) + "." + String(VCONTROLLER_FIRMWARE_VERSION_MINOR));  // Show startup message
  LCD_show_status_message("  by SixEight   ");  //Please give me the credits :-)
}

void LCD_show_bar(uint8_t value) { // Will show the bar for the expression pedal on the top line of the main display
  uint8_t full_blocks = (value >> 3); // Calculate the number of full blocks to display - value is between 0 and 127
  uint8_t part_block = (value & 7) * 6 / 8; // Calculate which part block to display
  Main_lcd.setCursor (0, 0);
  for (uint8_t i = 0; i < full_blocks; i++) Main_lcd.print(char(4)); // Display the full blocks
  if (part_block > 0) Main_lcd.print(char(part_block - 1)); // Display the correct part block
  else Main_lcd.print(" ");
  for (uint8_t i = full_blocks + 1; i < 16; i++) Main_lcd.print(" "); // Fill the rest with spaces
}

// ********************************* Section 3: Update of individual LCDs ********************************************

void LCD_update(uint8_t sw) {
  // Will update a specific display based on the data in the SP array
  if ((sw > 0) && (sw <= NUMBER_OF_DISPLAYS)) { // Check if there is a display for this switch

    //Determine what to display from the Switch type
    DEBUGMSG("Update display no:" + String(sw));

    uint8_t number = sw - 1; // The displays are numbered from zero upwards

    if (EEPROM_check4label(Current_page, sw)) {
      String msg;
      EEPROM_read_title(Current_page, sw, msg); // Override the label if a custom label exists
      LCD_set_label(sw, msg);
    }

    uint8_t Dev = SP[sw].Device;
    if (Dev == CURRENT) Dev = Current_device;

    if (Dev < NUMBER_OF_DEVICES) {
      switch (SP[sw].Type) {
        case PATCH_SEL:
        case PATCH_BANK:
          Display_number_string[number] = ""; //Clear the display_number_string
          Device[Dev]->number_format(SP[sw].PP_number, Display_number_string[number]);
          lcd[number].setCursor (0, 0);
          lcd[number].print(char(0)); // Virtual LEDs
          lcd[number].print(char(0));
          lcd[number].print(char(0));
          lcd[number].print("          ");
          //lcd[number].print("   ");
          //lcd[number].print(Display_number_string[number]);
          //lcd[number].print("    ");
          lcd[number].print(char(0));
          lcd[number].print(char(0));
          lcd[number].print(char(0));
          lcd[number].setCursor(8 - ((Display_number_string[number].length() + 1) / 2), 0);
          lcd[number].print(Display_number_string[number]);
          LCD_centre_print_label(sw);
          break;
        case PREV_PATCH:
          Display_number_string[number] = ""; //Clear the display_number_string
          Device[Dev]->number_format(SP[sw].PP_number, Display_number_string[number]);
          lcd[number].setCursor (0, 0);
          lcd[number].print(char(0)); // Virtual LEDs
          lcd[number].print(char(0));
          lcd[number].print(" PREV (");
          lcd[number].print(Display_number_string[number]);
          lcd[number].print(") ");
          lcd[number].print(char(0)); // Virtual LEDs
          lcd[number].print(char(0));
          LCD_centre_print_label(sw);
          break;
        case NEXT_PATCH:
          Display_number_string[number] = ""; //Clear the display_number_string
          Device[Dev]->number_format(SP[sw].PP_number, Display_number_string[number]);
          lcd[number].setCursor (0, 0);
          lcd[number].print(char(0)); // Virtual LEDs
          lcd[number].print(char(0));
          lcd[number].print(" NEXT (");
          lcd[number].print(Display_number_string[number]);
          lcd[number].print(") ");
          lcd[number].print(char(0)); // Virtual LEDs
          lcd[number].print(char(0));
          LCD_centre_print_label(sw);
          break;
        case BANK_DOWN:
        case PAR_BANK_DOWN:
          //case ASG_BANK_DOWN:
          lcd[number].setCursor (0, 0);
          lcd[number].print(char(0)); // Virtual LEDs
          lcd[number].print(char(0));
          lcd[number].print(" BANK DOWN ");
          lcd[number].print(char(0)); // Virtual LEDs
          lcd[number].print(char(0));
          lcd[number].setCursor (0, 1);
          lcd[number].print("     ");
          lcd[number].print(Device[Dev]->device_name);
          lcd[number].print("      ");
          break;
        case BANK_UP:
        case PAR_BANK_UP:
          //case ASG_BANK_UP:
          lcd[number].setCursor (0, 0);
          lcd[number].print(char(0)); // Virtual LEDs
          lcd[number].print(char(0));
          lcd[number].print("  BANK UP  ");
          lcd[number].print(char(0)); // Virtual LEDs
          lcd[number].print(char(0));
          lcd[number].setCursor (0, 1);
          lcd[number].print("     ");
          lcd[number].print(Device[Dev]->device_name);
          lcd[number].print("      ");
          break;
        case DIRECT_SELECT:
          Display_number_string[number] = ""; //Clear the display_number_string
          Device[Dev]->direct_select_format(SP[sw].PP_number, Display_number_string[number]);
          lcd[number].setCursor (0, 0);
          lcd[number].print(char(0)); // Virtual LEDs
          lcd[number].print(char(0));
          lcd[number].print("    ");
          lcd[number].print(Display_number_string[number]);
          lcd[number].print("    ");
          lcd[number].print(char(0)); // Virtual LEDs
          lcd[number].print(char(0));
          lcd[number].setCursor (0, 1);
          lcd[number].print("                ");
          break;
        case PARAMETER:
        case PAR_BANK:
          // What to show on the individual display
          lcd[number].setCursor (0, 0);
          lcd[number].print(char(0)); // Virtual LEDs
          lcd[number].print(char(0));
          lcd[number].print(char(0));
          LCD_par_state(number, Dev);
          lcd[number].print(char(0));
          lcd[number].print(char(0));
          lcd[number].print(char(0));
          //lcd[number].print(SP[sw].Label); // Show the current patchname
          LCD_centre_print_label(sw);
          break;
        case ASSIGN:
          //case ASG_BANK:
          // What to show on the individual display
          if (SP[sw].Assign_number <= 16) { // Normal assign
            lcd[number].setCursor (0, 0);
            lcd[number].print(char(0));
            lcd[number].print(char(0));
            lcd[number].print("[");
            lcd[number].print(Device[Dev]->device_name);
            lcd[number].print(" ASGN");
            lcd[number].print(String(SP[sw].Assign_number));
            lcd[number].print("]");
            lcd[number].print(char(0));
            lcd[number].print(char(0));
          }
          else { // Is only the case for the VG99 as it has 16 common assigns!!!
            lcd[number].setCursor (0, 0);
            lcd[number].print(char(0));
            lcd[number].print(char(0));
            if (SP[sw].Assign_number <= 24) {
              lcd[number].printf("   [CTL%d]   ", SP[sw].Assign_number - 16);
            }
            switch (SP[sw].Assign_number) {
              case 25: lcd[number].print("   [EXP1]   "); break;
              case 26: lcd[number].print(" [EXP SW1]  "); break;
              case 27: lcd[number].print("   [EXP2]   "); break;
              case 28: lcd[number].print(" [EXP SW2]  "); break;
              case 29: lcd[number].print(" [GK S1/S2] "); break;
              case 30: lcd[number].print("  [GK VOL]  "); break;
              case 31: lcd[number].print("   [EXP]    "); break;
              case 32: lcd[number].print("   [CTL1]   "); break;
              case 33: lcd[number].print("   [CTL2]   "); break;
              case 34: lcd[number].print("   [CTL3]   "); break;
              case 35: lcd[number].print("   [CTL4]   "); break;
              case 36: lcd[number].print(" [D-BEAM V] "); break;
              case 37: lcd[number].print(" [D-BEAM H] "); break;
              case 38: lcd[number].print("[RIBBON ACT]"); break;
              case 39: lcd[number].print("[RIBBON POS]"); break;
            }
            lcd[number].print(char(0));
            lcd[number].print(char(0));
          }
          //lcd[number].print(SP[sw].Label); // Show the current patchname
          LCD_centre_print_label(sw);
          break;
        case OPEN_PAGE_DEVICE:
        case OPEN_PAGE_PATCH:
        case OPEN_PAGE_PARAMETER:
        case OPEN_PAGE_ASSIGN:
          lcd[number].setCursor (0, 0);
          lcd[number].print(char(0));
          lcd[number].print("     ");
          lcd[number].print(Device[Dev]->device_name);
          lcd[number].print("     ");
          lcd[number].print(char(0));
          LCD_centre_print_label(sw);
          DEBUGMSG("Number: " + String(number) + " Label: " + SP[sw].Label);
          break;
        case MUTE:
          lcd[number].setCursor (0, 0);
          lcd[number].print(char(0));
          lcd[number].print("  MUTE ");
          lcd[number].print(Device[Dev]->device_name);
          lcd[number].print("   ");
          lcd[number].print(char(0));
          break;
        default:
          if (number < NUMBER_OF_DISPLAYS) {
            lcd[number].setCursor (0, 0);
            lcd[number].print("                ");
            lcd[number].setCursor (0, 1);
            lcd[number].print("                ");
          }
      }
    }
    if (Dev == COMMON) {
      switch (SP[sw].Type) {
        case MENU:
          LCD_centre_print_title(sw);
          LCD_centre_print_label(sw);
          break;
        case OPEN_PAGE:
          lcd[number].setCursor (0, 0);
          lcd[number].print(char(0));
          lcd[number].print("              ");
          lcd[number].print(char(0));
          LCD_centre_print_label(sw);
          DEBUGMSG("Number: " + String(number) + " Label: " + SP[sw].Label);
          break;
        case GLOBAL_TUNER:
          if (number < NUMBER_OF_DISPLAYS) {
            lcd[number].setCursor (0, 0);
            lcd[number].print(char(0));
            lcd[number].print("<GLOBAL TUNER>");
            lcd[number].print(char(0));
            lcd[number].setCursor (0, 1);
            lcd[number].print("                ");
          }
          break;
        case TAP_TEMPO:
          if (number < NUMBER_OF_DISPLAYS) {
            lcd[number].setCursor (0, 0);
            lcd[number].print(char(0));
            lcd[number].print(" <TAP TEMPO>  ");
            lcd[number].print(char(0));
            lcd[number].setCursor (0, 1);
            lcd[number].printf("    %3d BPM     ", Setting.Bpm);
          }
          break;
        case SET_TEMPO:
          if (number < NUMBER_OF_DISPLAYS) {
            lcd[number].setCursor (0, 0);
            lcd[number].print(char(0));
            lcd[number].print(" <SET TEMPO>  ");
            lcd[number].print(char(0));
            lcd[number].setCursor (0, 1);
            lcd[number].printf("    %3d BPM     ", SP[sw].PP_number);
          }
          break;
        case PAGE_UP:
          if (number < NUMBER_OF_DISPLAYS) {
            lcd[number].setCursor (0, 0);
            lcd[number].print(char(0));
            lcd[number].print("  <PAGE UP>   ");
            lcd[number].print(char(0));
            LCD_centre_print_label(sw);
          }
          break;
        case PAGE_DOWN:
          if (number < NUMBER_OF_DISPLAYS) {
            lcd[number].setCursor (0, 0);
            lcd[number].print(char(0));
            lcd[number].print(" <PAGE DOWN>  ");
            lcd[number].print(char(0));
            LCD_centre_print_label(sw);
          }
          break;
        case MIDI_PC:
          if (number < NUMBER_OF_DISPLAYS) {
            lcd[number].setCursor (0, 0);
            lcd[number].print(char(0));
            lcd[number].print(char(0));
            lcd[number].print(char(0));
            lcd[number].printf(" <PC %03d>  ", SP[sw].PP_number);
            lcd[number].print(char(0));
            lcd[number].print(char(0));
            lcd[number].print(char(0));
            LCD_centre_print_label(sw);
          }
          break;
        case MIDI_CC:
          if (number < NUMBER_OF_DISPLAYS) {
            lcd[number].setCursor (0, 0);
            lcd[number].print(char(0));
            lcd[number].print(char(0));
            lcd[number].print(char(0));
            if ((SP[sw].Latch == CC_ONE_SHOT) || (SP[sw].Latch == CC_MOMENTARY)) lcd[number].printf(" <CC #%03d>   ", SP[sw].PP_number);
            if ((SP[sw].Latch == CC_TOGGLE) || (SP[sw].Latch == CC_TOGGLE_ON)) lcd[number].printf(" [CC #%03d]   ", SP[sw].PP_number);
            if (SP[sw].Latch == CC_STEP) {
              lcd[number].setCursor (2, 0);
              lcd[number].printf("CC #%03d (%d/%d)", SP[sw].PP_number, SP[sw].Target_byte1, SP[sw].Assign_max);
            }
            if (SP[sw].Latch == CC_UPDOWN) {
              if (SP[sw].Direction) {
                lcd[number].print(char(1));
                lcd[number].printf(" CC #%03d ", SP[sw].PP_number);
                lcd[number].print(char(1));
              }
              else {
                lcd[number].print(char(2));
                lcd[number].printf(" CC #%03d ", SP[sw].PP_number);
                lcd[number].print(char(2));
              }
            }

            lcd[number].print(char(0));
            lcd[number].print(char(0));
            lcd[number].print(char(0));
            if (SP[sw].Latch == CC_UPDOWN) {
              lcd[number].setCursor (0, 1);
              lcd[number].printf("      %3d       ", SP[sw].Target_byte1);
            }
            else LCD_centre_print_label(sw);
          }
          break;
        case MIDI_NOTE:
          if (number < NUMBER_OF_DISPLAYS) {
            lcd[number].setCursor (0, 0);
            lcd[number].print(char(0));
            lcd[number].print(char(0));
            lcd[number].printf(" <NOTE %03d> ", SP[sw].PP_number);
            lcd[number].print(char(0));
            lcd[number].print(char(0));
            LCD_centre_print_label(sw);
          }
          break;
        default:
          if (number < NUMBER_OF_DISPLAYS) {
            lcd[number].setCursor (0, 0);
            lcd[number].print(Blank_line);
            lcd[number].setCursor (0, 1);
            lcd[number].print(Blank_line);
          }
      }
    }
  }
}

void LCD_par_state(uint8_t number, uint8_t Dev) { // Will print the right parameter message depending on the TOGGLE state
  switch (SP[number + 1].Latch) {
    case TOGGLE:
      lcd[number].print(" [ ");
      lcd[number].print(Device[Dev]->device_name);
      lcd[number].print(" ] ");
      break;
    case MOMENTARY:
      lcd[number].print(" < ");
      lcd[number].print(Device[Dev]->device_name);
      lcd[number].print(" > ");
      break;
    case TRISTATE:
      lcd[number].print(Device[Dev]->device_name);
      lcd[number].print(" (");
      lcd[number].print(String(SP[number + 1].State));
      lcd[number].print("/3)");
      break;
    case FOURSTATE:
      lcd[number].print(Device[Dev]->device_name);
      lcd[number].print(" (");
      lcd[number].print(String(SP[number + 1].State));
      lcd[number].print("/4)");
      break;
    case TGL_OFF:
      lcd[number].print("     --     ");
      break;
    case UPDOWN:
      if (SP[number + 1].Direction) {
        lcd[number].print(' ');
        lcd[number].print(char(1));
        lcd[number].print(' ');
        lcd[number].print(Device[Dev]->device_name);
        lcd[number].print(' ');
        lcd[number].print(char(1));
        lcd[number].print(' ');
      }
      else {
        lcd[number].print(' ');
        lcd[number].print(char(2));
        lcd[number].print(' ');
        lcd[number].print(Device[Dev]->device_name);
        lcd[number].print(' ');
        lcd[number].print(char(2));
        lcd[number].print(' ');
      }
      break;
    default: // STEP and RANGE
      lcd[number].print(Device[Dev]->device_name);
      lcd[number].print(" (");
      lcd[number].print(String(SP[number + 1].Target_byte1 + 1));
      lcd[number].print("/");
      lcd[number].print(String(SP[number + 1].Assign_max));
      lcd[number].print(")");
      break;
  }
}

// ********************************* Section 4: LCD Functions ********************************************

void LCD_clear_label(uint8_t no) { // Will clear the Label string in the SP array
  for (uint8_t i = 0; i < 16; i++) {
    SP[no].Label[i] = ' ';
  }
}

void LCD_set_title(uint8_t sw, String & msg) { // Will set the Title string in the SP array
  // Check length does not exceed LABEL_SIZE
  uint8_t msg_length = msg.length();
  if (msg_length > SP_LABEL_SIZE) msg_length = SP_LABEL_SIZE;
  for (uint8_t i = 0; i < msg_length; i++) {
    SP[sw].Title[i] = msg[i];
  }
  for (uint8_t i = msg_length; i < SP_LABEL_SIZE; i++) { // Fill the remaining chars with spaces
    SP[sw].Label[i] = ' ';
  }
}

void LCD_set_label(uint8_t sw, String & msg) { // Will set the Label string in the SP array
  // Check length does not exceed LABEL_SIZE
  uint8_t msg_length = msg.length();
  if (msg_length > SP_LABEL_SIZE) msg_length = SP_LABEL_SIZE;
  for (uint8_t i = 0; i < msg_length; i++) {
    SP[sw].Label[i] = msg[i];
  }
  for (uint8_t i = msg_length; i < SP_LABEL_SIZE; i++) { // Fill the remaining chars with spaces
    SP[sw].Label[i] = ' ';
  }
}

void LCD_centre_print_title(uint8_t sw) { // Will print the title in the middle of a display
  // Find out the number of spaces at the end of the label
  uint8_t endpoint = strlen(SP[sw].Title) - 1; // Go to last character
  if (endpoint > SP_LABEL_SIZE - 1) endpoint = SP_LABEL_SIZE - 1;
  while ((endpoint > 0) && (SP[sw].Title[endpoint] == ' ')) endpoint--; //Find last character that is not a space

  // Find the correct position on second line and print label
  if (sw > 0) {
    uint8_t s = sw - 1;
    lcd[s].setCursor (0, 0); // Go to start of 2nd line
    lcd[s].print(Blank_line); // Clear the title
    lcd[s].setCursor ((SP_LABEL_SIZE - endpoint - 1) / 2, 0);  // Set the cursor to the start of the label
    lcd[s].print(SP[sw].Title); // Print it here.
  }
}

void LCD_centre_print_label(uint8_t sw) { // Will print the label in the middle of a display
  // Find out the number of spaces at the end of the label
  uint8_t endpoint = strlen(SP[sw].Label) - 1; // Go to last character
  if (endpoint > SP_LABEL_SIZE - 1) endpoint = SP_LABEL_SIZE - 1;
  while ((endpoint > 0) && (SP[sw].Label[endpoint] == ' ')) endpoint--; //Find last character that is not a space

  // Find the correct position on second line and print label
  if (sw > 0) {
    uint8_t s = sw - 1;
    lcd[s].setCursor (0, 1); // Go to start of 2nd line
    lcd[s].print(Blank_line); // Clear the line
    lcd[s].setCursor ((SP_LABEL_SIZE - endpoint - 1) / 2, 1);  // Set the cursor to the start of the label
    lcd[s].print(SP[sw].Label); // Print it here.
  }
  //DEBUGMSG(SP[s].Label);
}

void LCD_switch_on_backlight(uint8_t number) {
  if (number < NUMBER_OF_DISPLAYS) {
    if (!backlight_on[number]) {
      lcd[number].setBacklight(HIGH);
      backlight_on[number] = true;
    }
  }
}

void LCD_switch_off_backlight(uint8_t number) {
  if (number < NUMBER_OF_DISPLAYS) {
    if (backlight_on[number]) {
      lcd[number].setBacklight(LOW);
      backlight_on[number] = false;
    }
  }
}

void LCD_backlight_on() { // Will switch all backlights on
  for (uint8_t i = 0; i < NUMBER_OF_DISPLAYS; i++) {
    LCD_switch_on_backlight(i);
  }
  Main_lcd.setBacklight(HIGH);
}

void LCD_backlight_off() { // Will switch all backlights off
  for (uint8_t i = 0; i < NUMBER_OF_DISPLAYS; i++) {
    LCD_switch_off_backlight(i); //switch on the backlight
  }
  Main_lcd.setBacklight(LOW);
  Main_lcd.clear(); // Clear main display
}

void LCD_clear_all_displays() {
  Main_lcd.clear();
  for (uint8_t i = 0; i < NUMBER_OF_DISPLAYS; i++) {
    lcd[i].clear();
  }
}

void LCD_show_program_mode() {
  LCD_clear_all_displays();
  Main_lcd.setCursor(0, 0);
  Main_lcd.print("* PROGRAM MODE *");
  lcd[9].setCursor(0, 0);
  lcd[9].print("Upload firmware ");
  lcd[9].setCursor(0, 1);
  lcd[9].print("from TeensyDuino");
  lcd[10].setCursor(0, 0);
  lcd[10].print(" or power cycle ");
  lcd[10].setCursor(0, 1);
  lcd[10].print("the VController ");
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
byte vLED_on[8] = { // The character for LED on
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b00000,
  0b00000,
  0b00000
};
byte vLED_off[8] = { // The character for LED off
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
  if (number < NUMBER_OF_DISPLAYS) lcd[number].createChar(0, vLED_off);
}

void LCD_set_virtual_LED(uint8_t number, uint8_t state) { // Will set the state of a virtual LED
  if (Display_LED[number] != state) { // Check if state is new
    Display_LED[number] = state; // Update state
    // Update virtual LED
    if (number < NUMBER_OF_DISPLAYS) {
      if (state == 0) lcd[number].createChar(0, vLED_off);
      if (state == 1) lcd[number].createChar(0, vLED_on);
      if (state == 2) lcd[number].createChar(0, vLED_dimmed);
    }
  }
}

void Set_virtual_LED_colour(uint8_t number, uint8_t colour) { // Called from LEDs.ino-show_colour()
  if ((colour == 0) | (colour > 9)) LCD_set_virtual_LED(number, 0); //Virtual LED off
  else LCD_set_virtual_LED(number, 1); //Virtual LED on

  /*if (colour == 0) LCD_set_virtual_LED(number, 0); //Virtual LED off
  else if (colour < 10) LCD_set_virtual_LED(number, 1); //Virtual LED on
  else if (colour > 10) LCD_set_virtual_LED(number, 2); //Dimmed
  else LCD_set_virtual_LED(number, 0); //Virtual LED off for colour 10 as well*/
}


