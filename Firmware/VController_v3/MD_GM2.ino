// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: TC-electronics G-Major 2 Initialization
// Section 2: TC-electronics G-Major 2 common MIDI in functions
// Section 3: TC-electronics G-Major 2 common MIDI out functions
// Section 4: TC-electronics G-Major 2 program change
// Section 5: TC-electronics G-Major 2 parameter control
// Section 6: TC-electronics G-Major 2 expression pedal control

// ********************************* Section 1: TC-electronics G-Major 2 initialization ********************************************

// TC-electronics G-Major 2 settings:
#define GM2_MIDI_CHANNEL 1
#define GM2_MIDI_PORT MIDI1_PORT
#define GM2_PATCH_MIN 0
#define GM2_PATCH_MAX 199
#define GM2_MIDI_TIMER_LENGTH 100 // Minimal time inbetween requests for patch data from MS70

// Documentation of TC Electronics GMajor 2 sysex has been moved to http://www.vguitarforums.com/smf/index.php?topic=4329.msg131444#msg131444 (The TC-electronics G-Major 2 v2 MIDI specification)
// The relevant messages are repeated here
// 1) The Zoom responds to an MIDI identity request message with F0 7E 00 (Device ID) 00 20 1F (Manufacturing ID for TCE) 66 (model number GM2) 00  00 00 32 2E 31 30 F7
#define GM2_MODEL_NUMBER 0x66
// 2) To request a patch: F0 00 20 1F 7F 66 45 65 00 00 F7 => 65 00 is patch number 101 in hex.
//    Reply: F0 00 20 1F 7F 66 20 65 00 + patch data
#define GM2_PATCH_REQUEST 0x45
#define GM2_PATCH_RESPONSE 0x20

#define GM2_TUNER_CC 50 // should be programmed on the G-Major 2 - is there a default value?
#define GM2_MODIFIER1_CC 51
#define GM2_MODIFIER2_CC 52

// Initialize device variables
// Called at startup of VController
void MD_GM2_class::init() // Default values for variables
{
  MD_base_class::init();

  // Variables
  enabled = DEVICE_DETECT; // Default value
  strcpy(device_name, "GM2");
  strcpy(full_device_name, "TC-E G-Major 2");
  patch_number_offset = 1; // First patch is numbered one
  patch_min = GM2_PATCH_MIN;
  patch_max = GM2_PATCH_MAX;
  sysex_delay_length = 0; // time between sysex messages (in msec).
  max_times_no_response = MAX_TIMES_NO_RESPONSE; // The number of times the TC-electronics G-Major 2 does not have to respond before disconnection
  my_LED_colour = 1; // Default value: green
  MIDI_channel = GM2_MIDI_CHANNEL; // Default value
  MIDI_port_manual = MIDI_port_number(GM2_MIDI_PORT); // Default value
  //bank_number = 0; // Default value
#if defined(CONFIG_VCTOUCH)
  my_device_page1 = GM2_DEFAULT_VCTOUCH_PAGE1; // Default value
  my_device_page2 = GM2_DEFAULT_VCTOUCH_PAGE2; // Default value
  my_device_page3 = GM2_DEFAULT_VCTOUCH_PAGE3; // Default value
  my_device_page4 = GM2_DEFAULT_VCTOUCH_PAGE4; // Default value
#elif defined(CONFIG_VCMINI)
  my_device_page1 = GM2_DEFAULT_VCMINI_PAGE1; // Default value
  my_device_page2 = GM2_DEFAULT_VCMINI_PAGE2; // Default value
  my_device_page3 = GM2_DEFAULT_VCMINI_PAGE3; // Default value
  my_device_page4 = GM2_DEFAULT_VCMINI_PAGE4; // Default value
#elif defined (CONFIG_CUSTOM)
  my_device_page1 = GM2_DEFAULT_CUSTOM_PAGE1; // Default value
  my_device_page2 = GM2_DEFAULT_CUSTOM_PAGE2; // Default value
  my_device_page3 = GM2_DEFAULT_CUSTOM_PAGE3; // Default value
  my_device_page4 = GM2_DEFAULT_CUSTOM_PAGE4; // Default value
#else
  my_device_page1 = GM2_DEFAULT_VC_PAGE1; // Default value
  my_device_page2 = GM2_DEFAULT_VC_PAGE2; // Default value
  my_device_page3 = GM2_DEFAULT_VC_PAGE3; // Default value
  my_device_page4 = GM2_DEFAULT_VC_PAGE4; // Default value
#endif
  current_exp_pedal = 1;

  for (uint8_t i = 0; i < GM2_NUMBER_OF_FX; i++) {
    FX_state[i] = 1;
    FX_type[i] = 0;
  }

#ifdef IS_VCTOUCH
  device_pic = img_GM2;
#endif
}

