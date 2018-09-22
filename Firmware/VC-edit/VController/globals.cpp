#include "globals.h"

// Copied data from globals.h (VController Teensy Code)

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
  1,     // Main display mode
  {0, 0, 0, 0}, // The maximum values of the expression pedals (0 = auto calibrate)
  {0, 0, 0, 0}, // The minumum values of the expression pedals (0 = auto calibrate)
  6,     // FX_LOOPER_colour (white)
  0,     // MEP_control - basic control
};
