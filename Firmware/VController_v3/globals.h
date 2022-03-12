// Please read VController_v3.ino for information about the license and authors

#ifndef GLOBALS_H
#define GLOBALS_H

// Need to compile the type in a seperate .h file, otherwise these types can not be used in functions headers

// Table below has a copy in VC-edit/Headers/VController/globals.cpp

struct Setting_struct { // All the global settings in one place.
  bool    Send_global_tempo_after_patch_change; // If true, the tempo of all patches will remain the same. Set it by using the tap tempo of the V-Controller
  bool    Hide_tap_tempo_LED; // Switch flasing tap tempo LED on and off
  bool    Physical_LEDs; // Does the VController have Physical LEDs
  bool    Virtual_LEDs; // Do you want to switch on Virtual LEDs - state indicators on displays?
  uint8_t LED_brightness; // The neopixels are very bright. I find setting them to 10 is just fine for an indicator light.
  uint8_t Backlight_brightness; //In case of RGB displays, here the backlight brightness is set
  uint8_t Bpm; // The current tempo
  bool    LED_FX_off_is_dimmed; // When an effect is off, the LED will be dimmed
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
  uint8_t Main_display_show_top_right; // What will be shown top right on the main display 
  uint8_t HNP_mode_cc_number; // Addition to Bass mode
  uint8_t CURNUM_action; // What to do when current patch number is pressed again
  uint8_t MIDI_forward_source_port[3]; // Input ports for MIDI forwarding
  uint8_t MIDI_forward_dest_port[3]; // Output ports for MIDI forwarding
  uint8_t MIDI_forward_filter[3]; // Filters for MIDI forwarding
  uint8_t BLE_mode; // Bluetooth mode
  uint8_t WIFI_mode; // WIFI mode
  bool    RTP_enabled; // Status of AppleMIDI / RTPMIDI
  bool    WIFI_server_enabled; // Enable the WIFI server for OTA updates
  uint8_t Follow_tempo_from_G2M; // Tempo is updated from playing speed using Guitar2midi
  uint8_t LED_bpm_follow_colour; // Colour of tap tempo LED when tempo following is on
  uint8_t MIDI_forward_bidirectional; // Forward midi in both directions
  bool    Is_katana50; // 50W version of Katana
};

#define SETTING_BACKLIGHT_BRIGHTNESS_BYTE 5
#define SETTING_BPM_BYTE 6

#ifdef IS_VCMINI
#define DEFAULT_MAIN_DISPLAY_MODE 3
#else
#define DEFAULT_MAIN_DISPLAY_MODE 1
#endif

// Table below has a copy in VC-edit/Sources/VController/globals.cpp 
const Setting_struct Default_settings = {  // Default values for global settings
  true,  // Send_global_tempo_after_patch_change
  false, // Hide tap tempo LED
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
  5,     // FX_PITCH_colour (Cyan) for pitch FX
  8,     // FX_FILTER_colour (Purple) for filter FX
  9,     // FX_DIST_colour (Pink) for distortion FX
  2,     // FX_AMP_colour (Red) for amp FX and amp solo
  3,     // FX_MODULATE_colour (Blue) for modulation FX
  1,     // FX_DELAY_colour (Green) for delays
  4,     // FX_REVERB_colour (Orange) for reverb FX
  2,     // MIDI_PC_colour (Red)
  1,     // MIDI_CC_colour (Green)
  3,     // MIDI_note_colour (Blue)
  1,     // Bass_mode_G2M_channel
  0,     // Bass_mode_device
  15,    // Bass_mode_cc_number
  100,   // Bass_mode_min_verlocity
  DEFAULT_MAIN_DISPLAY_MODE,     // Main display mode - 1 for VController, 3 for VC-mini
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
  2,     // CURNUM_action: Tap temp
  {0, 0, 0}, // Input ports for MIDI forwarding
  {0, 0, 0}, // Output ports for MIDI forwarding
  {0, 0, 0}, // Filters for MIDI forwarding
  1,     // Bluetooth mode
  1,     // WIFI mode
  true,  // Status of AppleMIDI / RTPMIDI
  true,  // Enable the WIFI server for OTA updates
  1,     // Enable Follow_tempo_from_G2M
  1,     // Colour of tap tempo LED when tempo following is on: green
  0,     // Forward midi in both directions
  false, // 50W version of Katana
};

