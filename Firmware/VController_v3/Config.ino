// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: VController commands
// Section 2: VController configuration for fixed command pages
// Section 3: VController default configuration for programmable pages
// Section 4: VC-mini configuration for fixed command pages
// Section 5: VC-mini configuration for programmable pages
// Section 6: CUSTOM configuration for fixed command pages
// Section 7: CUSTOM configuration for programmable pages

// ********************************* Section 1: VController commands ********************************************

// Here we define the supported devices
#define NUMBER_OF_DEVICES 14
#define GP10 0
#define GR55 1
#define VG99 2
#define ZG3 3
#define ZMS70 4
#define M13 5
#define HLX 6
#define AXEFX 7
#define KTN 8
#define KPA 9
#define SVL 10
#define SY1000 11
#define GM2 12
#define MG300 13

#define CURRENT 254 // To select the current device
#define COMMON 255 // Not really a device, but used for common procedures, that are not device specific or for all devices.

//Possible switch types:
#define LABEL 0xE0 // Bit 6 - 8 of the switch number is set to one all labels - this saved one byte and keeps the command structure short.
#define ON_DUAL_PRESS 0x80 // Switch and right neighbouring switch are pressed
#define ON_LONG_PRESS 0x40
#define ON_RELEASE 0x20
#define SWITCH_MASK 0x1F
#define SWITCH_TYPE_MASK 0xE0

// Example:  {<PAGE>, <SWITCH> | LABEL, 'P', 'A', 'T', 'C', 'H', ' ', ' ', ' ' }

// Common command types
#define NUMBER_OF_COMMON_TYPES 9 // Only the one that can be selected in the command builder
#define NOTHING 0           // Example: {<PAGE>, <SWITCH>, NOTHING, COMMON}
#define PAGE 1              // Example: {<PAGE>, <SWITCH>, PAGE, COMMON, SELECT, PAGE_COMBO1}
#define TAP_TEMPO 2         // Example: {<PAGE>, <SWITCH>, TAP_TEMPO, COMMON}
#define SET_TEMPO 3         // Example: {<PAGE>, <SWITCH>, SET_TEMPO, COMMON, 120}
#define GLOBAL_TUNER 4      // Example: {<PAGE>, <SWITCH>, GLOBAL_TUNER, COMMON}
#define MIDI_PC 5           // Example: {<PAGE>, <SWITCH>, MIDI_PC, COMMON, Program, Channel, Port}
#define MIDI_CC 6           // Example: {<PAGE>, <SWITCH>, MIDI_CC, COMMON, Controller, CC_TOGGLE_TYPE, Value1, Value2, Channel, Port}
#define MIDI_NOTE 7         // Example: {<PAGE>, <SWITCH>, MIDI_NOTE, COMMON, Note, Velocity, Channel, Port}
#define SELECT_NEXT_DEVICE 8// Example: {<PAGE>, <SWITCH>, SELECT_NEXT_DEVICE, COMMON}

// Common functions that can not be selected in the command builder
#define NUMBER_OF_COMMON_TYPES_NOT_SELECTABLE 1
#define MENU 9             // Example: {<PAGE>, <SWITCH>, MENU, COMMON, 1} - to display first menu item


// Device command types
#define NUMBER_OF_DEVICE_TYPES 10
#define PATCH 100      // Example: {<PAGE>, <SWITCH>, PATCH, <DEVICE>, 25, 1} - to select patch 125
#define PARAMETER 101      // Example: {<PAGE>, <SWITCH>, PARAMETER, <DEVICE>, 1, TOGGLE, 1, 0}
#define ASSIGN 102         // Example: {<PAGE>, <SWITCH>, ASSIGN, <DEVICE>, SELECT, 1, 21}
#define SNAPSCENE 103      // Example: {<PAGE>, <SWITCH>, SNAPSCENE, <DEVICE>, 1}
#define LOOPER 104         // Example: {<PAGE>, <SWITCH>, LOOPER, <DEVICE>, START}
#define MUTE 105           // Example: {<PAGE>, <SWITCH>, MUTE, <DEVICE>} 
#define OPEN_PAGE_DEVICE 106     // Example: {<PAGE>, <SWITCH>, OPEN_PAGE_DEVICE, ZG3, PAGE_ZOOM_PATCH_BANK}
#define OPEN_NEXT_PAGE_OF_DEVICE 107      // Example: {<PAGE>, <SWITCH>, OPEN_NEXT_PAGE_OF_DEVICE, <DEVICE>}
#define MASTER_EXP_PEDAL 108 // Example: {<PAGE>, <SWITCH>, MASTER_EXP_PEDAL, <DEVICE>, <number>} number of expression pedal: 0, 1, 2 or 3. If 0, it will choose the number as selected with TOGGLE_EXP_PEDAL
#define TOGGLE_EXP_PEDAL 109 // Example: {<PAGE>, <SWITCH>, TOGGLE_EXP_PEDAL, <DEVICE>}

//Common device types that can not be selected in the command builder
#define NUMBER_OF_DEVICE_TYPES_NOT_SELECTABLE 6
#define DIRECT_SELECT 110  // Example: {<PAGE>, <SWITCH>, DIRECT_SELECT, <DEVICE>, 1, 10}
#define PAR_BANK 111       // Example: {<PAGE>, <SWITCH>, PAR_BANK, <DEVICE>, 1, 10}
#define PAR_BANK_UP 112    // Example: {<PAGE>, <SWITCH>, PAR_BANK_UP, <DEVICE>, 10}
#define PAR_BANK_DOWN 113  // Example: {<PAGE>, <SWITCH>, PAR_BANK_DOWN, <DEVICE>, 10}
#define PAR_BANK_CATEGORY 114   // Example: {<PAGE>, <SWITCH>, PAR_BANK_CATEGORY, <DEVICE>, 1}
#define SAVE_PATCH 115    // Example: {<PAGE>, <SWITCH>, TOGGLE_EXP_PEDAL, <DEVICE>}

// Page and patch select types
#define SELECT 0           // Example: {<PAGE>, <SWITCH>, PAGE, COMMON, SELECT, PAGE_COMBO1}
#define BANKSELECT 1      // Example: {<PAGE>, <SWITCH>, PAGE, COMMON, BANKSELECT, 1, 10} - to select the first item in a bank of 10
#define BANKUP 2          // Example: {<PAGE>, <SWITCH>, PAGE, COMMON, BANKUP, 10}
#define BANKDOWN 3        // Example: {<PAGE>, <SWITCH>, PAGE, COMMON, BANKDOWN, 10}
#define NEXT 4             // Example: {<PAGE>, <SWITCH>, PAGE, COMMON, NEXT}
#define PREV 5             // Example: {<PAGE>, <SWITCH>, PAGE, COMMON, PREV}
#define NUMBER_OF_SELECT_TYPES 6

// Toggle types
#define MOMENTARY 0
#define TOGGLE 1
#define TRISTATE 2
#define FOURSTATE 3
#define STEP 4 // Set minimum, maximum and step value
#define RANGE 5 // For use with expression pedal
#define UPDOWN 6 // Press and hold to increase/decrease. Press shortly to change direction
#define TGL_OFF 255 // To show nothing

// Special max values:
#define TIME_2000 255 // For delay times up to 2000 ms
#define TIME_1000 254 // For delay times up to 1000 ms
#define TIME_500 253 // For pre delay times up to 500 ms
#define TIME_300 252 // For pre delay times up to 300 ms
#define RPT_600 251 // For Katana DC30 with min of 40 ms and max of 600 ms

// CC toggle types - had to be a limited list
#define CC_ONE_SHOT 0 // Send a single CC message on press
#define CC_MOMENTARY 1 // Send first message on press, second on release
#define CC_TOGGLE 2 // Default state off - send message 1 on first press, second message on second press
#define CC_TOGGLE_ON 3 // Default state on - send message 2 on first press, first message on second press
#define CC_RANGE 4 // For use with expression pedal
#define CC_STEP 5 // Step through the values from minimum to maximum
#define CC_UPDOWN 6 // Press and hold to increase/decrease. Press shortly to change direction

// Looper types
#define LOOPER_SHOW_HIDE 1
#define LOOPER_PLAY_STOP 2
#define LOOPER_REC_OVERDUB 3
#define LOOPER_UNDO_REDO 4
#define LOOPER_HALF_SPEED 5
#define LOOPER_REVERSE 6
#define LOOPER_PLAY_ONCE 7
#define LOOPER_PRE_POST 8
#define LOOPER_REC_PLAY_OVERDUB 9 // KPA looper option
#define LOOPER_STOP_ERASE 10 // KPA looper option
#define LOOPER_PLAY 11
#define LOOPER_STOP 12
#define LOOPER_REC 13
#define LOOPER_OVERDUB 14

// Ports you can use
#define NUMBER_OF_MIDI_PORTS 5
#define USBMIDI_PORT 0x00
#define MIDI1_PORT 0x10
#define MIDI2_PORT 0x20
#define MIDI3_PORT 0x30
#define USBHMIDI_PORT 0x40
#define ALL_MIDI_PORTS 0xF0

// VG99 commands
#define FC300_CTL1 ASSIGN, VG99, SELECT, 0, 1
#define FC300_CTL2 ASSIGN, VG99, SELECT, 1, 2
#define FC300_CTL3 ASSIGN, VG99, SELECT, 2, 3
#define FC300_CTL4 ASSIGN, VG99, SELECT, 3, 4
#define FC300_CTL5 ASSIGN, VG99, SELECT, 4, 5
#define FC300_CTL6 ASSIGN, VG99, SELECT, 5, 6
#define FC300_CTL7 ASSIGN, VG99, SELECT, 6, 7
#define FC300_CTL8 ASSIGN, VG99, SELECT, 7, 8
#define FC300_EXP1 ASSIGN, VG99, SELECT, 8, 9
#define FC300_EXP_SW1 ASSIGN, VG99, SELECT, 9, 10
#define FC300_EXP2 ASSIGN, VG99, SELECT, 10, 11
#define FC300_EXP_SW2 ASSIGN, VG99, SELECT, 11, 12

// Menu special functions
#define MENU_SELECT 255
#define MENU_BACK 254
#define MENU_SET_VALUE 253
#define MENU_PREV 252
#define MENU_NEXT 251


// ********************************* Section 2: VController configuration for fixed command pages ********************************************
#ifdef CONFIG_VCONTROLLER

#define FIRST_FIXED_CMD_PAGE 201
#define FIRST_SELECTABLE_FIXED_CMD_PAGE 203
#define PAGE_MENU 201
#define PAGE_CURRENT_DIRECT_SELECT 202
#define PAGE_SELECT 203
#define PAGE_DEVICE_SELECT 204
#define PAGE_CURRENT_PATCH_BANK 205
#define PAGE_GR55_PATCH_BANK 206
#define PAGE_ZOOM_PATCH_BANK 207
#define PAGE_CURRENT_PARAMETER 208
#define PAGE_GP10_ASSIGNS 209
#define PAGE_GR55_ASSIGNS 210
#define PAGE_VG99_EDIT 211
#define PAGE_VG99_ASSIGNS 212
#define PAGE_VG99_ASSIGNS2 213
#define PAGE_M13_PARAMETER 214
#define PAGE_FULL_LOOPER 215
#define PAGE_HLX_PATCH_BANK 216
#define PAGE_HLX_PARAMETER 217
#define PAGE_SNAPSCENE_LOOPER 218
#define PAGE_KTN_PATCH_BANK 219
#define PAGE_KTN_EDIT 220
#define PAGE_KTN_FX 221
#define PAGE_KPA_RIG_SELECT 222
#define PAGE_KPA_LOOPER 223
#define PAGE_CURRENT_ASSIGN 224
#define PAGE_SY1000_PATCH_BANK 225
#define PAGE_SY1000_ASSIGNS 226
#define PAGE_SY1000_SCENES 227
#define PAGE_MG300_PATCH_BANK 228
#define LAST_FIXED_CMD_PAGE 227

#define DEFAULT_PAGE PAGE_SELECT // The page that gets selected when a valid page number is unknown

// Default pages for devices
#define FAS_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK
#define FAS_DEFAULT_PAGE2 PAGE_CURRENT_PARAMETER
#define FAS_DEFAULT_PAGE3 PAGE_SNAPSCENE_LOOPER
#define FAS_DEFAULT_PAGE4 0

#define GP10_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK
#define GP10_DEFAULT_PAGE2 PAGE_CURRENT_PARAMETER
#define GP10_DEFAULT_PAGE3 PAGE_GP10_ASSIGNS
#define GP10_DEFAULT_PAGE4 0

#define GR55_DEFAULT_PAGE1 PAGE_GR55_PATCH_BANK
#define GR55_DEFAULT_PAGE2 PAGE_CURRENT_PARAMETER
#define GR55_DEFAULT_PAGE3 PAGE_GR55_ASSIGNS
#define GR55_DEFAULT_PAGE4 0

#define HLX_DEFAULT_PAGE1 PAGE_HLX_PATCH_BANK
#define HLX_DEFAULT_PAGE2 PAGE_HLX_PARAMETER
#define HLX_DEFAULT_PAGE3 PAGE_SNAPSCENE_LOOPER
#define HLX_DEFAULT_PAGE4 0

#define KPA_DEFAULT_PAGE1 PAGE_KPA_RIG_SELECT
#define KPA_DEFAULT_PAGE2 PAGE_CURRENT_PARAMETER
#define KPA_DEFAULT_PAGE3 PAGE_KPA_LOOPER
#define KPA_DEFAULT_PAGE4 0

#define KTN_DEFAULT_PAGE1 PAGE_KTN_PATCH_BANK
#define KTN_DEFAULT_PAGE2 PAGE_KTN_FX
#define KTN_DEFAULT_PAGE3 0
#define KTN_DEFAULT_PAGE4 0

#define M13_DEFAULT_PAGE1 PAGE_M13_PARAMETER
#define M13_DEFAULT_PAGE2 PAGE_FULL_LOOPER
#define M13_DEFAULT_PAGE3 0
#define M13_DEFAULT_PAGE4 0

#define VG99_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK;
#define VG99_DEFAULT_PAGE2 PAGE_VG99_EDIT;
#define VG99_DEFAULT_PAGE3 PAGE_VG99_ASSIGNS;
#define VG99_DEFAULT_PAGE4 0;

#define ZG3_DEFAULT_PAGE1 PAGE_ZOOM_PATCH_BANK
#define ZG3_DEFAULT_PAGE2 0
#define ZG3_DEFAULT_PAGE3 0
#define ZG3_DEFAULT_PAGE4 0

#define ZMS70_DEFAULT_PAGE1 PAGE_ZOOM_PATCH_BANK
#define ZMS70_DEFAULT_PAGE2 0
#define ZMS70_DEFAULT_PAGE3 0
#define ZMS70_DEFAULT_PAGE4 0

#define SVL_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK
#define SVL_DEFAULT_PAGE2 PAGE_CURRENT_PARAMETER
#define SVL_DEFAULT_PAGE3 0
#define SVL_DEFAULT_PAGE4 0

#define SY1000_DEFAULT_PAGE1 PAGE_SY1000_PATCH_BANK
#define SY1000_DEFAULT_PAGE2 PAGE_SY1000_SCENES
#define SY1000_DEFAULT_PAGE3 PAGE_SY1000_ASSIGNS
#define SY1000_DEFAULT_PAGE4 0

#define GM2_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK
#define GM2_DEFAULT_PAGE2 PAGE_CURRENT_PARAMETER
#define GM2_DEFAULT_PAGE3 0
#define GM2_DEFAULT_PAGE4 0

#define MG300_DEFAULT_PAGE1 PAGE_MG300_PATCH_BANK
#define MG300_DEFAULT_PAGE2 PAGE_CURRENT_PARAMETER
#define MG300_DEFAULT_PAGE3 0
#define MG300_DEFAULT_PAGE4 0