void MD_GM2_class::update() {
  if ((send_patch_change) && (millis() > midi_timer)) { // We delay the sending of a PC message if these come to close together.
    MIDI_send_CC(0, patch_number >> 7, MIDI_channel, MIDI_out_port);
    MIDI_send_PC(patch_number & 0x7F, MIDI_channel, MIDI_out_port);
    DEBUGMSG("out(" + String(device_name) + ") PC" + String(patch_number)); //Debug
    do_after_patch_selection();
    midi_timer = millis() + GM2_MIDI_TIMER_LENGTH;
    send_patch_change = false;
  }
  //if (connected) MIDI_disable_device_check();
}

// ********************************* Section 2: TC-electronics G-Major 2 common MIDI in functions ********************************************

void MD_GM2_class::check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port) { // Check incoming sysex messages from  Called from MIDI:OnSysEx/OnSerialSysEx

  // Check if it is a message from a TC-electronics G-Major 2
  if ((port == MIDI_in_port) && (sxdata[2] == 0x20) && (sxdata[3] == 0x1F) && (sxdata[4] == MIDI_device_id) && (sxdata[5] == GM2_MODEL_NUMBER)) {

    // Check if it is the patch data of a specific patch
    if ((sxdata[6] == 0x20) && ((sxdata[7] + (sxdata[8] << 7)) == last_requested_sysex_patch_number)) {
      //if (sxlength > 100) {
      //if (sxdata[4] == 0x08) {
      //DEBUGMSG("!!!!Length sysex message: " + String(sxlength));
      // Read the patch name
      for (uint8_t count = 0; count < 16; count++) { // Read the first four characters of the name
        SP[last_requested_sysex_switch].Label[count] = static_cast<char>(sxdata[count + 9]); //Add ascii character to the SP.Label String
      }

      if (SP[last_requested_sysex_switch].PP_number == patch_number) {
        current_patch_name = SP[last_requested_sysex_switch].Label; // Load current patchname when it is read
        update_main_lcd = true; // And show it on the main LCD
      }
      DEBUGMSG(SP[last_requested_sysex_switch].Label);
      PAGE_request_next_switch();
      /*}
        else {
        PAGE_request_current_switch(); // Retry reading
        }*/
    }

    // Check if it is the current patch data
    if (sxdata[6] == 0x32) {
      uint16_t address = (sxdata[8] << 7) + sxdata[7];
      switch (address) {
        case 0:
          // Read the patch name
          for (uint8_t count = 0; count < 16; count++) { // Read the first four characters of the name
            current_patch_name[count] = static_cast<char>(sxdata[count + 9]); //Add ascii character to the SP.Label String
          }
          DEBUGMSG(current_patch_name);
          update_main_lcd = true;
          if (popup_patch_name) {
            LCD_show_popup_label(current_patch_name, ACTION_TIMER_LENGTH);
            popup_patch_name = false;
          }
          break;

        // Read effect states
        case 0x46:
        case 0x36:
        case 0x56:
        case 0x66:
        case 0x76:
        case 0x86:
        case 0x96:
        case 0x8B:
          read_FX_parameter(last_requested_sysex_switch, sxdata[9]);
          PAGE_request_next_switch();
          break;

        // Read FX types
        case 0x37:
          FX_type[0] = sxdata[ 9 ]; // 0: Filter
          break;
        case 0x47:
          FX_type[2] = sxdata[ 9 ]; // 2: Pitch
          break;
        case 0x57:
          FX_type[3] = sxdata[ 9 ]; // 3: Mod
          break;
        case 0x67:
          FX_type[4] = sxdata[ 9 ]; // 4: Delay
          break;
        case 0x77:
          FX_type[5] = sxdata[ 9 ]; // 5: Reverb
          break;
      }

      //update_page = REFRESH_FX_ONLY; // Need to update everything, otherwise displays go blank on detection of the G3.
    }
  }
}

