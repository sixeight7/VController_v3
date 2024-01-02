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
#define KPA_MIDI_PORT MIDI1_PORT // Default port is MIDI1
#define KPA_PERFORMANCE_PATCH_MIN 0
#define KPA_PERFORMANCE_PATCH_MAX 124 // 125 performances
#define KPA_BROWSE_PATCH_MIN 0
#define KPA_BROWSE_PATCH_MAX 127 // 128 rigs in browse pool

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

// After sending a PC message, if there is a rig at this address, the Kemper sends after receiving the rig name:
// f0 00 20 33 00 00 03 00 00 00 32 00 f7
#define KPA_PARAM_CHECK_RIG_LOADED 0x0000

FLASHMEM void MD_KPA_class::init() { // Default values for variables
  MD_base_class::init();

  // Line6 KPA variables:
  enabled = DEVICE_DETECT; // Kemper can be detected via USB, but not via regular MIDI...
  strcpy(device_name, "KPA");
  strcpy(full_device_name, "Kemper Profiler");
  patch_min = KPA_PERFORMANCE_PATCH_MIN;
  patch_max = KPA_PERFORMANCE_PATCH_MAX;
  max_times_no_response = MAX_TIMES_NO_RESPONSE; // The number of times the KPA does not have to respond before disconnection
  sysex_delay_length = 5; // time between sysex messages (in msec).
  my_LED_colour = 10; // Default value: soft green
  MIDI_channel = KPA_MIDI_CHANNEL; // Default value
  MIDI_port_manual = MIDI_port_number(KPA_MIDI_PORT); // Default value
#if defined(CONFIG_VCTOUCH)
  my_device_page1 = KPA_DEFAULT_VCTOUCH_PAGE1; // Default values for VC-touch
  my_device_page2 = KPA_DEFAULT_VCTOUCH_PAGE2;
  my_device_page3 = KPA_DEFAULT_VCTOUCH_PAGE3;
  my_device_page4 = KPA_DEFAULT_VCTOUCH_PAGE4;
#elif defined(CONFIG_VCMINI)
  my_device_page1 = KPA_DEFAULT_VCMINI_PAGE1; // Default values for VC-mini
  my_device_page2 = KPA_DEFAULT_VCMINI_PAGE2;
  my_device_page3 = KPA_DEFAULT_VCMINI_PAGE3;
  my_device_page4 = KPA_DEFAULT_VCMINI_PAGE4;
#elif defined (CONFIG_CUSTOM)
  my_device_page1 = KPA_DEFAULT_CUSTOM_PAGE1; // Default values for custom VC device
  my_device_page2 = KPA_DEFAULT_CUSTOM_PAGE2;
  my_device_page3 = KPA_DEFAULT_CUSTOM_PAGE3;
  my_device_page4 = KPA_DEFAULT_CUSTOM_PAGE4;
#else
  my_device_page1 = KPA_DEFAULT_VC_PAGE1; // Default values for VController
  my_device_page2 = KPA_DEFAULT_VC_PAGE2;
  my_device_page3 = KPA_DEFAULT_VC_PAGE3;
  my_device_page4 = KPA_DEFAULT_VC_PAGE4;
#endif
  tuner_active = false;
  current_mode = KPA_PERFORMANCE_MODE;
  max_looper_length = 30000000; // Normal stereo looper time is 30 seconds - time given in microseconds
  page_update_timer = 0;
  last_read_rig_name.reserve(MAIN_LCD_DISPLAY_SIZE + 1);
  current_snapscene_label.reserve(MAIN_LCD_DISPLAY_SIZE + 1);

#ifdef IS_VCTOUCH
  device_pic = img_KPA;
#endif
}

FLASHMEM void MD_KPA_class::update() {
  if (!connected) return;
  page_update_timer_check();
  looper_timer_check();
}

