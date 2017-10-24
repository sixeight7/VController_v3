// Please read VController_v3.ino for information about the license and authors

// Here we define the menus for the VController.

// This page has the following parts::
// Section 1: Menu structure
// Section 2: Functions called from menu
// Section 3: Functions that make the menu work
// Section 4: Command build structure
// Section 5: Text entry

// ********************************* Section 1: Menu structure ********************************************
//A menu item can have one of the following types:
#define NONE 0
#define OPEN_MENU 1  // to open another menu
#define SET 2        // SET, &variable - will set a variable to an item from the list.
#define EXECUTE 3    // EXECUTE (void*) code - will execute the code - make sure you omit the brackets after the function name ().
#define EXIT_MENU 4  // Will exit the menu
#define CMD_BYTE 5   // Show command byte - will increase the value on press
#define CMD_BACK 6   // To decrease the value of the last selected command byte
#define KEYBOARD 7   // To create a VController keyboard
#define SET_COLOUR 8 // To set colours for LEDs and RGB displays
#define SAVE_AND_EXIT 9 // Will return to the select menu either with or without saving the data
#define DEVICE_SET 10 // Will set a variable on a device - check Device_class::get_setting(uint8_t variable)

uint8_t current_menu = 0;
uint8_t Current_cmd_switch = 0;
uint8_t Current_cmd_number = 0;
uint8_t dummy;

// Variables for text entry
uint8_t page_in_edit = 0; // The page we are editing
uint8_t switch_in_edit = 0; // The switch we are editing
String Text_entry; // The string we use for entering a text
uint8_t Text_previous_switch = 255; // The number of the switch that was pressed before
uint8_t Text_switch_char_index = 0; // The index of the characters of the switch that is being pressed
bool keyboard_timer_running = false;
uint32_t keyboard_timer = 0;
#define KEYBOARD_TIME 800 // Wait time before forwarding the cursor to the next position
bool no_hold; // Holding switches too long may trigger hold when we don't want it

struct menu_struct {
  char Label[17];
  uint8_t Type;
  uint8_t Sublist;
  uint8_t Number_of_items;
  void *Target; // Pointer to target, which can be a variable or a function
};

#define SELECT_MENU 0
#define GLOBAL_MENU 1
#define DEVICE_MENU 2
#define LED_SETTINGS_MENU 3
#define LED_FX_COLOURS_MENU 4
#define COMMAND_SELECT_MENU 5
#define COMMAND_DELETE_MENU 6
#define COMMAND_EDIT_MENU 7
#define KEYBOARD_MENU 8
#define FIRMWARE_MENU 9

#define DEVICE_SUBLIST 255
#define PAGE_SUBLIST 254

