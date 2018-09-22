// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: HLX Initialization
// Section 2: HLX common MIDI in functions
// Section 3: HLX common MIDI out functions
// Section 4: HLX program change
// Section 5: HLX parameter control
// Section 6: HLX assign control

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

void HLX_class::init() { // Default values for variables

  // Line6 HLX variables:
  enabled = DEVICE_DETECT; // Helix can be detected via USB, but not via regular MIDI...
  strcpy(device_name, "HELIX");
  strcpy(full_device_name, "Line6 HELIX");
  current_patch_name.reserve(17);
  current_patch_name = "                ";
  patch_min = HLX_PATCH_MIN;
  patch_max = HLX_PATCH_MAX;
  bank_size = 8;
  max_times_no_response = MAX_TIMES_NO_RESPONSE; // The number of times the HLX does not have to respond before disconnection
  sysex_delay_length = 0; // time between sysex messages (in msec).
  my_LED_colour = 2; // Default value: red
  MIDI_channel = HLX_MIDI_CHANNEL; // Default value
  MIDI_port = HLX_MIDI_PORT; // Default value
  bank_number = 0; // Default value
  is_always_on = true; // Default value
  my_device_page1 = PAGE_HLX_PATCH_BANK; // Default value
  my_device_page2 = PAGE_HLX_PARAMETER; // Default value
  my_device_page3 = PAGE_SNAPSCENE_LOOPER; // Default value
  my_device_page4 = 0; // Default value
  tuner_active = false;
  //for (uint8_t i = 0; i < HLX_PAR_ON_SIZE; i++) par_on[i] = 0;
  //overdub = false;
}

void HLX_class::update() {
  if (!connected) return;
  looper_timer_check();
}

// ********************************* Section 2: HLX common MIDI in functions ********************************************
void HLX_class::check_PC_in(uint8_t program, uint8_t channel, uint8_t port) {  // Check incoming PC messages from  Called from MIDI:OnProgramChange

  // Check the source by checking the channel
  if ((port == MIDI_port) && (channel == MIDI_channel)) { // HLX sends a program change
    if (patch_number != program) {
      patch_number = program;
      //request_sysex(HLX_REQUEST_CURRENT_PATCH_NAME); // So the main display always show the correct patch
      //page_check();
      do_after_patch_selection();
      update_page = REFRESH_PAGE;
    }
  }
}

// Detection of Helix only possible when connected via USB midi on the Helix
// This is a bug in the firmware (2.54) of the Helix

void HLX_class::identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port) {
  // Check if it is a Helix
  if ((sxdata[5] == 0x00) && (sxdata[6] == 0x01) && (sxdata[7] == 0x0C) && (sxdata[8] == 0x21)) {
    no_response_counter = 0;
    if (connected == false) connect(sxdata[2], port); //Byte 2 contains the correct device ID
  }
}

void HLX_class::do_after_connect() {
  current_exp_pedal = 2;
}

// ********************************* Section 3: HLX common MIDI out functions ********************************************
void HLX_class::bpm_tap() {
  if (connected) {
    MIDI_send_CC(64, 127, MIDI_channel, MIDI_port); // Tap tempo on HLX
  }
}

void HLX_class::start_tuner() {
  if (connected) {
    //MIDI_send_CC(68, 0, MIDI_channel, MIDI_port);
    MIDI_send_CC(68, 127, MIDI_channel, MIDI_port);
    tuner_active = true;
  }
}

void HLX_class::stop_tuner() {
  if (connected) {
    //MIDI_send_CC(68, 127, MIDI_channel, MIDI_port);
    MIDI_send_CC(68, 0, MIDI_channel, MIDI_port);
    tuner_active = false;
  }
}

// ********************************* Section 4: HLX program change ********************************************

void HLX_class::do_after_patch_selection() {
  is_on = connected;
  if (Setting.Send_global_tempo_after_patch_change == true) {
    SCO_retap_tempo();
  }
  Current_patch_number = patch_number;
  current_exp_pedal = 2; // Helix has pedal 2 default on
  current_snapscene = 0;
  looper_reset();
  update_LEDS = true;
  update_main_lcd = true;
  update_page = REFRESH_PAGE;
  if ((flash_bank_of_four != 255) && (Current_page != read_current_device_page())) SCO_select_page(read_current_device_page()); // When in direct select, go back to the current_device_page
  flash_bank_of_four = 255;
}

