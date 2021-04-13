// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: KPA Initialization
// Section 2: KPA common MIDI in functions
// Section 3: KPA common MIDI out functions
// Section 4: KPA program change
// Section 5: KPA parameter control
// Section 6: KPA assign control

// ********************************* Section 1: KPA Initialization ********************************************

// Kemper settings:
#define KPA_MIDI_CHANNEL 1
#define KPA_MIDI_PORT 1 // Default port is MIDI1
#define KPA_PERFORMANCE_PATCH_MIN 0
#define KPA_PERFORMANCE_PATCH_MAX 624 // 125 performances of 5 rigs
#define KPA_BROWSE_PATCH_MIN 0
#define KPA_BROWSE_PATCH_MAX 1004 // 5 browse numbers + 1000 rigs
#define KPA_PERFORMANCE_MAX 124 // numbered 0 - 124

// The Kemper can store up to 1000 rigs (presets)

// Request active rig name: 0xF0, 0x00, 0x20, 0x33, 0x02, 0x7F, 0x43, 0x00, 0x00, 0x01, 0xf7
// To get the slot names you have to send
// 0xF0, 0x00, 0x20, 0x33, 0x02, 0x00, 0x47, 0x00, 0x00, 0x00, 0x01, 0x00, <slot number>, 0xF7
// Slot Numbers 1-5. Number 0 returns the name of the performance

// Request delay block state: F0 00 20 33 02 7f 41 00 3c 03 F7
// Kemper will respond with:  F0 00 20 33 00 00 01 00 3c 03 00 00/01 F7

// Request current mode: F0 00 20 33 02 7F 41 00 7F 7D F7
// 00- browse mode 01-performance mode
#define KPA_BROWSE_MODE 00
#define KPA_PERFORMANCE_MODE 01

//KPA functions code
#define KPA_FUNCTION_SINGLE_PARAMETER_CHANGE                    0x01
#define KPA_FUNCTION_MULTI_PARAMETER_CHANGE                     0x02
#define KPA_FUNCTION_STRING_PARAMETER_CHANGE                    0x03
#define KPA_FUNCTION_BLOB                                       0x04
#define KPA_FUNCTION_EXTENDED_PARAMETER_CHANGE                  0x06
#define KPA_FUNCTION_EXTENDED_STRING_PARAMETER_CHANGE           0x07
#define KPA_FUNCTION_REQUEST_SINGLE_PARAMETER_VALUE             0x41
#define KPA_FUNCTION_REQUEST_MULTI_PARAMETER_VALUES             0x42
#define KPA_FUNCTION_REQUEST_STRING_PARAMETER                   0x43
#define KPA_FUNCTION_REQUEST_EXTENDED_STRING_PARAMETER          0x47
#define KPA_FUNCTION_REQUEST_PARAMETER_VALUE_AS_RENDERED_STRING 0x7C
//it used for "beacon" and "active sensing signal" only
#define KPA_FUNCTION_SYS_COMMUNICATION                          0x7E

// Setting the beacon on the KPA will make it send parameter changes, a bit like editor mode on other devices, but more suited towards external controllers.
// Beacon flags
#define KPA_BEACON_FLAG_INIT        0x01 // If the set should be initially sent after enabling the bidirectional mode
#define KPA_BEACON_FLAG_SYSEX       0x02 // If the set should send NRPN CC (0) or use SYSEX (1)
#define KPA_BEACON_FLAG_ECHO        0x04 // If set, parameters are sent back even if they were set via MIDI IN
#define KPA_BEACON_FLAG_NOFE        0x08 // if set, the KPA will stop sending 0xFE when protocol is initiated
#define KPA_BEACON_FLAG_NOCTR       0x10 // if set, the KPA will not send back the periodic KPA_PARAM_ACTIVE_SENSING_SIGNAL
#define KPA_BEACON_FLAG_TUNEMODE    0x20 // if set, the Tuning information is only sent in Tuner Mode, otherwise it's being sent all the time

/*uint8_t beaconFlags = KPA_BEACON_FLAG_INIT        // Send current state of the KPA on receiving beacon
                      | KPA_BEACON_FLAG_SYSEX     // library works only with SysEx
                      | KPA_BEACON_FLAG_TUNEMODE; // tuner will send only in tuner mode*/
uint8_t beaconFlags = 0; //KPA_BEACON_FLAG_SYSEX; // Decided not to use the beacon method of the KPA as it interferes with the VController message requests to the KPA

// To request the owner name of the rig send: F0 00 20 33 02 7F 06 00 00 00 06 15 09 00 00 00 00 00 F7
// KPA will respond with: F0 00 20 33 00 00 07 00 00 00 0C 1A 41 4F 6E 6E 6F 20 47 65 65 72 6C 69  6E 67 00 F7

// KPA addresses
#define KPA_RIG_TEMPO_ADDRESS 0x0400
#define KPA_MODE_ADDRESS 0x7F7D
#define KPA_PARAM_FLOORBOARD_NAME_ADDRESS  0x7F7F
#define KPA_PARAM_RIG_NAME_ADDRESS 0x0001