// ********************************* Section 2: KPA common MIDI in functions ********************************************
FLASHMEM void MD_KPA_class::check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port) { // Check incoming sysex messages from  Called from MIDI:OnSysEx/OnSerialSysEx

  // Check if it is a message from an KPA
  if ((sxdata[1] == 0x00) && (sxdata[2] == 0x20) && (sxdata[3] == 0x33) && (port == MIDI_in_port)) {

    if (sxdata[6] == KPA_FUNCTION_SINGLE_PARAMETER_CHANGE) {
      uint16_t address = (sxdata[8] << 8) + sxdata[9];

      if (address == KPA_MODE_ADDRESS) {
        if (!connected) {
          connect(0x7F, port, Current_MIDI_out_port); // Will connect to the device
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
        read_FX_type(last_requested_sysex_switch, (sxdata[10] << 7) + sxdata[11]);
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
        uint8_t patch_name_size = MAIN_LCD_DISPLAY_SIZE;
        if (patch_name_size > 24) patch_name_size = 24;
        for (uint8_t count = 10; count < (10 + patch_name_size); count++) {
          if (sxdata[count] == 0x00) end_reached = true;
          if (!end_reached) current_patch_name += static_cast<char>(sxdata[count]); //Add ascii character or space to Patch Name String
          else current_patch_name += ' ';
        }
        update_main_lcd = true;
        if (popup_patch_name) {
          if (LCD_check_popup_allowed(0)) LCD_show_popup_label(current_patch_name, ACTION_TIMER_LENGTH);
          popup_patch_name = false;
        }
        if ((ready_to_read_rig_name) && (patch_number != last_checked_rig_number) && (sxdata[sxlength - 1] == 0xF7)) {
          if (current_patch_name != last_read_rig_name) {
            check_write_performance_name(patch_number + KPA_RIG_BASE_NUMBER, current_patch_name);
            last_checked_rig_number = patch_number;
          }
          else clear_performance_name(patch_number + KPA_RIG_BASE_NUMBER);
          update_page = REFRESH_PATCH_BANK_ONLY;
        }
        last_read_rig_name = current_patch_name;
        ready_to_read_rig_name = false;
      }
    }
    if (sxdata[6] == KPA_FUNCTION_EXTENDED_STRING_PARAMETER_CHANGE) { // Check for performance/slot name passed as EXTENDED_STRING_PARAMETER_CHANGE
      if ((current_mode == KPA_PERFORMANCE_MODE) && (sxdata[8] == 0) && (sxdata[9] == 0) && (sxdata[10] == 1) && (sxdata[11] == 0)) {
        // Read name
        String pname = "";
        if (sxlength > 14) { // Return data contains name
          bool end_reached = false;
          uint8_t patch_name_size = MAIN_LCD_DISPLAY_SIZE;
          if (patch_name_size > 24) patch_name_size = 24;
          for (uint8_t count = 13; count < (13 + patch_name_size); count++) {
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
          if ((patch_number == performance_on_kpa) && (sxdata[sxlength - 1] == 0xF7)) {
            current_patch_name = pname;
            check_write_performance_name(patch_number, pname);
            update_main_lcd = true;
          }
        }
        else if ((last_requested_sysex_type == REQUEST_PATCH_NAME) && (sxdata[12] == last_requested_sysex_address)) { // Read performance slot name
          // If name is "Çrunch"'or "SLOT"'then slot is empty!
          if (((pname.trim() == "Crunch") && (last_requested_sysex_address > 1)) || (pname.trim() == "Slot")) pname = "--";
          LCD_set_SP_label(last_requested_sysex_switch, pname);
          update_main_lcd = true;
          if ((current_snapscene == SP[last_requested_sysex_switch].State) && (current_snapscene > 0)) current_snapscene_label = pname;
          PAGE_request_next_switch();
        }
      }
    }
  }
}

FLASHMEM void MD_KPA_class::check_PC_in(uint8_t program, uint8_t channel, uint8_t port) {  // Check incoming PC messages from  Called from MIDI:OnProgramChange

  // Check the source by checking the channel
  if ((port == MIDI_in_port) && (channel == MIDI_channel)) { // KPA sends a program change
    if (patch_number != program) {
      set_patch_number(program);
      //page_check();
      do_after_patch_selection();
      update_page = REFRESH_PAGE;
    }
  }
}

FLASHMEM void MD_KPA_class::check_CC_in(uint8_t control, uint8_t value, uint8_t channel, uint8_t port) {
  if (!connected) return;
  if ((port == MIDI_in_port) && (channel == MIDI_channel)) {
    switch (control) {
      /*case 47:
        if (millis() > pc_change_timeout) {
          set_patch_number(value);
          //page_check();
          do_after_patch_selection();
          update_page = REFRESH_PAGE;
        }
        break;*/
      case 47:
        if (value < 127) {
          performance_on_kpa = value;
          if (current_mode != KPA_PERFORMANCE_MODE) {
            switch_mode(KPA_PERFORMANCE_MODE);
            update_page = REFRESH_PAGE;
            request_current_patch_name();
          }
        }
        else {
          if (current_mode != KPA_BROWSE_MODE) {
            switch_mode(KPA_BROWSE_MODE);
            update_page = REFRESH_PAGE;
            request_current_patch_name();
          }
        }
        break;
      case 50:
        if (millis() > pc_change_timeout) {
          current_snapscene = 1;
          update_page = REFRESH_PAGE;
        }
        break;
      case 51:
        if (millis() > pc_change_timeout) {
          current_snapscene = 2;
          update_page = REFRESH_PAGE;
        }
        break;
      case 52:
        if (millis() > pc_change_timeout) {
          current_snapscene = 3;
          update_page = REFRESH_PAGE;
        }
        break;
      case 53:
        if (millis() > pc_change_timeout) {
          current_snapscene = 4;
          update_page = REFRESH_PAGE;
        }
        break;
      case 54:
        if (millis() > pc_change_timeout) {
          current_snapscene = 5;
          update_page = REFRESH_PAGE;
        }
        break;
    }

  }
}

// Device detection:
// KPA sends out active sense messages. The VController will receive these and then request the owner name from the KPA to confirm we are connected to a KPA and not some other device that supports Active Sense
// The active sense messages are also used to check the connection of the KPA to the VController

FLASHMEM void MD_KPA_class::check_active_sense_in(uint8_t port) {
  no_response_counter = 0; // KPA is still live...
  if ((enabled == DEVICE_DETECT) && (!connected)) { // Try to connect...
    start_KPA_detection = true;
    MIDI_in_port = port;
    //DEBUGMSG("Active sense received");
  }
}

FLASHMEM void MD_KPA_class::send_alternative_identity_request(uint8_t check_device_no) {
  if (start_KPA_detection) {
    if ((connected) && (MIDI_out_port != Current_MIDI_out_port)) return; // Do not send data to other MIDI ports after connection has been made
    //request_owner_name();
    // We keep requesting the mode, so a mode change on the KPA is also detected
    uint8_t sysexmessage[11] = {0xF0, 0x00, 0x20, 0x33, 0x02, 0x7F, KPA_FUNCTION_REQUEST_SINGLE_PARAMETER_VALUE, 0x00, (uint8_t) (KPA_MODE_ADDRESS >> 8), (uint8_t) (KPA_MODE_ADDRESS & 0x7F), 0xF7};
    check_sysex_delay();
    MIDI_send_sysex(sysexmessage, 11, Current_MIDI_out_port);
    //request_single_parameter(KPA_MODE_ADDRESS, Current_MIDI_out_port); // We keep requesting the mode, so a mode change on the KPA is also detected
  }
}

FLASHMEM void MD_KPA_class::do_after_connect() {
  current_looper_state = LOOPER_STATE_ERASED;
  current_exp_pedal = 3; // Select volume by default
  current_performance = 255;
  char floorboard_name[] = VC_NAME;
  write_sysex_string(KPA_PARAM_FLOORBOARD_NAME_ADDRESS, floorboard_name);
  send_beacon(0, 0, 127); // We are not using the beacon messages of the KPA, as it interferes with the info requests of the VController
  //send_beacon(2, beaconFlags, 127); // So changes on the KPA will be communicated through sysex messages.
  DEBUGMAIN("Request tempo:");
  request_single_parameter(KPA_RIG_TEMPO_ADDRESS);
}

// ********************************* Section 3: KPA common MIDI out functions ********************************************
FLASHMEM void MD_KPA_class::write_sysex(uint16_t address, uint16_t value) { // For single parameter change

  uint8_t sysexmessage[13] = {0xF0, 0x00, 0x20, 0x33, 0x02, 0x7F, KPA_FUNCTION_SINGLE_PARAMETER_CHANGE, 0x00, (uint8_t) (address >> 8), (uint8_t) (address & 0x7F), (uint8_t) (value >> 7), (uint8_t) (value & 0x7F), 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 13, MIDI_out_port);
}

FLASHMEM void MD_KPA_class::write_sysex_string(uint16_t address, char *str) {
  uint8_t str_length = strlen(str);
  uint8_t sysexmessage[12 + str_length] = {0xF0, 0x00, 0x20, 0x33, 0x02, 0x7F, KPA_FUNCTION_STRING_PARAMETER_CHANGE, 0x00, (uint8_t) (address >> 8), (uint8_t) (address & 0x7F)};
  memcpy(&sysexmessage[10], str, str_length); // Copy the string
  sysexmessage[10 + str_length] = 0x00;
  sysexmessage[11 + str_length] = 0xF7;
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 12 + str_length, MIDI_out_port);
}

FLASHMEM void MD_KPA_class::send_beacon(uint8_t setNum, uint8_t flags, uint8_t timeLease)
{
  uint8_t sysexmessage[13] = {0xF0, 0x00, 0x20, 0x33, 0x02, 0x7F, KPA_FUNCTION_SYS_COMMUNICATION, 0x00, 0x40, setNum, flags, timeLease, 0xF7};
  check_sysex_delay();
  DEBUGMSG("Send beacon");
  MIDI_send_sysex(sysexmessage, 13, MIDI_out_port);
}

FLASHMEM void MD_KPA_class::request_single_parameter(uint16_t address) {
  uint8_t sysexmessage[11] = {0xF0, 0x00, 0x20, 0x33, 0x02, 0x7F, KPA_FUNCTION_REQUEST_SINGLE_PARAMETER_VALUE, 0x00, (uint8_t) (address >> 8), (uint8_t) (address & 0x7F), 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 11, MIDI_out_port);
}

FLASHMEM void MD_KPA_class::request_current_rig_name() {
  uint8_t sysexmessage[11] = {0xF0, 0x00, 0x20, 0x33, 0x02, 0x7F, KPA_FUNCTION_REQUEST_STRING_PARAMETER, 0x00, 0x00, 0x01, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 11, MIDI_out_port);
}

FLASHMEM void MD_KPA_class::request_performance_name(uint8_t number) {
  uint8_t sysexmessage[14] = {0xF0, 0x00, 0x20, 0x33, 0x02, 0x00, KPA_FUNCTION_REQUEST_EXTENDED_STRING_PARAMETER, 0x00, 0x00, 0x00, 0x01, 0x00, number, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 14, MIDI_out_port);
}

FLASHMEM void MD_KPA_class::request_owner_name() {
  uint8_t sysexmessage[19] = {0xF0, 0x00, 0x20, 0x33, 0x02, 0x7F, 0x06, 0x00, 0x00, 0x00, 0x06, 0x15, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7};
  check_sysex_delay();
  DEBUGMSG("Requesting owner name");
  MIDI_send_sysex(sysexmessage, 19, MIDI_out_port);
}


FLASHMEM void MD_KPA_class::set_bpm() {
  if (connected) {
    write_sysex(KPA_RIG_TEMPO_ADDRESS, Setting.Bpm * 64);
  }
}

FLASHMEM void MD_KPA_class::start_tuner() {
  if (connected) {
    MIDI_send_CC(31, 1, MIDI_channel, MIDI_out_port);
    tuner_active = true;
  }
}

FLASHMEM void MD_KPA_class::stop_tuner() {
  if (connected) {
    MIDI_send_CC(31, 0, MIDI_channel, MIDI_out_port);
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

FLASHMEM void MD_KPA_class::switch_mode(uint8_t mode) {
  current_mode = mode;
  if (current_mode == KPA_BROWSE_MODE) {
    set_patch_number(browse_rig_number);
    patch_min = KPA_BROWSE_PATCH_MIN;
    patch_max = KPA_BROWSE_PATCH_MAX;
    current_snapscene = 0;
    rig_browsing = true;
  }
  else {
    set_patch_number(performance_rig_number);
    patch_min = KPA_PERFORMANCE_PATCH_MIN;
    patch_max = KPA_PERFORMANCE_PATCH_MAX;
  }
  update_bank_number(patch_number);
}

FLASHMEM void MD_KPA_class::select_patch(uint16_t new_patch) {
  prev_patch_number = patch_number;
  patch_number = new_patch;
  if (current_mode == KPA_BROWSE_MODE) {
    browse_rig_number = new_patch;
    MIDI_send_CC(32, new_patch >> 7, MIDI_channel, MIDI_out_port);
    MIDI_send_PC(new_patch & 0x7F, MIDI_channel, MIDI_out_port);
    ready_to_read_rig_name = true;
    current_snapscene = 0;
    rig_browsing = false;
  }
  else {
    performance_rig_number = new_patch;
    MIDI_send_CC(32, (new_patch * 5) >> 7, MIDI_channel, MIDI_out_port);
    MIDI_send_PC((new_patch * 5) & 0x7F, MIDI_channel, MIDI_out_port);
    current_snapscene = 1;
  }
  page_update_timer = millis() + KPA_PAGE_UPDATE_TIME;
  pc_change_timeout = millis() + PC_CHANGE_TIME;
  update_page = REFRESH_PATCH_BANK_ONLY;
  update_main_lcd = true;
}

FLASHMEM void MD_KPA_class::number_format(uint16_t number, String & Output) {
  if (current_mode == KPA_BROWSE_MODE) {
    if (rig_browsing) {
      Output += "BROWSE";
    }
    else {
      uint16_t rig_no = number + 1;
      Output += "RIG" + String(rig_no / 100) + String((rig_no / 10) % 10) + String(rig_no % 10);
    }
  }
  else {
    uint16_t performance_no = number + 1;
    Output += String(performance_no / 100) + String((performance_no / 10) % 10) + String(performance_no % 10);
  }
}

// Song/setlist

/*FLASHMEM void MD_KPA_class::setlist_song_select(uint16_t item) { // This may require more
  //uint16_t new_patch = SCO_read_setlist_item(item);
  if (item > setlist_song_get_number_of_items()) return;
  if (item <= KPA_BROWSE_PATCH_MAX) {
    if (current_mode != KPA_BROWSE_MODE) set_mode(KPA_BROWSE_MODE);
    select_patch(item);
    patch_number = item;
  }
  else {
    if (current_mode != KPA_PERFORMANCE_MODE) set_mode(KPA_PERFORMANCE_MODE);
    uint8_t performance_no = (item - KPA_BROWSE_PATCH_MAX - 1) / 5;
    uint8_t slot_no = (item - KPA_BROWSE_PATCH_MAX - 1) % 5;
    select_patch(performance_no);
    set_snapscene(0, slot_no);
  }
}

FLASHMEM uint16_t MD_KPA_class::setlist_song_get_current_item_state() {
  if (current_mode == KPA_BROWSE_MODE) return patch_number;
  else return (patch_number * 5) + current_snapscene + KPA_BROWSE_PATCH_MAX + 1;
}

FLASHMEM uint16_t MD_KPA_class::setlist_song_get_number_of_items() {
  return (KPA_PERFORMANCE_PATCH_MAX + 1) * 5 + KPA_BROWSE_PATCH_MAX + 1;
}

FLASHMEM void MD_KPA_class::setlist_song_full_item_format(uint16_t item, String &Output) {
  Output = device_name;
  Output += ": ";
  setlist_song_short_item_format(item, Output);
}

FLASHMEM void MD_KPA_class::setlist_song_short_item_format(uint16_t item, String &Output) {
  if (item <= KPA_BROWSE_PATCH_MAX) {
    uint16_t rig_no = item + 1;
    Output += "RIG" + String(rig_no / 100) + String((rig_no / 10) % 10) + String(rig_no % 10);
  }
  else {
    uint8_t performance_no = (item - KPA_BROWSE_PATCH_MAX - 1) / 5 + 1;
    uint8_t slot_no = (item - KPA_BROWSE_PATCH_MAX - 1) % 5 + 1;
    Output += String(performance_no / 100) + String((performance_no / 10) % 10) + String(performance_no % 10);
    Output += " / ";
    Output += String(slot_no);
  }
}*/

FLASHMEM void MD_KPA_class::page_update_timer_check() {
  if (page_update_timer == 0) return;
  if (millis() > page_update_timer) {
    page_update_timer = 0;
    request_current_patch_name();
    do_after_patch_selection();
    update_page = REFRESH_PAGE;
  }
}

FLASHMEM void MD_KPA_class::check_write_performance_name(uint16_t number, String pname) {
  String old_name;
  uint16_t buffer_number = (number / 8) + 1;
  bool found = read_performance_name(number, old_name);

  if (!found) { // initialize KPA_name_buffer
    for (uint8_t i = 0; i < VC_PATCH_SIZE; i++) KPA_name_buffer[i] = ' ';
    current_buffer_number = buffer_number;
  }

  if (pname != old_name) {
    uint8_t name_index = (number % 8) * 22 + 16;
    uint8_t max_index = MAIN_LCD_DISPLAY_SIZE;
    if (max_index > 22) max_index = 22;
    for (uint8_t i = 0; i < max_index; i++) {
      KPA_name_buffer[name_index++] = (uint8_t) pname[i];
    }
    //Serial.println("Wrote:" + pname + " with patch number " + String(number) + ", bufnumber " + String(buffer_number) + " and index " + String(name_index - max_index));
    EEPROM_save_device_patch(my_device_number + 1, buffer_number, KPA_name_buffer, VC_PATCH_SIZE);
  }
}

FLASHMEM void MD_KPA_class::clear_performance_name(uint16_t number) {
  String blank_line = "";
  for (uint8_t i = 0; i < MAIN_LCD_DISPLAY_SIZE; i++) blank_line[i] = ' ';
  check_write_performance_name(number, blank_line);
}

FLASHMEM bool MD_KPA_class::read_performance_name(uint16_t number, String &pname) {
  uint16_t new_buffer_number = (number / 8) + 1;
  if (current_buffer_number != new_buffer_number) {
    //Serial.println("Loading buffer " + String(new_buffer_number));
    bool loaded = EEPROM_load_device_patch(my_device_number + 1, new_buffer_number, KPA_name_buffer, VC_PATCH_SIZE);
    if (loaded) current_buffer_number = new_buffer_number;
    else {
      //Serial.println("Buffer not loaded");
      return false;
    }
  }

  uint8_t name_index = (number % 8) * 22 + 16;
  pname = "";
  uint8_t max_index = MAIN_LCD_DISPLAY_SIZE;
  if (max_index > 22) max_index = 22;
  for (uint8_t i = 0; i < max_index; i++) {
    pname += (char) KPA_name_buffer[name_index++];
  }
  //Serial.println("Read:" + pname + " with patch number " + String(number) + ", bufnumber " + String(new_buffer_number) + " and index " + String(name_index - max_index));
  return true;
}

FLASHMEM void MD_KPA_class::check_after_editor_patch_dump() {
  current_buffer_number = 0; // Will trigger a reread of the buffer
  performance_on_kpa = 255;
  ready_to_read_rig_name = false;
  last_read_rig_name = "";
  last_checked_rig_number = 255;
}

FLASHMEM void MD_KPA_class::direct_select_format(uint16_t number, String & Output) {
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

FLASHMEM bool MD_KPA_class::valid_direct_select_switch(uint8_t number) {
  bool result = false;
  if (direct_select_state == 0) {
    result = ((number * 10) + (bank_select_number * 100) <= (patch_max - patch_min));
  }
  else {
    if ((bank_select_number == 0) && (number == 0)) return false;
    if ((bank_select_number * 10) + number <= (patch_max - patch_min)) result = true;
  }
  return result;
}

FLASHMEM void MD_KPA_class::direct_select_start() {
  Previous_bank_size = bank_size; // Remember the bank size
  device_in_bank_selection = my_device_number + 1;
  if (current_mode == KPA_BROWSE_MODE) bank_size = 100;
  else bank_size = 500;
  bank_select_number = 0; // Reset bank number
  direct_select_state = 0;
}

FLASHMEM void MD_KPA_class::direct_select_press(uint8_t number) {
  if (!valid_direct_select_switch(number)) return;
  //if (current_mode == KPA_BROWSE_MODE) {
  if (direct_select_state == 0) {
    // First digit pressed
    bank_select_number = number + (bank_select_number * 10);
    bank_size = 10;
    direct_select_state = 1;
  }
  else {
    // Second digit pressed
    device_in_bank_selection = 0;
    uint16_t new_patch = (bank_select_number * 10) + number;
    if (new_patch <= patch_min) new_patch = patch_min + patch_number_offset;
    if (new_patch > patch_max) new_patch = patch_max;
    //bank_number = ((new_patch - patch_number_offset) / Previous_bank_size); // Set bank number to the new patch
    bank_size = Previous_bank_size;
    select_patch(new_patch - patch_number_offset);
    update_bank_number(patch_number);
    SCO_select_page(Previous_page);
  }
  /*}
    else {
    if (direct_select_state == 0) {
      // First digit pressed
      bank_size = 10;
      bank_select_number = (bank_select_number * 10) + number;
      direct_select_state = 1;
    }
    else  {
      // Second digit pressed
      uint16_t base_patch = (bank_select_number * 10) + number - 1;
      //flash_bank_of_five = base_patch / 5;
      bank_select_number = (base_patch / Previous_bank_size);
      bank_size = Previous_bank_size;
      //DEBUGMSG("PREVIOUS BANK_SIZE: " + String(Previous_bank_size));
      SCO_select_page(my_device_page1); // Which should give PAGE_KPA_RIG_SELECT
      device_in_bank_selection = my_device_number + 1; // Go into bank mode
    }
    }*/
}

FLASHMEM void MD_KPA_class::do_after_patch_selection() {
  is_on = connected;
  if (Setting.Send_global_tempo_after_patch_change == true) {
    set_bpm();
  }
  Current_patch_number = patch_number;
  update_LEDS = true;
  update_main_lcd = true;
  MD_base_class::do_after_patch_selection();
}

FLASHMEM void MD_KPA_class::request_current_patch_name() {
  if (current_mode == KPA_BROWSE_MODE) request_current_rig_name();
  if (current_mode == KPA_PERFORMANCE_MODE) request_performance_name(0); // Request performance name
}

const PROGMEM char KPA_number[5][4] = { "1st", "2nd", "3rd", "4th", "5th" };

FLASHMEM bool MD_KPA_class::request_patch_name(uint8_t sw, uint16_t number) {
  if (current_mode == KPA_PERFORMANCE_MODE) {
    // Read from EEPROM
    String pname;
    bool found = read_performance_name(number, pname);
    if (found) LCD_set_SP_label(sw, pname);
    else LCD_clear_SP_label(sw);
    return true;
  }
  else {
    String msg;
    bool found = read_performance_name(number + KPA_RIG_BASE_NUMBER, msg);
    if ((!found) || (msg.trim() == "")) msg = "(PC " + String(number) + ')';
    LCD_set_SP_label(sw, msg);
    return true;
  }
}

// ********************************* Section 5: KPA parameter control ********************************************
// Define array for KPA effeect names and colours
struct KPA_CC_type_struct { // Combines all the data we need for controlling a parameter in a device
  char Name[11]; // The name for the label
  uint16_t Address;
  uint8_t CC; // The cc for this effect.
  uint8_t latch_type;
};

const PROGMEM KPA_CC_type_struct KPA_CC_types[] = {
  {"STOMP A", 0x3203, 17, TOGGLE}, // 0
  {"STOMP B", 0x3303, 18, TOGGLE},
  {"STOMP C", 0x3403, 19, TOGGLE},
  {"STOMP D", 0x3503, 20, TOGGLE},
  {"STOMP X", 0x3803, 22, TOGGLE},
  {"STOMP MOD", 0x3A03, 24, TOGGLE},
  {"STOMP DLY", 0x3C03, 27, TOGGLE},
  {"STOMP RVB", 0x3D03, 29, TOGGLE},
  {"ROTARY SPD", 0x0000, 33, TOGGLE},
  {"DLY FB INF", 0x0000, 34, TOGGLE},
  {"DELAY HOLD", 0x0000, 35, TOGGLE}, // 10
  {"MORPH BTN", 0x0000, 80, MOMENTARY},
  {"WAH PDL", 0x0000, 1, UPDOWN},
  {"MORPH PDL", 0x0000, 11, UPDOWN},
  {"VOLUME", 0x0000, 7, UPDOWN},
  {"PITCH PDL", 0x0000, 2, UPDOWN},
  {"MODE", KPA_MODE_ADDRESS, 0, TOGGLE},
  {"RIG UP", 0x0000, 48, ONE_SHOT},
  {"RIG DOWN", 0x0000, 49, ONE_SHOT},
  {"LOOPER POS", 0x7F35, 0, TOGGLE}
};

const uint16_t KPA_NUMBER_OF_PARAMETERS = sizeof(KPA_CC_types) / sizeof(KPA_CC_types[0]);
#define KPA_NUMBER_OF_FX_SLOTS 8

#define KPA_STOMP_DLY 6
#define KPA_STOMP_RVB 7
#define KPA_WAH_PEDAL 12
#define KPA_PITCH_PEDAL 15
#define KPA_VOL_PEDAL 14
#define KPA_MORPH_PEDAL 13
#define KPA_MODE 16
#define KPA_RIG_UP 17
#define KPA_RIG_DOWN 18
#define KPA_LOOPER_PRE_POST 19

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
  { 32, "Kemper Drive", FX_DIST_TYPE },
  { 33, "Green Scream", FX_DIST_TYPE },
  { 34, "Plus DS", FX_DIST_TYPE },
  { 35, "One DS", FX_DIST_TYPE  },
  { 36, "Muffin", FX_DIST_TYPE },
  { 37, "Mouse", FX_DIST_TYPE },
  { 38, "Kemper Fuzz", FX_DIST_TYPE },
  { 40, "Metal DS", FX_DIST_TYPE },
  { 42, "Fuzz OC", FX_DIST_TYPE },
  //Boosters
  { 113, "Treble Booster", FX_DIST_TYPE },
  { 114, "Lead Booster", FX_DIST_TYPE  },
  { 115, "Pure Booster", FX_DIST_TYPE },
  { 116, "Wah Pdl Booster", FX_DIST_TYPE },
  //EQs
  { 97, "Graphic Eq", FX_FILTER_TYPE },
  { 98, "Studio Eq", FX_FILTER_TYPE },
  { 99, "Metal Eq", FX_FILTER_TYPE },
  { 100, "Ac Sim", FX_FILTER_TYPE },
  { 101, "Stereo Widener", FX_FILTER_TYPE },
  { 102, "Phase Widener", FX_FILTER_TYPE },
  { 103, "Delay Widener", FX_FILTER_TYPE },
  { 104, "Double Tacker", FX_FILTER_TYPE },
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

FLASHMEM void MD_KPA_class::clear_FX_states() {
  memset(effect_state, 0, KPA_NUMBER_OF_FX);
}

FLASHMEM void MD_KPA_class::set_FX_state(uint8_t index, bool state) {
  effect_state[index] = (state == 0) ? 2 : 1; // Set state to 2 if state is zero and to 1 if state is not zero
}

FLASHMEM void MD_KPA_class::set_mode(uint8_t mode) {
  write_sysex(KPA_CC_types[KPA_MODE].Address, mode);
  switch_mode (mode);
}

FLASHMEM void MD_KPA_class::parameter_press(uint8_t Sw, Cmd_struct * cmd, uint16_t number) {
  // Send cc command to Line6 KPA
  uint8_t value = SCO_return_parameter_value(Sw, cmd);
  if (number == KPA_MODE) {
    switch_mode(current_mode ^ 1);
  }

  if ((number == KPA_RIG_DOWN) || (number == KPA_RIG_UP)) {
    if (current_mode == KPA_PERFORMANCE_MODE) {
      uint8_t s = current_snapscene;
      if (s > 5) s -= 5; // Check we are not in morph
      if (number == KPA_RIG_DOWN) {
        if (s > 1) set_snapscene(Sw, s - 1);
        return;
      }
      if (number == KPA_RIG_UP) {
        if (s < 5) set_snapscene(Sw, s + 1);
        return;
      }
    }
    else {
      rig_browsing = true;
    }
  }

  // Send the sysex or CC message
  if (KPA_CC_types[number].Address != 0x0000) write_sysex(KPA_CC_types[number].Address, value);
  else MIDI_send_CC(KPA_CC_types[number].CC, value, MIDI_channel, MIDI_out_port);
  set_FX_state(number, value);
  check_update_label(Sw, value);
  if (LCD_check_popup_allowed(Sw)) LCD_show_popup_label(SP[Sw].Label, ACTION_TIMER_LENGTH);
  if ((number == KPA_MODE) || (number == KPA_RIG_UP) || (number == KPA_RIG_DOWN)) {
    request_current_patch_name();
  }
  if (number == KPA_LOOPER_PRE_POST) looper_reset();
  update_page = REFRESH_FX_ONLY; // To update the other switch states, we re-load the current page
}

FLASHMEM void MD_KPA_class::parameter_release(uint8_t Sw, Cmd_struct * cmd, uint16_t number) {
  DEBUGMSG("SP.Latch: " + String(SP[Sw].Latch));
  if (SP[Sw].Latch == MOMENTARY) {
    SP[Sw].State = 2; // Switch state off
    effect_state[number] = 2;
    if (KPA_CC_types[number].Address != 0x0000) write_sysex(KPA_CC_types[number].Address, cmd->Value2);
    else MIDI_send_CC(KPA_CC_types[number].CC, cmd->Value2, MIDI_channel, MIDI_out_port);
  }
}

FLASHMEM void MD_KPA_class::read_parameter_title(uint16_t number, String & Output) {
  Output += KPA_CC_types[number].Name;
}

FLASHMEM bool MD_KPA_class::request_parameter(uint8_t sw, uint16_t number) {
  if ((can_request_sysex_data()) && (number < KPA_NUMBER_OF_FX_SLOTS)) {
    last_requested_sysex_address = KPA_CC_types[number].Address;
    last_requested_sysex_switch = sw;
    request_single_parameter(KPA_CC_types[number].Address & 0xFF00); // Request type
    request_single_parameter(KPA_CC_types[number].Address); // Request state
    return false;
  }
  else if ((current_mode == KPA_PERFORMANCE_MODE) && (number == KPA_RIG_DOWN)) {
    uint8_t s = current_snapscene;
    if (s > 5) s -= 5; // Check we are not in morph
    if (s > 1) {
      last_requested_sysex_type = REQUEST_PATCH_NAME;
      last_requested_sysex_address = s - 1;
      last_requested_sysex_switch = sw;
      request_performance_name(last_requested_sysex_address);
      return false;
    }
    else {
      LCD_clear_SP_label(sw);
      return true;
    }
  }
  else if ((current_mode == KPA_PERFORMANCE_MODE) && (number == KPA_RIG_UP)) {
    uint8_t s = current_snapscene;
    if (s > 5) s -= 5; // Check we are not in morph
    if (s < 5) {
      last_requested_sysex_type = REQUEST_PATCH_NAME;
      last_requested_sysex_address = s + 1;
      last_requested_sysex_switch = sw;
      request_performance_name(last_requested_sysex_address);
      return false;
    }
    else {
      LCD_clear_SP_label(sw);
      return true;
    }
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
    if (number == KPA_LOOPER_PRE_POST) {
      if (SP[sw].State == 1) msg = " PRE [POST]";
      if (SP[sw].State == 2) msg = "[PRE] POST";
    }
    if (SP[sw].Type == PAR_BANK) {
      SP[sw].Latch = KPA_CC_types[number].latch_type;
    }
    LCD_set_SP_label(sw, msg);
    return true;
  }
}

FLASHMEM void MD_KPA_class::read_FX_type(uint8_t sw, uint8_t type) {
  if (type == 0) { // No effect in slot
    String msg = "--";
    LCD_set_SP_label(sw, msg);
    SP[sw].Colour = FX_TYPE_OFF;
    return;
  }

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

FLASHMEM void MD_KPA_class::check_update_label(uint8_t Sw, uint8_t value) {
  uint16_t index = SP[Sw].PP_number;
  if ((index == KPA_RIG_UP) || (index == KPA_RIG_DOWN)) return;

  String msg = KPA_CC_types[index].Name;
  if ((index == KPA_WAH_PEDAL) || (index == KPA_PITCH_PEDAL) || (index == KPA_VOL_PEDAL) || (index == KPA_MORPH_PEDAL)) {
    msg += ':';
    LCD_add_3digit_number(value, msg);
  }
  else if ((index == KPA_MODE) && (value < 2)) {
    msg += ':';
    msg += KPA_parameter_names[value];
  }
  else return;

  LCD_set_SP_label(Sw, msg);

  //Update the current switch label
  update_lcd = Sw;
}

// Menu options for FX states
FLASHMEM void MD_KPA_class::read_parameter_name(uint16_t number, String & Output) { // Called from menu
  if (number < number_of_parameters())  Output = KPA_CC_types[number].Name;
  else Output = "?";
}

FLASHMEM uint16_t MD_KPA_class::number_of_parameters() {
  return KPA_NUMBER_OF_PARAMETERS;
}

FLASHMEM uint8_t MD_KPA_class::number_of_values(uint16_t parameter) {
  if ((parameter == KPA_WAH_PEDAL) || (parameter == KPA_PITCH_PEDAL) || (parameter == KPA_VOL_PEDAL) || (parameter == KPA_MORPH_PEDAL)) return 128; // Return 128 for the expression pedals
  if (parameter < number_of_parameters()) return 2;
  else return 0;
}

FLASHMEM void MD_KPA_class::read_parameter_value_name(uint16_t number, uint16_t value, String & Output) {
  if (number < number_of_parameters()) Output += String(value);
  else Output += " ? "; // Unknown parameter
}

FLASHMEM void MD_KPA_class::move_expression_pedal(uint8_t sw, uint8_t value, uint8_t exp_pedal) {
  uint8_t number;
  if (exp_pedal == 0) exp_pedal = current_exp_pedal;
  switch (exp_pedal) {
    case 1: number = KPA_WAH_PEDAL; break;
    case 2: number = KPA_MORPH_PEDAL; break;
    case 3: number = KPA_VOL_PEDAL; break;
    default: return;
  }
  LCD_show_bar(0, value, 0); // Show it on the main display
  MIDI_send_CC(KPA_CC_types[number].CC, value, MIDI_channel, MIDI_out_port);
  check_update_label(sw, value);
  String msg = KPA_CC_types[number].Name;
  msg += ':';
  LCD_add_3digit_number(value, msg);
  LCD_show_popup_label(msg, ACTION_TIMER_LENGTH);
  update_page = REFRESH_FX_ONLY; // To update the other switch states, we re-load the current page
}

FLASHMEM void MD_KPA_class::toggle_expression_pedal(uint8_t sw) {
  if (current_exp_pedal == 0) return;
  current_exp_pedal++;
  if (current_exp_pedal > 3) current_exp_pedal = 1;
  update_page = REFRESH_FX_ONLY;
}

FLASHMEM void MD_KPA_class::set_expr_title(uint8_t sw) {
  const char KPA_Exp0[] = " WAH  MRPH  VOL ";
  const char KPA_Exp1[] = "[WAH] MRPH  VOL ";
  const char KPA_Exp2[] = " WAH [MRPH] VOL ";
  const char KPA_Exp3[] = " WAH  MRPH [VOL]";
  if (current_exp_pedal == 1) LCD_set_SP_title(sw, KPA_Exp1);
  else if (current_exp_pedal == 2) LCD_set_SP_title(sw, KPA_Exp2);
  else if (current_exp_pedal == 3) LCD_set_SP_title(sw, KPA_Exp3);
  else LCD_set_SP_title(sw, KPA_Exp0);
}

FLASHMEM bool MD_KPA_class::request_exp_pedal(uint8_t sw, uint8_t exp_pedal) {
  if (exp_pedal == 0) exp_pedal = current_exp_pedal;
  uint8_t number = 0;
  if (exp_pedal == 1) number = KPA_WAH_PEDAL;
  if (exp_pedal == 2) number = KPA_MORPH_PEDAL;
  if (exp_pedal == 3) number = KPA_VOL_PEDAL;
  SP[sw].PP_number = number;
  LCD_clear_SP_label(sw);
  return true;
}

FLASHMEM void MD_KPA_class::get_snapscene_title(uint8_t number, String &Output) {
  if (number <= 5) Output += "SLOT " + String(number);
  else Output += "MORPH " + String(number - 5);
}

void MD_KPA_class::get_snapscene_title_short(uint8_t number, String &Output) {
  if (number <= 5) Output += "S" + String(number);
  else Output += "M" + String(number - 5);
}

FLASHMEM bool MD_KPA_class::request_snapscene_name(uint8_t sw, uint8_t sw1, uint8_t sw2, uint8_t sw3) {
  if (current_mode == KPA_PERFORMANCE_MODE) {
    if (can_request_sysex_data()) {
      last_requested_sysex_type = REQUEST_PATCH_NAME;
      last_requested_sysex_address = (SP[sw].PP_number - 1) % 5 + 1;
      last_requested_sysex_switch = sw;
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
    msg = KPA_number[((SP[sw].PP_number - 1) % 5)];
    msg += " listed rig";
    LCD_set_SP_label(sw, msg);
    return true;
  }
}

FLASHMEM void MD_KPA_class::get_snapscene_label(uint8_t number, String &Output) {
  if (current_mode == KPA_PERFORMANCE_MODE) Output += current_snapscene_label;
}

FLASHMEM void MD_KPA_class::set_snapscene(uint8_t sw, uint8_t number) {
  if (!is_on) unmute();
  if ((number < 1) || (number > 10)) return;
  send_morph_message = false;
  if (number > 5) send_morph_message = true;
  if ((current_snapscene - number) == 5)  send_morph_message = true;
  current_snapscene = number;
  if (current_snapscene <= 5) {
    uint8_t cc = current_snapscene + 49; // cc50-54 - used both in browse and performance mode
    MIDI_send_CC(cc, 1, MIDI_channel, MIDI_out_port);
    MIDI_send_current_snapscene(my_device_number, current_snapscene);
  }
  if (current_mode == KPA_PERFORMANCE_MODE) {
    if (send_morph_message) {
      MIDI_send_CC(80, 1, MIDI_channel, MIDI_out_port);
    }
    current_snapscene_label = SP[sw].Label;
  }
  else { // We are in browse mode
    request_current_patch_name();
    rig_browsing = true;
  }
}

FLASHMEM void MD_KPA_class::release_snapscene(uint8_t sw, uint8_t number) {
  if (send_morph_message) {
    MIDI_send_CC(80, 0, MIDI_channel, MIDI_out_port);
  }
}

FLASHMEM void MD_KPA_class::show_snapscene(uint8_t number) {
  if ((number < 1) || (number > 10)) return;
  if (number == current_snapscene) return;
  current_snapscene = number;
}

FLASHMEM void MD_KPA_class::snapscene_number_format(String &Output) { // Add snapshot number to potchnumber
  if (current_snapscene == 0) return;
  Output += '-';
  Output += String(current_snapscene);
}

FLASHMEM uint8_t MD_KPA_class::get_number_of_snapscenes() {
  return 10;
}


FLASHMEM bool MD_KPA_class::looper_active() {
  return true;
}

const PROGMEM uint8_t KPA_looper_cc2_val[] = { // Table with the cc messages
  0,  // HIDE
  0,  // SHOW
  89, // STOP
  88, // PLAY
  88, // REC
  88, // OVERDUB
  91, // FORWARD
  91, // REVERSE
  92, // FULL_SPEED
  92, // HALF_SPEED
  93, // UNDO
  93, // REDO
  90, // TRIGGER
  53, // PRE
  53, // POST
};

const uint8_t KPA_LOOPER_NUMBER_OF_CCS = sizeof(KPA_looper_cc2_val);

FLASHMEM bool MD_KPA_class::send_looper_cmd(uint8_t cmd) {
  if (cmd < KPA_LOOPER_NUMBER_OF_CCS) {
    if (KPA_looper_cc2_val[cmd] > 0) {
      MIDI_send_CC(99, 125, MIDI_channel, MIDI_out_port);
      MIDI_send_CC(98, KPA_looper_cc2_val[cmd], MIDI_channel, MIDI_out_port);
      MIDI_send_CC(6, 0, MIDI_channel, MIDI_out_port);
      MIDI_send_CC(38, 1, MIDI_channel, MIDI_out_port);
      //write_sysex(0x7D00 | KPA_looper_cc2_val[cmd], 1);
      last_looper_cmd = cmd;
    }
  }
  return true;
}

FLASHMEM void MD_KPA_class::looper_release() {
  if (KPA_looper_cc2_val[last_looper_cmd] > 0) {
    MIDI_send_CC(99, 125, MIDI_channel, MIDI_out_port);
    MIDI_send_CC(98, KPA_looper_cc2_val[last_looper_cmd], MIDI_channel, MIDI_out_port);
    MIDI_send_CC(6, 0, MIDI_channel, MIDI_out_port);
    MIDI_send_CC(38, 0, MIDI_channel, MIDI_out_port);
    //write_sysex(0x7D00 | KPA_looper_cc2_val[last_looper_cmd], 0);
  }
}
