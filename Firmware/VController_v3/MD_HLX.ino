// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: HLX Initialization
// Section 2: HLX common MIDI in functions
// Section 3: HLX common MIDI out functions
// Section 4: HLX program change
// Section 5: HLX parameter control
// Section 6: HLX special functions
// Section 7: HELIX MIDI forward messaging

// ********************************* Section 1: HLX Initialization ********************************************

// Line6 HELIX settings:
#define HLX_MIDI_CHANNEL 1
#define HLX_MIDI_PORT MIDI1_PORT // Default port is MIDI1
#define HLX_PATCH_MIN 0
#define HLX_PATCH_MAX 127
#define HLX_NUMBER_OF_SETLISTS 7 // Helix only

// The Helix does not respond to any sysex messages. Therefore we must implement one-way midi connectivity.

// The Helix does support the following CC-messages:
// CC#01 0-127 Emulates EXP 1 Pedal
// CC#02 0-127 Emulates EXP 2 Pedal
// CC#03 0-127 Emulates EXP 3 Pedal (Helix only)
// CC#49 0-127 Emulates Stomp footswitch mode's FS1
// CC#50 0-127 Emulates Stomp footswitch mode's FS2
// CC#51 0-127 Emulates Stomp footswitch mode's FS3
// CC#52 0-127 Emulates Stomp footswitch mode's FS4
// CC#53 0-127 Emulates Stomp footswitch mode's FS5
// CC#54 0-127 Emulates Stomp footswitch mode's FS7 (not HX stomp / effects)
// CC#55 0-127 Emulates Stomp footswitch mode's FS8 (not HX stomp / effects)
// CC#56 0-127 Emulates Stomp footswitch mode's FS9 (Helix only)
// CC#57 0-127 Emulates Stomp footswitch mode's FS10 (Helix only)
// CC#58 0-127 Emulates Stomp footswitch mode's FS11 (Helix only)
// CC#59 0-127 Emulates EXP Toe switch (Helix only)

// CC#60 0-63: Overdub;
// CC#60 64-127: Record Looper Record/Overdub switch (FS8)
// CC#61 0-63: Stop;
// CC#61 64-127: Play Looper Play/Stop switch (FS9)
// CC#62 64-127 Looper Play Once switch (FS3)
// CC#63 64-127 Looper Undo switch (FS2)
// CC#65 0-63: Forward;
// CC#65 64-127: Reverse Looper Forward/Reverse switch (FS11)
// CC#66 0-63: Full;
// CC#66 64-127: Half Looper Full/Half Speed switch (FS10)
// CC#67 0-63: Off;
// CC#67 64-127: On
// Looper block on/off (if available); also enters/exits Looper footswitch mode

// CC#00 0-7 Bank MSB
// CC#32 0-7 Bank LSB—Setlist select
// CC#64 64-127 Tap Tempo
// CC#68 0-127 Tuner screen on/off
// CC#69 0-7 Snapshot select
// CC#71 0 - 3 Foot switch mode  (0= Stomp, 1= Scroll, 2= Preset, 3=Snapshot, 4= prev foot switch mode, 5= next foot switch mode) (HX stomp only)

#define HLX_SNAPSHOT_SELECT_CC 69

#define HLX_EXP_PEDAL_VALUE_FOR_TUNER_CC 2
#define HLX_SET_SEQUENCER_BEATS_CC 3
#define HLX_SET_SEQUENCER_PATTERN_CC 4

FLASHMEM void MD_HLX_class::init() { // Default values for variables
  MD_base_class::init();

  // Line6 HLX variables:
  enabled = DEVICE_DETECT; // Helix can be detected via USB, but not via regular MIDI...
  strcpy(device_name, "HELIX");
  strcpy(full_device_name, "Line6 HELIX");
  current_patch_name.reserve(17);
  patch_max = HLX_PATCH_MAX;
  max_times_no_response = MAX_TIMES_NO_RESPONSE; // The number of times the HLX does not have to respond before disconnection
  sysex_delay_length = 0; // time between sysex messages (in msec).
  my_LED_colour = 2; // Default value: red
  MIDI_channel = HLX_MIDI_CHANNEL; // Default value
  MIDI_port_manual = MIDI_port_number(HLX_MIDI_PORT); // Default value
#if defined(CONFIG_VCTOUCH)
  my_device_page1 = HLX_DEFAULT_VCTOUCH_PAGE1; // Default value
  my_device_page2 = HLX_DEFAULT_VCTOUCH_PAGE2; // Default value
  my_device_page3 = HLX_DEFAULT_VCTOUCH_PAGE3; // Default value
  my_device_page4 = HLX_DEFAULT_VCTOUCH_PAGE4; // Default value
#elif defined(CONFIG_VCMINI)
  my_device_page1 = HLX_DEFAULT_VCMINI_PAGE1; // Default value
  my_device_page2 = HLX_DEFAULT_VCMINI_PAGE2; // Default value
  my_device_page3 = HLX_DEFAULT_VCMINI_PAGE3; // Default value
  my_device_page4 = HLX_DEFAULT_VCMINI_PAGE4; // Default value
#elif defined (CONFIG_CUSTOM)
  my_device_page1 = HLX_DEFAULT_CUSTOM_PAGE1; // Default value
  my_device_page2 = HLX_DEFAULT_CUSTOM_PAGE2; // Default value
  my_device_page3 = HLX_DEFAULT_CUSTOM_PAGE3; // Default value
  my_device_page4 = HLX_DEFAULT_CUSTOM_PAGE4; // Default value
#else
  my_device_page1 = HLX_DEFAULT_VC_PAGE1; // Default value
  my_device_page2 = HLX_DEFAULT_VC_PAGE2; // Default value
  my_device_page3 = HLX_DEFAULT_VC_PAGE3; // Default value
  my_device_page4 = HLX_DEFAULT_VC_PAGE4; // Default value
#endif
  tuner_active = false;
  max_looper_length = 30000000; // Normal stereo looper time is 30 seconds - time given in microseconds

#ifdef IS_VCTOUCH
  device_pic = img_HLX;
#endif
}