const PROGMEM menu_struct menu[][15] = {
  { // Menu 0 - Select menu
    { "SELECT MENU     ", NONE }, // Menu title
    { "GLOBAL SETTINGS", OPEN_MENU, GLOBAL_MENU }, // Switch 1
    { "DEVICE SETTINGS", OPEN_MENU, DEVICE_MENU }, // Switch 2
    { "LED SETTINGS    ", OPEN_MENU, LED_SETTINGS_MENU },// Switch 3
    { "PROGRAM SWITCHES", OPEN_MENU, COMMAND_SELECT_MENU }, // Switch 4
    { "FIRMWARE MENU", OPEN_MENU, FIRMWARE_MENU }, // Switch 5
    { "", NONE }, // Switch 5
    { "", NONE }, // Switch 7
    { "", NONE }, // Switch 8
    { "", NONE }, // Switch 9
    { "", NONE }, // Switch 10
    { "", NONE }, // Switch 11
    { "EXIT MENU", EXIT_MENU, 0 }, // Switch 12
    { "", NONE }, // Switch 13 (LEFT)
    { "", NONE }, // Switch 14 (RIGHT)
  },

  { // Menu 1 - Global settings
    { "GLOBAL SETTINGS ", NONE }, // Menu title
    { "US20 Emulation", SET, 1, 1, &Setting.US20_emulation_active }, // Switch 1
    { "Glob.tempo on PC", SET, 1, 1, &Setting.Send_global_tempo_after_patch_change }, // Switch 2
    { "Main disply mode", SET, 19, 2, &Setting.Main_display_mode },// Switch 3
    { "", NONE }, // Switch 4
    { "Bass mode G2M ch", SET, 0, 16, &Setting.Bass_mode_G2M_channel }, // Switch 5
    { "Bass mode device", SET, DEVICE_SUBLIST, NUMBER_OF_DEVICES - 1, &Setting.Bass_mode_device }, // Switch 5
    { "Bass mode CC", SET, 0, 127, &Setting.Bass_mode_cc_number }, // Switch 7
    { "Bass mode min vl", SET, 0, 127, &Setting.Bass_mode_min_velocity}, // Switch 8
    { "", NONE }, // Switch 9
    { "", NONE }, // Switch 10
    { "SAVE & EXIT", SAVE_AND_EXIT, 1 }, // Switch 11
    { "Cancel", SAVE_AND_EXIT, 0 }, // Switch 12
    { "", NONE }, // Switch 13 (LEFT)
    { "", NONE }, // Switch 14 (RIGHT)
  },

  { // Menu 2 - Device settings
    { "DEVICE MENU     ", NONE }, // Menu title
    { "Select device", SET, DEVICE_SUBLIST, NUMBER_OF_DEVICES - 1, &Current_device }, // Switch 1
    { "Midi channel", DEVICE_SET, 0, 16, (void*) 1 }, // Switch 2
    { "Colour", DEVICE_SET, 3, NUMBER_OF_SELECTABLE_COLOURS - 1, (void*) 0 },// Switch 3
    { "Is always on", DEVICE_SET, 1, 1, (void*) 5 }, // Switch 4
    { "Device page #1", DEVICE_SET, PAGE_SUBLIST, LAST_FIXED_CMD_PAGE, (void*) 6 }, // Switch 5
    { "Device page #2", DEVICE_SET, PAGE_SUBLIST, LAST_FIXED_CMD_PAGE, (void*) 7 }, // Switch 6
    { "Device page #3", DEVICE_SET, PAGE_SUBLIST, LAST_FIXED_CMD_PAGE, (void*) 8 }, // Switch 7
    { "", NONE }, // Switch 8
    { "", NONE }, // Switch 9
    { "", NONE }, // Switch 10
    { "SAVE & EXIT", SAVE_AND_EXIT, 1 }, // Switch 11
    { "Cancel", SAVE_AND_EXIT, 0 }, // Switch 12
    { "", NONE }, // Switch 13 (LEFT)
    { "", NONE }, // Switch 14 (RIGHT)
  },

  { // Menu 3 - LED settings menu
    { "LED SETTINGS    ", NONE }, // Menu title
    { "FX COLOUR MENU", OPEN_MENU, LED_FX_COLOURS_MENU}, // Switch 1
    { "LED Brightness", SET, 0, 100, &Setting.LED_brightness }, // Switch 2
    { "Backlight Bright", SET, 0, 255, &Setting.Backlight_brightness }, // Switch 3
    { "Virtual LEDs", SET, 1, 1, &Setting.Virtual_LEDs },// Switch 4
    { "FX off is dimmed", SET, 1, 1, &Setting.LED_FX_off_is_dimmed }, // Switch 5
    { "Global colour", SET, 3, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.LED_global_colour }, // Switch 6
    { "BPM colour", SET, 3, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.LED_bpm_colour }, // Switch 7
    { "MIDI PC colour", SET, 3, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.MIDI_PC_colour }, // Switch 8
    { "MIDI CC colour", SET, 3, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.MIDI_CC_colour }, // Switch 9
    { "MIDI note colour", SET, 3, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.MIDI_note_colour }, // Switch 10
    { "SAVE & EXIT", SAVE_AND_EXIT, 1 }, // Switch 11
    { "Cancel", SAVE_AND_EXIT, 0 }, // Switch 12
    { "", NONE }, // Switch 13 (LEFT)
    { "", NONE }, // Switch 14 (RIGHT)
  },

  { // Menu 4 - LED effect colours
    { "LED FX COLOURS  ", NONE }, // Menu title
    { "GTR/COSM COLOUR", SET, 3, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_GTR_colour }, // Switch 1
    { "PITCH FX COLOUR", SET, 3, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_PITCH_colour }, // Switch 2
    { "FILTER FX COLOUR", SET, 3, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_FILTER_colour },// Switch 3
    { "DIST FX COLOUR", SET, 3, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_DIST_colour }, // Switch 4
    { "AMP FX COLOUR", SET, 3, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_AMP_colour }, // Switch 5
    { "MOD FX COLOUR", SET, 3, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_MODULATION_colour }, // Switch 5
    { "DELAY FX COLOUR", SET, 3, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_DELAY_colour }, // Switch 7
    { "REVERB FX COLOUR", SET, 3, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_REVERB_colour}, // Switch 8
    { "", NONE }, // Switch 9
    { "", NONE }, // Switch 10
    { "", NONE }, // Switch 11
    { "EXIT", OPEN_MENU, LED_SETTINGS_MENU }, // Switch 12
    { "", NONE }, // Switch 13 (LEFT)
    { "", NONE }, // Switch 14 (RIGHT)
  },

  { // Menu 5 - Command select menu
    { "SELECT PG/SW/CMD", NONE }, // Menu title
    { "", CMD_BYTE, 8 }, // Switch 1
    { "", CMD_BYTE, 9 }, // Switch 2
    { "", CMD_BYTE, 10 },// Switch 3
    { "", NONE }, // Switch 4
    { "EDIT PAGE NAME", EXECUTE, 0, 0, (void*)edit_page_name }, // Switch 5
    { "EDIT SWITCH NAME", EXECUTE, 0, 0, (void*)edit_switch_name }, // Switch 6
    { "EDIT COMMAND", EXECUTE, 0, 0, (void*)go_load_cmd }, // Switch 7
    { "MORE...", OPEN_MENU, COMMAND_DELETE_MENU }, // Switch 8
    { "DECREASE VALUE", CMD_BACK }, // Switch 9
    { "", NONE }, // Switch 10
    { "", NONE }, // Switch 11
    { "EXIT", OPEN_MENU, SELECT_MENU }, // Switch 12
    { "", NONE }, // Switch 13 (LEFT)
    { "", NONE }, // Switch 14 (RIGHT)
  },

  { // Menu 6 - Command delete menu
    { "SELECT PG/SW/CMD", NONE }, // Menu title
    { "", CMD_BYTE, 8 }, // Switch 1
    { "", CMD_BYTE, 9 }, // Switch 2
    { "", CMD_BYTE, 10 },// Switch 3
    { "", NONE }, // Switch 4
    { "INITIALIZE PAGE", EXECUTE, 0, 0, (void*)go_init_page }, // Switch 5
    { "INIT SWITCH", EXECUTE, 0, 0, (void*)go_init_switch }, // Switch 6
    { "DELETE COMMAND", EXECUTE, 0, 0, (void*)go_delete_cmd }, // Switch 7
    { "LESS...", OPEN_MENU, COMMAND_SELECT_MENU }, // Switch 8
    { "DECREASE VALUE", CMD_BACK }, // Switch 9
    { "DEL SWITCH NAME", EXECUTE, 0, 0, (void*)delete_switch_name }, // Switch 10
    { "", NONE }, // Switch 11
    { "EXIT", OPEN_MENU, SELECT_MENU }, // Switch 12
    { "", NONE }, // Switch 13 (LEFT)
    { "", NONE }, // Switch 14 (RIGHT)
  },

  { // Menu 7 - Command edit command
    { "Edit COMMAND    ", NONE }, // Menu title
    { "", CMD_BYTE, 0 }, // Switch 1
    { "", CMD_BYTE, 1 }, // Switch 2
    { "", CMD_BYTE, 2 },// Switch 3
    { "", CMD_BYTE, 3 }, // Switch 4
    { "", CMD_BYTE, 4 }, // Switch 5
    { "", CMD_BYTE, 5 }, // Switch 6
    { "", CMD_BYTE, 6 }, // Switch 7
    { "", CMD_BYTE, 7 }, // Switch 8
    { "DECREASE VALUE", CMD_BACK}, // Switch 9
    { "", NONE  }, // Switch 10
    { "SAVE CMD", EXECUTE, 0, 0, (void*)go_save_cmd }, // Switch 11
    { "EXIT", OPEN_MENU, COMMAND_SELECT_MENU }, // Switch 12
    { "", NONE }, // Switch 13 (LEFT)
    { "", NONE }, // Switch 14 (RIGHT)
  },
  { // Menu 8 - Virtual keyboard
    { "Enter Name:     ", NONE }, // Menu title
    { "ABC1abc", KEYBOARD}, // Switch 1
    { "DE2de", KEYBOARD}, // Switch 2
    { "FGH3fgh", KEYBOARD},// Switch 3
    { "IJ4ij", KEYBOARD}, // Switch 4
    { "KL5kl", KEYBOARD}, // Switch 5
    { "MN6mn", KEYBOARD}, // Switch 6
    { "OPQ7opq", KEYBOARD}, // Switch 7
    { "RST8rst", KEYBOARD}, // Switch 8
    { "UVW9uvw", KEYBOARD}, // Switch 9
    { "XYZ0xyz", KEYBOARD}, // Switch 10
    { " !?-+_", KEYBOARD}, // Switch 11
    { "<OK>", EXECUTE, 0, 0, (void*)close_edit_name }, // Switch 12
    { "<=", EXECUTE, 0, 0, (void*)cursor_left_page_name }, // Switch 13 (LEFT)
    { "=>", EXECUTE, 0, 0, (void*)cursor_right_page_name }, // Switch 14 (RIGHT)
  },

  { // Menu 9 - Firmware menu
    { "FIRMWARE MENU   ", NONE }, // Menu title
    { "Init Settings",  EXECUTE, 0, 0, (void*)initialize_settings }, // Switch 1
    { "Init Commands",  EXECUTE, 0, 0, (void*)initialize_commands }, // Switch 2
    { "Program Mode",  EXECUTE, 0, 0, (void*)reboot_program_mode },// Switch 3
    { "", NONE }, // Switch 4
    { "", NONE }, // Switch 5
    { "", NONE }, // Switch 6
    { "", NONE }, // Switch 7
    { "", NONE }, // Switch 8
    { "", NONE }, // Switch 9
    { "", NONE }, // Switch 10
    { "", NONE }, // Switch 11
    { "EXIT",  OPEN_MENU, SELECT_MENU }, // Switch 12
    { "", NONE }, // Switch 13 (LEFT)
    { "", NONE }, // Switch 14 (RIGHT)
  },

};

const uint16_t NUMBER_OF_MENUS = sizeof(menu) / sizeof(menu[0]);

const PROGMEM char menu_sublist[][17] = {
  // Sublist 1 - 2: Booleans
  "OFF", "ON",

  // Sublist 3 - 18: LED colours
  "OFF", "GREEN", "RED", "BLUE", "ORANGE", "TURQUOISE", "WHITE", "YELLOW", "PURPLE", "PINK", "", "", "", "", "", "",
  
  // Sublist 19 - 21: Main display modes
  "PAGE NAME", "PATCH NAME", "PATCHES COMBINED",
};

// ********************************* Section 2: Functions called from menu ********************************************
void initialize_settings() {
  if (menu_are_you_sure("All settings", "reset! Sure?")) {
    EEP_initialize_internal_eeprom_data();
    EEP_read_eeprom_common_data();
  }
}

void initialize_commands() {
  if (menu_are_you_sure("All commands", "reset! Sure?")) {
    EEP_initialize_external_eeprom_data();
    EEPROM_clear_indexes(); // First clear the indexes
    EEPROM_create_indexes(); // Then recreate them
  }
}

void reboot_program_mode() { // Reboot the Teensy to program mode
  DEBUGMSG("Rebooting to program mode...");
  LCD_show_program_mode();
  _reboot_Teensyduino_();
}

// ********************************* Section 3: Functions that make the menu work ********************************************
void menu_open() { // Called when the menu is started the first time
  current_menu = SELECT_MENU; // Go to top menu
  no_hold = true;
}

