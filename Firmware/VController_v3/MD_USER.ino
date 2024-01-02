// Please read VController_v3.ino for information about the license and authors

// Class structure for User devices

// This page has the following parts:
// Section 1: GEN Initialization
// Section 2: GEN common MIDI in functions
// Section 3: GEN common MIDI out functions
// Section 4: GEN program change
// Section 5: GEN parameter control
// Section 6: GEN expression pedal control

#define USER_FULL_NAME_INDEX 0 // 16 bytes
#define USER_SHORT_NAME_INDEX 16 // 7 bytes
#define USER_PATCH_MIN_INDEX 23 // 2 bytes
#define USER_PATCH_MAX_INDEX 25 // 2 bytes
#define USER_PATCH_FORMAT_INDEX 27 // 6 bytes
#define USER_PATCH_MASK_INDEX 33 // 6 bytes
#define USER_SCENE_CC_INDEX 39 // 1 byte
#define USER_LOOPER_DATA_INDEX 40 // 28 bytes
#define USER_DETECTION_DATA_INDEX 68 // 4 bytes
// Total: 72 bytes – reserve 128 bytes for each device.


// ********************************* Section 1: GEN Initialization ********************************************

// User Device settings:
#define USER_MIDI_CHANNEL 1
#define USER_MIDI_PORT MIDI1_PORT // Default port is MIDI1
#define USER_PATCH_MIN 0
#define USER_PATCH_MAX 299

FLASHMEM void MD_USER_class::init() { // Default values for variables
  MD_base_class::init();

  // User variables:
  my_instance = my_device_number - USER1;
  my_number = my_instance + 1;
  init_from_device_data();

  enabled = DEVICE_DETECT;
  max_times_no_response = MAX_TIMES_NO_RESPONSE; // The number of times the GEN does not have to respond before disconnection
  sysex_delay_length = 0; // time between sysex messages (in msec).
  my_LED_colour = 1; // Default value: green
  MIDI_channel = USER_MIDI_CHANNEL; // Default value
  MIDI_port_manual = MIDI_port_number(USER_MIDI_PORT); // Default value
#if defined(CONFIG_VCTOUCH)
  my_device_page1 = USER_DEFAULT_VCTOUCH_PAGE1; // Default values for VC-touch VC-touch
  my_device_page2 = USER_DEFAULT_VCTOUCH_PAGE2;
  my_device_page3 = USER_DEFAULT_VCTOUCH_PAGE3;
  my_device_page4 = USER_DEFAULT_VCTOUCH_PAGE4;
#elif defined(CONFIG_VCMINI)
  my_device_page1 = USER_DEFAULT_VCMINI_PAGE1; // Default values for VC-mini VC-mini
  my_device_page2 = USER_DEFAULT_VCMINI_PAGE2;
  my_device_page3 = USER_DEFAULT_VCMINI_PAGE3;
  my_device_page4 = USER_DEFAULT_VCMINI_PAGE4;
#elif defined (CONFIG_CUSTOM)
  my_device_page1 = USER_DEFAULT_CUSTOM_PAGE1; // Default values for custom VC device custom VC device
  my_device_page2 = USER_DEFAULT_CUSTOM_PAGE2;
  my_device_page3 = USER_DEFAULT_CUSTOM_PAGE3;
  my_device_page4 = USER_DEFAULT_CUSTOM_PAGE4;
#else
  my_device_page1 = USER_DEFAULT_VC_PAGE1; // Default values for VControllers VController
  my_device_page2 = USER_DEFAULT_VC_PAGE2;
  my_device_page3 = USER_DEFAULT_VC_PAGE3;
  my_device_page4 = USER_DEFAULT_VC_PAGE4;
#endif

#ifdef IS_VCTOUCH
  device_pic = img_user;
#endif
}