void MD_KPA_class::init() { // Default values for variables
  MD_base_class::init();

  // Line6 KPA variables:
  enabled = DEVICE_DETECT; // Kemper can be detected via USB, but not via regular MIDI...
  strcpy(device_name, "KPA");
  strcpy(full_device_name, "Kemper Profiler");
  patch_min = KPA_PERFORMANCE_PATCH_MIN;
  patch_max = KPA_PERFORMANCE_PATCH_MAX;
  max_times_no_response = MAX_TIMES_NO_RESPONSE; // The number of times the KPA does not have to respond before disconnection
  sysex_delay_length = 0; // time between sysex messages (in msec).
  my_LED_colour = 10; // Default value: soft green
  MIDI_channel = KPA_MIDI_CHANNEL; // Default value
  MIDI_port = KPA_MIDI_PORT; // Default value
  my_device_page1 = KPA_DEFAULT_PAGE1; // Default value
  my_device_page2 = KPA_DEFAULT_PAGE2; // Default value
  my_device_page3 = KPA_DEFAULT_PAGE3; // Default value
  my_device_page4 = KPA_DEFAULT_PAGE4; // Default value
  tuner_active = false;
  current_mode = KPA_PERFORMANCE_MODE;
  max_looper_length = 30000000; // Normal stereo looper time is 30 seconds - time given in microseconds
}

void MD_KPA_class::update() {
  if (!connected) return;
  looper_timer_check();
}

// ********************************* Section 2: KPA common MIDI in functions ********************************************
void MD_KPA_class::check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port) { // Check incoming sysex messages from  Called from MIDI:OnSysEx/OnSerialSysEx

  // Check if it is a message from an KPA
  if ((sxdata[1] == 0x00) && (sxdata[2] == 0x20) && (sxdata[3] == 0x33) && (port == MIDI_port)) {

    if (sxdata[6] == KPA_FUNCTION_SINGLE_PARAMETER_CHANGE) {
      uint16_t address = (sxdata[8] << 8) + sxdata[9];

      if (address == KPA_MODE_ADDRESS) {
        if (!connected) {
          connect(0x7F, port); // Will connect to the device
        }
        if (current_mode != sxdata[11]) { // Check for mode change
          switch_mode(sxdata[11]);
          if (last_requested_sysex_address == KPA_MODE_ADDRESS) {
            SP[last_requested_sysex_switch].Target_byte1 = current_mode;
            check_update_label(last_requested_sysex_switch, current_mode);
          }
          update_page = REFRESH_PAGE;
          request_current_patch_name();
        }
      }

      if (address == (last_requested_sysex_address & 0xFF00)) { // Effect type received
        read_FX_type(last_requested_sysex_switch, sxdata[11]);
      }

      if (address == last_requested_sysex_address)  { // Effect state received
        SP[last_requested_sysex_switch].State = (sxdata[11] == 0x01) ? 1 : 2;
        PAGE_request_next_switch();
      }
    }
    if (sxdata[6] == KPA_FUNCTION_STRING_PARAMETER_CHANGE) { //Rig Name passed as STRING_PARAMETER_CHANGE
      uint16_t address = (sxdata[8] << 8) + sxdata[9];
      if ((current_mode == KPA_BROWSE_MODE) && (address == KPA_PARAM_RIG_NAME_ADDRESS)) {
        current_patch_name = "";
        bool end_reached = false;
        for (uint8_t count = 10; count < 26; count++) {
          if (sxdata[count] == 0x00) end_reached = true;
          if (!end_reached) current_patch_name += static_cast<char>(sxdata[count]); //Add ascii character or space to Patch Name String
          else current_patch_name += ' ';
        }
        update_main_lcd = true;
        if (popup_patch_name) {
          LCD_show_popup_label(current_patch_name, ACTION_TIMER_LENGTH);
          popup_patch_name = false;
        }
      }
    }
    if (sxdata[6] == KPA_FUNCTION_EXTENDED_STRING_PARAMETER_CHANGE) { // Check for performance/slot name passed as EXTENDED_STRING_PARAMETER_CHANGE
      if ((current_mode == KPA_PERFORMANCE_MODE) && (sxdata[8] == 0) && (sxdata[9] == 0) && (sxdata[10] == 1) && (sxdata[11] == 0)) {
        // Read name
        String pname = "";
        if (sxlength > 14) { // Return data contains name
          bool end_reached = false;
          for (uint8_t count = 13; count < 29; count++) {
            if (sxdata[count] == 0x00) end_reached = true; // End-of-string read
            if (!end_reached) pname += static_cast<char>(sxdata[count]); //Add ascii character or space to pname
            else pname += ' ';
          }
        }
        else { // Return data contains no name - slot is empty
          pname = "--";
        }
        DEBUGMSG(pname);
        if (sxdata[12] == 0) { // Read performance name
          current_patch_name = pname;
          update_main_lcd = true;
        }
        else if ((last_requested_sysex_type == REQUEST_PATCH_NAME) && (sxdata[12] == last_requested_sysex_address)) { // Read performance slot name
          // If name is "Çrunch"'or "SLOT"'then slot is empty!
          if (((pname == "Crunch          ") && (last_requested_sysex_address > 1)) || (pname == "Slot            ")) pname = "--";
          LCD_set_SP_label(last_requested_sysex_switch, pname);
          PAGE_request_next_switch();
        }
      }
    }
  }
}

void MD_KPA_class::check_PC_in(uint8_t program, uint8_t channel, uint8_t port) {  // Check incoming PC messages from  Called from MIDI:OnProgramChange

  // Check the source by checking the channel
  if ((port == MIDI_port) && (channel == MIDI_channel)) { // KPA sends a program change
    if (patch_number != program) {
      prev_patch_number = patch_number;
      patch_number = program;
      //page_check();
      do_after_patch_selection();
      update_page = REFRESH_PAGE;
    }
  }
}

// Device detection:
// KPA sends out active sense messages. The VController will receive these and then request the owner name from the KPA to confirm we are connected to a KPA and not some other device that supports Active Sense
// The active sense messages are also used to check the connection of the KPA to the VController