void MD_GM2_class::check_PC_in(uint8_t program, uint8_t channel, uint8_t port) {  // Check incoming PC messages from  Called from MIDI:OnProgramChange

  // Check the source by checking the channel
  if ((port == MIDI_in_port) && (channel == MIDI_channel)) { // AXEFX sends a program change
    uint16_t new_patch = (CC00 * 128) + program;
    if (patch_number != new_patch) {
      set_patch_number(new_patch);
      //page_check();
      do_after_patch_selection();
      update_page = REFRESH_PAGE;
    }
  }
}
// Detection of TC-electronics G-Major 2

void MD_GM2_class::identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t in_port, uint8_t out_port) {
  // Check if it is a TC-electronics G-Major 2: F0 7E 7F 06 02 00 20 1F 66 01 00 03 F7
  if ((sxdata[6] == 0x20) && (sxdata[7] == 0x1F) && (sxdata[8] == GM2_MODEL_NUMBER) && (enabled == DEVICE_DETECT)) {
    no_response_counter = 0;
    if (connected == false) connect(sxdata[2], in_port, out_port); //Byte 2 contains the correct device ID
  }
}

void MD_GM2_class::do_after_connect() {
  is_on = true;
  //write_sysex(GM2_REQUEST_CURRENT_PATCH_NUMBER);
  MIDI_disable_device_check();
  //start_editor_mode();
  request_current_patch(); //This will update the FX buttons
  do_after_patch_selection();
}

// ********************************* Section 3: TC-electronics G-Major 2 common MIDI out functions ********************************************

void MD_GM2_class::write_sysex(uint8_t address, uint8_t data1, uint8_t data2) { // F0 00 20 1F 7F 66 32 37 00 val 00 00 00 F7
  uint8_t sysexmessage[14] = {0xF0, 0x00, 0x20, 0x1F, MIDI_device_id, GM2_MODEL_NUMBER, 0x32, (uint8_t) (address & 0x7F), (uint8_t) (address >> 7), data1, data2, 0x00, 0x00, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 14, MIDI_out_port);
}

void MD_GM2_class::request_sysex(uint8_t address) { // F0 00 20 1F 7F 66 47 37 00 F7
  uint8_t sysexmessage[10] = {0xF0, 0x00, 0x20, 0x1F, MIDI_device_id, GM2_MODEL_NUMBER, 0x47, (uint8_t) (address & 0x7F), (uint8_t) (address >> 7), 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 10, MIDI_out_port);
}

void MD_GM2_class::request_patch(uint16_t number) {
  uint8_t sysexmessage[10] = {0xF0, 0x00, 0x20, 0x1F, MIDI_device_id, GM2_MODEL_NUMBER, GM2_PATCH_REQUEST, (uint8_t) (number & 0x7F), (uint8_t) (number >> 7), 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 10, MIDI_out_port);
}

void MD_GM2_class::request_current_patch() { //F0 00 20 1F 7F 66 45 00 00 F7
  uint8_t sysexmessage[10] = {0xF0, 0x00, 0x20, 0x1F, MIDI_device_id, GM2_MODEL_NUMBER, GM2_PATCH_REQUEST, 0x00, 0x00, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 10, MIDI_out_port);
}

void MD_GM2_class::set_bpm() { //Will change the bpm to the specified value
  if (connected) {
    uint16_t bpm_in_msec = 60000 / Setting.Bpm;
    write_sysex(0x26, (uint8_t) (bpm_in_msec & 0x7F), (uint8_t) (bpm_in_msec >> 7));
  }
}

void MD_GM2_class::start_tuner() {
  if (connected) {
    MIDI_send_CC(GM2_TUNER_CC, 0x7F, MIDI_channel, MIDI_out_port);
  }
}

void MD_GM2_class::stop_tuner() {
  if (connected) {
    MIDI_send_CC(GM2_TUNER_CC, 0x00, MIDI_channel, MIDI_out_port);
  }
}

// ********************************* Section 4: G3 program change ********************************************

void MD_GM2_class::select_patch(uint16_t new_patch) {
  if (new_patch == patch_number) unmute();
  prev_patch_number = patch_number;
  patch_number = new_patch;
  send_patch_change = true;
  update_LEDS = true;
  update_main_lcd = true;
}

void MD_GM2_class::do_after_patch_selection() {
  is_on = connected;
  Current_patch_number = patch_number;
  update_LEDS = true;
  update_main_lcd = true;
  current_patch_name = "                ";
  update_main_lcd = true;
  request_sysex(0);
  MIDI_disable_device_check();
  MD_base_class::do_after_patch_selection();
}