FLASHMEM void MD_USER_class::init_from_device_data() {
  EEPROM_load_user_device_data(my_instance, &device_data);
  if (device_data.short_name[0] != (char)255) { // Memory initialized
    strcpy(device_name, device_data.short_name);
    strcpy(full_device_name, device_data.full_name);
    patch_min = (device_data.patch_min_msb << 8) + device_data.patch_min_lsb;
    patch_max = (device_data.patch_max_msb << 8) + device_data.patch_max_lsb;
  }
  else { // Memory not initialized
    strcpy(device_name, "USER");
    strcpy(full_device_name, "USER device");
    patch_min = 0;
    patch_max = 99;
  }
}

FLASHMEM void MD_USER_class::update() {
  if (!connected) return;
  if (looper_active()) looper_timer_check();
}

// ********************************* Section 2: GEN common MIDI in functions ********************************************

FLASHMEM void MD_USER_class::check_PC_in(uint8_t program, uint8_t channel, uint8_t port) {  // Check incoming PC messages from  Called from MIDI:OnProgramChange
  if (!connected) return;
  // Check the source by checking the channel
  if ((port == MIDI_in_port) && (channel == MIDI_channel)) { // GEN sends a program change
    uint16_t new_patch_number = 0;
    switch (device_data.pc_type) {
      case PC_TYPE_PC_ONLY:
        new_patch_number = program;
        break;
      case PC_TYPE_PC_AND_CC0:
        new_patch_number = program + (CC00 << 7);
        break;
      case PC_TYPE_PC_AND_CC32:
        new_patch_number = program + (CC32 << 7);
        break;
      case PC_TYPE_PC_AND_CC0_MOD100:
        new_patch_number = program + (CC00 * 100);
        break;
      case PC_TYPE_PC_AND_CC32_MOD100:
        new_patch_number = program + (CC32 * 100);
        break;
    }
    if (patch_number != program) {
      set_patch_number(new_patch_number);
      do_after_patch_selection();
      update_page = REFRESH_PAGE;
    }
  }
}

FLASHMEM void MD_USER_class::check_CC_in(uint8_t control, uint8_t value, uint8_t channel, uint8_t port) {
  if (!connected) return;
  if ((port == MIDI_in_port) && (channel == MIDI_channel)) {
    if (control == 0) CC00 = value;
    if (control == 32) CC32 = value;
    if ((control == 60) && (device_data.pc_type == PC_TPE_CC60)) {
      set_patch_number(value);
      do_after_patch_selection();
      update_page = REFRESH_PAGE;
    }
    if (control == device_data.parameter_CC[USER_PAR_SCENE_SELECT]) {
      current_snapscene = value + 1;
      update_page = REFRESH_PAGE;
      update_main_lcd = true;
    }

    if ((control == device_data.parameter_CC[USER_PAR_LOOPER_STOP])      && (value == device_data.parameter_value_max[USER_PAR_LOOPER_STOP]))      looper_press(LOOPER_STOP, false);
    if ((control == device_data.parameter_CC[USER_PAR_LOOPER_REC])       && (value == device_data.parameter_value_max[USER_PAR_LOOPER_REC]))       looper_press(LOOPER_REC, false);
    if ((control == device_data.parameter_CC[USER_PAR_LOOPER_PLAY])      && (value == device_data.parameter_value_max[USER_PAR_LOOPER_PLAY]))      looper_press(LOOPER_PLAY, false);
    if ((control == device_data.parameter_CC[USER_PAR_LOOPER_OVERDUB])   && (value == device_data.parameter_value_max[USER_PAR_LOOPER_OVERDUB]))   looper_press(LOOPER_OVERDUB, false);
    if ((control == device_data.parameter_CC[USER_PAR_LOOPER_FORWARD])   && (value == device_data.parameter_value_max[USER_PAR_LOOPER_FORWARD]))   looper_press(LOOPER_FORWARD, false);
    if ((control == device_data.parameter_CC[USER_PAR_LOOPER_REVERSE])   && (value == device_data.parameter_value_max[USER_PAR_LOOPER_REVERSE]))   looper_press(LOOPER_REVERSE, false);
    if ((control == device_data.parameter_CC[USER_PAR_LOOPER_FULLSPEED]) && (value == device_data.parameter_value_max[USER_PAR_LOOPER_FULLSPEED])) looper_press(LOOPER_FULL_SPEED, false);
    if ((control == device_data.parameter_CC[USER_PAR_LOOPER_HALFSPEED]) && (value == device_data.parameter_value_max[USER_PAR_LOOPER_HALFSPEED])) looper_press(LOOPER_HALF_SPEED, false);
    if ((control == device_data.parameter_CC[USER_PAR_LOOPER_UNDO])      && (value == device_data.parameter_value_max[USER_PAR_LOOPER_UNDO]))      looper_press(LOOPER_UNDO, false);
    if ((control == device_data.parameter_CC[USER_PAR_LOOPER_REDO])      && (value == device_data.parameter_value_max[USER_PAR_LOOPER_REDO]))      looper_press(LOOPER_REDO, false);
    if ((control == device_data.parameter_CC[USER_PAR_LOOPER_PLAY_ONCE]) && (value == device_data.parameter_value_max[USER_PAR_LOOPER_PLAY_ONCE])) looper_press(LOOPER_PLAY_ONCE, false);

    update_parameter_state_through_cc(control, value);
  }
}