void menu_load(uint8_t Sw) {
  uint8_t number = SP[Sw].PP_number;
  uint8_t *val;
  uint8_t vnumber;
  uint8_t value;
  uint8_t index;
  String msg;
  //uint8_t dest;
  SP[Sw].Colour = Setting.LED_global_colour; // Set default colour

  DEBUGMSG("Loading menu:" + String(current_menu) + ", item:" + String(number) + ", type:" + String(menu[current_menu][number].Type) + ", switch " + String(Sw));
  switch (menu[current_menu][number].Type) {
    case NONE:
      LCD_set_title(Sw, Blank_line);
      LCD_set_label(Sw, Blank_line);
      SP[Sw].Colour = 0;
      break;
    case CMD_BYTE:
      load_cmd_byte(Sw, number);
      break;
    case SAVE_AND_EXIT:
    case EXIT_MENU:
    case OPEN_MENU:
    case EXECUTE:
    case CMD_BACK:
    case KEYBOARD:
      strcpy(SP[Sw].Title, menu[current_menu][number].Label);
      LCD_set_label(Sw, Blank_line);
      break;
    case SET:
      strcpy(SP[Sw].Title, menu[current_menu][number].Label);
      val = reinterpret_cast<uint8_t*>(menu[current_menu][number].Target);
      if (menu[current_menu][number].Sublist == DEVICE_SUBLIST) { // Check for DEVICE_SUBLIST
        if (*val < NUMBER_OF_DEVICES) strcpy(SP[Sw].Label, Device[*val]->full_device_name);
      }
      else if (menu[current_menu][number].Sublist > 0) { // Show sublist if neccesary
        // Determine sublist item
        index = *val;
        index += menu[current_menu][number].Sublist;
        strcpy(SP[Sw].Label, menu_sublist[index - 1]);  // Copy to the label
      }
      else { // just show the value
        msg = String(*val);
        LCD_set_label(Sw, msg);
      }
      if (menu[current_menu][number].Sublist == 3) { // When we set colours, let LED and backlight change
        if (*val == 0) SP[Sw].Colour = Setting.LED_global_colour; // We need a real color for LED off - otherwise the backlight will be off
        else SP[Sw].Colour = *val;
      }
      else SP[Sw].Colour = Setting.LED_global_colour;
      break;
    case DEVICE_SET:
      strcpy(SP[Sw].Title, menu[current_menu][number].Label);
      val = reinterpret_cast<uint8_t*>(menu[current_menu][number].Target);
      vnumber = uint32_t(val); // Here we have the number back that we entered in the menu
      if (Current_device < NUMBER_OF_DEVICES) value = Device[Current_device]->get_setting(vnumber);
      else value = 0;
      
      if (menu[current_menu][number].Sublist == PAGE_SUBLIST) {
        if (value == 0)  msg = "---";
        else EEPROM_read_title(value, 0, msg); // Read page name from EEPROM
          LCD_set_label(Sw, msg);
      }
      else if (menu[current_menu][number].Sublist > 0) { // Show sublist if neccesary
        // Determine sublist item
        index = value;
        index += menu[current_menu][number].Sublist;
        strcpy(SP[Sw].Label, menu_sublist[index - 1]);  // Copy to the label
      }
      else { // just show the value
        msg = String(value);
        LCD_set_label(Sw, msg);
      }
      
      if (menu[current_menu][number].Sublist == 3) { // When we set colours, let LED and backlight change
        if (value == 0) SP[Sw].Colour = Setting.LED_global_colour; // We need a real color for LED off - otherwise the backlight will be off
        else SP[Sw].Colour = value;
      }
      else SP[Sw].Colour = Setting.LED_global_colour;
      break;
    default:
      strcpy(SP[Sw].Title, menu[current_menu][number].Label);
      break;
  }

  //LED_update();
  LCD_update(Sw);
  update_LEDS = true;
}

void menu_press(uint8_t Sw) { // Called when button for this menu is pressed
  uint8_t number = SP[Sw].PP_number;
  uint8_t cmd_no;
  uint8_t *val;
  uint8_t vnumber;
  uint8_t value;
  no_hold = true;
  switch (menu[current_menu][number].Type) {
    case CMD_BYTE:
      cmd_no = menu[current_menu][number].Sublist; // Command number is in the Sublist variable
      cmdbyte_increase(cmd_no);
      Current_cmd_switch = Sw;
      Current_cmd_number = number;
      menu_load(Sw); // Will update the label
      no_hold = false;
      break;
    case CMD_BACK:
      cmd_no = menu[current_menu][Current_cmd_number].Sublist; // Command number is in the Sublist variable
      cmdbyte_decrease(cmd_no);
      menu_load(Current_cmd_switch); // Will update the label
      no_hold = false;
      break;
    case OPEN_MENU:
      menu_select(menu[current_menu][number].Sublist);
      break;
    case SET:
      val = reinterpret_cast<uint8_t*>(menu[current_menu][number].Target);
      if (*val < menu[current_menu][number].Number_of_items)*val = *val + 1;
      else *val = 0;
      menu_load(Sw); // Will update the label
      if (menu[current_menu][number].Sublist == DEVICE_SUBLIST) update_page = REFRESH_PAGE; // So the device menu updates when another device is selected
      no_hold = false;
      break;
    case DEVICE_SET:
      if (Current_device < NUMBER_OF_DEVICES) {
        val = reinterpret_cast<uint8_t*>(menu[current_menu][number].Target);
        vnumber = uint32_t(val); // Here we have the number back that we entered in the menu
        value = Device[Current_device]->get_setting(vnumber);
        if (value < menu[current_menu][number].Number_of_items) value = value + 1;
        else value = 0;
        if ((menu[current_menu][number].Sublist == PAGE_SUBLIST) && (value == Number_of_pages)) value = FIRST_FIXED_CMD_PAGE; // Hop over gap in pages
        Device[Current_device]->set_setting(vnumber, value);
        menu_load(Sw); // Will update the label
        no_hold = false;
      }
      break;
    case EXECUTE:
      DEBUGMSG("Executing menu command");
      menu_call_function(menu[current_menu][number].Target);
      break;
    case KEYBOARD:
      key_press(number);
      break;
    case SAVE_AND_EXIT:
      if (menu[current_menu][number].Sublist == 1) EEP_write_eeprom_common_data(); //Save data if sublist is 1
      else {
        EEP_read_eeprom_common_data(); // Otherwise return to the settings that have been saved previously in EEPROM
        Current_page = PAGE_MENU; // Restore current page to PAGE_MENU, otherwise we lose our way...
      }
      menu_select(SELECT_MENU);
      break;
    case EXIT_MENU:
      SCO_select_page(Previous_page);
      break;
    case NONE:
      break;
  }
}

void menu_press_hold(uint8_t Sw) { // Called when button for this menu is held
  uint8_t number = SP[Sw].PP_number;
  uint8_t cmd_no;
  uint8_t *val;
  uint8_t vnumber;
  uint8_t value;
  if (!no_hold) {
    switch (menu[current_menu][number].Type) {
      case CMD_BYTE:
        cmd_no = menu[current_menu][number].Sublist; // Command number is in the Sublist variable
        cmdbyte_increase(cmd_no);
        Current_cmd_switch = Sw;
        Current_cmd_number = number;
        menu_load(Sw); // Will update the label
        break;
      case CMD_BACK:
        cmd_no = menu[current_menu][Current_cmd_number].Sublist; // Command number is in the Sublist variable
        cmdbyte_decrease(cmd_no);
        menu_load(Current_cmd_switch); // Will update the label
        break;
      case SET:
        val = reinterpret_cast<uint8_t*>(menu[current_menu][number].Target);
        if (*val < menu[current_menu][number].Number_of_items)*val = *val + 1;
        else *val = 0;
        menu_load(Sw); // Will update the label
        if (menu[current_menu][number].Sublist == DEVICE_SUBLIST) update_page = REFRESH_PAGE; // So the device menu updates when another device is selected
        break;
      case DEVICE_SET:
        if (Current_device < NUMBER_OF_DEVICES) {
          val = reinterpret_cast<uint8_t*>(menu[current_menu][number].Target);
          vnumber = uint32_t(val); // Here we have the number back that we entered in the menu
          value = Device[Current_device]->get_setting(vnumber);
          if (value < menu[current_menu][number].Number_of_items) value = value + 1;
          else value = 0;
          Device[Current_device]->set_setting(vnumber, value);
          menu_load(Sw); // Will update the label
        }
        break;
    }
  }
}

