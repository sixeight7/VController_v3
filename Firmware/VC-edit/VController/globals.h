#ifndef GLOBALS_H
#define GLOBALS_H

#include <QApplication>
#include <QVector>

#define LCD_DISPLAY_SIZE 16

// Copied data from globals.h (VController Teensy Code)

struct Setting_struct { // All the global settings in one place.
  uint8_t Send_global_tempo_after_patch_change; // If true, the tempo of all patches will remain the same. Set it by using the tap tempo of the V-Controller
  uint8_t US20_emulation_active; // Switch software emulation of US20 on and off
  uint8_t Physical_LEDs; // Does the VController have Physical LEDs
  uint8_t Virtual_LEDs; // Do you want to switch on Virtual LEDs - state indicators on displays?
  uint8_t LED_brightness; // The neopixels are very bright. I find setting them to 10 is just fine for an indicator light.
  uint8_t Backlight_brightness; //In case of RGB displays, here the backlight brightness is set
  uint8_t Bpm; // The current tempo
  uint8_t LED_FX_off_is_dimmed; // When an effect is off, the LED will be dimmed
  uint8_t LED_global_colour; // Colour of Global functions
  uint8_t LED_bpm_colour; // Colour of the tempo LED
  uint8_t FX_default_colour; // Default colour
  uint8_t FX_GTR_colour; // Colour for guitar settings
  uint8_t FX_PITCH_colour; // Colour for pitch FX
  uint8_t FX_FILTER_colour; // Colour for filter FX
  uint8_t FX_DIST_colour; // Colour for distortion FX
  uint8_t FX_AMP_colour; // Colour for amp FX and amp solo
  uint8_t FX_MODULATION_colour; // Colour for modulation FX
  uint8_t FX_DELAY_colour; // Colour for delays
  uint8_t FX_REVERB_colour; // Colour for reverb FX
  uint8_t MIDI_PC_colour; // Colour for midi PC buttons
  uint8_t MIDI_CC_colour; // Colour for midi CC buttons
  uint8_t MIDI_note_colour; // Colour for midi note buttons
  uint8_t Bass_mode_G2M_channel; // The channel of the guitar to midi messages
  uint8_t Bass_mode_device; // The device that will perform bass mode
  uint8_t Bass_mode_cc_number;
  uint8_t Bass_mode_min_velocity; // The minimum velocity
  uint8_t Main_display_mode; // The mode of the main display
  uint8_t exp_max[4]; // the maximum values of the expression pedals
  uint8_t exp_min[4]; // the minimum values of the expression pedals
  uint8_t FX_LOOPER_colour; // Colour for the looper
  uint8_t MEP_control; // Control options for Master Expression Pedal
  uint8_t FX_WAH_colour; // Colour for wahs
  uint8_t FX_DYNAMICS_colour; // Colour for dynamics
  uint8_t Read_MIDI_clock_port; // Read MIDI clock data port
  uint8_t Send_MIDI_clock_port; // SendMIDI clock data port
  uint8_t LED_bpm_synced_colour; // Colour of the tempo LED when tempo is synced with MIDI clock
  uint8_t RGB_Backlight_scheme; // The colour scheme of the backlight
};

extern Setting_struct Setting;

struct MIDI_switch_settings_struct {
  uint8_t type;
  uint8_t port;
  uint8_t channel;
  uint8_t cc;
};

#define MIDI_SWITCH_OFF 0
#define MIDI_SWITCH_CC_MOMENTARY 1 // CC controlled by momentary switch
#define MIDI_SWITCH_CC_TOGGLE 2 // CC controlled by toggle switch
#define MIDI_SWITCH_CC_RANGE 3 // CC controlled by expression pedal or encoder knob
#define MIDI_SWITCH_PC 4

#define NUMBER_OF_MIDI_SWITCHES 24

extern MIDI_switch_settings_struct MIDI_switch[NUMBER_OF_MIDI_SWITCHES];

#endif // GLOBALS_H
