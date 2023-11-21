// Please read VController_v3.ino for information about the license and authors

// Class structure for NUX MG300

// This page has the following parts:
// Section 1: MG300 Initialization
// Section 2: MG300 common MIDI in functions
// Section 3: MG300 common MIDI out functions
// Section 4: MG300 program change
// Section 5: MG300 parameter control
// Section 6: MG300 expression pedal control

// ********************************* Section 1: MG300 Initialization ********************************************

// NUX MG300 settings:
#define MG300_MIDI_CHANNEL 1
#define MG300_MIDI_PORT USBHMIDI_PORT
#define MG300_PATCH_MIN 0
#define MG300_PATCH_MAX 71

#define MG300_CURRENT_PATCH_REQUEST_TIME 100

FLASHMEM void MD_MG300_class::init() { // Default values for variables
  MD_base_class::init();

  // NUX MG300 variables:
  enabled = DEVICE_DETECT; // NUX MG300 can be detected via USB, but not via regular MIDI...
  strcpy(device_name, "MG300");
  strcpy(full_device_name, "NUX MG300");
  patch_min = MG300_PATCH_MIN;
  patch_max = MG300_PATCH_MAX;
  max_times_no_response = MAX_TIMES_NO_RESPONSE; // The number of times the MG300 does not have to respond before disconnection
  sysex_delay_length = 5; // time between sysex messages (in msec).
  my_LED_colour = 6; // Default value: white
  MIDI_channel = MG300_MIDI_CHANNEL; // Default value
  MIDI_port_manual = MIDI_port_number(MG300_MIDI_PORT); // Default value
#if defined(CONFIG_VCTOUCH)
  my_device_page1 = MG300_DEFAULT_VCTOUCH_PAGE1; // Default value
  my_device_page2 = MG300_DEFAULT_VCTOUCH_PAGE2; // Default value
  my_device_page3 = MG300_DEFAULT_VCTOUCH_PAGE3; // Default value
  my_device_page4 = MG300_DEFAULT_VCTOUCH_PAGE4; // Default value
#elif defined(CONFIG_VCMINI)
  my_device_page1 = MG300_DEFAULT_VCMINI_PAGE1; // Default value
  my_device_page2 = MG300_DEFAULT_VCMINI_PAGE2; // Default value
  my_device_page3 = MG300_DEFAULT_VCMINI_PAGE3; // Default value
  my_device_page4 = MG300_DEFAULT_VCMINI_PAGE4; // Default value
#elif defined (CONFIG_CUSTOM)
  my_device_page1 = MG300_DEFAULT_CUSTOM_PAGE1; // Default value
  my_device_page2 = MG300_DEFAULT_CUSTOM_PAGE2; // Default value
  my_device_page3 = MG300_DEFAULT_CUSTOM_PAGE3; // Default value
  my_device_page4 = MG300_DEFAULT_CUSTOM_PAGE4; // Default value
#else
  my_device_page1 = MG300_DEFAULT_VC_PAGE1; // Default value
  my_device_page2 = MG300_DEFAULT_VC_PAGE2; // Default value
  my_device_page3 = MG300_DEFAULT_VC_PAGE3; // Default value
  my_device_page4 = MG300_DEFAULT_VC_PAGE4; // Default value
#endif

#ifdef IS_VCTOUCH
  device_pic = img_MG300;
#endif
}

FLASHMEM void MD_MG300_class::update() {
  if (current_patch_request_timer > 0) {
    if (millis() > current_patch_request_timer) request_current_patch();
  }
}