typedef void(*f_valueHandler)();

void menu_call_function(void *Target) {
  f_valueHandler callfunction = reinterpret_cast<f_valueHandler>( reinterpret_cast<void*>(Target));
  if (callfunction != 0) {
    callfunction(); // Execute the target function
  }
}

void menu_select(uint8_t _menu) {
  current_menu = _menu;
  update_page = REFRESH_PAGE;
  update_main_lcd = true;
}

bool menu_are_you_sure(String line1, String line2) {
  LCD_clear_all_displays();
  LCD_show_are_you_sure(line1, line2);

  LED_show_are_you_sure();
  bool sure = SCO_are_you_sure();

  update_page = REFRESH_PAGE;
  update_main_lcd = true;
  update_LEDS = true;

  return (sure);
}


// ********************************* Section 4: Command build structure ********************************************
// Here we create the user interface for building commands in the VController
// The commandbytes have a number of "types". These are descibed below

uint8_t selected_device_cmd = PATCH_SEL - 100;
uint8_t selected_common_cmd = OPEN_PAGE;

struct cmdtype_struct {
  char Title[17];
  uint8_t Sublist;
  uint8_t Min;
  uint8_t Max;
};

#define TYPE_OFF 0
#define TYPE_DEVICE_SELECT 1
#define TYPE_COMMON_COMMANDS 2
#define TYPE_DEVICE_COMMANDS 3
#define TYPE_PAGE 4
#define TYPE_MIDI_PORT 5
#define TYPE_MIDI_CHANNEL 6
#define TYPE_CC_NUMBER 7
#define TYPE_CC_TOGGLE 8
#define TYPE_VALUE 9
#define TYPE_NOTE_NUMBER 10
#define TYPE_NOTE_VELOCITY 11
#define TYPE_PC 12
#define TYPE_REL_NUMBER 13
#define TYPE_BANK_SIZE 14
#define TYPE_PATCH_NUMBER 15
#define TYPE_PATCH_100 16
#define TYPE_PARAMETER 17
#define TYPE_PAR_STATE 18
#define TYPE_ASSIGN 19
#define TYPE_ASSIGN_TRIGGER 20
#define TYPE_TOGGLE 21
#define TYPE_STEP 22
#define TYPE_SWITCH 23
#define TYPE_CMD 24
#define TYPE_BPM 25
#define TYPE_MIN 26
#define TYPE_MAX 27

// Some of the data for the sublists below is not fixed, but must be read from a Device class or from EEPROM
// Here we define these sublists
#define SUBLIST_PATCH 255 // To show the patchnumber
#define SUBLIST_PARAMETER 254 // To show the parameter name
#define SUBLIST_PAR_STATE 253 // To show the parameter state
#define SUBLIST_ASSIGN 252 // To show the assign name
#define SUBLIST_TRIGGER 251 // To show the assign trigger
#define SUBLIST_PAGE 250 // To show the page name
#define SUBLIST_CMD 249 // To show the selected command
#define SUBLIST_DEVICES 248 // To show the devices + Current and Common

const PROGMEM cmdtype_struct cmdtype[] = {
  { "", 0, 0, 0 }, // TYPE_OFF 0
  { "DEVICE", SUBLIST_DEVICES, 0, (NUMBER_OF_DEVICES + 1) }, // TYPE_DEVICE_SELECT 1
  { "COMMAND", 1, 0, NUMBER_OF_COMMON_TYPES - 1 }, // TYPE_COMMON_COMMANDS 2
  { "COMMAND", 18, 0, NUMBER_OF_DEVICE_TYPES - 1 }, // TYPE_DEVICE_COMMANDS 3
  { "PAGE", SUBLIST_PAGE, 0, LAST_FIXED_CMD_PAGE }, // TYPE_PAGE 4
  { "MIDI PORT", 40, 0, 4 }, // TYPE_MIDI_PORT 5
  { "MIDI CHANNEL", 0, 1, 16 }, // TYPE_MIDI_CHANNEL 6
  { "CC NUMBER", 0, 0, 127 }, // TYPE_CC_NUMBER 7
  { "CC TOGGLE TYPE", 83, 0, 6 }, // TYPE_CC_TOGGLE 8
  { "VALUE", 0, 0, 127 }, // TYPE_VALUE 9
  { "NOTE NUMBER", 0, 0, 127 }, // NOTE_NUMBER 10
  { "NOTE VELOCITY", 0, 0, 127 }, // TYPE_NOTE_VELOCITY 11
  { "PROGRAM", 0, 0, 127 }, // TYPE_PC 12
  { "NUMBER", 0, 1, 16 }, // TYPE_REL_NUMBER 13
  { "BANK SIZE", 0, 1, 16 }, // TYPE_BANK_SIZE 14
  { "PATCH NUMBER", SUBLIST_PATCH, 0, 99 }, // TYPE_PATCH_NUMBER 15
  { "PATCH BANK (100)", 0, 0, 255 }, // TYPE_PATCH_100 16
  { "PARAMETER", SUBLIST_PARAMETER, 0, 255 }, // TYPE_PARAMETER 17
  { "VALUE", SUBLIST_PAR_STATE, 0, 255 }, // TYPE_PAR_STATE 18
  { "ASSIGN", SUBLIST_ASSIGN, 0, 255 }, // TYPE_ASSIGN 19
  { "TRIGGER", SUBLIST_TRIGGER, 1, 127 }, // TYPE_ASSIGN 20
  { "TOGGLE TYPE", 48, 0, 6 }, // TYPE_TOGGLE 21
  { "STEP", 0, 1, 127 }, // TYPE_STEP 22
  { "SWITCH", 58, 0, 24 }, // TYPE_SWITCH 23
  { "COMMAND", SUBLIST_CMD, 0, 255 }, // TYPE_CMD 24
  { "BPM", 0, 40, 250 }, // TYPE_BPM 25
  { "MIN", 0, 0, 127 }, // TYPE_MIN 26
  { "MAX", 0, 0, 127 }, // TYPE_MAX 27
};

const PROGMEM char cmd_sublist[][17] = {

  // Sublist 1 - 17: Common Types
  "NO COMMAND", "SELECT PAGE", "PAGE UP", "PAGE DOWN", "TAP TEMPO", "SET TEMPO", "GLOBAL TUNER", "MIDI PC", "MIDI CC", "MIDI NOTE", "NEXT DEVICE", "MENU", "", "", "", "", "",

  // Sublist 18 - 39: Device Types
  "PATCH SELECT", "PARAMETER", "ASSIGN", "PATCHBANK SELECT", "BANK UP", "BANK DOWN", "NEXT PATCH", "PREV PATCH",
  "MUTE", "SEL DEVICE PAGE", "SEL PAGE PATCH", "SELECT PAGE PAR", "SELECT PAGE ASGN", "DIR.SELECT", "PAR BANK", "PAR BANK UP", "PAR BANK DOWN", "", "", "", "", "",

  // Sublist 40 - 47: MIDI ports
  "USB MIDI", "MIDI 1", "MIDI2/RRC", "MIDI 3", "ALL PORTS", "", "", "",

  // Sublist 48 - 57: Toggle types
  "MOMENTARY", "TOGGLE", "TRISTATE", "FOURSTATE", "STEP",  "RANGE",  "UPDOWN", "", "", "",

  // Sublist 58 - 82: Switch types
  "On Page Select", "Switch 1", "Switch 2", "Switch 3", "Switch 4", "Switch 5", "Switch 6", "Switch 7", "Switch 8",
  "Switch 9", "Switch 10", "Switch 11", "Switch 12", "Switch 13", "Switch 14", "Switch 15", "Switch 16",
  "Ext 1 / Exp1", "Ext 2", "Ext 3 / Exp 2", "Ext 4", "Ext 5 / Exp 3", "Ext 6", "Ext 7 / Exp 4", "Ext 8",

  // Sublist 83 - 89: CC toggle types
  "ONE SHOT", "MOMENTARY", "TOGGLE", "TOGGLE ON", "RANGE", "STEP", "UPDOWN",
};

