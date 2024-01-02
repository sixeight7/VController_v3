// Please read VController_v3.ino for information about the license and authors

// Class structure for Strymon Volante

// This page has the following parts:
// Section 1: SVL Initialization
// Section 2: SVL common MIDI in functions
// Section 3: SVL common MIDI out functions
// Section 4: SVL program change
// Section 5: SVL parameter control
// Section 6: SVL expression pedal control

// ********************************* Section 1: SVL Initialization ********************************************

// MIDI CC NUMBERS
// Bank Select CC#0 0-2

// Type CC#11 1-3 (1=studio, 2=drum, 3=tape)
// Echo Level CC#12 0-127
// Rec Level CC#13 0-127
// Mechanics CC#14 0-127
// Wear CC#15 0-127
// Low Cut CC#16 0-127
// Time CC#17 0-127
// Spacing CC#18 0-127 (even=0, triplet=34, golden=94, silver=127)
// Speed CC#19 1-3 (1=double, 2=half, 3=normal)
// Repeats CC#20 0-127
// Head 1 Playback Off/On CC#21 0, 127 (0=off, 1-127=on)
// Head 2 Playback Off/On CC#22 0, 127 (0=off, 1-127=on)
// Head 3 Playback Off/On CC#23 0, 127 (0=off, 1-127=on)
// Head 4 Playback Off/On CC#24 0, 127 (0=off, 1-127=on)
// Head 1 Level CC#25 0-127
// Head 2 Level CC#26 0-127
// Head 3 Level CC#27 0-127
// Head 4 Level CC#28 0-127
// Head 1 Pan CC#29 0-127
// Head 2 Pan CC#30 0-127
// Head 3 Pan CC#31 0-127
// Head 4 Pan CC#32 0-127
// Head 1 Feedback Off/On CC#34 0, 127 (0=off, 1-127=on)
// Head 2 Feedback Off/On CC#35 0, 127 (0=off, 1-127=on)
// Head 3 Feedback Off/On CC#36 0, 127 (0=off, 1-127=on)
// Head 4 Feedback Off/On CC#37 0, 127 (0=off, 1-127=on)
// Pause ramp speed CC#38 0-127
// Spring (level) CC#39 0-127
// Spring Decay CC#40 0-127

// SOS mode CC#41 0, 127 (0=normal, 1-127=SOS)
// Pause (no ramp) CC#42 0, 127 (0=unpause, 1-127=pause)
// Pause (ramp) CC#43 0, 127 (0=unpause, 1-127=pause)
// Reverse CC#44 0, 127 (0=normal, 1-127=reverse)
// Infinite Hold (w/ oscillation) CC#45 0, 127 (0=release, 1-127=hold)
// Infinite Hold (w/o oscillation) CC#46 0-127 (0=release, 1-127=hold)
// SOS Repeats Level CC#47 0-127
// SOS Loop Level CC#48 0-127

// MIDI Expression Off/On CC#60 0, 127 (0=off, 1-127=on)
// MIDI Clock Off/On CC#63 0, 127 (0=off, 1-127=on)
// Echo On/Off CC#78 0, 127 (0=off, 127=on)
// Reverb On/Off CC#79 0, 127 (0=off, 127=on)
// Footswitch On CC#80 0, 127 (0=release, 1-127=press)
// Footswitch Favorite CC#81 0, 127 (0=release, 1-127=press)
// Footswitch Tap CC#82 0, 127 (0=release, 1-127=press)
// Persist CC#83 0, 127 (0=persist off, 1-127=persist on)
// Kill Dry CC#84 0, 127 (0=dry off, 1-127=dry on)
// Output Sum CC#85 0, 127 (0=stereo, 1-127=sum)
// Remote Tap CC#93 any
// Expression Pedal CC#100 0-127
// Bypass/On CC#102 0, 127 (0=bypass, 1-127=on)

// Strymon Volante settings:
#define SVL_MIDI_CHANNEL 1
#define SVL_MIDI_PORT MIDI1_PORT // Default port is MIDI1
#define SVL_PATCH_MIN 0
#define SVL_PATCH_MAX 299