// ********************************* Section 2: MG300 common MIDI in functions ********************************************
FLASHMEM void MD_MG300_class::check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port) { // Check incoming sysex messages from  Called from MIDI:OnSysEx/OnSerialSysEx
  // Check if it is a message from a NUX MG300 (message starting with F0 00 11 22 66 00 70 )
  if ((sxdata[2] == 0x11) && (sxdata[3] == 0x22) && (sxdata[4] == 0x66)) {
    if ((port == MIDI_in_port) && (sxdata[6] == 0x70)) {

      // Check if it is patch data for a specific patch
      if ((sxdata[7] == 0x06)  && (sxdata[8] == last_requested_sysex_patch_number)) {
        if  ((sxlength == 112) || (sxlength == 113)) { // Check length - latest firmware added one byte.
          // Read the patch name
          uint8_t len = sxdata[0x4D]; // Length of patch name is variable.
          if (len > 16) len = 16;
          for (uint8_t count = 0; count < len; count++) {
            SP[last_requested_sysex_switch].Label[count] = convert_char(sxdata[count + 0x4E]);
          }
          for (uint8_t count = len; count < 16; count++) { // Fill the rest with spaces
            SP[last_requested_sysex_switch].Label[count] = ' ';
          }
          DEBUGMSG (SP[last_requested_sysex_switch].Label);

          PAGE_request_next_switch();
        }
        else { // Data is not complete
          PAGE_request_current_switch(); // Retry reading data for current switch
        }
      }

      // Check if it is patch data for the current patch
      if (sxdata[7] == 0x12) {
        if  ((sxlength == 112) || (sxlength == 113)) { // Check length - latest firmware added one byte.
          read_patch_name(sxdata, sxlength);

          // Read FX types
          read_FX_state_from_memory();
          fx_type[0] = sxdata[9]; // Type of expression pedal
          fx_state[0] = 0;
          for (uint8_t fx_no = 1; fx_no < MG300_NUMBER_OF_FX; fx_no++) {
            uint8_t type = sxdata[9 + fx_no];
            if ((fx_no == 3) && (type > 26)) type = 26; // Catch the STAGEMAN amp type, which has a higher number here, but can be set by CC#3 value 26
            if (type > 0) fx_type[fx_no] = type;
            if (type > 0) fx_state[fx_no] = 1;
            else fx_state[fx_no] = 0;
            DEBUGMSG("FX type " + String(fx_no) + ": " + String(fx_type[fx_no]));
          }
          current_patch_read = true;
          update_page = REFRESH_PAGE;
          update_LEDS = true;
          current_patch_request_timer = 0;
        }
        else { // Data is not complete
          request_current_patch();
        }
      }

      if (sxdata[7] == 0x18) { // Receiving device state
        if (patch_number != sxdata[8]) { // Set current patch
          set_patch_number(sxdata[8]);
          do_after_patch_selection();
          update_page = REFRESH_PAGE;
        }
      }

      if (sxdata[7] == 0x20) { // Change of FX type on MG300
        uint8_t FX_no = sxdata[9];
        uint8_t FX_type = sxdata[10];
        fx_type[FX_no] = FX_type;
        fx_state[FX_no] = 1;
        update_page = REFRESH_PAGE;
      }

      if (sxdata[7] == 0x1D) { // Change of amp type on MG300
        uint8_t FX_no = 3; // Amp type
        uint8_t FX_type = sxdata[10];
        if (FX_type > 26) FX_type = 26; // Catch the STAGEMAN amp type, which has a higher number here, but can be set by CC#3 value 26
        fx_type[FX_no] = FX_type;
        fx_state[FX_no] = 1;
        update_page = REFRESH_PAGE;
      }

      if (sxdata[7] == 0x0E) { // Patch is being saved on the MG300
        read_patch_name(sxdata, sxlength); // So patchname and checksum are current in case the name has been edited on the MG300
        EEPROM_store_MG300_effect_types(sxdata[8], current_checksum); // Also store the effect types on the VContoller/VC-mini
        LCD_show_popup_label("Patch saved.", MESSAGE_TIMER_LENGTH);
        DEBUGMSG("MG-300 saving effect states for patch " + String (sxdata[8]));
        // Read FX types
        fx_type[0] = sxdata[9]; // Type of expression pedal
        update_page = REFRESH_PAGE;
      }
    }

    if (sxdata[6] == 0x10) { // Check for an identity response
      // Not sure how many bytes to check as the response is quite long. We check for
      // F0 00 11 22 66 00 10 56  30 31 5F 32 30 31 39 31 31 30 36
      if ((sxdata[7] == 0x56) && (sxdata[8] == 0x30) && (sxdata[9] == 0x31) && (sxdata[10] == 0x5F) && (sxdata[11] == 0x32)
          && (sxdata[12] == 0x30) && (sxdata[13] == 0x31) && (sxdata[14] == 0x39) && (sxdata[15] == 0x31) && (sxdata[16] == 0x31)
          && (sxdata[17] == 0x30) && (sxdata[18] == 0x36) && (enabled == DEVICE_DETECT)) {
        if (!connected) connect(sxdata[5], port, Current_MIDI_out_port);
        no_response_counter = 0;
      }
    }
  }
}