// Detection of User Device
// Volante - identity response:
// F0 7E 00 06 02 00 01 55 13 00 04 00 30 31 31 32 F7

FLASHMEM void MD_USER_class::identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t in_port, uint8_t out_port) {
  // Check if it is this User Device
  if ((sxdata[5] == device_data.device_detect[0]) && (sxdata[6] == device_data.device_detect[1])
      && (sxdata[7] == device_data.device_detect[2]) && (sxdata[8] == device_data.device_detect[3]) && (enabled == DEVICE_DETECT)) {
    no_response_counter = 0;
    if (connected == false) connect(sxdata[2], in_port, out_port); //Byte 2 contains the correct device ID
  }
}

FLASHMEM void MD_USER_class::do_after_connect() {
  current_exp_pedal = 1;
  request_current_patch_name();
  load_par_state();
  max_looper_length = device_data.looper_length * 1000000;
}

// ********************************* Section 3: GEN common MIDI out functions ********************************************
FLASHMEM void MD_USER_class::bpm_tap() {
  if (connected) {
    if (device_data.parameter_CC[USER_PAR_TAP_TEMPO] > 0) {
      MIDI_send_CC(device_data.parameter_CC[USER_PAR_TAP_TEMPO], device_data.parameter_value_max[USER_PAR_TAP_TEMPO], MIDI_channel, MIDI_out_port);
    }
  }
}

FLASHMEM void MD_USER_class::start_tuner() {
  if (connected) {
    if (device_data.parameter_CC[USER_PAR_TUNER] > 0) {
      MIDI_send_CC(device_data.parameter_CC[USER_PAR_TUNER], device_data.parameter_value_max[USER_PAR_TUNER], MIDI_channel, MIDI_out_port);
    }
  }
}

FLASHMEM void MD_USER_class::stop_tuner() {
  if (connected) {
    if (device_data.parameter_CC[USER_PAR_TUNER] > 0) {
      MIDI_send_CC(device_data.parameter_CC[USER_PAR_TUNER], device_data.parameter_value_min[USER_PAR_TUNER], MIDI_channel, MIDI_out_port);
    }
  }
}

// ********************************* Section 4: GEN program change ********************************************

