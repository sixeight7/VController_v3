// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: VController commands
// Section 2: VController configuration for fixed command pages
// Section 3: VController default configuration for programmable pages

// ********************************* Section 1: VController commands ********************************************

// Here we define the supported devices
#define NUMBER_OF_DEVICES 9
#define GP10 0
#define GR55 1
#define VG99 2
#define ZG3 3
#define ZMS70 4
#define M13 5
#define HLX 6
#define AXEFX 7
#define KTN 8

#define CURRENT 254 // To select the current device
#define COMMON 255 // Not really a device, but used for common procedures, that are not device specific or for all devices.

//Possible switch types:
#define LABEL 0x80 // Bit 8 of the switch number is set to one all labels - this saved one byte and keeps the command structure short.
                            // Example:  {<PAGE>, <SWITCH> | LABEL, 'P', 'A', 'T', 'C', 'H', ' ', ' ', ' ' } 
// Common command types
#define NUMBER_OF_COMMON_TYPES 11 // Only the one that can be selected in the command builder
#define NOTHING 0           // Example: {<PAGE>, <SWITCH>, NOTHING, COMMON}
#define OPEN_PAGE 1       // Example: {<PAGE>, <SWITCH>, OPEN_PAGE, COMMON, PAGE_COMBO1}
#define PAGE_UP 2           // Example: {<PAGE>, <SWITCH>, PAGE_UP, COMMON, 1}
#define PAGE_DOWN 3         // Example: {<PAGE>, <SWITCH>, PAGE_DOWN, COMMON, 1}
#define TAP_TEMPO 4         // Example: {<PAGE>, <SWITCH>, TAP_TEMPO, COMMON}
#define SET_TEMPO 5         // Example: {<PAGE>, <SWITCH>, SET_TEMPO, COMMON, 120}
#define GLOBAL_TUNER 6      // Example: {<PAGE>, <SWITCH>, GLOBAL_TUNER, COMMON}
#define MIDI_PC 7           // Example: {<PAGE>, <SWITCH>, MIDI_PC, COMMON, Program, Channel, Port}
#define MIDI_CC 8           // Example: {<PAGE>, <SWITCH>, MIDI_CC, COMMON, Controller, CC_TOGGLE_TYPE, Value1, Value2, Channel, Port}
#define MIDI_NOTE 9         // Example: {<PAGE>, <SWITCH>, MIDI_NOTE, COMMON, Note, Velocity, Channel, Port}
#define SELECT_NEXT_DEVICE 10// Example: {<PAGE>, <SWITCH>, SELECT_NEXT_DEVICE, COMMON}

// Common functions that can not be selected in the command builder
#define NUMBER_OF_COMMON_TYPES_NOT_SELECTABLE 1
#define MENU 11             // Example: {<PAGE>, <SWITCH>, MENU, COMMON, 1} - to display first menu item

// Device command types
#define NUMBER_OF_DEVICE_TYPES 15
#define PATCH_SEL 100      // Example: {<PAGE>, <SWITCH>, PATCH_SEL, <DEVICE>, 25, 1} - to select patch 125
#define PARAMETER 101      // Example: {<PAGE>, <SWITCH>, PARAMETER, <DEVICE>, 1, TOGGLE, 1, 0}
#define ASSIGN 102         // Example: {<PAGE>, <SWITCH>, ASSIGN, <DEVICE>, 1, 21}
#define PATCH_BANK 103     // Example: {<PAGE>, <SWITCH>, PATCH_BANK, <DEVICE>, 1, 10}
#define BANK_UP 104        // Example: {<PAGE>, <SWITCH>, BANK_UP, <DEVICE>, 10, PAGE_CURRENT_DIRECT_SELECT}
#define BANK_DOWN 105      // Example: {<PAGE>, <SWITCH>, BANK_DOWN, <DEVICE>, 10, PAGE_CURRENT_DIRECT_SELECT}
#define NEXT_PATCH 106     // Example: {<PAGE>, <SWITCH>, NEXT_PATCH, <DEVICE>, PAGE_CURRENT_DIRECT_SELECT}
#define PREV_PATCH 107     // Example: {<PAGE>, <SWITCH>, PREV_PATCH, <DEVICE>, PAGE_CURRENT_DIRECT_SELECT}
#define MUTE 108           // Example: {<PAGE>, <SWITCH>, MUTE, <DEVICE>} 
#define OPEN_PAGE_DEVICE 109     // Example: {<PAGE>, <SWITCH>, OPEN_PAGE_DEVICE, ZG3, PAGE_ZOOM_PATCH_BANK}
#define OPEN_NEXT_PAGE_OF_DEVICE 110      // Example: {<PAGE>, <SWITCH>, OPEN_NEXT_PAGE_OF_DEVICE, <DEVICE>}
#define TOGGLE_EXP_PEDAL 111 // Example: {<PAGE>, <SWITCH>, TOGGLE_EXP_PEDAL, <DEVICE>}
#define SNAPSCENE 112      // Example: {<PAGE>, <SWITCH>, SNAPSCENE, <DEVICE>, 1}
#define LOOPER 113         // Example: {<PAGE>, <SWITCH>, LOOPER, <DEVICE>, START}
#define MASTER_EXP_PEDAL 114 // Example: {<PAGE>, <SWITCH>, MASTER_EXP_PEDAL, <DEVICE>, <number>} number of expression pedal: 0, 1, 2 or 3. If 0, it will choose the number as selected with TOGGLE_EXP_PEDAL

//Common device types that can not be selected in the command builder
#define NUMBER_OF_DEVICE_TYPES_NOT_SELECTABLE 6
#define DIRECT_SELECT 115  // Example: {<PAGE>, <SWITCH>, DIRECT_SELECT, <DEVICE>, 1, 10}
#define PAR_BANK 116       // Example: {<PAGE>, <SWITCH>, PAR_BANK, <DEVICE>, 1, 10}
#define PAR_BANK_UP 117    // Example: {<PAGE>, <SWITCH>, PAR_BANK_UP, <DEVICE>, 10}
#define PAR_BANK_DOWN 118  // Example: {<PAGE>, <SWITCH>, PAR_BANK_DOWN, <DEVICE>, 10}
#define PAR_BANK_CATEGORY 119   // Example: {<PAGE>, <SWITCH>, PAR_BANK_CATEGORY, <DEVICE>, 1}
#define SAVE_PATCH 120    // Example: {<PAGE>, <SWITCH>, TOGGLE_EXP_PEDAL, <DEVICE>}