FLASHMEM void MD_HLX_class::update() {
  if (!connected) return;
  looper_timer_check();
  if (update_sequencer) { // Is triggered from the device_sequencer_timer_expired().
    send_sequence_step_CC();
    update_sequencer = false;
  }
}

FLASHMEM uint8_t MD_HLX_class::get_number_of_dev_types() {
  return 8;
}

FLASHMEM void MD_HLX_class::get_dev_type_name(uint8_t number, String &name) {
  switch (number) {
    case TYPE_HELIX_01A: name = "Helix (01A)"; break;
    case TYPE_HELIX_000: name = "Helix (000)"; break;
    case TYPE_HX_STOMP_01A: name = "HX stomp (01A)"; break;
    case TYPE_HX_STOMP_000: name = "HX stomp (000)"; break;
    case TYPE_HX_STOMP_XL_01A: name = "HX stomp XL(01A)"; break;
    case TYPE_HX_STOMP_XL_000: name = "HX stomp XL(000)"; break;
    case TYPE_HX_EFFECTS_01A: name = "HX effects (01A)"; break;
    case TYPE_HX_EFFECTS_000: name = "HX effects (000)"; break;
    default: name = "?"; break;
  }
}

FLASHMEM void MD_HLX_class::do_after_dev_type_update() {
  switch (dev_type) {
    case TYPE_HELIX_01A:
    case TYPE_HELIX_000:
      patch_max = HLX_PATCH_MAX;
      number_of_snapshots = 8;
      break;
    case TYPE_HX_STOMP_01A:
    case TYPE_HX_STOMP_000:
      patch_max = 125;
      number_of_snapshots = 3;
      break;
    case TYPE_HX_STOMP_XL_01A:
    case TYPE_HX_STOMP_XL_000:
    case TYPE_HX_EFFECTS_01A:
    case TYPE_HX_EFFECTS_000:
      patch_max = HLX_PATCH_MAX;
      number_of_snapshots = 4;
      break;
  }
}

// ********************************* Section 2: HLX common MIDI in functions ********************************************
FLASHMEM void MD_HLX_class::check_PC_in(uint8_t program, uint8_t channel, uint8_t port) {  // Check incoming PC messages from  Called from MIDI:OnProgramChange
  if (!connected) return;
  // Check the source by checking the channel
  if ((port == MIDI_in_port) && (channel == MIDI_channel)) { // HLX sends a program change
    if (patch_number != program) {
      set_patch_number(program);
      //request_sysex(HLX_REQUEST_CURRENT_PATCH_NAME); // So the main display always show the correct patch
      //page_check();
      do_after_patch_selection();
      update_page = REFRESH_PAGE;
    }
  }
  PC_forwarding(program, channel, port);
}

FLASHMEM void MD_HLX_class::check_CC_in(uint8_t control, uint8_t value, uint8_t channel, uint8_t port) {
  if (!connected) return;
  if ((port == MIDI_in_port) && (channel == MIDI_channel)) {
    switch (control) {
      case HLX_EXP_PEDAL_VALUE_FOR_TUNER_CC: // EXP pedal minimized will start tuner - must be set in Command Center on each patch
        if ((!tuner_active) && (value == 0)) {
          start_tuner();
        }
        else if ((tuner_active) && (value > 0)) {
          stop_tuner();
        }
        break;
      case HLX_SET_SEQUENCER_PATTERN_CC: // set the pattern for the MIDI sequencer
        set_sequence(value);
        break;
      case HLX_SET_SEQUENCER_BEATS_CC:
        set_sequence_beats(value);
        break;
      case 32: // Current setlist
        current_setlist = value;
        break;
      case 60: // Looper REC / OVERDUB
        if (value < 64) looper_press(LOOPER_OVERDUB, false);
        else looper_press(LOOPER_REC, false);
        update_page = REFRESH_PAGE;
        break;
      case 61: // Looper PLAY/STOP
        if (value < 64) looper_press(LOOPER_STOP, false);
        else looper_press(LOOPER_PLAY, false);
        update_page = REFRESH_PAGE;
        break;
      case 62: // Looper PLAY ONCE
        looper_press(LOOPER_PLAY_ONCE, false);
        update_page = REFRESH_PAGE;
        break;
      case 63: // Looper UNDO REDO
        looper_press(LOOPER_UNDO_REDO, false);
        update_page = REFRESH_PAGE;
        break;
      case 65: // Looper FWD/REV
        looper_press(LOOPER_REVERSE, false);
        update_page = REFRESH_PAGE;
        break;
      case 66: // Looper SPEED
        looper_press(LOOPER_HALF_SPEED, false);
        update_page = REFRESH_PAGE;
        break;
      case 69: // Set current snapshot
        current_snapscene = value + 1;
        update_page = REFRESH_PAGE;
        update_main_lcd = true;
        break;
    }
  }
  CC_forwarding(control, value, channel, port);
}