FLASHMEM void MD_USER_class::select_patch(uint16_t new_patch) {
  //if (new_patch == patch_number) unmute();
  prev_patch_number = patch_number;
  patch_number = new_patch;
#define PC_TYPE_PC_ONLY 0

  switch (device_data.pc_type) {
    case PC_TYPE_PC_ONLY:
      MIDI_send_PC(new_patch & 0x7F, MIDI_channel, MIDI_out_port);
      break;
    case PC_TYPE_PC_AND_CC0:
      MIDI_send_CC(0, new_patch >> 7, MIDI_channel, MIDI_out_port);
      MIDI_send_PC(new_patch & 0x7F, MIDI_channel, MIDI_out_port);
      break;
    case PC_TYPE_PC_AND_CC32:
      MIDI_send_CC(32, new_patch >> 7, MIDI_channel, MIDI_out_port);
      MIDI_send_PC(new_patch & 0x7F, MIDI_channel, MIDI_out_port);
      break;
    case PC_TYPE_PC_AND_CC0_MOD100:
      MIDI_send_CC(0, new_patch / 100, MIDI_channel, MIDI_out_port);
      MIDI_send_PC(new_patch % 100, MIDI_channel, MIDI_out_port);
      break;
    case PC_TYPE_PC_AND_CC32_MOD100:
      MIDI_send_CC(32, new_patch / 100, MIDI_channel, MIDI_out_port);
      MIDI_send_PC(new_patch % 100, MIDI_channel, MIDI_out_port);
      break;
    case PC_TPE_CC60:
      MIDI_send_CC(60, new_patch & 0x7F, MIDI_channel, MIDI_out_port);
      break;
  }

  if (popup_patch_name) {
    if (LCD_check_popup_allowed(0)) {
      String pname;
      EEPROM_read_user_item_name(USER_DEVICE_PATCH_NAME_TYPE, my_instance, new_patch, pname);
      if (pname.trim() != "") LCD_show_popup_label(pname, ACTION_TIMER_LENGTH);
    }
    popup_patch_name = false;
  }

  DEBUGMSG("out(GEN) PC" + String(new_patch)); //Debug
  do_after_patch_selection();
  update_page = REFRESH_PAGE;
}