// Toggle types
#define MOMENTARY 0
#define TOGGLE 1
#define TRISTATE 2
#define FOURSTATE 3
#define STEP 4 // Set minimum ,maximum and step value
#define RANGE 5 // For use with expression pedal
#define UPDOWN 6 // Press and hold to increase/decrease. Press shortly to change direction
#define TGL_OFF 255 // To show nothing

// Special max values:
#define TIME_2000 255 // For delay times up to 2000 ms
#define TIME_1000 254 // For delay times up to 1000 ms
#define TIME_500 253 // For pre delay times up to 500 ms
#define TIME_300 252 // For pre delay times up to 300 ms

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

// Ports you can use - because they follow binary rules, you can also send commands to combinations of ports:
// To send to MIDI port 1, 2 and 3 - specify port as 1 + 2 + 4 = 7!
#define NUMBER_OF_MIDI_PORTS 4
#define USBMIDI_PORT 0x00
#define MIDI1_PORT 0x10
#define MIDI2_PORT 0x20
#define MIDI3_PORT 0x30
#define ALL_PORTS 0xF0

// VG99 commands
#define FC300_CTL1 ASSIGN, VG99, 16, 1
#define FC300_CTL2 ASSIGN, VG99, 17, 2
#define FC300_CTL3 ASSIGN, VG99, 18, 3
#define FC300_CTL4 ASSIGN, VG99, 19, 4
#define FC300_CTL5 ASSIGN, VG99, 20, 5
#define FC300_CTL6 ASSIGN, VG99, 21, 6
#define FC300_CTL7 ASSIGN, VG99, 22, 7
#define FC300_CTL8 ASSIGN, VG99, 23, 8
#define FC300_EXP1 ASSIGN, VG99, 24, 9
#define FC300_EXP_SW1 ASSIGN, VG99, 25, 10
#define FC300_EXP2 ASSIGN, VG99, 26, 11
#define FC300_EXP_SW2 ASSIGN, VG99, 27, 12

// ********************************* Section 2: VController configuration for fixed command pages ********************************************

#define FIRST_FIXED_CMD_PAGE 201
#define PAGE_MENU 201
#define PAGE_CURRENT_DIRECT_SELECT 202
#define PAGE_USER_SELECT 203
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
#define LAST_FIXED_CMD_PAGE 221

