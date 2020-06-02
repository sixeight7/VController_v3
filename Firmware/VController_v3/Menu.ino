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
#define DEVICE_SET 10 // Will set a variable on a device - check MD_base_class::get_setting(uint8_t variable)
#define SET_NO_EXP 11 // Same as set, but not possible to control from expression pedal
#define MIDI_SWITCH_SET 12 // Will set a variable for the selected MIDI switch

uint8_t current_menu = 0; // The menu we are in
uint8_t previous_menu = 0;
uint8_t current_menu_switch = 1; // The switch that is being edited
uint8_t last_main_menu_selection = 1;
//uint8_t show_menu_item = false;
uint8_t Last_set_switch = 0; // The last switch that has been set.
uint8_t Last_cmd_switch = 0; // The current cmdbyte switch we are editing
uint8_t Last_cmd_number = 0; // The current cmdbyte we are editing
uint8_t current_device_when_menu_was_opened = 0;
uint8_t dummy;
char menu_title[LCD_DISPLAY_SIZE + 1];
char menu_label[LCD_DISPLAY_SIZE + 1];

// Variables for text entry
uint8_t page_in_edit = 0; // The page we are editing
uint8_t switch_in_edit = 0; // The switch we are editing
String Text_entry; // The string we use for entering a text
uint8_t Text_previous_switch = 255; // The number of the switch that was pressed before
uint8_t Text_switch_char_index = 0; // The index of the characters of the switch that is being pressed
bool keyboard_timer_running = false;
uint32_t keyboard_timer = 0;
#define KEYBOARD_TIMER_LENGTH 800 // Wait time before forwarding the cursor to the next position
bool no_hold; // Holding switches too long may trigger hold when we don't want it
bool KTN_name_edited = false;
uint8_t Current_MIDI_switch = 1;