FLASHMEM void MD_SVL_class::init() { // Default values for variables
  MD_base_class::init();

  // Line6 SVL variables:
  enabled = DEVICE_DETECT; // Strymon Volante can be detected via USB, but not via regular MIDI...
  strcpy(device_name, "SVL");
  strcpy(full_device_name, "Strymon Volante");
  patch_min = SVL_PATCH_MIN;
  patch_max = SVL_PATCH_MAX;
  max_times_no_response = MAX_TIMES_NO_RESPONSE; // The number of times the SVL does not have to respond before disconnection
  sysex_delay_length = 0; // time between sysex messages (in msec).
  my_LED_colour = 7; // Default value: yellow
  MIDI_channel = SVL_MIDI_CHANNEL; // Default value
  MIDI_port_manual = MIDI_port_number(SVL_MIDI_PORT); // Default value
#if defined(CONFIG_VCTOUCH)
  my_device_page1 = SVL_DEFAULT_VCTOUCH_PAGE1; // Default values for VC-touch
  my_device_page2 = SVL_DEFAULT_VCTOUCH_PAGE2;
  my_device_page3 = SVL_DEFAULT_VCTOUCH_PAGE3;
  my_device_page4 = SVL_DEFAULT_VCTOUCH_PAGE4;
#elif defined(CONFIG_VCMINI)
  my_device_page1 = SVL_DEFAULT_VCMINI_PAGE1; // Default values for VC-mini
  my_device_page2 = SVL_DEFAULT_VCMINI_PAGE2;
  my_device_page3 = SVL_DEFAULT_VCMINI_PAGE3;
  my_device_page4 = SVL_DEFAULT_VCMINI_PAGE4;
#elif defined (CONFIG_CUSTOM)
  my_device_page1 = SVL_DEFAULT_CUSTOM_PAGE1; // Default values for custom VC device
  my_device_page2 = SVL_DEFAULT_CUSTOM_PAGE2;
  my_device_page3 = SVL_DEFAULT_CUSTOM_PAGE3;
  my_device_page4 = SVL_DEFAULT_CUSTOM_PAGE4;
#else
  my_device_page1 = SVL_DEFAULT_VC_PAGE1; // Default values for VController
  my_device_page2 = SVL_DEFAULT_VC_PAGE2;
  my_device_page3 = SVL_DEFAULT_VC_PAGE3;
  my_device_page4 = SVL_DEFAULT_VC_PAGE4;
#endif

#ifdef IS_VCTOUCH
  device_pic = img_SVL;
#endif
}

// ********************************* Section 2: SVL common MIDI in functions ********************************************
FLASHMEM void MD_SVL_class::check_PC_in(uint8_t program, uint8_t channel, uint8_t port) {  // Check incoming PC messages from  Called from MIDI:OnProgramChange
  if (!connected) return;
  // Check the source by checking the channel
  if ((port == MIDI_in_port) && (channel == MIDI_channel)) { // SVL sends a program change
    if (patch_number != program) {
      set_patch_number(program + (CC00 << 7));
      //request_sysex(SVL_REQUEST_CURRENT_PATCH_NAME); // So the main display always show the correct patch
      //page_check();
      do_after_patch_selection();
      update_page = REFRESH_PAGE;
    }
  }
}

FLASHMEM void MD_SVL_class::check_CC_in(uint8_t control, uint8_t value, uint8_t channel, uint8_t port) {
  if (!connected) return;
  if ((port == MIDI_in_port) && (channel == MIDI_channel)) {
    if (control == 0) CC00 = value;
    else update_parameter_state_through_cc(control, value);
  }
}

// Detection of Strymon Volante
// Volante - identity response:
// F0 7E 00 06 02 00 01 55 13 00 04 00 30 31 31 32 F7

FLASHMEM void MD_SVL_class::identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t in_port, uint8_t out_port) {
  // Check if it is a Strymon Volante
  if ((sxdata[5] == 0x00) && (sxdata[6] == 0x01) && (sxdata[7] == 0x55) && (sxdata[8] == 0x13) && (enabled == DEVICE_DETECT)) {
    no_response_counter = 0;
    if (connected == false) connect(sxdata[2], in_port, out_port); //Byte 2 contains the correct device ID
  }
}

FLASHMEM void MD_SVL_class::do_after_connect() {
  //  current_exp_pedal = 2;
}