void MD_KPA_class::check_active_sense_in(uint8_t port) {
  no_response_counter = 0; // KPA is still live...
  if ((enabled == DEVICE_DETECT) && (!connected)) { // Try to connect...
    start_KPA_detection = true;
    MIDI_port = port;
    //DEBUGMSG("Active sense received");
  }
}

void MD_KPA_class::send_alternative_identity_request(uint8_t check_device_no) {
  if ((start_KPA_detection) && (check_device_no == 0)) {
    //request_owner_name();
    request_single_parameter(KPA_MODE_ADDRESS); // We keep requesting the mode, so a mode change on the KPA is also detected
  }
}

void MD_KPA_class::do_after_connect() {
  current_looper_state = LOOPER_STATE_ERASED;
  current_exp_pedal = 3; // Select volume by default
  current_performance = 255;
  char floorboard_name[] = "V-Controller";
  write_sysex_string(KPA_PARAM_FLOORBOARD_NAME_ADDRESS, floorboard_name);
  send_beacon(0, 0, 127); // We are not using the beacon messages of the KPA, as it interferes with the info requests of the VController
  //send_beacon(2, beaconFlags, 127); // So changes on the KPA will be communicated through sysex messages.
}

// ********************************* Section 3: KPA common MIDI out functions ********************************************
void MD_KPA_class::write_sysex(uint16_t address, uint16_t value) { // For single parameter change

  uint8_t sysexmessage[13] = {0xF0, 0x00, 0x20, 0x33, 0x02, 0x7F, KPA_FUNCTION_SINGLE_PARAMETER_CHANGE, 0x00, (uint8_t) (address >> 8), (uint8_t) (address & 0x7F), (uint8_t) (value >> 7), (uint8_t) (value & 0x7F), 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 13, MIDI_port);
}

void MD_KPA_class::write_sysex_string(uint16_t address, char *str) {
  uint8_t str_length = strlen(str);
  uint8_t sysexmessage[12 + str_length] = {0xF0, 0x00, 0x20, 0x33, 0x02, 0x7F, KPA_FUNCTION_STRING_PARAMETER_CHANGE, 0x00, (uint8_t) (address >> 8), (uint8_t) (address & 0x7F)};
  memcpy(&sysexmessage[10], str, str_length); // Copy the string
  sysexmessage[10 + str_length] = 0x00;
  sysexmessage[11 + str_length] = 0xF7;
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 12 + str_length, MIDI_port);
}

void MD_KPA_class::send_beacon(uint8_t setNum, uint8_t flags, uint8_t timeLease)
{
  uint8_t sysexmessage[13] = {0xF0, 0x00, 0x20, 0x33, 0x02, 0x7F, KPA_FUNCTION_SYS_COMMUNICATION, 0x00, 0x40, setNum, flags, timeLease, 0xF7};
  check_sysex_delay();
  DEBUGMSG("Send beacon");
  MIDI_send_sysex(sysexmessage, 13, MIDI_port);
}

void MD_KPA_class::request_single_parameter(uint16_t address) {
  uint8_t sysexmessage[11] = {0xF0, 0x00, 0x20, 0x33, 0x02, 0x7F, KPA_FUNCTION_REQUEST_SINGLE_PARAMETER_VALUE, 0x00, (uint8_t) (address >> 8), (uint8_t) (address & 0x7F), 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 11, MIDI_port);
}

void MD_KPA_class::request_current_rig_name() {
  uint8_t sysexmessage[11] = {0xF0, 0x00, 0x20, 0x33, 0x02, 0x7F, KPA_FUNCTION_REQUEST_STRING_PARAMETER, 0x00, 0x00, 0x01, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 11, MIDI_port);
}

void MD_KPA_class::request_performance_name(uint8_t number) {
  uint8_t sysexmessage[14] = {0xF0, 0x00, 0x20, 0x33, 0x02, 0x00, KPA_FUNCTION_REQUEST_EXTENDED_STRING_PARAMETER, 0x00, 0x00, 0x00, 0x01, 0x00, number, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 14, MIDI_port);
}

void MD_KPA_class::request_owner_name() {
  uint8_t sysexmessage[19] = {0xF0, 0x00, 0x20, 0x33, 0x02, 0x7F, 0x06, 0x00, 0x00, 0x00, 0x06, 0x15, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7};
  check_sysex_delay();
  DEBUGMSG("Requesting owner name");
  MIDI_send_sysex(sysexmessage, 19, MIDI_port);
}


void MD_KPA_class::set_bpm() {
  if (connected) {
    write_sysex(KPA_RIG_TEMPO_ADDRESS, Setting.Bpm);
  }
}

void MD_KPA_class::start_tuner() {
  if (connected) {
    MIDI_send_CC(31, 1, MIDI_channel, MIDI_port);
    tuner_active = true;
  }
}

void MD_KPA_class::stop_tuner() {
  if (connected) {
    MIDI_send_CC(31, 0, MIDI_channel, MIDI_port);
    tuner_active = false;
  }
}

// ********************************* Section 4: KPA program change ********************************************
// The Kemper has a number of ways to select rigs:
// 1) Program change. In browse mode this will select a rig if the number has been assigned to the rig. In performance mode this will select a performance/rig. There is no way to read the name of these rigs in advance
// 2) CC #47 val 0-125 - will pre-select a performance - after this a rig can be selected through CC #50 - 54, but the performance name of the new performance cannot be read.
// 3) CC #48/49. In browse mode this will do rig left/right. In performance mode this will do previous/next performance.
// 4) CC #50-54. In browse mode this will select item 1-5 in the list. In performance mode this will select slot 1-5 and these names can be read.