FLASHMEM void MD_USER_class::do_after_patch_selection() {
  is_on = connected;
  load_par_state();
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

FLASHMEM void MD_USER_class::number_format(uint16_t number, String & Output) {
  build_patch_number(number, Output, device_data.first_patch_format, device_data.last_patch_format);
}

FLASHMEM bool MD_USER_class::request_patch_name(uint8_t sw, uint16_t number) {
  // Read from EEPROM
  String pname;
  EEPROM_read_user_item_name(USER_DEVICE_PATCH_NAME_TYPE, my_instance, number, pname);
  LCD_set_SP_label(sw, pname);
  return true;
}

FLASHMEM void MD_USER_class::request_current_patch_name() {
  // Read from EEPROM
  EEPROM_read_user_item_name(USER_DEVICE_PATCH_NAME_TYPE, my_instance, patch_number, current_patch_name);
}
FLASHMEM void MD_USER_class::direct_select_format(uint16_t number, String & Output) {
  if (direct_select_state == 0) {
    Output += String(bank_select_number) + String(number) + "_";
  }
  else {
    Output +=  String(bank_select_number / 10) + String(bank_select_number % 10) + String(number);
  }
}

// ********************************* Section 5: GEN parameter control ********************************************
const char par_name[][17] = {
  "FX 1", "FX 2", "FX 3", "FX 4", "FX 5", "FX 6", "FX 7", "FX 8", "EXP1", "EXP2",
  "EXP TOGGLE", "TAP TEMPO", "TUNER", "SCENE SELECT", "STOP", "REC", "PLAY", "OVERDUB", "FORWARD", "REVERSE",
  "FULLSPEED", "HALFSPEED", "UNDO", "REDO", "PLAY ONCE"
};

FLASHMEM void MD_USER_class::parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {
  // Send cc command to User device
  if (number >= USER_NUMBER_OF_CC_PARAMETERS) return;
  if (check_cc_not_zero(number)) {
    uint8_t value = SCO_return_parameter_value(Sw, cmd);
    if (SP[Sw].Latch == TOGGLE) {
      if (value == 1) value = device_data.parameter_value_max[number];
      else value = device_data.parameter_value_min[number];
    }
    if ((SP[Sw].Latch == MOMENTARY) || (SP[Sw].Latch == ONE_SHOT)) {
      value = device_data.parameter_value_max[number];
    }

    my_parameter_values[number] = value;
    SP[Sw].Target_byte1 = value;

    /*if ((SP[Sw].Latch == STEP) || (SP[Sw].Latch == UPDOWN)) {
      value += device_data.parameter_value_min[number];
      }*/

    MIDI_send_CC(device_data.parameter_CC[number], value, MIDI_channel, MIDI_out_port);

    String msg = par_name[number];
    /*if (USER_CC_types[number].Sublist >= 1) {
      msg += ':';
      msg += USER_sublists[USER_CC_types[number].Sublist - 1 + value];
      }
      //} */
    if (SP[Sw].Latch == UPDOWN ) {
      msg += ':';
      msg += String(value);
    }
    if (LCD_check_popup_allowed(Sw)) LCD_show_popup_label(msg, ACTION_TIMER_LENGTH);
    update_page = REFRESH_FX_ONLY; // To update the other switch states, we re-load the current page
  }
  last_selected_parameter = number;
}

FLASHMEM void MD_USER_class::parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {
  if (device_data.parameter_type[number] == USER_MOMENTARY) {
    if (check_cc_not_zero(number)) {
      MIDI_send_CC(device_data.parameter_CC[number], device_data.parameter_value_min[number], MIDI_channel, MIDI_out_port);
      my_parameter_values[number] = 0;
    }
  }
}

FLASHMEM void MD_USER_class::read_parameter_title(uint16_t number, String &Output) {
  String str = "";
  if (EEPROM_read_user_item_name(USER_DEVICE_FX_NAME_TYPE, my_instance, (number << 11) + patch_number + 1, str)) {
    uint8_t last_char = 0;
    for (uint8_t s = 0; s < 12; s++) if ((str[s] > ' ') && (str[s] <= 'z')) last_char = s;
    Output += str.substring(0, last_char + 1);
    return;
  }

  if (EEPROM_read_user_item_name(USER_DEVICE_FX_NAME_TYPE, my_instance, (number << 11), str)) {
    uint8_t last_char = 0;
    for (uint8_t s = 0; s < 12; s++) if ((str[s] > ' ') && (str[s] <= 'z')) last_char = s;
    Output += str.substring(0, last_char + 1);
    return;
  }
  Output += par_name[number];
}

FLASHMEM bool MD_USER_class::request_parameter(uint8_t sw, uint16_t number) {
  SP[sw].Target_byte1 = my_parameter_values[number];
  uint8_t min_value = 0;
  if (number < USER_NUMBER_OF_CC_PARAMETERS_WITH_TYPE) min_value = device_data.parameter_value_min[number];
  if (my_parameter_values[number] == min_value) SP[sw].State = 2; // Effect off
  else if (device_data.parameter_CC[number] > 0) SP[sw].State = 1; // Effect on
  else SP[sw].State = 0; // Effect disabled
  SP[sw].Colour = load_fx_colour(number);

  String lbl = "";
  if (SP[sw].Latch == UPDOWN) LCD_add_3digit_number(SP[sw].Target_byte1, lbl);
  LCD_set_SP_label(sw, lbl);
  return true; // Move to next switch is true.
}

FLASHMEM void MD_USER_class::check_update_label(uint8_t Sw, uint8_t value) { // Updates the label for extended sublists
  String msg = "";
  //uint16_t index = SP[Sw].PP_number;
  LCD_set_SP_label(Sw, msg);

  //Update the current switch label
  update_lcd = Sw;
}

// Menu options for FX states
FLASHMEM void MD_USER_class::read_parameter_name(uint16_t number, String &Output) { // Called from menu
  if (number < number_of_parameters())  Output = par_name[number];
  else Output = "?";
}

FLASHMEM uint16_t MD_USER_class::number_of_parameters() {
  return USER_NUMBER_OF_CC_PARAMETERS;
}

FLASHMEM uint8_t MD_USER_class::number_of_values(uint16_t parameter) {
  if (parameter < USER_NUMBER_OF_CC_PARAMETERS_WITH_TYPE) {
    switch (device_data.parameter_type[parameter]) {
      case USER_MOMENTARY:
      case USER_TOGGLE:
        return 2;
      case USER_STEP:
      case USER_RANGE:
      case USER_UPDOWN:
        return device_data.parameter_value_max[parameter] - device_data.parameter_value_min[parameter] + 1;
    }
  }
  if (parameter < USER_NUMBER_OF_CC_PARAMETERS) return 1;
  else return 0;
}

FLASHMEM void MD_USER_class::read_parameter_value_name(uint16_t number, uint16_t value, String &Output) {
  if (number < number_of_parameters()) Output += String(value);
  else Output += " ? "; // Unknown parameter
}

FLASHMEM uint8_t MD_USER_class::get_toggle_type(uint16_t parameter) {
  if (parameter < USER_NUMBER_OF_CC_PARAMETERS_WITH_TYPE) {
    switch (device_data.parameter_type[parameter]) {
      case USER_MOMENTARY:
        return MOMENTARY;
      case USER_TOGGLE:
        return TOGGLE;
      case USER_STEP:
        return STEP;
      case USER_RANGE:
      //return RANGE;
      case USER_UPDOWN:
        return UPDOWN;
    }
  }
  if (parameter == USER_NUMBER_OF_CC_PARAMETERS_WITH_TYPE) return STEP; // For scene select
  return ONE_SHOT; // for all looper commands
}

FLASHMEM void MD_USER_class::set_par_data(uint8_t sw, uint16_t par_id) {
  if (device_data.parameter_CC[par_id] > 0) SP[sw].Latch = get_toggle_type(par_id);
  else SP[sw].Latch = MOMENTARY;
  SP[sw].Assign_min = device_data.parameter_value_min[par_id];
  SP[sw].Assign_max = device_data.parameter_value_max[par_id];
}

FLASHMEM void MD_USER_class::update_parameter_state_through_cc(uint8_t control, uint8_t value) {
  for (uint8_t i = 0; i < USER_NUMBER_OF_CC_PARAMETERS_WITH_TYPE; i++) {
    if (control == device_data.parameter_CC[i]) my_parameter_values[i] = value;
  }
}

FLASHMEM uint8_t MD_USER_class::get_par_state() {
  uint8_t state = 0;
  for (uint8_t p = 0; p < 8; p++) {
    if (my_parameter_values[p] == device_data.parameter_value_max[p]) bitSet(state, p);
    else bitClear(state, p);
  }
  return state;;
}

FLASHMEM void MD_USER_class::save_par_state() {
  EEPROM_store_user_item_name(USER_DEVICE_PATCH_NAME_TYPE , my_instance, patch_number, get_par_state(), current_patch_name);
}

FLASHMEM void MD_USER_class::load_par_state() {
  uint8_t state = EEPROM_read_user_par_state(1, my_instance, patch_number);
  for (uint8_t p = 0; p < 8; p++) {
    if ((state & (1 << p)) != 0) my_parameter_values[p] = device_data.parameter_value_max[p];
    else my_parameter_values[p] = device_data.parameter_value_min[p];
  }
}

FLASHMEM uint8_t MD_USER_class::load_fx_colour(uint16_t parameter) {
  if (parameter < 10) {
    uint8_t colour = EEPROM_read_user_par_state(2, my_instance, (parameter << 11) + patch_number + 1);
    if (colour > 0) return colour;
    colour = EEPROM_read_user_par_state(2, my_instance, (parameter << 11));
    if (colour > 0) return colour;
  }
  return my_LED_colour;
}

FLASHMEM bool MD_USER_class::check_cc_not_zero(uint8_t parameter) {
  uint8_t cc = device_data.parameter_CC[parameter];
  if (cc > 0) return true;
  LCD_show_popup_label("CC not set!", ACTION_TIMER_LENGTH);
  return false;
}

// ********************************* Section 6: GEN expression pedal control ********************************************

FLASHMEM void MD_USER_class::move_expression_pedal(uint8_t sw, uint8_t value, uint8_t exp_pedal) {
  uint8_t number;
  if (exp_pedal == 0) exp_pedal = current_exp_pedal;
  switch (exp_pedal) {
    case 1: number = USER_PAR_EXP1; break;
    case 2: number = USER_PAR_EXP2; break;
    default: return;
  }
  LCD_show_bar(0, value, 0); // Show it on the main display
  if (check_cc_not_zero(number)) {
    uint8_t val_map = map(value, 0, 127, device_data.parameter_value_min[number], device_data.parameter_value_max[number]);
    MIDI_send_CC(device_data.parameter_CC[number], val_map, MIDI_channel, MIDI_out_port);
    String msg = SP[sw].Label;
    msg.trim();
    msg += ':';
    LCD_add_3digit_number(val_map, msg);
    LCD_show_popup_label(msg, ACTION_TIMER_LENGTH);
  }
  last_selected_parameter = SP[sw].PP_number;
  update_page = REFRESH_FX_ONLY; // To update the other switch states, we re-load the current page
}

FLASHMEM void MD_USER_class::toggle_expression_pedal(uint8_t sw) {
  MD_base_class::toggle_expression_pedal(sw);

  uint8_t number;
  uint8_t toggle_val;
  switch (current_exp_pedal) {
    case 1:
      number = USER_PAR_EXP1;
      toggle_val = device_data.parameter_value_min[USER_PAR_EXP_TGL];
      break;
    case 2:
      number = USER_PAR_EXP2;
      toggle_val = device_data.parameter_value_max[USER_PAR_EXP_TGL];
      break;
    default:
      return;
  }
  last_selected_parameter = number;
  if (device_data.parameter_CC[USER_PAR_EXP_TGL] > 0) MIDI_send_CC(device_data.parameter_CC[USER_PAR_EXP_TGL], toggle_val, MIDI_channel, MIDI_out_port);
}

FLASHMEM bool MD_USER_class::request_exp_pedal(uint8_t sw, uint8_t exp_pedal) {
  if (exp_pedal == 0) exp_pedal = current_exp_pedal;
  switch (exp_pedal) {
    case 1: SP[sw].PP_number = USER_PAR_EXP1; break;
    case 2: SP[sw].PP_number = USER_PAR_EXP2; break;
    default: return true;
  }
  String lbl = "";
  read_parameter_title(SP[sw].PP_number, lbl);
  LCD_set_SP_label(sw, lbl);
  SP[sw].Colour = load_fx_colour(SP[sw].PP_number);
  return true;
}

// Looper control
FLASHMEM bool MD_USER_class::looper_active() {
  return (device_data.parameter_CC[USER_PAR_LOOPER_REC] > 0);
}

FLASHMEM bool MD_USER_class::send_looper_cmd(uint8_t cmd) {
  if ((cmd >= 2) && (cmd <= 12)) {
    if (device_data.parameter_CC[cmd + 12] > 0) {
      MIDI_send_CC(device_data.parameter_CC[cmd + 12], device_data.parameter_value_max[cmd + 12], MIDI_channel, MIDI_out_port);
      return true;
    }
  }
  return false;
}

// Scenes
FLASHMEM bool MD_USER_class::request_snapscene_name(uint8_t sw, uint8_t sw1, uint8_t sw2, uint8_t sw3) {
  String lbl = "";
  if ((sw2 == 0) && (sw3 == 0)) { // One scene under switch
    read_scene_name_from_buffer(sw1);
    LCD_add_char_to_string(scene_label_buffer, lbl, 16);
  }
  if ((sw2 > 0) && (sw3 == 0)) { // Two scenes under switch
    read_scene_name_from_buffer(sw1);
    LCD_add_char_to_string(scene_label_buffer, lbl, 8);
    lbl += ' ';
    read_scene_name_from_buffer(sw2);
    LCD_add_char_to_string(scene_label_buffer, lbl, 7);
  }
  if ((sw2 > 0) && (sw3 > 0)) { // Three scenes under switch
    read_scene_name_from_buffer(sw1);
    LCD_add_char_to_string(scene_label_buffer, lbl, 5);
    lbl += ' ';
    read_scene_name_from_buffer(sw2);
    LCD_add_char_to_string(scene_label_buffer, lbl, 5);
    lbl += ' ';
    read_scene_name_from_buffer(sw3);
    LCD_add_char_to_string(scene_label_buffer, lbl, 4);
  }
  LCD_set_SP_label(sw, lbl);
  return true;
}

FLASHMEM void MD_USER_class::get_snapscene_title(uint8_t number, String &Output) {
  Output += "SCENE " + String(number);
}

FLASHMEM void MD_USER_class::get_snapscene_label(uint8_t number, String &Output) {
  if (number == 0) {
    Output += "--";
  }
  String str = "";
  if (EEPROM_read_user_item_name(USER_DEVICE_SCENE_NAME_TYPE, my_instance, (number << 11) + patch_number + 1, str)) {
    uint8_t last_char = 0;
    for (uint8_t s = 0; s < 12; s++) if ((str[s] > ' ') && (str[s] <= 'z')) last_char = s;
    Output += str.substring(0, last_char + 1);
    return;
  }

  if (EEPROM_read_user_item_name(USER_DEVICE_SCENE_NAME_TYPE, my_instance, (number << 11), str)) {
    uint8_t last_char = 0;
    for (uint8_t s = 0; s < 12; s++) if ((str[s] > ' ') && (str[s] <= 'z')) last_char = s;
    Output += str.substring(0, last_char + 1);
    return;
  }
}

FLASHMEM void MD_USER_class::set_snapscene(uint8_t sw, uint8_t number) {
  if (device_data.parameter_CC[USER_PAR_SCENE_SELECT] == 0) return;
  if (!is_on) unmute();
  if ((number < 1) || (number > (device_data.parameter_value_max[USER_PAR_SCENE_SELECT] + 1))) return;
  current_snapscene = number;
  MIDI_send_CC(device_data.parameter_CC[USER_PAR_SCENE_SELECT], number - 1, MIDI_channel, MIDI_out_port);
  MIDI_send_current_snapscene(my_device_number, current_snapscene);
  if (sw > 0) {
    String msg = "[S" + String(number) + "] ";
    get_snapscene_label(number, msg);
    if (LCD_check_popup_allowed(sw)) LCD_show_popup_label(msg, MESSAGE_TIMER_LENGTH);
  }
}

/*FLASHMEM void MD_USER_class::show_snapscene(uint8_t number) {
  if ((number < 1) || (number > 8)) return;
  if (number == current_snapscene) return;
  current_snapscene = number;
  }*/

FLASHMEM uint8_t MD_USER_class::get_number_of_snapscenes() {
  if (device_data.parameter_CC[USER_PAR_SCENE_SELECT] > 0) {
    return device_data.parameter_value_max[USER_PAR_SCENE_SELECT] + 1;
  }
  else {
    return 0;
  }
}

FLASHMEM void MD_USER_class::read_scene_name_from_buffer(uint8_t scene) {
  String lbl;
  get_snapscene_label(scene, lbl);
  for (uint8_t c = 0; c < 12; c++) {
    scene_label_buffer[c] = lbl[c];
  }
}

// Menu edit function

FLASHMEM void MD_USER_class::edit() {
  USER_selected_device = my_device_number;
  selected_parameter_colour = load_fx_colour(last_selected_parameter);
  DEBUGMSG("Edit started in class for device " + String(USER_selected_device));
  memcpy(&USER_current_device_data, &device_data, USER_DATA_SIZE);
  USER_current_device_data_patch_min = (device_data.patch_min_msb << 8) + device_data.patch_min_lsb;
  USER_current_device_data_patch_max = (device_data.patch_max_msb << 8) + device_data.patch_max_lsb;

  open_specific_menu = my_device_number + 1;
  SCO_select_page(PAGE_MENU); // Open the menu
}

FLASHMEM void MD_USER_class::edit_done() {
  DEBUGMSG("Edit done in class for device " + String(USER_selected_device));
  memcpy(&device_data, &USER_current_device_data, USER_DATA_SIZE);
  device_data.patch_min_msb = USER_current_device_data_patch_min >> 8;
  device_data.patch_min_lsb = USER_current_device_data_patch_min & 0xFF;
  device_data.patch_max_msb = USER_current_device_data_patch_max >> 8;
  device_data.patch_max_lsb = USER_current_device_data_patch_max & 0xFF;
  EEPROM_store_user_device_data(my_instance, &device_data);
  init_from_device_data();
  max_looper_length = device_data.looper_length * 1000000;
  update_page = RELOAD_PAGE;
}