struct menu_struct {
  char Label[17];
  uint8_t Type;
  uint8_t Sublist;
  uint8_t Min;
  uint8_t Max;
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
#define CALIBRATION_MENU 10
#define KATANA_MENU 11
#define MIDI_SWITCH_MENU 12
#define MIDI_ADV_MENU 13

#define DEVICE_SUBLIST 255
#define PAGE_SUBLIST 254
#define KTN_NUMBER_SUBLIST 253
#define SWITCH_SUBLIST 252

// The table below has an edited copy in VC-eit/Headers/vcsettings.h

const PROGMEM menu_struct menu[][15] = {
  { // Menu 0 - Select menu
    { "SELECT MENU     ", NONE }, // Menu title
    { "GLOBAL SETTINGS", OPEN_MENU, GLOBAL_MENU }, // Switch 1
    { "DEVICE SETTINGS", OPEN_MENU, DEVICE_MENU }, // Switch 2
    { "MIDI SWITCH MENU", OPEN_MENU, MIDI_SWITCH_MENU },// Switch 3
    { "PROGRAM SWITCHES", OPEN_MENU, COMMAND_SELECT_MENU }, // Switch 4
    { "LED SETTINGS    ", OPEN_MENU, LED_SETTINGS_MENU }, // Switch 5
    { "FX COLOURS MENU ", OPEN_MENU, LED_FX_COLOURS_MENU }, // Switch 6
    { "MIDI ADVNCD MENU", OPEN_MENU, MIDI_ADV_MENU }, // Switch 7
    { "CALIBRATION MENU", OPEN_MENU, CALIBRATION_MENU }, // Switch 8
    { "FIRMWARE MENU", OPEN_MENU, FIRMWARE_MENU }, // Switch 9
    { "", NONE }, // Switch 10
    { "", NONE }, // Switch 11
    { "EXIT MENU", EXIT_MENU, 0 }, // Switch 12
    { "", NONE }, // Switch 13 (LEFT)
    { "", NONE }, // Switch 14 (RIGHT)
  },

  { // Menu 1 - Global settings
    { "GLOBAL SETTINGS ", NONE }, // Menu title
    { "Main disp mode", SET, 19, 0, 3, &Setting.Main_display_mode },// Switch 1
    { "Main disp shows", SET, 48, 0, 1, &Setting.Main_display_show_top_right }, // Switch 2
    { "CURNUM action", SET, 51, 0, 5, &Setting.CURNUM_action }, // Switch 3
    { "MEP also cntrols", SET, 35, 0, 2, &Setting.MEP_control }, // Switch 4
    { "Glob.tempo on PC", SET, 1, 0, 1, &Setting.Send_global_tempo_after_patch_change }, // Switch 5
    { "Hide tempo LED", SET, 1, 0, 1, &Setting.Hide_tap_tempo_LED }, // Switch 6
    { "Backlight Type", SET, 46, 0, 1, &Setting.RGB_Backlight_scheme }, // Switch 7
    { "", NONE }, // Switch 8
    { "", NONE }, // Switch 9
    { "", NONE }, // Switch 10
    { "SAVE & EXIT", SAVE_AND_EXIT, 1 }, // Switch 11
    { "Cancel", SAVE_AND_EXIT, 0 }, // Switch 12
    { "", NONE }, // Switch 13 (LEFT)
    { "", NONE }, // Switch 14 (RIGHT)
  },

  { // Menu 2 - Device settings
    { "DEVICE MENU     ", NONE }, // Menu title
    { "Select device", SET, DEVICE_SUBLIST, 0, NUMBER_OF_DEVICES - 1, &Current_device }, // Switch 1
    { "Enabled", DEVICE_SET, 1, 0, 2, (void*) 10 }, // Switch 2
    { "Midi channel", DEVICE_SET, 0, 1, 16, (void*) 1 }, // Switch 3
    { "Midi port", DEVICE_SET, 24, 0, NUMBER_OF_MIDI_PORTS, (void*) 2 }, // Switch 4
    { "Page #1", DEVICE_SET, PAGE_SUBLIST, 0, LAST_FIXED_CMD_PAGE, (void*) 6 },// Switch 5
    { "Page #2", DEVICE_SET, PAGE_SUBLIST, 0, LAST_FIXED_CMD_PAGE, (void*) 7 }, // Switch 6
    { "Page #3", DEVICE_SET, PAGE_SUBLIST, 0, LAST_FIXED_CMD_PAGE, (void*) 8 }, // Switch 7
    { "Page #4", DEVICE_SET, PAGE_SUBLIST, 0, LAST_FIXED_CMD_PAGE, (void*) 9 }, // Switch 8
    { "Colour", DEVICE_SET, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, (void*) 0 }, // Switch 9
    { "Is always on", DEVICE_SET, 1, 0, 1, (void*) 5 }, // Switch 10
    { "SAVE & EXIT", SAVE_AND_EXIT, 1 }, // Switch 11
    { "Cancel", SAVE_AND_EXIT, 0 }, // Switch 12
    { "", NONE }, // Switch 13 (LEFT)
    { "", NONE }, // Switch 14 (RIGHT)
  },

  { // Menu 3 - LED settings menu
    { "LED SETTINGS    ", NONE }, // Menu title
    { "LED Brightness", SET, 0, 0, 100, &Setting.LED_brightness }, // Switch 1
    { "Backlight Bright", SET, 0, 0, 254, &Setting.Backlight_brightness }, // Switch 2
    { "Virtual LEDs", SET, 1, 0, 1, &Setting.Virtual_LEDs },// Switch 3
    { "FX off is dimmed", SET, 1, 0, 1, &Setting.LED_FX_off_is_dimmed }, // Switch 4
    { "Global colour", SET, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.LED_global_colour }, // Switch 5
    { "MIDI PC colour", SET, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.MIDI_PC_colour }, // Switch 6
    { "MIDI CC colour", SET, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.MIDI_CC_colour }, // Switch 7
    { "MIDI note colour", SET, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.MIDI_note_colour }, // Switch 8
    { "BPM colour", SET, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.LED_bpm_colour }, // Switch 9
    { "BPM sync colour", SET, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.LED_bpm_synced_colour }, // Switch 10
    { "SAVE & EXIT", SAVE_AND_EXIT, 1 }, // Switch 11
    { "Cancel", SAVE_AND_EXIT, 0 }, // Switch 12
    { "", NONE }, // Switch 13 (LEFT)
    { "", NONE }, // Switch 14 (RIGHT)
  },

  { // Menu 4 - LED effect colours
    { "LED FX COLOURS  ", NONE }, // Menu title
    { "GTR/COSM COLOUR", SET, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_GTR_colour }, // Switch 1
    { "PITCH FX COLOUR", SET, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_PITCH_colour }, // Switch 2
    { "FILTER FX COLOUR", SET, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_FILTER_colour },// Switch 3
    { "DIST FX COLOUR", SET, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_DIST_colour }, // Switch 4
    { "AMP FX COLOUR", SET, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_AMP_colour }, // Switch 5
    { "MOD FX COLOUR", SET, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_MODULATION_colour }, // Switch 5
    { "DELAY FX COLOUR", SET, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_DELAY_colour }, // Switch 7
    { "REVERB FX COLOUR", SET, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_REVERB_colour}, // Switch 8
    { "LOOPER COLOUR", SET, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_LOOPER_colour }, // Switch 9
    { "WAH COLOUR", SET, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_WAH_colour }, // Switch 10
    { "DYNAMICS COLOUR", SET, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_DYNAMICS_colour }, // Switch 11
    { "SAVE & EXIT", SAVE_AND_EXIT, 1 }, // Switch 12
    { "", NONE }, // Switch 13 (LEFT)
    { "", NONE }, // Switch 14 (RIGHT)
  },

#ifdef IS_VCMINI
  { // Menu 5 - Command select menu for VC-mini
    { "SELECT PG/SW/CMD", NONE }, // Menu title
    { "", CMD_BYTE, 9 }, // Switch 1
    { "", CMD_BYTE, 10 }, // Switch 2
    { "", CMD_BYTE, 11 },// Switch 3
    { "EDIT COMMAND", EXECUTE, 0, 0, 0, (void*)go_load_cmd }, // Switch 4
    { "EDIT PAGE NAME", EXECUTE, 0, 0, 0, (void*)edit_page_name }, // Switch 5
    { "EDIT SWITCH NAME", EXECUTE, 0, 0, 0, (void*)edit_switch_name }, // Switch 6
    { "DELETE COMMAND", EXECUTE, 0, 0, 0, (void*)go_delete_cmd }, // Switch 7
    { "INITIALIZE PAGE", EXECUTE, 0, 0, 0, (void*)go_init_page }, // Switch 8
    { "INIT SWITCH", EXECUTE, 0, 0, 0, (void*)go_init_switch }, // Switch 9
    { "EXIT", OPEN_MENU, SELECT_MENU }, // Switch 10
    { "", NONE }, // Switch 11
    { "", NONE }, // Switch 12
    { "", NONE }, // Switch 13
    { "", NONE }, // Switch 14
  },
#else
  { // Menu 5 - Command select menu for full VController
    { "SELECT PG/SW/CMD", NONE }, // Menu title
    { "", CMD_BYTE, 9 }, // Switch 1
    { "", CMD_BYTE, 10 }, // Switch 2
    { "", CMD_BYTE, 11 },// Switch 3
    { "", NONE }, // Switch 4
    { "EDIT PAGE NAME", EXECUTE, 0, 0, 0, (void*)edit_page_name }, // Switch 5
    { "EDIT SWITCH NAME", EXECUTE, 0, 0, 0, (void*)edit_switch_name }, // Switch 6
    { "EDIT COMMAND", EXECUTE, 0, 0, 0, (void*)go_load_cmd }, // Switch 7
    { "MORE...", OPEN_MENU, COMMAND_DELETE_MENU }, // Switch 8
    { "DECREASE VALUE", CMD_BACK }, // Switch 9
    { "", NONE }, // Switch 10
    { "", NONE }, // Switch 11
    { "EXIT", OPEN_MENU, SELECT_MENU }, // Switch 12
    { "", NONE }, // Switch 13 (LEFT)
    { "", NONE }, // Switch 14 (RIGHT)
  },
#endif

  { // Menu 6 - Command MORE menu
    { "SELECT PG/SW/CMD", NONE }, // Menu title
    { "", CMD_BYTE, 9 }, // Switch 1 - PAGE select
    { "", CMD_BYTE, 10 }, // Switch 2 - SWITCH select
    { "", CMD_BYTE, 11 },// Switch 3 - COMMAND select
    { "", NONE }, // Switch 4
    { "INITIALIZE PAGE", EXECUTE, 0, 0, 0, (void*)go_init_page }, // Switch 5
    { "INIT SWITCH", EXECUTE, 0, 0, 0, (void*)go_init_switch }, // Switch 6
    { "DELETE COMMAND", EXECUTE, 0, 0, 0, (void*)go_delete_cmd }, // Switch 7
    { "LESS...", OPEN_MENU, COMMAND_SELECT_MENU }, // Switch 8
    { "DECREASE VALUE", CMD_BACK }, // Switch 9
    { "DEL SWITCH NAME", EXECUTE, 0, 0, 0, (void*)delete_switch_name }, // Switch 10
    { "", NONE }, // Switch 11
    { "EXIT", OPEN_MENU, SELECT_MENU }, // Switch 12
    { "", NONE }, // Switch 13 (LEFT)
    { "", NONE }, // Switch 14 (RIGHT)
  },

  { // Menu 7 - Command edit command
    { "Edit COMMAND    ", NONE }, // Menu title
    { "", CMD_BYTE, 0 }, // Switch 1
    { "", CMD_BYTE, 1 }, // Switch 2
    { "", CMD_BYTE, 2 }, // Switch 3
    { "", CMD_BYTE, 3 }, // Switch 4
    { "", CMD_BYTE, 4 }, // Switch 5
    { "", CMD_BYTE, 5 }, // Switch 6
    { "", CMD_BYTE, 6 }, // Switch 7
    { "", CMD_BYTE, 7 }, // Switch 8
    { "", CMD_BYTE, 8 }, // Switch 9
    { "DECREASE VALUE", CMD_BACK}, // Switch 10
    { "SAVE CMD", EXECUTE, 0, 0, 0, (void*)go_save_cmd }, // Switch 11
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
    { "<OK>", EXECUTE, 0, 0, 0, (void*)close_edit_name }, // Switch 12
    { "<=", EXECUTE, 0, 0, 0, (void*)cursor_left_page_name }, // Switch 13 (LEFT)
    { "=>", EXECUTE, 0, 0, 0, (void*)cursor_right_page_name }, // Switch 14 (RIGHT)
  },

  { // Menu 9 - Firmware menu
    { "FIRMWARE MENU   ", NONE }, // Menu title
    { "Init Settings",  EXECUTE, 0, 0, 0, (void*)initialize_settings }, // Switch 1
    { "Init Commands",  EXECUTE, 0, 0, 0, (void*)initialize_commands }, // Switch 2
    { "Init KTN patches",  EXECUTE, 0, 0, 0, (void*)initialize_KTN_patches },// Switch 3
    { "", NONE }, // Switch 4
    { "Program Mode",  EXECUTE, 0, 0, 0, (void*)reboot_program_mode }, // Switch 5
    { "Reboot",  EXECUTE, 0, 0, 0, (void*)reboot }, // Switch 6
    { "", NONE }, // Switch 7
    { "", NONE }, // Switch 8
    { "", NONE }, // Switch 9
    { "", NONE }, // Switch 10
    { "", NONE }, // Switch 11
    { "EXIT",  OPEN_MENU, SELECT_MENU }, // Switch 12
    { "", NONE }, // Switch 13 (LEFT)
    { "", NONE }, // Switch 14 (RIGHT)
  },

  { // Menu 10 - Calibration menu
    { "CALIBRATION MENU", NONE }, // Menu title
    { "Select exp pedal", SET_NO_EXP, 31, 0, NUMBER_OF_CTL_JACKS - 1, &calibrate_exp_pedal }, // Switch 1
    { "Set Max (Toe)",  EXECUTE, 0, 0, 0, (void*)SC_set_expr_max }, // Switch 2
    { "Set Min (Heel)",  EXECUTE, 0, 0, 0, (void*)SC_set_expr_min },// Switch 3
    { "Auto Calibrate  ",  EXECUTE, 0, 0, 0, (void*)SC_set_auto_calibrate }, // Switch 4
    { "", NONE }, // Switch 5
    { "", NONE }, // Switch 6
    { "", NONE }, // Switch 7
    { "", NONE }, // Switch 8
    { "", NONE }, // Switch 9
    { "", NONE }, // Switch 10
    { "SAVE & EXIT", SAVE_AND_EXIT, 1 }, // Switch 11
    { "Cancel", SAVE_AND_EXIT, 0 }, // Switch 12
    { "", NONE }, // Switch 13 (LEFT)
    { "", NONE }, // Switch 14 (RIGHT)
  },


  { // Menu 11 - Katana menu
    { "KATANA SAVE", NONE }, // Menu title
    { "Write to:", SET, KTN_NUMBER_SUBLIST, 0, 80, &My_KTN.save_patch_number }, // Switch 1
    { "Write",  EXECUTE, 0, 0, 0, (void*)KTN_save },// Switch 2
    { "Cancel", EXECUTE, 0, 0, 0, (void*)KTN_exit }, // Switch 3
    { "Rename",  EXECUTE, 0, 0, 0, (void*)KTN_rename }, // Switch 4
    { "Exchange",  EXECUTE, 0, 0, 0, (void*)KTN_exchange }, // Switch 5
    { "", NONE }, // Switch 6
    { "", NONE }, // Switch 7
    { "", NONE }, // Switch 8
    { "", NONE }, // Switch 9
    { "", NONE }, // Switch 10
    { "", NONE }, // Switch 11
    { "", NONE }, // Switch 12
    { "", NONE }, // Switch 13 (LEFT)
    { "", NONE }, // Switch 14 (RIGHT)
  },

  { // Menu 12 - MIDI switch
    { "MIDI SWITCH MENU", NONE }, // Menu title
    { "Select switch", SET, SWITCH_SUBLIST, 1, TOTAL_NUMBER_OF_SWITCHES, &Current_MIDI_switch }, // Switch 1
    { "Type",  MIDI_SWITCH_SET, 38, 0, 4, (void*) 1 }, // Switch 2
    { "Midi port", MIDI_SWITCH_SET, 24, 0, NUMBER_OF_MIDI_PORTS, (void*) 2 }, // Switch 3
    { "Midi channel", MIDI_SWITCH_SET, 0, 1, 16, (void*) 3 }, // Switch 4
    { "CC",  MIDI_SWITCH_SET, 0, 0, 127, (void*) 4 }, // Switch 5
    { "", NONE }, // Switch 6
    { "", NONE }, // Switch 7
    { "", NONE }, // Switch 8
    { "", NONE }, // Switch 9
    { "", NONE }, // Switch 10
    { "SAVE & EXIT", SAVE_AND_EXIT, 1 }, // Switch 11
    { "Cancel", SAVE_AND_EXIT, 0 }, // Switch 12
    { "", NONE }, // Switch 13 (LEFT)
    { "", NONE }, // Switch 14 (RIGHT)
  },

  { // Menu 13 - Midi advanced settings
    { "MIDI ADVNCD MENU", NONE }, // Menu title
    { "Read MIDI clock", SET, 23, 0, NUMBER_OF_MIDI_PORTS + 1, &Setting.Read_MIDI_clock_port }, // Switch 1
    { "Send MIDI clock", SET, 23, 0, NUMBER_OF_MIDI_PORTS + 1, &Setting.Send_MIDI_clock_port }, // Switch 2
    { "", NONE }, // Switch 3
    { "", NONE }, // Switch 4
    { "Bass mode G2M ch", SET, 0, 1, 16, &Setting.Bass_mode_G2M_channel }, // Switch 5
    { "Bass mode device", SET, DEVICE_SUBLIST, 0, NUMBER_OF_DEVICES - 1, &Setting.Bass_mode_device }, // Switch 6
    { "Bass mode CC", SET, 0, 0, 127, &Setting.Bass_mode_cc_number }, // Switch 7
    { "Bass mode min vl", SET, 0, 0, 127, &Setting.Bass_mode_min_velocity}, // Switch 8
    { "HighNotePriotyCC", SET, 0, 0, 127, &Setting.HNP_mode_cc_number }, // Switch 9
    { "", NONE }, // Switch 10
    { "SAVE & EXIT", SAVE_AND_EXIT, 1 }, // Switch 11
    { "Cancel", SAVE_AND_EXIT, 0 }, // Switch 12
    { "", NONE }, // Switch 13 (LEFT)
    { "", NONE }, // Switch 14 (RIGHT)
  },
};


const uint16_t NUMBER_OF_MENUS = sizeof(menu) / sizeof(menu[0]);

const PROGMEM char menu_sublist[][17] = {
  // Sublist 1 - 3: Booleans
  "OFF", "ON", "DETECT",

  // Sublist 4 - 18: LED colours
  "OFF", "GREEN", "RED", "BLUE", "ORANGE", "CYAN", "WHITE", "YELLOW", "PURPLE", "PINK", "SOFT GREEN", "", "", "", "",

  // Sublist 19 - 22: Main display modes
  "PAGE NAME", "PATCH NAME", "PATCHES COMBINED", "VCMINI LABELS",

  // Sublist 23 - 30: MIDI ports
  "OFF", "USB MIDI", "MIDI 1", "MIDI2/RRC", "MIDI 3", "USB HOST PORT", "ALL PORTS", "",

  // Sublist 31 - 34: Expression pedals
  "EXP PEDAL #1", "EXP PEDAL #2", "EXP PEDAL #3", "EXP PEDAL #4",

  // Sublist 35 - 37: MEP control options
  "NONE", "UP/DOWN", "UP/DN + STEP",

  // Sublist 38 - 45: MIDI switch types
  "OFF", "CC MOMENTARY", "CC SINGLE SHOT", "CC RANGE", "PC", "", "", "",

  // Sublist 46 - 47: RGB Display colour schemes
  "ADAFRUIT", "BUYDISPLAY",

  // Sublist 48 - 50: Main display top right types
  "CURRENT DEVICE", "CURRENT TEMPO", "",

  // Sublist 51 - 57: Current number actions
  "OFF", "PREVIOUS PATCH", "TAP TEMPO", "TUNER", "US20 EMULATION", "DIRECT SELECT", "",
};

#define SUBLIST_COLOUR 4

// ********************************* Section 2: Functions called from menu ********************************************
void initialize_settings() {
  if (menu_are_you_sure("Reset settings?", "Sure?")) {
    EEP_initialize_internal_eeprom_data();
    EEP_read_eeprom_common_data();
  }
}

void initialize_commands() {
  if (menu_are_you_sure("Reset commands?", "Sure?")) {
    EEP_initialize_external_eeprom_data();
  }
}

void initialize_KTN_patches() {
  if (menu_are_you_sure("Wipe all Katana", "patches? Sure?")) {
    EEP_initialize_katana_preset_memory();
  }
}

void reboot_program_mode() { // Reboot the Teensy to program mode
  DEBUGMSG("Rebooting to program mode...");
  LCD_show_program_mode();
  delay(200);
  _reboot_Teensyduino_();
}

void reboot() {
  DEBUGMAIN("Rebooting VController...");
  delay(200);
  SCB_AIRCR = 0x05FA0004; // request reset
}

void KTN_save() {
  My_KTN.store_patch();
  KTN_exit();
}

void KTN_rename() {
  // Read patch name into Text_entry
  My_KTN.read_patch_name_from_buffer(Text_entry);

  // Select the keyboard menu
  previous_menu = current_menu;
  current_menu = KEYBOARD_MENU;
  KTN_name_edited = true;
  update_page = REFRESH_PAGE;
  update_main_lcd = true;

  // How do we get out?
}

void KTN_rename_done() {
  My_KTN.store_patch_name_to_buffer(Text_entry);
  KTN_name_edited = false;
}

void KTN_exit() {
  SC_set_enc1_acceleration(true);
  //SCO_select_next_page_of_device(Current_device);
  SCO_select_page(Previous_page);
  update_page = RELOAD_PAGE;
}

void KTN_exchange() {
  bool swapped = My_KTN.exchange_patch();
  if (swapped) KTN_exit();
}


// ********************************* Section 3: Functions that make the menu work ********************************************
void SCO_toggle_menu() { // Will open or close the menu
  if (Current_page != PAGE_MENU) { // Open the menu
    SCO_select_page(PAGE_MENU);
  }
  else { // close the menu
    if (Previous_page == PAGE_MENU) Previous_page = PAGE_DEFAULT;
    set_current_device(current_device_when_menu_was_opened);
    SCO_select_page(Previous_page);
  }
}

void menu_open() { // Called when the menu is started the first time
  SC_set_enc1_acceleration(false);
  if (open_menu_for_Katana_edit) current_menu = KATANA_MENU;
  else {
    current_menu = SELECT_MENU; // Go to top menu
#ifdef IS_VCMINI
    LCD_show_popup_title(" E1:SEL  E2:SET ", MESSAGE_TIMER_LENGTH);
    LCD_show_popup_label("PREV  NEXT   SET", MESSAGE_TIMER_LENGTH);
#endif
  }
  current_menu_switch = 1;
  current_device_when_menu_was_opened = Current_device;
  //show_menu_item = false;
  no_hold = true;
}

void menu_load(uint8_t Sw) {
  uint8_t number = 0;
#ifndef IS_VCMINI
  number = SP[Sw].PP_number;
#else
  if (Sw == MENU_SET_VALUE) { // Coming from expression_pedal
    number = current_menu_switch;
  }
  else {
    switch (SP[Sw].PP_number) {
      case MENU_SELECT:
      case MENU_BACK:
      case MENU_SET_VALUE:
      case MENU_PREV:
      case MENU_NEXT:
        number = current_menu_switch;
        break;
      default:
        return; // Not a valid switch
    }
  }
#endif

  uint8_t *val;
  uint8_t vnumber;
  uint8_t value;
  uint8_t index;
  String msg;
  uint8_t colour;

  //uint8_t dest;
  if (Sw != MENU_BACK) SP[Sw].Colour = Setting.LED_global_colour; // Set default colour

  DEBUGMSG("Loading menu:" + String(current_menu) + ", item:" + String(number) + ", type:" + String(menu[current_menu][number].Type) + ", switch " + String(Sw));
  switch (menu[current_menu][number].Type) {
    case NONE:
      memset(menu_title, ' ', LCD_DISPLAY_SIZE);
      memset(menu_label, ' ', LCD_DISPLAY_SIZE);
      if (Sw != MENU_BACK) SP[Sw].Colour = 0;
      break;
    case CMD_BYTE:
      load_cmd_byte(number);
      break;
    case SAVE_AND_EXIT:
    case EXIT_MENU:
    case OPEN_MENU:
    case EXECUTE:
    case CMD_BACK:
    case KEYBOARD:
      strcpy(menu_title, menu[current_menu][number].Label);
      memset(menu_label, ' ', LCD_DISPLAY_SIZE);
      break;
    case SET:
    case SET_NO_EXP:
      strcpy(menu_title, menu[current_menu][number].Label);
      val = reinterpret_cast<uint8_t*>(menu[current_menu][number].Target);
      if (menu[current_menu][number].Sublist == DEVICE_SUBLIST) {
        if (*val < NUMBER_OF_DEVICES) strcpy(menu_label, Device[*val]->full_device_name);
      }
      else if (menu[current_menu][number].Sublist == SWITCH_SUBLIST) {
        //msg = "Switch " + String(*val); // Need a better solution here
        read_cmd_sublist(25, *val, msg); // 25 = TYPE_SWITCH
        //LCD_set_SP_label(Sw, msg);
        msg.toCharArray(menu_label, LCD_DISPLAY_SIZE + 1);
      }
      else if (menu[current_menu][number].Sublist == KTN_NUMBER_SUBLIST) {
        My_KTN.number_format(My_KTN.save_patch_number + 9 , msg);
        msg += ':';
        EEPROM_read_KTN_title(My_KTN.save_patch_number, msg);
        //LCD_set_SP_label(Sw, msg);
        msg.toCharArray(menu_label, LCD_DISPLAY_SIZE + 1);
      }
      else if (menu[current_menu][number].Sublist > 0) { // Show sublist if neccesary
        // Determine sublist item
        index = *val;
        index += menu[current_menu][number].Sublist;
        strcpy(menu_label, menu_sublist[index - 1]);  // Copy to the label
      }
      else { // just show the value
        msg = String(*val);
        //LCD_set_SP_label(Sw, msg);
        msg.toCharArray(menu_label, LCD_DISPLAY_SIZE + 1);
      }
      if (Sw != MENU_BACK) {
        if (menu[current_menu][number].Sublist == SUBLIST_COLOUR) { // When we set colours, let LED and backlight change
          if (*val == 0) colour = Setting.LED_global_colour; // We need a real color for LED off - otherwise the backlight will be off
          else colour = *val;
        }
        else colour = Setting.LED_global_colour;
        SP[Sw].Colour = colour;
        Main_backlight_show_colour(colour);
      }
      break;
    case MIDI_SWITCH_SET:
      strcpy(menu_title, menu[current_menu][number].Label);
      val = reinterpret_cast<uint8_t*>(menu[current_menu][number].Target);
      vnumber = uint32_t(val); // Here we have the number back that we entered in the menu
      if (Current_MIDI_switch <= TOTAL_NUMBER_OF_SWITCHES) {
        switch (vnumber) {
          case 1:
            value = MIDI_switch[Current_MIDI_switch].type;
            break;
          case 2:
            value = MIDI_switch[Current_MIDI_switch].port;
            break;
          case 3:
            value = MIDI_switch[Current_MIDI_switch].channel;
            break;
          case 4:
            value = MIDI_switch[Current_MIDI_switch].cc;
            break;
          default:
            value = 0;
        }
      }
      else value = 0;

      if (menu[current_menu][number].Sublist > 0) { // Show sublist if neccesary
        // Determine sublist item
        index = value;
        index += menu[current_menu][number].Sublist;
        strcpy(menu_label, menu_sublist[index - 1]);  // Copy to the label
      }
      else { // just show the value
        msg = String(value);
        //LCD_set_SP_label(Sw, msg);
        msg.toCharArray(menu_label, LCD_DISPLAY_SIZE + 1);
      }
      break;
    case DEVICE_SET:
      strcpy(menu_title, menu[current_menu][number].Label);
      val = reinterpret_cast<uint8_t*>(menu[current_menu][number].Target);
      vnumber = uint32_t(val); // Here we have the number back that we entered in the menu
      if (Current_device < NUMBER_OF_DEVICES) value = Device[Current_device]->get_setting(vnumber);
      else value = 0;

      if (menu[current_menu][number].Sublist == PAGE_SUBLIST) {
        if (value == 0)  msg = "---";
        else EEPROM_read_title(value, 0, msg); // Read page name from EEPROM
        //LCD_set_SP_label(Sw, msg);
        msg.toCharArray(menu_label, LCD_DISPLAY_SIZE + 1);
      }
      else if (menu[current_menu][number].Sublist > 0) { // Show sublist if neccesary
        // Determine sublist item
        index = value;
        index += menu[current_menu][number].Sublist;
        strcpy(menu_label, menu_sublist[index - 1]);  // Copy to the label
      }
      else { // just show the value
        msg = String(value);
        //LCD_set_SP_label(Sw, msg);
        msg.toCharArray(menu_label, LCD_DISPLAY_SIZE + 1);
      }

      if (Sw != MENU_BACK) {
        if (menu[current_menu][number].Sublist == SUBLIST_COLOUR) { // When we set colours, let LED and backlight change
          if (value == 0) colour = Setting.LED_global_colour; // We need a real color for LED off - otherwise the backlight will be off
          else colour = value;
        }
        else colour = Setting.LED_global_colour;
        Main_backlight_show_colour(colour);
        SP[Sw].Colour = colour;
      }
      break;
    default:
      strcpy(menu_title, menu[current_menu][number].Label);
      break;
  }

#ifdef IS_VCMINI
  // Show data on main display
  if (!popup_title_showing) LCD_main_set_title(menu_title);
  if (!popup_label_showing) LCD_main_set_label_right(menu_label);
  update_main_lcd = true;
#else
  // Show data on individual display
  strcpy(SP[Sw].Title, menu_title);
  strcpy(SP[Sw].Label, menu_label);
  LCD_update(Sw, true);
  update_LEDS = true;
#endif
}

void menu_set_menu_label(String & lbl) { // Will set the Label string in the SP array
  // Check length does not exceed LABEL_SIZE
  uint8_t len = lbl.length();
  if (len > LCD_DISPLAY_SIZE) len = LCD_DISPLAY_SIZE;
  for (uint8_t i = 0; i < len; i++) {
    menu_label[i] = lbl[i];
  }
  for (uint8_t i = len; i < LCD_DISPLAY_SIZE; i++) { // Fill the remaining chars with spaces
    menu_label[i] = ' ';
  }
}

void menu_press(uint8_t Sw, bool go_up) { // Called when button for this menu is pressed
  uint8_t number = SP[Sw].PP_number;
  if (number == MENU_BACK) { // The menu back button has limited functionality, allowing the user to go back the menu structure and saving everything
    if (current_menu == KEYBOARD_MENU) {
      close_edit_name();
      return;
    }
    if (current_menu == SELECT_MENU) {
      // Exit menu
      if (Previous_page == PAGE_MENU) Previous_page = PAGE_DEFAULT;
      set_current_device(current_device_when_menu_was_opened);
      SCO_select_page(Previous_page);
      return;
    }
    if (current_menu == COMMAND_EDIT_MENU) {
      // Save the command first
      go_save_cmd();
      return;
    }
    if (current_menu == KATANA_MENU) {
      KTN_save();
      return;
    }
    number = current_menu_switch;
    if ((menu[current_menu][number].Type != SAVE_AND_EXIT) && (menu[current_menu][number].Type != EXIT_MENU)) {
      //save current item
      if (current_menu == CALIBRATION_MENU) SC_check_calibration();
      EEP_write_eeprom_common_data();
      check_all_devices_for_manual_connection();
      menu_select(SELECT_MENU);
      return;
    }
  }
  if (number == MENU_SET_VALUE) {
    if (current_menu == KEYBOARD_MENU) {
      key_jump_category();
      return;
    }
    number = current_menu_switch;
  }
  if (number == MENU_PREV) {
    if (current_menu == KEYBOARD_MENU) {
      close_edit_name();
      return;
    }
    menu_select_prev();
    menu_load(Sw);
  }
  if (number == MENU_NEXT) {
    if (current_menu == KEYBOARD_MENU) {
      close_edit_name();
      return;
    }
    menu_select_next();
    menu_load(Sw);
  }
  uint8_t cmd_byte_no;
  uint8_t *val;
  uint8_t vnumber;
  uint8_t value;
  no_hold = true;
  Last_set_switch = 0; // Reset last set switch
  switch (menu[current_menu][number].Type) {
    case CMD_BYTE:
      Last_set_switch = Sw;
      cmd_byte_no = menu[current_menu][number].Sublist; // Command number is in the Sublist variable
      if (go_up) cmdbyte_increase(cmd_byte_no);
      else cmdbyte_decrease(cmd_byte_no);
      Last_cmd_switch = Sw;
      Last_cmd_number = number;
      menu_load(Sw); // Will update the label
      no_hold = false;
      break;
    case CMD_BACK:
      cmd_byte_no = menu[current_menu][Last_cmd_number].Sublist; // Command number is in the Sublist variable
      cmdbyte_decrease(cmd_byte_no);
      menu_load(Last_cmd_switch); // Will update the label
      no_hold = false;
      break;
    case OPEN_MENU:
      menu_select(menu[current_menu][number].Sublist);
      break;
    case SET:
    case SET_NO_EXP:
      Last_set_switch = Sw;
      val = reinterpret_cast<uint8_t*>(menu[current_menu][number].Target);
      if (go_up) {
        if (*val < menu[current_menu][number].Max)*val = *val + 1;
        else *val = menu[current_menu][number].Min;
      }
      else {
        if (*val > menu[current_menu][number].Min)*val = *val - 1;
        else *val = menu[current_menu][number].Max;
      }
      DEBUGMSG("Menu target " + String(number) + " set to value " + String (*val));
      menu_load(Sw); // Will update the label
      if ((menu[current_menu][number].Sublist == DEVICE_SUBLIST) || (menu[current_menu][number].Sublist == SWITCH_SUBLIST)) update_page = REFRESH_PAGE; // So the device menu updates when another device is selected
      no_hold = false;
      break;
    case DEVICE_SET:
      if (Current_device < NUMBER_OF_DEVICES) {
        Last_set_switch = Sw;
        val = reinterpret_cast<uint8_t*>(menu[current_menu][number].Target);
        vnumber = uint32_t(val); // Here we have the number back that we entered in the menu
        value = Device[Current_device]->get_setting(vnumber);
        if (go_up) {
          if (value < menu[current_menu][number].Max) value = value + 1;
          else value = menu[current_menu][number].Min;
        }
        else {
          if (value > menu[current_menu][number].Min) value = value - 1;
          else value = menu[current_menu][number].Max;
        }
        if ((menu[current_menu][number].Sublist == PAGE_SUBLIST) && (value == Number_of_pages)) value = FIRST_FIXED_CMD_PAGE; // Hop over gap in pages
        Device[Current_device]->set_setting(vnumber, value);
        menu_load(Sw); // Will update the label
        no_hold = false;
      }
      break;
    case MIDI_SWITCH_SET:
      if (Current_device < NUMBER_OF_DEVICES) {
        Last_set_switch = Sw;
        val = reinterpret_cast<uint8_t*>(menu[current_menu][number].Target);
        vnumber = uint32_t(val); // Here we have the number back that we entered in the menu
        uint8_t *p = NULL;
        if (Current_MIDI_switch <= TOTAL_NUMBER_OF_SWITCHES) {
          switch (vnumber) {
            case 1:
              p = &MIDI_switch[Current_MIDI_switch].type;
              break;
            case 2:
              p = &MIDI_switch[Current_MIDI_switch].port;
              break;
            case 3:
              p = &MIDI_switch[Current_MIDI_switch].channel;
              break;
            case 4:
              p = &MIDI_switch[Current_MIDI_switch].cc;
              break;
          }
          if (go_up) {
            if (*p < menu[current_menu][number].Max) *p = *p + 1;
            else *p = menu[current_menu][number].Min;
          }
          else {
            if (*p > menu[current_menu][number].Min) *p = *p - 1;
            else *p = menu[current_menu][number].Max;
          }
        }
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
      if (menu[current_menu][number].Sublist == 1) { // Save settings
        if (current_menu == CALIBRATION_MENU) SC_check_calibration();
        EEP_write_eeprom_common_data(); //Save data if sublist is 1
        check_all_devices_for_manual_connection();
      }
      else {
        EEP_read_eeprom_common_data(); // Otherwise return to the settings that have been saved previously in EEPROM
        Current_page = PAGE_MENU; // Restore current page to PAGE_MENU, otherwise we lose our way...
      }
      menu_select(SELECT_MENU);
      break;
    case EXIT_MENU:
      SC_set_enc1_acceleration(true);
      if (Previous_page == PAGE_MENU) Previous_page = PAGE_DEFAULT;
      set_current_device(current_device_when_menu_was_opened);
      SCO_select_page(Previous_page);
      break;
    case NONE:
      break;
  }
}

void menu_press_hold(uint8_t Sw) { // Called when button for this menu is held
  uint8_t number = SP[Sw].PP_number;
  if (number == MENU_SET_VALUE) {
    if (current_menu == KEYBOARD_MENU) {
      close_edit_name();
      return;
    }
    number = current_menu_switch;
  }
  uint8_t cmd_byte_no;
  uint8_t *val;
  uint8_t vnumber;
  uint8_t value;
  if (!no_hold) {
    switch (menu[current_menu][number].Type) {
      case CMD_BYTE:
        cmd_byte_no = menu[current_menu][number].Sublist; // Command number is in the Sublist variable
        cmdbyte_increase(cmd_byte_no);
        Last_cmd_switch = Sw;
        Last_cmd_number = number;
        menu_load(Sw); // Will update the label
        break;
      case CMD_BACK:
        cmd_byte_no = menu[current_menu][Last_cmd_number].Sublist; // Command number is in the Sublist variable
        cmdbyte_decrease(cmd_byte_no);
        menu_load(Last_cmd_switch); // Will update the label
        break;
      case SET:
      case SET_NO_EXP:
        val = reinterpret_cast<uint8_t*>(menu[current_menu][number].Target);
        if (*val < menu[current_menu][number].Max)*val = *val + 1;
        else *val = menu[current_menu][number].Min;
        menu_load(Sw); // Will update the label
        if (menu[current_menu][number].Sublist == DEVICE_SUBLIST) update_page = REFRESH_PAGE; // So the device menu updates when another device is selected
        break;
      case DEVICE_SET:
        if (Current_device < NUMBER_OF_DEVICES) {
          val = reinterpret_cast<uint8_t*>(menu[current_menu][number].Target);
          vnumber = uint32_t(val); // Here we have the number back that we entered in the menu
          value = Device[Current_device]->get_setting(vnumber);
          if (value < menu[current_menu][number].Max) value = value + 1;
          else value = menu[current_menu][number].Min;
          Device[Current_device]->set_setting(vnumber, value);
          menu_load(Sw); // Will update the label
        }
        break;
      case MIDI_SWITCH_SET:
        if (Current_device < NUMBER_OF_DEVICES) {
          val = reinterpret_cast<uint8_t*>(menu[current_menu][number].Target);
          vnumber = uint32_t(val); // Here we have the number back that we entered in the menu
          uint8_t *p = NULL;
          if (Current_MIDI_switch < TOTAL_NUMBER_OF_SWITCHES) {
            switch (vnumber) {
              case 1:
                p = &MIDI_switch[Current_MIDI_switch].type;
                break;
              case 2:
                p = &MIDI_switch[Current_MIDI_switch].port;
                break;
              case 3:
                p = &MIDI_switch[Current_MIDI_switch].channel;
                break;
              case 4:
                p = &MIDI_switch[Current_MIDI_switch].cc;
                break;
            }
            if (*p < menu[current_menu][number].Max) *p = *p + 1;
            else *p = menu[current_menu][number].Min;
          }
          menu_load(Sw); // Will update the label
          no_hold = false;
        }
        break;
    }
  }
}

void menu_move_expr_pedal(uint8_t value) { // Called when the master expression pedal is moved

  if (current_menu == CALIBRATION_MENU) {
    calibrate_exp_pedal = SC_current_exp_pedal();
    SC_display_raw_value();
    reload_menus(1);
    return;
  }

#ifdef IS_VCMINI
  uint8_t number = current_menu_switch;
  uint8_t lcd_for_bar = 0;
  uint8_t lcd_for_update = MENU_SET_VALUE;
#else
  uint8_t number = SP[Last_set_switch].PP_number;
  uint8_t lcd_for_bar = Last_set_switch;
  uint8_t lcd_for_update = Last_set_switch;
  if (Last_set_switch == 0) return; // Exit when last switch moved is zero
#endif

  uint8_t *val;
  uint8_t vnumber;
  uint8_t setting;
  uint8_t no_of_pages;

  no_hold = true;
  switch (menu[current_menu][number].Type) {
    case CMD_BYTE:
      cmdbyte_from_exp_pedal(value, lcd_for_bar, lcd_for_update);
      break;
    case SET:
      LCD_show_bar(lcd_for_bar, value); // Show it on the switch display
      //LCD_show_popup_label(menu[current_menu][number].Label);
      val = reinterpret_cast<uint8_t*>(menu[current_menu][number].Target);
      *val = map (value, 0, 127, menu[current_menu][number].Min, menu[current_menu][number].Max);
      if (menu[current_menu][number].Sublist == DEVICE_SUBLIST) reload_menus(10); // So the device menu updates when another device is selected
      menu_load(lcd_for_update); // Will update the label
      no_hold = false;
      break;
    case DEVICE_SET:
      LCD_show_bar(lcd_for_bar, value); // Show it on the switch display
      //LCD_show_popup_label(menu[current_menu][number].Label);
      if (Current_device < NUMBER_OF_DEVICES) {
        val = reinterpret_cast<uint8_t*>(menu[current_menu][number].Target);
        vnumber = uint32_t(val); // Here we have the number back that we entered in the menu
        setting = Device[Current_device]->get_setting(vnumber);
        if (menu[current_menu][number].Sublist != PAGE_SUBLIST) {
          setting = map (value, 0, 127, menu[current_menu][number].Min, menu[current_menu][number].Max);
        }
        else { // Setting PAGE_SUBLIST. Here we have a gap in the pages
          no_of_pages = LAST_FIXED_CMD_PAGE - FIRST_FIXED_CMD_PAGE + Number_of_pages;
          setting = map (value, 0, 127, 0, no_of_pages);
          if (setting >= Number_of_pages) setting += (FIRST_FIXED_CMD_PAGE - Number_of_pages); // Hop over gap in pages
        }
        Device[Current_device]->set_setting(vnumber, setting);
        menu_load(lcd_for_update); // Will update the label
        no_hold = false;
      }
      break;
    case MIDI_SWITCH_SET:
      LCD_show_bar(lcd_for_bar, value); // Show it on the switch display
      if (Current_device < NUMBER_OF_DEVICES) {
        val = reinterpret_cast<uint8_t*>(menu[current_menu][number].Target);
        vnumber = uint32_t(val); // Here we have the number back that we entered in the menu
        uint8_t *p = NULL;
        if (Current_MIDI_switch < TOTAL_NUMBER_OF_SWITCHES) {
          switch (vnumber) {
            case 1:
              p = &MIDI_switch[Current_MIDI_switch].type;
              break;
            case 2:
              p = &MIDI_switch[Current_MIDI_switch].port;
              break;
            case 3:
              p = &MIDI_switch[Current_MIDI_switch].channel;
              break;
            case 4:
              p = &MIDI_switch[Current_MIDI_switch].cc;
              break;
          }
          *p = map (value, 0, 127, menu[current_menu][number].Min, menu[current_menu][number].Max);
        }
        menu_load(lcd_for_update); // Will update the label
        no_hold = false;
      }
      break;
  }
}

void menu_encoder_turn(uint8_t Sw, signed int value) {
  Last_set_switch = MENU_SET_VALUE;
  uint8_t type = SP[Sw].PP_number;
  if (current_menu == KEYBOARD_MENU) {
    for (uint8_t i = 0; i < abs(value); i++) {
      bool dir_up = (value > 0);
      if (type == MENU_SELECT) key_encoder_select_character(dir_up);
      if (type == MENU_SET_VALUE) key_encoder_edit_character(dir_up);
    }
    return;
  }

  if (type == MENU_SELECT) {
    if (value < 0) {
      for (uint8_t i = 0; i < abs(value); i++) {
        menu_select_prev();
      }
    }
    if (value > 0) {
      for (uint8_t i = 0; i < value; i++) {
        menu_select_next();
      }
    }
    menu_load(Sw); // Will update the label
  }
  if (type == MENU_SET_VALUE) {
    if (value < 0) {
      for (uint8_t i = 0; i < abs(value); i++) {
        // How?
        menu_press(Sw, false);
      }
    }
    if (value > 0) {
      for (uint8_t i = 0; i < value; i++) {
        menu_press(Sw, true);
      }
    }
    update_main_lcd = true;
  }
}

void menu_select_prev() {
  do { // Skip menu items of type NONE
    if (current_menu_switch <= 1) current_menu_switch = 12;
    else current_menu_switch--;
  } while (skip_menu_item_for_encoder());
}

void menu_select_next() {
  do { // Skip menu items of type NONE
    if (current_menu_switch >= 12) current_menu_switch = 1;
    else current_menu_switch++;
  } while (skip_menu_item_for_encoder());
}

typedef void(*f_valueHandler)();

void menu_call_function(void *Target) {
  f_valueHandler callfunction = reinterpret_cast<f_valueHandler>( reinterpret_cast<void*>(Target));
  if (callfunction != 0) {
    callfunction(); // Execute the target function
  }
}

void menu_select(uint8_t _menu) {
  if (current_menu == SELECT_MENU) last_main_menu_selection = current_menu_switch; // Remember the selection we had when leaving the main menu
  current_menu = _menu;
  if (_menu == SELECT_MENU) {
    current_menu_switch = last_main_menu_selection; // So the selection return to the last opened menu
  }
  else {
    current_menu_switch = 1;
  }
  //show_menu_item = false;
  update_page = REFRESH_PAGE;
  update_main_lcd = true;
}

bool menu_are_you_sure(String line1, String line2) {
  LCD_clear_all_displays();
  LCD_show_are_you_sure(line1, line2);

#ifndef IS_VCMINI
  LED_show_are_you_sure();
#endif
  bool sure = SCO_are_you_sure();

  update_page = REFRESH_PAGE;
  update_main_lcd = true;
  update_LEDS = true;

  return (sure);
}

void reload_menus(uint8_t number) { // Will reload the set number of menu's starting at the top row, moving to bottom row
  const uint8_t rows = 3;
  const uint8_t cols = NUMBER_OF_DISPLAYS / rows;

  if (number > NUMBER_OF_DISPLAYS) number = NUMBER_OF_DISPLAYS;

  for (uint8_t i = 0; i <= number; i++) {
    uint8_t my_row = i / cols + 1;
    uint8_t my_col = i % cols + 1;
    menu_load((rows - my_row) * cols + my_col);
  }
}


// ********************************* Section 4: Command build structure ********************************************
// Here we create the user interface for building commands in the VController
// The commandbytes have a number of "types". These are descibed below

uint8_t selected_device_cmd = PATCH - 100;
uint8_t selected_common_cmd = PAGE;

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
#define TYPE_CMDTYPE 5
#define TYPE_MIDI_PORT 6
#define TYPE_MIDI_CHANNEL 7
#define TYPE_CC_NUMBER 8
#define TYPE_CC_TOGGLE 9
#define TYPE_VALUE 10
#define TYPE_NOTE_NUMBER 11
#define TYPE_NOTE_VELOCITY 12
#define TYPE_PC 13
#define TYPE_REL_NUMBER 14
#define TYPE_BANK_SIZE 15
#define TYPE_PATCH_NUMBER 16
#define TYPE_PATCH_100 17
#define TYPE_PARAMETER 18
#define TYPE_PAR_VALUE 19
#define TYPE_ASSIGN 20
#define TYPE_ASSIGN_TRIGGER 21
#define TYPE_TOGGLE 22
#define TYPE_STEP 23
#define TYPE_SWITCH_TRIGGER 24
#define TYPE_SWITCH 25
#define TYPE_CMD 26
#define TYPE_BPM 27
#define TYPE_MIN 28
#define TYPE_MAX 29
#define TYPE_SNAPSCENE 30
#define TYPE_LOOPER 31
#define TYPE_EXP_PEDAL 32
#define TYPE_CMDTYPE_ASSIGN 33

// Some of the data for the sublists below is not fixed, but must be read from a Device class or from EEPROM
// Here we define these sublists
#define SUBLIST_PATCH 255 // To show the patchnumber
#define SUBLIST_PARAMETER 254 // To show the parameter name
#define SUBLIST_PAR_VALUE 253 // To show the parameter state
#define SUBLIST_ASSIGN 252 // To show the assign name
#define SUBLIST_TRIGGER 251 // To show the assign trigger
#define SUBLIST_PAGE 250 // To show the page name
#define SUBLIST_CMD 249 // To show the selected command
#define SUBLIST_DEVICES 248 // To show the devices + Current and Common
#define SUBLIST_PATCH_BANK 247 // To show the bank number

const PROGMEM cmdtype_struct cmdtype[] = {
  { "", 0, 0, 0 }, // TYPE_OFF 0
  { "DEVICE", SUBLIST_DEVICES, 0, (NUMBER_OF_DEVICES + 1) }, // TYPE_DEVICE_SELECT 1
  { "COMMAND", 1, 0, NUMBER_OF_COMMON_TYPES - 1 }, // TYPE_COMMON_COMMANDS 2
  { "COMMAND", 18, 0, NUMBER_OF_DEVICE_TYPES - 1 }, // TYPE_DEVICE_COMMANDS 3
  { "PAGE", SUBLIST_PAGE, 0, LAST_FIXED_CMD_PAGE }, // TYPE_PAGE 4
  { "SELECT TYPE", 117, 0, NUMBER_OF_SELECT_TYPES - 1 }, // TYPE_CMDTYPE 5
  { "MIDI PORT", 40, 0, NUMBER_OF_MIDI_PORTS }, // TYPE_MIDI_PORT 6
  { "MIDI CHANNEL", 0, 1, 16 }, // TYPE_MIDI_CHANNEL 7
  { "CC NUMBER", 0, 0, 127 }, // TYPE_CC_NUMBER 8
  { "CC TOGGLE TYPE", 83, 0, 6 }, // TYPE_CC_TOGGLE 9
  { "VALUE", 0, 0, 127 }, // TYPE_VALUE 10
  { "NOTE NUMBER", 0, 0, 127 }, // NOTE_NUMBER 11
  { "NOTE VELOCITY", 0, 0, 127 }, // TYPE_NOTE_VELOCITY 12
  { "PROGRAM", 0, 0, 127 }, // TYPE_PC 13
  { "NUMBER", 0, 1, 16 }, // TYPE_REL_NUMBER 14
  { "BANK SIZE", 0, 1, 16 }, // TYPE_BANK_SIZE 15
  { "PATCH NUMBER", SUBLIST_PATCH, 0, 99 }, // TYPE_PATCH_NUMBER 16
  { "PATCH BANK (100)", SUBLIST_PATCH_BANK, 0, 255 }, // TYPE_PATCH_100 17
  { "PARAMETER", SUBLIST_PARAMETER, 0, 255 }, // TYPE_PARAMETER 18
  { "VALUE", SUBLIST_PAR_VALUE, 0, 255 }, // TYPE_PAR_VALUE 19
  { "ASSIGN", SUBLIST_ASSIGN, 0, 255 }, // TYPE_ASSIGN 20
  { "TRIGGER", SUBLIST_TRIGGER, 1, 127 }, // TYPE_ASSIGN 21
  { "TOGGLE TYPE", 48, 0, 6 }, // TYPE_TOGGLE 22
  { "STEP", 0, 1, 127 }, // TYPE_STEP 24
  { "SWITCH TRIGGER", 109, 0, 5 }, // TYPE_SWITCH_TRIGGER 24
  { "SWITCH", 58, 0, 24 }, // TYPE_SWITCH 25
  { "COMMAND", SUBLIST_CMD, 0, 255 }, // TYPE_CMD 26
  { "BPM", 0, 40, 250 }, // TYPE_BPM 27
  { "MIN", 0, 0, 127 }, // TYPE_MIN 28
  { "MAX", 0, 0, 127 }, // TYPE_MAX 29
  { "SNAPSHOT/SCENE", 0, 0, 8 }, // TYPE_SNAPSCENE 30
  { "LOOPER", 90, 0, 10 }, // TYPE_LOOPER 31
  { "EXP.PEDAL", 101, 0, 3 }, // TYPE_EXP_PEDAL 32
  { "SELECT TYPE", 117, 0, 3 }, // TYPE_CMDTYPE_ASSIGN 33
};

const PROGMEM char cmd_sublist[][17] = {

  // Sublist 1 - 17: Common Command Types
  "NO COMMAND", "PAGE", "TAP TEMPO", "SET TEMPO", "GLOBAL TUNER", "MIDI PC", "MIDI CC", "MIDI NOTE", "NEXT DEVICE", "MENU", "", "", "", "", "", "", "",

  // Sublist 18 - 39: Device Command Types
  "PATCH", "PARAMETER", "ASSIGN", "SNAPSHOT/SCENE", "LOOPER", "MUTE", "SEL DEVICE PAGE", "SEL NEXT PAGE", "MASTER EXP PEDAL", "TOGGL MASTER EXP",
  "DIR.SELECT", "PAR BANK", "PAR BANK UP", "PAR BANK DOWN", "PARBANK_CATEGORY", "SAVE PATCH", "", "", "", "", "", "",

  // Sublist 40 - 47: MIDI ports
  "USB MIDI", "MIDI 1", "MIDI2/RRC", "MIDI 3", "ALL PORTS", "", "", "",

  // Sublist 48 - 57: Toggle types
  "MOMENTARY", "TOGGLE", "TRISTATE", "FOURSTATE", "STEP",  "RANGE",  "UPDOWN", "", "", "",

  // Sublist 58 - 82: Switch types
#ifndef IS_VCMINI
  "On Page Select", "Switch 1", "Switch 2", "Switch 3", "Switch 4", "Switch 5", "Switch 6", "Switch 7", "Switch 8",
  "Switch 9", "Switch 10", "Switch 11", "Switch 12", "Switch 13", "Switch 14", "Switch 15", "Switch 16",
  "Ext 1 / Exp1", "Ext 2", "Ext 3 / Exp 2", "Ext 4", "Ext 5 / Exp 3", "Ext 6", "Ext 7 / Exp 4", "Ext 8",
#else
  "On Page Select", "Switch 1", "Switch 2", "Switch 3", "Encoder #1", "Encoder #1 SW", "Encoder #2", "Encoder #2 SW", "Ext 1 / Exp1",
  "Ext 2", "MIDI Switch 1", "MIDI Switch 2", "MIDI Switch 3", "MIDI Switch 4", "MIDI Switch 5", "MIDI Switch 6", "MIDI Switch 7",
  "MIDI Switch 8", "MIDI Switch 9", "MIDI Switch 10", "MIDI Switch 11", "MIDI Switch 12", "MIDI Switch 13", "MIDI Switch 14", "MIDI Switch 15",
#endif

  // Sublist 83 - 89: CC toggle types
  "ONE SHOT", "MOMENTARY", "TOGGLE", "TOGGLE ON", "RANGE", "STEP", "UPDOWN",

  // Sublist 90 - 100: CC toggle types
  "OFF", "ON/OFF", "PLAY/STOP", "REC/OVERDUB", "UNDO/REDO", "HALF SPEED", "REVERSE", "PLAY ONCE", "PRE/POST", "REC/PLAY/OVERDUB", "STOP/ERASE",

  // Sublist 101 - 108: EXP pedal types
  "TOGGLE", "EXP1", "EXP2", "EXP3", "", "", "", "",

  // Sublist 109 - 116: Command switch trigger types
  "ON PRESS", "ON RELEASE", "ON LONG PRESS", "ON DUAL PRESS", "ON DUAL RELEASE", "ON DUAL LONG PRS", "", "",

  // Sublist 117 - 124: Page/ patch select types
  "SELECT", "BANK SELECT", "BANK UP", "BANK DOWN", "NEXT", "PREVIOUS", "", "",
};

#define SUBLIST_DEVICE_COMMAND_NUMBER 18
#define SUBLIST_SELECT_TYPE_NUMBER 117

struct cmdbyte_struct {
  uint8_t Type;
  uint8_t Value;
};

#define CB_SWITCH_TRIGGER 0
#define CB_DEVICE 1
#define CB_TYPE 2
#define CB_DATA1 3
#define CB_DATA2 4
#define CB_VAL1 5
#define CB_VAL2 6
#define CB_VAL3 7
#define CB_VAL4 8
#define CB_PAGE 9
#define CB_SWITCH 10
#define CB_CMD_NO 11

cmdbyte_struct cmdbyte[] = {
  // Default command to edit
  { TYPE_SWITCH_TRIGGER, 0 }, // CB_SWITCH_TRIGGER
  { TYPE_DEVICE_SELECT, (NUMBER_OF_DEVICES + 1) }, // CB_DEVICE
  { TYPE_COMMON_COMMANDS, 0}, // CB_TYPE
  { TYPE_OFF, 0}, // CB_DATA1
  { TYPE_OFF, 0}, // CB_DATA2
  { TYPE_OFF, 0}, // CB_VAL1
  { TYPE_OFF, 0}, // CB_VAL2
  { TYPE_OFF, 0}, // CB_VAL3
  { TYPE_OFF, 0}, // CB_VAL4
  // Default select command fields
  { TYPE_PAGE, 1}, // CB_PAGE
  { TYPE_SWITCH, 1}, // CB_SWITCH
  { TYPE_CMD, 0}, // CB_CMD_NO
};

void go_load_cmd() { // Load the selected command and open the edit command menu
  // Read the selected command
  uint8_t sel_page = cmdbyte[CB_PAGE].Value;
  uint8_t sel_switch = cmdbyte[CB_SWITCH].Value;
  uint8_t sel_cmd_no = cmdbyte[CB_CMD_NO].Value;
  current_menu_switch = 1;
  Cmd_struct sel_cmd;
  EEPROM_read_cmd(sel_page, sel_switch, sel_cmd_no, &sel_cmd);
  cmdbyte[CB_SWITCH_TRIGGER].Value = get_switch_trigger_number(sel_cmd.Switch);
  DEBUGMSG("Load cmd: " + String(sel_cmd.Device) + ", "  + String(sel_cmd.Type) + ", " + String(sel_cmd.Data1));
  // Load it into the cmdbyte array and build the structure

  if (sel_cmd.Device < NUMBER_OF_DEVICES) {
    cmdbyte[CB_DEVICE].Type = TYPE_DEVICE_SELECT;
    cmdbyte[CB_DEVICE].Value = sel_cmd.Device;
    cmdbyte[CB_TYPE].Type =  TYPE_DEVICE_COMMANDS;
    cmdbyte[CB_TYPE].Value = sel_cmd.Type - 100;
    DEBUGMSG("Device " + String(sel_cmd.Device) + " has type " + String(cmdbyte[CB_TYPE].Value));
  }
  else if (sel_cmd.Device == CURRENT) {
    cmdbyte[CB_DEVICE].Type = TYPE_DEVICE_SELECT; // Set to current
    cmdbyte[CB_DEVICE].Value = NUMBER_OF_DEVICES;
    cmdbyte[CB_TYPE].Type =  TYPE_DEVICE_COMMANDS;
    cmdbyte[CB_TYPE].Value = sel_cmd.Type - 100;
    DEBUGMSG("Device CURRENT has type " + String(cmdbyte[CB_TYPE].Value));
  }
  else { // Common device type
    cmdbyte[CB_DEVICE].Type = TYPE_DEVICE_SELECT;
    cmdbyte[CB_DEVICE].Value = NUMBER_OF_DEVICES + 1; // Set to common
    cmdbyte[CB_TYPE].Type =  TYPE_COMMON_COMMANDS;
    cmdbyte[CB_TYPE].Value = sel_cmd.Type;
  }

  cmdbyte[CB_DATA1].Value = sel_cmd.Data1;
  cmdbyte[CB_DATA2].Value = sel_cmd.Data2;
  cmdbyte[CB_VAL1].Value = sel_cmd.Value1;
  cmdbyte[CB_VAL2].Value = sel_cmd.Value2;
  cmdbyte[CB_VAL3].Value = sel_cmd.Value3;
  cmdbyte[CB_VAL4].Value = sel_cmd.Value4;

  for (uint8_t i = CB_TYPE; i <= CB_VAL4; i++) {
    build_command_structure(i, cmdbyte[i].Type, false);
  }

  // Select the menu page for editing a command: 4
  current_menu = COMMAND_EDIT_MENU;
  update_page = REFRESH_PAGE;
  update_main_lcd = true;
}

void go_init_page() { // Delete all the commands of the current page
  uint8_t sel_page = cmdbyte[CB_PAGE].Value;
  if (menu_are_you_sure("Clear Page?", "Sure?")) {
    EEPROM_clear_page(sel_page);
    EEPROM_purge_cmds();
    LCD_show_popup_label("Page initialized", MESSAGE_TIMER_LENGTH);
  }
  update_page = REFRESH_PAGE;
  update_main_lcd = true;
}

void go_init_switch() { // Delete all the commands of the current page
  // Read the selected page
  uint8_t sel_page = cmdbyte[CB_PAGE].Value;
  uint8_t sel_switch = cmdbyte[CB_SWITCH].Value;
  if (menu_are_you_sure("Clear switch?", "Sure?")) {
    EEPROM_clear_switch(sel_page, sel_switch);
    EEPROM_purge_cmds();
    LCD_show_popup_label("Switch initialized", MESSAGE_TIMER_LENGTH);
  }
  update_page = REFRESH_PAGE;
  update_main_lcd = true;
}

void go_delete_cmd() { // Delete the selected command
  // Find the selected command
  uint8_t sel_page = cmdbyte[CB_PAGE].Value;
  uint8_t sel_switch = cmdbyte[CB_SWITCH].Value;
  uint8_t sel_cmd_no = cmdbyte[CB_CMD_NO].Value;
  Cmd_struct empty_cmd = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  EEPROM_write_cmd(sel_page, sel_switch, sel_cmd_no, &empty_cmd);
  EEPROM_purge_cmds();
  LCD_show_popup_label("Command deleted ", MESSAGE_TIMER_LENGTH);
  update_page = REFRESH_PAGE;
  update_main_lcd = true;
}

void go_save_cmd() {
  uint8_t sel_page = cmdbyte[CB_PAGE].Value;
  uint8_t sel_switch = cmdbyte[CB_SWITCH].Value;
  uint8_t sel_cmd_no = cmdbyte[CB_CMD_NO].Value;

  DEBUGMAIN("***Saving command on page " + String(sel_page) + " switch " + String(sel_switch) + " and command " + String(sel_cmd_no));

  Cmd_struct my_cmd;
  my_cmd.Page = sel_page;
  my_cmd.Switch = get_switch_trigger_type(cmdbyte[CB_SWITCH_TRIGGER].Value) | sel_switch;

  if (cmdbyte[CB_DEVICE].Value < NUMBER_OF_DEVICES) { // Device
    my_cmd.Device = cmdbyte[CB_DEVICE].Value;
    my_cmd.Type = cmdbyte[CB_TYPE].Value + 100;
  }
  if (cmdbyte[CB_DEVICE].Value == NUMBER_OF_DEVICES) { // Current device
    my_cmd.Device = CURRENT;
    my_cmd.Type = cmdbyte[CB_TYPE].Value + 100;
  }
  if (cmdbyte[CB_DEVICE].Value == NUMBER_OF_DEVICES + 1) { // Common "device"
    my_cmd.Device = COMMON;
    my_cmd.Type = cmdbyte[CB_TYPE].Value;
  }

  my_cmd.Data1 = cmdbyte[CB_DATA1].Value;
  my_cmd.Data2 = cmdbyte[CB_DATA2].Value;
  my_cmd.Value1 = cmdbyte[CB_VAL1].Value;
  my_cmd.Value2 = cmdbyte[CB_VAL2].Value;
  my_cmd.Value3 = cmdbyte[CB_VAL3].Value;
  my_cmd.Value4 = cmdbyte[CB_VAL4].Value;

  // Save the command
  EEPROM_write_cmd(sel_page, sel_switch, sel_cmd_no, &my_cmd);
  //LCD_show_popup_label("Wrote command!", ACTION_TIMER_LENGTH);
  DEBUGMSG("Wrote command for page " + String(sel_page) + " switch " + String(sel_switch) + " and command " + String(sel_cmd_no));
  // Recreate indexes
  //EEPROM_create_indexes();

  // Go to switch select page
  current_menu = COMMAND_SELECT_MENU;
  current_menu_switch = 3; // Go to command select menu option
  update_page = REFRESH_PAGE;
  update_main_lcd = true;
}

uint8_t get_switch_trigger_number(uint8_t sw) {
  switch (sw & SWITCH_TYPE_MASK) {
    case 0: return 0;
    case ON_RELEASE: return 1;
    case ON_LONG_PRESS: return 2;
    case ON_DUAL_PRESS: return 3;
    case ON_DUAL_PRESS | ON_RELEASE: return 4;
    case ON_DUAL_PRESS | ON_LONG_PRESS: return 5;
    default: return 0;
  }
}

uint8_t get_switch_trigger_type(uint8_t value) {
  switch (value) {
    case 0: return 0;
    case 1: return ON_RELEASE;
    case 2: return ON_LONG_PRESS;
    case 3: return ON_DUAL_PRESS;
    case 4: return ON_DUAL_PRESS | ON_RELEASE;
    case 5: return ON_DUAL_PRESS | ON_LONG_PRESS;
    default: return 0;
  }
}

// Functions for command bytes

void load_cmd_byte(uint8_t number) { // Display a command byte on a display.
  String msg;
  uint8_t cmd_byte_no = menu[current_menu][number].Sublist; // Command number is in the Sublist variable
  uint8_t cmd_type = cmdbyte[cmd_byte_no].Type;
  uint8_t cmd_value = cmdbyte[cmd_byte_no].Value;
  strcpy(menu_title, cmdtype[cmd_type].Title); // Copy the title from the cmdbyte array
  read_cmd_sublist(cmd_type, cmd_value, msg);
  //LCD_set_SP_label(Sw, msg);
  msg.toCharArray(menu_label, LCD_DISPLAY_SIZE + 1);
  DEBUGMSG("Cmd_byte_no: " + String(cmd_byte_no));
  DEBUGMSG("Cmd_type: " + String(cmd_type));
}

void read_cmd_sublist(uint8_t cmd_type, uint8_t value, String &msg) {
  uint8_t index;
  uint8_t dev;
  uint8_t sel_page;
  uint8_t sel_switch;
  uint8_t sel_cmd_no;
  Cmd_struct sel_cmd;
  uint16_t patch_no = 0;
  uint16_t bank_low;
  uint16_t bank_high;
  String msg1;

  switch (cmdtype[cmd_type].Sublist) { // Show sublist if neccesary
    case 0: // If Sublist is 0, we will show the value unless Max value is zero.
      if (cmdtype[cmd_type].Max > 0) {
        msg = String(value);
      }
      else { // Or clear the label if max value is zero
        msg = "";
      }
      break;
    case SUBLIST_PATCH: // Copy the patchname from the device
      dev = cmdbyte[CB_DEVICE].Value;
      msg = "";
      patch_no = cmdbyte[CB_DATA2].Value + (cmdbyte[CB_VAL1].Value * 100);
      if (dev < NUMBER_OF_DEVICES) Device[dev]->number_format(patch_no, msg);
      else msg = String(patch_no);
      break;
    case SUBLIST_PATCH_BANK:
      dev = cmdbyte[CB_DEVICE].Value;
      msg = "";
      if (dev < NUMBER_OF_DEVICES) {
        bank_low = cmdbyte[CB_VAL1].Value * 100;
        bank_high = bank_low + 99;
        if (bank_high > Device[dev]->patch_max) bank_high = Device[dev]->patch_max;
        Device[dev]->number_format(bank_low, msg);
        msg += '-';
        Device[dev]->number_format(bank_high, msg);
      }
      else msg = String(patch_no);
      break;
    case SUBLIST_PARAMETER:  // Copy the parameter name from the device
      dev = cmdbyte[CB_DEVICE].Value;
      msg = "";
      if (dev < NUMBER_OF_DEVICES) Device[dev]->read_parameter_name(value, msg);
      else msg = String(value);
      break;
    case SUBLIST_PAR_VALUE: // Copy the parameter state from the device
      dev = cmdbyte[CB_DEVICE].Value;
      msg = "";
      if (dev < NUMBER_OF_DEVICES) Device[dev]->read_parameter_value_name(cmdbyte[CB_DATA1].Value, value, msg);
      else msg = String(value);
      break;
    case SUBLIST_ASSIGN:  // Copy the assign name from the device
      dev = cmdbyte[CB_DEVICE].Value;
      msg = "";
      if (dev < NUMBER_OF_DEVICES) Device[dev]->read_assign_name(value, msg);
      else msg = String(value);
      break;
    case SUBLIST_TRIGGER:  // Copy the assign trigger name from the device
      dev = cmdbyte[CB_DEVICE].Value;
      msg = "";
      if (dev < NUMBER_OF_DEVICES) Device[dev]->read_assign_trigger(value, msg);
      else msg = String(value);
      break;
    case SUBLIST_PAGE: // Find the page name in EEPROM
      sel_page = value;
      if (sel_page == 0) msg = "0: Default";
      else if ((sel_page >= Number_of_pages) && (sel_page < FIRST_FIXED_CMD_PAGE)) msg = String(sel_page) + ": New Page";
      else {
        EEPROM_read_title(sel_page, 0, msg); // Read page name from EEPROM
        if (sel_page >= FIRST_FIXED_CMD_PAGE) msg = "F" + String(sel_page - FIRST_FIXED_CMD_PAGE + 1) + ": " + msg; // Show fixed pages with prefix 'F'
        else msg = String(sel_page) + ": " + msg;
      }
      //cmdtype[cmd_type].Max = Number_of_pages; // Set the maximum value for the number of pages
      break;
    case SUBLIST_CMD: // Find the command name, by reading the EEPROM and finding it in the menu sublist
      sel_page = cmdbyte[CB_PAGE].Value;
      sel_switch = cmdbyte[CB_SWITCH].Value;
      sel_cmd_no = value;
      if (sel_cmd_no >= EEPROM_count_cmds(sel_page, sel_switch)) msg = String(sel_cmd_no + 1) + "/" + String(sel_cmd_no + 1) + ": New command";
      else {
        EEPROM_read_cmd(sel_page, sel_switch, sel_cmd_no, &sel_cmd); // Read this command
        msg = String(sel_cmd_no + 1) + "/" + EEPROM_count_cmds(sel_page, sel_switch) + ": "; // Add command number and number of commands to the string
        if (sel_cmd.Device < NUMBER_OF_DEVICES) {
          msg1 = Device[sel_cmd.Device]->device_name;
          msg += msg1;
          msg1 = cmd_sublist[sel_cmd.Type - 100 + SUBLIST_DEVICE_COMMAND_NUMBER - 1]; // Add command name for device to the string
          msg += ' ' + msg1;
        }
        if (sel_cmd.Device == CURRENT) {
          msg1 = cmd_sublist[sel_cmd.Type - 100 + SUBLIST_DEVICE_COMMAND_NUMBER - 1]; // Add command name for the current device to the string
          msg += msg1;
        }
        if (sel_cmd.Device == COMMON) {
          //msg += "COM ";
          msg += cmd_sublist[sel_cmd.Type]; // Add common command name to the string
        }
        if ((sel_cmd.Type == PAGE) || (sel_cmd.Type == PATCH) || (sel_cmd.Type == ASSIGN)) { // Add SELECT / BANK SELECT / etc to the string
          msg1 = cmd_sublist[sel_cmd.Data1 + SUBLIST_SELECT_TYPE_NUMBER - 1];
          msg += ' ' + msg1;
        }
      }
      break;
    case SUBLIST_DEVICES:
      dev = cmdbyte[CB_DEVICE].Value;
      if (dev < NUMBER_OF_DEVICES) msg = Device[dev]->full_device_name;
      if (dev == NUMBER_OF_DEVICES) msg = "Current Device";
      if (dev == NUMBER_OF_DEVICES + 1) msg = "Common Functions";
      break;
    default: // Static sublist - read it from the cmd_sublist array
      index = value + cmdtype[cmd_type].Sublist;
      msg = cmd_sublist[index - 1];
      break;
  }
}

bool skip_menu_item_for_encoder() {
  uint8_t type = menu[current_menu][current_menu_switch].Type;
  if (type == NONE) return true;
  if (type == CMD_BACK) return true;
  if (type != CMD_BYTE) return false;
  uint8_t cmd_byte_no = menu[current_menu][current_menu_switch].Sublist; // Command number is in the Sublist variable
  return (cmdbyte[cmd_byte_no].Type == TYPE_OFF);
}

void build_command_structure(uint8_t cmd_byte_no, uint8_t cmd_type, bool in_edit_mode) {

  // Will update the status of the other command fields, according to the device, type, or number of parameters
  // Function is called after a command is loaded or the switch of the command has been pressed:

  uint8_t dev = 0;
  uint8_t my_trigger = 0;

  // *******************************************
  // **********     BYTE1 updated     **********
  // *******************************************
  if (cmd_byte_no == CB_DEVICE) { // The first byte (device select) has been changed
    // *****************************************
    // * BYTE1: Device byte updated            *
    // *****************************************
    if (cmdbyte[CB_DEVICE].Value == NUMBER_OF_DEVICES + 1) { // If Device is "Common"
      // Set command to: COMMON, NONE
      set_type_and_value(CB_TYPE, TYPE_COMMON_COMMANDS, selected_common_cmd, in_edit_mode);

      build_command_structure(CB_TYPE, cmdbyte[CB_TYPE].Type, in_edit_mode);

      //clear_cmd_bytes(CB_DATA1, in_edit_mode); // Clear bytes 2-7
    }
    else { // Device is not "common"
      // Set command to: <selected device>, PATCH, SELECT, <current_patch_number>
      set_type_and_value(CB_TYPE, TYPE_DEVICE_COMMANDS, selected_device_cmd, in_edit_mode);

      build_command_structure(CB_TYPE, cmdbyte[CB_TYPE].Type, in_edit_mode);

      if (selected_device_cmd == PATCH - 100) {
        uint8_t dev = cmdbyte[CB_DEVICE].Value;
        uint16_t patch_no = 0;
        if (dev < NUMBER_OF_DEVICES) patch_no = Device[dev]->patch_number;
        set_type_and_value(CB_DATA1, TYPE_CMDTYPE, SELECT, in_edit_mode);
        set_type_and_value(CB_DATA2, TYPE_PATCH_NUMBER, patch_no % 100, in_edit_mode);
        set_type_and_value(CB_VAL1, TYPE_PATCH_100, patch_no / 100, in_edit_mode);
        //clear_cmd_bytes(CB_VAL1, in_edit_mode); // Clear bytes 2-7
      }
    }
    //reload_cmd_menus();
    //update_page = REFRESH_PAGE;
  }

  // *******************************************
  // **********     BYTE2 updated     **********
  // *******************************************

  if (cmd_byte_no == CB_TYPE) {
    // *****************************************
    // * BYTE2: Common command byte updated    *
    // *****************************************
    if (cmd_type == TYPE_COMMON_COMMANDS) { // The common command byte has been changed
      selected_common_cmd = cmdbyte[cmd_byte_no].Value;
      switch (selected_common_cmd) {
        case PAGE:
          // Command: COMMON, PAGE, 0
          set_type_and_value(CB_DATA1, TYPE_CMDTYPE, 0, in_edit_mode);
          set_type_and_value(CB_DATA2, TYPE_PAGE, 0, in_edit_mode);
          clear_cmd_bytes(CB_VAL1, in_edit_mode); // Clear bytes 3-7
          break;
        case MIDI_PC:
          // Command: COMMON, MIDI_PC, NUMBER, CHANNEL, PORT
          set_type_and_value(CB_DATA1, TYPE_PC, 0, in_edit_mode);
          set_type_and_value(CB_DATA2, TYPE_MIDI_CHANNEL, 1, in_edit_mode);
          set_type_and_value(CB_VAL1, TYPE_MIDI_PORT, 0, in_edit_mode);
          clear_cmd_bytes(CB_VAL2, in_edit_mode); // Clear bytes 4-7
          break;
        case MIDI_CC:
          // Command: COMMON, CC_number, CC_TOGGLE_TYPE, Value1, Value2, Channel, Port
          set_type_and_value(CB_DATA1, TYPE_CC_NUMBER, 0, in_edit_mode);
          set_type_and_value(CB_DATA2, TYPE_CC_TOGGLE, 0, in_edit_mode);
          set_type_and_value(CB_VAL1, TYPE_MAX, 127, in_edit_mode);
          set_type_and_value(CB_VAL2, TYPE_MIN, 0, in_edit_mode);
          set_type_and_value(CB_VAL3, TYPE_MIDI_CHANNEL, 1, in_edit_mode);
          set_type_and_value(CB_VAL4, TYPE_MIDI_PORT, 0, in_edit_mode);
          break;
        case MIDI_NOTE:
          // Command: COMMON, MIDI_NOTE, NUMBER, VELOCITY, CHANNEL, PORT
          set_type_and_value(CB_DATA1, TYPE_NOTE_NUMBER, 0, in_edit_mode);
          set_type_and_value(CB_DATA2, TYPE_NOTE_VELOCITY, 100, in_edit_mode);
          set_type_and_value(CB_VAL1, TYPE_MIDI_CHANNEL, 1, in_edit_mode);
          set_type_and_value(CB_VAL2, TYPE_MIDI_PORT, 0, in_edit_mode);
          clear_cmd_bytes(CB_VAL3, in_edit_mode); // Clear bytes 6-7
          break;
        case SET_TEMPO:
          // Command: COMMON, SET_TEMPO, number
          set_type_and_value(CB_DATA1, TYPE_BPM, Setting.Bpm, in_edit_mode);
          clear_cmd_bytes(CB_DATA2, in_edit_mode); // Clear bytes 3-7
          break;
        default:
          // For all two byte commands
          clear_cmd_bytes(CB_DATA1, in_edit_mode); // Clear bytes 2-7
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
      selected_device_cmd = cmdbyte[cmd_byte_no].Value;
      switch (selected_device_cmd + 100) {
        case PATCH:
          // Command: <selected device>, PATCH, SELECT, <current_patch_number>
          dev = cmdbyte[CB_DEVICE].Value;
          if (dev < NUMBER_OF_DEVICES) patch_no = Device[dev]->patch_number;
          set_type_and_value(CB_DATA1, TYPE_CMDTYPE, SELECT, in_edit_mode);
          set_type_and_value(CB_DATA2, TYPE_PATCH_NUMBER, patch_no % 100, in_edit_mode);
          set_type_and_value(CB_VAL1, TYPE_PATCH_100, patch_no / 100, in_edit_mode);
          DEBUGMSG("Patchnumber of this device: " + String(patch_no));
          clear_cmd_bytes(CB_VAL2, in_edit_mode); // Clear bytes 4-7
          break;
        case PARAMETER:
          // Command: <selected device>, PARAMETER, NUMBER, TOGGLE, VALUE 1, VALUE 2
          set_type_and_value(CB_DATA1, TYPE_PARAMETER, 0, in_edit_mode);
          set_type_and_value(CB_DATA2, TYPE_TOGGLE, 1, in_edit_mode);
          set_type_and_value(CB_VAL1, TYPE_PAR_VALUE, 0, in_edit_mode);
          set_type_and_value(CB_VAL2, TYPE_PAR_VALUE, 1, in_edit_mode);
          clear_cmd_bytes(CB_VAL3, in_edit_mode); // Clear bytes 6-7
          set_default_parameter_values(in_edit_mode);
          break;
        case ASSIGN:
          // Command: <selected device>, ASSIGN, NUMBER
          set_type_and_value(CB_DATA1, TYPE_CMDTYPE_ASSIGN, 0, in_edit_mode);
          set_type_and_value(CB_DATA2, TYPE_ASSIGN, 0, in_edit_mode);
          dev = cmdbyte[CB_DEVICE].Value;
          if (dev < NUMBER_OF_DEVICES) my_trigger = Device[dev]->trigger_follow_assign(0);
          else my_trigger = 0;
          set_type_and_value(CB_VAL1, TYPE_ASSIGN_TRIGGER, my_trigger, in_edit_mode);
          clear_cmd_bytes(CB_VAL2, in_edit_mode); // Clear bytes 3-7
          break;
        case OPEN_PAGE_DEVICE:
          // Command: <selected device>, OPEN_PAGE, 0
          set_type_and_value(CB_DATA1, TYPE_PAGE, 0, in_edit_mode);
          clear_cmd_bytes(CB_DATA2, in_edit_mode); // Clear bytes 3-7
          break;
        case SNAPSCENE:
          // Command: <selected device>, SNAPSCENE, 0
          set_type_and_value(CB_DATA1, TYPE_SNAPSCENE, 1, in_edit_mode);
          set_type_and_value(CB_DATA2, TYPE_SNAPSCENE, 0, in_edit_mode);
          set_type_and_value(CB_VAL1, TYPE_SNAPSCENE, 0, in_edit_mode);
          clear_cmd_bytes(CB_VAL2, in_edit_mode); // Clear bytes 5-7
          break;
        case LOOPER:
          // Command: <selected device>, LOOPER, 0
          set_type_and_value(CB_DATA1, TYPE_LOOPER, 2, in_edit_mode);
          clear_cmd_bytes(CB_DATA2, in_edit_mode); // Clear bytes 3-7
          break;
        case MASTER_EXP_PEDAL:
          // Command: <selected device>, MASTER_EXP_EDAL, 0
          set_type_and_value(CB_DATA1, TYPE_EXP_PEDAL, 2, in_edit_mode);
          clear_cmd_bytes(CB_DATA2, in_edit_mode); // Clear bytes 3-7
          break;
        default:
          clear_cmd_bytes(CB_DATA1, in_edit_mode); // Clear bytes 2-7
          break;
      }
      //reload_cmd_menus();
      //update_page = REFRESH_PAGE;
    }
  }

  // *******************************************
  // **********     BYTE3 updated     **********
  // *******************************************

  if (cmd_byte_no == CB_DATA1) {
    // *****************************************
    // * BYTE3: Patch number byte updated      *
    // *****************************************
    if (cmd_type == TYPE_PATCH_NUMBER) {
      uint16_t patch_no = cmdbyte[CB_DATA1].Value + (cmdbyte[CB_DATA2].Value * 100);
      uint8_t Dev = cmdbyte[CB_DEVICE].Value;
      if (Dev < NUMBER_OF_DEVICES) {
        if (patch_no >= Device[Dev]->patch_max) { // Check if we've reached the max value
          cmdbyte[CB_DATA1].Value = Device[Dev]->patch_min % 100;
          cmdbyte[CB_DATA2].Value = Device[Dev]->patch_min / 100;
        }
        if (patch_no < Device[Dev]->patch_min) {
          cmdbyte[CB_DATA1].Value = Device[Dev]->patch_max % 100;
          cmdbyte[CB_DATA2].Value = Device[Dev]->patch_max / 100;
        }
      }
    }

    // *****************************************
    // * BYTE3: Parameter byte updated            *
    // *****************************************
    if (cmd_type == TYPE_PARAMETER) {
      uint8_t dev = cmdbyte[CB_DEVICE].Value;
      if (dev < NUMBER_OF_DEVICES) {
        // Check if we have reached the max value
        if (cmdbyte[cmd_byte_no].Value >= Device[dev]->number_of_parameters()) cmdbyte[cmd_byte_no].Value = 0;
        set_default_parameter_values(in_edit_mode);
      }
    }


    // *****************************************
    // * BYTE3: Command type updated           *
    // *****************************************
    if ((cmd_type == TYPE_CMDTYPE) || (cmd_type == TYPE_CMDTYPE_ASSIGN)) {
      switch (cmdbyte[cmd_byte_no].Value) {
        case SELECT:
          if (current_cmd_function() == PAGE) {
            set_type_and_value(CB_DATA2, TYPE_PAGE, 0, in_edit_mode);
            clear_cmd_bytes(CB_VAL1, in_edit_mode); // Clear bytes 3-7
          }
          if (current_cmd_function() == PATCH) {
            uint8_t dev = cmdbyte[CB_DEVICE].Value;
            uint16_t patch_no = 0;
            if (dev < NUMBER_OF_DEVICES) patch_no = Device[dev]->patch_number;
            set_type_and_value(CB_DATA2, TYPE_PATCH_NUMBER, patch_no % 100, in_edit_mode);
            set_type_and_value(CB_VAL1, TYPE_PATCH_100, patch_no / 100, in_edit_mode);
            clear_cmd_bytes(CB_VAL2, in_edit_mode); // Clear bytes 4-7
          }
          if (current_cmd_function() == ASSIGN) {
            set_type_and_value(CB_DATA2, TYPE_ASSIGN, 0, in_edit_mode);
            dev = cmdbyte[CB_DEVICE].Value;
            if (dev < NUMBER_OF_DEVICES) my_trigger = Device[dev]->trigger_follow_assign(0);
            else my_trigger = 0;
            set_type_and_value(CB_VAL1, TYPE_ASSIGN_TRIGGER, my_trigger, in_edit_mode);
            clear_cmd_bytes(CB_VAL2, in_edit_mode); // Clear bytes 3-7
          }
          break;
        case NEXT:
        case PREV:
          clear_cmd_bytes(CB_DATA2, in_edit_mode); // Clear bytes 3-7
          break;
        case BANKSELECT:
          set_type_and_value(CB_DATA2, TYPE_REL_NUMBER, 1, in_edit_mode);
          set_type_and_value(CB_VAL1, TYPE_BANK_SIZE, 10, in_edit_mode);
          clear_cmd_bytes(CB_VAL2, in_edit_mode); // Clear bytes 3-7
          break;
        case BANKUP:
        case BANKDOWN:
          set_type_and_value(CB_DATA2, TYPE_BANK_SIZE, 10, in_edit_mode);
          clear_cmd_bytes(CB_VAL1, in_edit_mode); // Clear bytes 3-7
          break;
      }
    }
  }

  // *******************************************
  // **********     BYTE4 updated     **********
  // *******************************************

  if (cmd_byte_no == CB_DATA2) {

    // *****************************************
    // * BYTE4: Toggle type byte updated       *
    // *****************************************
    if (cmd_type == TYPE_TOGGLE) {
      switch (cmdbyte[cmd_byte_no].Value) {
        case TRISTATE:
          // Command: <selected device>, PARAMETER, NUMBER, TRISTATE, VALUE 1, VALUE 2, VALUE3
          set_type_and_value(CB_VAL3, TYPE_PAR_VALUE, 1, in_edit_mode);
          clear_cmd_bytes(CB_VAL4, in_edit_mode);
          break;
        case FOURSTATE:
          // Command: <selected device>, PARAMETER, NUMBER, TRISTATE, VALUE 1, VALUE 2, VALUE3
          set_type_and_value(CB_VAL4, TYPE_PAR_VALUE, 1, in_edit_mode);
          break;
        case STEP:
          // Command: <selected device>, PARAMETER, NUMBER, TRISTATE, VALUE 1, VALUE 2, VALUE3
          set_type_and_value(CB_VAL3, TYPE_STEP, 1, in_edit_mode);
          clear_cmd_bytes(CB_VAL4, in_edit_mode);
          break;
        default:
          clear_cmd_bytes(CB_VAL3, in_edit_mode);
      }
      set_default_parameter_values(in_edit_mode);
    }
  }

  // *******************************************
  // **********   BYTE4 - 10 updated  **********
  // *******************************************

  if ((cmd_byte_no >= CB_DATA1) && (cmd_byte_no <= CB_VAL4)) {
    // *****************************************
    // * BYTE4+: Parameter value byte updated  *
    // *****************************************
    if (cmd_type == TYPE_PAR_VALUE) {
      uint8_t dev = cmdbyte[CB_DEVICE].Value;
      if (dev < NUMBER_OF_DEVICES) {
        // Check if we have reached the max value
        if (cmdbyte[cmd_byte_no].Value >= Device[dev]->number_of_values(cmdbyte[CB_DATA1].Value)) cmdbyte[cmd_byte_no].Value = 0;
      }
    }
    // *****************************************
    // * BYTE4+: Page byte updated             *
    // *****************************************
    if (cmd_type == TYPE_PAGE) {
      if (cmdbyte[cmd_byte_no].Value == Number_of_pages) cmdbyte[cmd_byte_no].Value = FIRST_FIXED_CMD_PAGE; // Jump over the gap between the external and internal pages
      if (cmdbyte[cmd_byte_no].Value == FIRST_FIXED_CMD_PAGE - 1) cmdbyte[cmd_byte_no].Value = Number_of_pages - 1;
    }

    // *****************************************
    // * BYTE4+: Type patch 100 byte updated    *
    // *****************************************
    if (cmd_type == TYPE_PATCH_100) { // Check if we reached the minimum or maximum value
      uint16_t patch_no = (cmdbyte[CB_VAL1].Value * 100);
      uint8_t Dev = cmdbyte[CB_DEVICE].Value;
      if (Dev < NUMBER_OF_DEVICES) {
        if (patch_no >= Device[Dev]->patch_max) {
          cmdbyte[CB_VAL1].Value = Device[Dev]->patch_min / 100;
        }
        if (patch_no < Device[Dev]->patch_min) {
          cmdbyte[CB_VAL1].Value = Device[Dev]->patch_max / 100;
        }
      }
    }

    // *****************************************
    // * BYTE4+: Assign number byte updated    *
    // *****************************************

    if (cmd_type == TYPE_ASSIGN) {
      uint8_t dev = cmdbyte[CB_DEVICE].Value;
      if (dev < NUMBER_OF_DEVICES) {
        // Check if we have reached the max value
        if (cmdbyte[cmd_byte_no].Value >= Device[dev]->get_number_of_assigns()) cmdbyte[cmd_byte_no].Value = 0;
        // Let the trigger field follow the assign field if neccesary
        uint8_t my_trigger = Device[dev]->trigger_follow_assign(cmdbyte[cmd_byte_no].Value);
        cmdbyte[CB_VAL1].Value = my_trigger;
      }
    }

  }

  // SWITCH SELECT MENU:

  // *******************************************
  // **********     BYTE9 updated     **********
  // *******************************************

  if (cmd_byte_no == CB_PAGE) {
    // Check if maximum page is reached - +1 for new page
    if (cmd_type == TYPE_PAGE) {
      if (cmdbyte[cmd_byte_no].Value >= 254) cmdbyte[cmd_byte_no].Value = Number_of_pages; // Value is set to 255 and is then decreased after CMD_BACK has reached zero. Here we set is to the max number of pages.
      if (cmdbyte[cmd_byte_no].Value > Number_of_pages) cmdbyte[cmd_byte_no].Value = 0; // Here we check if we have passed the max value/
      cmdbyte[CB_CMD_NO].Value = 0; // Select the first command
      update_main_lcd = true;
    }
  }

  // *******************************************
  // **********     BYTE10 updated    **********
  // *******************************************

  if (cmd_byte_no == CB_SWITCH) {
    cmdbyte[CB_CMD_NO].Value = 0; // Select the first command
    update_main_lcd = true;
  }

  // *******************************************
  // **********     BYTE11 updated    **********
  // *******************************************

  if (cmd_byte_no == CB_CMD_NO) {
    // Check if maximum number of commands is reached - +1 for new command
    if (cmd_type == TYPE_CMD) {
      uint8_t sel_page = cmdbyte[CB_PAGE].Value;
      uint8_t sel_switch = cmdbyte[CB_SWITCH].Value;
      if (cmdbyte[cmd_byte_no].Value >= EEPROM_count_cmds(sel_page, sel_switch) + 1) cmdbyte[cmd_byte_no].Value = 0;
      update_main_lcd = true;
    }
  }

  if (in_edit_mode) reload_menus(8);
}

uint8_t current_cmd_function() {
  if (cmdbyte[CB_DEVICE].Value == NUMBER_OF_DEVICES + 1) { // Common device
    return cmdbyte[CB_TYPE].Value;
  }
  else { // Current device
    return cmdbyte[CB_TYPE].Value + 100;
  }
}

void clear_cmd_bytes(uint8_t start_byte, bool in_edit_mode) { // Will clear the command bytes that are not needed for a certain command
  for (uint8_t b = start_byte; b <= CB_VAL4; b++) {
    set_type_and_value(b, TYPE_OFF, 0, in_edit_mode);
  }
}

void set_type_and_value(uint8_t number, uint8_t type, uint8_t value, bool in_edit_mode) {
  cmdbyte[number].Type = type;
  if (in_edit_mode) cmdbyte[number].Value = value;
}

void set_default_parameter_values(bool in_edit_mode) {
  if (!in_edit_mode) return;
  uint8_t dev = cmdbyte[CB_DEVICE].Value;
  if (dev >= NUMBER_OF_DEVICES) return;
  uint8_t max = Device[dev]->number_of_values(cmdbyte[CB_DATA1].Value);
  if (max > 0) max--;
  switch (cmdbyte[CB_DATA2].Value) {
    case TRISTATE:
      cmdbyte[CB_VAL1].Value = 0;
      cmdbyte[CB_VAL2].Value = max / 2;
      cmdbyte[CB_VAL3].Value = max;
      break;
    case FOURSTATE:
      cmdbyte[CB_VAL1].Value = 0;
      cmdbyte[CB_VAL2].Value = max / 3;
      cmdbyte[CB_VAL3].Value = (max * 2) / 3;
      cmdbyte[CB_VAL4].Value = max;
      break;
    case TOGGLE:
    case MOMENTARY:
      cmdbyte[CB_VAL1].Value = max;
      cmdbyte[CB_VAL2].Value = 0;
      break;
    default:
      cmdbyte[CB_VAL1].Value = 0;
      cmdbyte[CB_VAL2].Value = max;
  }
}

void cmdbyte_increase(uint8_t cmd_byte_no) { // Will increase the value of a command byte
  uint8_t cmd_type = cmdbyte[cmd_byte_no].Type;
  if (cmdtype[cmd_type].Max > 0) {
    cmdbyte[cmd_byte_no].Value++;
    if (cmdbyte[cmd_byte_no].Value > cmdtype[cmd_type].Max) cmdbyte[cmd_byte_no].Value = cmdtype[cmd_type].Min;
    build_command_structure(cmd_byte_no, cmd_type, true);
  }
}

void cmdbyte_decrease(uint8_t cmd_byte_no) { // Will decrease the value of a command byte
  uint8_t cmd_type = cmdbyte[cmd_byte_no].Type;
  if (cmdtype[cmd_type].Max > 0) {
    if (cmdbyte[cmd_byte_no].Value <= cmdtype[cmd_type].Min) cmdbyte[cmd_byte_no].Value = cmdtype[cmd_type].Max;
    else cmdbyte[cmd_byte_no].Value--;
    build_command_structure(cmd_byte_no, cmd_type, true);
  }
}

void cmdbyte_from_exp_pedal(uint8_t value, uint8_t lcd_for_bar, uint8_t lcd_for_update) { // Set the value of a command byte from the expression pedal
#ifdef IS_VCMINI
  uint8_t cmd_byte_no = menu[current_menu][current_menu_switch].Sublist; // Command number is in the Sublist variable
#else
  uint8_t cmd_byte_no = menu[current_menu][Last_cmd_number].Sublist; // Command number is in the Sublist variable
#endif
  uint8_t cmd_type = cmdbyte[cmd_byte_no].Type;
  if (cmdtype[cmd_type].Max > 0) {
    cmdbyte[cmd_byte_no].Value = map(value, 0, 127, cmdtype[cmd_type].Min, cmdtype[cmd_type].Max);
    //LCD_show_popup_label(cmdtype[cmd_type].Title);
    build_command_structure(cmd_byte_no, cmd_type, true);
    menu_load(lcd_for_update); // So the correct text shows below the display
    LCD_show_bar(lcd_for_bar, value); // Show it on the Sw display
  }
}

void menu_set_main_title() { // Called from main_LCD_control() when the main display is updated and it the page is the menu page
#ifndef IS_VCMINI
  // Full VController main display screen
  LCD_main_set_title(menu[current_menu][0].Label);
  switch (current_menu) {
    case COMMAND_SELECT_MENU:
    case COMMAND_EDIT_MENU:
      LCD_main_set_label("Pg:" + String(cmdbyte[CB_PAGE].Value) + " Sw:" + String(cmdbyte[CB_SWITCH].Value) + " Cmd:" + String(cmdbyte[CB_CMD_NO].Value + 1));
      break;
    case KEYBOARD_MENU:
      if (!popup_label_showing) LCD_main_set_label(Text_entry);
      break;
  }
#else
  // VCmini menu
  // Show menu title and label of the current menu for the menu items that have one line
  if ((menu[current_menu][current_menu_switch].Type == OPEN_MENU) || (menu[current_menu][current_menu_switch].Type == EXECUTE) || (menu[current_menu][current_menu_switch].Type == SAVE_AND_EXIT)) {
    LCD_main_set_title(menu[current_menu][0].Label);
    if (!popup_label_showing) LCD_main_set_label(menu[current_menu][current_menu_switch].Label);
  }
  else if (current_menu == KEYBOARD_MENU) {
    LCD_main_set_title(menu[current_menu][0].Label);
    if (!popup_label_showing) LCD_main_set_label(Text_entry);
  }
  else {
    menu_load(MENU_BACK);
    if ((current_menu == DEVICE_MENU) && (menu[current_menu][current_menu_switch].Type == DEVICE_SET) && (Current_device < NUMBER_OF_DEVICES)) { // Show adapted device menu name on VCmini
      String msg = Device[Current_device]->device_name;
      msg += ' ';
      msg += menu[current_menu][current_menu_switch].Label;
      LCD_main_set_title(msg);
    }
    else if ((current_menu == MIDI_SWITCH_MENU) && (menu[current_menu][current_menu_switch].Type == MIDI_SWITCH_SET)) { // Show adapter switch menu name on VCmini
      String msg = "SW" + String(Current_MIDI_switch) + ' ';
      msg += menu[current_menu][current_menu_switch].Label;
      LCD_main_set_title(msg);
    }
    else if (menu[current_menu][current_menu_switch].Type != CMD_BYTE) {
      LCD_main_set_title(menu[current_menu][current_menu_switch].Label);
    }
    else {
      LCD_main_set_title(menu_title);
    }
  }
#endif
}

// ********************************* Section 5: Text entry ********************************************


void edit_page_name() { // Load page name and start edit. Called from menu
  // Set the Text_entry to the current page name
  page_in_edit = cmdbyte[CB_PAGE].Value;
  switch_in_edit = 0;
  EEPROM_read_title(page_in_edit, switch_in_edit, Text_entry);
  Main_menu_cursor = 1;

  // Select the keyboard menu
  previous_menu = current_menu;
  current_menu = KEYBOARD_MENU;
  update_page = REFRESH_PAGE;
  update_main_lcd = true;
}

void edit_switch_name() { // Load page name and start edit. Called from menu
  // Set the Text_entry to the current page name
  page_in_edit = cmdbyte[CB_PAGE].Value;
  switch_in_edit = cmdbyte[CB_SWITCH].Value;
  EEPROM_read_title(page_in_edit, switch_in_edit, Text_entry);
  Main_menu_cursor = 1;

  // Select the keyboard menu
  previous_menu = current_menu;
  current_menu = KEYBOARD_MENU;
  update_page = REFRESH_PAGE;
  update_main_lcd = true;
}

void close_edit_name() { // Called when pressing OK on the edit name menu page
  if (KTN_name_edited) {
    KTN_rename_done();
  }
  else {
    EEPROM_write_title(page_in_edit, switch_in_edit, Text_entry);
    EEPROM_create_indexes();
  }

  Main_menu_cursor = 0; // Switch off the cursor
  keyboard_timer_running = false;

  // Return to previous menu
  current_menu = previous_menu;
  update_page = REFRESH_PAGE;
  update_main_lcd = true;
}

void delete_switch_name() { // Delete the selected switch name and purge the commands
  // Read the selected command
  uint8_t sel_page = cmdbyte[CB_PAGE].Value;
  uint8_t sel_switch = cmdbyte[CB_SWITCH].Value;
  EEPROM_delete_title(sel_page, sel_switch);
  EEPROM_purge_cmds();
  LCD_show_popup_label("Name deleted.   ", MESSAGE_TIMER_LENGTH);
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

void key_encoder_select_character(bool dir_up) {
  if (dir_up) {
    cursor_right_page_name();
  }
  else {
    cursor_left_page_name();
  }
}

void key_encoder_edit_character(bool dir_up) {

  // The ASCII character set:
  // Char 32 - 47: <space>!"#$%&'()*+,-./
  // Char 48 - 57: 0123456789
  // Char 58 - 64: :;<=>?@
  // Char 65 - 90: ABCDEFGHIJKLMNOPQRSTUVWXYZ
  // Char 91 - 96: [Y]^_`
  // Chr 97 - 122: abcdefghijklmnopqrstuvwxyz

  char my_char = Text_entry[Main_menu_cursor - 1];
  if (dir_up) {
    switch (my_char) { // Select next character
      case 32: my_char = 65; break; // Jump from <space> to A
      case 90: my_char = 97; break; // Jump from Z to a
      case 122: my_char = 48; break;// Jump from z to 0
      case 57: my_char = 33; break; // Jump from 9 to !
      case 47: my_char = 58; break; // Jump from / to :
      case 64: my_char = 32; break; // Jump from @ to <space>
      default: my_char++;
    }
  }
  else {
    switch (my_char) { // Select previous character
      case 65: my_char = 32; break; // Jump from A to <space>
      case 97: my_char = 90; break; // Jump from a to Z
      case 48: my_char = 122; break;// Jump from 0 to z
      case 33: my_char = 57; break; // Jump from ! to 9
      case 58: my_char = 47; break; // Jump from : to /
      case 32: my_char = 64; break; // Jump from <space> to @
      default: my_char--;
    }
  }
  Text_entry[Main_menu_cursor - 1] = my_char;
  update_main_lcd = true;
}

void key_jump_category() {
  char my_char = Text_entry[Main_menu_cursor - 1];

  if ((my_char >= 65) && (my_char <= 90)) my_char = 97; // Jump from [A-Z] to a
  else if ((my_char >= 97) && (my_char <= 122)) my_char = 48; // Jump from [a-z] to 0
  else if ((my_char >= 48) && (my_char <= 57)) my_char = 32; // Jump from [0-9] to <space>
  else my_char = 65; // else jump to A

  Text_entry[Main_menu_cursor - 1] = my_char;
  update_main_lcd = true;
}

void check_keyboard_press_expired() { // Will advance the cursor to the right when the timer expires
  if (keyboard_timer_running) { // First check if keyboard input is active
    if (millis() > KEYBOARD_TIMER_LENGTH + keyboard_timer) { // Check timer
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