FLASHMEM void MD_MG300_class::read_patch_name(const unsigned char* sxdata, short unsigned int sxlength) {
  uint8_t len = sxdata[0x4D]; // Length of patch name is variable.
  current_checksum = 0;
  if (len > 16) len = 16;
  for (uint8_t count = 0; count < len; count++) {
    current_patch_name[count] = convert_char(sxdata[count + 0x4E]);
    current_checksum += sxdata[count + 0x4E];
  }
  for (uint8_t count = len; count < 16; count++) { // Fill the rest with spaces
    current_patch_name[count] = ' ';
  }
  update_main_lcd = true; // And show it on the main LCD
  if (popup_patch_name) {
    if (LCD_check_popup_allowed(0)) LCD_show_popup_label(current_patch_name, ACTION_TIMER_LENGTH);
    popup_patch_name = false;
  }
}

char MD_MG300_class::convert_char(uint8_t data) { // Patch names are not sent as ASCII
  const char other_chars[] = {' ', '?', '!', '-', '\'', ':', '&', '@', ',', '.'};
  if (data < 26) return data + 'A';
  if (data < 52) return data - 26 + 'a';
  if (data < 62) return data - 52 + '0';
  if (data < 72) return other_chars[data - 62];
  return '?';
}

FLASHMEM void MD_MG300_class::check_CC_in(uint8_t control, uint8_t value, uint8_t channel, uint8_t port) {
  if (!connected) return;
  if ((port == MIDI_in_port) && (channel == MIDI_channel)) {
    if (control == 60) { // patch change from MG300
      if (patch_number != value) {
        set_patch_number(value);
        do_after_patch_selection();
        //update_page = REFRESH_PAGE;
      }
    }
    else update_parameter_state_through_cc(control, value);
  }
}

// Detection of NUX MG300
// Very messy - the MG300 basically bounces any midi sysex messages it cannot interpret and adds a string of zero's.

FLASHMEM void MD_MG300_class::send_alternative_identity_request(uint8_t check_device_no) {
  if (enabled == DEVICE_DETECT) { // F0 00 11 22 66 00 00 00  00 F7
    uint8_t sysexmessage[10] = {0xF0, 0x00, 0x11, 0x22, 0x66, 0x00, 0x00, 0x00, 0x00, 0xF7}; // Will get any NUX device to respond - The NUX editor uses this command as well
    check_sysex_delay();
    MIDI_send_sysex(sysexmessage, 10, Current_MIDI_out_port);
  }
}

FLASHMEM void MD_MG300_class::do_after_connect() {
  request_device_state();
  do_after_patch_selection();
  //  current_exp_pedal = 2;
}

// ********************************* Section 3: MG300 common MIDI out functions ********************************************

FLASHMEM void MD_MG300_class::request_patch(uint8_t number) { //Will request the complete patch information from the MG300 (will receive 120 bytes as an answer)
  // F0 00 11 22 66 00 70 07  00 F7
  uint8_t sysexmessage[10] = {0xF0, 0x00, 0x11, 0x22, 0x66, 0x00, 0x70, 0x07, number, 0xF7};
  check_sysex_delay();
  last_requested_sysex_patch_number = number;
  MIDI_send_sysex(sysexmessage, 10, MIDI_out_port);
}

FLASHMEM void MD_MG300_class::request_current_patch() { //Will request the complete patch information from the MG300 (will receive 120 bytes as an answer)
  // F0 00 11 22 66 00 70 13 00 F7
  DEBUGMAIN("REQUESTING CURRENT PATCH");
  uint8_t sysexmessage[10] = {0xF0, 0x00, 0x11, 0x22, 0x66, 0x00, 0x70, 0x13, 0x00, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 10, MIDI_out_port);
  current_patch_request_timer = millis() + MG300_CURRENT_PATCH_REQUEST_TIME;
}
FLASHMEM void MD_MG300_class::request_device_state() {
  // F0 00 11 22 66 00 70 19  00 F7
  uint8_t sysexmessage[10] = {0xF0, 0x00, 0x11, 0x22, 0x66, 0x00, 0x70, 0x19, 0x00, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 10, MIDI_out_port);
  delay(5); // MG300 responds slow to this message
}