// ********************************* Section 3: SVL common MIDI out functions ********************************************
FLASHMEM void MD_SVL_class::bpm_tap() {
  if (connected) {
    MIDI_send_CC(93, 127, MIDI_channel, MIDI_out_port); // Tap tempo on the Strymon Volante
  }
}

// ********************************* Section 4: SVL program change ********************************************

FLASHMEM void MD_SVL_class::select_patch(uint16_t new_patch) {
  //if (new_patch == patch_number) unmute();
  prev_patch_number = patch_number;
  patch_number = new_patch;

  MIDI_send_CC(0, new_patch >> 7, MIDI_channel, MIDI_out_port);
  MIDI_send_PC(new_patch & 0x7F, MIDI_channel, MIDI_out_port);
  DEBUGMSG("out(SVL) PC" + String(new_patch)); //Debug
  do_after_patch_selection();
  update_page = REFRESH_PAGE;
}

FLASHMEM void MD_SVL_class::do_after_patch_selection() {
  is_on = connected;
  if (Setting.Send_global_tempo_after_patch_change == true) {
    SCO_retap_tempo();
  }
  Current_patch_number = patch_number;
  update_LEDS = true;
  update_main_lcd = true;
  if ((flash_bank_of_four != 255) && (Current_page != read_current_device_page())) SCO_select_page(read_current_device_page()); // When in direct select, go back to the current_device_page
  flash_bank_of_four = 255;
  MD_base_class::do_after_patch_selection();
}

FLASHMEM void MD_SVL_class::number_format(uint16_t number, String & Output) {
  build_patch_number(number, Output, "001", "999");
}

FLASHMEM void MD_SVL_class::direct_select_format(uint16_t number, String & Output) {
  if (direct_select_state == 0) {
    Output += String(bank_select_number) + String(number) + "_";
  }
  else {
    Output +=  String(bank_select_number / 10) + String(bank_select_number % 10) + String(number);
  }
}

// ********************************* Section 5: SVL parameter control ********************************************
// Define array for SVL effeect names and colours
struct SVL_CC_type_struct { // Combines all the data we need for controlling a parameter in a device
  char Name[17]; // The name for the label
  uint8_t CC; // The colour for this effect.
  uint8_t NumVals; // The number of values for this parameter
  uint8_t Sublist;
  bool Momentary;
};

const PROGMEM SVL_CC_type_struct SVL_CC_types[SVL_NUMBER_OF_PARAMETERS] = {
  {"FS ON", 80, 2, 0, true}, // 0
  {"FS FAVORITE", 81, 2, 0, true},
  {"FS TAP", 82, 2, 0, true},
  {"SPEED", 19, 3, 1, false},
  {"SOS mode", 41, 2, 0, false},
  {"SPLICE", 82, 2, 0, true},
  {"REVERSE", 44, 2, 0,  false},
  {"PAUSE", 43, 2, 0,  false},
  {"INFINITE", 45, 2, 0,  false},
  {"BYPASS", 102, 2, 0,  false},
  {"TYPE", 11, 3, 4, false},
  {"REVERB ON/OFF", 79, 2, 0,  false},
  {"ECHO ON/OFF", 78, 2, 0,  false},
  {"KILL DRY", 84, 2, 0,  false},
  {"EXP", 100, 128, 0, true},
};

const PROGMEM char SVL_sublists[][8] = {
  // SPEED types
  "DOUBLE", "HALF", "NORMAL",

  // TYPE types
  "STUDIO", "DRUM", "TAPE",
};

#define SVL_EXP 14

FLASHMEM void MD_SVL_class::parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {
  // Send cc command to Line6 SVL
  uint8_t value = SCO_return_parameter_value(Sw, cmd);
  if (SVL_CC_types[number].NumVals == 2) {
    MIDI_send_CC(SVL_CC_types[number].CC, value, MIDI_channel, MIDI_out_port);
  }
  else if (SVL_CC_types[number].NumVals == 3) {
    MIDI_send_CC(SVL_CC_types[number].CC, value + 1, MIDI_channel, MIDI_out_port);
  }
  else {
    MIDI_send_CC(SVL_CC_types[number].CC, value, MIDI_channel, MIDI_out_port);
  }
  par_on[number] = value;

  String msg = SVL_CC_types[number].Name;
  //if (SVL_CC_types[number].NumVals < 127) {
  //  msg += SP[Sw].Label;
  if (SVL_CC_types[number].Sublist >= 1) {
    msg += ':';
    msg += SVL_sublists[SVL_CC_types[number].Sublist - 1 + value];
  }
  //}
  if (SVL_CC_types[number].NumVals == 127) { // Expression pedal
    msg += ':';
    msg += String(value);
  }
  if (LCD_check_popup_allowed(Sw)) LCD_show_popup_label(msg, ACTION_TIMER_LENGTH);
  update_page = REFRESH_FX_ONLY; // To update the other switch states, we re-load the current page
}