bool HLX_class::flash_LEDs_for_patch_bank_switch(uint8_t sw) { // Will flash the LEDs in banks of three when coming from direct select mode.
  if (!bank_selection_active()) return false;

  if (flash_bank_of_four == 255) return true; // We are not coming from direct select, so all LEDs should flash

  bool in_right_bank_of_eight = (flash_bank_of_four / 2 == SP[sw].PP_number / 8); // Going bank up and down coming from direct select will make all LEDs flash in other banks
  if (!in_right_bank_of_eight) return true;

  bool in_right_bank_of_four = (flash_bank_of_four == SP[sw].PP_number / 4); // Only flash the four LEDs of the corresponding bank
  if (in_right_bank_of_four) return true;
  return false;
}


void HLX_class::number_format(uint16_t number, String &Output) {
  char PatchChar;
  uint8_t bank_no = number >> 2;
  PatchChar = 65 + number % 4;
  Output += String((bank_no + 1) / 10) + String((bank_no + 1) % 10) + PatchChar;
}

void HLX_class::direct_select_format(uint16_t number, String &Output) {
  if (direct_select_state == 0) Output += String(number) + "__";
  else Output += String(bank_select_number) + String(number) + "_";
}

bool HLX_class::valid_direct_select_switch(uint8_t number) {
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

void HLX_class::direct_select_start() {
  Previous_bank_size = bank_size; // Remember the bank size
  device_in_bank_selection = my_device_number + 1;
  bank_size = 400;
  bank_select_number = 0; // Reset bank number
  direct_select_state = 0;
}

void HLX_class::direct_select_press(uint8_t number) {
  if (!valid_direct_select_switch(number)) return;
  if (direct_select_state == 0) {
    // First digit pressed
    bank_size = 40;
    bank_select_number = number;
    direct_select_state = 1;
  }
  else  {
    // Second digit pressed
    device_in_bank_selection = my_device_number + 1; // Go into bank mode
    uint16_t base_patch = (bank_select_number * 40) + (number - 1) * 4;
    flash_bank_of_four = base_patch / 4;
    bank_select_number = (base_patch / Previous_bank_size);
    bank_size = Previous_bank_size;
    DEBUGMSG("PREVIOUS BANK_SIZE: " + String(Previous_bank_size));
    SCO_select_page(PAGE_HLX_PATCH_BANK);
  }
}

// ********************************* Section 5: HLX parameter control ********************************************
// Define array for HLX effeect names and colours
struct HLX_CC_type_struct { // Combines all the data we need for controlling a parameter in a device
  char Name[17]; // The name for the label
  uint8_t CC; // The colour for this effect.
};

const PROGMEM HLX_CC_type_struct HLX_CC_types[] = { // Table with the name and colour for every effect of the Zoom G3
  {"FS 01", 49}, // 0
  {"FS 02", 50},
  {"FS 03", 51},
  {"FS 04", 52},
  {"FS 05", 53},
  {"FS 07", 54},
  {"FS 08", 55},
  {"FS 09", 56},
  {"FS 10", 57},
  {"FS 11", 58},
  {"EXP TOE SW", 59}, // 10
  {"EXP 1", 1}, // 11
  {"EXP 2", 2},
  {"EXP 3", 3},
  {"SETLIST", 32}, // 14
  {"SNAPSHOT", 69},
};

const uint16_t HLX_NUMBER_OF_PARAMETERS = sizeof(HLX_CC_types) / sizeof(HLX_CC_types[0]);

#define HLX_SW_EXP_TOE 10
#define HLX_SW_EXP1 11
#define HLX_SW_EXP2 12
#define HLX_SW_EXP3 13
#define HLX_SETLIST 14
#define HLX_SNAPSHOT 15

void HLX_class::parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {
  // Send cc command to Line6 HLX
  uint8_t value = SCO_return_parameter_value(Sw, cmd);

  // Send the CC message
  //if (SP[Sw].Latch == RANGE) MIDI_send_CC(HLX_CC_types[number].CC, SP[Sw].Target_byte1, MIDI_channel, MIDI_port);
  MIDI_send_CC(HLX_CC_types[number].CC, value, MIDI_channel, MIDI_port);
  LCD_show_status_message(SP[Sw].Label);
  update_page = REFRESH_FX_ONLY; // To update the other switch states, we re-load the current page
}

void HLX_class::parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {
}

bool HLX_class::request_parameter(uint8_t sw, uint16_t number) {
  String msg;
  SP[sw].State = 2; // Effect off
  SP[sw].Colour = my_LED_colour;
  msg = HLX_CC_types[number].Name;

  if ((number == HLX_SETLIST) || (number == HLX_SNAPSHOT)) {
    //if (number == HLX_SETLIST) SP[sw].Target_byte1 = current_setlist;
    if (SP[sw].Latch == STEP) msg += " " + String(SP[sw].Target_byte1 + 1);
    else msg += " " + String(SP[sw].Value1 + 1);
  }

  LCD_set_SP_label(sw, msg);
  return true; // Move to next switch is true.
}

void HLX_class::check_update_label(uint8_t Sw, uint8_t value) { // Updates the label for extended sublists
  String msg;
  uint16_t index = SP[Sw].PP_number;
  msg = HLX_CC_types[index].Name;
  if ((index >= HLX_SW_EXP1) && (index <= HLX_SW_EXP3)) {
    msg += ":";
    LCD_add_3digit_number(value, msg);
  }
  LCD_set_SP_label(Sw, msg);

  //Update the current switch label
  update_lcd = Sw;
}

// Menu options for FX states
void HLX_class::read_parameter_name(uint16_t number, String &Output) { // Called from menu
  if (number < number_of_parameters())  Output = HLX_CC_types[number].Name;
  else Output = "?";
}

uint16_t HLX_class::number_of_parameters() {
  return HLX_NUMBER_OF_PARAMETERS;
}

uint8_t HLX_class::number_of_values(uint16_t parameter) {
  //if ((parameter == HLX_SW_EXP1) || (parameter == HLX_SW_EXP2) || (parameter == HLX_SW_EXP3)) return 128; // Return 128 for the expression pedals
  if (parameter == HLX_SNAPSHOT) return 8;
  if (parameter == HLX_SETLIST) return 8;
  if (parameter < number_of_parameters()) return 128; // So far all parameters have one states: momentary
  else return 0;
}

void HLX_class::read_parameter_value_name(uint16_t number, uint16_t value, String &Output) {
  if (number < number_of_parameters()) Output += String(value);
  else Output += " ? "; // Unknown parameter
}

void HLX_class::move_expression_pedal(uint8_t sw, uint8_t value, uint8_t exp_pedal) {
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
  LCD_show_status_message(SP[sw].Label);
  update_page = REFRESH_FX_ONLY; // To update the other switch states, we re-load the current page
}

void HLX_class::toggle_expression_pedal(uint8_t sw) {
  uint8_t value;
  if (current_exp_pedal == 0) return;
  current_exp_pedal++;
  if (current_exp_pedal > 2) current_exp_pedal = 1;
  // Send toe switch message
  if (current_exp_pedal <= 2) value = 127;
  MIDI_send_CC(HLX_CC_types[HLX_SW_EXP_TOE].CC, value, MIDI_channel, MIDI_port);
  update_page = REFRESH_FX_ONLY;
}

bool HLX_class::request_exp_pedal(uint8_t sw, uint8_t exp_pedal) {
  if (exp_pedal == 0) exp_pedal = current_exp_pedal;
  uint8_t number = 0;
  if (exp_pedal == 1) number = HLX_SW_EXP1;
  if (exp_pedal == 2) number = HLX_SW_EXP2;
  if (exp_pedal == 3) number = HLX_SW_EXP3;
  SP[sw].PP_number = number;
  LCD_clear_SP_label(sw);
  return true;
}

void HLX_class::set_snapscene_name(uint8_t number, String &Output) {
  Output += "SNAPSHOT " + String(number);
}

void HLX_class::set_snapscene(uint8_t number) {
  current_snapscene = number;
  MIDI_send_CC(HLX_SNAPSHOT_SELECT_CC, number - 1, MIDI_channel, MIDI_port);
}

bool HLX_class::looper_active() {
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

void HLX_class::send_looper_cmd(uint8_t cmd) {
  if (cmd < HLX_LOOPER_NUMBER_OF_CCS) {
    if (HLX_looper_cc[cmd].cc > 0) MIDI_send_CC(HLX_looper_cc[cmd].cc, HLX_looper_cc[cmd].value, MIDI_channel, MIDI_port);
  }
}