// The VController uses the following:
// 1) In performance mode, we use PC messages to change performance/slot. When going bank up/down, we use CC #48, 49 to pre-select the performance, as tis does allow to read the proper names
// 2) In browse mode, the first five patches will send CC #50 -54, so you can select the first five rigs that are currently selected in browse mode. Going bank up will select rigs. To get
//    this to work, you need to assign PC numbers to these rigs.

void MD_KPA_class::switch_mode(uint8_t mode) {
  current_mode = mode;
  if (current_mode == KPA_BROWSE_MODE) {
    patch_number = browse_rig_number;
    patch_min = KPA_BROWSE_PATCH_MIN;
    patch_max = KPA_BROWSE_PATCH_MAX;
  }
  else {
    patch_number = performance_rig_number;
    patch_min = KPA_PERFORMANCE_PATCH_MIN;
    patch_max = KPA_PERFORMANCE_PATCH_MAX;
  }
  prev_patch_number = patch_number;
  update_bank_number(patch_number);
}

void MD_KPA_class::select_patch(uint16_t new_patch) {
  prev_patch_number = patch_number;
  patch_number = new_patch;
  if (current_mode == KPA_BROWSE_MODE) {
    browse_rig_number = new_patch;
    if (new_patch < 5) { // Browse this number)
      uint8_t cc = 50 + new_patch;
      MIDI_send_CC(cc, 1, MIDI_channel, MIDI_port); // send CC #50 - #54 for rig select
    }
    else { // Select it as patch
      MIDI_send_CC(32, (new_patch - 5) >> 7, MIDI_channel, MIDI_port);
      MIDI_send_PC((new_patch - 5) & 0x7F, MIDI_channel, MIDI_port);
    }
  }
  else {
    performance_rig_number = new_patch;
    MIDI_send_CC(32, new_patch >> 7, MIDI_channel, MIDI_port);
    MIDI_send_PC(new_patch & 0x7F, MIDI_channel, MIDI_port);

    DEBUGMSG("out(KPA) PC" + String(new_patch)); //Debug
  }
  request_current_patch_name();
  do_after_patch_selection();

  update_page = REFRESH_PAGE;
}

void MD_KPA_class::number_format(uint16_t number, String & Output) {
  if (current_mode == KPA_BROWSE_MODE) {
    //uint16_t number_plus_one = number + 1;
    //Output +=  "B" + String(number_plus_one / 100) + String((number_plus_one / 10) % 10) + String(number_plus_one % 10);
    if (number < 5) Output += "BROWSE";
    else {
      uint16_t rig_no = number - 4;
      Output += "RIG" + String(rig_no / 100) + String((rig_no / 10) % 10) + String(rig_no % 10);
    }
  }
  else {
    uint16_t performance_no = (number / 5) + 1;
    Output += String(performance_no / 100) + String((performance_no / 10) % 10) + String(performance_no % 10) + "-" + String((number % 5) + 1);
  }
}

void MD_KPA_class::direct_select_format(uint16_t number, String & Output) {
  if (current_mode == KPA_BROWSE_MODE) {
    if (direct_select_state == 0) {
      uint8_t bank_no = (bank_select_number * 10) + number;
      Output +=  String(bank_no / 10) + String(bank_no % 10) + "_";
    }
    else {
      Output +=  String(bank_select_number / 10) + String(bank_select_number % 10) + String(number);
    }
  }
  else {
    if (direct_select_state == 0) {
      uint16_t bank_no = (bank_select_number * 10) + number;
      Output +=  String(bank_no / 10) + String(bank_no % 10) + "_-_";
    }
    else {
      Output +=  String(bank_select_number / 10) + String(bank_select_number % 10) + String(number) + "-_";
    }
  }
}

bool MD_KPA_class::valid_direct_select_switch(uint8_t number) {
  bool result = false;
  if (current_mode == KPA_BROWSE_MODE) {
    result = true;
  }
  else {
    if (direct_select_state == 0) { // Show all switches on first digit
      result = ((number * 50) + (bank_select_number * 500) <= (patch_max - patch_min));
    }
    else {
      if ((bank_select_number == 0) && (number == 0)) return false;
      if ((bank_select_number * 50) + (number * 5) <= (patch_max - patch_min)) result = true;
    }
  }
  return result;
}

void MD_KPA_class::direct_select_start() {
  Previous_bank_size = bank_size; // Remember the bank size
  device_in_bank_selection = my_device_number + 1;
  if (current_mode == KPA_BROWSE_MODE) bank_size = 100;
  else bank_size = 500;
  bank_select_number = 0; // Reset bank number
  direct_select_state = 0;
}

void MD_KPA_class::direct_select_press(uint8_t number) {
  if (!valid_direct_select_switch(number)) return;
  if (current_mode == KPA_BROWSE_MODE) {
    if (direct_select_state == 0) {
      // First digit pressed
      bank_select_number = number + (bank_select_number * 10);
      bank_size = 10;
      direct_select_state = 1;
    }
    else {
      // Second digit pressed
      device_in_bank_selection = 0;
      uint16_t new_patch = (bank_select_number * 10) + number + 5;
      if (new_patch <= patch_min) new_patch = patch_min + patch_number_offset;
      if (new_patch > patch_max) new_patch = patch_max;
      //bank_number = ((new_patch - patch_number_offset) / Previous_bank_size); // Set bank number to the new patch
      bank_size = Previous_bank_size;
      select_patch(new_patch - patch_number_offset);
      update_bank_number(patch_number);
      SCO_select_page(Previous_page);
    }
  }
  else {
    if (direct_select_state == 0) {
      // First digit pressed
      bank_size = 50;
      bank_select_number = (bank_select_number * 10) + number;
      direct_select_state = 1;
    }
    else  {
      // Second digit pressed
      uint16_t base_patch = (bank_select_number * 50) + (number - 1) * 5;
      //flash_bank_of_five = base_patch / 5;
      bank_select_number = (base_patch / Previous_bank_size);
      bank_size = Previous_bank_size;
      //DEBUGMSG("PREVIOUS BANK_SIZE: " + String(Previous_bank_size));
      SCO_select_page(KPA_DEFAULT_PAGE1); // Which should give PAGE_KPA_RIG_SELECT
      device_in_bank_selection = my_device_number + 1; // Go into bank mode
    }
  }
}

