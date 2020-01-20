#include "globals.h"

// Copied data from globals.h (VController Teensy Code)
#ifdef IS_VCMINI
#define DEFAULT_MAIN_DISPLAY_MODE 3
#else
#define DEFAULT_MAIN_DISPLAY_MODE 1
#endif

Setting_struct Setting = {  // Default values for global settings
  true,  // Send_global_tempo_after_patch_change
  false,  // US20_emulation_active
  true,  // Physical_LEDs
  false, // Virtual_LEDs
  10,    // LED_brightness
  255,   // Backlight_brightness
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
  DEFAULT_MAIN_DISPLAY_MODE,     // Main display mode
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
};

MIDI_switch_settings_struct MIDI_switch[NUMBER_OF_MIDI_SWITCHES] = { // Default settings for MIDI_switch settings
  { MIDI_SWITCH_OFF, 0, 0, 0 }, // switch 0
  { MIDI_SWITCH_OFF, 0, 0, 0 }, // switch 1
  { MIDI_SWITCH_OFF, 0, 0, 0 }, // switch 2
  { MIDI_SWITCH_OFF, 0, 0, 0 }, // switch 3
  { MIDI_SWITCH_OFF, 0, 0, 0 }, // switch 4
  { MIDI_SWITCH_OFF, 0, 0, 0 }, // switch 5
  { MIDI_SWITCH_OFF, 0, 0, 0 }, // switch 6
  { MIDI_SWITCH_OFF, 0, 0, 0 }, // switch 7
  { MIDI_SWITCH_OFF, 0, 0, 0 }, // switch 8
  { MIDI_SWITCH_OFF, 0, 0, 0 }, // switch 9
  { MIDI_SWITCH_OFF, 0, 0, 0 }, // switch 10
  { MIDI_SWITCH_OFF, 0, 0, 0 }, // switch 11
  { MIDI_SWITCH_CC_MOMENTARY, 1, 9, 26 }, // switch 12
  { MIDI_SWITCH_OFF, 0, 0, 0 }, // switch 13
  { MIDI_SWITCH_OFF, 0, 0, 0 }, // switch 14
  { MIDI_SWITCH_OFF, 0, 0, 0 }, // switch 15
  { MIDI_SWITCH_OFF, 0, 0, 0 }, // switch 16
  { MIDI_SWITCH_OFF, 0, 0, 0 }, // switch 17
  { MIDI_SWITCH_OFF, 0, 0, 0 }, // switch 18
  { MIDI_SWITCH_OFF, 0, 0, 0 }, // switch 19
  { MIDI_SWITCH_OFF, 0, 0, 0 }, // switch 20
  { MIDI_SWITCH_OFF, 0, 0, 0 }, // switch 21
  { MIDI_SWITCH_OFF, 0, 0, 0 }, // switch 22
  { MIDI_SWITCH_OFF, 0, 0, 0 }, // switch 23
};