void MD_GM2_class::page_check() { // Checks if the current patch is on the page
  bool onpage = false;
  for (uint8_t s = 0; s < NUMBER_OF_SWITCHES; s++) {
    if ((SP[s].Type == PATCH) && (SP[s].Sel_type == BANKSELECT) && (SP[s].PP_number == patch_number)) {
      onpage = true;
      current_patch_name = SP[s].Label; // Set patchname correctly
      GM2_Recall_FXs(s); // Copy the FX button states from memory
      update_page = REFRESH_PAGE;
    }
  }
  if (!onpage) {
    request_current_patch();
    MIDI_disable_device_check();
  }
}

bool MD_GM2_class::request_patch_name(uint8_t sw, uint16_t number) {
  //LCD_clear_SP_label(sw);
  //return true;
  DEBUGMSG("Requesting patch " + String(number));
  /*if (number < 201) {
    LCD_set_SP_label(sw, "Preset");
    return true;
    }*/
  if (number > patch_max) return true;
  last_requested_sysex_switch = sw;
  last_requested_sysex_patch_number = number + 1;
  request_patch(last_requested_sysex_patch_number);
  return false;
}

void MD_GM2_class::number_format(uint16_t number, String &Output) {
  uint16_t number_plus_one = number + 1;
  Output += String(number_plus_one / 100) + String((number_plus_one / 10) % 10) + String(number_plus_one % 10);
}

void MD_GM2_class::direct_select_format(uint16_t number, String &Output) {
  if (direct_select_state == 0) {
    Output += String(bank_select_number) + String(number) + "_";
  }
  else {
    Output += String(bank_select_number / 10) + String(bank_select_number % 10) + String(number);
  }
}

// ********************************* Section 5: TC-electronics G-Major 2 parameter control ********************************************

// The G-Major has the following effects:
// 0: Filter
// 1: Comp
// 2: Pitch
// 3: Mod
// 4: Delay
// 5: Reverb
// 6: Gate
// 7: Eq

// Define array for G3 effeect names and colours
struct GM2_FX_type_struct { // Combines all the data we need for controlling a parameter in a device
  uint8_t Address;
  uint8_t Address_type;
  char Name[7]; // The name for the label
  uint8_t Sublist;
  uint8_t Colour; // The colour for this effect.
};

const PROGMEM GM2_FX_type_struct GM2_FX_types[GM2_NUMBER_OF_FX] = { // Table with the name and colour for every effect of the TC-electronics G-Major 2
  { 0x46, 0x37, "Filter", 1, FX_FILTER_TYPE },
  { 0x36, 0x00, "Comp", 0, FX_DYNAMICS_TYPE },
  { 0x56, 0x47, "Pitch", 8, FX_PITCH_TYPE },
  { 0x66, 0x57, "Mod", 13, FX_MODULATE_TYPE },
  { 0x76, 0x67, "Delay", 26, FX_DELAY_TYPE },
  { 0x86, 0x77, "Reverb", 30, FX_REVERB_TYPE },
  { 0x96, 0x00, "Gate", 0, FX_DYNAMICS_TYPE },
  { 0x8B, 0x00, "Eq", 0, FX_FILTER_TYPE },
};

const PROGMEM char GM2_sublists[][9] = {
  // Sublist 1 - 7: Filter types
  "Wah Wah", "TouchWah", "Auto Wah", "Resonanc", "TouchRes", "Tremolo", "Panner",

  // Sublist 8 - 12: Pitch types
  "Detune", "Whammy", "Octaver", "Shifter", "Diatonic",

  // Sublist 13 - 25: Mod types
  "C.Chorus", "A.Chorus", "CFlanger", "AFlanger", "3ChorusN", "3ChorusA", "0Flangr-", "0Flangr+", "Vibrato", "V.Phaser", "S.Phaser", "V.Uni-V", "M.Uni-V",

  // Sublist 26 - 29: Delay types
  "PingPong", "Dynamic", "Dual", "Reverse",

  // Sublist 30 - 33: Reverb types
  "Spring", "Hall", "Room", "Plate",
};

void MD_GM2_class::parameter_press(uint8_t Sw, Cmd_struct * cmd, uint16_t number) {
  // Send sysex MIDI command to TC-electronics G-Major 2
  uint8_t value = SCO_return_parameter_value(Sw, cmd);
  write_sysex(GM2_FX_types[number].Address, value ^ 1, 0);
  FX_state[number] = value ^ 1;

  LCD_show_popup_label(SP[Sw].Label, ACTION_TIMER_LENGTH);
}