void MD_KPA_class::do_after_patch_selection() {
  is_on = connected;
  if (Setting.Send_global_tempo_after_patch_change == true) {
    set_bpm();
  }
  Current_patch_number = patch_number;
  update_LEDS = true;
  update_main_lcd = true;
  MD_base_class::do_after_patch_selection();
}

void MD_KPA_class::request_current_patch_name() {
  if (current_mode == KPA_BROWSE_MODE) request_current_rig_name();
  if (current_mode == KPA_PERFORMANCE_MODE) {
    //preselect_performance(patch_number);
    performance_name_requested = true;
    request_performance_name(0); // Request performance name
  }
}

const PROGMEM char KPA_number[5][4] = { "1st", "2nd", "3rd", "4th", "5th" };

bool MD_KPA_class::request_patch_name(uint8_t sw, uint16_t number) {
  if ((current_mode == KPA_PERFORMANCE_MODE) && (Current_page != PAGE_CURRENT_DIRECT_SELECT)) {
    if (can_request_sysex_data()) {
      last_requested_sysex_type = REQUEST_PATCH_NAME;
      last_requested_sysex_address = number % 5 + 1;
      last_requested_sysex_switch = sw;
      //preselect_performance(number);
      request_performance_name(last_requested_sysex_address);
      return false;
    }
    else {
      LCD_clear_SP_label(sw);
      return true;
    }
  }
  else {
    String msg;
    if (number < 5) {
      msg = KPA_number[number];
      msg += " listed rig";
    }
    else {
      msg = "(Bank:" + String ((number - 5) / 128) + " PC:" + String((number - 5) % 128) + ')';
    }
    LCD_set_SP_label(sw, msg);
    return true;
  }
}

void MD_KPA_class::request_bank_name(signed int delta, uint16_t number) {
  if (current_mode == KPA_PERFORMANCE_MODE) {
    //preselect_performance(number);
    if (delta < 0) MIDI_send_CC(49, 0, MIDI_channel, MIDI_port);
    else MIDI_send_CC(48, 0, MIDI_channel, MIDI_port);
    request_performance_name(0); // Request performance name
  }
}

// ********************************* Section 5: KPA parameter control ********************************************
// Define array for KPA effeect names and colours
struct KPA_CC_type_struct { // Combines all the data we need for controlling a parameter in a device
  char Name[11]; // The name for the label
  uint16_t Address;
  uint8_t CC; // The cc for this effect.
};

const PROGMEM KPA_CC_type_struct KPA_CC_types[] = {
  {"STOMP A", 0x3203, 17}, // 0
  {"STOMP B", 0x3303, 18},
  {"STOMP C", 0x3403, 19},
  {"STOMP D", 0x3503, 20},
  {"STOMP X", 0x3803, 22},
  {"STOMP MOD", 0x3A03, 24},
  {"STOMP DLY", 0x3C03, 26},
  {"STOMP RVB", 0x4B02, 28},
  {"ROTARY SPD", 0x0000, 33},
  {"DLY FB INF", 0x0000, 34},
  {"DELAY HOLD", 0x0000, 35}, // 10
  {"WAH PDL", 0x0000, 1},
  {"MORPH PDL", 0x0000, 11},
  {"VOLUME", 0x0000, 7},
  {"PITCH PDL", 0x0000, 2},
  {"MODE", KPA_MODE_ADDRESS, 0},
  {"RIG LEFT", 0x0000, 48},
  {"RIG RIGHT", 0x0000, 49}
};

const uint16_t KPA_NUMBER_OF_PARAMETERS = sizeof(KPA_CC_types) / sizeof(KPA_CC_types[0]);
#define KPA_NUMBER_OF_FX_SLOTS 8

#define KPA_STOMP_DLY 6
#define KPA_STOMP_RVB 7
#define KPA_WAH_PEDAL 11
#define KPA_PITCH_PEDAL 14
#define KPA_VOL_PEDAL 13
#define KPA_MORPH_PEDAL 12
#define KPA_MODE 15
#define KPA_RIG_LEFT 16
#define KPA_RIG_RIGHT 17

struct KPA_FX_type_struct { // Combines all the data we need for controlling a parameter in a device
  uint8_t Number;
  char Name[17]; // The name for the label
  uint8_t Colour;
};