FLASHMEM void MD_SVL_class::parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {
  if (SVL_CC_types[number].Momentary) {
    MIDI_send_CC(SVL_CC_types[number].CC, 0, MIDI_channel, MIDI_out_port);
    par_on[number] = 0;
  }
}

FLASHMEM void MD_SVL_class::read_parameter_title(uint16_t number, String &Output) {
  Output += SVL_CC_types[number].Name;
}

FLASHMEM bool MD_SVL_class::request_parameter(uint8_t sw, uint16_t number) {
  String msg = "";
  if (par_on[number] == 0) SP[sw].State = 2; // Effect off
  else SP[sw].State = 1; // Effect on
  SP[sw].Colour = my_LED_colour;

  LCD_set_SP_label(sw, msg);
  /*if (SP[sw].Type == PAR_BANK) {
    if ((!SVL_CC_types[number].Momentary) && (SVL_CC_types[number].NumVals == 2)) SP[sw].Latch = TOGGLE;
    else SP[sw].Latch = MOMENTARY;
    }*/
  return true; // Move to next switch is true.
}

FLASHMEM void MD_SVL_class::check_update_label(uint8_t Sw, uint8_t value) { // Updates the label for extended sublists
  String msg = "";
  uint16_t index = SP[Sw].PP_number;
  if (index == SVL_EXP) {
    LCD_add_3digit_number(value, msg);
  }
  LCD_set_SP_label(Sw, msg);

  //Update the current switch label
  update_lcd = Sw;
}

// Menu options for FX states
FLASHMEM void MD_SVL_class::read_parameter_name(uint16_t number, String &Output) { // Called from menu
  if (number < number_of_parameters())  Output = SVL_CC_types[number].Name;
  else Output = "?";
}

FLASHMEM uint16_t MD_SVL_class::number_of_parameters() {
  return SVL_NUMBER_OF_PARAMETERS;
}

FLASHMEM uint8_t MD_SVL_class::number_of_values(uint16_t parameter) {
  if (parameter < SVL_NUMBER_OF_PARAMETERS) {
    return SVL_CC_types[parameter].NumVals;
  }
  else return 0;
}

FLASHMEM void MD_SVL_class::read_parameter_value_name(uint16_t number, uint16_t value, String &Output) {
  if (number < number_of_parameters()) Output += String(value);
  else Output += " ? "; // Unknown parameter
}

FLASHMEM void MD_SVL_class::update_parameter_state_through_cc(uint8_t control, uint8_t value) {
  for (uint8_t i = 0; i < SVL_NUMBER_OF_PARAMETERS; i++) {
    if (control == SVL_CC_types[i].CC) par_on[i] = value;
  }
}

// ********************************* Section 6: SVL expression pedal control ********************************************

FLASHMEM void MD_SVL_class::move_expression_pedal(uint8_t sw, uint8_t value, uint8_t exp_pedal) {
  uint8_t number = SVL_EXP;
  LCD_show_bar(0, value, 0); // Show it on the main display
  MIDI_send_CC(SVL_CC_types[number].CC, value, MIDI_channel, MIDI_out_port);
  check_update_label(sw, value);
  String msg = SVL_CC_types[number].Name;
  msg += ':';
  LCD_add_3digit_number(value, msg);
  LCD_show_popup_label(msg, ACTION_TIMER_LENGTH);
  update_page = REFRESH_FX_ONLY; // To update the other switch states, we re-load the current page
}

FLASHMEM bool MD_SVL_class::request_exp_pedal(uint8_t sw, uint8_t exp_pedal) {
  SP[sw].PP_number = SVL_EXP;
  LCD_clear_SP_label(sw);
  return true;
}