// Detection of Helix only possible when connected via USB midi on the Helix in older versions of firmware.

FLASHMEM void MD_HLX_class::identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t in_port, uint8_t out_port) {
  // Check if it is a Helix
  if ((sxdata[5] == 0x00) && (sxdata[6] == 0x01) && (sxdata[7] == 0x0C) && (sxdata[8] == 0x21) && (enabled == DEVICE_DETECT)) {
    no_response_counter = 0;
    if (connected == false) connect(sxdata[2], in_port, out_port); //Byte 2 contains the correct device ID
  }
}

FLASHMEM void MD_HLX_class::do_after_connect() {
  current_exp_pedal = 2;
}

// ********************************* Section 3: HLX common MIDI out functions ********************************************

FLASHMEM void MD_HLX_class::set_bpm() {
  device_sequencer_update(number_of_sequence_steps, beat_divider);
}

FLASHMEM void MD_HLX_class::bpm_tap() {
  if (connected) {
    MIDI_send_CC(64, 127, MIDI_channel, MIDI_out_port); // Tap tempo on HLX
  }
}

FLASHMEM void MD_HLX_class::start_tuner() {
  if ((connected) && (!tuner_active)) {
    MIDI_send_CC(68, 127, MIDI_channel, MIDI_out_port);
    tuner_active = true;
  }
}

FLASHMEM void MD_HLX_class::stop_tuner() {
  if ((connected) && (tuner_active) && (is_on)) {
    MIDI_send_CC(68, 0, MIDI_channel, MIDI_out_port);
    tuner_active = false;
  }
}

// ********************************* Section 4: HLX program change ********************************************

FLASHMEM void MD_HLX_class::do_after_patch_selection() {
  is_on = connected;
  unmute();
  if (Setting.Send_global_tempo_after_patch_change == true) {
    SCO_retap_tempo();
  }
  Current_patch_number = patch_number;
  current_exp_pedal = 2; // Helix has pedal 2 default on
  current_snapscene = 0;
  //looper_reset();
  stop_sequence();
  update_LEDS = true;
  update_main_lcd = true;
  if ((flash_bank_of_four != 255) && (Current_page != read_current_device_page())) SCO_select_page(read_current_device_page()); // When in direct select, go back to the current_device_page
  flash_bank_of_four = 255;
  MD_base_class::do_after_patch_selection();
}

FLASHMEM bool MD_HLX_class::flash_LEDs_for_patch_bank_switch(uint8_t sw) { // Will flash the LEDs in banks of three when coming from direct select mode.
  if (!bank_selection_active()) return false;

  if (flash_bank_of_four == 255) return true; // We are not coming from direct select, so all LEDs should flash

  bool in_right_bank_of_eight = (flash_bank_of_four / 2 == SP[sw].PP_number / 8); // Going bank up and down coming from direct select will make all LEDs flash in other banks
  if (!in_right_bank_of_eight) return true;

  bool in_right_bank_of_four = (flash_bank_of_four == SP[sw].PP_number / 4); // Only flash the four LEDs of the corresponding bank
  if (in_right_bank_of_four) return true;
  return false;
}


FLASHMEM void MD_HLX_class::number_format(uint16_t number, String &Output) {
  char PatchChar;
  uint8_t hlx_bank_size;
  uint8_t bank_no;

  switch (dev_type) {
    case TYPE_HELIX_01A:
    case TYPE_HX_STOMP_01A:
    case TYPE_HX_STOMP_XL_01A:
    case TYPE_HX_EFFECTS_01A:
      hlx_bank_size = 4;
      if (dev_type == TYPE_HX_STOMP_01A) hlx_bank_size = 3;
      bank_no = number / hlx_bank_size;
      PatchChar = 65 + number % hlx_bank_size;
      Output += String((bank_no + 1) / 10) + String((bank_no + 1) % 10) + PatchChar;
      break;
    case TYPE_HELIX_000:
    case TYPE_HX_STOMP_000:
    case TYPE_HX_STOMP_XL_000:
    case TYPE_HX_EFFECTS_000:
      Output += String(number / 100) + String((number / 10) % 10) + String(number % 10);
      break;
  }

}

// Setlists and songs