const PROGMEM KPA_FX_type_struct KPA_FX_types[] = {
  { 0, "--", FX_TYPE_OFF },
  // Wah Wah
  { 1, "Wah Wah", FX_WAH_TYPE },
  { 2, "Wah Low Pass", FX_WAH_TYPE },
  { 3, "Wah High Pass", FX_WAH_TYPE },
  { 4, "Wah Vowel Filter", FX_WAH_TYPE },
  { 6, "Wah Phaser", FX_WAH_TYPE  },
  { 7, "Wah Flanger", FX_WAH_TYPE },
  { 8, "Wah Rate Red", FX_WAH_TYPE },
  { 9, "Wah Ring Mod", FX_WAH_TYPE },
  { 10, "Wah Freq Shftr", FX_WAH_TYPE },
  { 12, "Wah Formant Shft", FX_WAH_TYPE },
  //Shapers
  { 17, "Bit Shaper", FX_FILTER_TYPE },
  { 18, "Recti Shaper", FX_FILTER_TYPE },
  { 19, "Soft Shaper", FX_FILTER_TYPE },
  { 20, "Hard Shaper", FX_FILTER_TYPE },
  { 21, "Wave Shaper", FX_FILTER_TYPE },
  //Distortion
  { 33, "Green Scream", FX_DIST_TYPE },
  { 34, "Plus DS", FX_DIST_TYPE },
  { 35, "One DS", FX_DIST_TYPE  },
  { 36, "Muffin", FX_DIST_TYPE },
  { 37, "Mouse", FX_DIST_TYPE },
  { 38, "Fuzz DS", FX_DIST_TYPE },
  { 39, "Metal DS", FX_DIST_TYPE },
  //Boosters
  { 113, "Treble Booster", FX_DIST_TYPE },
  { 114, "Lead Booster", FX_DIST_TYPE  },
  { 115, "Pure Booster", FX_DIST_TYPE },
  { 116, "Wah Pdl Booster", FX_DIST_TYPE },
  //EQs
  { 97, "Graphic Eq", FX_FILTER_TYPE },
  { 98, "Studio Eq", FX_FILTER_TYPE },
  { 99, "Metal Eq", FX_FILTER_TYPE },
  { 101, "Stereo Widener", FX_FILTER_TYPE },
  //Dynamics
  { 49, "Compressor", FX_DYNAMICS_TYPE },
  { 57, "Gate 2:1", FX_DYNAMICS_TYPE },
  { 58, "Gate 4:1", FX_DYNAMICS_TYPE },
  //Modulation
  { 65, "Vintage Chorus", FX_MODULATE_TYPE  },
  { 66, "Hyper Chorus", FX_MODULATE_TYPE },
  { 67, "Air Chorus", FX_MODULATE_TYPE },
  { 71, "Micro Pitch", FX_MODULATE_TYPE },
  { 68, "Vibrato", FX_MODULATE_TYPE },
  { 69, "Rotary Speaker", FX_MODULATE_TYPE },
  { 70, "Tremolo" , FX_MODULATE_TYPE },
  { 81, "Phaser", FX_MODULATE_TYPE },
  { 82, "Phaser Vibe", FX_MODULATE_TYPE },
  { 83, "Phaser Oneway", FX_MODULATE_TYPE },
  { 89, "Flanger", FX_MODULATE_TYPE },
  { 91, "Flanger Oneway", FX_MODULATE_TYPE },
  { 64, "Space" , FX_MODULATE_TYPE },
  //Pitch
  { 129, "Transpose", FX_PITCH_TYPE },
  { 11, "Pedal Pitch", FX_PITCH_TYPE },
  { 13, "Pedal Vinyl Stop", FX_PITCH_TYPE },
  { 130, "Chromatic Pitch", FX_PITCH_TYPE },
  { 131, "Harmonic Pitch", FX_PITCH_TYPE },
  { 132, "Analog Octaver" , FX_PITCH_TYPE },
  //Loops
  { 121, "Loop Mono", FX_LOOPER_TYPE },
  { 122, "Loop Stereo", FX_LOOPER_TYPE },
  { 123, "Loop Distortion", FX_LOOPER_TYPE },
  //Delay
  { 145, "Legacy Delay", FX_DELAY_TYPE },
  { 146, "Single Delay", FX_DELAY_TYPE },
  { 147, "Dual Delay", FX_DELAY_TYPE },
  { 148, "TwoTap Delay" , FX_DELAY_TYPE },
  { 149, "Serial 2Tap Dly", FX_DELAY_TYPE },
  { 150, "Crystal Delay", FX_DELAY_TYPE },
  { 151, "Loop Pitch Dly", FX_DELAY_TYPE },
  { 152, "Freq Shifter Dly", FX_DELAY_TYPE },
  { 161, "Rhythm Delay", FX_DELAY_TYPE },
  { 162, "Melody Chrom Dly" , FX_DELAY_TYPE },
  { 163, "Melody Harmo Dly", FX_DELAY_TYPE },
  { 164, "Quad Delay", FX_DELAY_TYPE },
  { 165, "Quad Chrom Dly", FX_DELAY_TYPE },
  { 166, "Quad Harmo Dly", FX_DELAY_TYPE },
  { 137, "Dual Chrom Dly", FX_DELAY_TYPE },
  { 138, "Dual Harmo Dly", FX_DELAY_TYPE },
  { 139, "Dual Crystal Dly" , FX_DELAY_TYPE },
  { 140, "2Loop Pitch Dly", FX_DELAY_TYPE },

  // Reverbs
  { 193, "Spring Reverb", FX_REVERB_TYPE },
  { 178, "Natural Reverb", FX_REVERB_TYPE },
  { 179, "Easy Reverb", FX_REVERB_TYPE },
  { 180, "Echo Reverb", FX_REVERB_TYPE },
  { 181, "Cirrus Reverb", FX_REVERB_TYPE },
  { 182, "Formant Reverb", FX_REVERB_TYPE },
  { 183, "Ionosphere Rvb", FX_REVERB_TYPE },
  { 177, "Legacy Reverb", FX_REVERB_TYPE },
};

const uint16_t KPA_NUMBER_OF_FX_TYPES = sizeof(KPA_FX_types) / sizeof(KPA_FX_types[0]);