FLASHMEM void MD_MG300_class::set_bpm() {
  // F0 00 11 22 66 00 70 17  05 0C 00 F7
  if (connected) {
    uint8_t sysexmessage[12] = {0xF0, 0x00, 0x11, 0x22, 0x66, 0x00, 0x70, 0x17, 0x05, (uint8_t)(Setting.Bpm >> 4), (uint8_t)(Setting.Bpm & 0x0F), 0xF7};
    check_sysex_delay();
    MIDI_send_sysex(sysexmessage, 12, MIDI_out_port);
    //expr_pedal_cc++;
    //if (expr_pedal_cc > 127) expr_pedal_cc = 0;
  }
}

// ********************************* Section 4: MG300 program change ********************************************

FLASHMEM void MD_MG300_class::select_patch(uint16_t new_patch) {
  //if (new_patch == patch_number) unmute();
  prev_patch_number = patch_number;
  patch_number = new_patch;

  MIDI_send_CC(60, new_patch, MIDI_channel, MIDI_out_port);
  DEBUGMSG("out(MG300) CC #60 " + String(new_patch)); //Debug
  do_after_patch_selection();
  update_page = REFRESH_PAGE;
}

FLASHMEM void MD_MG300_class::do_after_patch_selection() {
  is_on = connected;
  if (Setting.Send_global_tempo_after_patch_change == true) {
    delay(5);
    set_bpm();
  }
  current_patch_read = false;
  request_current_patch();
  update_LEDS = true;
  update_main_lcd = true;
  if ((flash_bank_of_four != 255) && (Current_page != read_current_device_page())) SCO_select_page(read_current_device_page()); // When in direct select, go back to the current_device_page
  flash_bank_of_four = 255;
  MD_base_class::do_after_patch_selection();
}

FLASHMEM bool MD_MG300_class::request_patch_name(uint8_t sw, uint16_t number) {
  DEBUGMSG("Requesting patch " + String(number));
  if (number > patch_max) return true;
  last_requested_sysex_switch = sw;
  request_patch(number);
  return false;
}

FLASHMEM void MD_MG300_class::number_format(uint16_t number, String & Output) {
  char patchChar = (number % 4) + 65;
  if (number < 36) Output += String(number / 4 + 1) + patchChar; // For user patches
  else Output += 'F' + String(number / 4 - 8) + patchChar; // For factory patches
}

FLASHMEM void MD_MG300_class::direct_select_format(uint16_t number, String & Output) {
  if (direct_select_state == 0) {
    if (bank_select_number == 0) Output += String(number) + "_";
    else Output += 'F' + String(number) + "_";
  }
  else {
    char patchChar = (number % 4) + 65;
    if (number < 36) Output += String(bank_select_number + 1) + patchChar;
    else Output += 'F' + String(bank_select_number - 8) + patchChar;
  }
}

FLASHMEM bool MD_MG300_class::flash_LEDs_for_patch_bank_switch(uint8_t sw) { // Will flash the LEDs in banks of three when coming from direct select mode.
  if (!bank_selection_active()) return false;

  if (flash_bank_of_four == 255) return true; // We are not coming from direct select, so all LEDs should flash

  bool in_right_bank_of_eight = (flash_bank_of_four / 2 == SP[sw].PP_number / 8); // Going bank up and down coming from direct select will make all LEDs flash in other banks
  if (!in_right_bank_of_eight) return true;

  bool in_right_bank_of_four = (flash_bank_of_four == SP[sw].PP_number / 4); // Only flash the four LEDs of the corresponding bank
  if (in_right_bank_of_four) return true;
  return false;
}

FLASHMEM bool MD_MG300_class::valid_direct_select_switch(uint8_t number) {
  bool result = false;
  if (number == 0) return false;
  if (number < 10) result = true;
  return result;
}

FLASHMEM void MD_MG300_class::direct_select_start() {
  Previous_bank_size = bank_size; // Remember the bank size
  device_in_bank_selection = my_device_number + 1;
  bank_size = 36;
  bank_select_number = 0; // Reset bank number
  direct_select_state = 0;
}