const PROGMEM Cmd_struct Fixed_commands[] = {
  // ******************************* PAGE 201: MENU *************************************************
  {PAGE_MENU, LABEL, 'M', 'E', 'N', 'U', ' ', ' ', ' ', ' ' },
  {PAGE_MENU, 1, MENU, COMMON, 9}, // ** Switch 01 **
  {PAGE_MENU, 2, MENU, COMMON, 10}, // ** Switch 02 **
  {PAGE_MENU, 3, MENU, COMMON, 11}, // ** Switch 03 **
  {PAGE_MENU, 4, MENU, COMMON, 12}, // ** Switch 04 **
  {PAGE_MENU, 5, MENU, COMMON, 5}, // ** Switch 05 **
  {PAGE_MENU, 6, MENU, COMMON, 6}, // ** Switch 06 **
  {PAGE_MENU, 7, MENU, COMMON, 7}, // ** Switch 07 **
  {PAGE_MENU, 8, MENU, COMMON, 8}, // ** Switch 08 **
  {PAGE_MENU, 9, MENU, COMMON, 1}, // ** Switch 09 **
  {PAGE_MENU, 10, MENU, COMMON, 2}, // ** Switch 10 **
  {PAGE_MENU, 11, MENU, COMMON, 3}, // ** Switch 11 **
  {PAGE_MENU, 12, MENU, COMMON, 4}, // ** Switch 12 **
  {PAGE_MENU, 13, MENU, COMMON, 13}, // ** Switch 13 **
  {PAGE_MENU, 14, MENU, COMMON, 14}, // ** Switch 14 **
  {PAGE_MENU, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_MENU, 16, NOTHING, COMMON}, // ** Switch 16 **
  {PAGE_MENU, 17, MASTER_EXP_PEDAL, CURRENT, 0}, // External switch 1 or expr pedal 1
  {PAGE_MENU, 18, NOTHING, COMMON}, // External switch 2
  {PAGE_MENU, 19, MASTER_EXP_PEDAL, CURRENT, 0}, // External switch 3 or expr pedal 2
  {PAGE_MENU, 20, NOTHING, COMMON}, // External switch 4
  {PAGE_MENU, 21, MASTER_EXP_PEDAL, CURRENT, 0}, // External switch 5 or expr pedal 3
  {PAGE_MENU, 22, NOTHING, COMMON}, // External switch 6
  {PAGE_MENU, 23, MASTER_EXP_PEDAL, CURRENT, 0}, // External switch 7 or expr pedal 4
  {PAGE_MENU, 24, NOTHING, COMMON}, // External switch 8

  // ******************************* PAGE 202: Current device Direct Select *************************************************
  {PAGE_CURRENT_DIRECT_SELECT, LABEL, 'D', 'I', 'R', 'E', 'C', 'T', ' ', 'S' },
  {PAGE_CURRENT_DIRECT_SELECT, LABEL, 'E', 'L', 'E', 'C', 'T', ' ', ' ', ' ' },
  {PAGE_CURRENT_DIRECT_SELECT, 1, DIRECT_SELECT, CURRENT, 1, 10}, // ** Switch 01 **
  {PAGE_CURRENT_DIRECT_SELECT, 2, DIRECT_SELECT, CURRENT, 2, 10}, // ** Switch 02 **
  {PAGE_CURRENT_DIRECT_SELECT, 3, DIRECT_SELECT, CURRENT, 3, 10}, // ** Switch 03 **
  {PAGE_CURRENT_DIRECT_SELECT, 4, DIRECT_SELECT, CURRENT, 4, 10}, // ** Switch 04 **
  {PAGE_CURRENT_DIRECT_SELECT, 5, DIRECT_SELECT, CURRENT, 5, 10}, // ** Switch 05 **
  {PAGE_CURRENT_DIRECT_SELECT, 6, DIRECT_SELECT, CURRENT, 6, 10}, // ** Switch 06 **
  {PAGE_CURRENT_DIRECT_SELECT, 7, DIRECT_SELECT, CURRENT, 7, 10}, // ** Switch 07 **
  {PAGE_CURRENT_DIRECT_SELECT, 8, DIRECT_SELECT, CURRENT, 8, 10}, // ** Switch 08 **
  {PAGE_CURRENT_DIRECT_SELECT, 9, DIRECT_SELECT, CURRENT, 9, 10}, // ** Switch 09 **
  {PAGE_CURRENT_DIRECT_SELECT, 10, DIRECT_SELECT, CURRENT, 0, 10}, // ** Switch 10 **
  {PAGE_CURRENT_DIRECT_SELECT, 11, NOTHING, COMMON}, // ** Switch 11 **
  //{PAGE_CURRENT_DIRECT_SELECT, 12, NOTHING, COMMON}, // ** Switch 12 **
  {PAGE_CURRENT_DIRECT_SELECT, 13, PATCH, CURRENT, BANKDOWN, 100}, // ** Switch 13 **
  {PAGE_CURRENT_DIRECT_SELECT, 14, PATCH, CURRENT, BANKUP, 100}, // ** Switch 14 **
  //{PAGE_CURRENT_DIRECT_SELECT, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_CURRENT_DIRECT_SELECT, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

  // ******************************* PAGE 203: PAGE USER SELECT *************************************************
  {PAGE_SELECT, LABEL, 'S', 'E', 'L', 'E', 'C', 'T', ' ', 'P' },
  {PAGE_SELECT, LABEL, 'A', 'G', 'E', ' ', ' ', ' ', ' ', ' ' },
  {PAGE_SELECT, 1, PAGE, COMMON, BANKSELECT, 1, 10}, // ** Switch 01 **
  {PAGE_SELECT, 2, PAGE, COMMON, BANKSELECT, 2, 10}, // ** Switch 02 **
  {PAGE_SELECT, 3, PAGE, COMMON, BANKSELECT, 3, 10}, // ** Switch 03 **
  {PAGE_SELECT, 4, PAGE, COMMON, BANKSELECT, 4, 10}, // ** Switch 04 **
  {PAGE_SELECT, 5, PAGE, COMMON, BANKSELECT, 5, 10}, // ** Switch 05 **
  {PAGE_SELECT, 6, PAGE, COMMON, BANKSELECT, 6, 10}, // ** Switch 06 **
  {PAGE_SELECT, 7, PAGE, COMMON, BANKSELECT, 7, 10}, // ** Switch 07 **
  {PAGE_SELECT, 8, PAGE, COMMON, BANKSELECT, 8, 10}, // ** Switch 08 **
  {PAGE_SELECT, 9, PAGE, COMMON, BANKSELECT, 9, 10}, // ** Switch 09 **
  {PAGE_SELECT, 10, PAGE, COMMON, BANKSELECT, 10, 10}, // ** Switch 10 **
  {PAGE_SELECT, 11, PAGE, COMMON, SELECT, PAGE_DEVICE_SELECT}, // ** Switch 11 **
  {PAGE_SELECT, 12, PAGE, COMMON, SELECT, PAGE_MENU}, // ** Switch 12 **
  {PAGE_SELECT, 13, PAGE, COMMON, BANKDOWN, 10}, // ** Switch 13 **
  {PAGE_SELECT, 14, PAGE, COMMON, BANKUP, 10}, // ** Switch 14 **
  //{PAGE_SELECT, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_SELECT, 16, NOTHING, COMMON}, // ** Switch 16 **

  // ******************************* PAGE 204: DEVICE PAGE SELECT *************************************************
  {PAGE_DEVICE_SELECT, LABEL, 'S', 'E', 'L', ' ', 'D', 'E', 'V', 'I' },
  {PAGE_DEVICE_SELECT, LABEL, 'C', 'E', ' ', 'P', 'A', 'G', 'E', ' ' },
  {PAGE_DEVICE_SELECT, 1, OPEN_NEXT_PAGE_OF_DEVICE, GP10}, // ** Switch 01 **
  {PAGE_DEVICE_SELECT, 2, OPEN_NEXT_PAGE_OF_DEVICE, GR55}, // ** Switch 02 **
  {PAGE_DEVICE_SELECT, 3, OPEN_NEXT_PAGE_OF_DEVICE, VG99}, // ** Switch 03 **
  {PAGE_DEVICE_SELECT, 4, OPEN_NEXT_PAGE_OF_DEVICE, ZG3}, // ** Switch 04 **
  {PAGE_DEVICE_SELECT, 5, OPEN_NEXT_PAGE_OF_DEVICE, ZMS70}, // ** Switch 05 **
  {PAGE_DEVICE_SELECT, 6, OPEN_NEXT_PAGE_OF_DEVICE, M13}, // ** Switch 06 **
  {PAGE_DEVICE_SELECT, 7, OPEN_NEXT_PAGE_OF_DEVICE, HLX}, // ** Switch 07 **
  {PAGE_DEVICE_SELECT, 8, OPEN_NEXT_PAGE_OF_DEVICE, AXEFX}, // ** Switch 08 **
  {PAGE_DEVICE_SELECT, 9, OPEN_NEXT_PAGE_OF_DEVICE, KTN}, // ** Switch 09 **
  {PAGE_DEVICE_SELECT, 10, OPEN_NEXT_PAGE_OF_DEVICE, KPA}, // ** Switch 10 **
  {PAGE_DEVICE_SELECT, 11, PAGE, COMMON, SELECT, PAGE_SELECT}, // ** Switch 11 **
  {PAGE_DEVICE_SELECT, 12, PAGE, COMMON, SELECT, PAGE_MENU}, // ** Switch 12 **
  {PAGE_DEVICE_SELECT, 13, NOTHING, COMMON}, // ** Switch 13 **
  {PAGE_DEVICE_SELECT, 14, NOTHING, COMMON}, // ** Switch 14 **
  //{PAGE_DEVICE_SELECT, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_DEVICE_SELECT, 16, NOTHING, COMMON}, // ** Switch 16 **

  // ******************************* PAGE 205: Current_patch_bank (10 buttons per page) *************************************************
  {PAGE_CURRENT_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', 'B', 'A' },
  {PAGE_CURRENT_PATCH_BANK, LABEL, 'N', 'K', ' ', ' ', ' ', ' ', ' ', ' ' },
  {PAGE_CURRENT_PATCH_BANK, 1, PATCH, CURRENT, BANKSELECT, 1, 10}, // ** Switch 01 **
  {PAGE_CURRENT_PATCH_BANK, 2, PATCH, CURRENT, BANKSELECT, 2, 10}, // ** Switch 02 **
  {PAGE_CURRENT_PATCH_BANK, 3, PATCH, CURRENT, BANKSELECT, 3, 10}, // ** Switch 03 **
  {PAGE_CURRENT_PATCH_BANK, 4, PATCH, CURRENT, BANKSELECT, 4, 10}, // ** Switch 04 **
  {PAGE_CURRENT_PATCH_BANK, 5, PATCH, CURRENT, BANKSELECT, 5, 10}, // ** Switch 05 **
  {PAGE_CURRENT_PATCH_BANK, 6, PATCH, CURRENT, BANKSELECT, 6, 10}, // ** Switch 06 **
  {PAGE_CURRENT_PATCH_BANK, 7, PATCH, CURRENT, BANKSELECT, 7, 10}, // ** Switch 07 **
  {PAGE_CURRENT_PATCH_BANK, 8, PATCH, CURRENT, BANKSELECT, 8, 10}, // ** Switch 08 **
  {PAGE_CURRENT_PATCH_BANK, 9, PATCH, CURRENT, BANKSELECT, 9, 10}, // ** Switch 09 **
  {PAGE_CURRENT_PATCH_BANK, 10, PATCH, CURRENT, BANKSELECT, 10, 10}, // ** Switch 10 **
  //{PAGE_CURRENT_PATCH_BANK, 11, PARAMETER, CURRENT, 1, TOGGLE, 1, 0}, // ** Switch 11 **
  //{PAGE_CURRENT_PATCH_BANK, 11, PARAMETER, CURRENT, 2, TOGGLE, 1, 0}, // ** Switch 11 **
  //{PAGE_CURRENT_PATCH_BANK, 11, PARAMETER, CURRENT, 3, TOGGLE, 1, 0}, // ** Switch 11 **
  //{PAGE_CURRENT_PATCH_BANK, 11, PARAMETER, CURRENT, 4, TOGGLE, 1, 0}, // ** Switch 11 **
  //{PAGE_CURRENT_PATCH_BANK, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_CURRENT_PATCH_BANK, 13, PATCH, CURRENT, BANKDOWN, 10}, // ** Switch 13 **
  {PAGE_CURRENT_PATCH_BANK, 13 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
  {PAGE_CURRENT_PATCH_BANK, 14, PATCH, CURRENT, BANKUP, 10}, // ** Switch 14 **
  {PAGE_CURRENT_PATCH_BANK, 14 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_CURRENT_DIRECT_SELECT}, // ** Switch 14 **
  //{PAGE_CURRENT_PATCH_BANK, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_CURRENT_PATCH_BANK, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

  // ******************************* PAGE 206: GR55 select *************************************************
  {PAGE_GR55_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', 'B', 'A' },
  {PAGE_GR55_PATCH_BANK, LABEL, 'N', 'K', ' ', 'G', 'R', ' ', ' ', ' ' },
  {PAGE_GR55_PATCH_BANK, 1, PATCH, GR55, BANKSELECT, 1, 9}, // ** Switch 01 **
  {PAGE_GR55_PATCH_BANK, 2, PATCH, GR55, BANKSELECT, 2, 9}, // ** Switch 02 **
  {PAGE_GR55_PATCH_BANK, 3, PATCH, GR55, BANKSELECT, 3, 9}, // ** Switch 03 **
  {PAGE_GR55_PATCH_BANK, 4, PARAMETER, GR55, 33, TOGGLE, 1, 0}, // ** Switch 04 **
  {PAGE_GR55_PATCH_BANK, 5, PATCH, GR55, BANKSELECT, 4, 9}, // ** Switch 05 **
  {PAGE_GR55_PATCH_BANK, 6, PATCH, GR55, BANKSELECT, 5, 9}, // ** Switch 06 **
  {PAGE_GR55_PATCH_BANK, 7, PATCH, GR55, BANKSELECT, 6, 9}, // ** Switch 07 **
  {PAGE_GR55_PATCH_BANK, 8, NOTHING, COMMON}, // ** Switch 08 **
  {PAGE_GR55_PATCH_BANK, 9, PATCH, GR55, BANKSELECT, 7, 9}, // ** Switch 09 **
  {PAGE_GR55_PATCH_BANK, 10, PATCH, GR55, BANKSELECT, 8, 9}, // ** Switch 10 **
  {PAGE_GR55_PATCH_BANK, 11, PATCH, GR55, BANKSELECT, 9, 9}, // ** Switch 11 **
  //{PAGE_GR55_PATCH_BANK, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_GR55_PATCH_BANK, 13, PATCH, GR55, BANKDOWN, 9}, // ** Switch 13 **
  {PAGE_GR55_PATCH_BANK, 13 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
  {PAGE_GR55_PATCH_BANK, 14, PATCH, GR55, BANKUP, 9}, // ** Switch 14 **
  {PAGE_GR55_PATCH_BANK, 14 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
  //{PAGE_GR55_PATCH_BANK, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_GR55_PATCH_BANK, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

  // ******************************* PAGE 207: Zoom patch bank *************************************************
  {PAGE_ZOOM_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', '+', ' ' },
  {PAGE_ZOOM_PATCH_BANK, LABEL, 'F', 'X', ' ', ' ', ' ', ' ', ' ', ' ' },
  {PAGE_ZOOM_PATCH_BANK, 1, PATCH, CURRENT, BANKSELECT, 1, 5}, // ** Switch 01 **
  {PAGE_ZOOM_PATCH_BANK, 2, PATCH, CURRENT, BANKSELECT, 2, 5}, // ** Switch 02 **
  {PAGE_ZOOM_PATCH_BANK, 3, PATCH, CURRENT, BANKSELECT, 3, 5}, // ** Switch 03 **
  {PAGE_ZOOM_PATCH_BANK, 4, PATCH, CURRENT, BANKSELECT, 4, 5}, // ** Switch 04 **
  {PAGE_ZOOM_PATCH_BANK, 5, PARAMETER, CURRENT, 0, TOGGLE, 1, 0}, // ** Switch 05 **
  {PAGE_ZOOM_PATCH_BANK, 6, PARAMETER, CURRENT, 1, TOGGLE, 1, 0}, // ** Switch 06 **
  {PAGE_ZOOM_PATCH_BANK, 7, PARAMETER, CURRENT, 2, TOGGLE, 1, 0}, // ** Switch 07 **
  {PAGE_ZOOM_PATCH_BANK, 8, PATCH, CURRENT, BANKSELECT, 5, 5}, // ** Switch 08 **
  {PAGE_ZOOM_PATCH_BANK, 9, PARAMETER, CURRENT, 3, TOGGLE, 1, 0}, // ** Switch 09 **
  {PAGE_ZOOM_PATCH_BANK, 10, PARAMETER, CURRENT, 4, TOGGLE, 1, 0}, // ** Switch 10 **
  {PAGE_ZOOM_PATCH_BANK, 11, PARAMETER, CURRENT, 5, TOGGLE, 1, 0}, // ** Switch 11 **
  //{PAGE_ZOOM_PATCH_BANK, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_ZOOM_PATCH_BANK, 13, PATCH, CURRENT, BANKDOWN, 5}, // ** Switch 13 **
  {PAGE_ZOOM_PATCH_BANK, 13 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
  {PAGE_ZOOM_PATCH_BANK, 14, PATCH, CURRENT, BANKUP, 5}, // ** Switch 14 **
  {PAGE_ZOOM_PATCH_BANK, 14 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
  //{PAGE_ZOOM_PATCH_BANK, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_ZOOM_PATCH_BANK, 16, NOTHING, COMMON}, // ** Switch 16 **

  // ******************************* PAGE 208: Parameters current device *************************************************
  {PAGE_CURRENT_PARAMETER, LABEL, 'P', 'A', 'R', ' ', 'B', 'A', 'N', 'K' },
  {PAGE_CURRENT_PARAMETER, 1, PAR_BANK, CURRENT, 1, 10}, // ** Switch 01 **
  {PAGE_CURRENT_PARAMETER, 2, PAR_BANK, CURRENT, 2, 10}, // ** Switch 02 **
  {PAGE_CURRENT_PARAMETER, 3, PAR_BANK, CURRENT, 3, 10}, // ** Switch 03 **
  {PAGE_CURRENT_PARAMETER, 4, PAR_BANK, CURRENT, 4, 10}, // ** Switch 04 **
  {PAGE_CURRENT_PARAMETER, 5, PAR_BANK, CURRENT, 5, 10}, // ** Switch 05 **
  {PAGE_CURRENT_PARAMETER, 6, PAR_BANK, CURRENT, 6, 10}, // ** Switch 06 **
  {PAGE_CURRENT_PARAMETER, 7, PAR_BANK, CURRENT, 7, 10}, // ** Switch 07 **
  {PAGE_CURRENT_PARAMETER, 8, PAR_BANK, CURRENT, 8, 10}, // ** Switch 08 **
  {PAGE_CURRENT_PARAMETER, 9, PAR_BANK, CURRENT, 9, 10}, // ** Switch 09 **
  {PAGE_CURRENT_PARAMETER, 10, PAR_BANK, CURRENT, 10, 10}, // ** Switch 10 **
  //{PAGE_CURRENT_PARAMETER, 11, NOTHING, COMMON}, // ** Switch 11 **
  //{PAGE_CURRENT_PARAMETER, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_CURRENT_PARAMETER, 13, PAR_BANK_DOWN, CURRENT, 10}, // ** Switch 13 **
  {PAGE_CURRENT_PARAMETER, 14, PAR_BANK_UP, CURRENT, 10}, // ** Switch 14 **
  //{PAGE_CURRENT_PARAMETER, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_CURRENT_PARAMETER, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

  // ******************************* PAGE 209: GP10 assign *************************************************
  {PAGE_GP10_ASSIGNS, LABEL, 'A', 'S', 'S', 'G', 'N', ' ', 'G', 'P' },
  {PAGE_GP10_ASSIGNS, LABEL, '1', '0', ' ', ' ', ' ', ' ', ' ', ' ' },
  {PAGE_GP10_ASSIGNS, 1, ASSIGN, GP10, SELECT, 0, 21}, // ** Switch 01 **
  {PAGE_GP10_ASSIGNS, 2, ASSIGN, GP10, SELECT, 1, 22}, // ** Switch 02 **
  {PAGE_GP10_ASSIGNS, 3, ASSIGN, GP10, SELECT, 2, 23}, // ** Switch 03 **
  {PAGE_GP10_ASSIGNS, 4, ASSIGN, GP10, SELECT, 3, 24}, // ** Switch 04 **
  {PAGE_GP10_ASSIGNS, 5, ASSIGN, GP10, SELECT, 4, 25}, // ** Switch 05 **
  {PAGE_GP10_ASSIGNS, 6, ASSIGN, GP10, SELECT, 5, 26}, // ** Switch 06 **
  {PAGE_GP10_ASSIGNS, 7, ASSIGN, GP10, SELECT, 6, 27}, // ** Switch 07 **
  {PAGE_GP10_ASSIGNS, 8, ASSIGN, GP10, SELECT, 7, 28}, // ** Switch 08 **
  {PAGE_GP10_ASSIGNS, 9, NOTHING, COMMON}, // ** Switch 09 **
  {PAGE_GP10_ASSIGNS, 10, NOTHING, COMMON}, // ** Switch 10 **
  //{PAGE_GP10_ASSIGNS, 11, NOTHING, COMMON}, // ** Switch 11 **
  //{PAGE_GP10_ASSIGNS, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_GP10_ASSIGNS, 13, PATCH, GP10, PREV, 9}, // ** Switch 13 **
  {PAGE_GP10_ASSIGNS, 14, PATCH, GP10, NEXT, 9}, // ** Switch 14 **
  //{PAGE_GP10_ASSIGNS, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_GP10_ASSIGNS, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

  // ******************************* PAGE 210: GR55 Assign *************************************************
  {PAGE_GR55_ASSIGNS, LABEL, 'A', 'S', 'S', 'G', 'N', ' ', 'G', 'R'},
  {PAGE_GR55_ASSIGNS, LABEL, '5', '5', ' ', ' ', ' ', ' ', ' ', ' '},
  {PAGE_GR55_ASSIGNS, 1, ASSIGN, GR55, SELECT, 5, 26}, // ** Switch 01 **
  {PAGE_GR55_ASSIGNS, 2, ASSIGN, GR55, SELECT, 6, 27}, // ** Switch 02 **
  {PAGE_GR55_ASSIGNS, 3, ASSIGN, GR55, SELECT, 7, 28}, // ** Switch 03 **
  {PAGE_GR55_ASSIGNS, 4, PARAMETER, GR55, 33, TOGGLE, 1, 0}, // ** Switch 04 **
  {PAGE_GR55_ASSIGNS, 5, PARAMETER, GR55, 0, TOGGLE, 1, 0}, // ** Switch 05 **
  {PAGE_GR55_ASSIGNS, 6, PARAMETER, GR55, 1, STEP, 0, 19, 1}, // ** Switch 08 **
  {PAGE_GR55_ASSIGNS, 7, PARAMETER, GR55, 2, TOGGLE, 1, 0}, // ** Switch 06 **
  {PAGE_GR55_ASSIGNS, 8, PARAMETER, GR55, 3, STEP, 0, 13, 1}, // ** Switch 07 **
  {PAGE_GR55_ASSIGNS, 9, PARAMETER, GR55, 15, TOGGLE, 1, 0}, // ** Switch 09 **
  {PAGE_GR55_ASSIGNS, 10, PARAMETER, GR55, 9, TOGGLE, 1, 0}, // ** Switch 10 **
  //{PAGE_GR55_ASSIGNS, 11, NOTHING, COMMON}, // ** Switch 11 **
  //{PAGE_GR55_ASSIGNS, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_GR55_ASSIGNS, 13, PATCH, GR55, PREV}, // ** Switch 13 **
  {PAGE_GR55_ASSIGNS, 14, PATCH, GR55, NEXT}, // ** Switch 14 **
  //{PAGE_GR55_ASSIGNS, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_GR55_ASSIGNS, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

  // ******************************* PAGE 211: VG99 Edit *************************************************
  {PAGE_VG99_EDIT, LABEL, 'V', 'G', '9', '9', ' ', 'E', 'D', 'I'},
  {PAGE_VG99_EDIT, LABEL, 'T', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
  {PAGE_VG99_EDIT, 1, PAR_BANK_CATEGORY, VG99, 1}, // ** Switch 01 **
  {PAGE_VG99_EDIT, 2, PAR_BANK_CATEGORY, VG99, 2}, // ** Switch 02 **
  {PAGE_VG99_EDIT, 3, PAR_BANK_CATEGORY, VG99, 3}, // ** Switch 03 **
  {PAGE_VG99_EDIT, 4, PAR_BANK_CATEGORY, VG99, 4}, // ** Switch 04 **
  {PAGE_VG99_EDIT, 5, PAR_BANK_CATEGORY, VG99, 5}, // ** Switch 05 **
  {PAGE_VG99_EDIT, 6, PAR_BANK_CATEGORY, VG99, 6}, // ** Switch 06 **
  {PAGE_VG99_EDIT, 7, PAR_BANK_CATEGORY, VG99, 7}, // ** Switch 07 **
  {PAGE_VG99_EDIT, 8, PAR_BANK_CATEGORY, VG99, 8}, // ** Switch 08 **
  {PAGE_VG99_EDIT, 9, PAR_BANK_CATEGORY, VG99, 9}, // ** Switch 09 **
  {PAGE_VG99_EDIT, 10, PAR_BANK_CATEGORY, VG99, 10}, // ** Switch 10 **
  {PAGE_VG99_EDIT, 11, PAR_BANK_CATEGORY, VG99, 11 }, // ** Switch 11 **
  {PAGE_VG99_EDIT, 12, PAR_BANK_CATEGORY, VG99, 12}, // ** Switch 12 **
  {PAGE_VG99_EDIT, 13, PATCH, VG99, PREV}, // ** Switch 10 **
  {PAGE_VG99_EDIT, 14, PATCH, VG99, NEXT}, // ** Switch 11 **
  //{PAGE_VG99_EDIT, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 12 **
  {PAGE_VG99_EDIT, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 12 **

  // ******************************* PAGE 212: VG99 Assign *************************************************
  {PAGE_VG99_ASSIGNS, LABEL, 'F', 'C', '3', '0', '0', ' ', 'A', 'S'},
  {PAGE_VG99_ASSIGNS, LABEL, 'G', 'N', ' ', ' ', ' ', ' ', ' ', ' '},
  {PAGE_VG99_ASSIGNS, 1, FC300_CTL1}, // ** Switch 01 **
  {PAGE_VG99_ASSIGNS, 2, FC300_CTL2}, // ** Switch 02 **
  {PAGE_VG99_ASSIGNS, 3, FC300_CTL3}, // ** Switch 03 **
  {PAGE_VG99_ASSIGNS, 4, FC300_CTL4}, // ** Switch 04 **
  {PAGE_VG99_ASSIGNS, 5, FC300_CTL5}, // ** Switch 05 **
  {PAGE_VG99_ASSIGNS, 6, FC300_CTL6}, // ** Switch 06 **
  {PAGE_VG99_ASSIGNS, 7, FC300_CTL7}, // ** Switch 07 **
  {PAGE_VG99_ASSIGNS, 8, FC300_CTL8}, // ** Switch 08 **
  {PAGE_VG99_ASSIGNS, 9, FC300_EXP_SW1}, // ** Switch 09 **
  {PAGE_VG99_ASSIGNS, 10, FC300_EXP_SW2}, // ** Switch 10 **
  {PAGE_VG99_ASSIGNS, 11, PAGE, COMMON, SELECT, PAGE_VG99_ASSIGNS2 }, // ** Switch 11 **
  //{PAGE_VG99_ASSIGNS, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_VG99_ASSIGNS, 13, PATCH, VG99, PREV}, // ** Switch 10 **
  {PAGE_VG99_ASSIGNS, 14, PATCH, VG99, NEXT}, // ** Switch 11 **
  //{PAGE_VG99_ASSIGNS, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 12 **
  {PAGE_VG99_ASSIGNS, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 12 **

  // ******************************* PAGE 213: VG99 Assign2 *************************************************
  {PAGE_VG99_ASSIGNS2, LABEL, 'M', 'O', 'R', 'E', ' ', 'A', 'S', 'G'},
  {PAGE_VG99_ASSIGNS2, 1, ASSIGN, VG99, SELECT, 15, 255}, // ** Switch 01 **
  {PAGE_VG99_ASSIGNS2, 2, ASSIGN, VG99, SELECT, 16, 255}, // ** Switch 02 **
  {PAGE_VG99_ASSIGNS2, 3, ASSIGN, VG99, SELECT, 17, 255}, // ** Switch 03 **
  {PAGE_VG99_ASSIGNS2, 4, ASSIGN, VG99, SELECT, 18, 255}, // ** Switch 04 **
  {PAGE_VG99_ASSIGNS2, 5, ASSIGN, VG99, SELECT, 19, 255}, // ** Switch 05 **
  {PAGE_VG99_ASSIGNS2, 6, ASSIGN, VG99, SELECT, 20, 255}, // ** Switch 06 **
  {PAGE_VG99_ASSIGNS2, 7, ASSIGN, VG99, SELECT, 21, 255}, // ** Switch 07 **
  {PAGE_VG99_ASSIGNS2, 8, ASSIGN, VG99, SELECT, 22, 255}, // ** Switch 08 **
  {PAGE_VG99_ASSIGNS2, 9, ASSIGN, VG99, SELECT, 12, 255}, // ** Switch 09 **
  {PAGE_VG99_ASSIGNS2, 10, ASSIGN, VG99, SELECT, 13, 255}, // ** Switch 10 **
  {PAGE_VG99_ASSIGNS2, 11, ASSIGN, VG99, SELECT, 14, 255}, // ** Switch 11 **
  // ** Switch 12 **
  {PAGE_VG99_ASSIGNS2, 13, PATCH, VG99, PREV}, // ** Switch 10 **
  {PAGE_VG99_ASSIGNS2, 14, PATCH, VG99, NEXT}, // ** Switch 11 **
  //{PAGE_VG99_ASSIGNS2, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 12 **
  {PAGE_VG99_ASSIGNS2, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 12 **

  // ******************************* PAGE 214: Line 6 M13 effects *************************************************
  {PAGE_M13_PARAMETER, LABEL, 'L', 'I', 'N', 'E', '6', ' ', 'M', '1' },
  {PAGE_M13_PARAMETER, LABEL, '3', ' ', 'F', 'X', ' ', ' ', ' ', ' ' },
  {PAGE_M13_PARAMETER, 1, PARAMETER, M13, 0, TOGGLE, 1, 0}, // ** Switch 01 **
  {PAGE_M13_PARAMETER, 2, PARAMETER, M13, 3, TOGGLE, 1, 0}, // ** Switch 02 **
  {PAGE_M13_PARAMETER, 3, PARAMETER, M13, 6, TOGGLE, 1, 0}, // ** Switch 03 **
  {PAGE_M13_PARAMETER, 4, PARAMETER, M13, 9, TOGGLE, 1, 0}, // ** Switch 04 **
  {PAGE_M13_PARAMETER, 5, PARAMETER, M13, 1, TOGGLE, 1, 0}, // ** Switch 05 **
  {PAGE_M13_PARAMETER, 6, PARAMETER, M13, 4, TOGGLE, 1, 0}, // ** Switch 06 **
  {PAGE_M13_PARAMETER, 7, PARAMETER, M13, 7, TOGGLE, 1, 0}, // ** Switch 07 **
  {PAGE_M13_PARAMETER, 8, PARAMETER, M13, 10, TOGGLE, 1, 0}, // ** Switch 08 **
  {PAGE_M13_PARAMETER, 9, PARAMETER, M13, 2, TOGGLE, 1, 0}, // ** Switch 09 **
  {PAGE_M13_PARAMETER, 10, PARAMETER, M13, 5, TOGGLE, 1, 0}, // ** Switch 10 **
  {PAGE_M13_PARAMETER, 11, PARAMETER, M13, 8, TOGGLE, 1, 0}, // ** Switch 11 **
  {PAGE_M13_PARAMETER, 12, PARAMETER, M13, 11, TOGGLE, 1, 0}, // ** Switch 12 **
  {PAGE_M13_PARAMETER, 13, PATCH, M13, PREV}, // ** Switch 13 **
  {PAGE_M13_PARAMETER, 14, PATCH, M13, NEXT}, // ** Switch 14 **
  //{PAGE_M13_PARAMETER, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_M13_PARAMETER, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

  // ******************************* PAGE 215: Looper control *************************************************
  {PAGE_FULL_LOOPER, LABEL, 'L', 'O', 'O', 'P', 'E', 'R', ' ', 'C' },
  {PAGE_FULL_LOOPER, LABEL, 'O', 'N', 'T', 'R', 'O', 'L', ' ', ' ' },
  {PAGE_FULL_LOOPER, 1, LOOPER, CURRENT, LOOPER_REC_OVERDUB}, // ** Switch 01 **
  {PAGE_FULL_LOOPER, 2, LOOPER, CURRENT, LOOPER_PLAY_STOP}, // ** Switch 02 **
  {PAGE_FULL_LOOPER, 3, LOOPER, CURRENT, LOOPER_HALF_SPEED}, // ** Switch 03 **
  {PAGE_FULL_LOOPER, 4, LOOPER, CURRENT, LOOPER_REVERSE}, // ** Switch 04 **
  {PAGE_FULL_LOOPER, 5, LOOPER, CURRENT, LOOPER_UNDO_REDO}, // ** Switch 05 **
  {PAGE_FULL_LOOPER, 6, LOOPER, CURRENT, LOOPER_PLAY_ONCE}, // ** Switch 06 **
  {PAGE_FULL_LOOPER, 7, LOOPER, CURRENT, LOOPER_PRE_POST}, // ** Switch 07 **
  {PAGE_FULL_LOOPER, 8, LOOPER, CURRENT, LOOPER_SHOW_HIDE}, // ** Switch 08 **
  //{PAGE_FULL_LOOPER, 9, PATCH, CURRENT, BANKSELECT, 1, 3}, // ** Switch 09 **
  //{PAGE_FULL_LOOPER, 10, PATCH, CURRENT, BANKSELECT, 2, 3}, // ** Switch 10 **
  //{PAGE_FULL_LOOPER, 11, PATCH, CURRENT, BANKSELECT, 3, 3}, // ** Switch 11 **
  {PAGE_FULL_LOOPER, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_FULL_LOOPER, 13, PATCH, CURRENT, 3, PREV}, // ** Switch 13 **
  {PAGE_FULL_LOOPER, 14, PATCH, CURRENT, 3, NEXT}, // ** Switch 14 **
  //{PAGE_FULL_LOOPER, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_FULL_LOOPER, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

  // ******************************* PAGE 216: Helix_patch_bank (8 buttons per page) *************************************************
  {PAGE_HLX_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', 'B', 'A' },
  {PAGE_HLX_PATCH_BANK, LABEL, 'N', 'K', ' ', 'H', 'E', 'L', 'I', 'X' },
  {PAGE_HLX_PATCH_BANK, 1, PATCH, HLX, BANKSELECT, 1, 8}, // ** Switch 01 **
  {PAGE_HLX_PATCH_BANK, 2, PATCH, HLX, BANKSELECT, 2, 8}, // ** Switch 02 **
  {PAGE_HLX_PATCH_BANK, 3, PATCH, HLX, BANKSELECT, 3, 8}, // ** Switch 03 **
  {PAGE_HLX_PATCH_BANK, 4, PATCH, HLX, BANKSELECT, 4, 8}, // ** Switch 04 **
  {PAGE_HLX_PATCH_BANK, 5, PATCH, HLX, BANKSELECT, 5, 8}, // ** Switch 05 **
  {PAGE_HLX_PATCH_BANK, 6, PATCH, HLX, BANKSELECT, 6, 8}, // ** Switch 06 **
  {PAGE_HLX_PATCH_BANK, 7, PATCH, HLX, BANKSELECT, 7, 8}, // ** Switch 07 **
  {PAGE_HLX_PATCH_BANK, 8, PATCH, HLX, BANKSELECT, 8, 8}, // ** Switch 08 **
  {PAGE_HLX_PATCH_BANK, 9, PARAMETER, HLX, 14, STEP, 0, 6, 1}, // ** Switch 09 **
  {PAGE_HLX_PATCH_BANK, 10, PAGE, COMMON, SELECT, PAGE_FULL_LOOPER}, // ** Switch 10 **
  //{PAGE_HLX_PATCH_BANK, 11, PARAMETER, HLX, 1, TOGGLE, 1, 0}, // ** Switch 11 **
  //{PAGE_HLX_PATCH_BANK, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_HLX_PATCH_BANK, 13, PATCH, HLX, BANKDOWN, 8}, // ** Switch 13 **
  {PAGE_HLX_PATCH_BANK, 13 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
  {PAGE_HLX_PATCH_BANK, 14, PATCH, HLX, BANKUP, 8}, // ** Switch 14 **
  {PAGE_HLX_PATCH_BANK, 14 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
  //{PAGE_HLX_PATCH_BANK, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_HLX_PATCH_BANK, 16, OPEN_NEXT_PAGE_OF_DEVICE, HLX}, // ** Switch 16 **

  // ******************************* PAGE 217: Helix_parameter *************************************************
  {PAGE_HLX_PARAMETER, LABEL, 'P', 'A', 'R', 'A', 'M', 'E', 'T', 'E' },
  {PAGE_HLX_PARAMETER, LABEL, 'R', 'S', ' ', 'H', 'E', 'L', 'I', 'X' },
  {PAGE_HLX_PARAMETER, 1, PARAMETER, HLX, 6, MOMENTARY, 127, 0}, // ** Switch 01 **
  {PAGE_HLX_PARAMETER, 2, PARAMETER, HLX, 7, MOMENTARY, 127, 0}, // ** Switch 02 **
  {PAGE_HLX_PARAMETER, 3, PARAMETER, HLX, 8, MOMENTARY, 127, 0}, // ** Switch 03 **
  {PAGE_HLX_PARAMETER, 4, PARAMETER, HLX, 9, MOMENTARY, 127, 0}, // ** Switch 04 **
  {PAGE_HLX_PARAMETER, 5, PARAMETER, HLX, 1, MOMENTARY, 127, 0}, // ** Switch 05 **
  {PAGE_HLX_PARAMETER, 6, PARAMETER, HLX, 2, MOMENTARY, 127, 0}, // ** Switch 06 **
  {PAGE_HLX_PARAMETER, 7, PARAMETER, HLX, 3, MOMENTARY, 127, 0}, // ** Switch 07 **
  {PAGE_HLX_PARAMETER, 8, PARAMETER, HLX, 4, MOMENTARY, 127, 0}, // ** Switch 08 **
  {PAGE_HLX_PARAMETER, 9, LOOPER, HLX, LOOPER_REC_OVERDUB}, // ** Switch 09 **
  {PAGE_HLX_PARAMETER, 10, LOOPER, HLX, LOOPER_PLAY_STOP}, // ** Switch 10 **
  //{PAGE_HLX_PARAMETER, 11, PARAMETER, HLX, 10, MOMENTARY, 127, 0}, // ** Switch 11 **
  //{PAGE_HLX_PARAMETER, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_HLX_PARAMETER, 13, PATCH, HLX, PREV}, // ** Switch 13 **
  {PAGE_HLX_PARAMETER, 14, PATCH, HLX, NEXT}, // ** Switch 14 **
  //{PAGE_HLX_PARAMETER, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_HLX_PARAMETER, 16, OPEN_NEXT_PAGE_OF_DEVICE, HLX}, // ** Switch 16 **

  // ******************************* PAGE 218: Snapshots (Helix) /scenes (AxeFX) *************************************************
  {PAGE_SNAPSCENE_LOOPER, LABEL, 'S', 'N', 'A', 'P', 'S', 'C', 'E', 'N' },
  {PAGE_SNAPSCENE_LOOPER, LABEL, 'E', '/', 'L', 'O', 'O', 'P', 'E', 'R' },
  {PAGE_SNAPSCENE_LOOPER, 1, SNAPSCENE, CURRENT, 1, 0, 0}, // ** Switch 01 **
  {PAGE_SNAPSCENE_LOOPER, 2, SNAPSCENE, CURRENT, 2, 0, 0}, // ** Switch 02 **
  {PAGE_SNAPSCENE_LOOPER, 3, SNAPSCENE, CURRENT, 3, 0, 0}, // ** Switch 03 **
  {PAGE_SNAPSCENE_LOOPER, 4, SNAPSCENE, CURRENT, 4, 0, 0}, // ** Switch 04 **
  {PAGE_SNAPSCENE_LOOPER, 5, SNAPSCENE, CURRENT, 5, 0, 0}, // ** Switch 05 **
  {PAGE_SNAPSCENE_LOOPER, 6, SNAPSCENE, CURRENT, 6, 0, 0}, // ** Switch 06 **
  {PAGE_SNAPSCENE_LOOPER, 7, SNAPSCENE, CURRENT, 7, 0, 0}, // ** Switch 07 **
  {PAGE_SNAPSCENE_LOOPER, 8, SNAPSCENE, CURRENT, 8, 0, 0}, // ** Switch 08 **
  {PAGE_SNAPSCENE_LOOPER, 9, LOOPER, CURRENT, LOOPER_REC_OVERDUB}, // ** Switch 09 **
  {PAGE_SNAPSCENE_LOOPER, 10, LOOPER, CURRENT, LOOPER_PLAY_STOP}, // ** Switch 10 **
  //{PAGE_SNAPSCENE_LOOPER, 11, PARAMETER, CURRENT, 10, MOMENTARY, 127, 0}, // ** Switch 11 **
  //{PAGE_SNAPSCENE_LOOPER, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_SNAPSCENE_LOOPER, 13, PATCH, CURRENT, PREV}, // ** Switch 13 **
  {PAGE_SNAPSCENE_LOOPER, 14, PATCH, CURRENT, NEXT}, // ** Switch 14 **
  //{PAGE_SNAPSCENE_LOOPER, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_SNAPSCENE_LOOPER, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

  // ******************************* PAGE 219: KATANA_patch_bank (8 buttons per page) *************************************************
  {PAGE_KTN_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', 'B', 'A', 'N' },
  {PAGE_KTN_PATCH_BANK, LABEL, 'K', ' ', 'K', 'A', 'T', 'A', 'N', 'A' },
  {PAGE_KTN_PATCH_BANK, 1, PATCH, KTN, BANKSELECT, 2, 8}, // ** Switch 01 **
  {PAGE_KTN_PATCH_BANK, 2, PATCH, KTN, BANKSELECT, 3, 8}, // ** Switch 02 **
  {PAGE_KTN_PATCH_BANK, 3, PATCH, KTN, BANKSELECT, 4, 8}, // ** Switch 03 **
  {PAGE_KTN_PATCH_BANK, 4, PATCH, KTN, BANKSELECT, 5, 8}, // ** Switch 04 **
  {PAGE_KTN_PATCH_BANK, 5, PATCH, KTN, BANKSELECT, 6, 8}, // ** Switch 05 **
  {PAGE_KTN_PATCH_BANK, 6, PATCH, KTN, BANKSELECT, 7, 8}, // ** Switch 06 **
  {PAGE_KTN_PATCH_BANK, 7, PATCH, KTN, BANKSELECT, 8, 8}, // ** Switch 07 **
  {PAGE_KTN_PATCH_BANK, 8, PATCH, KTN, BANKSELECT, 9, 8}, // ** Switch 08 **
  {PAGE_KTN_PATCH_BANK, 9, PATCH, KTN, SELECT, 0, 0}, // ** Switch 09 **
  {PAGE_KTN_PATCH_BANK, 10, OPEN_PAGE_DEVICE, KTN, PAGE_KTN_EDIT}, // ** Switch 10 **
  //{PAGE_KTN_PATCH_BANK, 11, PARAMETER, KTN, 1, TOGGLE, 1, 0}, // ** Switch 11 **
  //{PAGE_KTN_PATCH_BANK, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_KTN_PATCH_BANK, 13, PATCH, KTN, BANKDOWN, 8}, // ** Switch 13 **
  {PAGE_KTN_PATCH_BANK, 13 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
  {PAGE_KTN_PATCH_BANK, 14, PATCH, KTN, BANKUP, 8}, // ** Switch 14 **
  {PAGE_KTN_PATCH_BANK, 14 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
  //{PAGE_KTN_PATCH_BANK, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_KTN_PATCH_BANK, 16, OPEN_NEXT_PAGE_OF_DEVICE, KTN}, // ** Switch 16 **

  // ******************************* PAGE 220: KATANA Edit *************************************************
  {PAGE_KTN_EDIT, LABEL, 'K', 'A', 'T', 'A', 'N', 'A', ' ', 'E'},
  {PAGE_KTN_EDIT, LABEL, 'D', 'I', 'T', ' ', ' ', ' ', ' ', ' '},
  {PAGE_KTN_EDIT, 1, PAR_BANK_CATEGORY, KTN, 1}, // ** Switch 01 **
  {PAGE_KTN_EDIT, 2, PAR_BANK_CATEGORY, KTN, 2}, // ** Switch 02 **
  {PAGE_KTN_EDIT, 3, PAR_BANK_CATEGORY, KTN, 3}, // ** Switch 03 **
  {PAGE_KTN_EDIT, 4, PAR_BANK_CATEGORY, KTN, 4}, // ** Switch 04 **
  {PAGE_KTN_EDIT, 5, PAR_BANK_CATEGORY, KTN, 5}, // ** Switch 05 **
  {PAGE_KTN_EDIT, 6, PAR_BANK_CATEGORY, KTN, 6}, // ** Switch 06 **
  {PAGE_KTN_EDIT, 7, PAR_BANK_CATEGORY, KTN, 7}, // ** Switch 07 **
  {PAGE_KTN_EDIT, 8, PAR_BANK_CATEGORY, KTN, 8}, // ** Switch 08 **
  {PAGE_KTN_EDIT, 9, PAR_BANK_CATEGORY, KTN, 9}, // ** Switch 09 **
  {PAGE_KTN_EDIT, 10, PAR_BANK_CATEGORY, KTN, 10}, // ** Switch 10 **
  {PAGE_KTN_EDIT, 11, PAR_BANK_CATEGORY, KTN, 11 }, // ** Switch 11 **
  {PAGE_KTN_EDIT, 12, SAVE_PATCH, KTN }, // ** Switch 11 **
  {PAGE_KTN_EDIT, 13, PATCH, KTN, PREV}, // ** Switch 10 **
  {PAGE_KTN_EDIT, 14, PATCH, KTN, NEXT}, // ** Switch 11 **
  //{PAGE_KTN_EDIT, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 12 **
  {PAGE_KTN_EDIT, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 12 **

  // ******************************* PAGE 221: KATANA FX CTRL  *************************************************
  {PAGE_KTN_FX, LABEL, 'K', 'A', 'T', 'A', 'N', 'A', ' ', 'F'},
  {PAGE_KTN_FX, LABEL, 'X', ' ', 'C', 'T', 'R', 'L', ' ', ' '},
  {PAGE_KTN_FX, 1, PARAMETER, KTN, 0, TOGGLE, 1, 0}, // ** Switch 01 **
  {PAGE_KTN_FX, 2, PARAMETER, KTN, 8, TOGGLE, 1, 0}, // ** Switch 02 **
  {PAGE_KTN_FX, 3, PARAMETER, KTN, 23, TOGGLE, 1, 0}, // ** Switch 03 **
  {PAGE_KTN_FX, 4, PARAMETER, KTN, 70, TOGGLE, 1, 0}, // ** Switch 04 **
  {PAGE_KTN_FX, 5, PARAMETER, KTN, 61, TOGGLE, 1, 0}, // ** Switch 05 **
  {PAGE_KTN_FX, 6, PARAMETER, KTN, 87, TOGGLE, 1, 0}, // ** Switch 06 **
  {PAGE_KTN_FX, 7, PARAMETER, KTN, 96, TOGGLE, 1, 0}, // ** Switch 07 **
  {PAGE_KTN_FX, 8, PARAMETER, KTN, 48, TOGGLE, 1, 0}, // ** Switch 08 **
  {PAGE_KTN_FX, 9, PARAMETER, KTN, 106, TOGGLE, 1, 0}, // ** Switch 09 **
  {PAGE_KTN_FX, 10, OPEN_PAGE_DEVICE, KTN, PAGE_KTN_EDIT}, // ** Switch 10 **
  //{PAGE_KTN_FX, 11, PAR_BANK_CATEGORY, KTN, 11 }, // ** Switch 11 **
  //{PAGE_KTN_FX, 12, PAR_BANK_CATEGORY, KTN, 12}, // ** Switch 12 **
  {PAGE_KTN_FX, 13, PATCH, KTN, PREV}, // ** Switch 10 **
  {PAGE_KTN_FX, 14, PATCH, KTN, NEXT}, // ** Switch 11 **
  //{PAGE_KTN_FX, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 12 **
  {PAGE_KTN_FX, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 12 **

  // ******************************* PAGE 222: KPA Rig select *************************************************
  {PAGE_KPA_RIG_SELECT, LABEL, 'K', 'P', 'A', ' ', 'R', 'I', 'G', ' ' },
  {PAGE_KPA_RIG_SELECT, LABEL, 'S', 'E', 'L', 'E', 'C', 'T', ' ', ' ' },
  {PAGE_KPA_RIG_SELECT, 1, PATCH, KPA, BANKSELECT, 1, 5}, // ** Switch 01 **
  {PAGE_KPA_RIG_SELECT, 2, PATCH, KPA, BANKSELECT, 2, 5}, // ** Switch 02 **
  {PAGE_KPA_RIG_SELECT, 3, PATCH, KPA, BANKSELECT, 3, 5}, // ** Switch 03 **
  {PAGE_KPA_RIG_SELECT, 4, PATCH, KPA, BANKSELECT, 4, 5}, // ** Switch 04 **
  {PAGE_KPA_RIG_SELECT, 5, PARAMETER, KPA, 0, TOGGLE, 1, 0}, // ** Switch 05 **
  {PAGE_KPA_RIG_SELECT, 6, PARAMETER, KPA, 1, TOGGLE, 1, 0}, // ** Switch 06 **
  {PAGE_KPA_RIG_SELECT, 7, PARAMETER, KPA, 2, TOGGLE, 1, 0}, // ** Switch 07 **
  {PAGE_KPA_RIG_SELECT, 8, PATCH, KPA, BANKSELECT, 5, 5}, // ** Switch 08 **
  {PAGE_KPA_RIG_SELECT, 9, PARAMETER, KPA, 3, TOGGLE, 1, 0}, // ** Switch 09 **
  {PAGE_KPA_RIG_SELECT, 10, PARAMETER, KPA, 5, TOGGLE, 1, 0}, // ** Switch 10 **
  {PAGE_KPA_RIG_SELECT, 11, PARAMETER, KPA, 6, TOGGLE, 1, 0}, // ** Switch 11 **
  {PAGE_KPA_RIG_SELECT, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_KPA_RIG_SELECT, 13, PATCH, KPA, BANKDOWN, 5}, // ** Switch 13 **
  {PAGE_KPA_RIG_SELECT, 13 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
  {PAGE_KPA_RIG_SELECT, 14, PATCH, KPA, BANKUP, 5}, // ** Switch 14 **
  {PAGE_KPA_RIG_SELECT, 14 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
  //{PAGE_KPA_RIG_SELECT, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_KPA_RIG_SELECT, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

  // ******************************* PAGE 223: KPA Looper control *************************************************
  {PAGE_KPA_LOOPER, LABEL, 'K', 'P', 'A', ' ', 'L', 'O', 'O', 'P' },
  {PAGE_KPA_LOOPER, LABEL, 'E', 'R', ' ', 'C', 'T', 'L', ' ', ' ' },
  {PAGE_KPA_LOOPER, 1, LOOPER, CURRENT, LOOPER_REC_PLAY_OVERDUB}, // ** Switch 01 **
  {PAGE_KPA_LOOPER, 2, LOOPER, CURRENT, LOOPER_STOP_ERASE}, // ** Switch 02 **
  {PAGE_KPA_LOOPER, 3, PARAMETER, KPA, 0, TOGGLE, 1, 0}, // ** Switch 03 **
  {PAGE_KPA_LOOPER, 4, PARAMETER, KPA, 1, TOGGLE, 1, 0}, // ** Switch 04 **
  {PAGE_KPA_LOOPER, 5, LOOPER, CURRENT, LOOPER_UNDO_REDO}, // ** Switch 05 **
  {PAGE_KPA_LOOPER, 6, LOOPER, CURRENT, LOOPER_PLAY_ONCE}, // ** Switch 06 **
  {PAGE_KPA_LOOPER, 7, PARAMETER, KPA, 5, TOGGLE, 1, 0}, // ** Switch 07 **
  {PAGE_KPA_LOOPER, 8, PARAMETER, KPA, 6, TOGGLE, 1, 0}, // ** Switch 08 **
  {PAGE_KPA_LOOPER, 9, LOOPER, CURRENT, LOOPER_HALF_SPEED}, // ** Switch 09 **
  {PAGE_KPA_LOOPER, 10, LOOPER, CURRENT, LOOPER_REVERSE}, // ** Switch 10 **
  //{PAGE_KPA_LOOPER, 11, PATCH, CURRENT, BANKSELECT, 3, 3}, // ** Switch 11 **
  {PAGE_KPA_LOOPER, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_KPA_LOOPER, 13, PATCH, CURRENT, PREV, 3}, // ** Switch 13 **
  {PAGE_KPA_LOOPER, 14, PATCH, CURRENT, NEXT, 3}, // ** Switch 14 **
  //{PAGE_KPA_LOOPER, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_KPA_LOOPER, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

  // ******************************* PAGE 224: Parameters current device *************************************************
  {PAGE_CURRENT_ASSIGN, LABEL, 'A', 'S', 'G', ' ', 'B', 'A', 'N', 'K' },
  {PAGE_CURRENT_ASSIGN, 1, ASSIGN, CURRENT, BANKSELECT, 1, 10}, // ** Switch 01 **
  {PAGE_CURRENT_ASSIGN, 2, ASSIGN, CURRENT, BANKSELECT, 2, 10}, // ** Switch 02 **
  {PAGE_CURRENT_ASSIGN, 3, ASSIGN, CURRENT, BANKSELECT, 3, 10}, // ** Switch 03 **
  {PAGE_CURRENT_ASSIGN, 4, ASSIGN, CURRENT, BANKSELECT, 4, 10}, // ** Switch 04 **
  {PAGE_CURRENT_ASSIGN, 5, ASSIGN, CURRENT, BANKSELECT, 5, 10}, // ** Switch 05 **
  {PAGE_CURRENT_ASSIGN, 6, ASSIGN, CURRENT, BANKSELECT, 6, 10}, // ** Switch 06 **
  {PAGE_CURRENT_ASSIGN, 7, ASSIGN, CURRENT, BANKSELECT, 7, 10}, // ** Switch 07 **
  {PAGE_CURRENT_ASSIGN, 8, ASSIGN, CURRENT, BANKSELECT, 8, 10}, // ** Switch 08 **
  {PAGE_CURRENT_ASSIGN, 9, ASSIGN, CURRENT, BANKSELECT, 9, 10}, // ** Switch 09 **
  {PAGE_CURRENT_ASSIGN, 10, ASSIGN, CURRENT, BANKSELECT, 10, 10}, // ** Switch 10 **
  //{PAGE_CURRENT_ASSIGN, 11, NOTHING, COMMON}, // ** Switch 11 **
  //{PAGE_CURRENT_ASSIGN, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_CURRENT_ASSIGN, 13, ASSIGN, CURRENT, BANKDOWN, 10}, // ** Switch 13 **
  {PAGE_CURRENT_ASSIGN, 14, ASSIGN, CURRENT, BANKUP, 10}, // ** Switch 14 **
  //{PAGE_CURRENT_ASSIGN, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_CURRENT_ASSIGN, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

  // ******************************* PAGE 225: SY1000_patch_bank (8 buttons per page) *************************************************
  {PAGE_SY1000_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', 'B', 'N' },
  {PAGE_SY1000_PATCH_BANK, LABEL, 'K', ' ', 'S', 'Y', '1', '0', '0', '0' },
  {PAGE_SY1000_PATCH_BANK, 1, PATCH, SY1000, BANKSELECT, 1, 8}, // ** Switch 01 **
  {PAGE_SY1000_PATCH_BANK, 2, PATCH, SY1000, BANKSELECT, 2, 8}, // ** Switch 02 **
  {PAGE_SY1000_PATCH_BANK, 3, PATCH, SY1000, BANKSELECT, 3, 8}, // ** Switch 03 **
  {PAGE_SY1000_PATCH_BANK, 4, PATCH, SY1000, BANKSELECT, 4, 8}, // ** Switch 04 **
  {PAGE_SY1000_PATCH_BANK, 5, PATCH, SY1000, BANKSELECT, 5, 8}, // ** Switch 05 **
  {PAGE_SY1000_PATCH_BANK, 6, PATCH, SY1000, BANKSELECT, 6, 8}, // ** Switch 06 **
  {PAGE_SY1000_PATCH_BANK, 7, PATCH, SY1000, BANKSELECT, 7, 8}, // ** Switch 07 **
  {PAGE_SY1000_PATCH_BANK, 8, PATCH, SY1000, BANKSELECT, 8, 8}, // ** Switch 08 **
  {PAGE_SY1000_PATCH_BANK, 9, ASSIGN, SY1000, SELECT, 0, 0}, // ** Switch 09 **
  {PAGE_SY1000_PATCH_BANK, 10, ASSIGN, SY1000, SELECT, 1, 1}, // ** Switch 10 **
  //{PAGE_SY1000_PATCH_BANK, 11, PARAMETER, SY1000, 1, TOGGLE, 1, 0}, // ** Switch 11 **
  //{PAGE_SY1000_PATCH_BANK, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_SY1000_PATCH_BANK, 13, PATCH, SY1000, BANKDOWN, 8}, // ** Switch 13 **
  {PAGE_SY1000_PATCH_BANK, 13 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
  {PAGE_SY1000_PATCH_BANK, 14, PATCH, SY1000, BANKUP, 8}, // ** Switch 14 **
  {PAGE_SY1000_PATCH_BANK, 14 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
  //{PAGE_SY1000_PATCH_BANK, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_SY1000_PATCH_BANK, 16, OPEN_NEXT_PAGE_OF_DEVICE, SY1000}, // ** Switch 16 **

  // ******************************* PAGE 226: SY1000 assign *************************************************
  {PAGE_SY1000_ASSIGNS, LABEL, 'A', 'S', 'S', 'I', 'G', 'N', 'S', ' ' },
  {PAGE_SY1000_ASSIGNS, LABEL, 'S', 'Y', '1', '0', '0', '0', ' ', ' ' },
  {PAGE_SY1000_ASSIGNS, 1, ASSIGN, SY1000, BANKSELECT, 1, 8}, // ** Switch 01 **
  {PAGE_SY1000_ASSIGNS, 2, ASSIGN, SY1000, BANKSELECT, 2, 8}, // ** Switch 02 **
  {PAGE_SY1000_ASSIGNS, 3, ASSIGN, SY1000, BANKSELECT, 3, 8}, // ** Switch 03 **
  {PAGE_SY1000_ASSIGNS, 4, ASSIGN, SY1000, BANKSELECT, 4, 8}, // ** Switch 04 **
  {PAGE_SY1000_ASSIGNS, 5, ASSIGN, SY1000, BANKSELECT, 5, 8}, // ** Switch 05 **
  {PAGE_SY1000_ASSIGNS, 6, ASSIGN, SY1000, BANKSELECT, 6, 8}, // ** Switch 06 **
  {PAGE_SY1000_ASSIGNS, 7, ASSIGN, SY1000, BANKSELECT, 7, 8}, // ** Switch 07 **
  {PAGE_SY1000_ASSIGNS, 8, ASSIGN, SY1000, BANKSELECT, 8, 8}, // ** Switch 08 **
  {PAGE_SY1000_ASSIGNS, 9, PATCH, SY1000, PREV, 1}, // ** Switch 09 **
  {PAGE_SY1000_ASSIGNS, 10, PATCH, SY1000, NEXT, 1}, // ** Switch 10 **
  //{PAGE_SY1000_ASSIGNS, 11, NOTHING, COMMON}, // ** Switch 11 **
  //{PAGE_SY1000_ASSIGNS, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_SY1000_ASSIGNS, 13, ASSIGN, SY1000, BANKDOWN, 8}, // ** Switch 13 **
  {PAGE_SY1000_ASSIGNS, 14, ASSIGN, SY1000, BANKUP, 8}, // ** Switch 14 **
  //{PAGE_SY1000_ASSIGNS, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_SY1000_ASSIGNS, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

  // ******************************* PAGE 227: Scenes (SY1000) *************************************************
  {PAGE_SY1000_SCENES, LABEL, 'S', 'C', 'E', 'N', 'E', 'S', ' ', 'S' },
  {PAGE_SY1000_SCENES, LABEL, 'Y', '1', '0', '0', '0', ' ', ' ', ' ' },
  {PAGE_SY1000_SCENES, 1, SNAPSCENE, CURRENT, 1, 0, 0}, // ** Switch 01 **
  {PAGE_SY1000_SCENES, 2, SNAPSCENE, CURRENT, 2, 0, 0}, // ** Switch 02 **
  {PAGE_SY1000_SCENES, 3, SNAPSCENE, CURRENT, 3, 0, 0}, // ** Switch 03 **
  {PAGE_SY1000_SCENES, 4, SNAPSCENE, CURRENT, 4, 0, 0}, // ** Switch 04 **
  {PAGE_SY1000_SCENES, 5, SNAPSCENE, CURRENT, 5, 0, 0}, // ** Switch 05 **
  {PAGE_SY1000_SCENES, 6, SNAPSCENE, CURRENT, 6, 0, 0}, // ** Switch 06 **
  {PAGE_SY1000_SCENES, 7, SNAPSCENE, CURRENT, 7, 0, 0}, // ** Switch 07 **
  {PAGE_SY1000_SCENES, 8, SNAPSCENE, CURRENT, 8, 0, 0}, // ** Switch 08 **
  //{PAGE_SY1000_SCENES, 9, SAVE_PATCH, SY1000}, // ** Switch 09 **
  {PAGE_SY1000_SCENES, 10, SAVE_PATCH, SY1000}, // ** Switch 10 **
  //{PAGE_SY1000_SCENES, 11, PARAMETER, CURRENT, 10, MOMENTARY, 127, 0}, // ** Switch 11 **
  //{PAGE_SY1000_SCENES, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_SY1000_SCENES, 13, PATCH, CURRENT, PREV}, // ** Switch 13 **
  {PAGE_SY1000_SCENES, 14, PATCH, CURRENT, NEXT}, // ** Switch 14 **
  //{PAGE_SY1000_SCENES, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_SY1000_SCENES, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

  // ******************************* PAGE 228: MG300_patch_bank (8 buttons per page) *************************************************
  {PAGE_MG300_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', 'B', 'A' },
  {PAGE_MG300_PATCH_BANK, LABEL, 'N', 'K', ' ', 'M', 'G', '3', '0', '0' },
  {PAGE_MG300_PATCH_BANK, 1, PATCH, MG300, BANKSELECT, 1, 8}, // ** Switch 01 **
  {PAGE_MG300_PATCH_BANK, 2, PATCH, MG300, BANKSELECT, 2, 8}, // ** Switch 02 **
  {PAGE_MG300_PATCH_BANK, 3, PATCH, MG300, BANKSELECT, 3, 8}, // ** Switch 03 **
  {PAGE_MG300_PATCH_BANK, 4, PATCH, MG300, BANKSELECT, 4, 8}, // ** Switch 04 **
  {PAGE_MG300_PATCH_BANK, 5, PATCH, MG300, BANKSELECT, 5, 8}, // ** Switch 05 **
  {PAGE_MG300_PATCH_BANK, 6, PATCH, MG300, BANKSELECT, 6, 8}, // ** Switch 06 **
  {PAGE_MG300_PATCH_BANK, 7, PATCH, MG300, BANKSELECT, 7, 8}, // ** Switch 07 **
  {PAGE_MG300_PATCH_BANK, 8, PATCH, MG300, BANKSELECT, 8, 8}, // ** Switch 08 **
  //{PAGE_MG300_PATCH_BANK, 9, PARAMETER, MG300, 14, STEP, 0, 6, 1}, // ** Switch 09 **
  //{PAGE_MG300_PATCH_BANK, 10, PAGE, COMMON, SELECT, PAGE_FULL_LOOPER}, // ** Switch 10 **
  //{PAGE_MG300_PATCH_BANK, 11, PARAMETER, MG300, 1, TOGGLE, 1, 0}, // ** Switch 11 **
  //{PAGE_MG300_PATCH_BANK, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_MG300_PATCH_BANK, 13, PATCH, MG300, BANKDOWN, 8}, // ** Switch 13 **
  {PAGE_MG300_PATCH_BANK, 13 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
  {PAGE_MG300_PATCH_BANK, 14, PATCH, MG300, BANKUP, 8}, // ** Switch 14 **
  {PAGE_MG300_PATCH_BANK, 14 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
  //{PAGE_MG300_PATCH_BANK, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_MG300_PATCH_BANK, 16, OPEN_NEXT_PAGE_OF_DEVICE, MG300}, // ** Switch 16 **
};

const uint16_t NUMBER_OF_INTERNAL_COMMANDS = sizeof(Fixed_commands) / sizeof(Fixed_commands[0]);

// ********************************* Section 3: VController default configuration for programmable pages ********************************************

// Default configuration of the switches of the user pages - this configuration will be restored when selecting menu - firmware menu - Init commands

// Every switch can have any number commands. Only the first command is shown in the display.

#define LOWEST_USER_PAGE 1 // Minimum value for PAGE DOWN

#define PAGE_DEFAULT 0
#define PAGE_COMBO1 1
#define PAGE_COMBO2 2
#define PAGE_FUNCTIONS_TEST 3
#define PAGE_GM_TEST 4

const PROGMEM Cmd_struct Default_commands[] = {
  // ******************************* PAGE 00: Default page *************************************************
  // Page, Switch, Type, Device, Data1, Data2, Value1, Value2, Value3, Value4, Value5
  //{PAGE_DEFAULT, LABEL, 'D', 'E', 'F', 'A', 'U', 'L', 'T', ' ' },
  {PAGE_DEFAULT, 1, NOTHING, COMMON}, // ** Switch 01 **
  {PAGE_DEFAULT, 2, NOTHING, COMMON}, // ** Switch 02 **
  {PAGE_DEFAULT, 3, NOTHING, COMMON}, // ** Switch 03 **
  {PAGE_DEFAULT, 4, NOTHING, COMMON}, // ** Switch 04 **
  {PAGE_DEFAULT, 5, NOTHING, COMMON}, // ** Switch 05 **
  {PAGE_DEFAULT, 6, NOTHING, COMMON}, // ** Switch 06 **
  {PAGE_DEFAULT, 7, NOTHING, COMMON}, // ** Switch 07 **
  {PAGE_DEFAULT, 8, NOTHING, COMMON}, // ** Switch 08 **
  {PAGE_DEFAULT, 9, NOTHING, COMMON}, // ** Switch 09 **
  {PAGE_DEFAULT, 10, NOTHING, COMMON}, // ** Switch 10 **
  {PAGE_DEFAULT, 11, TOGGLE_EXP_PEDAL, CURRENT}, // ** Switch 11 **
  {PAGE_DEFAULT, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_DEFAULT, 12 | ON_LONG_PRESS, GLOBAL_TUNER, COMMON}, // ** Switch 12 **
  {PAGE_DEFAULT, 13, NOTHING, COMMON}, // ** Switch 13 **
  {PAGE_DEFAULT, 14, NOTHING, COMMON}, // ** Switch 14 **
  {PAGE_DEFAULT, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_DEFAULT, 15 | ON_LONG_PRESS, PAGE, COMMON, SELECT, PAGE_SELECT}, // ** Switch 15 **
  {PAGE_DEFAULT, 16, PAGE, COMMON, NEXT}, // ** Switch 16 **
  {PAGE_DEFAULT, 16 | ON_LONG_PRESS, PAGE, COMMON, SELECT, PAGE_SELECT}, // ** Switch 15 **
  {PAGE_DEFAULT, 17, MASTER_EXP_PEDAL, CURRENT, 0}, // External switch 1 or expr pedal 1
  {PAGE_DEFAULT, 18, NOTHING, COMMON}, // External switch 2
  {PAGE_DEFAULT, 19, NOTHING, COMMON}, // External switch 3 or expr pedal 2
  {PAGE_DEFAULT, 20, NOTHING, COMMON}, // External switch 4
  {PAGE_DEFAULT, 21, NOTHING, COMMON}, // External switch 5 or expr pedal 3
  {PAGE_DEFAULT, 22, NOTHING, COMMON}, // External switch 6
  {PAGE_DEFAULT, 23, PATCH, CURRENT, PREV}, // External switch 7 or expr pedal 4
  {PAGE_DEFAULT, 24, PATCH, CURRENT, NEXT}, // External switch 8

  // ******************************* PAGE 01: GP+GR *************************************************
  {PAGE_COMBO1, LABEL, 'G', 'P', '+', 'G', 'R', ' ', ' ', ' ' },
  {PAGE_COMBO1, 1, PATCH, GP10, BANKSELECT, 1, 5}, // ** Switch 01 **
  {PAGE_COMBO1, 2, PATCH, GP10, BANKSELECT, 2, 5}, // ** Switch 02 **
  {PAGE_COMBO1, 3, PATCH, GP10, BANKSELECT, 3, 5}, // ** Switch 03 **
  {PAGE_COMBO1, 4, PATCH, GP10, BANKSELECT, 4, 5}, // ** Switch 04 **
  {PAGE_COMBO1, 5, PATCH, GR55, BANKSELECT, 1, 6}, // ** Switch 05 **
  {PAGE_COMBO1, 6, PATCH, GR55, BANKSELECT, 2, 6}, // ** Switch 06 **
  {PAGE_COMBO1, 7, PATCH, GR55, BANKSELECT, 3, 6}, // ** Switch 07 **
  {PAGE_COMBO1, 8, PATCH, GP10, BANKSELECT, 5, 5}, // ** Switch 08 **
  {PAGE_COMBO1, 9, PATCH, GR55, BANKSELECT, 4, 6}, // ** Switch 09 **
  {PAGE_COMBO1, 10, PATCH, GR55, BANKSELECT, 5, 6}, // ** Switch 10 **
  {PAGE_COMBO1, 11, PATCH, GR55, BANKSELECT, 6, 6}, // ** Switch 11 **
  //{PAGE_COMBO1, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_COMBO1, 13, PATCH, CURRENT, BANKDOWN, 5}, // ** Switch 13 **
  {PAGE_COMBO1, 13 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
  {PAGE_COMBO1, 14, PATCH, CURRENT, BANKUP, 5}, // ** Switch 14 **
  {PAGE_COMBO1, 14 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_CURRENT_DIRECT_SELECT}, // ** Switch 14 **
  //Switch 15 and 16 are BANK_DOWN and BANK_UP

  // ******************************* PAGE 02: GPVGGR *************************************************
  {PAGE_COMBO2, LABEL, 'G', 'P', 'V', 'G', 'G', 'R', ' ', ' ' },
  {PAGE_COMBO2, 1, PATCH, GP10, BANKSELECT, 1, 4}, // ** Switch 01 **
  {PAGE_COMBO2, 2, PATCH, GP10, BANKSELECT, 2, 4}, // ** Switch 02 **
  {PAGE_COMBO2, 3, PATCH, GP10, BANKSELECT, 3, 4}, // ** Switch 03 **
  {PAGE_COMBO2, 4, PATCH, GP10, BANKSELECT, 4, 4}, // ** Switch 04 **
  {PAGE_COMBO2, 5, PATCH, VG99, BANKSELECT, 1, 4}, // ** Switch 05 **
  {PAGE_COMBO2, 6, PATCH, VG99, BANKSELECT, 2, 4}, // ** Switch 06 **
  {PAGE_COMBO2, 7, PATCH, VG99, BANKSELECT, 3, 4}, // ** Switch 07 **
  {PAGE_COMBO2, 8, PATCH, VG99, BANKSELECT, 4, 4}, // ** Switch 08 **
  {PAGE_COMBO2, 9, PATCH, GR55, BANKSELECT, 1, 3}, // ** Switch 09 **
  {PAGE_COMBO2, 10, PATCH, GR55, BANKSELECT, 2, 3}, // ** Switch 10 **
  {PAGE_COMBO2, 11, PATCH, GR55, BANKSELECT, 3, 3}, // ** Switch 11 **
  //{PAGE_COMBO2, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_COMBO2, 13, PATCH, CURRENT, BANKDOWN, 4}, // ** Switch 13 **
  {PAGE_COMBO2, 13 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_CURRENT_DIRECT_SELECT}, // ** Switch 13 **
  {PAGE_COMBO2, 14, PATCH, CURRENT, BANKUP, 4}, // ** Switch 14 **
  {PAGE_COMBO2, 14 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, CURRENT, PAGE_CURRENT_DIRECT_SELECT}, // ** Switch 14 **
  //Switch 15 and 16 are BANK_DOWN and BANK_UP

  // ******************************* PAGE 03: FUNCTION TEST *************************************************
  {PAGE_FUNCTIONS_TEST, LABEL, 'F', 'U', 'N', 'C', 'T', 'I', 'O', 'N' },
  {PAGE_FUNCTIONS_TEST, LABEL, ' ', 'T', 'E', 'S', 'T', ' ', ' ', ' ' },
  {PAGE_FUNCTIONS_TEST, 1, PATCH, CURRENT, SELECT, 1, 0}, // ** Switch 01 **
  {PAGE_FUNCTIONS_TEST, 2, PATCH, CURRENT, BANKDOWN, 1}, // ** Switch 02 **
  {PAGE_FUNCTIONS_TEST, 3, PATCH, CURRENT, BANKSELECT, 1, 1}, // ** Switch 03 **
  {PAGE_FUNCTIONS_TEST, 4, PATCH, CURRENT, BANKUP, 1}, // ** Switch 04 **
  {PAGE_FUNCTIONS_TEST, 5, PARAMETER, CURRENT, 0, TOGGLE, 1, 0}, // ** Switch 05 **
  {PAGE_FUNCTIONS_TEST, 6, PARAMETER, CURRENT, 1, TOGGLE, 1, 0}, // ** Switch 06 **
  {PAGE_FUNCTIONS_TEST, 7, PATCH, CURRENT, NEXT}, // ** Switch 07 **
  {PAGE_FUNCTIONS_TEST, 8, MUTE, CURRENT}, // ** Switch 08 **
  {PAGE_FUNCTIONS_TEST, 9, GLOBAL_TUNER, COMMON}, // ** Switch 09 **
  {PAGE_FUNCTIONS_TEST, 10, SET_TEMPO, COMMON, 95}, // ** Switch 10 **
  {PAGE_FUNCTIONS_TEST, 11, SET_TEMPO, COMMON, 120}, // ** Switch 11 **
  //{PAGE_FUNCTIONS_TEST, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  //{PAGE_FUNCTIONS_TEST, 13, PATCH, CURRENT, BANKDOWN, 4}, // ** Switch 13 **
  //{PAGE_FUNCTIONS_TEST, 14, PATCH, CURRENT, BANKUP, 4}, // ** Switch 14 **
  //Switch 15 and 16 are BANK_DOWN and BANK_UP

  // ******************************* PAGE 04: GM TEST *************************************************
  {PAGE_GM_TEST, LABEL, 'G', 'E', 'N', '.', 'M', 'I', 'D', 'I' },
  {PAGE_GM_TEST, LABEL, ' ', 'T', 'E', 'S', 'T', ' ', ' ', ' ' },
  {PAGE_GM_TEST, 1, MIDI_PC, COMMON, 1, 1, ALL_MIDI_PORTS}, // ** Switch 01 **
  {PAGE_GM_TEST, 2, MIDI_PC, COMMON, 2, 1, ALL_MIDI_PORTS}, // ** Switch 02 **
  {PAGE_GM_TEST, 3, MIDI_PC, COMMON, 3, 1, ALL_MIDI_PORTS}, // ** Switch 03 **
  {PAGE_GM_TEST, 3 | LABEL, 'C', 'U', 'S', 'T', 'O', 'M', ' ', 'L'}, // ** Switch 03 **
  {PAGE_GM_TEST, 3 | LABEL, 'A', 'B', 'E', 'L', ' ', 'P', 'C', '3'}, // ** Switch 03 **
  {PAGE_GM_TEST, 4, MIDI_NOTE, COMMON, 52, 100, 1, ALL_MIDI_PORTS}, // ** Switch 04 **
  {PAGE_GM_TEST, 5, MIDI_CC, COMMON, 30, CC_ONE_SHOT, 127, 0, 1, ALL_MIDI_PORTS}, // ** Switch 05 **
  {PAGE_GM_TEST, 5 | LABEL, 'O', 'N', 'E', ' ', 'S', 'H', 'O', 'T'}, // ** Switch 05 **
  {PAGE_GM_TEST, 6, MIDI_CC, COMMON, 31, CC_MOMENTARY, 127, 0, 1, ALL_MIDI_PORTS}, // ** Switch 06 **
  {PAGE_GM_TEST, 6 | LABEL, 'M', 'O', 'M', 'E', 'N', 'T', 'A', 'R'}, // ** Switch 06 **
  {PAGE_GM_TEST, 6 | LABEL, 'Y', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // ** Switch 06 **
  {PAGE_GM_TEST, 7, MIDI_CC, COMMON, 30, CC_TOGGLE, 127, 0, 1, ALL_MIDI_PORTS}, // ** Switch 07 **
  {PAGE_GM_TEST, 7 | LABEL, 'T', 'O', 'G', 'G', 'L', 'E', ' ', ' '}, // ** Switch 07 **
  {PAGE_GM_TEST, 8, MIDI_CC, COMMON, 31, CC_TOGGLE_ON, 127, 0, 1, ALL_MIDI_PORTS}, // ** Switch 08 **
  {PAGE_GM_TEST, 8 | LABEL, 'T', 'O', 'G', 'G', 'L', 'E', ' ', 'O'}, // ** Switch 08 **
  {PAGE_GM_TEST, 8 | LABEL, 'N', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // ** Switch 08 **
  {PAGE_GM_TEST, 9, MIDI_CC, COMMON, 32, CC_UPDOWN, 127, 0, 1, ALL_MIDI_PORTS}, // ** Switch 09 **
  {PAGE_GM_TEST, 10, MIDI_CC, COMMON, 33, CC_STEP, 3, 0, 1, ALL_MIDI_PORTS}, // ** Switch 10 **
  {PAGE_GM_TEST, 11, SET_TEMPO, COMMON, 120}, // ** Switch 11 **
  //{PAGE_GM_TEST, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  //{PAGE_GM_TEST, 13, PATCH, CURRENT, BANKDOWN, 4}, // ** Switch 13 **
  //{PAGE_GM_TEST, 14, PATCH, CURRENT, BANKUP, 4}, // ** Switch 14 **
  //Switch 15 and 16 are BANK_DOWN and BANK_UP
};

const uint16_t NUMBER_OF_INIT_COMMANDS = sizeof(Default_commands) / sizeof(Default_commands[0]);

#define MENU_KEY_COMBINATION 0x9000 // Switches 13 and 16
#endif

// ********************************* Section 4: VC-mini configuration for fixed command pages ********************************************
#ifdef CONFIG_VCMINI

#define FIRST_FIXED_CMD_PAGE 201
#define FIRST_SELECTABLE_FIXED_CMD_PAGE 203
#define PAGE_MENU 201
#define PAGE_CURRENT_DIRECT_SELECT 202
#define PAGE_SELECT 203
#define PAGE_BANK_SELECT 204
#define PAGE_CURRENT_PATCH_BANK 205
#define PAGE_UP_DOWN_TAP 206
#define PAGE_CURRENT_PARAMETER 207
#define PAGE_CURRENT_ASSIGN 208
#define PAGE_SNAPSCENE 209
#define PAGE_LOOPER 210
#define PAGE_KPA_LOOPER 211
#define PAGE_KTN_FX1 212
#define PAGE_KTN_FX2 213
#define PAGE_KTN_FX3 214
#define PAGE_KTN4_FX1 215
#define PAGE_KTN4_FX2 216
#define PAGE_KTN4_FX3 217
#define PAGE_MG300_FX1 218
#define PAGE_MG300_FX2 219
#define PAGE_MG300_FX3 220
#define PAGE_SY1000_MODE_SEL1 221
#define PAGE_SY1000_MODE_SEL2 222
#define LAST_FIXED_CMD_PAGE 222

#define DEFAULT_PAGE PAGE_CURRENT_PATCH_BANK // The page that gets selected when a valid page number is unknown

// Default pages for devices
#define FAS_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK
#define FAS_DEFAULT_PAGE2 PAGE_CURRENT_PARAMETER
#define FAS_DEFAULT_PAGE3 PAGE_SNAPSCENE
#define FAS_DEFAULT_PAGE4 0

#define GP10_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK
#define GP10_DEFAULT_PAGE2 PAGE_CURRENT_PARAMETER
#define GP10_DEFAULT_PAGE3 PAGE_CURRENT_ASSIGN
#define GP10_DEFAULT_PAGE4 0

#define GR55_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK
#define GR55_DEFAULT_PAGE2 PAGE_CURRENT_PARAMETER
#define GR55_DEFAULT_PAGE3 PAGE_CURRENT_ASSIGN
#define GR55_DEFAULT_PAGE4 0

#define HLX_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK
#define HLX_DEFAULT_PAGE2 PAGE_SNAPSCENE
#define HLX_DEFAULT_PAGE3 PAGE_LOOPER
#define HLX_DEFAULT_PAGE4 0

#define KPA_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK
#define KPA_DEFAULT_PAGE2 PAGE_CURRENT_PARAMETER
#define KPA_DEFAULT_PAGE3 PAGE_KPA_LOOPER
#define KPA_DEFAULT_PAGE4 0

#define KTN_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK
#define KTN_DEFAULT_PAGE2 PAGE_KTN4_FX1
#define KTN_DEFAULT_PAGE3 0
#define KTN_DEFAULT_PAGE4 0

#define M13_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK
#define M13_DEFAULT_PAGE2 PAGE_CURRENT_PARAMETER
#define M13_DEFAULT_PAGE3 PAGE_LOOPER
#define M13_DEFAULT_PAGE4 0

#define VG99_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK;  // Default value
#define VG99_DEFAULT_PAGE2 PAGE_CURRENT_ASSIGN; // Default value
#define VG99_DEFAULT_PAGE3 0; // Default value
#define VG99_DEFAULT_PAGE4 0; // Default value

#define ZG3_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK
#define ZG3_DEFAULT_PAGE2 PAGE_CURRENT_PARAMETER
#define ZG3_DEFAULT_PAGE3 0
#define ZG3_DEFAULT_PAGE4 0

#define ZMS70_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK
#define ZMS70_DEFAULT_PAGE2 PAGE_CURRENT_PARAMETER
#define ZMS70_DEFAULT_PAGE3 0
#define ZMS70_DEFAULT_PAGE4 0

#define SVL_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK
#define SVL_DEFAULT_PAGE2 PAGE_CURRENT_PARAMETER
#define SVL_DEFAULT_PAGE3 0
#define SVL_DEFAULT_PAGE4 0

#define SY1000_DEFAULT_PAGE1 PAGE_SY1000_MODE_SEL1
#define SY1000_DEFAULT_PAGE2 PAGE_CURRENT_PATCH_BANK
#define SY1000_DEFAULT_PAGE3 PAGE_CURRENT_ASSIGN
#define SY1000_DEFAULT_PAGE4 0

#define GM2_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK
#define GM2_DEFAULT_PAGE2 PAGE_CURRENT_PARAMETER
#define GM2_DEFAULT_PAGE3 0
#define GM2_DEFAULT_PAGE4 0

#define MG300_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK
#define MG300_DEFAULT_PAGE2 PAGE_MG300_FX1
#define MG300_DEFAULT_PAGE3 0
#define MG300_DEFAULT_PAGE4 0

const PROGMEM Cmd_struct Fixed_commands[] = {
  // ******************************* PAGE 201: MENU *************************************************
  {PAGE_MENU, LABEL, 'M', 'E', 'N', 'U', ' ', ' ', ' ', ' ' },
  {PAGE_MENU, 1, MENU, COMMON, MENU_PREV}, // ** Switch 01 **
  {PAGE_MENU, 2, MENU, COMMON, MENU_NEXT}, // ** Switch 02 **
  {PAGE_MENU, 3, MENU, COMMON, MENU_SET_VALUE}, // ** Switch 03 *
  {PAGE_MENU, 4, MENU, COMMON, MENU_SELECT }, // Encoder 1
  {PAGE_MENU, 5, MENU, COMMON, MENU_BACK }, // Encoder 1 button
  {PAGE_MENU, 6, MENU, COMMON, MENU_SET_VALUE }, // Encoder 2
  {PAGE_MENU, 7, MENU, COMMON, MENU_SET_VALUE }, // Encoder 2 button
  {PAGE_MENU, 8, MASTER_EXP_PEDAL, CURRENT, 0}, // External switch 1 or expr pedal 1
  {PAGE_MENU, 9, NOTHING, COMMON}, // External switch 2

  // ******************************* PAGE 202: Current device Direct Select *************************************************
  {PAGE_CURRENT_DIRECT_SELECT, LABEL, 'D', 'I', 'R', 'E', 'C', 'T', ' ', 'S' },
  {PAGE_CURRENT_DIRECT_SELECT, LABEL, 'E', 'L', 'E', 'C', 'T', ' ', ' ', ' ' },
  {PAGE_CURRENT_DIRECT_SELECT, 1, DIRECT_SELECT, CURRENT, 1, 10}, // ** Switch 01 **
  {PAGE_CURRENT_DIRECT_SELECT, 2, DIRECT_SELECT, CURRENT, 2, 10}, // ** Switch 02 **
  {PAGE_CURRENT_DIRECT_SELECT, 3, DIRECT_SELECT, CURRENT, 3, 10}, // ** Switch 03 **

  // ******************************* PAGE 203: PAGE USER SELECT *************************************************
  {PAGE_SELECT, LABEL, 'S', 'E', 'L', 'E', 'C', 'T', ' ', 'U' },
  {PAGE_SELECT, LABEL, 'S', 'E', 'R', ' ', 'P', 'A', 'G', 'E' },
  {PAGE_SELECT, 1, PAGE, COMMON, SELECT, 1}, // ** Switch 01 **
  {PAGE_SELECT, 2, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 02 **
  {PAGE_SELECT, 3, PAGE, COMMON, SELECT, PAGE_MENU}, // ** Switch 03 **
  {PAGE_SELECT, 5 | ON_RELEASE, SELECT_NEXT_DEVICE, COMMON}, // ** Enc1 SW **
  {PAGE_SELECT, 7 | ON_RELEASE, PAGE, COMMON, SELECT, PAGE_MENU}, // ** Enc2 SW **

  // ******************************* PAGE 204: Pages current device *************************************************
  {PAGE_BANK_SELECT, LABEL, 'P', 'A', 'G', 'E', ' ', 'B', 'A', 'N' },
  {PAGE_BANK_SELECT, LABEL, 'K', ' ', 'S', 'E', 'L', 'E', 'C', 'T' },
  {PAGE_BANK_SELECT, 1 | ON_RELEASE, PAGE, COMMON, BANKSELECT, 1, 3}, // ** Switch 01 **
  {PAGE_BANK_SELECT, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
  {PAGE_BANK_SELECT, 2 | ON_RELEASE, PAGE, COMMON, BANKSELECT, 2, 3}, // ** Switch 02 **
  {PAGE_BANK_SELECT, 3 | ON_RELEASE, PAGE, COMMON, BANKSELECT, 3, 3}, // ** Switch 03 **
  {PAGE_BANK_SELECT, 3 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT},
  {PAGE_BANK_SELECT, 1 | ON_DUAL_PRESS, PAGE, COMMON, BANKDOWN, 3}, // ** Switch 01 + 02
  {PAGE_BANK_SELECT, 2 | ON_DUAL_PRESS, PAGE, COMMON, BANKUP, 3}, // ** Switch 02 + 03
  {PAGE_BANK_SELECT, 4, PAGE, COMMON, BANKUP, 3}, // ** Switch 04 - ENC #1 turn **

  // ******************************* PAGE 205: Current_patch_bank (3 buttons per page) *************************************************
  {PAGE_CURRENT_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', 'B', 'A' },
  {PAGE_CURRENT_PATCH_BANK, LABEL, 'N', 'K', ' ', ' ', ' ', ' ', ' ', ' ' },
  {PAGE_CURRENT_PATCH_BANK, 1 | ON_RELEASE, PATCH, CURRENT, BANKSELECT, 1, 3}, // ** Switch 01 **
  {PAGE_CURRENT_PATCH_BANK, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
  {PAGE_CURRENT_PATCH_BANK, 2 | ON_RELEASE, PATCH, CURRENT, BANKSELECT, 2, 3}, // ** Switch 02 **
  {PAGE_CURRENT_PATCH_BANK, 2 | ON_LONG_PRESS, SAVE_PATCH, CURRENT}, // ** Switch 02 long press
  {PAGE_CURRENT_PATCH_BANK, 3 | ON_RELEASE, PATCH, CURRENT, BANKSELECT, 3, 3}, // ** Switch 03 **
  {PAGE_CURRENT_PATCH_BANK, 3 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT},
  {PAGE_CURRENT_PATCH_BANK, 1 | ON_DUAL_PRESS, PATCH, CURRENT, BANKDOWN, 3}, // ** Switch 01 + 02 **
  {PAGE_CURRENT_PATCH_BANK, 2 | ON_DUAL_PRESS, PATCH, CURRENT, BANKUP, 3}, // ** Switch 02 + 03 **
  {PAGE_CURRENT_PATCH_BANK, 4, PATCH, CURRENT, BANKUP, 3}, // ** Switch 07 - ENC #1 turn **
  {PAGE_CURRENT_PATCH_BANK, 6, SET_TEMPO, COMMON, 120}, // ** Switch 06 - ENC #2 turn **

  // ******************************* PAGE 206: Page up/down tap *************************************************
  {PAGE_UP_DOWN_TAP, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', '+', '/' },
  {PAGE_UP_DOWN_TAP, LABEL, '-', ' ', '+', ' ', 'T', 'A', 'P', ' ' },
  {PAGE_UP_DOWN_TAP, 1 | ON_RELEASE, PATCH, CURRENT, PREV}, // ** Switch 01 **
  {PAGE_UP_DOWN_TAP, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
  {PAGE_UP_DOWN_TAP, 2 | ON_RELEASE, PATCH, CURRENT, NEXT}, // ** Switch 02 **
  {PAGE_UP_DOWN_TAP, 3 | ON_RELEASE, TAP_TEMPO, COMMON}, // ** Switch 03 **
  {PAGE_UP_DOWN_TAP, 3 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 03 - on long press **
  {PAGE_UP_DOWN_TAP, 4, PATCH, CURRENT, NEXT}, // ** Switch 04 - ENC #1 turn **
  {PAGE_UP_DOWN_TAP, 6, SET_TEMPO, COMMON, 120}, // ** Switch 06 - ENC #2 turn **

  // ******************************* PAGE 207: Parameters current device *************************************************
  {PAGE_CURRENT_PARAMETER, LABEL, 'P', 'A', 'R', ' ', 'B', 'A', 'N', 'K' },
  {PAGE_CURRENT_PARAMETER, 1 | ON_RELEASE, PAR_BANK, CURRENT, 1, 3}, // ** Switch 01 **
  {PAGE_CURRENT_PARAMETER, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
  {PAGE_CURRENT_PARAMETER, 2 | ON_RELEASE, PAR_BANK, CURRENT, 2, 3}, // ** Switch 02 **
  {PAGE_CURRENT_PARAMETER, 3 | ON_RELEASE, PAR_BANK, CURRENT, 3, 3}, // ** Switch 03 **
  {PAGE_CURRENT_PARAMETER, 3 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT},
  {PAGE_CURRENT_PARAMETER, 1 | ON_DUAL_PRESS, PAR_BANK_DOWN, CURRENT, 3}, // ** Switch 01 + 02
  {PAGE_CURRENT_PARAMETER, 2 | ON_DUAL_PRESS, PAR_BANK_UP, CURRENT, 3}, // ** Switch 02 + 03
  {PAGE_CURRENT_PARAMETER, 4, PAR_BANK_UP, CURRENT, 3}, // ** Switch 04 - ENC #1 turn **

  // ******************************* PAGE 208: Assigns current device *************************************************
  {PAGE_CURRENT_ASSIGN, LABEL, 'A', 'S', 'S', 'I', 'G', 'N', ' ', 'B' },
  {PAGE_CURRENT_ASSIGN, LABEL, 'A', 'N', 'K', ' ', 'S', 'E', 'L', ' ' },
  {PAGE_CURRENT_ASSIGN, 1 | ON_RELEASE, ASSIGN, CURRENT, BANKSELECT, 1, 3}, // ** Switch 01 **
  {PAGE_CURRENT_ASSIGN, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
  {PAGE_CURRENT_ASSIGN, 2 | ON_RELEASE, ASSIGN, CURRENT, BANKSELECT, 2, 3}, // ** Switch 02 **
  {PAGE_CURRENT_ASSIGN, 3 | ON_RELEASE, ASSIGN, CURRENT, BANKSELECT, 3, 3}, // ** Switch 03 **
  {PAGE_CURRENT_ASSIGN, 3 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT},
  {PAGE_CURRENT_ASSIGN, 1 | ON_DUAL_PRESS, ASSIGN, CURRENT, BANKDOWN, 3}, // ** Switch 01 + 02
  {PAGE_CURRENT_ASSIGN, 2 | ON_DUAL_PRESS, ASSIGN, CURRENT, BANKUP, 3}, // ** Switch 02 + 03
  {PAGE_CURRENT_ASSIGN, 4, ASSIGN, CURRENT, BANKUP, 3}, // ** Switch 04 - ENC #1 turn **

  // ******************************* PAGE 209: Snapshots (Helix) /scenes (AxeFX) *************************************************
  {PAGE_SNAPSCENE, LABEL, 'S', 'N', 'A', 'P', 'S', 'C', 'E', 'N' },
  {PAGE_SNAPSCENE, LABEL, 'E', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
  {PAGE_SNAPSCENE, 1 | ON_RELEASE, SNAPSCENE, CURRENT, 1, 5, 0}, // ** Switch 01 **
  {PAGE_SNAPSCENE, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
  {PAGE_SNAPSCENE, 2 | ON_RELEASE, SNAPSCENE, CURRENT, 2, 6, 7}, // ** Switch 02 **
  {PAGE_SNAPSCENE, 3 | ON_RELEASE, SNAPSCENE, CURRENT, 3, 4, 8}, // ** Switch 03 **
  {PAGE_SNAPSCENE, 3 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 03 - on long press **

  // ******************************* PAGE 210: Looper control *************************************************
  {PAGE_LOOPER, LABEL, 'L', 'O', 'O', 'P', 'E', 'R', ' ', 'C' },
  {PAGE_LOOPER, LABEL, 'O', 'N', 'T', 'R', 'O', 'L', ' ', ' ' },
  {PAGE_LOOPER, 1, LOOPER, CURRENT, LOOPER_REC_OVERDUB}, // ** Switch 01 **
  {PAGE_LOOPER, 2, LOOPER, CURRENT, LOOPER_PLAY_STOP}, // ** Switch 02 **
  {PAGE_LOOPER, 3 | ON_RELEASE, LOOPER, CURRENT, LOOPER_UNDO_REDO}, // ** Switch 03 **
  {PAGE_LOOPER, 3 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 03 - on long press **

  // ******************************* PAGE 211: KPA Looper control *************************************************
  {PAGE_KPA_LOOPER, LABEL, 'K', 'P', 'A', ' ', 'L', 'O', 'O', 'P' },
  {PAGE_KPA_LOOPER, LABEL, 'E', 'R', ' ', 'C', 'T', 'L', ' ', ' ' },
  {PAGE_KPA_LOOPER, 1, LOOPER, CURRENT, LOOPER_REC_PLAY_OVERDUB}, // ** Switch 01 **
  {PAGE_KPA_LOOPER, 2, LOOPER, CURRENT, LOOPER_STOP_ERASE}, // ** Switch 02 **
  {PAGE_KPA_LOOPER, 3 | ON_RELEASE, LOOPER, CURRENT, LOOPER_UNDO_REDO}, // ** Switch 03 **
  {PAGE_KPA_LOOPER, 3 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 03 - on long press **


  // ******************************* PAGE 212: KATANA FX CTRL #1  *************************************************
  {PAGE_KTN_FX1, LABEL, 'K', 'A', 'T', 'A', 'N', 'A', ' ', 'F'},
  {PAGE_KTN_FX1, LABEL, 'X', ' ', 'C', 'T', 'R', 'L', ' ', '1'},
  {PAGE_KTN_FX1, 1 | ON_RELEASE, PARAMETER, KTN, 8, TOGGLE, 1, 0}, // ** Switch 01 **
  {PAGE_KTN_FX1, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
  {PAGE_KTN_FX1, 1 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, KTN, PAGE_KTN_FX3}, // ** Switch 01 + 02 **
  {PAGE_KTN_FX1, 2 | ON_RELEASE, PARAMETER, KTN, 23, TOGGLE, 1, 0}, // ** Switch 02 **
  {PAGE_KTN_FX1, 2 | ON_LONG_PRESS, SAVE_PATCH, KTN}, // ** Switch 02 long press
  {PAGE_KTN_FX1, 2 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, KTN, PAGE_KTN_FX2}, // ** Switch 02 +| 03 **
  {PAGE_KTN_FX1, 3 | ON_RELEASE, PARAMETER, KTN, 70, TOGGLE, 1, 0}, // ** Switch 03 **
  {PAGE_KTN_FX1, 3 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, KTN, KTN_DEFAULT_PAGE1}, // ** Switch 03 - on long press **
  {PAGE_KTN_FX1, 4, PAR_BANK_UP, KTN, 1}, // ** Switch 04 - ENC #1 turn **
  {PAGE_KTN_FX1, 6, PAR_BANK, KTN, 1, 1}, // ** Switch 06 - ENC #2 turn **
  {PAGE_KTN_FX1, 7, OPEN_PAGE_DEVICE, KTN, KTN_DEFAULT_PAGE1}, // ** Switch 07 - ENC #2 press **

  // ******************************* PAGE 213: KATANA FX CTRL #2  *************************************************
  {PAGE_KTN_FX2, LABEL, 'K', 'A', 'T', 'A', 'N', 'A', ' ', 'F'},
  {PAGE_KTN_FX2, LABEL, 'X', ' ', 'C', 'T', 'R', 'L', ' ', '2'},
  {PAGE_KTN_FX2, 1 | ON_RELEASE, PARAMETER, KTN, 61, TOGGLE, 1, 0}, // ** Switch 01 **
  {PAGE_KTN_FX2, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
  {PAGE_KTN_FX2, 1 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, KTN, PAGE_KTN_FX1}, // ** Switch 01 + 02 **
  {PAGE_KTN_FX2, 2 | ON_RELEASE, PARAMETER, KTN, 87, TOGGLE, 1, 0}, // ** Switch 02 **
  {PAGE_KTN_FX2, 2 | ON_LONG_PRESS, SAVE_PATCH, KTN}, // ** Switch 02 long press
  {PAGE_KTN_FX2, 2 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, KTN, PAGE_KTN_FX3}, // ** Switch 02 +| 03 **
  {PAGE_KTN_FX2, 3, TAP_TEMPO, COMMON}, // ** Switch 03 **
  {PAGE_KTN_FX2, 3 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, KTN, KTN_DEFAULT_PAGE1}, // ** Switch 03 - on long press **
  {PAGE_KTN_FX2, 4, PAR_BANK_UP, KTN, 1}, // ** Switch 04 - ENC #1 turn **
  {PAGE_KTN_FX2, 6, PAR_BANK, KTN, 1, 1}, // ** Switch 06 - ENC #2 turn **
  {PAGE_KTN_FX2, 7, OPEN_PAGE_DEVICE, KTN, KTN_DEFAULT_PAGE1}, // ** Switch 07 - ENC #2 press **

  // ******************************* PAGE 214: KATANA FX CTRL #3  *************************************************
  {PAGE_KTN_FX3, LABEL, 'K', 'A', 'T', 'A', 'N', 'A', ' ', 'F'},
  {PAGE_KTN_FX3, LABEL, 'X', ' ', 'C', 'T', 'R', 'L', ' ', '3'},
  {PAGE_KTN_FX3, 1 | ON_RELEASE, PARAMETER, KTN, 96, TOGGLE, 1, 0}, // ** Switch 01 **
  {PAGE_KTN_FX3, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
  {PAGE_KTN_FX3, 1 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, KTN, PAGE_KTN_FX2}, // ** Switch 01 + 02 **
  {PAGE_KTN_FX3, 2 | ON_RELEASE, PARAMETER, KTN, 48, TOGGLE, 1, 0}, // ** Switch 02 **
  {PAGE_KTN_FX3, 2 | ON_LONG_PRESS, SAVE_PATCH, KTN}, // ** Switch 02 long press
  {PAGE_KTN_FX3, 2 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, KTN, PAGE_KTN_FX1}, // ** Switch 02 +| 03 **
  {PAGE_KTN_FX3, 3 | ON_RELEASE, PARAMETER, KTN, 106, TOGGLE, 1, 0}, // ** Switch 03 **
  {PAGE_KTN_FX3, 3 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, KTN, KTN_DEFAULT_PAGE1}, // ** Switch 03 - on long press **
  {PAGE_KTN_FX3, 4, PAR_BANK_UP, KTN, 1}, // ** Switch 04 - ENC #1 turn **
  {PAGE_KTN_FX3, 6, PAR_BANK, KTN, 1, 1}, // ** Switch 06 - ENC #2 turn **
  {PAGE_KTN_FX3, 7, OPEN_PAGE_DEVICE, KTN, KTN_DEFAULT_PAGE1}, // ** Switch 07 - ENC #2 press **

  // ******************************* PAGE 215: KATANA4 FX CTRL #1  *************************************************
  {PAGE_KTN4_FX1, LABEL, 'K', 'T', 'N', '_', 'V', '4', ' ', 'F'},
  {PAGE_KTN4_FX1, LABEL, 'X', ' ', 'C', 'T', 'R', 'L', ' ', '1'},
  {PAGE_KTN4_FX1, 1 | ON_RELEASE, PARAMETER, KTN, 0, TOGGLE, 1, 0}, // ** Switch 01 **
  {PAGE_KTN4_FX1, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
  {PAGE_KTN4_FX1, 1 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, KTN, PAGE_KTN4_FX3}, // ** Switch 01 + 02 **
  {PAGE_KTN4_FX1, 2 | ON_RELEASE, PARAMETER, KTN, 8, TOGGLE, 1, 0}, // ** Switch 02 **
  {PAGE_KTN4_FX1, 2 | ON_LONG_PRESS, SAVE_PATCH, KTN}, // ** Switch 02 long press
  {PAGE_KTN4_FX1, 2 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, KTN, PAGE_KTN4_FX2}, // ** Switch 02 +| 03 **
  {PAGE_KTN4_FX1, 3 | ON_RELEASE, PARAMETER, KTN, 23, TOGGLE, 1, 0}, // ** Switch 03 **
  {PAGE_KTN4_FX1, 3 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, KTN, KTN_DEFAULT_PAGE1}, // ** Switch 03 - on long press **
  {PAGE_KTN4_FX1, 4, PAR_BANK_UP, KTN, 1}, // ** Switch 04 - ENC #1 turn **
  {PAGE_KTN4_FX1, 6, PAR_BANK, KTN, 1, 1}, // ** Switch 06 - ENC #2 turn **
  {PAGE_KTN4_FX1, 7, OPEN_PAGE_DEVICE, KTN, KTN_DEFAULT_PAGE1}, // ** Switch 07 - ENC #2 press **

  // ******************************* PAGE 216: KATANA4 FX CTRL #2  *************************************************
  {PAGE_KTN4_FX2, LABEL, 'K', 'T', 'N', '_', 'V', '4', ' ', 'F'},
  {PAGE_KTN4_FX2, LABEL, 'X', ' ', 'C', 'T', 'R', 'L', ' ', '2'},
  {PAGE_KTN4_FX2, 1 | ON_RELEASE, PARAMETER, KTN, 61, TOGGLE, 1, 0}, // ** Switch 01 **
  {PAGE_KTN4_FX2, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
  {PAGE_KTN4_FX2, 1 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, KTN, PAGE_KTN4_FX1}, // ** Switch 01 + 02 **
  {PAGE_KTN4_FX2, 2 | ON_RELEASE, PARAMETER, KTN, 87, TOGGLE, 1, 0}, // ** Switch 02 **
  {PAGE_KTN4_FX2, 2 | ON_LONG_PRESS, SAVE_PATCH, KTN}, // ** Switch 02 long press
  {PAGE_KTN4_FX2, 2 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, KTN, PAGE_KTN4_FX3}, // ** Switch 02 +| 03 **
  {PAGE_KTN4_FX2, 3, TAP_TEMPO, COMMON}, // ** Switch 03 **
  {PAGE_KTN4_FX2, 3 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, KTN, KTN_DEFAULT_PAGE1}, // ** Switch 03 - on long press **
  {PAGE_KTN4_FX2, 4, PAR_BANK_UP, KTN, 1}, // ** Switch 04 - ENC #1 turn **
  {PAGE_KTN4_FX2, 6, PAR_BANK, KTN, 1, 1}, // ** Switch 06 - ENC #2 turn **
  {PAGE_KTN4_FX2, 7, OPEN_PAGE_DEVICE, KTN, KTN_DEFAULT_PAGE1}, // ** Switch 07 - ENC #2 press **

  // ******************************* PAGE 217: KATANA4 FX CTRL #3  *************************************************
  {PAGE_KTN4_FX3, LABEL, 'K', 'T', 'N', '_', 'V', '4', ' ', 'F'},
  {PAGE_KTN4_FX3, LABEL, 'X', ' ', 'C', 'T', 'R', 'L', ' ', '3'},
  {PAGE_KTN4_FX3, 1 | ON_RELEASE, PARAMETER, KTN, 70, TOGGLE, 1, 0}, // ** Switch 01 **
  {PAGE_KTN4_FX3, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
  {PAGE_KTN4_FX3, 1 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, KTN, PAGE_KTN4_FX2}, // ** Switch 01 + 02 **
  {PAGE_KTN4_FX3, 2 | ON_RELEASE, PARAMETER, KTN, 96, TOGGLE, 1, 0}, // ** Switch 02 **
  {PAGE_KTN4_FX3, 2 | ON_LONG_PRESS, SAVE_PATCH, KTN}, // ** Switch 02 long press
  {PAGE_KTN4_FX3, 2 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, KTN, PAGE_KTN4_FX1}, // ** Switch 02 +| 03 **
  {PAGE_KTN4_FX3, 3 | ON_RELEASE, PARAMETER, KTN, 106, TOGGLE, 1, 0}, // ** Switch 03 **
  {PAGE_KTN4_FX3, 3 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, KTN, KTN_DEFAULT_PAGE1}, // ** Switch 03 - on long press **
  {PAGE_KTN4_FX3, 4, PAR_BANK_UP, KTN, 1}, // ** Switch 04 - ENC #1 turn **
  {PAGE_KTN4_FX3, 6, PAR_BANK, KTN, 1, 1}, // ** Switch 06 - ENC #2 turn **
  {PAGE_KTN4_FX3, 7, OPEN_PAGE_DEVICE, KTN, KTN_DEFAULT_PAGE1}, // ** Switch 07 - ENC #2 press **

  // ******************************* PAGE 218: MG-300 FX CTRL #1  *************************************************
  {PAGE_MG300_FX1, LABEL, 'M', 'G', '-', '3', '0', '0', ' ', 'F'},
  {PAGE_MG300_FX1, LABEL, 'X', ' ', 'C', 'T', 'R', 'L', ' ', '1'},
  {PAGE_MG300_FX1, 1 | ON_RELEASE, PARAMETER, MG300, 1, TOGGLE, 1, 0}, // ** Switch 01 **
  {PAGE_MG300_FX1, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
  {PAGE_MG300_FX1, 1 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, MG300, PAGE_MG300_FX3}, // ** Switch 01 + 02 **
  {PAGE_MG300_FX1, 2 | ON_RELEASE, PARAMETER, MG300, 2, TOGGLE, 1, 0}, // ** Switch 02 **
  {PAGE_MG300_FX1, 2 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, MG300, PAGE_MG300_FX2}, // ** Switch 02 +| 03 **
  {PAGE_MG300_FX1, 3 | ON_RELEASE, PARAMETER, MG300, 6, TOGGLE, 1, 0}, // ** Switch 03 **
  {PAGE_MG300_FX1, 3 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, MG300, MG300_DEFAULT_PAGE1}, // ** Switch 03 - on long press **
  {PAGE_MG300_FX1, 4, PATCH, MG300, NEXT}, // ** Switch 04 - ENC #1 turn **
  {PAGE_MG300_FX1, 6, SET_TEMPO, COMMON, 120}, // ** Switch 06 - ENC #2 turn **
  {PAGE_MG300_FX1, 7, OPEN_PAGE_DEVICE, MG300, MG300_DEFAULT_PAGE1}, // ** Switch 07 - ENC #2 press **

  // ******************************* PAGE 219: MG-300 FX CTRL #2  *************************************************
  {PAGE_MG300_FX2, LABEL, 'M', 'G', '-', '3', '0', '0', ' ', 'F'},
  {PAGE_MG300_FX2, LABEL, 'X', ' ', 'C', 'T', 'R', 'L', ' ', '2'},
  {PAGE_MG300_FX2, 1 | ON_RELEASE, PARAMETER, MG300, 7, TOGGLE, 1, 0}, // ** Switch 01 **
  {PAGE_MG300_FX2, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
  {PAGE_MG300_FX2, 1 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, MG300, PAGE_MG300_FX1}, // ** Switch 01 + 02 **
  {PAGE_MG300_FX2, 2 | ON_RELEASE, PARAMETER, MG300, 8, TOGGLE, 1, 0}, // ** Switch 02 **
  {PAGE_MG300_FX2, 2 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, MG300, PAGE_MG300_FX3}, // ** Switch 02 +| 03 **
  {PAGE_MG300_FX2, 3, TAP_TEMPO, COMMON}, // ** Switch 03 **
  {PAGE_MG300_FX2, 3 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, MG300, MG300_DEFAULT_PAGE1}, // ** Switch 03 - on long press **
  {PAGE_MG300_FX2, 4, PATCH, MG300, NEXT}, // ** Switch 04 - ENC #1 turn **
  {PAGE_MG300_FX2, 6, SET_TEMPO, COMMON, 120}, // ** Switch 06 - ENC #2 turn **
  {PAGE_MG300_FX2, 7, OPEN_PAGE_DEVICE, MG300, MG300_DEFAULT_PAGE1}, // ** Switch 07 - ENC #2 press **

  // ******************************* PAGE 220: MG-300 FX CTRL #3  *************************************************
  {PAGE_MG300_FX3, LABEL, 'M', 'G', '-', '3', '0', '0', ' ', 'F'},
  {PAGE_MG300_FX3, LABEL, 'X', ' ', 'C', 'T', 'R', 'L', ' ', '3'},
  {PAGE_MG300_FX3, 1 | ON_RELEASE, PARAMETER, MG300, 3, TOGGLE, 1, 0}, // ** Switch 01 **
  {PAGE_MG300_FX3, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
  {PAGE_MG300_FX3, 1 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, MG300, PAGE_MG300_FX2}, // ** Switch 01 + 02 **
  {PAGE_MG300_FX3, 2 | ON_RELEASE, PARAMETER, MG300, 4, TOGGLE, 1, 0}, // ** Switch 02 **
  {PAGE_MG300_FX3, 2 | ON_DUAL_PRESS, OPEN_PAGE_DEVICE, MG300, PAGE_MG300_FX1}, // ** Switch 02 +| 03 **
  {PAGE_MG300_FX3, 3 | ON_RELEASE, PARAMETER, MG300, 5, TOGGLE, 1, 0}, // ** Switch 03 **
  {PAGE_MG300_FX3, 3 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, MG300, MG300_DEFAULT_PAGE1}, // ** Switch 03 - on long press **
  {PAGE_MG300_FX3, 4, PATCH, MG300, NEXT}, // ** Switch 04 - ENC #1 turn **
  {PAGE_MG300_FX3, 6, SET_TEMPO, COMMON, 120}, // ** Switch 06 - ENC #2 turn **
  {PAGE_MG300_FX3, 7, OPEN_PAGE_DEVICE, MG300, MG300_DEFAULT_PAGE1}, // ** Switch 07 - ENC #2 press **

  // ******************************* PAGE 221: SY1000 MODE SELECT  *************************************************
  {PAGE_SY1000_MODE_SEL1, LABEL, 'M', 'O', 'D', 'E', ' ', 'S', 'E', 'L'},
  {PAGE_SY1000_MODE_SEL1, LABEL, '1', ' ', 'S', 'Y', '1', '0', '0', '0'},
  {PAGE_SY1000_MODE_SEL1, 1 | LABEL, 'P', 'A', 'T', 'C', 'H', ' ', ' ', ' '},
  {PAGE_SY1000_MODE_SEL1, 1 | ON_RELEASE, PARAMETER, SY1000, 0, TOGGLE, 2, 2}, // ** Switch 01 **
  {PAGE_SY1000_MODE_SEL1, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
  {PAGE_SY1000_MODE_SEL1, 2 | LABEL, 'S', 'C', 'E', 'N', 'E', ' ', ' ', ' '},
  {PAGE_SY1000_MODE_SEL1, 2 | ON_RELEASE, PARAMETER, SY1000, 0, TOGGLE, 3, 4}, // ** Switch 02 **
  {PAGE_SY1000_MODE_SEL1, 2 | ON_LONG_PRESS, SAVE_PATCH, SY1000},
  {PAGE_SY1000_MODE_SEL1, 3 | LABEL, 'M', 'A', 'N', 'U', 'A', 'L', ' ', ' '},
  {PAGE_SY1000_MODE_SEL1, 3 | ON_RELEASE, PARAMETER, SY1000, 0, TOGGLE, 1, 0}, // ** Switch 03 **
  {PAGE_SY1000_MODE_SEL1, 3 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT},
  {PAGE_SY1000_MODE_SEL1, 4, PATCH, SY1000, NEXT}, // ** Switch 04 - ENC #1 turn **
  {PAGE_SY1000_MODE_SEL1, 6, SET_TEMPO, COMMON, 120}, // ** Switch 06 - ENC #2 turn **
  //{PAGE_SY1000_MODE_SEL1, 7, TAP_TEMPO, COMMON}, // ** Switch 07 - ENC #2 press **
  
  // ******************************* PAGE 221: SY1000 MODE SELECT (TOP ROW)  *************************************************
  {PAGE_SY1000_MODE_SEL2, LABEL, 'M', 'O', 'D', 'E', ' ', 'S', 'E', 'L'},
  {PAGE_SY1000_MODE_SEL2, LABEL, '2', ' ', 'S', 'Y', '1', '0', '0', '0'},
  {PAGE_SY1000_MODE_SEL2, 1 | LABEL, 'P', 'A', 'T', 'C', 'H', ' ', ' ', ' '},
  {PAGE_SY1000_MODE_SEL2, 1 | ON_RELEASE, PARAMETER, SY1000, 0, TOGGLE, 2, 2}, // ** Switch 01 **
  {PAGE_SY1000_MODE_SEL2, 1 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 01 long press
  {PAGE_SY1000_MODE_SEL2, 2 | LABEL, 'S', 'C', 'E', 'N', 'E', ' ', ' ', ' '},
  {PAGE_SY1000_MODE_SEL2, 2 | ON_RELEASE, PARAMETER, SY1000, 0, TOGGLE, 6, 4}, // ** Switch 02 **
  {PAGE_SY1000_MODE_SEL2, 2 | ON_LONG_PRESS, SAVE_PATCH, SY1000},
  {PAGE_SY1000_MODE_SEL2, 3 | LABEL, 'M', 'A', 'N', 'U', 'A', 'L', ' ', ' '},
  {PAGE_SY1000_MODE_SEL2, 3 | ON_RELEASE, PARAMETER, SY1000, 0, TOGGLE, 5, 0}, // ** Switch 03 **
  {PAGE_SY1000_MODE_SEL2, 3 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT},
  {PAGE_SY1000_MODE_SEL2, 4, PATCH, SY1000, NEXT}, // ** Switch 04 - ENC #1 turn **
  {PAGE_SY1000_MODE_SEL2, 6, SET_TEMPO, COMMON, 120}, // ** Switch 06 - ENC #2 turn **
  //{PAGE_SY1000_MODE_SEL2, 7, TAP_TEMPO, COMMON}, // ** Switch 07 - ENC #2 press **
  
};

const uint16_t NUMBER_OF_INTERNAL_COMMANDS = sizeof(Fixed_commands) / sizeof(Fixed_commands[0]);

// ********************************* Section 5: VC-mini default configuration for programmable pages ********************************************

// Default configuration of the switches of the user pages - this configuration will be restored when selecting menu - firmware menu - Init commands

// Every switch can have any number commands. Only the first command is shown in the display.

#define LOWEST_USER_PAGE 1 // Minimum value for PAGE DOWN

#define PAGE_DEFAULT 0
#define PAGE_GM_TEST 1
#define PAGE_COMBO1 2
#define PAGE_FUNC 3

const PROGMEM Cmd_struct Default_commands[] = {
  // ******************************* PAGE 00: Default page *************************************************
  // Page, Switch, Type, Device, Data1, Data2, Value1, Value2, Value3, Value4, Value5
  //{PAGE_DEFAULT, LABEL, 'D', 'E', 'F', 'A', 'U', 'L', 'T', ' ' },
  {PAGE_DEFAULT, 1, NOTHING, COMMON}, // ** Switch 01 **
  {PAGE_DEFAULT, 2, NOTHING, COMMON}, // ** Switch 02 **
  {PAGE_DEFAULT, 3, NOTHING, COMMON}, // ** Switch 03 **
  {PAGE_DEFAULT, 4, PAGE, COMMON, NEXT }, // ** ENC01 turn **
  {PAGE_DEFAULT, 5 | ON_RELEASE, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 05 - ENC #1 press **
  {PAGE_DEFAULT, 6, NOTHING, COMMON}, // ** ENC02 turn **
  {PAGE_DEFAULT, 7 | ON_RELEASE, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 07 - ENC #2 press **
  {PAGE_DEFAULT, 8, MASTER_EXP_PEDAL, CURRENT, 0}, // External switch 1 or expr pedal 1
  {PAGE_DEFAULT, 9, NOTHING, COMMON}, // External switch 2
  {PAGE_DEFAULT, 10, NOTHING, COMMON}, // MIDI switch 01
  {PAGE_DEFAULT, 11, NOTHING, COMMON}, // MIDI switch 02
  {PAGE_DEFAULT, 12, NOTHING, COMMON}, // MIDI switch 03
  {PAGE_DEFAULT, 13, NOTHING, COMMON}, // MIDI switch 04
  {PAGE_DEFAULT, 14, NOTHING, COMMON}, // MIDI switch 05
  {PAGE_DEFAULT, 15, NOTHING, COMMON}, // MIDI switch 06
  {PAGE_DEFAULT, 16, NOTHING, COMMON}, // MIDI switch 07
  {PAGE_DEFAULT, 17, NOTHING, COMMON}, // MIDI switch 08
  {PAGE_DEFAULT, 18, NOTHING, COMMON}, // MIDI switch 09
  {PAGE_DEFAULT, 19, NOTHING, COMMON}, // MIDI switch 10
  {PAGE_DEFAULT, 20, NOTHING, COMMON}, // MIDI switch 11
  {PAGE_DEFAULT, 21, NOTHING, COMMON}, // MIDI switch 12
  {PAGE_DEFAULT, 22, NOTHING, COMMON}, // MIDI switch 13
  {PAGE_DEFAULT, 23, NOTHING, COMMON}, // MIDI switch 14
  {PAGE_DEFAULT, 24, NOTHING, COMMON}, // MIDI switch 15

  // ******************************* PAGE 01: GM TEST *************************************************
  {PAGE_GM_TEST, LABEL, 'G', 'E', 'N', '.', 'M', 'I', 'D', 'I' },
  {PAGE_GM_TEST, LABEL, ' ', 'T', 'E', 'S', 'T', ' ', ' ', ' ' },
  {PAGE_GM_TEST, 1, MIDI_PC, COMMON, 1, 1, ALL_MIDI_PORTS}, // ** Switch 01 **
  {PAGE_GM_TEST, 1 | ON_LONG_PRESS, PAGE, COMMON, PREV },
  {PAGE_GM_TEST, 2, MIDI_PC, COMMON, 2, 1, ALL_MIDI_PORTS}, // ** Switch 02 **
  {PAGE_GM_TEST, 3, MIDI_NOTE, COMMON, 52, 100, 1, ALL_MIDI_PORTS}, // ** Switch 03 **
  {PAGE_GM_TEST, 3 | ON_LONG_PRESS, PAGE, COMMON, NEXT }, // Switch 03

  {PAGE_COMBO1, LABEL, 'G', 'P', 'V', 'G', 'G', 'R', ' ', ' ' },
  {PAGE_COMBO1, 1 | ON_RELEASE, PATCH, GP10, BANKSELECT, 1, 1}, // ** Switch 01 **
  {PAGE_COMBO1, 1 | ON_LONG_PRESS, PAGE, COMMON, PREV},
  {PAGE_COMBO1, 2 | ON_RELEASE, PATCH, GR55, BANKSELECT, 1, 1}, // ** Switch 02 **
  {PAGE_COMBO1, 3 | ON_RELEASE, PATCH, VG99, BANKSELECT, 1, 1}, // ** Switch 03 **
  {PAGE_COMBO1, 3 | ON_LONG_PRESS, PAGE, COMMON, NEXT},
  {PAGE_COMBO1, 1 | ON_DUAL_PRESS, PATCH, CURRENT, BANKDOWN, 1}, // ** Switch 1 + 2 **
  {PAGE_COMBO1, 2 | ON_DUAL_PRESS, PATCH, CURRENT, BANKUP, 1}, // ** Switch 1 + 2 **

  {PAGE_FUNC, 1 | ON_RELEASE, TOGGLE_EXP_PEDAL, CURRENT}, // ** Switch 1 **
  {PAGE_FUNC, 1 | ON_LONG_PRESS, PAGE, COMMON, PREV},
  {PAGE_FUNC, 2 | ON_RELEASE, TAP_TEMPO, COMMON}, // ** Switch 2 **
  {PAGE_FUNC, 2 | ON_LONG_PRESS, GLOBAL_TUNER, COMMON}, // ** Switch 2 **
  {PAGE_FUNC, 3 | ON_RELEASE, SET_TEMPO, COMMON, 120 },
  {PAGE_FUNC, 3 | ON_LONG_PRESS, PAGE, COMMON, NEXT},
};

const uint16_t NUMBER_OF_INIT_COMMANDS = sizeof(Default_commands) / sizeof(Default_commands[0]);

#define MENU_KEY_COMBINATION B01010000 // Both encoder switches pressed
#endif

// ********************************* Section 6: CUSTOM configuration for fixed command pages ********************************************
#ifdef CONFIG_CUSTOM

#define FIRST_FIXED_CMD_PAGE 201
#define FIRST_SELECTABLE_FIXED_CMD_PAGE 203
#define PAGE_MENU 201
#define PAGE_CURRENT_DIRECT_SELECT 202
#define PAGE_SELECT 203
#define PAGE_DEVICE_SELECT 204
#define PAGE_CURRENT_PATCH_BANK 205
#define PAGE_GR55_PATCH_BANK 206
#define PAGE_CURRENT_PARAMETER 207
#define PAGE_CURRENT_ASSIGN 208
#define PAGE_FULL_LOOPER 209
#define PAGE_SNAPSCENE 210
#define PAGE_KTN_PATCH_BANK 211
#define PAGE_KTN_FX1 212
#define PAGE_KTN_FX2 213
#define PAGE_KPA_LOOPER 214
#define LAST_FIXED_CMD_PAGE 214

#define DEFAULT_PAGE PAGE_SELECT // The page that gets selected when a valid page number is unknown

// Default pages for devices
#define FAS_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK
#define FAS_DEFAULT_PAGE2 PAGE_CURRENT_PARAMETER
#define FAS_DEFAULT_PAGE3 PAGE_SNAPSCENE
#define FAS_DEFAULT_PAGE4 0

#define GP10_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK
#define GP10_DEFAULT_PAGE2 PAGE_CURRENT_PARAMETER
#define GP10_DEFAULT_PAGE3 PAGE_CURRENT_ASSIGN
#define GP10_DEFAULT_PAGE4 0

#define GR55_DEFAULT_PAGE1 PAGE_GR55_PATCH_BANK
#define GR55_DEFAULT_PAGE2 PAGE_CURRENT_PARAMETER
#define GR55_DEFAULT_PAGE3 PAGE_CURRENT_ASSIGN
#define GR55_DEFAULT_PAGE4 0

#define HLX_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK
#define HLX_DEFAULT_PAGE2 PAGE_CURRENT_PARAMETER
#define HLX_DEFAULT_PAGE3 PAGE_SNAPSCENE
#define HLX_DEFAULT_PAGE4 0

#define KPA_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK
#define KPA_DEFAULT_PAGE2 PAGE_CURRENT_PARAMETER
#define KPA_DEFAULT_PAGE3 PAGE_KPA_LOOPER
#define KPA_DEFAULT_PAGE4 0

#define KTN_DEFAULT_PAGE1 PAGE_KTN_PATCH_BANK
#define KTN_DEFAULT_PAGE2 PAGE_KTN_FX1
#define KTN_DEFAULT_PAGE3 0
#define KTN_DEFAULT_PAGE4 0

#define M13_DEFAULT_PAGE1 PAGE_CURRENT_PARAMETER
#define M13_DEFAULT_PAGE2 PAGE_FULL_LOOPER
#define M13_DEFAULT_PAGE3 0
#define M13_DEFAULT_PAGE4 0

#define VG99_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK;
#define VG99_DEFAULT_PAGE2 PAGE_CURRENT_ASSIGN;
#define VG99_DEFAULT_PAGE3 0;
#define VG99_DEFAULT_PAGE4 0;

#define ZG3_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK
#define ZG3_DEFAULT_PAGE2 PAGE_CURRENT_PARAMETER
#define ZG3_DEFAULT_PAGE3 0
#define ZG3_DEFAULT_PAGE4 0

#define ZMS70_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK
#define ZMS70_DEFAULT_PAGE2 PAGE_CURRENT_PARAMETER
#define ZMS70_DEFAULT_PAGE3 0
#define ZMS70_DEFAULT_PAGE4 0

#define SVL_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK
#define SVL_DEFAULT_PAGE2 PAGE_CURRENT_PARAMETER
#define SVL_DEFAULT_PAGE3 0
#define SVL_DEFAULT_PAGE4 0

#define SY1000_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK
#define SY1000_DEFAULT_PAGE2 PAGE_CURRENT_PARAMETER
#define SY1000_DEFAULT_PAGE3 PAGE_CURRENT_ASSIGN
#define SY1000_DEFAULT_PAGE4 0

#define GM2_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK
#define GM2_DEFAULT_PAGE2 PAGE_CURRENT_PARAMETER
#define GM2_DEFAULT_PAGE3 0
#define GM2_DEFAULT_PAGE4 0

#define MG300_DEFAULT_PAGE1 PAGE_CURRENT_PATCH_BANK
#define MG300_DEFAULT_PAGE2 PAGE_CURRENT_PARAMETER
#define MG300_DEFAULT_PAGE3 0
#define MG300_DEFAULT_PAGE4 0

const PROGMEM Cmd_struct Fixed_commands[] = {
  // ******************************* PAGE 201: MENU *************************************************
  {PAGE_MENU, LABEL, 'M', 'E', 'N', 'U', ' ', ' ', ' ', ' ' },
  {PAGE_MENU, 1, MENU, COMMON, MENU_PREV}, // ** Switch 01 **
  {PAGE_MENU, 2, MENU, COMMON, MENU_NEXT}, // ** Switch 02 **
  {PAGE_MENU, 3, MENU, COMMON, MENU_SET_VALUE}, // ** Switch 03 *
  {PAGE_MENU, 4, MENU, COMMON, MENU_BACK}, // ** Switch 04 **
  {PAGE_MENU, 5, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 05 **
  {PAGE_MENU, 6, NOTHING, COMMON}, // ** Switch 06 **
  {PAGE_MENU, 7, MASTER_EXP_PEDAL, CURRENT, 0}, // External switch 1 or expr pedal 1
  {PAGE_MENU, 8, NOTHING, COMMON}, // External switch 2
  {PAGE_MENU, 9, MASTER_EXP_PEDAL, CURRENT, 0}, // External switch 3 or expr pedal 2
  {PAGE_MENU, 10, NOTHING, COMMON}, // External switch 4

  // ******************************* PAGE 202: Current device Direct Select *************************************************
  {PAGE_CURRENT_DIRECT_SELECT, LABEL, 'D', 'I', 'R', 'E', 'C', 'T', ' ', 'S' },
  {PAGE_CURRENT_DIRECT_SELECT, LABEL, 'E', 'L', 'E', 'C', 'T', ' ', ' ', ' ' },
  {PAGE_CURRENT_DIRECT_SELECT, 1, DIRECT_SELECT, CURRENT, 1, 10}, // ** Switch 01 **
  {PAGE_CURRENT_DIRECT_SELECT, 2, DIRECT_SELECT, CURRENT, 2, 10}, // ** Switch 02 **
  {PAGE_CURRENT_DIRECT_SELECT, 3, DIRECT_SELECT, CURRENT, 3, 10}, // ** Switch 03 **
  {PAGE_CURRENT_DIRECT_SELECT, 4, DIRECT_SELECT, CURRENT, 4, 10}, // ** Switch 04 **
  {PAGE_CURRENT_DIRECT_SELECT, 5, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 05 **
  {PAGE_CURRENT_DIRECT_SELECT, 6, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 06 **

  // ******************************* PAGE 203: PAGE USER SELECT *************************************************
  {PAGE_SELECT, LABEL, 'S', 'E', 'L', 'E', 'C', 'T', ' ', 'P' },
  {PAGE_SELECT, LABEL, 'A', 'G', 'E', ' ', ' ', ' ', ' ', ' ' },
  {PAGE_SELECT, 1, PAGE, COMMON, SELECT, PAGE_MENU}, // ** Switch 01 **
  {PAGE_SELECT, 2, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 02 **
  {PAGE_SELECT, 3, PAGE, COMMON, BANKSELECT, 1, 2}, // ** Switch 03 **
  {PAGE_SELECT, 4, PAGE, COMMON, BANKSELECT, 2, 2}, // ** Switch 04 **
  {PAGE_SELECT, 5, PAGE, COMMON, BANKDOWN, 4}, // ** Switch 05 **
  {PAGE_SELECT, 6, PAGE, COMMON, BANKUP, 4}, // ** Switch 06 **

  // ******************************* PAGE 204: DEVICE PAGE SELECT *************************************************
  {PAGE_DEVICE_SELECT, LABEL, 'S', 'E', 'L', ' ', 'D', 'E', 'V', 'I' },
  {PAGE_DEVICE_SELECT, LABEL, 'C', 'E', ' ', 'P', 'A', 'G', 'E', ' ' },
  {PAGE_DEVICE_SELECT, 1, OPEN_NEXT_PAGE_OF_DEVICE, KTN}, // ** Switch 01 **
  {PAGE_DEVICE_SELECT, 2, OPEN_NEXT_PAGE_OF_DEVICE, GR55}, // ** Switch 02 **
  {PAGE_DEVICE_SELECT, 3, PAGE, COMMON, SELECT, PAGE_SELECT}, // ** Switch 03 **
  {PAGE_DEVICE_SELECT, 4, PAGE, COMMON, SELECT, PAGE_MENU}, // ** Switch 04 **

  // ******************************* PAGE 205: Current_patch_bank *************************************************
  {PAGE_CURRENT_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', 'B', 'A' },
  {PAGE_CURRENT_PATCH_BANK, LABEL, 'N', 'K', ' ', ' ', ' ', ' ', ' ', ' ' },
  {PAGE_CURRENT_PATCH_BANK, 1, PATCH, CURRENT, BANKSELECT, 1, 4}, // ** Switch 01 **
  {PAGE_CURRENT_PATCH_BANK, 2, PATCH, CURRENT, BANKSELECT, 2, 4}, // ** Switch 02 **
  {PAGE_CURRENT_PATCH_BANK, 3, PATCH, CURRENT, BANKSELECT, 3, 4}, // ** Switch 03 **
  {PAGE_CURRENT_PATCH_BANK, 4, PATCH, CURRENT, BANKSELECT, 4, 4}, // ** Switch 04 **
  {PAGE_CURRENT_PATCH_BANK, 5, PATCH, CURRENT, BANKDOWN, 4}, // ** Switch 05 **
  {PAGE_CURRENT_PATCH_BANK, 6, PATCH, CURRENT, BANKUP, 4}, // ** Switch 06 **
  {PAGE_CURRENT_PATCH_BANK, 5 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 5 **
  {PAGE_CURRENT_PATCH_BANK, 6 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 6 **

  // ******************************* PAGE 206: GR55 select *************************************************
  {PAGE_GR55_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', 'B', 'A' },
  {PAGE_GR55_PATCH_BANK, LABEL, 'N', 'K', ' ', 'G', 'R', ' ', ' ', ' ' },
  {PAGE_GR55_PATCH_BANK, 1, PATCH, GR55, BANKSELECT, 1, 3}, // ** Switch 01 **
  {PAGE_GR55_PATCH_BANK, 2, PATCH, GR55, BANKSELECT, 2, 3}, // ** Switch 02 **
  {PAGE_GR55_PATCH_BANK, 3, PATCH, GR55, BANKSELECT, 3, 3}, // ** Switch 03 **
  {PAGE_GR55_PATCH_BANK, 4, PARAMETER, GR55, 33, TOGGLE, 1, 0}, // ** Switch 04 **
  {PAGE_GR55_PATCH_BANK, 5, PATCH, GR55, BANKDOWN, 3}, // ** Switch 05 **
  {PAGE_GR55_PATCH_BANK, 6, PATCH, GR55, BANKUP, 3}, // ** Switch 06 **
  {PAGE_GR55_PATCH_BANK, 5 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 5 **
  {PAGE_GR55_PATCH_BANK, 6 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 6 **

  // ******************************* PAGE 207: Parameters current device *************************************************
  {PAGE_CURRENT_PARAMETER, LABEL, 'P', 'A', 'R', ' ', 'B', 'A', 'N', 'K' },
  {PAGE_CURRENT_PARAMETER, 1, PAR_BANK, CURRENT, 1, 4}, // ** Switch 01 **
  {PAGE_CURRENT_PARAMETER, 2, PAR_BANK, CURRENT, 2, 4}, // ** Switch 02 **
  {PAGE_CURRENT_PARAMETER, 3, PAR_BANK, CURRENT, 3, 4}, // ** Switch 03 **
  {PAGE_CURRENT_PARAMETER, 4, PAR_BANK, CURRENT, 4, 4}, // ** Switch 04 **
  {PAGE_CURRENT_PARAMETER, 5, PAR_BANK_DOWN, CURRENT, 4}, // ** Switch 05 **
  {PAGE_CURRENT_PARAMETER, 6, PAR_BANK_UP, CURRENT, 4}, // ** Switch 06 **
  {PAGE_CURRENT_PARAMETER, 5 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 5 **
  {PAGE_CURRENT_PARAMETER, 6 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 6 **

  // ******************************* PAGE 208: Assigns current device *************************************************
  {PAGE_CURRENT_ASSIGN, LABEL, 'A', 'S', 'S', 'I', 'G', 'N', ' ', 'B' },
  {PAGE_CURRENT_ASSIGN, LABEL, 'A', 'N', 'K', ' ', 'S', 'E', 'L', ' ' },
  {PAGE_CURRENT_ASSIGN, 1, ASSIGN, CURRENT, BANKSELECT, 1, 4}, // ** Switch 01 **
  {PAGE_CURRENT_ASSIGN, 2, ASSIGN, CURRENT, BANKSELECT, 2, 4}, // ** Switch 02 **
  {PAGE_CURRENT_ASSIGN, 3, ASSIGN, CURRENT, BANKSELECT, 3, 4}, // ** Switch 03 **
  {PAGE_CURRENT_ASSIGN, 4, ASSIGN, CURRENT, BANKSELECT, 3, 4}, // ** Switch 03 **
  {PAGE_CURRENT_ASSIGN, 5, ASSIGN, CURRENT, BANKDOWN, 4}, // ** Switch 05
  {PAGE_CURRENT_ASSIGN, 6, ASSIGN, CURRENT, BANKUP, 4}, // ** Switch 06
  {PAGE_CURRENT_ASSIGN, 5 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 5 **
  {PAGE_CURRENT_ASSIGN, 6 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 6 **

  // ******************************* PAGE 209: Looper control *************************************************
  {PAGE_FULL_LOOPER, LABEL, 'L', 'O', 'O', 'P', 'E', 'R', ' ', 'C' },
  {PAGE_FULL_LOOPER, LABEL, 'O', 'N', 'T', 'R', 'O', 'L', ' ', ' ' },
  {PAGE_FULL_LOOPER, 1, LOOPER, CURRENT, LOOPER_REC_OVERDUB}, // ** Switch 01 **
  {PAGE_FULL_LOOPER, 2, LOOPER, CURRENT, LOOPER_PLAY_STOP}, // ** Switch 02 **
  {PAGE_FULL_LOOPER, 3, LOOPER, CURRENT, LOOPER_HALF_SPEED}, // ** Switch 03 **
  {PAGE_FULL_LOOPER, 4, LOOPER, CURRENT, LOOPER_UNDO_REDO}, // ** Switch 04 **
  {PAGE_FULL_LOOPER, 5, PATCH, CURRENT, 3, PREV}, // ** Switch 05 **
  {PAGE_FULL_LOOPER, 6, PATCH, CURRENT, 3, NEXT}, // ** Switch 06 **
  {PAGE_FULL_LOOPER, 5 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 5 **
  {PAGE_FULL_LOOPER, 6 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 6 **

  // ******************************* PAGE 210: Snapshots (Helix) /scenes (AxeFX) *************************************************
  {PAGE_SNAPSCENE, LABEL, 'S', 'N', 'A', 'P', 'S', 'C', 'E', 'N' },
  {PAGE_SNAPSCENE, LABEL, 'E', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
  {PAGE_SNAPSCENE, 1, SNAPSCENE, CURRENT, 1, 5, 0}, // ** Switch 01 **
  {PAGE_SNAPSCENE, 2, SNAPSCENE, CURRENT, 2, 6, 0}, // ** Switch 02 **
  {PAGE_SNAPSCENE, 3, SNAPSCENE, CURRENT, 3, 7, 0}, // ** Switch 03 **
  {PAGE_SNAPSCENE, 4, SNAPSCENE, CURRENT, 4, 8, 0}, // ** Switch 04 **
  {PAGE_SNAPSCENE, 5, PATCH, CURRENT, PREV}, // ** Switch 05 **
  {PAGE_SNAPSCENE, 6, PATCH, CURRENT, NEXT}, // ** Switch 06 **
  {PAGE_SNAPSCENE, 5 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 5 **
  {PAGE_SNAPSCENE, 6 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 6 **

  // ******************************* PAGE 211: Katana patch bank *************************************************
  {PAGE_KTN_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', 'B', 'A' },
  {PAGE_KTN_PATCH_BANK, LABEL, 'N', 'K', ' ', 'K', 'T', 'N', ' ', ' ' },
  {PAGE_KTN_PATCH_BANK, 1 | ON_RELEASE, PATCH, CURRENT, BANKSELECT, 2, 4}, // ** Switch 01 **
  {PAGE_KTN_PATCH_BANK, 1 | ON_LONG_PRESS, SAVE_PATCH, KTN }, // ** Switch 01 (HOLD) **
  {PAGE_KTN_PATCH_BANK, 2 | ON_RELEASE, PATCH, CURRENT, BANKSELECT, 3, 4}, // ** Switch 02 **
  {PAGE_KTN_PATCH_BANK, 2 | ON_LONG_PRESS, SAVE_PATCH, KTN }, // ** Switch 02 (HOLD) **
  {PAGE_KTN_PATCH_BANK, 3 | ON_RELEASE, PATCH, CURRENT, BANKSELECT, 4, 4}, // ** Switch 03 **
  {PAGE_KTN_PATCH_BANK, 3 | ON_LONG_PRESS, SAVE_PATCH, KTN }, // ** Switch 03 (HOLD) **
  {PAGE_KTN_PATCH_BANK, 4 | ON_RELEASE, PATCH, CURRENT, BANKSELECT, 5, 4}, // ** Switch 04 **
  {PAGE_KTN_PATCH_BANK, 4 | ON_LONG_PRESS, SAVE_PATCH, KTN }, // ** Switch 04 (HOLD) **
  {PAGE_KTN_PATCH_BANK, 5, PATCH, CURRENT, BANKDOWN, 4}, // ** Switch 05 **
  {PAGE_KTN_PATCH_BANK, 6, PATCH, CURRENT, BANKUP, 4}, // ** Switch 06 **
  {PAGE_KTN_PATCH_BANK, 5 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 5 **
  {PAGE_KTN_PATCH_BANK, 6 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 6 **

  // ******************************* PAGE 212: KATANA FX1 CTRL  *************************************************
  {PAGE_KTN_FX1, LABEL, 'K', 'A', 'T', 'A', 'N', 'A', ' ', 'F'},
  {PAGE_KTN_FX1, LABEL, 'X', ' ', 'C', 'T', 'R', 'L', ' ', '1'},
  {PAGE_KTN_FX1, 1, PARAMETER, KTN, 0, TOGGLE, 1, 0}, // ** Switch 01 **
  {PAGE_KTN_FX1, 2, PARAMETER, KTN, 8, TOGGLE, 1, 0}, // ** Switch 02 **
  {PAGE_KTN_FX1, 3, PARAMETER, KTN, 23, TOGGLE, 1, 0}, // ** Switch 03 **
  {PAGE_KTN_FX1, 4, PARAMETER, KTN, 70, TOGGLE, 1, 0}, // ** Switch 04 **
  {PAGE_KTN_FX1, 5, OPEN_PAGE_DEVICE, KTN, PAGE_KTN_PATCH_BANK}, // ** Switch 5 **
  {PAGE_KTN_FX1, 6, OPEN_PAGE_DEVICE, KTN, PAGE_KTN_FX2}, // ** Switch 6 **

  // ******************************* PAGE 213: KATANA FX2 CTRL  *************************************************
  {PAGE_KTN_FX2, LABEL, 'K', 'A', 'T', 'A', 'N', 'A', ' ', 'F'},
  {PAGE_KTN_FX2, LABEL, 'X', ' ', 'C', 'T', 'R', 'L', ' ', '2'},
  {PAGE_KTN_FX2, 1, PARAMETER, KTN, 61, TOGGLE, 1, 0}, // ** Switch 01 **
  {PAGE_KTN_FX2, 2, PARAMETER, KTN, 87, TOGGLE, 1, 0}, // ** Switch 02 **
  {PAGE_KTN_FX2, 3, PARAMETER, KTN, 96, TOGGLE, 1, 0}, // ** Switch 03 **
  {PAGE_KTN_FX2, 4, TAP_TEMPO, COMMON}, // ** Switch 04 **
  {PAGE_KTN_FX2, 5, OPEN_PAGE_DEVICE, KTN, PAGE_KTN_PATCH_BANK}, // ** Switch 5 **
  {PAGE_KTN_FX2, 6, OPEN_PAGE_DEVICE, KTN, PAGE_KTN_FX1}, // ** Switch 6 **

  // ******************************* PAGE 214: KPA Looper control *************************************************
  {PAGE_KPA_LOOPER, LABEL, 'K', 'P', 'A', ' ', 'L', 'O', 'O', 'P' },
  {PAGE_KPA_LOOPER, LABEL, 'E', 'R', ' ', 'C', 'T', 'L', ' ', ' ' },
  {PAGE_KPA_LOOPER, 1, LOOPER, CURRENT, LOOPER_REC_PLAY_OVERDUB}, // ** Switch 01 **
  {PAGE_KPA_LOOPER, 2, LOOPER, CURRENT, LOOPER_STOP_ERASE}, // ** Switch 02 **
  {PAGE_KPA_LOOPER, 3, LOOPER, CURRENT, LOOPER_UNDO_REDO}, // ** Switch 03 **
  {PAGE_KPA_LOOPER, 4, LOOPER, CURRENT, LOOPER_HALF_SPEED}, // ** Switch 04 **
  {PAGE_KPA_LOOPER, 5, PATCH, CURRENT, PREV, 3}, // ** Switch 05 **
  {PAGE_KPA_LOOPER, 6, PATCH, CURRENT, NEXT, 3}, // ** Switch 06 **
  {PAGE_KPA_LOOPER, 5 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 5 **
  {PAGE_KPA_LOOPER, 6 | ON_LONG_PRESS, OPEN_PAGE_DEVICE, KTN, PAGE_KTN_FX1}, // ** Switch 6 **
};

const uint16_t NUMBER_OF_INTERNAL_COMMANDS = sizeof(Fixed_commands) / sizeof(Fixed_commands[0]);

// ********************************* Section 7: CUSTOM default configuration for programmable pages ********************************************

// Default configuration of the switches of the user pages - this configuration will be restored when selecting menu - firmware menu - Init commands

// Every switch can have any number commands. Only the first command is shown in the display.

#define LOWEST_USER_PAGE 1 // Minimum value for PAGE DOWN

#define PAGE_DEFAULT 0
#define PAGE_COMBO1 1
#define PAGE_COMBO2 2
#define PAGE_FUNCTIONS_TEST 3
#define PAGE_GM_TEST 4

const PROGMEM Cmd_struct Default_commands[] = {
  // ******************************* PAGE 00: Default page *************************************************
  // Page, Switch, Type, Device, Data1, Data2, Value1, Value2, Value3, Value4, Value5
  //{PAGE_DEFAULT, LABEL, 'D', 'E', 'F', 'A', 'U', 'L', 'T', ' ' },
  {PAGE_DEFAULT, 1, NOTHING, COMMON}, // ** Switch 01 **
  {PAGE_DEFAULT, 2, NOTHING, COMMON}, // ** Switch 02 **
  {PAGE_DEFAULT, 3, NOTHING, COMMON}, // ** Switch 03 **
  {PAGE_DEFAULT, 4, NOTHING, COMMON}, // ** Switch 04 **
  {PAGE_DEFAULT, 5, NOTHING, COMMON}, // ** Switch 05 **
  {PAGE_DEFAULT, 6, NOTHING, COMMON}, // ** Switch 06 **
  {PAGE_DEFAULT, 7, NOTHING, COMMON}, // ** Switch 07 **
  {PAGE_DEFAULT, 8, NOTHING, COMMON}, // ** Switch 08 **
  {PAGE_DEFAULT, 9, NOTHING, COMMON}, // ** Switch 09 **
  {PAGE_DEFAULT, 10, NOTHING, COMMON}, // ** Switch 10 **
  {PAGE_DEFAULT, 5 | ON_LONG_PRESS, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 5 **
  {PAGE_DEFAULT, 6 | ON_LONG_PRESS, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 6 **

  // ******************************* PAGE 01: GP+GR *************************************************
  {PAGE_COMBO1, LABEL, 'G', 'P', '+', 'G', 'R', ' ', ' ', ' ' },
  {PAGE_COMBO1, 1, PATCH, GP10, BANKSELECT, 1, 4}, // ** Switch 01 **
  {PAGE_COMBO1, 2, PATCH, GP10, BANKSELECT, 2, 4}, // ** Switch 02 **
  {PAGE_COMBO1, 3, PATCH, GP10, BANKSELECT, 3, 4}, // ** Switch 03 **
  {PAGE_COMBO1, 4, PATCH, GP10, BANKSELECT, 4, 4}, // ** Switch 04 **

  // ******************************* PAGE 02: GPVGGR *************************************************
  {PAGE_COMBO2, LABEL, 'G', 'P', 'V', 'G', 'G', 'R', ' ', ' ' },
  {PAGE_COMBO2, 1, PATCH, GP10, BANKSELECT, 1, 4}, // ** Switch 01 **
  {PAGE_COMBO2, 2, PATCH, GP10, BANKSELECT, 2, 4}, // ** Switch 02 **
  {PAGE_COMBO2, 3, PATCH, GP10, BANKSELECT, 3, 4}, // ** Switch 03 **
  {PAGE_COMBO2, 4, PATCH, GP10, BANKSELECT, 4, 4}, // ** Switch 04 **

  // ******************************* PAGE 03: FUNCTION TEST *************************************************
  {PAGE_FUNCTIONS_TEST, LABEL, 'F', 'U', 'N', 'C', 'T', 'I', 'O', 'N' },
  {PAGE_FUNCTIONS_TEST, LABEL, ' ', 'T', 'E', 'S', 'T', ' ', ' ', ' ' },
  {PAGE_FUNCTIONS_TEST, 1, PATCH, CURRENT, SELECT, 1, 0}, // ** Switch 01 **
  {PAGE_FUNCTIONS_TEST, 2, PATCH, CURRENT, BANKDOWN, 1}, // ** Switch 02 **
  {PAGE_FUNCTIONS_TEST, 3, PATCH, CURRENT, BANKSELECT, 1, 1}, // ** Switch 03 **
  {PAGE_FUNCTIONS_TEST, 4, PATCH, CURRENT, BANKUP, 1}, // ** Switch 04 **

  // ******************************* PAGE 04: GM TEST *************************************************
  {PAGE_GM_TEST, LABEL, 'G', 'E', 'N', '.', 'M', 'I', 'D', 'I' },
  {PAGE_GM_TEST, LABEL, ' ', 'T', 'E', 'S', 'T', ' ', ' ', ' ' },
  {PAGE_GM_TEST, 1, MIDI_PC, COMMON, 1, 1, ALL_MIDI_PORTS}, // ** Switch 01 **
  {PAGE_GM_TEST, 2, MIDI_PC, COMMON, 2, 1, ALL_MIDI_PORTS}, // ** Switch 02 **
  {PAGE_GM_TEST, 3, MIDI_PC, COMMON, 3, 1, ALL_MIDI_PORTS}, // ** Switch 03 **
  {PAGE_GM_TEST, 3 | LABEL, 'C', 'U', 'S', 'T', 'O', 'M', ' ', 'L'}, // ** Switch 03 **
  {PAGE_GM_TEST, 3 | LABEL, 'A', 'B', 'E', 'L', ' ', 'P', 'C', '3'}, // ** Switch 03 **
  {PAGE_GM_TEST, 4, MIDI_NOTE, COMMON, 52, 100, 1, ALL_MIDI_PORTS}, // ** Switch 04 **
};

const uint16_t NUMBER_OF_INIT_COMMANDS = sizeof(Default_commands) / sizeof(Default_commands[0]);

#define MENU_KEY_COMBINATION 0x09 // Switches 1 and 4
#endif