const PROGMEM char KPA_rvb_type_names[6][11] = {
  "None",
  "Hall",
  "Large Room",
  "Small Room",
  "Ambience",
  "Matchbox"
};

const PROGMEM char KPA_parameter_names[][9] = {
  "BROWSE",
  "PERFORM"
};

void MD_KPA_class::clear_FX_states() {
  memset(effect_state, 0, KPA_NUMBER_OF_FX);
}

void MD_KPA_class::set_FX_state(uint8_t index, bool state) {
  effect_state[index] = (state == 0) ? 2 : 1; // Set state to 2 if state is zero and to 1 if state is not zero
}

void MD_KPA_class::parameter_press(uint8_t Sw, Cmd_struct * cmd, uint16_t number) {
  // Send cc command to Line6 KPA
  uint8_t value = SCO_return_parameter_value(Sw, cmd);
  if (number == KPA_MODE) {
    switch_mode(current_mode ^ 1);
  }

  //if ((number == KPA_RIG_LEFT) || (number == KPA_RIG_RIGHT)) value = 0;

  // Send the sysex or CC message
  if (KPA_CC_types[number].Address != 0x0000) write_sysex(KPA_CC_types[number].Address, value);
  else MIDI_send_CC(KPA_CC_types[number].CC, value, MIDI_channel, MIDI_port);
  set_FX_state(number, value);
  check_update_label(Sw, value);
  LCD_show_popup_label(SP[Sw].Label, ACTION_TIMER_LENGTH);
  if ((number == KPA_MODE) || (number == KPA_RIG_LEFT) || (number == KPA_RIG_RIGHT)) {
    request_current_patch_name();
  }
  update_page = REFRESH_FX_ONLY; // To update the other switch states, we re-load the current page
}

void MD_KPA_class::parameter_release(uint8_t Sw, Cmd_struct * cmd, uint16_t number) {
  if (SP[Sw].Latch == MOMENTARY) {
    SP[Sw].State = 2; // Switch state off
    effect_state[number] = 2;
    if (KPA_CC_types[number].Address != 0x0000) write_sysex(KPA_CC_types[number].Address, cmd->Value2);
    else MIDI_send_CC(KPA_CC_types[number].CC, cmd->Value2, MIDI_channel, MIDI_port);
  }
}

void MD_KPA_class::read_parameter_title(uint16_t number, String & Output) {
  Output += KPA_CC_types[number].Name;
}

bool MD_KPA_class::request_parameter(uint8_t sw, uint16_t number) {
  if ((can_request_sysex_data()) && (number < KPA_NUMBER_OF_FX_SLOTS)) {
    last_requested_sysex_address = KPA_CC_types[number].Address;
    last_requested_sysex_switch = sw;
    request_single_parameter(KPA_CC_types[number].Address & 0xFF00); // Request type
    request_single_parameter(KPA_CC_types[number].Address); // Request state
    return false; // Move to next switch is true.
  }
  else {
    SP[sw].State = effect_state[number];
    SP[sw].Colour = my_LED_colour;
    String msg = "";
    if ((number == KPA_WAH_PEDAL) || (number == KPA_PITCH_PEDAL) || (number == KPA_VOL_PEDAL) || (number == KPA_MORPH_PEDAL)) {
      msg = String(SP[sw].Target_byte1);
    }
    if ((number == KPA_MODE)  && (current_mode < 2)) {
      msg = KPA_parameter_names[current_mode];
      set_FX_state(KPA_MODE, current_mode);
    }
    if ((number == KPA_RIG_LEFT) || (number == KPA_RIG_RIGHT)) {
      SP[sw].Latch = MOMENTARY;
    }
    LCD_set_SP_label(sw, msg);
    return true;
  }
}

void MD_KPA_class::read_FX_type(uint8_t sw, uint8_t type) {
  if (type == 0) { // No effect in slot
    String msg = "--";
    LCD_set_SP_label(sw, msg);
    SP[sw].Colour = FX_TYPE_OFF;
    return;
  }

  // Support legacy reverb numbers - can probably be removed when Kemper brings out the new reverbs...
  if ((SP[sw].PP_number == KPA_STOMP_RVB) && (type < 6)) { // Lookup in reverb table
    LCD_set_SP_label(sw, KPA_rvb_type_names[type]);
    SP[sw].Colour = FX_REVERB_TYPE;
    return;
  }

  // Support legacy delay number
  if ((SP[sw].PP_number == KPA_STOMP_DLY) && (type = 1)) type = 145; // Legacy delay is sometimes numbered "1" - coming from older firmware?

  // Lookup FX type name in the KPA_FX_types array
  for (uint8_t i = 0; i < KPA_NUMBER_OF_FX_TYPES; i++) {
    if (KPA_FX_types[i].Number == type) {
      LCD_set_SP_label(sw, KPA_FX_types[i].Name);
      SP[sw].Colour = KPA_FX_types[i].Colour;
      return;
    }
  }
  String msg = "Type " + String(type); // Unknow type, just give the type number
  LCD_set_SP_label(sw, msg);
}

void MD_KPA_class::check_update_label(uint8_t Sw, uint8_t value) {
  String msg;
  uint16_t index = SP[Sw].PP_number;
  msg = KPA_CC_types[index].Name;
  if ((index == KPA_WAH_PEDAL) || (index == KPA_PITCH_PEDAL) || (index == KPA_VOL_PEDAL) || (index == KPA_MORPH_PEDAL)) {
    msg += ':';
    LCD_add_3digit_number(value, msg);
  }
  if ((index == KPA_MODE) && (value < 2)) {
    msg += ':';
    msg += KPA_parameter_names[value];
  }
  LCD_set_SP_label(Sw, msg);

  //Update the current switch label
  update_lcd = Sw;
}

