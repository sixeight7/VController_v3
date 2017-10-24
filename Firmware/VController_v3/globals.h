// Please read VController_v3.ino for information about the license and authors

#ifndef GLOBALS_H
#define GLOBALS_H

// Need to compile the type in a seperate .h file, otherwise these types can not be used in functions headers

struct Setting_struct { // All the global settings in one place.
  bool Send_global_tempo_after_patch_change; // If true, the tempo of all patches will remain the same. Set it by using the tap tempo of the V-Controller
  bool US20_emulation_active; // Switch software emulation of US20 on and off
  bool Physical_LEDs; // Does the VController have Physical LEDs
  bool Virtual_LEDs; // Do you want to switch on Virtual LEDs - state indicators on displays?
  uint8_t LED_brightness; // The neopixels are very bright. I find setting them to 10 is just fine for an indicator light.
  uint8_t Backlight_brightness; //In case of RGB displays, here the backlight brightness is set
  uint8_t Bpm;
  bool LED_FX_off_is_dimmed; // When an effect is off, the LED will be dimmed
  uint8_t LED_global_colour; // Colour of Global functions
  uint8_t LED_bpm_colour;
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
};

Setting_struct Default_settings = {  // Default values for global settings
  true,  // Send_global_tempo_after_patch_change
  true,  // US20_emulation_active
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
  0,     // Main display mode
};

Setting_struct Setting;

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

/*struct Cmd_struct_index {
  uint8_t Page;
  uint8_t Switch;
  uint8_t Type;
  uint8_t Device;
};*/

#define SP_LABEL_SIZE 16
// Switch parameter memory
struct SP_struct {
  uint8_t Device;       // The device the switch is assigned to
  uint8_t Type;         // The type of this switch
  uint16_t Cmd;         // The command number that will set the display and LED state
  bool Cmd_from_default_bank;          // Switch has commands from the default bank
  bool Always_read;     // Switches do not always need reading - after load this variable will indicate if it is neccesary or not.
  uint8_t State;        // State of the switch: on (1) or off(0) or three extra states for TRI/FOURSTATE/RANGE/UPDOWN (2-4)
  uint8_t LED_state;    // State of the LED: on (1), off (0), dimmed (2) or blink (3)
  bool Pressed;         // True when switch is pressed, false when released.
  uint8_t Latch;        // MOMENTARY (0), LATCH (1) TRI/FOURSTATE/RANGE/UPDOWN (2-4)
  uint8_t Step;         // Step value
  bool Direction;       // For UPDOWN type. Up = true
  char Title[SP_LABEL_SIZE + 1];        //Title shows on LCD line 1
  char Label[SP_LABEL_SIZE + 1];        //Label shows on LCD line 2
  uint8_t Colour;        //LED settings
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
  //uint8_t Target_byte3;  // For the Zoom G5, i needed some extra locations for target data (1 for each FX slot)
  //uint8_t Target_byte4;
  //uint8_t Target_byte5;
  //uint8_t Target_byte6;
  uint8_t Value1;
  uint8_t Value2;
  uint8_t Value3;
  uint8_t Value4;
  uint8_t Value5;
};

// Reserve the memory for the switches on the page and the external switches and the default page switch
SP_struct SP[NUMBER_OF_SWITCHES + NUMBER_OF_EXTERNAL_SWITCHES + 1];  // SP = Switch Parameters

#endif