struct cmdbyte_struct {
  uint8_t Type;
  uint8_t Value;
};

#define NUMBER_OF_CMD_BYTES 16

cmdbyte_struct cmdbyte[NUMBER_OF_CMD_BYTES] = {
  // Default command to edit
  { TYPE_DEVICE_SELECT, (NUMBER_OF_DEVICES + 1) },
  { TYPE_COMMON_COMMANDS, 0},
  { TYPE_OFF, 0},
  { TYPE_OFF, 0},
  { TYPE_OFF, 0},
  { TYPE_OFF, 0},
  { TYPE_OFF, 0},
  { TYPE_OFF, 0},
  // Default select command fields
  { TYPE_PAGE, 1},
  { TYPE_SWITCH, 1},
  { TYPE_CMD, 0},
  { TYPE_OFF, 0},
  { TYPE_OFF, 0},
  { TYPE_OFF, 0},
  { TYPE_OFF, 0},
  { TYPE_OFF, 0},
};

void go_load_cmd() { // Load the selected command and open the edit command menu
  // Read the selected command
  uint8_t sel_page = cmdbyte[8].Value;
  uint8_t sel_switch = cmdbyte[9].Value;
  uint8_t sel_cmd_no = cmdbyte[10].Value;
  Cmd_struct sel_cmd;
  EEPROM_read_cmd(sel_page, sel_switch, sel_cmd_no, &sel_cmd);
  DEBUGMSG("Load cmd: " + String(sel_cmd.Device) + ", "  + String(sel_cmd.Type) + ", " + String(sel_cmd.Data1));
  // Load it into the cmdbyte array and build the structure

  if (sel_cmd.Device < NUMBER_OF_DEVICES) {
    cmdbyte[0].Type = TYPE_DEVICE_SELECT;
    cmdbyte[0].Value = sel_cmd.Device;
    cmdbyte[1].Type =  TYPE_DEVICE_COMMANDS;
    cmdbyte[1].Value = sel_cmd.Type - 100;
    DEBUGMSG("Device " + String(sel_cmd.Device) + " has type " + String(cmdbyte[1].Value));
  }
  else if (sel_cmd.Device == CURRENT) {
    cmdbyte[0].Type = TYPE_DEVICE_SELECT; // Set to current
    cmdbyte[0].Value = NUMBER_OF_DEVICES;
    cmdbyte[1].Type =  TYPE_DEVICE_COMMANDS;
    cmdbyte[1].Value = sel_cmd.Type - 100;
    DEBUGMSG("Device CURRENT has type " + String(cmdbyte[1].Value));
  }
  else { // Common device type
    cmdbyte[0].Type = TYPE_DEVICE_SELECT;
    cmdbyte[0].Value = NUMBER_OF_DEVICES + 1; // Set to common
    cmdbyte[1].Type =  TYPE_COMMON_COMMANDS;
    cmdbyte[1].Value = sel_cmd.Type;
  }
  cmdbyte[2].Value = sel_cmd.Data1;
  cmdbyte[3].Value = sel_cmd.Data2;
  cmdbyte[4].Value = sel_cmd.Value1;
  cmdbyte[5].Value = sel_cmd.Value2;
  cmdbyte[6].Value = sel_cmd.Value3;
  cmdbyte[7].Value = sel_cmd.Value4;

  for (uint8_t i = 1; i < 8; i++) {
    build_command_structure(i, cmdbyte[i].Type, false);
  }

  // Select the menu page for editing a command: 4
  current_menu = COMMAND_EDIT_MENU;
  update_page = REFRESH_PAGE;
  update_main_lcd = true;
}

void go_init_page() { // Delete all the commands of the current page
  uint8_t sel_page = cmdbyte[8].Value;
  if (menu_are_you_sure("Page will be", "cleared. Sure?")) {
    EEPROM_clear_page(sel_page);
    EEPROM_purge_cmds();
    LCD_show_status_message("Page initialized");
  }
  update_page = REFRESH_PAGE;
  update_main_lcd = true;
}

void go_init_switch() { // Delete all the commands of the current page
  // Read the selected page
  uint8_t sel_page = cmdbyte[8].Value;
  uint8_t sel_switch = cmdbyte[9].Value;
  if (menu_are_you_sure("Switch will be", "cleared. Sure?")) {
    EEPROM_clear_switch(sel_page, sel_switch);
    EEPROM_purge_cmds();
    LCD_show_status_message("Switch initialized");
  }
  update_page = REFRESH_PAGE;
  update_main_lcd = true;
}

void go_delete_cmd() { // Delete the selected command
  // Find the selected command
  uint8_t sel_page = cmdbyte[8].Value;
  uint8_t sel_switch = cmdbyte[9].Value;
  uint8_t sel_cmd_no = cmdbyte[10].Value;
  Cmd_struct empty_cmd = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  EEPROM_write_cmd(sel_page, sel_switch, sel_cmd_no, &empty_cmd);
  EEPROM_purge_cmds();
  LCD_show_status_message("Command deleted ");
  update_page = REFRESH_PAGE;
  update_main_lcd = true;
}

void go_save_cmd() {
  uint8_t sel_page = cmdbyte[8].Value;
  uint8_t sel_switch = cmdbyte[9].Value;
  uint8_t sel_cmd_no = cmdbyte[10].Value;
  Cmd_struct my_cmd;
  my_cmd.Page = sel_page;
  my_cmd.Switch = sel_switch;
  if (cmdbyte[0].Value < NUMBER_OF_DEVICES) {
    my_cmd.Device = cmdbyte[0].Value;
    my_cmd.Type = cmdbyte[1].Value + 100;
  }
  if (cmdbyte[0].Value == NUMBER_OF_DEVICES) { // Current device
    my_cmd.Device = CURRENT;
    my_cmd.Type = cmdbyte[1].Value + 100;
  }
  if (cmdbyte[0].Value == NUMBER_OF_DEVICES + 1) {
    my_cmd.Device = COMMON;
    my_cmd.Type = cmdbyte[1].Value;
  }
  my_cmd.Data1 = cmdbyte[2].Value;
  my_cmd.Data2 = cmdbyte[3].Value;
  my_cmd.Value1 = cmdbyte[4].Value;
  my_cmd.Value2 = cmdbyte[5].Value;
  my_cmd.Value3 = cmdbyte[6].Value;
  my_cmd.Value4 = cmdbyte[7].Value;

  // Save the command
  EEPROM_write_cmd(sel_page, sel_switch, sel_cmd_no, &my_cmd);
  DEBUGMSG("Wrote command!");
  // Recreate indexes
  //EEPROM_create_indexes();

  // Go to switch select page
  current_menu = COMMAND_SELECT_MENU;
  update_page = REFRESH_PAGE;
  update_main_lcd = true;
}


// Functions for command bytes