#define DEFAULT_PAGE PAGE_USER_SELECT // The page that gets selected when a valid page number is unknown

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
  {PAGE_CURRENT_DIRECT_SELECT, 13, BANK_DOWN, CURRENT, 100}, // ** Switch 13 **
  {PAGE_CURRENT_DIRECT_SELECT, 14, BANK_UP, CURRENT, 100}, // ** Switch 14 **
  {PAGE_CURRENT_DIRECT_SELECT, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_CURRENT_DIRECT_SELECT, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

// ******************************* PAGE 203: PAGE USER SELECT *************************************************
  {PAGE_USER_SELECT, LABEL, 'S', 'E', 'L', 'E', 'C', 'T', ' ', 'U' },
  {PAGE_USER_SELECT, LABEL, 'S', 'E', 'R', ' ', 'P', 'A', 'G', 'E' },
  {PAGE_USER_SELECT, 1, OPEN_PAGE, COMMON, 1}, // ** Switch 01 **
  {PAGE_USER_SELECT, 2, OPEN_PAGE, COMMON, 2}, // ** Switch 02 **
  {PAGE_USER_SELECT, 3, OPEN_PAGE, COMMON, 3}, // ** Switch 03 **
  {PAGE_USER_SELECT, 4, OPEN_PAGE, COMMON, 4}, // ** Switch 04 **
  {PAGE_USER_SELECT, 5, OPEN_PAGE, COMMON, 5}, // ** Switch 05 **
  {PAGE_USER_SELECT, 6, OPEN_PAGE, COMMON, 6}, // ** Switch 06 **
  {PAGE_USER_SELECT, 7, OPEN_PAGE, COMMON, 7}, // ** Switch 07 **
  {PAGE_USER_SELECT, 8, OPEN_PAGE, COMMON, 8}, // ** Switch 08 **
  {PAGE_USER_SELECT, 9, OPEN_PAGE, COMMON, 9}, // ** Switch 09 **
  {PAGE_USER_SELECT, 10, OPEN_PAGE, COMMON, 10}, // ** Switch 10 **
  {PAGE_USER_SELECT, 11, OPEN_PAGE, COMMON, PAGE_DEVICE_SELECT}, // ** Switch 11 **
  {PAGE_USER_SELECT, 12, OPEN_PAGE, COMMON, PAGE_MENU}, // ** Switch 12 **
  {PAGE_USER_SELECT, 13, NOTHING, COMMON}, // ** Switch 13 **
  {PAGE_USER_SELECT, 14, NOTHING, COMMON}, // ** Switch 14 **
  {PAGE_USER_SELECT, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_USER_SELECT, 16, NOTHING, COMMON}, // ** Switch 16 **

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
  {PAGE_DEVICE_SELECT, 10, NOTHING, COMMON}, // ** Switch 10 **
  {PAGE_DEVICE_SELECT, 11, OPEN_PAGE, COMMON, PAGE_USER_SELECT}, // ** Switch 11 **
  {PAGE_DEVICE_SELECT, 12, OPEN_PAGE, COMMON, PAGE_MENU}, // ** Switch 12 **
  {PAGE_DEVICE_SELECT, 13, NOTHING, COMMON}, // ** Switch 13 **
  {PAGE_DEVICE_SELECT, 14, NOTHING, COMMON}, // ** Switch 14 **
  {PAGE_DEVICE_SELECT, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_DEVICE_SELECT, 16, NOTHING, COMMON}, // ** Switch 16 **

  // ******************************* PAGE 205: Current_patch_bank (10 buttons per page) *************************************************
  {PAGE_CURRENT_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', 'B', 'A' },
  {PAGE_CURRENT_PATCH_BANK, LABEL, 'N', 'K', ' ', ' ', ' ', ' ', ' ', ' ' },
  {PAGE_CURRENT_PATCH_BANK, 1, PATCH_BANK, CURRENT, 1, 10}, // ** Switch 01 **
  {PAGE_CURRENT_PATCH_BANK, 2, PATCH_BANK, CURRENT, 2, 10}, // ** Switch 02 **
  {PAGE_CURRENT_PATCH_BANK, 3, PATCH_BANK, CURRENT, 3, 10}, // ** Switch 03 **
  {PAGE_CURRENT_PATCH_BANK, 4, PATCH_BANK, CURRENT, 4, 10}, // ** Switch 04 **
  {PAGE_CURRENT_PATCH_BANK, 5, PATCH_BANK, CURRENT, 5, 10}, // ** Switch 05 **
  {PAGE_CURRENT_PATCH_BANK, 6, PATCH_BANK, CURRENT, 6, 10}, // ** Switch 06 **
  {PAGE_CURRENT_PATCH_BANK, 7, PATCH_BANK, CURRENT, 7, 10}, // ** Switch 07 **
  {PAGE_CURRENT_PATCH_BANK, 8, PATCH_BANK, CURRENT, 8, 10}, // ** Switch 08 **
  {PAGE_CURRENT_PATCH_BANK, 9, PATCH_BANK, CURRENT, 9, 10}, // ** Switch 09 **
  {PAGE_CURRENT_PATCH_BANK, 10, PATCH_BANK, CURRENT, 10, 10}, // ** Switch 10 **
  //{PAGE_CURRENT_PATCH_BANK, 11, PARAMETER, CURRENT, 1, TOGGLE, 1, 0}, // ** Switch 11 **
  //{PAGE_CURRENT_PATCH_BANK, 11, PARAMETER, CURRENT, 2, TOGGLE, 1, 0}, // ** Switch 11 **
  //{PAGE_CURRENT_PATCH_BANK, 11, PARAMETER, CURRENT, 3, TOGGLE, 1, 0}, // ** Switch 11 **
  //{PAGE_CURRENT_PATCH_BANK, 11, PARAMETER, CURRENT, 4, TOGGLE, 1, 0}, // ** Switch 11 **
  //{PAGE_CURRENT_PATCH_BANK, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_CURRENT_PATCH_BANK, 13, BANK_DOWN, CURRENT, 10}, // ** Switch 13 **
  {PAGE_CURRENT_PATCH_BANK, 14, BANK_UP, CURRENT, 10}, // ** Switch 14 **
  {PAGE_CURRENT_PATCH_BANK, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_CURRENT_PATCH_BANK, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

  // ******************************* PAGE 206: GR55 select *************************************************
  {PAGE_GR55_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', 'B', 'A' },
  {PAGE_GR55_PATCH_BANK, LABEL, 'N', 'K', ' ', 'G', 'R', ' ', ' ', ' ' },
  {PAGE_GR55_PATCH_BANK, 1, PATCH_BANK, GR55, 1, 9}, // ** Switch 01 **
  {PAGE_GR55_PATCH_BANK, 2, PATCH_BANK, GR55, 2, 9}, // ** Switch 02 **
  {PAGE_GR55_PATCH_BANK, 3, PATCH_BANK, GR55, 3, 9}, // ** Switch 03 **
  {PAGE_GR55_PATCH_BANK, 4, PARAMETER, GR55, 33, TOGGLE, 1, 0}, // ** Switch 04 **
  {PAGE_GR55_PATCH_BANK, 5, PATCH_BANK, GR55, 4, 9}, // ** Switch 05 **
  {PAGE_GR55_PATCH_BANK, 6, PATCH_BANK, GR55, 5, 9}, // ** Switch 06 **
  {PAGE_GR55_PATCH_BANK, 7, PATCH_BANK, GR55, 6, 9}, // ** Switch 07 **
  {PAGE_GR55_PATCH_BANK, 8, NOTHING, COMMON}, // ** Switch 08 **
  {PAGE_GR55_PATCH_BANK, 9, PATCH_BANK, GR55, 7, 9}, // ** Switch 09 **
  {PAGE_GR55_PATCH_BANK, 10, PATCH_BANK, GR55, 8, 9}, // ** Switch 10 **
  {PAGE_GR55_PATCH_BANK, 11, PATCH_BANK, GR55, 9, 9}, // ** Switch 11 **
  //{PAGE_GR55_PATCH_BANK, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_GR55_PATCH_BANK, 13, BANK_DOWN, GR55, 9}, // ** Switch 13 **
  {PAGE_GR55_PATCH_BANK, 14, BANK_UP, GR55, 9}, // ** Switch 14 **
  {PAGE_GR55_PATCH_BANK, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_GR55_PATCH_BANK, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

// ******************************* PAGE 207: Zoom patch bank *************************************************
  {PAGE_ZOOM_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', '+', ' ' },
  {PAGE_ZOOM_PATCH_BANK, LABEL, 'F', 'X', ' ', ' ', ' ', ' ', ' ', ' ' },
  {PAGE_ZOOM_PATCH_BANK, 1, PATCH_BANK, CURRENT, 1, 5}, // ** Switch 01 **
  {PAGE_ZOOM_PATCH_BANK, 2, PATCH_BANK, CURRENT, 2, 5}, // ** Switch 02 **
  {PAGE_ZOOM_PATCH_BANK, 3, PATCH_BANK, CURRENT, 3, 5}, // ** Switch 03 **
  {PAGE_ZOOM_PATCH_BANK, 4, PATCH_BANK, CURRENT, 4, 5}, // ** Switch 04 **
  {PAGE_ZOOM_PATCH_BANK, 5, PARAMETER, CURRENT, 0, TOGGLE, 1, 0}, // ** Switch 05 **
  {PAGE_ZOOM_PATCH_BANK, 6, PARAMETER, CURRENT, 1, TOGGLE, 1, 0}, // ** Switch 06 **
  {PAGE_ZOOM_PATCH_BANK, 7, PARAMETER, CURRENT, 2, TOGGLE, 1, 0}, // ** Switch 07 **
  {PAGE_ZOOM_PATCH_BANK, 8, PATCH_BANK, CURRENT, 5, 5}, // ** Switch 08 **
  {PAGE_ZOOM_PATCH_BANK, 9, PARAMETER, CURRENT, 3, TOGGLE, 1, 0}, // ** Switch 09 **
  {PAGE_ZOOM_PATCH_BANK, 10, PARAMETER, CURRENT, 4, TOGGLE, 1, 0}, // ** Switch 10 **
  {PAGE_ZOOM_PATCH_BANK, 11, PARAMETER, CURRENT, 5, TOGGLE, 1, 0}, // ** Switch 11 **
  //{PAGE_ZOOM_PATCH_BANK, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_ZOOM_PATCH_BANK, 13, BANK_DOWN, CURRENT, 5}, // ** Switch 13 **
  {PAGE_ZOOM_PATCH_BANK, 14, BANK_UP, CURRENT, 5}, // ** Switch 14 **
  {PAGE_ZOOM_PATCH_BANK, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
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
  {PAGE_CURRENT_PARAMETER, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_CURRENT_PARAMETER, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **
  
  // ******************************* PAGE 209: GP10 assign *************************************************
  {PAGE_GP10_ASSIGNS, LABEL, 'A', 'S', 'S', 'G', 'N', ' ', 'G', 'P' },
  {PAGE_GP10_ASSIGNS, LABEL, '1', '0', ' ', ' ', ' ', ' ', ' ', ' ' },
  {PAGE_GP10_ASSIGNS, 1, ASSIGN, GP10, 0, 21}, // ** Switch 01 **
  {PAGE_GP10_ASSIGNS, 2, ASSIGN, GP10, 1, 22}, // ** Switch 02 **
  {PAGE_GP10_ASSIGNS, 3, ASSIGN, GP10, 2, 23}, // ** Switch 03 **
  {PAGE_GP10_ASSIGNS, 4, ASSIGN, GP10, 3, 24}, // ** Switch 04 **
  {PAGE_GP10_ASSIGNS, 5, ASSIGN, GP10, 4, 25}, // ** Switch 05 **
  {PAGE_GP10_ASSIGNS, 6, ASSIGN, GP10, 5, 26}, // ** Switch 06 **
  {PAGE_GP10_ASSIGNS, 7, ASSIGN, GP10, 6, 27}, // ** Switch 07 **
  {PAGE_GP10_ASSIGNS, 8, ASSIGN, GP10, 7, 28}, // ** Switch 08 **
  {PAGE_GP10_ASSIGNS, 9, NOTHING, COMMON}, // ** Switch 09 **
  {PAGE_GP10_ASSIGNS, 10, NOTHING, COMMON}, // ** Switch 10 **
  //{PAGE_GP10_ASSIGNS, 11, NOTHING, COMMON}, // ** Switch 11 **
  //{PAGE_GP10_ASSIGNS, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_GP10_ASSIGNS, 13, PREV_PATCH, GP10, 9}, // ** Switch 13 **
  {PAGE_GP10_ASSIGNS, 14, NEXT_PATCH, GP10, 9}, // ** Switch 14 **
  {PAGE_GP10_ASSIGNS, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_GP10_ASSIGNS, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

  // ******************************* PAGE 210: GR55 Assign *************************************************
  {PAGE_GR55_ASSIGNS, LABEL, 'A', 'S', 'S', 'G', 'N', ' ', 'G', 'R'},
  {PAGE_GR55_ASSIGNS, LABEL, '5', '5', ' ', ' ', ' ', ' ', ' ', ' '},
  {PAGE_GR55_ASSIGNS, 1, ASSIGN, GR55, 5, 26}, // ** Switch 01 **
  {PAGE_GR55_ASSIGNS, 2, ASSIGN, GR55, 6, 27}, // ** Switch 02 **
  {PAGE_GR55_ASSIGNS, 3, ASSIGN, GR55, 7, 28}, // ** Switch 03 **
  {PAGE_GR55_ASSIGNS, 4, PARAMETER, GR55, 33, TOGGLE, 1, 0}, // ** Switch 04 **
  {PAGE_GR55_ASSIGNS, 5, PARAMETER, GR55, 0, TOGGLE, 1, 0}, // ** Switch 05 **
  {PAGE_GR55_ASSIGNS, 6, PARAMETER, GR55, 1, STEP, 0, 19, 1}, // ** Switch 08 **
  {PAGE_GR55_ASSIGNS, 7, PARAMETER, GR55, 2, TOGGLE, 1, 0}, // ** Switch 06 **
  {PAGE_GR55_ASSIGNS, 8, PARAMETER, GR55, 3, STEP, 0, 13, 1}, // ** Switch 07 **
  {PAGE_GR55_ASSIGNS, 9, PARAMETER, GR55, 15, TOGGLE, 1, 0}, // ** Switch 09 **
  {PAGE_GR55_ASSIGNS, 10, PARAMETER, GR55, 9, TOGGLE, 1, 0}, // ** Switch 10 **
  //{PAGE_GR55_ASSIGNS, 11, NOTHING, COMMON}, // ** Switch 11 **
  //{PAGE_GR55_ASSIGNS, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_GR55_ASSIGNS, 13, PREV_PATCH, GR55}, // ** Switch 13 **
  {PAGE_GR55_ASSIGNS, 14, NEXT_PATCH, GR55}, // ** Switch 14 **
  {PAGE_GR55_ASSIGNS, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
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
  {PAGE_VG99_EDIT, 13, PREV_PATCH, VG99}, // ** Switch 10 **
  {PAGE_VG99_EDIT, 14, NEXT_PATCH, VG99}, // ** Switch 11 **
  {PAGE_VG99_EDIT, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 12 **
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
  {PAGE_VG99_ASSIGNS, 11, OPEN_PAGE, COMMON, PAGE_VG99_ASSIGNS2 }, // ** Switch 11 **
  //{PAGE_VG99_ASSIGNS, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_VG99_ASSIGNS, 13, PREV_PATCH, VG99}, // ** Switch 10 **
  {PAGE_VG99_ASSIGNS, 14, NEXT_PATCH, VG99}, // ** Switch 11 **
  {PAGE_VG99_ASSIGNS, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 12 **
  {PAGE_VG99_ASSIGNS, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 12 **

  // ******************************* PAGE 213: VG99 Assign2 *************************************************
  {PAGE_VG99_ASSIGNS2, LABEL, 'M', 'O', 'R', 'E', ' ', 'A', 'S', 'G'},
  {PAGE_VG99_ASSIGNS2, 1, ASSIGN, VG99, 31, 255}, // ** Switch 01 **
  {PAGE_VG99_ASSIGNS2, 2, ASSIGN, VG99, 32, 255}, // ** Switch 02 **
  {PAGE_VG99_ASSIGNS2, 3, ASSIGN, VG99, 33, 255}, // ** Switch 03 **
  {PAGE_VG99_ASSIGNS2, 4, ASSIGN, VG99, 34, 255}, // ** Switch 04 **
  {PAGE_VG99_ASSIGNS2, 5, ASSIGN, VG99, 35, 255}, // ** Switch 05 **
  {PAGE_VG99_ASSIGNS2, 6, ASSIGN, VG99, 36, 255}, // ** Switch 06 **
  {PAGE_VG99_ASSIGNS2, 7, ASSIGN, VG99, 37, 255}, // ** Switch 07 **
  {PAGE_VG99_ASSIGNS2, 8, ASSIGN, VG99, 38, 255}, // ** Switch 08 **
  {PAGE_VG99_ASSIGNS2, 9, ASSIGN, VG99, 28, 255}, // ** Switch 09 **
  {PAGE_VG99_ASSIGNS2, 10, ASSIGN, VG99, 29, 255}, // ** Switch 10 **
  {PAGE_VG99_ASSIGNS2, 11, ASSIGN, VG99, 30, 255}, // ** Switch 11 **
   // ** Switch 12 **
  {PAGE_VG99_ASSIGNS2, 13, PREV_PATCH, VG99}, // ** Switch 10 **
  {PAGE_VG99_ASSIGNS2, 14, NEXT_PATCH, VG99}, // ** Switch 11 **
  {PAGE_VG99_ASSIGNS2, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 12 **
  {PAGE_VG99_ASSIGNS2, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 12 **
  
  // ******************************* PAGE 214: Line 6 M13 effects *************************************************
  {PAGE_M13_PARAMETER, LABEL, 'L', 'I', 'N', 'E', '6', ' ', 'M', '1' },
  {PAGE_M13_PARAMETER, LABEL, '3', ' ', 'F', 'X', ' ', ' ', ' ', ' ' },
  {PAGE_M13_PARAMETER, 1, PARAMETER, M13, 0, TOGGLE, 1, 0}, // ** Switch 01 **
  {PAGE_M13_PARAMETER, 2, PARAMETER, M13, 1, TOGGLE, 1, 0}, // ** Switch 02 **
  {PAGE_M13_PARAMETER, 3, PARAMETER, M13, 2, TOGGLE, 1, 0}, // ** Switch 03 **
  {PAGE_M13_PARAMETER, 4, PARAMETER, M13, 3, TOGGLE, 1, 0}, // ** Switch 04 **
  {PAGE_M13_PARAMETER, 5, PARAMETER, M13, 4, TOGGLE, 1, 0}, // ** Switch 05 **
  {PAGE_M13_PARAMETER, 6, PARAMETER, M13, 5, TOGGLE, 1, 0}, // ** Switch 06 **
  {PAGE_M13_PARAMETER, 7, PARAMETER, M13, 6, TOGGLE, 1, 0}, // ** Switch 07 **
  {PAGE_M13_PARAMETER, 8, PARAMETER, M13, 7, TOGGLE, 1, 0}, // ** Switch 08 **
  {PAGE_M13_PARAMETER, 9, PARAMETER, M13, 8, TOGGLE, 1, 0}, // ** Switch 09 **
  {PAGE_M13_PARAMETER, 10, PARAMETER, M13, 9, TOGGLE, 1, 0}, // ** Switch 10 **
  {PAGE_M13_PARAMETER, 11, PARAMETER, M13, 10, TOGGLE, 1, 0}, // ** Switch 11 **
  {PAGE_M13_PARAMETER, 12, PARAMETER, M13, 11, TOGGLE, 1, 0}, // ** Switch 12 **
  {PAGE_M13_PARAMETER, 13, PREV_PATCH, M13}, // ** Switch 13 **
  {PAGE_M13_PARAMETER, 14, NEXT_PATCH, M13}, // ** Switch 14 **
  {PAGE_M13_PARAMETER, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
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
  //{PAGE_FULL_LOOPER, 9, PATCH_BANK, CURRENT, 1, 3}, // ** Switch 09 **
  //{PAGE_FULL_LOOPER, 10, PATCH_BANK, CURRENT, 2, 3}, // ** Switch 10 **
  //{PAGE_FULL_LOOPER, 11, PATCH_BANK, CURRENT, 3, 3}, // ** Switch 11 **
  {PAGE_FULL_LOOPER, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_FULL_LOOPER, 13, PREV_PATCH, CURRENT, 3}, // ** Switch 13 **
  {PAGE_FULL_LOOPER, 14, NEXT_PATCH, CURRENT, 3}, // ** Switch 14 **
  {PAGE_FULL_LOOPER, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_FULL_LOOPER, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **
  
  // ******************************* PAGE 216: Helix_patch_bank (8 buttons per page) *************************************************
  {PAGE_HLX_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', ' ', 'B', 'A' },
  {PAGE_HLX_PATCH_BANK, LABEL, 'N', 'K', ' ', 'H', 'E', 'L', 'I', 'X' },
  {PAGE_HLX_PATCH_BANK, 1, PATCH_BANK, HLX, 1, 8}, // ** Switch 01 **
  {PAGE_HLX_PATCH_BANK, 2, PATCH_BANK, HLX, 2, 8}, // ** Switch 02 **
  {PAGE_HLX_PATCH_BANK, 3, PATCH_BANK, HLX, 3, 8}, // ** Switch 03 **
  {PAGE_HLX_PATCH_BANK, 4, PATCH_BANK, HLX, 4, 8}, // ** Switch 04 **
  {PAGE_HLX_PATCH_BANK, 5, PATCH_BANK, HLX, 5, 8}, // ** Switch 05 **
  {PAGE_HLX_PATCH_BANK, 6, PATCH_BANK, HLX, 6, 8}, // ** Switch 06 **
  {PAGE_HLX_PATCH_BANK, 7, PATCH_BANK, HLX, 7, 8}, // ** Switch 07 **
  {PAGE_HLX_PATCH_BANK, 8, PATCH_BANK, HLX, 8, 8}, // ** Switch 08 **
  {PAGE_HLX_PATCH_BANK, 9, PARAMETER, HLX, 14, STEP, 0, 6, 1}, // ** Switch 09 **
  {PAGE_HLX_PATCH_BANK, 10, OPEN_PAGE, COMMON, PAGE_FULL_LOOPER}, // ** Switch 10 **
  //{PAGE_HLX_PATCH_BANK, 11, PARAMETER, HLX, 1, TOGGLE, 1, 0}, // ** Switch 11 **
  //{PAGE_HLX_PATCH_BANK, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_HLX_PATCH_BANK, 13, BANK_DOWN, HLX, 8}, // ** Switch 13 **
  {PAGE_HLX_PATCH_BANK, 14, BANK_UP, HLX, 8}, // ** Switch 14 **
  {PAGE_HLX_PATCH_BANK, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
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
  {PAGE_HLX_PARAMETER, 13, PREV_PATCH, HLX}, // ** Switch 13 **
  {PAGE_HLX_PARAMETER, 14, NEXT_PATCH, HLX}, // ** Switch 14 **
  {PAGE_HLX_PARAMETER, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_HLX_PARAMETER, 16, OPEN_NEXT_PAGE_OF_DEVICE, HLX}, // ** Switch 16 **
  
  // ******************************* PAGE 218: Snapshots (Helix) /scenes (AxeFX) *************************************************
  {PAGE_SNAPSCENE_LOOPER, LABEL, 'S', 'N', 'A', 'P', 'S', 'C', 'E', 'N' },
  {PAGE_SNAPSCENE_LOOPER, LABEL, 'E', '/', 'L', 'O', 'O', 'P', 'E', 'R' },
  {PAGE_SNAPSCENE_LOOPER, 1, SNAPSCENE, CURRENT, 1}, // ** Switch 01 **
  {PAGE_SNAPSCENE_LOOPER, 2, SNAPSCENE, CURRENT, 2}, // ** Switch 02 **
  {PAGE_SNAPSCENE_LOOPER, 3, SNAPSCENE, CURRENT, 3}, // ** Switch 03 **
  {PAGE_SNAPSCENE_LOOPER, 4, SNAPSCENE, CURRENT, 4}, // ** Switch 04 **
  {PAGE_SNAPSCENE_LOOPER, 5, SNAPSCENE, CURRENT, 5}, // ** Switch 05 **
  {PAGE_SNAPSCENE_LOOPER, 6, SNAPSCENE, CURRENT, 6}, // ** Switch 06 **
  {PAGE_SNAPSCENE_LOOPER, 7, SNAPSCENE, CURRENT, 7}, // ** Switch 07 **
  {PAGE_SNAPSCENE_LOOPER, 8, SNAPSCENE, CURRENT, 8}, // ** Switch 08 **
  {PAGE_SNAPSCENE_LOOPER, 9, LOOPER, CURRENT, LOOPER_REC_OVERDUB}, // ** Switch 09 **
  {PAGE_SNAPSCENE_LOOPER, 10, LOOPER, CURRENT, LOOPER_PLAY_STOP}, // ** Switch 10 **
  //{PAGE_SNAPSCENE_LOOPER, 11, PARAMETER, CURRENT, 10, MOMENTARY, 127, 0}, // ** Switch 11 **
  //{PAGE_SNAPSCENE_LOOPER, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_SNAPSCENE_LOOPER, 13, PREV_PATCH, CURRENT}, // ** Switch 13 **
  {PAGE_SNAPSCENE_LOOPER, 14, NEXT_PATCH, CURRENT}, // ** Switch 14 **
  {PAGE_SNAPSCENE_LOOPER, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_SNAPSCENE_LOOPER, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 16 **

  // ******************************* PAGE 219: KATANA_patch_bank (8 buttons per page) *************************************************
  {PAGE_KTN_PATCH_BANK, LABEL, 'P', 'A', 'T', 'C', 'H', 'B', 'A', 'N' },
  {PAGE_KTN_PATCH_BANK, LABEL, 'K', ' ', 'K', 'A', 'T', 'A', 'N', 'A' },
  {PAGE_KTN_PATCH_BANK, 1, PATCH_BANK, KTN, 2, 8}, // ** Switch 01 **
  {PAGE_KTN_PATCH_BANK, 2, PATCH_BANK, KTN, 3, 8}, // ** Switch 02 **
  {PAGE_KTN_PATCH_BANK, 3, PATCH_BANK, KTN, 4, 8}, // ** Switch 03 **
  {PAGE_KTN_PATCH_BANK, 4, PATCH_BANK, KTN, 5, 8}, // ** Switch 04 **
  {PAGE_KTN_PATCH_BANK, 5, PATCH_BANK, KTN, 6, 8}, // ** Switch 05 **
  {PAGE_KTN_PATCH_BANK, 6, PATCH_BANK, KTN, 7, 8}, // ** Switch 06 **
  {PAGE_KTN_PATCH_BANK, 7, PATCH_BANK, KTN, 8, 8}, // ** Switch 07 **
  {PAGE_KTN_PATCH_BANK, 8, PATCH_BANK, KTN, 9, 8}, // ** Switch 08 **
  {PAGE_KTN_PATCH_BANK, 9, PATCH_SEL, KTN, 0, 0}, // ** Switch 09 **
  {PAGE_KTN_PATCH_BANK, 10, OPEN_PAGE_DEVICE, KTN, PAGE_KTN_EDIT}, // ** Switch 10 **
  //{PAGE_KTN_PATCH_BANK, 11, PARAMETER, KTN, 1, TOGGLE, 1, 0}, // ** Switch 11 **
  //{PAGE_KTN_PATCH_BANK, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_KTN_PATCH_BANK, 13, BANK_DOWN, KTN, 8}, // ** Switch 13 **
  {PAGE_KTN_PATCH_BANK, 14, BANK_UP, KTN, 8}, // ** Switch 14 **
  {PAGE_KTN_PATCH_BANK, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
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
  {PAGE_KTN_EDIT, 11, SAVE_PATCH, KTN }, // ** Switch 11 **
  //{PAGE_KTN_EDIT, 12, PAR_BANK_CATEGORY, KTN, 12}, // ** Switch 12 **
  {PAGE_KTN_EDIT, 13, PREV_PATCH, KTN}, // ** Switch 10 **
  {PAGE_KTN_EDIT, 14, NEXT_PATCH, KTN}, // ** Switch 11 **
  {PAGE_KTN_EDIT, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 12 **
  {PAGE_KTN_EDIT, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 12 **

  // ******************************* PAGE 221: KATANA FX CTRL  *************************************************
  {PAGE_KTN_FX, LABEL, 'K', 'A', 'T', 'A', 'N', 'A', ' ', 'F'},
  {PAGE_KTN_FX, LABEL, 'X', ' ', 'C', 'T', 'R', 'L', ' ', ' '},
  {PAGE_KTN_FX, 1, PARAMETER, KTN, 0, TOGGLE, 1, 0}, // ** Switch 01 **
  {PAGE_KTN_FX, 2, PARAMETER, KTN, 15, TOGGLE, 1, 0}, // ** Switch 02 **
  {PAGE_KTN_FX, 3, PARAMETER, KTN, 61, TOGGLE, 1, 0}, // ** Switch 03 **
  {PAGE_KTN_FX, 4, PARAMETER, KTN, 97, TOGGLE, 1, 0}, // ** Switch 04 **
  {PAGE_KTN_FX, 5, PARAMETER, KTN, 52, TOGGLE, 1, 0}, // ** Switch 05 **
  {PAGE_KTN_FX, 6, PARAMETER, KTN, 78, TOGGLE, 1, 0}, // ** Switch 06 **
  {PAGE_KTN_FX, 7, PARAMETER, KTN, 87, TOGGLE, 1, 0}, // ** Switch 07 **
  {PAGE_KTN_FX, 8, PARAMETER, KTN, 40, TOGGLE, 1, 0}, // ** Switch 08 **
  {PAGE_KTN_FX, 9, PARAMETER, KTN, 112, TOGGLE, 1, 0}, // ** Switch 09 **
  {PAGE_KTN_FX, 10, OPEN_PAGE_DEVICE, KTN, PAGE_KTN_EDIT}, // ** Switch 10 **
  //{PAGE_KTN_FX, 11, PAR_BANK_CATEGORY, KTN, 11 }, // ** Switch 11 **
  //{PAGE_KTN_FX, 12, PAR_BANK_CATEGORY, KTN, 12}, // ** Switch 12 **
  {PAGE_KTN_FX, 13, PREV_PATCH, KTN}, // ** Switch 10 **
  {PAGE_KTN_FX, 14, NEXT_PATCH, KTN}, // ** Switch 11 **
  {PAGE_KTN_FX, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 12 **
  {PAGE_KTN_FX, 16, OPEN_NEXT_PAGE_OF_DEVICE, CURRENT}, // ** Switch 12 **
};

const uint16_t NUMBER_OF_INTERNAL_COMMANDS = sizeof(Fixed_commands) / sizeof(Fixed_commands[0]);

// ********************************* Section 3: VController default configuration for programmable pages ********************************************

// Default configuration of the switches of the user pages - this configuration will be restored when selecting menu - firmware menu - Init commands

// Every switch can have any number commands. Only the first command is shown in the display.

#define LOWEST_USER_PAGE 1 // Minimum value for PAGE_DOWN

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
  {PAGE_DEFAULT, 13, NOTHING, COMMON}, // ** Switch 13 **
  {PAGE_DEFAULT, 14, NOTHING, COMMON}, // ** Switch 14 **
  {PAGE_DEFAULT, 15, SELECT_NEXT_DEVICE, COMMON}, // ** Switch 15 **
  {PAGE_DEFAULT, 16, PAGE_UP, COMMON}, // ** Switch 16 **
  {PAGE_DEFAULT, 17, MASTER_EXP_PEDAL, CURRENT, 0}, // External switch 1 or expr pedal 1
  {PAGE_DEFAULT, 18, NOTHING, COMMON}, // External switch 2
  {PAGE_DEFAULT, 19, NOTHING, COMMON}, // External switch 3 or expr pedal 2
  {PAGE_DEFAULT, 20, NOTHING, COMMON}, // External switch 4
  {PAGE_DEFAULT, 21, NOTHING, COMMON}, // External switch 5 or expr pedal 3
  {PAGE_DEFAULT, 22, NOTHING, COMMON}, // External switch 6
  {PAGE_DEFAULT, 23, PREV_PATCH, CURRENT}, // External switch 7 or expr pedal 4
  {PAGE_DEFAULT, 24, NEXT_PATCH, CURRENT}, // External switch 8

  // ******************************* PAGE 01: GP+GR *************************************************
  {PAGE_COMBO1, LABEL, 'G', 'P', '+', 'G', 'R', ' ', ' ', ' ' },
  {PAGE_COMBO1, 1, PATCH_BANK, GP10, 1, 5}, // ** Switch 01 **
  {PAGE_COMBO1, 2, PATCH_BANK, GP10, 2, 5}, // ** Switch 02 **
  {PAGE_COMBO1, 3, PATCH_BANK, GP10, 3, 5}, // ** Switch 03 **
  {PAGE_COMBO1, 4, PATCH_BANK, GP10, 4, 5}, // ** Switch 04 **
  {PAGE_COMBO1, 5, PATCH_BANK, GR55, 1, 6}, // ** Switch 05 **
  {PAGE_COMBO1, 6, PATCH_BANK, GR55, 2, 6}, // ** Switch 06 **
  {PAGE_COMBO1, 7, PATCH_BANK, GR55, 3, 6}, // ** Switch 07 **
  {PAGE_COMBO1, 8, PATCH_BANK, GP10, 5, 5}, // ** Switch 08 **
  {PAGE_COMBO1, 9, PATCH_BANK, GR55, 4, 6}, // ** Switch 09 **
  {PAGE_COMBO1, 10, PATCH_BANK, GR55, 5, 6}, // ** Switch 10 **
  {PAGE_COMBO1, 11, PATCH_BANK, GR55, 6, 6}, // ** Switch 11 **
  //{PAGE_COMBO1, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_COMBO1, 13, BANK_DOWN, CURRENT, 5}, // ** Switch 13 **
  {PAGE_COMBO1, 14, BANK_UP, CURRENT, 5}, // ** Switch 14 **
  //Switch 15 and 16 are BANK_DOWN and BANK_UP 

  // ******************************* PAGE 02: GPVGGR *************************************************
  {PAGE_COMBO2, LABEL, 'G', 'P', 'V', 'G', 'G', 'R', ' ', ' ' },
  {PAGE_COMBO2, 1, PATCH_BANK, GP10, 1, 4}, // ** Switch 01 **
  {PAGE_COMBO2, 2, PATCH_BANK, GP10, 2, 4}, // ** Switch 02 **
  {PAGE_COMBO2, 3, PATCH_BANK, GP10, 3, 4}, // ** Switch 03 **
  {PAGE_COMBO2, 4, PATCH_BANK, GP10, 4, 4}, // ** Switch 04 **
  {PAGE_COMBO2, 5, PATCH_BANK, VG99, 1, 4}, // ** Switch 05 **
  {PAGE_COMBO2, 6, PATCH_BANK, VG99, 2, 4}, // ** Switch 06 **
  {PAGE_COMBO2, 7, PATCH_BANK, VG99, 3, 4}, // ** Switch 07 **
  {PAGE_COMBO2, 8, PATCH_BANK, VG99, 4, 4}, // ** Switch 08 **
  {PAGE_COMBO2, 9, PATCH_BANK, GR55, 1, 3}, // ** Switch 09 **
  {PAGE_COMBO2, 10, PATCH_BANK, GR55, 2, 3}, // ** Switch 10 **
  {PAGE_COMBO2, 11, PATCH_BANK, GR55, 3, 3}, // ** Switch 11 **
  //{PAGE_COMBO2, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  {PAGE_COMBO2, 13, BANK_DOWN, CURRENT, 4}, // ** Switch 13 **
  {PAGE_COMBO2, 14, BANK_UP, CURRENT, 4}, // ** Switch 14 **
  //Switch 15 and 16 are BANK_DOWN and BANK_UP 
  
  // ******************************* PAGE 03: FUNCTION TEST *************************************************
  {PAGE_FUNCTIONS_TEST, LABEL, 'F', 'U', 'N', 'C', 'T', 'I', 'O', 'N' },
  {PAGE_FUNCTIONS_TEST, LABEL, ' ', 'T', 'E', 'S', 'T', ' ', ' ', ' ' },
  {PAGE_FUNCTIONS_TEST, 1, PATCH_SEL, CURRENT, 1, 0}, // ** Switch 01 **
  {PAGE_FUNCTIONS_TEST, 2, BANK_DOWN, CURRENT, 1}, // ** Switch 02 **
  {PAGE_FUNCTIONS_TEST, 3, PATCH_BANK, CURRENT, 1, 1}, // ** Switch 03 **
  {PAGE_FUNCTIONS_TEST, 4, BANK_UP, CURRENT, 1}, // ** Switch 04 **
  {PAGE_FUNCTIONS_TEST, 5, PARAMETER, CURRENT, 0, TOGGLE, 1, 0}, // ** Switch 05 **
  {PAGE_FUNCTIONS_TEST, 6, PARAMETER, CURRENT, 1, TOGGLE, 1, 0}, // ** Switch 06 **
  {PAGE_FUNCTIONS_TEST, 7, NEXT_PATCH,CURRENT}, // ** Switch 07 **
  {PAGE_FUNCTIONS_TEST, 8, MUTE, CURRENT}, // ** Switch 08 **
  {PAGE_FUNCTIONS_TEST, 9, GLOBAL_TUNER, COMMON}, // ** Switch 09 **
  {PAGE_FUNCTIONS_TEST, 10, SET_TEMPO, COMMON, 95}, // ** Switch 10 **
  {PAGE_FUNCTIONS_TEST, 11, SET_TEMPO, COMMON, 120}, // ** Switch 11 **
  //{PAGE_FUNCTIONS_TEST, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  //{PAGE_FUNCTIONS_TEST, 13, BANK_DOWN, CURRENT, 4}, // ** Switch 13 **
  //{PAGE_FUNCTIONS_TEST, 14, BANK_UP, CURRENT, 4}, // ** Switch 14 **
  //Switch 15 and 16 are BANK_DOWN and BANK_UP 

  // ******************************* PAGE 04: GM TEST *************************************************
  {PAGE_GM_TEST, LABEL, 'G', 'E', 'N', '.', 'M', 'I', 'D', 'I' },
  {PAGE_GM_TEST, LABEL, ' ', 'T', 'E', 'S', 'T', ' ', ' ', ' ' },
  {PAGE_GM_TEST, 1, MIDI_PC, COMMON, 1, 1, ALL_PORTS}, // ** Switch 01 **
  {PAGE_GM_TEST, 2, MIDI_PC, COMMON, 2, 1, ALL_PORTS}, // ** Switch 02 **
  {PAGE_GM_TEST, 3, MIDI_PC, COMMON, 3, 1, ALL_PORTS}, // ** Switch 03 **
  {PAGE_GM_TEST, 3 | LABEL, 'C', 'U', 'S', 'T', 'O', 'M', ' ', 'L'}, // ** Switch 03 **
  {PAGE_GM_TEST, 3 | LABEL, 'A', 'B', 'E', 'L', ' ', 'P', 'C', '3'}, // ** Switch 03 **
  {PAGE_GM_TEST, 4, MIDI_NOTE, COMMON, 52, 100, 1, ALL_PORTS}, // ** Switch 04 **
  {PAGE_GM_TEST, 5, MIDI_CC, COMMON, 30, CC_ONE_SHOT, 127, 0, 1, ALL_PORTS}, // ** Switch 05 **
  {PAGE_GM_TEST, 5 | LABEL, 'O', 'N', 'E', ' ', 'S', 'H', 'O', 'T'}, // ** Switch 05 **
  {PAGE_GM_TEST, 6, MIDI_CC, COMMON, 31, CC_MOMENTARY, 127, 0, 1, ALL_PORTS}, // ** Switch 06 **
  {PAGE_GM_TEST, 6 | LABEL, 'M', 'O', 'M', 'E', 'N', 'T', 'A', 'R'}, // ** Switch 06 **
  {PAGE_GM_TEST, 6 | LABEL, 'Y', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // ** Switch 06 **
  {PAGE_GM_TEST, 7, MIDI_CC, COMMON, 30, CC_TOGGLE, 127, 0, 1, ALL_PORTS}, // ** Switch 07 **
  {PAGE_GM_TEST, 7 | LABEL, 'T', 'O', 'G', 'G', 'L', 'E', ' ', ' '}, // ** Switch 07 **
  {PAGE_GM_TEST, 8, MIDI_CC, COMMON, 31, CC_TOGGLE_ON, 127, 0, 1, ALL_PORTS}, // ** Switch 08 **
  {PAGE_GM_TEST, 8 | LABEL, 'T', 'O', 'G', 'G', 'L', 'E', ' ', 'O'}, // ** Switch 08 **
  {PAGE_GM_TEST, 8 | LABEL, 'N', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // ** Switch 08 **
  {PAGE_GM_TEST, 9, MIDI_CC, COMMON, 32, CC_UPDOWN, 127, 0, 1, ALL_PORTS}, // ** Switch 09 **
  {PAGE_GM_TEST, 10, MIDI_CC, COMMON, 33, CC_STEP, 3, 0, 1, ALL_PORTS}, // ** Switch 10 **
  {PAGE_GM_TEST, 11, SET_TEMPO, COMMON, 120}, // ** Switch 11 **
  //{PAGE_GM_TEST, 12, TAP_TEMPO, COMMON}, // ** Switch 12 **
  //{PAGE_GM_TEST, 13, BANK_DOWN, CURRENT, 4}, // ** Switch 13 **
  //{PAGE_GM_TEST, 14, BANK_UP, CURRENT, 4}, // ** Switch 14 **
  //Switch 15 and 16 are BANK_DOWN and BANK_UP 
};

const uint16_t NUMBER_OF_INIT_COMMANDS = sizeof(Default_commands) / sizeof(Default_commands[0]);