FLASHMEM void MD_MG300_class::direct_select_press(uint8_t number) {
  if (!valid_direct_select_switch(number)) return;
  // Digit pressed
  uint16_t base_patch = (bank_select_number * 36) + (number - 1) * 4;
  flash_bank_of_four = base_patch / 4;
  bank_select_number = (base_patch / Previous_bank_size);
  bank_size = Previous_bank_size;
  SCO_select_page(my_device_page1); // Which should give PAGE_MG300_PATCH_BANK
  device_in_bank_selection = my_device_number + 1; // Go into bank mode
}

// ********************************* Section 5: MG300 parameter control ********************************************
// Define array for MG300 effect names and colours
struct MG300_CC_type_struct { // Combines all the data we need for controlling a parameter in a device
  char Name[6];
  uint8_t CC;
  uint8_t NumVals;
  uint8_t Sublist;
  uint8_t Colour;
};

const PROGMEM MG300_CC_type_struct MG300_CC_types[MG300_NUMBER_OF_FX] = {
  {"PEDAL", 62, 6, 1, FX_WAH_TYPE},
  {"CMP", 1, 2, 7, FX_DYNAMICS_TYPE},
  {"EFX", 2, 13, 9, FX_DIST_TYPE},
  {"AMP", 3, 27, 22, FX_AMP_TYPE},
  {"EQ", 4, 2, 49, FX_FILTER_TYPE},
  {"NR", 5, 2, 51, FX_DYNAMICS_TYPE},
  {"MOD", 6, 14, 53, FX_MODULATE_TYPE},
  {"DLY", 7, 7, 67, FX_DELAY_TYPE},
  {"RVB", 8, 5, 74, FX_REVERB_TYPE},
};

const PROGMEM char MG300_sublists[][11] = {
  // 1 - 6: WAH types
  "WAH", "EFX", "AMP", "MOD", "DLY", "RVB",

  // 7 - 8: CMP types
  "--", "DYNA CMP",

  // 9 - 21 EFX TYPES
  "--", "DIST+", "RC BST", "AC BST", "DIST UNO", "T SCREAMER", "BLUES DRIV", "MORNING D", "MODERN DST", "RED DIST", "CRUNCH", "MUF DIST", "PITCH BNDR",

  // 22 - 48 AMP TYPES
  "--", "JAZZ CLEAN", "DELUXE RVB", "BASS MATE", "TWEEDY", "TWIN RVB", "HIWIRE", "CALI CRNCH", "CLASS A15", "CLASS A30", "PLEXI 100W",
  "PLEXI 45", "BRIT 800", "1987X 50W", "SLO 100", "FRIEDMAN", "DUAL RECT", "DIE VH4", "VIBRO KING", "BUDDA", "MR.Z 38",
  "SUPER RVB", "BRIT BLUES", "MATCH D30", "BRIT 2000", "UBER HIGN", "STAGEMAN",

  // 49 - 50: EQ TYPES
  "--", "6-BAND EQ",

  // 51 - 52: NR TYPES
  "--", "NR",

  // 53 - 66: MOD TYPES
  "--", "CE-1", "CLASSIC CH", "ST.CHORUS", "VIBRATO", "DETUNE", "FLANGE", "PH90", "PHASE 100", "CHORUS+", "U_VIBE", "TREM", "RTY SPK", "HARMONY",

  // 67 - 73: DLY TYPES
  "--", "ANALOG", "DIGITAL", "MODULATE", "TAPE ECHO", "REVERSE", "PAN",

  // RVB TYPES
  "--", "ROOM", "HALL", "PLATE", "SPRING",
};

FLASHMEM void MD_MG300_class::parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {
  // Send cc command to NUX MG300
  uint8_t value = SCO_return_parameter_value(Sw, cmd);
  if (value == 1) {
    uint8_t on_value = fx_type[number];
    if (number == 0) on_value = 1; // For wah/expression pedal switch
    MIDI_send_CC(MG300_CC_types[number].CC, on_value, MIDI_channel, MIDI_out_port);
  }
  else {
    MIDI_send_CC(MG300_CC_types[number].CC, 0, MIDI_channel, MIDI_out_port);
  }
  fx_state[number] = value;
  if (number == 0) MIDI_send_CC(0, fx_type[0], MIDI_channel, MIDI_out_port); // Reset expression pedal type to original value after switching wah
  if (number < 9) MIDI_send_CC(66, number, MIDI_channel, MIDI_out_port); // Select this effect in edit mode on the MG300

  if (LCD_check_popup_allowed(Sw)) LCD_show_popup_label(SP[Sw].Label, ACTION_TIMER_LENGTH);
  update_page = REFRESH_FX_ONLY; // To update the other switch states, we re-load the current page
}

