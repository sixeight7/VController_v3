#include "globals.h"

// Copied data from globals.h (VController Teensy Code)

uint8_t VC_type = 255; // Set to non existing type
QString VC_name = "VC-mini";
uint8_t VC_hardware_version = 255;  // Set to non existing type

Setting_struct Setting = {  // Default values for global settings
  true,  // Send_global_tempo_after_patch_change
  false,  // US20_emulation_active
  true,  // Physical_LEDs
  false, // Virtual_LEDs
  10,    // LED_brightness
  254,   // Backlight_brightness
  120,   // Bpm
  true,  // LED_FX_off_is_dimmed
  6,     // LED_global_colour (white)
  2,     // LED_bpm_colour (red)
  6,     // FX_default_colour (white)
  6,     // FX_GTR_colour (White) for guitar settings
  5,     // FX_PITCH_colour (Turquoise) for pitch FX
  8,     // FX_FILTER_colour (Purple) for filter FX
  9,     // FX_DIST_colour (Pink) for distortion FX
  2,     // FX_AMP_colour (Red) for amp FX and amp solo
  3,     // FX_MODULATE_colour (Blue) for modulation FX
  1,     // FX_DELAY_colour (Green) for delays
  7,     // FX_REVERB_colour (Yellow) for reverb FX
  2,     // MIDI_PC_colour (Red)
  1,     // MIDI_CC_colour (Green)
  3,     // MIDI_note_colour (Blue)
  1,     // Bass_mode_G2M_channel
  0,     // Bass_mode_device
  15,    // Bass_mode_cc_number
  100,   // Bass_mode_min_verlocity
  1,     // Main display mode
  {0, 0, 0, 0}, // The maximum values of the expression pedals (0 = auto calibrate)
  {0, 0, 0, 0}, // The minumum values of the expression pedals (0 = auto calibrate)
  6,     // FX_LOOPER_colour (white)
  0,     // MEP_control - basic control
  5,     // FX_WAH_colour (Purple)
  7,     // FX_DYNAMICS_colour (Yellow)
  0,     // Read MIDI clock data port
  0,     // Send MIDI clock data port
  3,     // Colour of the tempo LED when tempo is synced with MIDI clock (Blue)
  0,     // The colour scheme of the backlight - Adafruit
  0,     // Main_display_show_top_right: show current device
  16,    // HNP_mode_cc_number
  2,     // CURNUM_action: Tap tempo
  {0, 0, 0}, // Input ports for MIDI forwarding
  {0, 0, 0},  // Output ports for MIDI forwarding
  {0, 0, 0},  // Filters for MIDI forwarding
  1,     // Bluetooth mode
  1,     // WIFI mode
  true,  // Status of AppleMIDI / RTPMIDI
  true,  // Enable the WIFI server for OTA updates
  1,     // Enable Follow_tempo_from_G2M
  1,     // Colour of tap tempo LED when tempo following is on: green
  0,     // Forward midi in both directions
  false, // 50W version of Katana
  5,     // Main_display_top_line_mode
  0,     // Block_identity_messages
};

MIDI_switch_settings_struct MIDI_switch[NUMBER_OF_MIDI_SWITCHES] = { // Default settings for MIDI_switch settings
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 0
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 1
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 2
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 3
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 4
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 5
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 6
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 7
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 8
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 9
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 10
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 11
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 12
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 13
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 14
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 15
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 16
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 17
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 18
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 19
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 20
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 21
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 22
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 23
};

uint8_t MIDI_seq_pattern[NUMBER_OF_SEQ_PATTERNS_VCTOUCH][EEPROM_SEQ_PATTERN_SIZE] = {
    { 1, 24, 0, 0, 64, 80, 95, 108, 118, 125, 127, 125, 118, 108, 95, 80, 64, 47, 32, 19, 9, 2, 0, 2, 9, 19, 32, 47 },   // 1: Sine wave
    { 1, 24, 0, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },     // 2: Block wave
    { 1, 24, 0, 0, 11, 21, 32, 42, 53, 64, 74, 85, 95, 106, 116, 127, 116, 106, 95, 85, 74, 64, 53, 42, 32, 21, 11, 0 }, // 3: Triangle wave
    { 1, 24, 0, 0, 0, 6, 11, 17, 22, 28, 33, 39, 44, 50, 55, 61, 66, 72, 77, 83, 88, 94, 99, 105, 110, 116, 121, 127 },  // 4: Saw tooth
    { 1, 24, 0, 0, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },                       // 5: Single 8th beat
    { 2, 24, 0, 0, 127, 127, 0, 32, 32, 0, 32, 0, 0, 127, 64, 0, 127, 127, 0, 32, 32, 0, 32, 32, 0, 32, 32, 0 },         // 6: Beat 1
    { 4, 24, 0, 0, 127, 0, 0, 64, 0, 0, 64, 0, 0, 127, 127, 127, 127, 0, 0, 64, 0, 0, 64, 0, 0, 64, 0, 0 },              // 7: Beat 2
    { 1, 24, 0, 0, 127, 127, 127, 127, 127, 127, 0, 0, 32, 32, 32, 32, 32, 32, 0, 0, 95, 95, 95, 95, 95, 95, 0, 0 },     // 8: Beat 3
};

uint8_t Device_patches[MAX_NUMBER_OF_DEVICE_PRESETS_VCTOUCH][VC_PATCH_SIZE]; // Storage for the actual patches

uint8_t number_of_midi_ports = 0;
QStringList midi_port_names = {};
uint8_t VCmidi_model_number;

uint8_t NUMBER_OF_SEQ_PATTERNS = NUMBER_OF_SEQ_PATTERNS_VC_AND_VCMINI;
uint16_t MAX_NUMBER_OF_DEVICE_PRESETS = MAX_NUMBER_OF_DEVICE_PRESETS_VC_AND_VCMINI; // Copy of EXT_MAX_NUMBER_OF_PATCH_PRESETS