// Menu options for FX states
void MD_KPA_class::read_parameter_name(uint16_t number, String & Output) { // Called from menu
  if (number < number_of_parameters())  Output = KPA_CC_types[number].Name;
  else Output = "?";
}

uint16_t MD_KPA_class::number_of_parameters() {
  return KPA_NUMBER_OF_PARAMETERS;
}

uint8_t MD_KPA_class::number_of_values(uint16_t parameter) {
  if ((parameter == KPA_WAH_PEDAL) || (parameter == KPA_PITCH_PEDAL) || (parameter == KPA_VOL_PEDAL) || (parameter == KPA_MORPH_PEDAL)) return 128; // Return 128 for the expression pedals
  if (parameter < number_of_parameters()) return 2;
  else return 0;
}

void MD_KPA_class::read_parameter_value_name(uint16_t number, uint16_t value, String & Output) {
  if (number < number_of_parameters()) Output += String(value);
  else Output += " ? "; // Unknown parameter
}

void MD_KPA_class::move_expression_pedal(uint8_t sw, uint8_t value, uint8_t exp_pedal) {
  uint8_t number;
  if (exp_pedal == 0) exp_pedal = current_exp_pedal;
  switch (exp_pedal) {
    case 1: number = KPA_WAH_PEDAL; break;
    case 2: number = KPA_MORPH_PEDAL; break;
    case 3: number = KPA_VOL_PEDAL; break;
    default: return;
  }
  LCD_show_bar(0, value); // Show it on the main display
  MIDI_send_CC(KPA_CC_types[number].CC, value, MIDI_channel, MIDI_port);
  check_update_label(sw, value);
  String msg = KPA_CC_types[number].Name;
  msg += ':';
  LCD_add_3digit_number(value, msg);
  LCD_show_popup_label(msg, ACTION_TIMER_LENGTH);
  update_page = REFRESH_FX_ONLY; // To update the other switch states, we re-load the current page
}

void MD_KPA_class::toggle_expression_pedal(uint8_t sw) {
  if (current_exp_pedal == 0) return;
  current_exp_pedal++;
  if (current_exp_pedal > 3) current_exp_pedal = 1;
  update_page = REFRESH_FX_ONLY;
}

void MD_KPA_class::set_expr_title(uint8_t sw) {
  const char KPA_Exp0[] = " WAH  MRPH  VOL ";
  const char KPA_Exp1[] = "[WAH] MRPH  VOL ";
  const char KPA_Exp2[] = " WAH [MRPH] VOL ";
  const char KPA_Exp3[] = " WAH  MRPH [VOL]";
  if (current_exp_pedal == 1) LCD_set_SP_title(sw, KPA_Exp1);
  else if (current_exp_pedal == 2) LCD_set_SP_title(sw, KPA_Exp2);
  else if (current_exp_pedal == 3) LCD_set_SP_title(sw, KPA_Exp3);
  else LCD_set_SP_title(sw, KPA_Exp0);
}

bool MD_KPA_class::request_exp_pedal(uint8_t sw, uint8_t exp_pedal) {
  if (exp_pedal == 0) exp_pedal = current_exp_pedal;
  uint8_t number = 0;
  if (exp_pedal == 1) number = KPA_WAH_PEDAL;
  if (exp_pedal == 2) number = KPA_MORPH_PEDAL;
  if (exp_pedal == 3) number = KPA_VOL_PEDAL;
  SP[sw].PP_number = number;
  LCD_clear_SP_label(sw);
  return true;
}

bool MD_KPA_class::looper_active() {
  return true;
}

const PROGMEM uint8_t KPA_looper_cc2_val[] = { // Table with the cc messages
  0,   // HIDE
  0, // SHOW
  89,   // STOP
  88, // PLAY
  88, // REC
  88,   // OVERDUB
  91,   // FORWARD
  91, // REVERSE
  92,   // FULL_SPEED
  92, // HALF_SPEED
  93, // UNDO
  93, // REDO
  90, // TRIGGER
  0,    // PRE
  0,    // POST
};

const uint8_t KPA_LOOPER_NUMBER_OF_CCS = sizeof(KPA_looper_cc2_val);

void MD_KPA_class::send_looper_cmd(uint8_t cmd) {
  if (cmd < KPA_LOOPER_NUMBER_OF_CCS) {
    if (KPA_looper_cc2_val[cmd] > 0) {
      MIDI_send_CC(99, 125, MIDI_channel, MIDI_port);
      MIDI_send_CC(98, KPA_looper_cc2_val[cmd], MIDI_channel, MIDI_port);
      MIDI_send_CC(6, 0, MIDI_channel, MIDI_port);
      MIDI_send_CC(38, 1, MIDI_channel, MIDI_port);
      //write_sysex(0x7D00 | KPA_looper_cc2_val[cmd], 1);
      last_looper_cmd = cmd;
    }
  }
}

void MD_KPA_class::looper_release() {
  if (KPA_looper_cc2_val[last_looper_cmd] > 0) {
    MIDI_send_CC(99, 125, MIDI_channel, MIDI_port);
    MIDI_send_CC(98, KPA_looper_cc2_val[last_looper_cmd], MIDI_channel, MIDI_port);
    MIDI_send_CC(6, 0, MIDI_channel, MIDI_port);
    MIDI_send_CC(38, 0, MIDI_channel, MIDI_port);
    //write_sysex(0x7D00 | KPA_looper_cc2_val[last_looper_cmd], 0);
  }
}