void MD_GM2_class::parameter_release(uint8_t Sw, Cmd_struct * cmd, uint16_t number) {
  if (SP[Sw].Latch == MOMENTARY) {
    //set_FX_state(SP[Sw].PP_number, cmd->Value2);
  }
}

void MD_GM2_class::read_parameter_title(uint16_t number, String &Output) {
  if (number < GM2_NUMBER_OF_FX) {
    Output += GM2_FX_types[number].Name;
  }
}

bool MD_GM2_class::request_parameter(uint8_t sw, uint16_t number) {
  //Effect type and state are read from the GM2 and stored in the GM2_FX array
  //Effect can have three states: 0 = no effect, 1 = on, 2 = off
  if (number < GM2_NUMBER_OF_FX) {
    if (GM2_FX_types[number].Address_type > 0x00) request_sysex(GM2_FX_types[number].Address_type);
    uint8_t my_address = GM2_FX_types[number].Address;
    last_requested_sysex_address = my_address;
    last_requested_sysex_type = REQUEST_PARAMETER_TYPE;
    last_requested_sysex_switch = sw;
    request_sysex(my_address);
    /**/
    SP[sw].Colour = GM2_FX_types[number].Colour; //Find the LED colour in the GM2_FX_types array
  }
  return false; // Move to next switch is true
}

void MD_GM2_class::read_FX_parameter(uint8_t sw, uint8_t byte) {
  uint8_t number = SP[sw].PP_number;
  DEBUGMSG("Reading sw:" + String(sw) + ", byte:" + String(byte));
  if (byte == 0x00) SP[sw].State = 1; //Effect on
  else SP[sw].State = 2; // Effect off
  String lbl = "";
  uint8_t sublist = GM2_FX_types[number].Sublist;
  if (sublist > 0) {
    uint8_t index = sublist + FX_type[number] - 1;
    lbl = '(';
    lbl += GM2_sublists[index];
    lbl += ')';
  }
  LCD_set_SP_label(sw, lbl);
}

// Menu options for FX states
void MD_GM2_class::read_parameter_name(uint16_t number, String & Output) { // Called from menu
  if (number < number_of_parameters())  Output = GM2_FX_types[number].Name;
  else Output = "?";
}

FLASHMEM uint16_t MD_GM2_class::number_of_parameters() {
  return GM2_NUMBER_OF_FX;
}

FLASHMEM uint8_t MD_GM2_class::number_of_values(uint16_t parameter) {
  if (parameter < number_of_parameters()) return 2; // So far all parameters have two states: on and off
  else return 0;
}

void MD_GM2_class::read_parameter_value_name(uint16_t number, uint16_t value, String & Output) {
  if (number < number_of_parameters())  {
    if (value == 1) Output += "ON";
    else Output += "OFF";
  }
  else Output += "?";
}

// ********************************* Section 6: TC-electronics G-Major 2 expression pedal control ********************************************

void MD_GM2_class::move_expression_pedal(uint8_t sw, uint8_t value, uint8_t exp_pedal) {
  uint8_t number;
  String lbl = "Modifier";
  if (exp_pedal == 0) exp_pedal = current_exp_pedal;
  if (exp_pedal == 1) {
    number = GM2_MODIFIER1_CC;
    lbl += '1';
  }
  else if (exp_pedal == 2) {
    number = GM2_MODIFIER2_CC;
    lbl += '2';
  }
  else return;
  LCD_show_bar(0, value, 0); // Show it on the main display
  MIDI_send_CC(number, value, MIDI_channel, MIDI_out_port);
  lbl += ':';
  LCD_add_3digit_number(value, lbl);
  LCD_show_popup_label(lbl, ACTION_TIMER_LENGTH);
  update_page = REFRESH_FX_ONLY; // To update the other switch states, we re-load the current page
}

bool MD_GM2_class::request_exp_pedal(uint8_t sw, uint8_t exp_pedal) {
  if (exp_pedal == 0) exp_pedal = current_exp_pedal;
  if (exp_pedal == 1) LCD_set_SP_label(sw, "Modifier1");
  else if (exp_pedal == 2) LCD_set_SP_label(sw, "Modifier2");
  else LCD_clear_SP_label(sw);
  return true;
}