void MD_HLX_class::setlist_song_select(uint16_t item) { // This may require more
  if (item > setlist_song_get_number_of_items()) return;
  uint8_t new_setlist = item / 128;
  uint8_t new_patch = item % 128;
  if (current_setlist != new_setlist) {
    MIDI_send_CC(32, new_setlist, MIDI_channel, MIDI_out_port);
    current_setlist = new_setlist;
  }
  select_patch(new_patch);
  patch_number = new_patch;
}

uint16_t MD_HLX_class::setlist_song_get_current_item_state() {
  return (current_setlist * HLX_NUMBER_OF_SETLISTS) + patch_number;
}

uint16_t MD_HLX_class::setlist_song_get_number_of_items() {
  return (HLX_NUMBER_OF_SETLISTS * (patch_max + 1)) - 1;
}

void MD_HLX_class::setlist_song_full_item_format(uint16_t item, String &Output) {
  Output = device_name;
  Output += ": ";
  setlist_song_short_item_format(item, Output);
}

void MD_HLX_class::setlist_song_short_item_format(uint16_t item, String &Output) {
  uint8_t setlist = item / 128;
  uint8_t patch = item % 128;
  Output += "SET";
  Output += String(setlist + 1);
  Output += ' ';
  number_format(patch, Output);
}

// Direct select

FLASHMEM void MD_HLX_class::direct_select_format(uint16_t number, String &Output) {
  if (direct_select_state == 0) Output += String(number) + "__";
  else Output += String(bank_select_number) + String(number) + "_";
}

FLASHMEM bool MD_HLX_class::valid_direct_select_switch(uint8_t number) {
  bool result = false;
  if (direct_select_state == 0) {
    result = ((number * 40) <= (patch_max - patch_min));
  }
  else {
    if ((bank_select_number == 0) && (number == 0)) return false;
    if ((bank_select_number * 40) + (number * 4) <= (patch_max - patch_min)) result = true;
  }
  return result;
}

FLASHMEM void MD_HLX_class::direct_select_start() {
  Previous_bank_size = bank_size; // Remember the bank size
  device_in_bank_selection = my_device_number + 1;
  bank_size = 400;
  bank_select_number = 0; // Reset bank number
  direct_select_state = 0;
}

FLASHMEM void MD_HLX_class::direct_select_press(uint8_t number) {
  if (!valid_direct_select_switch(number)) return;
  if (direct_select_state == 0) {
    // First digit pressed
    bank_size = 40;
    bank_select_number = number;
    direct_select_state = 1;
  }
  else  {
    // Second digit pressed
    uint16_t base_patch = (bank_select_number * 40) + (number - 1) * 4;
    flash_bank_of_four = base_patch / 4;
    bank_select_number = (base_patch / Previous_bank_size);
    bank_size = Previous_bank_size;
    SCO_select_page(my_device_page1); // Which should give PAGE_HLX_PATCH_BANK
    device_in_bank_selection = my_device_number + 1; // Go into bank mode
  }
}

FLASHMEM void MD_HLX_class::unmute() {
  is_on = connected;
  stop_tuner();
}

FLASHMEM void MD_HLX_class::mute() {
  if ((US20_mode_enabled()) && (!is_always_on) && (is_on)) {
    is_on = false;
    start_tuner();
  }
}

// ********************************* Section 5: HLX parameter control ********************************************
// Define array for HLX effeect names and colours
struct HLX_CC_type_struct { // Combines all the data we need for controlling a parameter in a device
  char Name[11]; // The name for the label
  uint8_t CC; // The colour for this effect.
  uint8_t NumVals; // The number of values for this parameter
};

const PROGMEM HLX_CC_type_struct HLX_CC_types[] = {
  {"FS01", 49, 2}, // 0
  {"FS02", 50, 2},
  {"FS03", 51, 2},
  {"FS04", 52, 2},
  {"FS05", 53, 2},
  {"FS07", 54, 2},
  {"FS08", 55, 2},
  {"FS09", 56, 2},
  {"FS10", 57, 2},
  {"FS11", 58, 2},
  {"TOE SW EXP", 59, 2}, // 10
  {"EXP1", 1, 128}, // 11
  {"EXP2", 2, 128},
  {"EXP3", 3, 128},
  {"SETLIST", 32, HLX_NUMBER_OF_SETLISTS - 1}, // 14
  {"SNAPSHOT", 69, 8},
};

const uint16_t HLX_NUMBER_OF_PARAMETERS = sizeof(HLX_CC_types) / sizeof(HLX_CC_types[0]);

#define HLX_SW_EXP_TOE 10
#define HLX_SW_EXP1 11
#define HLX_SW_EXP2 12
#define HLX_SW_EXP3 13
#define HLX_SETLIST 14
#define HLX_SNAPSHOT 15

