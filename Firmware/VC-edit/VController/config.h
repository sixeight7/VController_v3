#ifndef CONFIG_H
#define CONFIG_H

#include <QApplication>
#include <QVector>

// To compile the editor for the full VController, add // before #define IS_VCMINI
// To compile the editor for the VC-mini, remove the // before #define IS_VCMINI
#define IS_VCMINI

struct Cmd_struct
{
    uint8_t Page;   // The page this command belongs to
    uint8_t Switch; // The switch this command belongs to
    uint8_t Type;   // The type of command
    uint8_t Device; // The device for this command (can be device number, CURRENT or COMMON)
    uint8_t Data1;  // Can be the patch/parameter/assign/pc/cc/note or page number
    uint8_t Data2;  // Can be bank number of parameter/cc type
    uint8_t Value1; // Values for parameters, etc.
    uint8_t Value2;
    uint8_t Value3;
    uint8_t Value4;

    bool operator==(const Cmd_struct &other) const // In C++, structs do not have a comparison operator generated by default. You need to write your own
    {
        return Page == other.Page && Switch == other.Switch && Type == other.Type && Device == other.Device && Data1 == other.Device
                && Data2 == other.Data2 && Value1 == other.Value1 && Value2 == other.Value2 && Value3 == other.Value3 && Value4 == other.Value4;
    }
};

// Copied data from Config.ino (VController Teensy Code)

// ********************************* Section 1: VController commands ********************************************

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

//Toggle types
#define MOMENTARY 0
#define TOGGLE 1
#define TRISTATE 2
#define FOURSTATE 3
//#define FIVESTATE 4
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
#define NUMBER_OF_LOOPER_TYPES 11
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

// Ports you can use
#define NUMBER_OF_MIDI_PORTS 5
#define USBMIDI_PORT 0x00
#define MIDI1_PORT 0x10
#define MIDI2_PORT 0x20
#define MIDI3_PORT 0x30
#define USBHMIDI_PORT 0x40
#define ALL_PORTS 0xF0

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
#ifndef IS_VCMINI

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

// ********************************* Section 3: VController default configuration for programmable pages ********************************************

#define PAGE_DEFAULT 0
#define PAGE_COMBO1 1
#define PAGE_COMBO2 2
#define PAGE_FUNCTIONS_TEST 3
#define PAGE_GM_TEST 4

#endif // NO_VCMINI

// ********************************* Section 4: VC-mini configuration for fixed command pages ********************************************
#ifdef IS_VCMINI

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
#define KTN_DEFAULT_PAGE2 PAGE_KTN_FX1
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

// ********************************* Section 5: VController default configuration for programmable pages ********************************************
#define PAGE_DEFAULT 0
#define PAGE_GM_TEST 1
#define PAGE_COMBO1 2
#define PAGE_FUNC 3
#endif // IS_VCMINI

extern QVector<Cmd_struct> Commands;
extern const QVector<Cmd_struct> Fixed_commands;
extern int number_of_cmds;
extern int Number_of_pages;
extern const uint16_t NUMBER_OF_INTERNAL_COMMANDS;

#endif // CONFIG_H