void load_cmd_byte(uint8_t Sw, uint8_t number) { // Display a command byte on a display.
  uint8_t cmd_no;
  uint8_t cmd_type;
  uint8_t index;
  uint8_t dev;
  uint8_t sel_page;
  uint8_t sel_switch;
  uint8_t sel_cmd_no;
  Cmd_struct sel_cmd;
  uint16_t patch_no;

  String msg, msg1;
  cmd_no = menu[current_menu][number].Sublist; // Command number is in the Sublist variable
  cmd_type = cmdbyte[cmd_no].Type;
  strcpy(SP[Sw].Title, cmdtype[cmd_type].Title); // Copy the label from the cmdbyte array
  switch (cmdtype[cmd_type].Sublist) { // Show sublist if neccesary
    case 0: // If Sublist is 0, we will show the value unless Max value is zero.
      if (cmdtype[cmd_type].Max > 0) {
        msg = String(cmdbyte[cmd_no].Value);
        LCD_set_label(Sw, msg);
      }
      else { // Or clear the label if max value is zero
        LCD_set_label(Sw, Blank_line);
      }
      break;
    case SUBLIST_PATCH: // Copy the patchname from the device
      dev = cmdbyte[0].Value;
      msg = "";
      patch_no = cmdbyte[2].Value + (cmdbyte[3].Value * 100);
      if (dev < NUMBER_OF_DEVICES) Device[dev]->number_format(patch_no, msg);
      else msg = String(patch_no);
      LCD_set_label(Sw, msg);
      break;
    case SUBLIST_PARAMETER:  // Copy the parameter name from the device
      dev = cmdbyte[0].Value;
      msg = "";
      if (dev < NUMBER_OF_DEVICES) Device[dev]->read_parameter_name(cmdbyte[cmd_no].Value, msg);
      else msg = String(cmdbyte[cmd_no].Value);
      LCD_set_label(Sw, msg);
      break;
    case SUBLIST_PAR_STATE: // Copy the parameter state from the device
      dev = cmdbyte[0].Value;
      msg = "";
      if (dev < NUMBER_OF_DEVICES) Device[dev]->read_parameter_state(cmdbyte[2].Value, cmdbyte[cmd_no].Value, msg);
      else msg = String(cmdbyte[cmd_no].Value);
      LCD_set_label(Sw, msg);
      break;
    case SUBLIST_ASSIGN:  // Copy the assign name from the device
      dev = cmdbyte[0].Value;
      msg = "";
      if (dev < NUMBER_OF_DEVICES) Device[dev]->read_assign_name(cmdbyte[cmd_no].Value, msg);
      else msg = String(cmdbyte[cmd_no].Value);
      LCD_set_label(Sw, msg);
      break;
    case SUBLIST_TRIGGER:  // Copy the assign trigger name from the device
      dev = cmdbyte[0].Value;
      msg = "";
      if (dev < NUMBER_OF_DEVICES) Device[dev]->read_assign_trigger(cmdbyte[cmd_no].Value, msg);
      else msg = String(cmdbyte[cmd_no].Value);
      LCD_set_label(Sw, msg);
      break;
    case SUBLIST_PAGE: // Find the page name in EEPROM
      sel_page = cmdbyte[cmd_no].Value;
      if (sel_page == 0) msg = "0: Default";
      else if ((sel_page >= Number_of_pages) && (sel_page < FIRST_FIXED_CMD_PAGE)) msg = String(sel_page) + ": New Page";
      else {
        EEPROM_read_title(sel_page, 0, msg); // Read page name from EEPROM
        if (sel_page >= FIRST_FIXED_CMD_PAGE) msg = "F" + String(sel_page - FIRST_FIXED_CMD_PAGE + 1) + ": " + msg; // Show fixed pages with prefix 'F'
        else msg = String(sel_page) + ": " + msg;
      }
      LCD_set_label(Sw, msg);
      //cmdtype[cmd_type].Max = Number_of_pages; // Set the maximum value for the number of pages
      break;
    case SUBLIST_CMD: // Find the command name, by reading the EEPROM and finding it in the menu sublist
      sel_page = cmdbyte[8].Value;
      sel_switch = cmdbyte[9].Value;
      sel_cmd_no = cmdbyte[cmd_no].Value;
      if (sel_cmd_no >= EEPROM_count_cmds(sel_page, sel_switch)) msg = String(sel_cmd_no + 1) + "/" + String(sel_cmd_no + 1) + ": New command";
      else {
        EEPROM_read_cmd(sel_page, sel_switch, sel_cmd_no, &sel_cmd); // Read this command
        msg = String(sel_cmd_no + 1) + "/" + EEPROM_count_cmds(sel_page, sel_switch) + ": ";
        if (sel_cmd.Device < NUMBER_OF_DEVICES) {
          msg1 = Device[sel_cmd.Device]->device_name;
          msg += msg1;
          msg1 = cmd_sublist[sel_cmd.Type - 100 + 17];
          msg += " " + msg1;
        }
        if (sel_cmd.Device == CURRENT) {
          msg1 = cmd_sublist[sel_cmd.Type - 100 + 17];
          msg += msg1;
        }
        if (sel_cmd.Device == COMMON) {
          //msg += "COM ";
          msg += cmd_sublist[sel_cmd.Type];
        }
      }
      LCD_set_label(Sw, msg);
      break;
    case SUBLIST_DEVICES:
      dev = cmdbyte[0].Value;
      if (dev < NUMBER_OF_DEVICES) strcpy(SP[Sw].Label, Device[dev]->full_device_name);
      if (dev == NUMBER_OF_DEVICES) strcpy(SP[Sw].Label, "Current Device");
      if (dev == NUMBER_OF_DEVICES + 1) strcpy(SP[Sw].Label, "Common Functions");
      break;
    default:
      index = cmdbyte[cmd_no].Value;
      index += cmdtype[cmd_type].Sublist;
      strcpy(SP[Sw].Label, cmd_sublist[index - 1]);  // Copy to the label
      break;
  }
}