FLASHMEM void MD_HLX_class::parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {
  // Send cc command to Line6 HLX
  uint8_t value = SCO_return_parameter_value(Sw, cmd);
  if (HLX_CC_types[number].NumVals == 2) {
    MIDI_send_CC(HLX_CC_types[number].CC, 127, MIDI_channel, MIDI_out_port);
  }
  else {
    MIDI_send_CC(HLX_CC_types[number].CC, value, MIDI_channel, MIDI_out_port);
  }

  if (number == HLX_SETLIST) current_setlist = value;
  if (number == HLX_SNAPSHOT) current_snapscene = value;

  String msg = HLX_CC_types[number].Name;
  if (HLX_CC_types[number].NumVals < 127) {
    msg += ' ';
    msg += SP[Sw].Label;
  }
  else { // Expression pedal
    msg += ':';
    msg += String(value);
  }
  LCD_show_popup_label(msg, ACTION_TIMER_LENGTH);
  update_page = REFRESH_FX_ONLY; // To update the other switch states, we re-load the current page
}

FLASHMEM void MD_HLX_class::parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {
}

FLASHMEM void MD_HLX_class::read_parameter_title(uint16_t number, String &Output) {
  Output += HLX_CC_types[number].Name;
}

FLASHMEM bool MD_HLX_class::request_parameter(uint8_t sw, uint16_t number) {
  String msg = "";
  SP[sw].State = 2; // Effect off
  SP[sw].Colour = my_LED_colour;

  if ((number == HLX_SETLIST) || (number == HLX_SNAPSHOT)) {
    if (number == HLX_SETLIST) SP[sw].Target_byte1 = current_setlist;
    if (number == HLX_SNAPSHOT) SP[sw].Target_byte1 = current_snapscene;
    if (SP[sw].Latch == STEP) msg += String(SP[sw].Target_byte1 + 1);
    else msg += String(SP[sw].Value1 + 1);
  }

  LCD_set_SP_label(sw, msg);
  return true; // Move to next switch is true.
}

FLASHMEM void MD_HLX_class::check_update_label(uint8_t Sw, uint8_t value) { // Updates the label for extended sublists
  String msg = "";
  uint16_t index = SP[Sw].PP_number;
  if ((index >= HLX_SW_EXP1) && (index <= HLX_SW_EXP3)) {
    LCD_add_3digit_number(value, msg);
  }
  LCD_set_SP_label(Sw, msg);

  //Update the current switch label
  update_lcd = Sw;
}

// Menu options for FX states
FLASHMEM void MD_HLX_class::read_parameter_name(uint16_t number, String &Output) { // Called from menu
  if (number < number_of_parameters())  Output = HLX_CC_types[number].Name;
  else Output = "?";
}

FLASHMEM uint16_t MD_HLX_class::number_of_parameters() {
  return HLX_NUMBER_OF_PARAMETERS;
}

FLASHMEM uint8_t MD_HLX_class::number_of_values(uint16_t parameter) {
  if (parameter < HLX_NUMBER_OF_PARAMETERS) {
    return HLX_CC_types[parameter].NumVals;
  }
  else return 0;
}

FLASHMEM void MD_HLX_class::read_parameter_value_name(uint16_t number, uint16_t value, String &Output) {
  if (number < number_of_parameters()) Output += String(value);
  else Output += " ? "; // Unknown parameter
}

FLASHMEM void MD_HLX_class::move_expression_pedal(uint8_t sw, uint8_t value, uint8_t exp_pedal) {
  uint8_t number;
  if (exp_pedal == 0) exp_pedal = current_exp_pedal;
  switch (exp_pedal) {
    case 1: number = HLX_SW_EXP1; break;
    case 2: number = HLX_SW_EXP2; break;
    case 3: number = HLX_SW_EXP3; break;
    default: return;
  }
  LCD_show_bar(0, value, 0); // Show it on the main display
  MIDI_send_CC(HLX_CC_types[number].CC, value, MIDI_channel, MIDI_out_port);
  check_update_label(sw, value);
  String msg = HLX_CC_types[number].Name;
  msg += ':';
  LCD_add_3digit_number(value, msg);
  LCD_show_popup_label(msg, ACTION_TIMER_LENGTH);
  update_page = REFRESH_FX_ONLY; // To update the other switch states, we re-load the current page
}

FLASHMEM void MD_HLX_class::toggle_expression_pedal(uint8_t sw) {
  uint8_t value;
  if (current_exp_pedal == 0) return;
  current_exp_pedal++;
  if (current_exp_pedal > 2) current_exp_pedal = 1;
  // Send toe switch message
  if (current_exp_pedal <= 2) value = 127;
  MIDI_send_CC(HLX_CC_types[HLX_SW_EXP_TOE].CC, value, MIDI_channel, MIDI_out_port);
  update_page = REFRESH_FX_ONLY;
}

FLASHMEM bool MD_HLX_class::request_exp_pedal(uint8_t sw, uint8_t exp_pedal) {
  if (exp_pedal == 0) exp_pedal = current_exp_pedal;
  uint8_t number = 0;
  if (exp_pedal == 1) number = HLX_SW_EXP1;
  if (exp_pedal == 2) number = HLX_SW_EXP2;
  if (exp_pedal == 3) number = HLX_SW_EXP3;
  SP[sw].PP_number = number;
  LCD_clear_SP_label(sw);
  return true;
}