FLASHMEM void MD_MG300_class::parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {
  if (SP[Sw].Latch == MOMENTARY) {
    MIDI_send_CC(MG300_CC_types[number].CC, 0, MIDI_channel, MIDI_out_port);
    fx_state[number] = 0;
    if (number == 0) MIDI_send_CC(0, fx_type[0], MIDI_channel, MIDI_out_port); // Reset expression pedal type to original value after switching wah
  }
}

FLASHMEM void MD_MG300_class::read_parameter_title(uint16_t number, String &Output) {
  Output += MG300_CC_types[number].Name;
}

FLASHMEM bool MD_MG300_class::request_parameter(uint8_t sw, uint16_t number) {
  String msg = "";
  if ((fx_type[number] > 0) || (number == 0)) {
    if (fx_state[number] == 0) SP[sw].State = 2; // Effect off
    else SP[sw].State = 1; // Effect on

    SP[sw].Colour = MG300_CC_types[number].Colour;
  }
  else {
    SP[sw].State = 0; // Effect off
    SP[sw].Colour = FX_TYPE_OFF; // LED off
  }

  //msg = MG300_CC_types[number].Name;
  if (MG300_CC_types[number].Sublist >= 1) {
    //msg += ':';
    msg += MG300_sublists[MG300_CC_types[number].Sublist - 1 + fx_type[number]];
  }
  LCD_set_SP_label(sw, msg);
  return true; // Move to next switch is true.
}

// Menu options for FX states
FLASHMEM void MD_MG300_class::read_parameter_name(uint16_t number, String &Output) { // Called from menu
  if (number < number_of_parameters())  Output = MG300_CC_types[number].Name;
  else Output = "?";
}

FLASHMEM uint16_t MD_MG300_class::number_of_parameters() {
  return MG300_NUMBER_OF_FX;
}

FLASHMEM uint8_t MD_MG300_class::number_of_values(uint16_t parameter) {
  if (parameter < MG300_NUMBER_OF_FX) {
    return 2;
  }
  else return 0;
}

FLASHMEM void MD_MG300_class::read_parameter_value_name(uint16_t number, uint16_t value, String &Output) {
  if (number < number_of_parameters()) Output += String(value);
  else Output += " ? "; // Unknown parameter
}

FLASHMEM void MD_MG300_class::update_parameter_state_through_cc(uint8_t control, uint8_t value) {
  for (uint8_t i = 0; i < MG300_NUMBER_OF_FX; i++) {
    if (control == MG300_CC_types[i].CC) {
      if (i > 0) fx_type[i] = value; // Never switch off the wah
      fx_state[i] = value;
      update_page = REFRESH_PAGE;
    }
  }
}

FLASHMEM void MD_MG300_class::read_FX_state_from_memory() {
  if (!EEPROM_load_MG300_effect_types(patch_number, current_checksum)) {
    for (uint8_t p = 0; p < EXT_MAX_NUMBER_OF_MG300_PRESETS; p++) { // Run through all patches and see if we can find one with the same checksum
      if (EEPROM_load_MG300_effect_types(p, current_checksum)) {
        EEPROM_exchange_MG300_patches(p, patch_number);
        return;
      }
    }
    EEPROM_clear_MG300_message_array();
  }
}

// ********************************* Section 6: MG300 expression pedal control ********************************************

/*FLASHMEM void MD_MG300_class::move_expression_pedal(uint8_t sw, uint8_t value, uint8_t exp_pedal) {
  //uint8_t number;
  //number = 0;
  LCD_show_bar(0, value); // Show it on the main display
  MIDI_send_CC(61, value, MIDI_channel, MIDI_out_port);
  //check_update_label(sw, value);
  String msg = "CC";
  msg += ':';
  //LCD_add_3digit_number(expr_pedal_cc, msg);
  LCD_show_popup_label(msg, ACTION_TIMER_LENGTH);

  update_page = REFRESH_FX_ONLY; // To update the other switch states, we re-load the current page
  }

  FLASHMEM bool MD_MG300_class::request_exp_pedal(uint8_t sw, uint8_t exp_pedal) {
  SP[sw].PP_number = 0;
  LCD_clear_SP_label(sw);
  return true;
  }*/