void build_command_structure(uint8_t cmd_no, uint8_t cmd_type, bool in_edit_mode) {

  // Will update the status of the other command fields, according to the device, type, or number of parameters
  // Function is called after a command is loaded or the switch of the command has been pressed:

  // *****************************************
  // * BYTE1: Device byte updated            *
  // *****************************************
  if (cmd_no == 0) { // The first byte (device select) has been changed
    if (cmdbyte[0].Value == NUMBER_OF_DEVICES + 1) { // If Device is "Common"
      // Set command to: COMMON, NONE
      set_type_and_value(1, TYPE_COMMON_COMMANDS, selected_common_cmd, in_edit_mode);

      build_command_structure(1, cmdbyte[1].Type, false);

      //clear_cmd_bytes(2, in_edit_mode); // Clear bytes 2-7
    }
    else { // Device is not "common"
      // Set command to: <selected device>, PATCH_SEL, <current_patch_number>
      set_type_and_value(1, TYPE_DEVICE_COMMANDS, selected_device_cmd, in_edit_mode);

      build_command_structure(1, cmdbyte[1].Type, false);

      if (selected_device_cmd == PATCH_SEL - 100) {
        uint8_t dev = cmdbyte[0].Value;
        uint16_t patch_no = 0;
        if (dev < NUMBER_OF_DEVICES) patch_no = Device[dev]->patch_number;
        set_type_and_value(2, TYPE_PATCH_NUMBER, patch_no % 100, in_edit_mode);
        set_type_and_value(3, TYPE_PATCH_100, patch_no / 100, in_edit_mode);
        //clear_cmd_bytes(4, in_edit_mode); // Clear bytes 2-7
      }
    }
    //reload_cmd_menus();
    //update_page = REFRESH_PAGE;
  }

  if (cmd_no == 1) {
    // *****************************************
    // * BYTE2: Common command byte updated    *
    // *****************************************
    if (cmd_type == TYPE_COMMON_COMMANDS) { // The common command byte has been changed
      selected_common_cmd = cmdbyte[cmd_no].Value;
      switch (selected_common_cmd) {
        case OPEN_PAGE:
          // Command: COMMON, OPEN_PAGE, 0
          set_type_and_value(2, TYPE_PAGE, 0, in_edit_mode);
          clear_cmd_bytes(3, in_edit_mode); // Clear bytes 3-7
          break;
        case MIDI_PC:
          // Command: COMMON, MIDI_PC, NUMBER, CHANNEL, PORT
          set_type_and_value(2, TYPE_PC, 0, in_edit_mode);
          set_type_and_value(3, TYPE_MIDI_CHANNEL, 1, in_edit_mode);
          set_type_and_value(4, TYPE_MIDI_PORT, 0, in_edit_mode);
          clear_cmd_bytes(5, in_edit_mode); // Clear bytes 4-7
          break;
        case MIDI_CC:
          // Command: COMMON, CC_number, CC_TOGGLE_TYPE, Value1, Value2, Channel, Port
          set_type_and_value(2, TYPE_CC_NUMBER, 0, in_edit_mode);
          set_type_and_value(3, TYPE_CC_TOGGLE, 0, in_edit_mode);
          set_type_and_value(4, TYPE_MAX, 127, in_edit_mode);
          set_type_and_value(5, TYPE_MIN, 0, in_edit_mode);
          set_type_and_value(6, TYPE_MIDI_CHANNEL, 1, in_edit_mode);
          set_type_and_value(7, TYPE_MIDI_PORT, 0, in_edit_mode);
          break;
        case MIDI_NOTE:
          // Command: COMMON, MIDI_NOTE, NUMBER, VELOCITY, CHANNEL, PORT
          set_type_and_value(2, TYPE_NOTE_NUMBER, 0, in_edit_mode);
          set_type_and_value(3, TYPE_NOTE_VELOCITY, 100, in_edit_mode);
          set_type_and_value(4, TYPE_MIDI_CHANNEL, 1, in_edit_mode);
          set_type_and_value(5, TYPE_MIDI_PORT, 0, in_edit_mode);
          clear_cmd_bytes(6, in_edit_mode); // Clear bytes 6-7
          break;
        case SET_TEMPO:
          // Command: COMMON, SET_TEMPO, number
          set_type_and_value(2, TYPE_BPM, Setting.Bpm, in_edit_mode);
          clear_cmd_bytes(3, in_edit_mode); // Clear bytes 3-7
          break;
        default:
          // For all two byte commands
          clear_cmd_bytes(2, in_edit_mode); // Clear bytes 2-7
          break;
      }
      //reload_cmd_menus();
      //update_page = REFRESH_PAGE;
    }

    // *****************************************
    // * BYTE2: Device command byte updated    *
    // *****************************************
    if (cmd_type == TYPE_DEVICE_COMMANDS) {
      uint16_t patch_no = 0;
      uint8_t dev = 0;
      uint8_t my_trigger = 0;
      selected_device_cmd = cmdbyte[cmd_no].Value;
      switch (selected_device_cmd + 100) {
        case PATCH_SEL:
          // Command: <selected device>, PATCH_SEL, <current_patch_number>
          dev = cmdbyte[0].Value;
          if (dev < NUMBER_OF_DEVICES) patch_no = Device[dev]->patch_number;
          set_type_and_value(2, TYPE_PATCH_NUMBER, patch_no % 100, in_edit_mode);
          set_type_and_value(3, TYPE_PATCH_100, patch_no / 100, in_edit_mode);
          DEBUGMSG("Patchnumber of this device: " + String(patch_no));
          clear_cmd_bytes(4, in_edit_mode); // Clear bytes 3-7
          break;
        case PATCH_BANK:
          // Command: <selected device>, PATCH_BANK, NUMBER, BANK_SIZE
          set_type_and_value(2, TYPE_REL_NUMBER, 1, in_edit_mode);
          set_type_and_value(3, TYPE_BANK_SIZE, 10, in_edit_mode);
          clear_cmd_bytes(4, in_edit_mode); // Clear bytes 3-7
          break;
        case BANK_UP:
        case BANK_DOWN:
          // Command: <selected device>, BANK_UP/DOWN, BANK_SIZE
          set_type_and_value(2, TYPE_BANK_SIZE, 10, in_edit_mode);
          clear_cmd_bytes(3, in_edit_mode); // Clear bytes 3-7
          break;
        case PARAMETER:
          // Command: <selected device>, PARAMETER, NUMBER, TOGGLE, VALUE 1, VALUE 2
          set_type_and_value(2, TYPE_PARAMETER, 0, in_edit_mode);
          set_type_and_value(3, TYPE_TOGGLE, 1, in_edit_mode);
          set_type_and_value(4, TYPE_PAR_STATE, 0, in_edit_mode);
          set_type_and_value(5, TYPE_PAR_STATE, 1, in_edit_mode);
          clear_cmd_bytes(6, in_edit_mode); // Clear bytes 6-7
          break;
        case ASSIGN:
          // Command: <selected device>, ASSIGN, NUMBER
          set_type_and_value(2, TYPE_ASSIGN, 0, in_edit_mode);
          dev = cmdbyte[0].Value;
          if (dev < NUMBER_OF_DEVICES) my_trigger = Device[dev]->trigger_follow_assign(0);
          else my_trigger = 0;
          set_type_and_value(3, TYPE_ASSIGN_TRIGGER, my_trigger, in_edit_mode);
          clear_cmd_bytes(4, in_edit_mode); // Clear bytes 3-7
          break;
        case OPEN_PAGE_DEVICE:
          // Command: COMMON, OPEN_PAGE, 0
          set_type_and_value(2, TYPE_PAGE, 0, in_edit_mode);
          //set_type_and_value(3, TYPE_DEVICE_SELECT, cmdbyte[0].Value, in_edit_mode); // Copy the device type from cmdbyte[0]
          clear_cmd_bytes(3, in_edit_mode); // Clear bytes 3-7
          break;
        default:
          clear_cmd_bytes(2, in_edit_mode); // Clear bytes 2-7
          break;
      }
      //reload_cmd_menus();
      //update_page = REFRESH_PAGE;
    }
  }

  if (cmd_no == 2) {
    // *****************************************
    // * BYTE3: Patch number byte updated      *
    // *****************************************
    if (cmd_type == TYPE_PATCH_NUMBER) {
      uint16_t patch_no = cmdbyte[2].Value + (cmdbyte[3].Value * 100);
      uint8_t Dev = cmdbyte[0].Value;
      if (Dev < NUMBER_OF_DEVICES) {
        if (patch_no >= Device[Dev]->patch_max) { // Check if we've reached the max value
          cmdbyte[2].Value = Device[Dev]->patch_min % 100;
          cmdbyte[3].Value = Device[Dev]->patch_min / 100;
        }
        if (patch_no < Device[Dev]->patch_min) {
          cmdbyte[2].Value = Device[Dev]->patch_max % 100;
          cmdbyte[3].Value = Device[Dev]->patch_max / 100;
        }
      }
    }

    // *****************************************
    // * BYTE3: Parameter byte updated            *
    // *****************************************
    if (cmd_type == TYPE_PARAMETER) {
      uint8_t dev = cmdbyte[0].Value;
      if (dev < NUMBER_OF_DEVICES) {
        // Check if we have reached the max value
        if (cmdbyte[cmd_no].Value >= Device[dev]->number_of_parameters()) cmdbyte[cmd_no].Value = 0;
      }
    }

    // *****************************************
    // * BYTE3: Assign byte updated            *
    // *****************************************
    if (cmd_type == TYPE_ASSIGN) {
      uint8_t dev = cmdbyte[0].Value;
      if (dev < NUMBER_OF_DEVICES) {
        // Check if we have reached the max value
        if (cmdbyte[cmd_no].Value >= Device[dev]->get_number_of_assigns()) cmdbyte[cmd_no].Value = 0;
        // Let the trigger field follow the assign field if neccesary
        uint8_t my_trigger = Device[dev]->trigger_follow_assign(cmdbyte[cmd_no].Value);
        cmdbyte[3].Value = my_trigger;
      }
    }

    // *****************************************
    // * BYTE3: Page byte updated            *
    // *****************************************
    if (cmd_type == TYPE_PAGE) {
      if (cmdbyte[cmd_no].Value == Number_of_pages) cmdbyte[cmd_no].Value = FIRST_FIXED_CMD_PAGE; //Jump over the gap between the external and internal pages
      if (cmdbyte[cmd_no].Value == FIRST_FIXED_CMD_PAGE - 1) cmdbyte[cmd_no].Value = Number_of_pages - 1;
    }
  }

  if (cmd_no == 3) {

    if (cmd_type == TYPE_PATCH_100) {
      uint16_t patch_no = (cmdbyte[3].Value * 100);
      uint8_t Dev = cmdbyte[0].Value;
      if (Dev < NUMBER_OF_DEVICES) {
        if (patch_no >= Device[Dev]->patch_max) { // Check if we've reached the max value
          cmdbyte[3].Value = Device[Dev]->patch_min / 100;
        }
        if (patch_no < Device[Dev]->patch_min) {
          cmdbyte[3].Value = Device[Dev]->patch_max / 100;
        }
      }
    }

    // *****************************************
    // * BYTE4: Toggle type byte updated       *
    // *****************************************
    if (cmd_type == TYPE_TOGGLE) {
      switch (cmdbyte[cmd_no].Value) {
        case TRISTATE:
          // Command: <selected device>, PARAMETER, NUMBER, TRISTATE, VALUE 1, VALUE 2, VALUE3
          set_type_and_value(6, TYPE_PAR_STATE, 1, in_edit_mode);
          clear_cmd_bytes(7, in_edit_mode);
          break;
        case FOURSTATE:
          // Command: <selected device>, PARAMETER, NUMBER, TRISTATE, VALUE 1, VALUE 2, VALUE3
          set_type_and_value(7, TYPE_PAR_STATE, 1, in_edit_mode);
          break;
        case STEP:
          // Command: <selected device>, PARAMETER, NUMBER, TRISTATE, VALUE 1, VALUE 2, VALUE3
          set_type_and_value(6, TYPE_STEP, 1, in_edit_mode);
          clear_cmd_bytes(7, in_edit_mode);
          break;
        default:
          clear_cmd_bytes(6, in_edit_mode);
      }
    }
  }

  if ((cmd_no >= 4) && (cmd_no < 8)) {
    // *****************************************
    // * BYTE5+: Parameter value byte updated   *
    // *****************************************
    if (cmd_type == TYPE_PAR_STATE) {
      uint8_t dev = cmdbyte[0].Value;
      if (dev < NUMBER_OF_DEVICES) {
        // Check if we have reached the max value
        if (cmdbyte[cmd_no].Value >= Device[dev]->number_of_values(cmdbyte[2].Value)) cmdbyte[cmd_no].Value = 0;
      }
    }
  }

  // SWITCH SELECT MENU:

  // *****************************************
  // * BYTE9/11: Device byte updated            *
  // *****************************************
  if (cmd_no == 8) {
    // Check if maximum page is reached - +1 for new page
    if (cmd_type == TYPE_PAGE) {
      if (cmdbyte[cmd_no].Value >= 254) cmdbyte[cmd_no].Value = Number_of_pages; // Value is set to 255 and is then decreased after CMD_BACK has reached zero. Here we set is to the max number of pages.
      if (cmdbyte[cmd_no].Value > Number_of_pages) cmdbyte[cmd_no].Value = 0; // Here we check if we have passed the max value/
      cmdbyte[10].Value = 0; // Select the first command
      update_main_lcd = true;
    }
  }

  if (cmd_no == 9) {
    cmdbyte[10].Value = 0; // Select the first command
    update_main_lcd = true;
  }

  if (cmd_no == 10) {
    // Check if maximum number of commands is reached - +1 for new command
    if (cmd_type == TYPE_CMD) {
      uint8_t sel_page = cmdbyte[8].Value;
      uint8_t sel_switch = cmdbyte[9].Value;
      if (cmdbyte[cmd_no].Value >= EEPROM_count_cmds(sel_page, sel_switch) + 1) cmdbyte[cmd_no].Value = 0;
      update_main_lcd = true;
    }
  }

  if (in_edit_mode) reload_cmd_menus();
}

