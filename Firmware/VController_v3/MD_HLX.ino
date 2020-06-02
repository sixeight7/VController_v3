// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: HLX Initialization
// Section 2: HLX common MIDI in functions
// Section 3: HLX common MIDI out functions
// Section 4: HLX program change
// Section 5: HLX parameter control
// Section 6: HLX assign control
// Section 7: HELIX MIDI forward messaging

// ********************************* Section 1: HLX Initialization ********************************************

// Line6 HELIX settings:
#define HLX_MIDI_CHANNEL 1
#define HLX_MIDI_PORT 1 // Default port is MIDI1
#define HLX_PATCH_MIN 0
#define HLX_PATCH_MAX 127

// The Helix does not respond to any sysex messages. Therefore we must implement one-way midi connectivity.

// The Helix does support the following CC-messages:
// CC#01 0-127 Emulates EXP 1 Pedal
// CC#02 0-127 Emulates EXP 2 Pedal
// CC#03 0-127 Emulates EXP 3 Pedal
// CC#49 0-127 Emulates Stomp footswitch mode's FS1
// CC#50 0-127 Emulates Stomp footswitch mode's FS2
// CC#51 0-127 Emulates Stomp footswitch mode's FS3
// CC#52 0-127 Emulates Stomp footswitch mode's FS4
// CC#53 0-127 Emulates Stomp footswitch mode's FS5
// CC#54 0-127 Emulates Stomp footswitch mode's FS7
// CC#55 0-127 Emulates Stomp footswitch mode's FS8
// CC#56 0-127 Emulates Stomp footswitch mode's FS9
// CC#57 0-127 Emulates Stomp footswitch mode's FS10
// CC#58 0-127 Emulates Stomp footswitch mode's FS11
// CC#59 0-127 Emulates EXP Toe switch

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

#define HLX_SNAPSHOT_SELECT_CC 69

void MD_HLX_class::init() { // Default values for variables
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
  MIDI_port = HLX_MIDI_PORT; // Default value
  my_device_page1 = HLX_DEFAULT_PAGE1; // Default value
  my_device_page2 = HLX_DEFAULT_PAGE2; // Default value
  my_device_page3 = HLX_DEFAULT_PAGE3; // Default value
  my_device_page4 = HLX_DEFAULT_PAGE4; // Default value
  tuner_active = false;
  max_looper_length = 30000000; // Normal stereo looper time is 30 seconds - time given in microseconds
}

void MD_HLX_class::update() {
  if (!connected) return;
  looper_timer_check();
}

// ********************************* Section 2: HLX common MIDI in functions ********************************************
void MD_HLX_class::check_PC_in(uint8_t program, uint8_t channel, uint8_t port) {  // Check incoming PC messages from  Called from MIDI:OnProgramChange
  if (!connected) return;
  // Check the source by checking the channel
  if ((port == MIDI_port) && (channel == MIDI_channel)) { // HLX sends a program change
    if (patch_number != program) {
      prev_patch_number = patch_number;
      patch_number = program;
      //request_sysex(HLX_REQUEST_CURRENT_PATCH_NAME); // So the main display always show the correct patch
      //page_check();
      do_after_patch_selection();
      update_page = REFRESH_PAGE;
    }
  }
  PC_forwarding(program, channel, port);
}