FLASHMEM void MD_HLX_class::get_snapscene_title(uint8_t number, String &Output) {
  Output += "SNAPSHOT " + String(number);
}

/*FLASHMEM void MD_HLX_class::set_snapscene_name(uint8_t number, String &Output) {
  Output += "SNAPSHOT " + String(number);
  }*/

FLASHMEM void MD_HLX_class::set_snapscene(uint8_t sw, uint8_t number) {
  if (!is_on) unmute();
  if ((number < 1) || (number > 8)) return;
  current_snapscene = number;
  MIDI_send_CC(HLX_SNAPSHOT_SELECT_CC, number - 1, MIDI_channel, MIDI_out_port);
  MIDI_send_current_snapscene(my_device_number, current_snapscene);
}

FLASHMEM void MD_HLX_class::show_snapscene(uint8_t  number) {
  if ((number < 1) || (number > 8)) return;
  if (number == current_snapscene) return;
  current_snapscene = number;
}

FLASHMEM void MD_HLX_class::snapscene_number_format(String &Output) { // Add snapshot number to potchnumber
  if (current_snapscene == 0) return;
  Output += '-';
  Output += String(current_snapscene);
}

FLASHMEM bool MD_HLX_class::looper_active() {
  return true;
}

struct HLX_looper_cc_struct { // Combines all the data we need for controlling a parameter in a device
  uint8_t cc; // The cc number
  uint8_t value; // The value.
};

const PROGMEM HLX_looper_cc_struct HLX_looper_cc[] = { // Table with the cc messages
  {67, 0},   // HIDE
  {67, 127}, // SHOW
  {61, 0},   // STOP
  {61, 127}, // PLAY
  {60, 127}, // REC
  {60, 0},   // OVERDUB
  {65, 0},   // FORWARD
  {65, 127}, // REVERSE
  {66, 0},   // FULL_SPEED
  {66, 127}, // HALF_SPEED
  {63, 127}, // UNDO
  {63, 127}, // REDO
  {62, 127}, // PLAY_ONCE
  {0, 0},    // PRE
  {0, 0},    // POST
};

const uint8_t HLX_LOOPER_NUMBER_OF_CCS = sizeof(HLX_looper_cc) / sizeof(HLX_looper_cc[0]);

FLASHMEM void MD_HLX_class::send_looper_cmd(uint8_t cmd) {
  if (cmd < HLX_LOOPER_NUMBER_OF_CCS) {
    if (HLX_looper_cc[cmd].cc > 0) MIDI_send_CC(HLX_looper_cc[cmd].cc, HLX_looper_cc[cmd].value, MIDI_channel, MIDI_out_port);
  }
}

// ********************************* Section 6: HLX special MIDI functions **********************************************

// Function 1: MIDI sequencer
// There are 32 patterns that can be stored in EEPROM. Furthermore there are some more patterns defined here

// Every pattern consists of 36 bytes
// Byte 01: The beat divider
// Byte 02: The number of steps in this sequence
// Byte 03: Spare
// Byte 04: Spare
// Byte 05 - 36: The steps of the sequence.

const PROGMEM uint8_t HLX_sequencer_pattern[][36] = {
  // First value is the beat divider for this pattern.
  { 1, 24, 0, 0, 64, 80, 95, 108, 118, 125, 127, 125, 118, 108, 95, 80, 64, 47, 32, 19, 9, 2, 0, 2, 9, 19, 32, 47 },   // 33: Sine wave
  { 1, 24, 0, 0, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },     // 34: Block wave
  { 1, 24, 0, 0, 11, 21, 32, 42, 53, 64, 74, 85, 95, 106, 116, 127, 116, 106, 95, 85, 74, 64, 53, 42, 32, 21, 11, 0 }, // 35: Triangle wave
  { 1, 24, 0, 0, 0, 6, 11, 17, 22, 28, 33, 39, 44, 50, 55, 61, 66, 72, 77, 83, 88, 94, 99, 105, 110, 116, 121, 127 },  // 36: Saw tooth
  { 1, 24, 0, 0, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },                       // 37: Single 8th beat
  { 2, 24, 0, 0, 127, 127, 0, 32, 32, 0, 32, 0, 0, 127, 64, 0, 127, 127, 0, 32, 32, 0, 32, 32, 0, 32, 32, 0 },         // 38: Beat 1
  { 4, 24, 0, 0, 127, 0, 0, 64, 0, 0, 64, 0, 0, 127, 127, 127, 127, 0, 0, 64, 0, 0, 64, 0, 0, 64, 0, 0 },              // 39: Beat 2
  { 1, 24, 0, 0, 127, 127, 127, 127, 127, 127, 0, 0, 32, 32, 32, 32, 32, 32, 0, 0, 95, 95, 95, 95, 95, 95, 0, 0 },     // 40: Beat 3
};                                                                                                           // 41: Random pattern

#define HLX_NUMBER_OF_SEQUENCER_PATTERNS sizeof(HLX_sequencer_pattern) / sizeof(HLX_sequencer_pattern[0])