void clear_cmd_bytes(uint8_t start_byte, bool in_edit_mode) { // Will clear the command bytes that are not needed for a certain command
  for (uint8_t b = start_byte; b < 8; b++) {
    set_type_and_value(b, TYPE_OFF, 0, in_edit_mode);
  }
}

void set_type_and_value(uint8_t number, uint8_t type, uint8_t value, bool in_edit_mode) {
  cmdbyte[number].Type = type;
  if (in_edit_mode) cmdbyte[number].Value = value;
}

void reload_cmd_menus() {
  for (uint8_t i = 8; i <= 12; i++) { // Reload switch 8 - 12
    menu_load(i);
  }
  for (uint8_t i = 4; i <= 8; i++) { // Reload switch 4 - 8
    menu_load(i);
  }
}

void cmdbyte_increase(uint8_t cmd_no) { // Will increase the value of a command byte
  uint8_t cmd_type = cmdbyte[cmd_no].Type;
  if (cmdtype[cmd_type].Max > 0) {
    cmdbyte[cmd_no].Value++;
    if (cmdbyte[cmd_no].Value > cmdtype[cmd_type].Max) cmdbyte[cmd_no].Value = cmdtype[cmd_type].Min;
    build_command_structure(cmd_no, cmd_type, true);
  }
}

void cmdbyte_decrease(uint8_t cmd_no) { // Will decrease the value of a command byte
  uint8_t cmd_type = cmdbyte[cmd_no].Type;
  if (cmdtype[cmd_type].Max > 0) {
    if (cmdbyte[cmd_no].Value == cmdtype[cmd_type].Min) cmdbyte[cmd_no].Value = cmdtype[cmd_type].Max;
    else cmdbyte[cmd_no].Value--;
    build_command_structure(cmd_no, cmd_type, true);
  }
}

void menu_set_main_title() { // Called from main_LCD_control() when the main display is updated and it the page is the menu page
  Main_menu_line1 = menu[current_menu][0].Label;
  switch (current_menu) {
    case COMMAND_SELECT_MENU:
    case COMMAND_EDIT_MENU:
      Main_menu_line2 = "Pg:" + String(cmdbyte[8].Value) + " Sw:" + String(cmdbyte[9].Value) + " Cmd:" + String(cmdbyte[10].Value + 1);
      break;
    case KEYBOARD_MENU:
      Main_menu_line2 = Text_entry;
      break;
    default:
      Main_menu_line2 = Blank_line;
  }
}

// ********************************* Section 5: Text entry ********************************************


void edit_page_name() { // Load page name and start edit. Called from menu
  // Set the Text_entry to the current page name
  page_in_edit = cmdbyte[8].Value;
  switch_in_edit = 0;
  EEPROM_read_title(page_in_edit, switch_in_edit, Text_entry);
  Main_menu_cursor = 1;

  // Select the keyboard menu
  current_menu = KEYBOARD_MENU;
  update_page = REFRESH_PAGE;
  update_main_lcd = true;
}

void edit_switch_name() { // Load page name and start edit. Called from menu
  // Set the Text_entry to the current page name
  page_in_edit = cmdbyte[8].Value;
  switch_in_edit = cmdbyte[9].Value;
  EEPROM_read_title(page_in_edit, switch_in_edit, Text_entry);
  Main_menu_cursor = 1;

  // Select the keyboard menu
  current_menu = KEYBOARD_MENU;
  update_page = REFRESH_PAGE;
  update_main_lcd = true;
}

void close_edit_name() { // Called when pressing OK on the edit name menu page
  EEPROM_write_title(page_in_edit, switch_in_edit, Text_entry);
  EEPROM_clear_indexes();
  EEPROM_create_indexes();

  Main_menu_cursor = 0; // Switch off the cursor
  keyboard_timer_running = false;

  // Select the keyboard menu
  current_menu = COMMAND_SELECT_MENU;
  update_page = REFRESH_PAGE;
  update_main_lcd = true;
}

void delete_switch_name() { // Delete the selected switch name and purge the commands
  // Read the selected command
  uint8_t sel_page = cmdbyte[8].Value;
  uint8_t sel_switch = cmdbyte[9].Value;
  EEPROM_delete_title(sel_page, sel_switch);
  EEPROM_purge_cmds();
  LCD_show_status_message("Name deleted.   ");
  update_page = REFRESH_PAGE;
  update_main_lcd = true;
}

void key_press(uint8_t number) { // Called when key is pressed.
  if ((keyboard_timer_running) && (number != Text_previous_switch)) { // Advance the cursor when a different key is pressed then the previous one and the timer is still running
    cursor_right_page_name();
    Text_switch_char_index = 0;
  }
  else {
    update_main_lcd = true;
  }
  Text_entry[Main_menu_cursor - 1] = menu[current_menu][number].Label[Text_switch_char_index]; // Add the character we typed to the Text_entry string
  Text_switch_char_index++;
  if (Text_switch_char_index >= strlen(menu[current_menu][number].Label)) Text_switch_char_index = 0;

  Text_previous_switch = number;
  keyboard_timer = millis(); // Restart timer
  keyboard_timer_running = true;
}

void check_keyboard_press_expired() { // Will advance the cursor to the right when the timer expires
  if (keyboard_timer_running) { // First check if keyboard input is active
    if (millis() > KEYBOARD_TIME + keyboard_timer) { // Check timer
      cursor_right_page_name(); // Advance cursor
    }
  }
}

void cursor_left_page_name() {
  if (Main_menu_cursor > 1) Main_menu_cursor--;
  update_main_lcd = true;
  keyboard_timer_running = false;
  Text_switch_char_index = 0;
}

void cursor_right_page_name() {
  if (Main_menu_cursor < 16) Main_menu_cursor++;
  update_main_lcd = true;
  keyboard_timer_running = false;
  Text_switch_char_index = 0;
}