Setting_struct Setting;

// CURNUM states
#define CN_OFF 0
#define CN_PREV_PATCH 1
#define CN_TAP_TEMPO 2
#define CN_GLOBAL_TUNER 3
#define CN_US20_EMULATION 4
#define CN_DIRECT_SELECT 5


struct Cmd_struct { // The structure of a command as it is stored in EEPROM
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
};

struct MIDI_switch_settings_struct {
  uint8_t type;
  uint8_t port;
  uint8_t channel;
  uint8_t cc;
};

#define MIDI_SWITCH_OFF 0
#define MIDI_SWITCH_CC_MOMENTARY 1 // CC controlled by momentary switch
#define MIDI_SWITCH_PRESSED_NO_RELEASE 2 // CC controlled by toggle switch
#define MIDI_SWITCH_CC_RANGE 3 // CC controlled by expression pedal or encoder knob
#define MIDI_SWITCH_PC 4

#define NUMBER_OF_DEFAULT_MIDI_SWITCHES 32
MIDI_switch_settings_struct MIDI_switch[TOTAL_NUMBER_OF_SWITCHES + 1];

const MIDI_switch_settings_struct MIDI_switch_default_settings[NUMBER_OF_DEFAULT_MIDI_SWITCHES] = {
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
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 24
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 25
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 26
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 27
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 28
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 29
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 30
  { MIDI_SWITCH_OFF, 1, 1, 0 }, // switch 31
};

// Switch parameter memory
struct SP_struct {
  uint8_t Device;       // The device the switch is assigned to
  uint8_t Type;         // The cmdtype of this switch
  uint8_t Sel_type;     // The selection type: SELECT/NEXT/PREV/BANKSELECT/BANKUP/BANKDOWN
  uint16_t Cmd;         // The command number that will set the display and LED state
  bool Refresh_with_FX_only; // If true this effect will be refreshed on an update_page = REFRESH_FX_ONLY command.
  uint8_t State;        // State of the switch: on (1) or off(0) or three extra states for TRI/FOURSTATE/RANGE/UPDOWN (2-4)
  bool Pressed;         // True when switch is pressed, false when released.
  uint8_t Latch;        // MOMENTARY (0), LATCH (1) TRI/FOURSTATE/RANGE/UPDOWN (2-4)
  uint8_t Step;         // Step value
  bool Direction;       // For UPDOWN type. Up = true
  char Title[LCD_DISPLAY_SIZE + 1];        //Title shows on LCD line 1
  char Label[LCD_DISPLAY_SIZE + 1];        //Label shows on LCD line 2
  uint8_t Colour;        // LED settings
  uint8_t Exp_pedal;     // Which expression pedal we are controlling - used for MASTER_EXPRESSION_PEDAL
  uint8_t Trigger;       // The trigger of the pedal
  uint16_t PP_number;    //When it is a patch, here we store the patch number, for an parameter/assign it is the pointer to the Parameter table
  uint8_t Bank_position; // Used for PATCH_BANK and PAR_BANK. The relative position in this bank
  uint8_t Bank_size;     // Size of a PATCH_BANK or PAR_BANK
  uint32_t Address;      // The address that needs to be read
  uint8_t Assign_number; // The number of the assign
  bool Assign_on;        // Is the assign on?
  uint16_t Assign_min;   // Assign: min-value (switch is off) - also used for parameter STEP/UPDOWN type
  uint16_t Assign_max;   // Assign: max_value (switch is on) - also used for parameter STEP/UPDOWN type
  uint8_t Target_byte1;  // Once the assign target is known, the state of the target is read into two bytes
  uint8_t Target_byte2;  // This byte often contains the type of the assign - which we exploit in the part of parameter feedback
  uint8_t Value1;
  uint8_t Value2;
  uint8_t Value3;
  uint8_t Value4;
  uint8_t Value5;
  uint8_t Offline_value;
};

// Reserve the memory for the switches on the page and the external switches and the default page switch
SP_struct SP[TOTAL_NUMBER_OF_SWITCHES + 1];  // SP = Switch Parameters

#endif