#define PATTERN_EEPROM 254
#define PATTERN_RANDOM 255

FLASHMEM void MD_HLX_class::set_sequence(uint8_t pattern) {
  if (pattern == 0) { // Stop sequencer
    current_sequence = 0;
    number_of_sequence_steps = 0;
  }

  else if (pattern <= EXT_MAX_NUMBER_OF_SEQ_PATTERNS) { // Load from EEPROM
    EEPROM_load_seq_pattern(pattern - 1, EEPROM_seq_pattern);
    current_sequence = PATTERN_EEPROM;
    beat_divider = EEPROM_seq_pattern[0];
    number_of_sequence_steps = EEPROM_seq_pattern[1];
  }

  else if (pattern <= EXT_MAX_NUMBER_OF_SEQ_PATTERNS + HLX_NUMBER_OF_SEQUENCER_PATTERNS) { // Load HLX_sequence_pattern
    current_sequence = pattern - EXT_MAX_NUMBER_OF_SEQ_PATTERNS;
    beat_divider = HLX_sequencer_pattern[current_sequence - 1][0];
    number_of_sequence_steps = HLX_sequencer_pattern[current_sequence - 1][1];
  }

  else { // Load ramdom pattern
    current_sequence = PATTERN_RANDOM;
    beat_divider = 12;
    number_of_sequence_steps = 24;
  }

  device_sequencer_update(number_of_sequence_steps, beat_divider);
  DEBUGMSG("current_sequence:" + String(current_sequence) + ", number_of_sequence_steps:" + String(number_of_sequence_steps) + ", beat_divider:" + String(beat_divider));
}

FLASHMEM void MD_HLX_class::set_sequence_beats(uint8_t beats) {
  if (beats == 0) return;
  beat_divider = beats;
  device_sequencer_update(number_of_sequence_steps, beat_divider);
}

FLASHMEM void MD_HLX_class::stop_sequence() {
  current_sequence = 0;
  beat_divider = 1;
}

FLASHMEM void MD_HLX_class::send_sequence_step_CC() {
  uint8_t value;
  if (current_sequence == 0) return;
  if (current_sequence == PATTERN_EEPROM) value = EEPROM_seq_pattern[current_sequence_step + 4];
  else if (current_sequence <= HLX_NUMBER_OF_SEQUENCER_PATTERNS) value = HLX_sequencer_pattern[current_sequence - 1][current_sequence_step + 4];
  else if (current_sequence == PATTERN_RANDOM) value = generate_random_number();
  else return;
  if (value != previous_sequence_value) {
    MIDI_send_CC(HLX_SET_SEQUENCER_PATTERN_CC, value, MIDI_channel, MIDI_out_port);
    previous_sequence_value = value;
  }
  current_sequence_step++;
  if (current_sequence_step >= number_of_sequence_steps) current_sequence_step = 0;
}

// ********************************* Section 7: HELIX MIDI forward messaging ********************************************

// The Helix sends instant command messages after the patch has been loaded and activated in the Helix. But a PC message is sent before the patch has been loaded:

// PC Helix                          Instant command midi messages
// |----------------------------------|---|---|----|--------------------> time

// This results in a double gap when using another effect processor in parallel with the Helix (I use the VG-99)

// Gap Helix at PC           Sound on Second gap of external processor
// |                          |------|    |---|----|--------------------> time

// To bridge the gap the instant command MIDI messages from the Helix are stored in the EEPROM memory and send straight away the next time the same PC message is invoked.
// Now there will only be one gap as the Helix and the external processor will change program at the same time.
// This is a self-learning system, as it learns the proper program changes and CC messages from the Helix and updates the EEPROM data if there are any differences:
// New and/or changed messages are forwarded and added by checking the received messages from the Helix port within the 200 ms window
// Deleted messages are checked on the next program change by comparing the number of sent and received messages.

#define MIDI_HELIX_IC_TIME 200; // Time that instant commands are sent after sending program change message - the gap of the Helix is 120 ms on averge