void MD_HLX_class::check_CC_in(uint8_t control, uint8_t value, uint8_t channel, uint8_t port) {
  if (!connected) return;
  if ((port == MIDI_port) && (channel == MIDI_channel)) {
    switch (control) {
      case 2: // EXP pedal 2 - must be set in Command Center on each patch
        if ((!tuner_active) && (value == 0)) {
          start_tuner();
        }
        else if ((tuner_active) && (value > 0)) {
          stop_tuner();
        }
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

void MD_HLX_class::identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port) {
  // Check if it is a Helix
  if ((sxdata[5] == 0x00) && (sxdata[6] == 0x01) && (sxdata[7] == 0x0C) && (sxdata[8] == 0x21)) {
    no_response_counter = 0;
    if (connected == false) connect(sxdata[2], port); //Byte 2 contains the correct device ID
  }
}

void MD_HLX_class::do_after_connect() {
  current_exp_pedal = 2;
}

// ********************************* Section 3: HLX common MIDI out functions ********************************************
void MD_HLX_class::bpm_tap() {
  if (connected) {
    MIDI_send_CC(64, 127, MIDI_channel, MIDI_port); // Tap tempo on HLX
  }
}

void MD_HLX_class::start_tuner() {
  if (connected) {
    //MIDI_send_CC(68, 0, MIDI_channel, MIDI_port);
    MIDI_send_CC(68, 127, MIDI_channel, MIDI_port);
    tuner_active = true;
  }
}

void MD_HLX_class::stop_tuner() {
  if (connected) {
    //MIDI_send_CC(68, 127, MIDI_channel, MIDI_port);
    MIDI_send_CC(68, 0, MIDI_channel, MIDI_port);
    tuner_active = false;
  }
}

// ********************************* Section 4: HLX program change ********************************************

void MD_HLX_class::do_after_patch_selection() {
  is_on = connected;
  if (Setting.Send_global_tempo_after_patch_change == true) {
    SCO_retap_tempo();
  }
  Current_patch_number = patch_number;
  current_exp_pedal = 2; // Helix has pedal 2 default on
  current_snapscene = 0;
  //looper_reset();
  update_LEDS = true;
  update_main_lcd = true;
  if ((flash_bank_of_four != 255) && (Current_page != read_current_device_page())) SCO_select_page(read_current_device_page()); // When in direct select, go back to the current_device_page
  flash_bank_of_four = 255;
  MD_base_class::do_after_patch_selection();
}

bool MD_HLX_class::flash_LEDs_for_patch_bank_switch(uint8_t sw) { // Will flash the LEDs in banks of three when coming from direct select mode.
  if (!bank_selection_active()) return false;

  if (flash_bank_of_four == 255) return true; // We are not coming from direct select, so all LEDs should flash

  bool in_right_bank_of_eight = (flash_bank_of_four / 2 == SP[sw].PP_number / 8); // Going bank up and down coming from direct select will make all LEDs flash in other banks
  if (!in_right_bank_of_eight) return true;

  bool in_right_bank_of_four = (flash_bank_of_four == SP[sw].PP_number / 4); // Only flash the four LEDs of the corresponding bank
  if (in_right_bank_of_four) return true;
  return false;
}


void MD_HLX_class::number_format(uint16_t number, String &Output) {
  char PatchChar;
  uint8_t bank_no = number >> 2;
  PatchChar = 65 + number % 4;
  Output += String((bank_no + 1) / 10) + String((bank_no + 1) % 10) + PatchChar;
}

void MD_HLX_class::direct_select_format(uint16_t number, String &Output) {
  if (direct_select_state == 0) Output += String(number) + "__";
  else Output += String(bank_select_number) + String(number) + "_";
}

bool MD_HLX_class::valid_direct_select_switch(uint8_t number) {
  bool result = false;
  if (direct_select_state == 0) { // Show all switches on first digit
    result = ((number * 40) <= (patch_max - patch_min));
  }
  else {
    if ((bank_select_number == 0) && (number == 0)) return false;
    if ((bank_select_number * 40) + (number * 4) <= (patch_max - patch_min)) result = true;
  }
  return result;
}

void MD_HLX_class::direct_select_start() {
  Previous_bank_size = bank_size; // Remember the bank size
  device_in_bank_selection = my_device_number + 1;
  bank_size = 400;
  bank_select_number = 0; // Reset bank number
  direct_select_state = 0;
}

void MD_HLX_class::direct_select_press(uint8_t number) {
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
    SCO_select_page(HLX_DEFAULT_PAGE1); // Which should give PAGE_HLX_PATCH_BANK
    device_in_bank_selection = my_device_number + 1; // Go into bank mode
  }
}

// ********************************* Section 5: HLX parameter control ********************************************
// Define array for HLX effeect names and colours
struct HLX_CC_type_struct { // Combines all the data we need for controlling a parameter in a device
  char Name[17]; // The name for the label
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
  {"SETLIST", 32, 6}, // 14
  {"SNAPSHOT", 69, 8},
};

const uint16_t HLX_NUMBER_OF_PARAMETERS = sizeof(HLX_CC_types) / sizeof(HLX_CC_types[0]);

#define HLX_SW_EXP_TOE 10
#define HLX_SW_EXP1 11
#define HLX_SW_EXP2 12
#define HLX_SW_EXP3 13
#define HLX_SETLIST 14
#define HLX_SNAPSHOT 15

void MD_HLX_class::parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {
  // Send cc command to Line6 HLX
  uint8_t value = SCO_return_parameter_value(Sw, cmd);
  if (HLX_CC_types[number].NumVals == 2) {
    MIDI_send_CC(HLX_CC_types[number].CC, 127, MIDI_channel, MIDI_port);
  }
  else {
    MIDI_send_CC(HLX_CC_types[number].CC, value, MIDI_channel, MIDI_port);
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

void MD_HLX_class::parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {
}

void MD_HLX_class::read_parameter_title(uint16_t number, String &Output) {
  Output += HLX_CC_types[number].Name;
}

bool MD_HLX_class::request_parameter(uint8_t sw, uint16_t number) {
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

void MD_HLX_class::check_update_label(uint8_t Sw, uint8_t value) { // Updates the label for extended sublists
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
void MD_HLX_class::read_parameter_name(uint16_t number, String &Output) { // Called from menu
  if (number < number_of_parameters())  Output = HLX_CC_types[number].Name;
  else Output = "?";
}

uint16_t MD_HLX_class::number_of_parameters() {
  return HLX_NUMBER_OF_PARAMETERS;
}

uint8_t MD_HLX_class::number_of_values(uint16_t parameter) {
  if (parameter < HLX_NUMBER_OF_PARAMETERS) {
    return HLX_CC_types[parameter].NumVals;
  }
  else return 0;
}

void MD_HLX_class::read_parameter_value_name(uint16_t number, uint16_t value, String &Output) {
  if (number < number_of_parameters()) Output += String(value);
  else Output += " ? "; // Unknown parameter
}

void MD_HLX_class::move_expression_pedal(uint8_t sw, uint8_t value, uint8_t exp_pedal) {
  uint8_t number;
  if (exp_pedal == 0) exp_pedal = current_exp_pedal;
  switch (exp_pedal) {
    case 1: number = HLX_SW_EXP1; break;
    case 2: number = HLX_SW_EXP2; break;
    case 3: number = HLX_SW_EXP3; break;
    default: return;
  }
  LCD_show_bar(0, value); // Show it on the main display
  MIDI_send_CC(HLX_CC_types[number].CC, value, MIDI_channel, MIDI_port);
  check_update_label(sw, value);
  String msg = HLX_CC_types[number].Name;
  msg += ':';
  LCD_add_3digit_number(value, msg);
  LCD_show_popup_label(msg, ACTION_TIMER_LENGTH);
  update_page = REFRESH_FX_ONLY; // To update the other switch states, we re-load the current page
}

void MD_HLX_class::toggle_expression_pedal(uint8_t sw) {
  uint8_t value;
  if (current_exp_pedal == 0) return;
  current_exp_pedal++;
  if (current_exp_pedal > 2) current_exp_pedal = 1;
  // Send toe switch message
  if (current_exp_pedal <= 2) value = 127;
  MIDI_send_CC(HLX_CC_types[HLX_SW_EXP_TOE].CC, value, MIDI_channel, MIDI_port);
  update_page = REFRESH_FX_ONLY;
}

bool MD_HLX_class::request_exp_pedal(uint8_t sw, uint8_t exp_pedal) {
  if (exp_pedal == 0) exp_pedal = current_exp_pedal;
  uint8_t number = 0;
  if (exp_pedal == 1) number = HLX_SW_EXP1;
  if (exp_pedal == 2) number = HLX_SW_EXP2;
  if (exp_pedal == 3) number = HLX_SW_EXP3;
  SP[sw].PP_number = number;
  LCD_clear_SP_label(sw);
  return true;
}

void MD_HLX_class::set_snapscene_title(uint8_t number, String &Output) {
  Output += "SNAPSHOT " + String(number);
}

/*void MD_HLX_class::set_snapscene_name(uint8_t number, String &Output) {
  Output += "SNAPSHOT " + String(number);
  }*/

void MD_HLX_class::set_snapscene(uint8_t number) {
  current_snapscene = number;
  MIDI_send_CC(HLX_SNAPSHOT_SELECT_CC, number - 1, MIDI_channel, MIDI_port);
}

void MD_HLX_class::snapscene_number_format(String &Output) { // Add snapshot number to potchnumber
  if (current_snapscene == 0) return;
  Output += '-';
  Output += String(current_snapscene);
}

bool MD_HLX_class::looper_active() {
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

void MD_HLX_class::send_looper_cmd(uint8_t cmd) {
  if (cmd < HLX_LOOPER_NUMBER_OF_CCS) {
    if (HLX_looper_cc[cmd].cc > 0) MIDI_send_CC(HLX_looper_cc[cmd].cc, HLX_looper_cc[cmd].value, MIDI_channel, MIDI_port);
  }
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

void MD_HLX_class::PC_forwarding(uint8_t Program, uint8_t Channel, uint8_t Port) {
  if (Port != MIDI_port) return;
  if (Channel == MIDI_channel) { // Check if it is PC message from the Helix
    // Forward PC message from the Helix
    MIDI_forward_PC_to_all_ports_but_mine(Program, Channel, MIDI_port);

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
    DEBUGMSG("Stored setlist: " + String(HLX_message_setlist) + " == HLX setlist: " + String(current_setlist));
    if (current_setlist == HLX_message_setlist) {
      MIDI_Helix_sent_msg_no = 0;
      for (uint8_t m = 0; m < MIDI_HLX_MESSAGES; m++) {
        if (HLX_messages[m][0] != 0) {
          MIDI_Helix_sent_msg_no++;
          switch (HLX_messages[m][0] & 0xF0) {
            case 0xC0: // Program change message
              MIDI_forward_PC_to_all_ports_but_mine(HLX_messages[m][1], HLX_messages[m][0] & 0x0F,  MIDI_port);
              break;
            case 0xB0: // Control change message
              MIDI_forward_CC_to_all_ports_but_mine(HLX_messages[m][1], HLX_messages[m][2], HLX_messages[m][0] & 0x0F, MIDI_port);
              break;
          }
        }
      }
      do_not_forward_after_Helix_PC_message = true;

      DEBUGMSG("Done sending " + String(MIDI_Helix_sent_msg_no) + " messages from memory at " + String(millis()));
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
        MIDI_forward_PC_to_all_ports_but_mine(Program, Channel, MIDI_port);
        //MIDI_send_PC(Program, Channel, My_VG99.MIDI_port); // Forward the message
        HLX_messages[MIDI_Helix_received_msg_no][1] = Program;
        HLX_messages[MIDI_Helix_received_msg_no][0] = 0xC0 | Channel;
        EEPROM_store_HELIX_message(MIDI_Helix_current_program, current_setlist); // Store it in EEPROM
      }
      MIDI_Helix_received_msg_no++;
    }
    else {
      // Forward message
      MIDI_forward_PC_to_all_ports_but_mine(Program, Channel, MIDI_port);
    }
  }
}

void MD_HLX_class::CC_forwarding(uint8_t Controller, uint8_t Value, uint8_t Channel, uint8_t Port) {
  if (Port != MIDI_port) return;
  if ((millis() < MIDI_Helix_IC_timer) && (Channel != MIDI_channel) && (MIDI_Helix_received_msg_no < MIDI_HLX_MESSAGES)) { // Check timer running
    // Check message with stored message in EEPROM
    DEBUGMSG("Checking CC message " + String(MIDI_Helix_received_msg_no) + " at " + String(millis()));
    if ((Channel != (HLX_messages[MIDI_Helix_received_msg_no][0] & 0x0F)) || (Controller != HLX_messages[MIDI_Helix_received_msg_no][1]) || (Value != HLX_messages[MIDI_Helix_received_msg_no][2]) || (current_setlist != HLX_message_setlist)) {
      DEBUGMSG("CC message does not match message #" + String(MIDI_Helix_received_msg_no));
      DEBUGMSG("Received: CC#" + String(Controller) + ", value: " + String(Value) + " on channel " + String(Channel));
      DEBUGMSG("Stored:   CC#" + String(HLX_messages[MIDI_Helix_received_msg_no][1]) + ", value: " + String(HLX_messages[MIDI_Helix_received_msg_no][2]) + " on channel " + String(HLX_messages[MIDI_Helix_received_msg_no][0] & 0x0F));
      MIDI_forward_CC_to_all_ports_but_mine(Controller, Value, Channel, MIDI_port);
      HLX_messages[MIDI_Helix_received_msg_no][1] = Controller;
      HLX_messages[MIDI_Helix_received_msg_no][2] = Value;
      HLX_messages[MIDI_Helix_received_msg_no][0] = 0xB0 | Channel;
      EEPROM_store_HELIX_message(MIDI_Helix_current_program, current_setlist); // Store it in EEPROM
    }
    MIDI_Helix_received_msg_no++;
  }
  else {
    // Forward message
    MIDI_forward_CC_to_all_ports_but_mine(Controller, Value, Channel, MIDI_port);
  }
}