FLASHMEM void MD_HLX_class::PC_forwarding(uint8_t Program, uint8_t Channel, uint8_t Port) {
  if (Port != MIDI_in_port) return;
  if (Channel == MIDI_channel) { // Check if it is PC message from the Helix
    // Forward PC message from the Helix
    MIDI_forward_PC_to_all_ports_but_mine(Program, Channel, MIDI_out_port);

    // Check if more messages have been sent than received on last transmit - this means a message has been deleted from the Helix
    if (MIDI_Helix_sent_msg_no > MIDI_Helix_received_msg_no) {
      for (uint8_t m = MIDI_Helix_received_msg_no; m < MIDI_HLX_MESSAGES; m++) { // Clear the remained of the messages
        HLX_messages[m][0] = 0;
        HLX_messages[m][1] = 0;
        HLX_messages[m][2] = 0;
      }
      EEPROM_store_HELIX_message(MIDI_Helix_current_program, current_setlist); // Store updated messages in EEPROM
      DEBUGMSG("Cleared location  " + String(MIDI_Helix_current_program));
    }

    // Forward messages from EEPROM memory
    EEPROM_load_HELIX_message(Program);
    //DEBUGMSG("Stored Helix setlist: " + String(HLX_message_setlist) + " == HLX setlist: " + String(current_setlist));
    if (current_setlist == HLX_message_setlist) {
      MIDI_Helix_sent_msg_no = 0;
      for (uint8_t m = 0; m < MIDI_HLX_MESSAGES; m++) {
        if (HLX_messages[m][0] != 0) {
          MIDI_Helix_sent_msg_no++;
          switch (HLX_messages[m][0] & 0xF0) {
            case 0xC0: // Program change message
              MIDI_forward_PC_to_all_ports_but_mine(HLX_messages[m][1], HLX_messages[m][0] & 0x0F,  MIDI_out_port);
              break;
            case 0xB0: // Control change message
              MIDI_forward_CC_to_all_ports_but_mine(HLX_messages[m][1], HLX_messages[m][2], HLX_messages[m][0] & 0x0F, MIDI_out_port);
              break;
          }
        }
      }
      do_not_forward_after_Helix_PC_message = true;

      //DEBUGMSG("Done sending " + String(MIDI_Helix_sent_msg_no) + " messages from memory at " + String(millis()));
    }

    // Set timer
    MIDI_Helix_IC_timer = millis() + MIDI_HELIX_IC_TIME;
    MIDI_Helix_current_program = Program;
    MIDI_Helix_received_msg_no = 0;
  }
  else {
    if ((millis() < MIDI_Helix_IC_timer) && (MIDI_Helix_received_msg_no < MIDI_HLX_MESSAGES)) { // Check timer running
      // Check message with stored message in EEPROM
      DEBUGMSG("Checking PC message " + String(MIDI_Helix_received_msg_no) + " at " + String(millis()));
      if ((Channel != (HLX_messages[MIDI_Helix_received_msg_no][0] & 0x0F)) || (Program != HLX_messages[MIDI_Helix_received_msg_no][1]) || (current_setlist != HLX_message_setlist)) {
        DEBUGMSG("PC message does not match - Channel:" + String(Channel) + "!=" + String(HLX_messages[MIDI_Helix_received_msg_no][0] & 0x0F));
        MIDI_forward_PC_to_all_ports_but_mine(Program, Channel, MIDI_out_port);
        //MIDI_send_PC(Program, Channel, My_VG99.MIDI_out_port); // Forward the message
        HLX_messages[MIDI_Helix_received_msg_no][1] = Program;
        HLX_messages[MIDI_Helix_received_msg_no][0] = 0xC0 | Channel;
        EEPROM_store_HELIX_message(MIDI_Helix_current_program, current_setlist); // Store it in EEPROM
      }
      MIDI_Helix_received_msg_no++;
    }
    else {
      // Forward message
      MIDI_forward_PC_to_all_ports_but_mine(Program, Channel, MIDI_out_port);
    }
  }
}

FLASHMEM void MD_HLX_class::CC_forwarding(uint8_t Controller, uint8_t Value, uint8_t Channel, uint8_t Port) {
  if (Port != MIDI_in_port) return;
  if ((millis() < MIDI_Helix_IC_timer) && (Channel != MIDI_channel) && (MIDI_Helix_received_msg_no < MIDI_HLX_MESSAGES)) { // Check timer running
    // Check message with stored message in EEPROM
    DEBUGMSG("Checking CC message " + String(MIDI_Helix_received_msg_no) + " at " + String(millis()));
    if ((Channel != (HLX_messages[MIDI_Helix_received_msg_no][0] & 0x0F)) || (Controller != HLX_messages[MIDI_Helix_received_msg_no][1]) || (Value != HLX_messages[MIDI_Helix_received_msg_no][2]) || (current_setlist != HLX_message_setlist)) {
      DEBUGMSG("CC message does not match message #" + String(MIDI_Helix_received_msg_no));
      DEBUGMSG("Received: CC#" + String(Controller) + ", value: " + String(Value) + " on channel " + String(Channel));
      DEBUGMSG("Stored:   CC#" + String(HLX_messages[MIDI_Helix_received_msg_no][1]) + ", value: " + String(HLX_messages[MIDI_Helix_received_msg_no][2]) + " on channel " + String(HLX_messages[MIDI_Helix_received_msg_no][0] & 0x0F));
      MIDI_forward_CC_to_all_ports_but_mine(Controller, Value, Channel, MIDI_out_port);
      HLX_messages[MIDI_Helix_received_msg_no][1] = Controller;
      HLX_messages[MIDI_Helix_received_msg_no][2] = Value;
      HLX_messages[MIDI_Helix_received_msg_no][0] = 0xB0 | Channel;
      EEPROM_store_HELIX_message(MIDI_Helix_current_program, current_setlist); // Store it in EEPROM
    }
    MIDI_Helix_received_msg_no++;
  }
  else {
    // Forward message
    MIDI_forward_CC_to_all_ports_but_mine(Controller, Value, Channel, MIDI_out_port);
  }
}
