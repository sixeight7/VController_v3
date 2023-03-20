// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: SY1000 Initialization
// Section 2: SY1000 common MIDI in functions
// Section 3: SY1000 common MIDI out functions
// Section 4: SY1000 program change
// Section 5: SY1000 parameter control
// Section 6: SY1000 assign control
// Section 7: SY1000 Scene control
// Section 8: SY1000 expression pedal control

// ********************************* Section 1: SY1000 Initialization ********************************************

// A few notes about the Boss SY-1000:
// * The VController switches editor mode on before any data is sent, otherwise the SY-1000 will display a message "Receiving data" and all further sysex midi data is on hold.
// * Editor mode produces a lot of data. So after the sysex data has been sent, edit mode will be switched off, but only for 100 ms seconds, otherwise we still get the Receiving data message.
// * Quirk that needed a workaround: when the SY1000 is connected with regular MIDI cables (not USB midi) and edit mode is switched on on the SY1000, the SY1000 is no longer sending CC messages when pressing
//   switches on this unit. Therefore we switch edit mode off when in this mode, apart from when data is sent to avoid the "Receiving data" message. But now commands like Write, Exchange, Initialize and
//   Insert Patch is no longer received. Regular writing can be simulated by detecting a PC message of the current patch. But for the other operations, the VC-mini must be in Patch, Manual or Num mode.

// Boss SY-1000 settings:
#define SY1000_MIDI_CHANNEL 7
#define SY1000_MIDI_PORT USBHMIDI_PORT
#define SY1000_PATCH_MIN 0
#define SY1000_PATCH_MAX 399

//Messages are abbreviated to just the address and the data bytes. Checksum is calculated automatically
//Example: {0xF0, 0x41, 0x10, 0x00, 0x00, 0x00, 0x69, 0x12, 0x7F, 0x00, 0x00, 0x01, 0x01, 0x7F, 0xF7} is reduced to 0x7F000001, 0x01

#define SY1000_EDITOR_MODE_ON 0x7F000001, 0x01
#define SY1000_EDITOR_MODE_OFF 0x7F000001, 0x00

#define SY1000_GM_CURRENT_PATCH_NAME 0x10000000 // Address for current patch name (16 bytes)
#define SY1000_GM_CURRENT_PATCH_NUMBER 0x00010000 // Address for current patch number (4 bytes)

#define SY1000_BM_CURRENT_PATCH_NAME 0x10020000 // Address for current Bass patch name (16 bytes)
#define SY1000_BM_CURRENT_PATCH_NUMBER 0x00080000 // Address for current Bass patch number (4 bytes)

#define SY1000_PATCH_SELECT 0x7F000100

#define SY1000_TUNER_ON 0x7F000002, 0x01 // 0x01 for polytuner, 0x02 for normal tuner
#define SY1000_TUNER_OFF 0x7F000002, 0x00 //Changes the running mode of the SY1000 to play.
//#define SY1000_SOLO_ON 0x2000500B, 0x01
//#define SY1000_SOLO_OFF 0x2000500B, 0x00

#define SY1000_GM_TEMPO 0x1000123E  // Accepts values from 400 bpm - 2500 bpm
#define SY1000_BM_TEMPO 0x1002123E

#define SY1000_BASS_MODE 0x7F000004
#define SY1000_SAVE_PATCH 0x7F000104
#define SY1000_EXCHANGE_PATCH 0x7F00010C
#define SY1000_INITIALIZE_PATCH 0x7f000106
#define SY1000_INSERT_PATCH 0x7f000108
#define SY1000_DONE_SAVING_PATCH 0x7F000100

#define SY1000_FOOT_VOL 0x20020803 // The address of the footvolume - values between 0 and 100

#define SY1000_GM_INST1_SW 0x10001500 // The address of the INST1 switch (guitar mode)
#define SY1000_GM_INST2_SW 0x10002000 // The address of the INST2 switch (guitar mode)
#define SY1000_GM_INST3_SW 0x10002B00 // The address of the INST3 switch (guitar mode)
#define SY1000_GM_NORMAL_PU_SW 0x10001201 // The address of the COSM guitar switch (guitar mode)

#define SY1000_BM_INST1_SW 0x10021500 // The address of the INST1 switch (bass mode)
#define SY1000_BM_INST2_SW 0x10021F00 // The address of the INST2 switch (bass mode)
#define SY1000_BM_INST3_SW 0x10022900 // The address of the INST3 switch (bass mode)
#define SY1000_BM_NORMAL_PU_SW 0x10021201 // The address of the COSM guitar switch (bass mode)

#define SY1000_GM_MASTER_KEY 0x10001242
#define SY1000_BM_MASTER_KEY 0x10021242

#define SY1000_GM_PATCH_MIDI 0x10000039
#define SY1000_BM_PATCH_MIDI 0x10020039

#define SY1000_FIRST_SWITCH_CC 1 // Switches on SY1000 should send CC 1 - 8
#define SY1000_FIRST_SCENE_ASSIGN_SOURCE_CC 21
#define SY1000_SOURCE_CC94 0x58
#define SY1000_SOURCE_CC95 0x59

#define SY1000_READ_SCENE_MIDI_TIMER_LENGTH 1000
#define SY1000_BASS_MODE_NUMBER_OFFSET 0x1000 // Bass mode patches are stored with higher number

#define MODE_NUM 0
#define MODE_MANUAL 1
#define MODE_PATCH 2
#define MODE_SCENE 3
#define MODE_SCENE_ASSIGN 4
#define MODE_SCENE_TOP_ROW 5
#define MODE_SCENE_BOTTOM_ROW 6

#define SY1000_RED 1
#define SY1000_BLUE 2
#define SY1000_LIGHT_BLUE 3
#define SY1000_GREEN 4
#define SY1000_YELLOW 5
#define SY1000_WHITE 6
#define SY1000_PURPLE 7

#define SY1000_SCENE_SELECTED_COLOUR SY1000_LIGHT_BLUE
#define SY1000_SECOND_SCENE_SELECTED_COLOUR SY1000_WHITE
#define SY1000_ACTIVE_SCENE_COLOUR SY1000_BLUE
#define SY1000_SCENE_ASSIGN_COLOUR SY1000_RED

// Initialize device variables
// Called at startup of VController
FLASHMEM void MD_SY1000_class::init() { // Default values for variables
  MD_base_class::init();

  // Boss SY-1000 variables:
  enabled = DEVICE_DETECT; // Default value
  //enabled = DEVICE_ON; // Default value
  strcpy(device_name, "SY1000");
  strcpy(full_device_name, "Boss SY-1000");
  patch_min = SY1000_PATCH_MIN;
  patch_max = SY1000_PATCH_MAX;
  max_times_no_response = MAX_TIMES_NO_RESPONSE; // The number of times the SY-1000 does not have to respond before disconnection
  INST_onoff[0] = 0;
  INST_onoff[1] = 0;
  INST_onoff[2] = 0;
  normal_pu_mute = 0;
  sysex_delay_length = 5; // minimum delay time between sysex messages (in msec).
  my_LED_colour = 11; // Default value: light blue
  my_snapscene_colour = 11;
  MIDI_channel = SY1000_MIDI_CHANNEL; // Default value
  MIDI_port_manual = MIDI_port_number(SY1000_MIDI_PORT); // Default value
#if defined(CONFIG_VCTOUCH)
  my_device_page1 = SY1000_DEFAULT_VCTOUCH_PAGE1; // Default value
  my_device_page2 = SY1000_DEFAULT_VCTOUCH_PAGE2; // Default value
  my_device_page3 = SY1000_DEFAULT_VCTOUCH_PAGE3; // Default value
  my_device_page4 = SY1000_DEFAULT_VCTOUCH_PAGE4; // Default value
#elif defined(CONFIG_VCMINI)
  my_device_page1 = SY1000_DEFAULT_VCMINI_PAGE1; // Default value
  my_device_page2 = SY1000_DEFAULT_VCMINI_PAGE2; // Default value
  my_device_page3 = SY1000_DEFAULT_VCMINI_PAGE3; // Default value
  my_device_page4 = SY1000_DEFAULT_VCMINI_PAGE4; // Default value
#elif defined (CONFIG_CUSTOM)
  my_device_page1 = SY1000_DEFAULT_CUSTOM_PAGE1; // Default value
  my_device_page2 = SY1000_DEFAULT_CUSTOM_PAGE2; // Default value
  my_device_page3 = SY1000_DEFAULT_CUSTOM_PAGE3; // Default value
  my_device_page4 = SY1000_DEFAULT_CUSTOM_PAGE4; // Default value
#else
  my_device_page1 = SY1000_DEFAULT_VC_PAGE1; // Default value
  my_device_page2 = SY1000_DEFAULT_VC_PAGE2; // Default value
  my_device_page3 = SY1000_DEFAULT_VC_PAGE3; // Default value
  my_device_page4 = SY1000_DEFAULT_VC_PAGE4; // Default value
#endif
  initialize_patch_space();
  //bass_mode = true;
  initialize_scene_assigns();
  //edit_mode_always_on = false;

#ifdef IS_VCTOUCH
  device_pic = img_SY1000;
#endif
}

void MD_SY1000_class::update() {
  if (!connected) return;
  //update_slow_gear();
  if (!edit_mode_always_on) check_edit_mode_return_timer();
  check_delayed_release_assignments();
  check_read_scene_midi_timer();
  check_patch_gap_timer();
  check_send_tempo_timer();
  check_midi_timer();
}

// ********************************* Section 2: SY1000 common MIDI in functions ********************************************

// The SY-1000 does this weird thing with USB MIDI when it gets busy. Insted of just sending the data, it sends everything double in chunks of three bytes.
// So instead of sending 0xF0, 0x41, 0x10, 0x00, 0x00, 0x00, 0x69, 0x12, 0x7F, 0x00, 0x00, 0x01, 0x01, 0x7F, 0xF7
// It will send:         0xF0, 0x41, 0x10, 0xF0, 0x41, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x69, 0x12, 0x7F, 0x69, 0x12, 0x7F, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x7F, 0xF7
// Not sure if this is an error in the USB Host library or an error of the SY-1000. But the SY-1000 is the only unit showing this behaviour.

uint16_t MD_SY1000_class::sx_index(uint8_t data3, uint16_t index) { // Crude workaround for reading MIDI through the host port.
  if (data3 == 0xF0) return (((index / 3) * 6) + (index % 3));
  else return index;
}

void MD_SY1000_class::check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port) { // Check incoming sysex messages from  Called from MIDI:OnSysEx/OnSerialSysEx

  // Check if it is a message from a SY-1000
  uint8_t data3 = sxdata[3];
  if ((port == MIDI_in_port) && (sxdata[sx_index(data3, 1)] == 0x41) && (sxdata[sx_index(data3, 2)] == MIDI_device_id) && (sxdata[sx_index(data3, 3)] == 0x00) && (sxdata[sx_index(data3, 4)] == 0x00) && (sxdata[sx_index(data3, 5)] == 0x00) && (sxdata[sx_index(data3, 6)] == 0x69) && (sxdata[sx_index(data3, 7)] == 0x12)) {
    uint32_t address = (sxdata[sx_index(data3, 8)] << 24) + (sxdata[sx_index(data3, 9)] << 16) + (sxdata[sx_index(data3, 10)] << 8) + sxdata[sx_index(data3, 11)]; // Make the address 32 bit

    // Check checksum
    uint16_t sum = 0;
    uint16_t sxlen = sxlength;
    if (data3 == 0xF0) sxlen = ((sxlen / 6) * 3) + (sxlen % 6); // Another crude workaround
    for (uint8_t i = 8; i < sxlen - 2; i++) sum += sxdata[sx_index(data3, i)];
    bool checksum_ok = (sxdata[sx_index(data3, sxlen - 2)] == calc_Roland_checksum(sum));

    // Check if it is the current patch number
    if (((address == SY1000_GM_CURRENT_PATCH_NUMBER) || (address == SY1000_BM_CURRENT_PATCH_NUMBER)) && (checksum_ok)) {
      uint16_t new_patch = (sxdata[sx_index(data3, 12)] << 12) + (sxdata[sx_index(data3, 13)] << 8) + (sxdata[sx_index(data3, 14)] << 4) + sxdata[sx_index(data3, 15)];
      if ((patch_number != new_patch) || (data_item == 3)) { // Right after a patch change the patch number is sent again. So here we catch that message.
        set_patch_number(new_patch);
        //page_check();
        set_patch_gap_timer();
        do_after_patch_selection();
        update_page = REFRESH_PAGE;
        return;
      }
    }

    // Check if it is the current parameter
    if (address == last_requested_sysex_address) {
      if (!checksum_ok) {
        PAGE_request_current_switch();
        return;
      }
      switch (last_requested_sysex_type) {
        case REQUEST_PATCH_NAME:
          for (uint8_t count = 0; count < 16; count++) {
            SP[last_requested_sysex_switch].Label[count] = static_cast<char>(sxdata[sx_index(data3, count + 12)]); //Add ascii character to the SP.Label String
          }

          if (SP[last_requested_sysex_switch].PP_number == patch_number) {
            current_patch_name = SP[last_requested_sysex_switch].Label; // Load patchname when it is read
            update_main_lcd = true; // And show it on the main LCD
          }
          DEBUGMSG(SP[last_requested_sysex_switch].Label);
          PAGE_request_next_switch();
          return;

        case REQUEST_PARAMETER_TYPE:
          read_parameter(last_requested_sysex_switch, sxdata[sx_index(data3, 12)], sxdata[sx_index(data3, 13)]);
          PAGE_request_next_switch();
          return;

        case REQUEST_ASSIGN_TYPE:
          read_current_assign(last_requested_sysex_switch, address, sxdata, sxlength);
          return;
      }
    }

    // Check for assign data
    if (read_full_assign_number < SY1000_NUMBER_OF_ASSIGNS) {
      if (address == calculate_full_assign_address(read_full_assign_number)) read_full_assign(read_full_assign_number, address, sxdata, sxlength);
    }

    // Check for scene parameter data
    if (read_scene_parameter_number > 0) {
      if ((address == read_scene_parameter_address) && (checksum_ok)) read_scene_message(read_scene_parameter_number, sxdata[sx_index(data3, 12)]);
    }

    // Check if it is the current patch name
    if (((address == SY1000_GM_CURRENT_PATCH_NAME) || (address == SY1000_BM_CURRENT_PATCH_NAME)) && (sxlength == 30) && (checksum_ok)) {
      current_patch_name = "";
      for (uint8_t count = 12; count < 28; count++) {
        current_patch_name += static_cast<char>(sxdata[sx_index(data3, count)]); //Add ascii character to Patch Name String
      }
      update_main_lcd = true;
      if (popup_patch_name) {
        LCD_show_popup_label(current_patch_name, ACTION_TIMER_LENGTH);
        popup_patch_name = false;
      }
      request_next_data_item();
    }

    // Check if it is the exp sw, exp and exp_on state
    if (((address == 0x1000002A) || (address == 0x1002002A)) && (last_requested_sysex_type == REQUEST_PEDAL_ASSIGN) && (checksum_ok)) {
      exp1_type = sxdata[sx_index(data3, 12)]; // EXP1 type
      exp2_type = sxdata[sx_index(data3, 13)]; // EXP2 type
      update_exp_label(last_requested_sysex_switch);
      PAGE_request_next_switch();
    }

    // Check for bass mode
    if ((address == SY1000_BASS_MODE) && (checksum_ok)) {
      bass_mode = (sxdata[sx_index(data3, 12)] == 0x01);
      if (bass_mode) {
        DEBUGMAIN("BASS MODE DETECTED");
      }
      request_next_data_item();
    }

    // Check for tempo
    if ((address == SY1000_GM_TEMPO) || (address == SY1000_BM_TEMPO)) {
      if ((checksum_ok) && (millis() > patch_gap_timer + 1500)) {
        uint16_t new_tempo = (sxdata[sx_index(data3, 12)] << 12) + (sxdata[sx_index(data3, 13)] << 8) + (sxdata[sx_index(data3, 14)] << 4) + sxdata[sx_index(data3, 15)];
        DEBUGMSG("Tempo received: " + String(new_tempo / 10));
        SCO_set_global_tempo_press(new_tempo / 10);
      }
    }

    // Check for Master key
    if ((address == SY1000_GM_MASTER_KEY) || (address == SY1000_BM_MASTER_KEY)) {
      if (checksum_ok) {
        master_key = sxdata[sx_index(data3, 12)];
        request_next_data_item();
      }
    }

    // Check for patch midi data
    if ((address == SY1000_GM_PATCH_MIDI) || (address == SY1000_BM_PATCH_MIDI)) {
      if (checksum_ok) {
        check_patch_midi((sxdata[sx_index(data3, 19)] << 4) + sxdata[sx_index(data3, 20)], sxdata[sx_index(data3, 21)]);
        check_patch_midi((sxdata[sx_index(data3, 22)] << 4) + sxdata[sx_index(data3, 23)], sxdata[sx_index(data3, 24)]);
        check_patch_midi((sxdata[sx_index(data3, 32)] << 4) + sxdata[sx_index(data3, 33)], sxdata[sx_index(data3, 34)]);
        check_patch_midi((sxdata[sx_index(data3, 35)] << 4) + sxdata[sx_index(data3, 36)], sxdata[sx_index(data3, 37)]);
        check_patch_midi((sxdata[sx_index(data3, 45)] << 4) + sxdata[sx_index(data3, 46)], sxdata[sx_index(data3, 47)]);
        check_patch_midi((sxdata[sx_index(data3, 48)] << 4) + sxdata[sx_index(data3, 49)], sxdata[sx_index(data3, 50)]);
        check_patch_midi((sxdata[sx_index(data3, 58)] << 4) + sxdata[sx_index(data3, 59)], sxdata[sx_index(data3, 60)]);
        check_patch_midi((sxdata[sx_index(data3, 61)] << 4) + sxdata[sx_index(data3, 62)], sxdata[sx_index(data3, 63)]);
        request_next_data_item();
      }
    }
    else { // Request patch midi data again when editing values
      if ((!bass_mode) && (address > SY1000_GM_PATCH_MIDI) && (address < SY1000_GM_PATCH_MIDI + 52)) request_sysex(SY1000_GM_PATCH_MIDI, 52);
      if ((bass_mode) && (address > SY1000_BM_PATCH_MIDI) && (address < SY1000_BM_PATCH_MIDI + 52)) request_sysex(SY1000_BM_PATCH_MIDI, 52);
    }

    // Check for switch mode
    check_switch_mode(address, sxdata, sxlength);

    // Check if it is the instrument on/off states
    if (checksum_ok) check_inst_switch_states(sxdata, sxlength);

    // Check if a patch is being saved on the SY1000
    if (address == SY1000_SAVE_PATCH) {
      bool saved = store_patch((sxdata[sx_index(data3, 12)] << 7) + sxdata[sx_index(data3, 13)]);
      if (saved) LCD_show_popup_label("Patch saved.", MESSAGE_TIMER_LENGTH);
    }

    // Check if two patches are exchanged on the SY1000
    if (address == SY1000_EXCHANGE_PATCH) {
      bool saved = exchange_patches((sxdata[sx_index(data3, 12)] << 7) + sxdata[sx_index(data3, 13)]);
      if (saved) LCD_show_popup_label("Patches swapped", MESSAGE_TIMER_LENGTH);
    }

    // Check if patch if inserted
    if (address == SY1000_INSERT_PATCH) {
      bool saved = insert_patch((sxdata[sx_index(data3, 12)] << 7) + sxdata[sx_index(data3, 13)]);
      if (saved) LCD_show_popup_label("Patch inserted", MESSAGE_TIMER_LENGTH);
    }

    // Check if patch if initialized
    if (address == SY1000_INITIALIZE_PATCH) {
      bool saved = initialize_patch((sxdata[sx_index(data3, 12)] << 7) + sxdata[sx_index(data3, 13)]);
      if (saved) LCD_show_popup_label("Patch initializd", MESSAGE_TIMER_LENGTH);
    }

    // Check if it is some other stompbox function and copy the status to the right LED
    if (!PAGE_update_running()) update_switches_on_page(address, sxdata[sx_index(data3, 12)]);

    // Check if edit mode is switched off
    if (address == 0x7F000001) {
      if ((sxdata[sx_index(data3, 12)] == 0x00) && (edit_mode)) write_sysex(SY1000_EDITOR_MODE_ON); // Turn it back on
    }

    // Midi forwarding to allow editing via VController
    if ((MIDI_in_port != USBMIDI_PORT) && (connected)) { // Forward data from Katana to editor
      MIDI_send_sysex(sxdata, sxlength, USBMIDI_PORT);
    }
  }
}

void MD_SY1000_class::forward_MIDI_message(const unsigned char* sxdata, short unsigned int sxlength) { // Forward data from editor to Katana
  uint8_t data3 = sxdata[3];
  if ((connected) && (sxdata[sx_index(data3, 1)] == 0x41) && (sxdata[sx_index(data3, 2)] == MIDI_device_id) && (sxdata[sx_index(data3, 3)] == 0x00) && (sxdata[sx_index(data3, 4)] == 0x00) && (sxdata[sx_index(data3, 5)] == 0x00) && (sxdata[sx_index(data3, 6)] == 0x69)) {
    editor_connected = true;
    check_sysex_delay();
    MIDI_send_sysex(sxdata, sxlength, MIDI_out_port);

    // Update switches on VController when state is changed from editor
    if (sxdata[sx_index(data3, 7)] == 0x12) {
      uint32_t address = (sxdata[sx_index(data3, 8)] << 24) + (sxdata[sx_index(data3, 9)] << 16) + (sxdata[sx_index(data3, 10)] << 8) + sxdata[sx_index(data3, 11)]; // Make the address 32 bit
      update_switches_on_page(address, sxdata[sx_index(data3, 12)]);
    }
  }
}

void MD_SY1000_class::check_PC_in(uint8_t program, uint8_t channel, uint8_t port) {  // Check incoming PC messages from  Called from MIDI:OnProgramChange

  // Check the source by checking the channel
  if ((port == MIDI_in_port) && (channel == MIDI_channel)) { // SY1000 sends a program change
    uint16_t new_patch = (CC00 * 100) + program;
    if (patch_number != new_patch) {
      set_patch_number(new_patch);
      do_after_patch_selection();
      check_ample_time_between_pc_messages_timer = millis() + CHECK_AMPLE_TIME_BETWEEN_PC_MESSAGES_TIME; // Fix for patch up/down on SY1000 sending double PC messages, triggering save patch on VController!
    }
    else if ((!edit_mode_always_on) && (!patch_gap_timer_running) && (millis() > check_ample_time_between_pc_messages_timer) && ((port & 0xF0) != USBHMIDI_PORT)) { // Alternative way to detect WRITE pressed on SY1000.
      bool saved = store_patch(new_patch);
      if (saved) LCD_show_popup_label("Patch saved.", MESSAGE_TIMER_LENGTH);
    }
  }
}

void MD_SY1000_class::check_CC_in(uint8_t control, uint8_t value, uint8_t channel, uint8_t port) {
  if (!connected) return;
  DEBUGMAIN("CC" + String(control));
  if ((port == MIDI_in_port) && (channel == MIDI_channel)) {
    if (control == 0) {
      CC00 = value;
      control_edit_mode();
    }
    if ((control >= SY1000_FIRST_SWITCH_CC) && (control <= SY1000_FIRST_SWITCH_CC + 7)) cc_operate_switch_mode(control - SY1000_FIRST_SWITCH_CC, value);
  }
}

void MD_SY1000_class::check_note_in(uint8_t note, uint8_t velocity, uint8_t channel, uint8_t port) {
  if (!connected) return;

  set_harmony_interval(note, velocity, channel, port);
  //control_slow_gear(note, velocity, channel, port);
}


// Detection of SY-1000

void MD_SY1000_class::identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t in_port, uint8_t out_port) {
  // Check if it is a SY-1000
  if ((sxdata[5] == 0x41) && (sxdata[6] == 0x69) && (sxdata[7] == 0x03) && (enabled == DEVICE_DETECT)) {
    no_response_counter = 0;
    if (connected == false) connect(sxdata[2], in_port, out_port); //Byte 2 contains the correct device ID

    // Midi forwarding to allow editing via VController
    for (uint8_t b = 0; b < SY1000_IDENTITY_MESSAGE_SIZE; b++) {
      identity_message[b] = sxdata[b];
    }
  }
}

FLASHMEM void MD_SY1000_class::respond_to_identity_request_of_editor() {
  if (identity_message[0] == 0xF0) MIDI_send_sysex(identity_message, SY1000_IDENTITY_MESSAGE_SIZE, USBMIDI_PORT);
}

FLASHMEM void MD_SY1000_class::do_after_connect() {
  if (edit_mode_always_on) { // Force edit_mode on
    edit_mode = false;
    set_editor_mode(true);
  }
  /*else { // Force edit_mode off
    edit_mode = true;
    set_editor_mode(false);
    }*/
  delay(10);
  request_first_data_item();
  current_exp_pedal = 1;
  if (!can_request_sysex_data()) do_after_patch_selection();
  //update_page = REFRESH_PAGE;
}

FLASHMEM void MD_SY1000_class::do_after_disconnect() {
  /*edit_mode = false;
    write_sysex(SY1000_EDITOR_MODE_OFF);*/
}

FLASHMEM void MD_SY1000_class::request_first_data_item() {
  if (!can_request_sysex_data()) return;
  data_item = 1;
  request_current_data_item();
}

FLASHMEM void MD_SY1000_class::request_next_data_item() {
  if (!can_request_sysex_data()) return;
  if (data_item == 0) return;
  data_item++;
  request_current_data_item();
}

FLASHMEM void MD_SY1000_class::request_current_data_item() {
  bool set_timer = false;
  DEBUGMSG("Requesting data item " + String(data_item));
  switch (data_item) {
    case 1:
      request_sysex(SY1000_BASS_MODE, 1);
      set_timer = true;
      break;
    case 2:
      request_switch_mode();
      set_timer = true;
      break;
    case 3:
      request_current_patch_number(); // Will trigger do_after_patch_change after receiving data
      set_timer = true;
      break;
    case 5: // After patch change
      request_guitar_switch_states();
      set_timer = true;
      break;
    case 6:
      request_current_patch_name();
      set_timer = true;
      break;
    case 7:
      if (!bass_mode) request_sysex(SY1000_GM_MASTER_KEY, 1);
      else request_sysex(SY1000_BM_MASTER_KEY, 1);
      set_timer = true;
      break;
    case 8:
      if (!bass_mode) request_sysex(SY1000_GM_PATCH_MIDI, 52);
      else request_sysex(SY1000_BM_PATCH_MIDI, 52);
      set_timer = true;
      break;
    case 9:
      if (Setting.Send_global_tempo_after_patch_change == true) set_bpm(); // Set tempo again, so the SY1000 will not lose it when changing patches quickly
      request_full_assign(0);
      data_item = 0; // Done requesting items after patch change
      break;
  }
  if (set_timer) midi_timer = millis() + SY1000_MIDI_TIME;
  else midi_timer = 0;
}

FLASHMEM void MD_SY1000_class::check_midi_timer() {
  if (midi_timer == 0) return;
  if (millis() > midi_timer) {
    request_current_data_item();
    midi_timer = millis() + SY1000_MIDI_TIME;
  }
}


// ********************************* Section 3: SY1000 common MIDI out functions ********************************************

FLASHMEM void MD_SY1000_class::write_sysex(uint32_t address, uint8_t value) { // For sending one data byte

  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t checksum = calc_Roland_checksum(ad[3] + ad[2] + ad[1] + ad[0] + value); // Calculate the Roland checksum
  uint8_t sysexmessage[15] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x00, 0x69, 0x12, ad[3], ad[2], ad[1], ad[0], value, checksum, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 15, MIDI_out_port); // SY-1000 connected via USBHost_t36 library will only supoort sysex messages via cable 1 (default 0)
  if (editor_connected) MIDI_send_sysex(sysexmessage, 15, USBMIDI_PORT); // Forward message to BTS
}

FLASHMEM void MD_SY1000_class::write_sysex(uint32_t address, uint8_t value1, uint8_t value2) { // For sending two data bytes

  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t checksum = calc_Roland_checksum(ad[3] + ad[2] + ad[1] + ad[0] + value1 + value2); // Calculate the Roland checksum
  uint8_t sysexmessage[16] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x00, 0x69, 0x12, ad[3], ad[2], ad[1], ad[0], value1, value2, checksum, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 16, MIDI_out_port);
  if (editor_connected) MIDI_send_sysex(sysexmessage, 16, USBMIDI_PORT); // Forward message to BTS
}

FLASHMEM void MD_SY1000_class::write_sysex(uint32_t address, uint8_t value1, uint8_t value2, uint8_t value3, uint8_t value4) { // For sending four data bytes

  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t checksum = calc_Roland_checksum(ad[3] + ad[2] + ad[1] + ad[0] + value1 + value2 + value3 + value4); // Calculate the Roland checksum
  uint8_t sysexmessage[18] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x00, 0x69, 0x12, ad[3], ad[2], ad[1], ad[0], value1, value2, value3, value4, checksum, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 18, MIDI_out_port);
  if (editor_connected) MIDI_send_sysex(sysexmessage, 18, USBMIDI_PORT); // Forward message to BTS
}

FLASHMEM void MD_SY1000_class::write_sysex(uint32_t address, uint8_t value1, uint8_t value2, uint8_t value3, uint8_t value4, uint8_t value5, uint8_t value6, uint8_t value7, uint8_t value8) { // For sending eight data bytes

  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t checksum = calc_Roland_checksum(ad[3] + ad[2] + ad[1] + ad[0] + value1 + value2 + value3 + value4 + value5 + value6 + value7 + value8); // Calculate the Roland checksum
  uint8_t sysexmessage[22] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x00, 0x69, 0x12, ad[3], ad[2], ad[1], ad[0], value1, value2, value3, value4, value5, value6, value7, value8, checksum, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 22, MIDI_out_port);
  if (editor_connected) MIDI_send_sysex(sysexmessage, 22, USBMIDI_PORT); // Forward message to BTS
}

FLASHMEM void MD_SY1000_class::request_sysex(uint32_t address, uint8_t no_of_bytes) {
  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t no1 = no_of_bytes >> 7;
  uint8_t no2 = no_of_bytes & 0x7F;
  uint8_t checksum = calc_Roland_checksum(ad[3] + ad[2] + ad[1] + ad[0] +  no1 + no2); // Calculate the Roland checksum
  uint8_t sysexmessage[18] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x00, 0x69, 0x11, ad[3], ad[2], ad[1], ad[0], 0x00, 0x00, no1, no2, checksum, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 18, MIDI_out_port);
}

FLASHMEM void MD_SY1000_class::control_edit_mode() {
  set_editor_mode(true);
  //if (!edit_mode_always_on) {
  if ((!edit_mode_always_on) && ((MIDI_in_port & 0xF0) != USBHMIDI_PORT)) {
    edit_mode_return_timer = SY1000_EDIT_MODE_RETURN_TIME + millis();
    edit_return_timer_running = true;
  }
}

void MD_SY1000_class::check_edit_mode_return_timer() {
  if (!edit_return_timer_running) return;
  if (millis() > edit_mode_return_timer) {
    set_editor_mode(false);
    edit_return_timer_running = false;
  }
}

FLASHMEM void MD_SY1000_class::set_editor_mode(bool state) {
  if (state == edit_mode) return;
  if (((MIDI_in_port & 0xF0) == USBHMIDI_PORT) && (state == false)) return; // Do not switch off editor mode when connected to USB_MIDI
  if (state) write_sysex(SY1000_EDITOR_MODE_ON);
  else write_sysex(SY1000_EDITOR_MODE_OFF);
  edit_mode = state;
}

FLASHMEM void MD_SY1000_class::set_bpm() {
  if (connected) {
    uint32_t address;
    uint16_t t = Setting.Bpm * 10;
    control_edit_mode();
    if (!bass_mode) address = SY1000_GM_TEMPO;
    else address = SY1000_BM_TEMPO;
    write_sysex(address, (t & 0xF000) >> 12, (t & 0x0F00) >> 8, (t & 0x00F0) >> 4, (t & 0x000F)); // Tempo is modulus 16
  }
}

FLASHMEM void MD_SY1000_class::start_tuner() {
  if (connected) {
    control_edit_mode();
    write_sysex(SY1000_TUNER_ON); // Start tuner on SY-1000
    tuner_active = true;
  }
}

FLASHMEM void MD_SY1000_class::stop_tuner() {
  if (connected) {
    control_edit_mode();
    write_sysex(SY1000_TUNER_OFF); // Stop tuner on SY-1000
    tuner_active = false;
  }
}

void MD_SY1000_class::check_send_tempo_timer() {
  if (send_tempo_timer == 0) return;
  if (millis() > send_tempo_timer) {
    if (!bass_mode) request_sysex(SY1000_GM_TEMPO, 4);
    else request_sysex(SY1000_BM_TEMPO, 4);
    send_tempo_timer = 0;
  }
}

// ********************************* Section 4: SY1000 program change ********************************************

FLASHMEM void MD_SY1000_class::select_patch(uint16_t new_patch) {
  //if (new_patch == patch_number) unmute();
  set_patch_number(new_patch);

  if ((MIDI_out_port & 0xF0) != USBHMIDI_PORT) {
    MIDI_send_CC(0, new_patch / 100, MIDI_channel, MIDI_out_port);
    MIDI_send_PC(new_patch % 100, MIDI_channel, MIDI_out_port);
    DEBUGMSG("out(SY1000) CC/PC" + String(new_patch)); //Debug
  }
  else { // Via host port normal CC/PC no longer works.
    control_edit_mode();
    write_sysex(SY1000_PATCH_SELECT, (new_patch & 0x7F80) >> 7, new_patch & 0x7F);
    DEBUGMSG("out(SY1000) PC through sysex" + String(new_patch)); //Debug
  }
  set_patch_gap_timer(); // Delay the running of MD_SY1000_class::do_after_patch_selection()
}

FLASHMEM void MD_SY1000_class::set_patch_gap_timer() {
  patch_gap_timer = millis() + PATCH_GAP_TIME;
  patch_gap_timer_running = true;
}

void MD_SY1000_class::check_patch_gap_timer() {
  if (!patch_gap_timer_running) return;
  if (millis() > patch_gap_timer) {
    patch_gap_timer_running = false;
    do_after_patch_selection();
  }
}

FLASHMEM void MD_SY1000_class::do_after_patch_selection() {
  edit_mode = false; // So edit mode will be switched on again when data is sent
  is_on = connected;
  Current_patch_number = patch_number;
  update_LEDS = true;
  update_main_lcd = true;
  reset_special_functions();

  if (Setting.Send_global_tempo_after_patch_change == true) {
    delay(5);
    set_bpm();
  }
  load_patch(patch_number);
  //auto_return_switch_mode(); // Will update the LEDs on the SY1000 as well
  update_leds_on_SY1000();
  data_item = 4;
  request_next_data_item();
  MD_base_class::do_after_patch_selection();
  update_page = RELOAD_PAGE; // So Scene names are also updated when they are on the page.
}

FLASHMEM bool MD_SY1000_class::request_patch_name(uint8_t sw, uint16_t number) {
  if (number > patch_max) return true;
  uint32_t Address;
  Address = 0x52000000 + (((number * 0x10) / 0x80) * 0x100) + ((number * 0x10) % 0x80); //Calculate the address where the patchname is stored on the SY-1000
  //Address = 0x20000000 + ((number / 0x40) * 0x1000000) + ((number % 0x40) * 0x20000);
  if (bass_mode) Address += 0x01000000; // Start address is 0x53000000 in bass mode
  last_requested_sysex_address = Address;
  last_requested_sysex_type = REQUEST_PATCH_NAME;
  last_requested_sysex_switch = sw;
  request_sysex(Address, 16); //Request the 16 bytes of the SY1000 patchname
  return false;
}

FLASHMEM void MD_SY1000_class::request_current_patch_name() {
  if (!bass_mode) request_sysex(SY1000_GM_CURRENT_PATCH_NAME, 16);
  else request_sysex(SY1000_BM_CURRENT_PATCH_NAME, 16);
}

FLASHMEM void MD_SY1000_class::request_current_patch_number() {
  if (!bass_mode) request_sysex(SY1000_GM_CURRENT_PATCH_NUMBER, 4);
  else request_sysex(SY1000_BM_CURRENT_PATCH_NUMBER, 4);
}

FLASHMEM bool MD_SY1000_class::flash_LEDs_for_patch_bank_switch(uint8_t sw) { // Will flash the LEDs in banks of three when coming from direct select mode.
  if (!bank_selection_active()) return false;

  if (flash_bank_of_four == 255) return true; // We are not coming from direct select, so all LEDs should flash

  bool in_right_bank_of_eight = (flash_bank_of_four / 2 == SP[sw].PP_number / 8); // Going bank up and down coming from direct select will make all LEDs flash in other banks
  if (!in_right_bank_of_eight) return true;

  bool in_right_bank_of_four = (flash_bank_of_four == SP[sw].PP_number / 4); // Only flash the three LEDs of the corresponding bank
  if (in_right_bank_of_four) return true;
  return false;
}

FLASHMEM void MD_SY1000_class::number_format(uint16_t number, String &Output) {
  if (number < 200) Output += 'U';
  else Output += 'P';
  uint8_t bank_no = ((number % 200) / 4) + 1;
  Output += String(bank_no / 10) + String(bank_no % 10);
  Output += '-';
  Output += String((number % 4) + 1);
}

FLASHMEM void MD_SY1000_class::direct_select_format(uint16_t number, String &Output) {

  if (direct_select_state == 0) {
    if (number <= 5) Output += 'U' + String(number) + "_-_";
    else Output += 'P' + String(number - 5) + "_-_";
  }
  else {
    uint8_t b = bank_select_number;
    if ((b * 10) + number <= 50) {
      Output += 'U';
    }
    else {
      Output += 'P';
      b -= 5;
    }
    Output += String(b) + String(number) + "-_";
  }
}

FLASHMEM bool MD_SY1000_class::valid_direct_select_switch(uint8_t number) {
  bool result = false;
  if (direct_select_state == 0) {
    result = ((number * 40) <= (patch_max - patch_min + 1));
  }
  else {
    if ((bank_select_number == 0) && (number == 0)) return false;
    if ((bank_select_number * 40) + (number * 4) <= (patch_max - patch_min + 1)) result = true;
  }
  return result;
}

FLASHMEM void MD_SY1000_class::direct_select_start() {
  Previous_bank_size = bank_size; // Remember the bank size
  device_in_bank_selection = my_device_number + 1;
  bank_size = 400;
  bank_select_number = 0; // Reset bank number
  direct_select_state = 0;
}

FLASHMEM uint16_t MD_SY1000_class::direct_select_patch_number_to_request(uint8_t number) {
  uint16_t new_patch_number;
  if (direct_select_state == 0) new_patch_number = (number * 40);
  else new_patch_number = (bank_select_number * 40) + (number * 4);
  if (new_patch_number < 4) new_patch_number = 4;
  return new_patch_number - 4;
}

FLASHMEM void MD_SY1000_class::direct_select_press(uint8_t number) {
  if (!valid_direct_select_switch(number)) return;
  if (direct_select_state == 0) {
    // First digit pressed
    bank_select_number = number;
    bank_size = 40;
    direct_select_state = 1;
  }
  else  {
    // Second digit pressed
    //if (number > 0) number--;
    uint16_t base_patch = (bank_select_number * 40) + ((number - 1) * 4);
    flash_bank_of_four = base_patch / 4;
    bank_size = Previous_bank_size;
    bank_select_number = (base_patch / bank_size);
    Current_page = Previous_page; // SCO_select_page will overwrite Previous_page with Current_page, now it will know the way back
    SCO_select_page(my_device_page1); // Which should give PAGE_SY1000_PATCH_BANK
    device_in_bank_selection = my_device_number + 1; // Go into bank mode
  }
}

// ** US-20 simulation
// Selecting and muting the SY1000 is done by storing the settings of COSM guitar switch and Normal PU switch
// and switching both off when guitar is muted and back to original state when the SY1000 is selected

FLASHMEM void MD_SY1000_class::request_guitar_switch_states() {
  if (!bass_mode) {
    request_sysex(SY1000_GM_INST1_SW, 2);
    request_sysex(SY1000_GM_INST2_SW, 2);
    request_sysex(SY1000_GM_INST3_SW, 2);
    request_sysex(SY1000_GM_NORMAL_PU_SW, 1);
  }
  else {
    request_sysex(SY1000_BM_INST1_SW, 2);
    request_sysex(SY1000_BM_INST2_SW, 2);
    request_sysex(SY1000_BM_INST3_SW, 2);
    request_sysex(SY1000_BM_NORMAL_PU_SW, 1);
  }
  request_onoff = true;
}

FLASHMEM void MD_SY1000_class::check_inst_switch_states(const unsigned char* sxdata, short unsigned int sxlength) {
  if (request_onoff == true) {
    uint8_t data3 = sxdata[3];
    uint32_t address = (sxdata[sx_index(data3, 8)] << 24) + (sxdata[sx_index(data3, 9)] << 16) + (sxdata[sx_index(data3, 10)] << 8) + sxdata[sx_index(data3, 11)]; // Make the address 32 bit
    if ((address == SY1000_GM_INST1_SW) || (address == SY1000_BM_INST1_SW)) {
      INST_onoff[0] = sxdata[sx_index(data3, 12)];
      INST_type[0] = sxdata[sx_index(data3, 13)];
      DEBUGMSG("INST_type[0]:" + String(INST_type[0]));
    }
    if ((address == SY1000_GM_INST2_SW) || (address == SY1000_BM_INST2_SW)) {
      INST_onoff[1] = sxdata[sx_index(data3, 12)];
      INST_type[1] = sxdata[sx_index(data3, 13)];
      DEBUGMSG("INST_type[1]:" + String(INST_type[1]));
    }
    if ((address == SY1000_GM_INST3_SW) || (address == SY1000_BM_INST3_SW)) {
      INST_onoff[2] = sxdata[sx_index(data3, 12)];
      INST_type[2] = sxdata[sx_index(data3, 13)];
      DEBUGMSG("INST_type[2]:" + String(INST_type[2]));
    }
    if ((address == SY1000_GM_NORMAL_PU_SW) || (address == SY1000_BM_NORMAL_PU_SW)) {
      normal_pu_mute = sxdata[sx_index(data3, 12)];  // Store the value
      request_onoff = false;
      request_next_data_item();
    }
  }
}

FLASHMEM void MD_SY1000_class::unmute() {
  is_on = connected;
  control_edit_mode();
  if (!bass_mode) {
    write_sysex(SY1000_GM_INST1_SW, INST_onoff[0]); // Switch INST1 guitar on
    write_sysex(SY1000_GM_INST2_SW, INST_onoff[1]); // Switch INST2 guitar on
    write_sysex(SY1000_GM_INST3_SW, INST_onoff[2]); // Switch INST3 guitar on
    write_sysex(SY1000_GM_NORMAL_PU_SW, normal_pu_mute); // Switch normal pu on
  }
  else {
    write_sysex(SY1000_BM_INST1_SW, INST_onoff[0]); // Switch INST1 guitar on
    write_sysex(SY1000_BM_INST2_SW, INST_onoff[1]); // Switch INST2 guitar on
    write_sysex(SY1000_BM_INST3_SW, INST_onoff[2]); // Switch INST3 guitar on
    write_sysex(SY1000_BM_NORMAL_PU_SW, normal_pu_mute); // Switch normal pu on
  }
}

FLASHMEM void MD_SY1000_class::mute() {
  if ((US20_mode_enabled()) && (!is_always_on) && (is_on)) {
    is_on = false;
    control_edit_mode();
    request_guitar_switch_states();
    delay(20);

    mute_now();
  }
}

FLASHMEM void MD_SY1000_class::mute_now() {
  if (!bass_mode) {
    write_sysex(SY1000_GM_INST1_SW, 0x00); // Switch INST1 guitar off
    write_sysex(SY1000_GM_INST2_SW, 0x00); // Switch INST2 guitar off
    write_sysex(SY1000_GM_INST3_SW, 0x00); // Switch INST3 guitar off
    write_sysex(SY1000_GM_NORMAL_PU_SW, 0x00); // Switch normal pu off
  }
  else {
    write_sysex(SY1000_BM_INST1_SW, 0x00); // Switch INST1 guitar off
    write_sysex(SY1000_BM_INST2_SW, 0x00); // Switch INST2 guitar off
    write_sysex(SY1000_BM_INST3_SW, 0x00); // Switch INST3 guitar off
    write_sysex(SY1000_BM_NORMAL_PU_SW, 0x00); // Switch normal pu off
  }
}

// ********************************* Section 5: SY1000 parameter control ********************************************

// Most parameters are controlled by writing the correct value via sysex.
// Some parameters have the VIA_ASSIGN address, becasue they cannot be controlled directly. This temporary changes assign 16 to control the parameter via CC #95.
// Whenever the switch is released the original setting of the assign is restored.

// Procedures for the SY1000_PARAMETER:
// 1. Load in SP array L load_page()
// 2. Request parameter state - in PAGE_request_current_switch()
// 3. Read parameter state - SY1000_read_parameter() below
// 4. Press switch - SY1000_parameter_press() below - also calls check_update_label()
// 5. Release switch - SY1000_parameter_release() below - also calls check_update_label()

struct SY1000_parameter_struct { // Combines all the data we need for controlling a parameter in a device
  uint8_t Ctl_target; // The number of this effect in the target list of the CTLx
  uint16_t Target_GM; // Target of the assign as given in the assignments of the SY1000
  uint16_t Target_BM; // Target of the assign as given in the assignments of the SY1000
  uint16_t Address_GM; // The address of the parameter in guitar mode
  uint16_t Address_BM; // The address of the parameter in bass mode
  uint8_t NumVals; // The number of values for this parameter
  char Name[11]; // The name for the label
  uint16_t Sublist; // Which sublist to read for the FX or amp type - 0 if second byte does not contain the type or if there is no sublist +100 Show value from sublist.
  uint8_t Colour; // The colour for this effect.
  uint8_t Type; // Types - used for addressess that depend on the INST_type
};


#define SY1000_FX_COLOUR 255 // Just a colour number to pick the colour from the SY1000_FX_colours table
#define SY1000_FX_TYPE_COLOUR 254 //Another number for the FX type
#define SUBLIST_FROM_BYTE2 32768 //Bit 16 set high means read sublist from byte 2
#define SHOW_NUMBER 31767 // Special number for showing the number (used for gain/volume/etc) - set in sublist
#define SHOW_DOUBLE_NUMBER 31766 // Show number times 2 (used for patch volume/etc) - set in sublist
#define SHOW_PAN 31765 // Special number for showing the pan- set in sublist
#define SY1000_INST_SUBLIST 31750
#define PREV2 0x8000 // To execute the previous 2 items
#define PREV3 0x8001 // To execute the previous 3 items
#define PREV9 0x8002 // To execute the previous 9 items - existing PREV2 items are skipped

// Types - used for addressess that depend on the INST_type, like LFO,
#define INST1_LFO 0x11
#define INST2_LFO 0x12
#define INST3_LFO 0x13
#define INST1_AMP 0x21
#define INST2_AMP 0x22
#define INST3_AMP 0x23
#define DO_TAP_TEMPO 0xFF

#define VIA_ASSIGN 80 // Special address for controlling parameter via an assign
#define ADDR_MANUAL 81
#define SET_SW_MODE 82

const PROGMEM SY1000_parameter_struct SY1000_parameters[] = {
  {   0, 0xFFFF, 0xFFFF, SET_SW_MODE, SET_SW_MODE, 6, "SW MODE", 353, FX_DEFAULT_TYPE, 0 },
  {  20,   11,   11, 0x1500, 0x1500, 2, "INST1", SY1000_INST_SUBLIST | SUBLIST_FROM_BYTE2, FX_GTR_TYPE, 0 },
  {  21,  339,  339, 0x2000, 0x1F00, 2, "INST2", SY1000_INST_SUBLIST | SUBLIST_FROM_BYTE2, FX_GTR_TYPE, 0 },
  {  22,  669,  669, 0x2B00, 0x2900, 2, "INST3", SY1000_INST_SUBLIST | SUBLIST_FROM_BYTE2, FX_GTR_TYPE, 0 },
  {  23, PREV3, PREV3, 0x1500, 0x1500, 2, "INST ALL", 0, FX_GTR_TYPE, 0 },
  { 100, 1967, 1910, 0x3600, 0x3300, 2, "COMP", 16 | SUBLIST_FROM_BYTE2, FX_DYNAMICS_TYPE, 0 },
  { 101, 1848, 1791, 0x3700, 0x3400, 2, "DIST", 23 | SUBLIST_FROM_BYTE2, FX_DIST_TYPE, 0 },
  { 102, 1855, 1798, 0x3707, 0x3407, 2, "DS SOLO", 0, FX_DIST_TYPE, 0 },
  { 103, 1828, 1771, 0x3800, 0x3500, 2, "AMP", 58 | SUBLIST_FROM_BYTE2, FX_AMP_TYPE, 0 },
  { 104, 1840, 1783, 0x380C, 0x350C, 2, "AMP SOLO", 0, FX_AMP_TYPE, 0 },
  { 108, 1905, 1848, 0x3C00, 0x3900, 2, "DELAY 1", 0, FX_DELAY_TYPE, 0 },
  { 109, 1912, 1855, 0x3D00, 0x3A00, 2, "DELAY 2", 0, FX_DELAY_TYPE, 0 },
  { 110, 1881, 1824, 0x3E00, 0x3B00, 2, "MST DLY", 89 | SUBLIST_FROM_BYTE2, FX_DELAY_TYPE, 0 },
  { 117, 1889, 1832, 0x3E1A, 0x3B1A, 2, "MST DLY TR", 0, FX_DELAY_TYPE, 0 },
  { 111, 1942, 1885, 0x3F00, 0x3C00, 2, "CHORUS", 101 | SUBLIST_FROM_BYTE2, FX_MODULATE_TYPE, 0 },
  {  97, 1018,  961, 0x4000, 0x3D00, 2, "FX1", 105 | SUBLIST_FROM_BYTE2, SY1000_FX_COLOUR, 0 },
  {   0, 1019,  962, 0x4001, 0x3D01, 37, "FX1 TYPE", 105, SY1000_FX_TYPE_COLOUR, 0 },
  {  98, 1288, 1231, 0x6600, 0x6300, 2, "FX2", 105 | SUBLIST_FROM_BYTE2, SY1000_FX_COLOUR, 0 },
  {   0, 1289, 1232, 0x6601, 0x6301, 37, "FX2 TYPE", 105, SY1000_FX_TYPE_COLOUR, 0 },
  {  99, 1558, 1501, 0x8C00, 0x8900, 2, "FX3", 105 | SUBLIST_FROM_BYTE2, SY1000_FX_COLOUR, 0 },
  {   0, 1559, 1502, 0x8C01, 0x8901, 37, "FX3 TYPE", 105, SY1000_FX_TYPE_COLOUR, 0 },
  { 112, 1919, 1862, 0xB200, 0xAF00, 2, "RVB", 142 | SUBLIST_FROM_BYTE2, FX_REVERB_TYPE, 0 },
  { 118, 1981, 1923, 0x1221, 0x1221, 2, "DIV CH SEL", 0, FX_DEFAULT_TYPE, 0 },
  { 107, 1930, 1873, 0x3900, 0x3600, 2, "NS", 0, FX_DYNAMICS_TYPE, 0 },
  { 105, 1857, 1800, 0x3A00, 0x3700, 2, "EQ1", 0, FX_FILTER_TYPE, 0 },
  { 106, 1869, 1812, 0x3B00, 0x3800, 2, "EQ2", 0, FX_FILTER_TYPE, 0 },
  { 113, 1978, 1917, 0x122E, 0x122E, 2, "S/R LOOP", 0, FX_DEFAULT_TYPE, 0 },
  {  27, 1014,  657, 0x1201, 0x1201, 2, "NORMAL PU", 0, FX_GTR_TYPE, 0 },
  {  91,  304,  285, 0x1A0A, 0x1A16, 2, "1:AMP", 58 | SUBLIST_FROM_BYTE2, FX_AMP_TYPE, INST1_AMP },
  {  92,  634,  596, 0x250A, 0x2416, 2, "2:AMP", 58 | SUBLIST_FROM_BYTE2, FX_AMP_TYPE, INST2_AMP },
  {  93,  964,  907, 0x300A, 0x2E16, 2, "3:AMP", 58 | SUBLIST_FROM_BYTE2, FX_AMP_TYPE, INST3_AMP },
  {  94,  316,  297, 0x1A16, 0x1A22, 2, "1:AMP SOLO", 0, FX_AMP_TYPE, INST1_AMP },
  {  95,  646,  608, 0x2516, 0x2422, 2, "2:AMP SOLO", 0, FX_AMP_TYPE, INST2_AMP },
  {  96,  976,  919, 0x3016, 0x2E22, 2, "3:AMP SOLO", 0, FX_AMP_TYPE, INST3_AMP },
  {  28,   28,   28, 0x1F00, 0x1E00, 2, "1:ALT TN", 149 | SUBLIST_FROM_BYTE2, FX_GTR_TYPE, 0 },
  {  29,  356,  356, 0x2A00, 0x2800, 2, "2:ALT TN", 149 | SUBLIST_FROM_BYTE2, FX_GTR_TYPE, 0 },
  {  30,  686,  686, 0x3500, 0x3200, 2, "3:ALT TN", 149 | SUBLIST_FROM_BYTE2, FX_GTR_TYPE, 0 },
  {  31, PREV3, PREV3, 0x1F00, 0x1E00, 2, "A:ALT TN", 149 | SUBLIST_FROM_BYTE2, FX_GTR_TYPE, 0 },
  {  32,   42,   42, 0x1F0E, 0x1E0E, 2, "1:12-STR", 0, FX_GTR_TYPE, 0 },
  {  33,  370,  370, 0x2A0E, 0x280E, 2, "2:12-STR", 0, FX_GTR_TYPE, 0 },
  {  34,  700,  700, 0x350E, 0x320E, 2, "3:12-STR", 0, FX_GTR_TYPE, 0 },
  {  35, PREV3, PREV3, 0x1F0E, 0x1E0E, 2, "ALL:12STR", 0, FX_GTR_TYPE, 0 },
  {  36,   68,   68, 0x1F28, 0x1E28, 2, "1:BEND", 0, FX_GTR_TYPE, 0 },
  {  37,  396,  396, 0x2A28, 0x2828, 2, "2:BEND", 0, FX_GTR_TYPE, 0 },
  {  38,  726,  726, 0x3528, 0x3228, 2, "3:BEND", 0, FX_GTR_TYPE, 0 },
  {  39, PREV3, PREV3, 0x1F28, 0x1E28, 2, "ALL:BEND", 0, FX_GTR_TYPE, 0 },
  {  80,  999,  942, VIA_ASSIGN, VIA_ASSIGN, 2, "1:HOLD", 0, FX_GTR_TYPE, 0 },
  {  81, 1000,  943, VIA_ASSIGN, VIA_ASSIGN, 2, "2:HOLD", 0, FX_GTR_TYPE, 0 },
  {  82, 1001,  944, VIA_ASSIGN, VIA_ASSIGN, 2, "3:HOLD", 0, FX_GTR_TYPE, 0 },
  {  83, 1002,  945, VIA_ASSIGN, VIA_ASSIGN, 2, "ALL:HOLD", 0, FX_GTR_TYPE, 0 },
  {  40,  102,  102, 0x1627, 0x1627, 2, "1:LFO1", 0, FX_GTR_TYPE, INST1_LFO },
  {  41,  112,  113, 0x1631, 0x1631, 2, "1:LFO2", 0, FX_GTR_TYPE, INST1_LFO },
  {  42, PREV2, PREV2, 0x1627, 0x1627, 2, "1:LFO1+2", 0, FX_GTR_TYPE, INST1_LFO },
  {  43,  432,  413, 0x2127, 0x2027, 2, "2:LFO1", 0, FX_GTR_TYPE, INST2_LFO },
  {  44,  442,  423, 0x2131, 0x2031, 2, "2:LFO2", 0, FX_GTR_TYPE, INST2_LFO },
  {  45, PREV2, PREV2, 0x2127, 0x2027, 2, "2:LFO1+2", 0, FX_GTR_TYPE, INST2_LFO },
  {  46,  762,  724, 0x2C27, 0x2A27, 2, "3:LFO1", 0, FX_GTR_TYPE, INST3_LFO },
  {  47,  772,  736, 0x2A31, 0x2A31, 2, "3:LFO2", 0, FX_GTR_TYPE, INST3_LFO },
  {  48, PREV2, PREV2, 0x2C27, 0x2A27, 2, "3:LFO1+2", 0, FX_GTR_TYPE, INST3_LFO },
  {  49, PREV9, PREV9, 0x1627, 0x1627, 2, "ALL:LFO", 0, FX_GTR_TYPE, INST1_LFO },
  {  50,  125,  125, 0x171E, 0x171E, 2, "1:SEQ1", 0, FX_GTR_TYPE, 0 },
  {  51,  131,  131, 0x1734, 0x1734, 2, "1:SEQ2", 0, FX_GTR_TYPE, 0 },
  {  52, PREV2, PREV2, 0x171E, 0x171E, 2, "1:SEQ1+2", 0, FX_GTR_TYPE, 0 },
  {  53,  455,  436, 0x221E, 0x211E, 2, "2:SEQ1", 0, FX_GTR_TYPE, 0 },
  {  54,  461,  442, 0x2234, 0x2134, 2, "2:SEQ2", 0, FX_GTR_TYPE, 0 },
  {  55, PREV2, PREV2, 0x221E, 0x211E, 2, "2:SEQ1+2", 0, FX_GTR_TYPE, 0 },
  {  56,  785,  747, 0x2d1E, 0x2B1E, 2, "3:SEQ1", 0, FX_GTR_TYPE, 0 },
  {  57,  791,  753, 0x2D34, 0x2B34, 2, "3:SEQ2", 0, FX_GTR_TYPE, 0 },
  {  58, PREV2, PREV2, 0x2D1E, 0x2B1E, 2, "3:SEQ1+2", 0, FX_GTR_TYPE, 0 },
  {  59, PREV9, PREV9, 0x171E, 0x171E, 2, "ALL:SEQ", 0, FX_GTR_TYPE, 0 },
  {  70,  130,  130, 0x1723, 0x1723, 2, "1:TURBO1", 0, FX_GTR_TYPE, 0 },
  {  71,  136,  136, 0x1739, 0x1739, 2, "1:TURBO2", 0, FX_GTR_TYPE, 0 },
  {  72, PREV2, PREV2, 0x1723, 0x1723, 2, "1:TURBO1+2", 0, FX_GTR_TYPE, 0 },
  {  73,  460,  441, 0x2223, 0x2123, 2, "2:TURBO1", 0, FX_GTR_TYPE, 0 },
  {  74,  466,  447, 0x2239, 0x2139, 2, "2:TURBO2", 0, FX_GTR_TYPE, 0 },
  {  75, PREV2, PREV2, 0x2223, 0x2123, 2, "2:TURBO1+2", 0, FX_GTR_TYPE, 0 },
  {  76,  790,  752, 0x2D23, 0x2B23, 2, "3:TURBO1", 0, FX_GTR_TYPE, 0 },
  {  77,  796,  758, 0x2D39, 0x2B39, 2, "3:TURBO2", 0, FX_GTR_TYPE, 0 },
  {  78, PREV2, PREV2, 0x2D23, 0x2B23, 2, "3:TURBO1+2", 0, FX_GTR_TYPE, 0 },
  {  79, PREV9, PREV9, 0x1723, 0x1723, 2, "ALL:TURBO", 0, FX_GTR_TYPE, 0 },
  {  60, 1003,  946, VIA_ASSIGN, VIA_ASSIGN, 2, "1:SEQ1 TRG", 0, FX_GTR_TYPE, 0 },
  {  61, 1004,  947, VIA_ASSIGN, VIA_ASSIGN, 2, "1:SEQ2 TRG", 0, FX_GTR_TYPE, 0 },
  {  62, 1005,  948, VIA_ASSIGN, VIA_ASSIGN, 2, "1:SEQ1&2TR", 0, FX_GTR_TYPE, 0 },
  {  63, 1006,  949, VIA_ASSIGN, VIA_ASSIGN, 2, "2:SEQ1 TRG", 0, FX_GTR_TYPE, 0 },
  {  64, 1007,  950, VIA_ASSIGN, VIA_ASSIGN, 2, "2:SEQ2 TRG", 0, FX_GTR_TYPE, 0 },
  {  65, 1008,  951, VIA_ASSIGN, VIA_ASSIGN, 2, "2:SEQ1&2 R", 0, FX_GTR_TYPE, 0 },
  {  66, 1009,  952, VIA_ASSIGN, VIA_ASSIGN, 2, "3:SEQ1 TRG", 0, FX_GTR_TYPE, 0 },
  {  67, 1010,  953, VIA_ASSIGN, VIA_ASSIGN, 2, "3:SEQ2 TRG", 0, FX_GTR_TYPE, 0 },
  {  68, 1011,  954, VIA_ASSIGN, VIA_ASSIGN, 2, "3:SEQ1&2TR", 0, FX_GTR_TYPE, 0 },
  {  69, 1012,  955, VIA_ASSIGN, VIA_ASSIGN, 2, "ALL:SEQ TR", 0, FX_GTR_TYPE, 0 },
  {  24,   14,   14, 0x1503, 0x1503, 2, "1:NOR MIX", 0, FX_GTR_TYPE, 0 },
  {  25,  342,  323, 0x2003, 0x1F03, 2, "2:NOR MIX", 0, FX_GTR_TYPE, 0 },
  {  26,  672,  634, 0x2B03, 0x2903, 2, "3:NOR MIX", 0, FX_GTR_TYPE, 0 },
  {  17, 0xFFFF, 0xFFFF, ADDR_MANUAL, ADDR_MANUAL, 2, "MANUAL", 0, FX_GTR_TYPE, 0 },
  {   9,    4,    4, VIA_ASSIGN, VIA_ASSIGN, 2, "BPM TAP", 0, FX_SHOW_TAP_TEMPO, DO_TAP_TEMPO },
  {   0, 0xFFFF, 0xFFFF, 0x001C, 0x001C, 121, "CTL1 TYPE", 231, FX_DEFAULT_TYPE, 0 },
  {   0, 0xFFFF, 0xFFFF, 0x001E, 0x001E, 121, "CTL2 TYPE", 231, FX_DEFAULT_TYPE, 0 },
  {   0, 0xFFFF, 0xFFFF, 0x002A, 0x002A, 46, "EXP1 TYPE", 186, FX_DEFAULT_TYPE, 0 },
  {   0, 0xFFFF, 0xFFFF, 0x002B, 0x002B, 46, "EXP2 TYPE", 186, FX_DEFAULT_TYPE, 0 },
};

const uint16_t SY1000_NUMBER_OF_PARAMETERS = sizeof(SY1000_parameters) / sizeof(SY1000_parameters[0]);

#define SY1000_EXP1 SY1000_NUMBER_OF_PARAMETERS - 2
#define SY1000_EXP2 SY1000_NUMBER_OF_PARAMETERS - 1

const PROGMEM char SY1000_sublists[][9] = {
  // Sublist 1 - 8: INST types (GTR mode)
  "DYN SYN", "OSC SYN", "GR-300", "E.GUITAR", "ACOUSTIC", "E.BASS", "VIO GTR", "POLY FX",

  // Sublist 9 - 15: INST types (BASS mode)
  "DYN SYN", "OSC SYN", "ANALG GR", "E.BASS", "AC BASS", "E.GTR", "POLY FX",

  // Sublist 16 - 22: COMP types
  "BOSS CMP", "Hi-BAND", "LIGHT", "D-COMP", "ORANGE", "FAT", "MILD",

  // Sublist 23 - 57: DIST Types
  "MID BST", "CLN BST", "TREBLE B", "CRUNCH", "NAT OD", "WARM OD", "FAT DS", "LEAD DS", "METAL DS", "OCT FUZZ",
  "A-DIST", "X-OD", "X-DIST", "BLUES OD", "OD-1", "T-SCREAM", "TURBO OD", "DIST", "CENTA OD", "RAT",
  "GUV DS", "DIST+", "MTL ZONE", "HM-2", "MTL CORE", "60S FUZZ", "MUFF FUZ", "BASS OD", "BASS DS", "BASS MT",
  "BASS FUZ", "HI BND D", "X-BAS OD", "BASS DRV", "BASS DI",

  // Sublist 58 - 88: AMP types
  "TRANSP", "NATURAL", "BOUTIQUE", "SUPREME", "MAXIMUM", "JUGGERNT", "X-CRUNCH", "X-HI GN", "X-MODDED", "JC-120",
  "TWIN CMB", "DELUXE" , "TWEED", "DIAMOND", "BRIT STK", "RECTI ST", "MATCH CB", "BG COMBO", "ORNG STK", "BGNR UB",
  "NAT BASS", "X-DRV BS", "CONCERT", "SUPER FL", "FLIP TOP", "B MAN", "BASS 360", "SW-TOUR", "AC BASS", "GK BASS", "MARK",

  // Sublist 89 - 100: MAST DLY types
  "STEREO1", "STEREO2", "PAN", "DUAL-S", "DUAL-P", "DUAL L/R", "REVERSE", "ANALOG", "TAPE", "MOD", "WARP", "TWIST",

  // Sublist 101 - 104: CHORUS types
  "MONO", "STEREO1", "STEREO2", "DUAL",

  // Sublist 105 - 141: FX types
  "AC RESO", "AUTO WAH", "CHORUS", "CL-VIBE", "COMPRSSR", "DEFRETTR", "DEFRET B", "DELAY", "FLANGER", "FLANGR B",
  "FOOT VOL", "GRAPH EQ", "HARMO", "HUMANIZR", "ISOLATOR", "LIMITER", "LO-FI", "OCTAVE", "OCT BASS", "PAN",
  "PARAM EQ", "PDL BEND", "PHASER", "PITCH SH", "REVERB", "RING MOD", "ROTARY", "SITAR SM", "SLICER", "SLOW GR",
  "SLW GR B", "SND HOLD", "TOUCH W", "TW BASS", "TREMOLO", "VIBRATO", "WAH",

  // Sublist 142 - 148: REVERB types
  "AMBIENCE", "ROOM", "HALL1", "HALL2", "PLATE", "SPRING", "MOD",

  // Sublist 149 - 185: Alt Tune types
  "OPEN D", "OPEN E", "OPEN G", "OPEN A", "DROP D", "DROP Db", "DROP C", "DROP B", "DROP Bb", "DROP A",
  "D-MODAL", "NASHVL", "-12STEP", "-11STEP", "-10STEP", "-9STEP", "-8STEP", "-7STEP", "-6STEP", "-5STEP",
  "-4STEP", "-3STEP", "-2STEP", "-1STEP", "+1STEP", "+2STEP", "+3STEP", "+4STEP", "+5STEP", "+6STEP",
  "+7STEP", "+8STEP", "+9STEP", "+10STEP", "+11STEP", "+12STEP", "USER",

  // Sublist 186 - 230: EXP pedal types
  "OFF", "FV1", "FV2", "FV1+TUNR", "FV2+TUNR", "FX1 P.PS", "FX2 P.PS", "FX3 P.PS", "P.LVL100", "P.LVL200",
  "INST1 LV", "INST2 LV", "INST3 LV", "INST ALL", "1:CUTOFF", "2:CUTOFF", "3:CUTOFF", "A:CUTOFF", "1:RESO", "2:RESO",
  "3:RESO", "ALL RESO", "1:GTR VL", "2:GTR VL", "3:GTR VL", "ALL:G VL", "1:NOR100", "1:NOR200", "2:NOR100", "2:NOR200",
  "3:NOR100", "3:NOR200", "A:NOR100", "A:NOR200", "1:S.BEND", "2:S.BEND", "3:S.BEND", "A:S.BEND", "1:DYNA B", "2:DYNA B",
  "3:DYNA B", "A:DYNA B", "MIXR A/B", "BAL1 A/B", "BAL2 A/B", "BAL3 A/B",

  // Sublist 231 - 352: CTL types
  "OFF", "*/BANK+", "BANK-", "PATCH +1", "PATCH -1", "LEVEL+10", "LEVEL+20", "LEVEL-10", "LEVEL-20", "BPM TAP",
  "DLY1 TAP", "DLY2 TAP", "MDLY TAP", "FX1D TAP", "FX2D TAP", "FX3D TAP", "TUNER", "MANUAL", "MANUAL/T", "TUNER/M",
  "INST1", "INST2", "INST3", "INST ALL", "1:NM MIX", "2:NM MIX", "3:NM MIX", "NORMAL", "1:ALT TN", "2:ALT TN",
  "3:ALT TN", "A:ALT TN", "1:12STR", "2:12STR", "3:12STR", "ALL:12ST", "1:STR BD", "2:STR BD", "3:STR BD", "ALL:ST B",
  "1:LFO1", "1:LFO2", "1:LFO1&2", "2:LFO1", "2:LFO2", "2:LFO1&2", "3:LFO1", "3:LFO2", "3:LFO1&2", "ALL:LFO",
  "1:SEQ1", "1:SEQ2", "1:SEQ1&2", "2:SEQ1", "2:SEQ2", "2:SEQ1&2", "3:SEQ1", "3:SEQ2", "3:SEQ1&2", "ALL:SEQ",
  "1:SEQ1 T", "1:SEQ2 T", "1:S1&2 T", "2:SEQ1 T", "2:SEQ2 T", "2:S1&2 T", "3:SEQ1 T", "3:SEQ2 T", "3:S1&2 T", "ALL:SQ T",
  "1:S1 TBO", "1:S2 TBO", "1:1&2TBO", "2:S1 TBO", "2:S2 TBO", "2:1&2TBO", "3:S1 TBO", "3:S2 TBO", "3:1&2TBO", "ALL:TBO",
  "1:HOLD", "2:HOLD", "3:HOLD", "ALL:HOLD", "SYNC TRG", "1:PU UP", "2:PU UP", "3:PU UP", "1:PU DN", "2:PU DN",
  "3:PU DN", "1:AMP", "2:AMP", "3:AMP", "1:AMP SL", "2:AMP SL", "3:AMP SL", "FX1", "FX2", "FX3",
  "CMP", "DS", "DS SOLO", "AMP", "AMP SOLO", "EQ1", "EQ2", "NS", "DLY1", "DLY2",
  "MST DLY", "CHO", "REV", "S/R", "FX1 TRIG", "FX2 TRIG", "FX3 TRIG", "MDLY TRG", "DIV CH S", "MD START",
  "MMC PLAY",

  // Sublist 353 -356: Switch mode
  "NUM", "MANUAL", "PATCH", "SCENE", "SC_ASSGN", "SCENE T", "SCENE B",
};

const uint16_t SY1000_SIZE_OF_SUBLIST = sizeof(SY1000_sublists) / sizeof(SY1000_sublists[0]);

#define SY1000_GTR_INST_TYPES_SUBLIST 1
#define SY1000_BASS_INST_TYPES_SUBLIST 9
#define SY1000_CTL_TYPE_SUBLIST 231

const uint8_t SY1000_FX_colours[] = { // Table with the LED colours for the different FX types
  FX_GTR_TYPE, // Colour for "AC RESO"
  FX_WAH_TYPE, // Colour for "AUTO WAH"
  FX_MODULATE_TYPE, // Colour for "CHORUS"
  FX_MODULATE_TYPE, // Colour for "CL-VIBE"
  FX_DYNAMICS_TYPE, // Colour for "COMPRSSR"
  FX_FILTER_TYPE, // Colour for "DEFRETTR"
  FX_FILTER_TYPE, // Colour for "DEFRET B"
  FX_DELAY_TYPE, // Colour for "DELAY",
  FX_MODULATE_TYPE, // Colour for "FLANGER"
  FX_MODULATE_TYPE, // Colour for "FLANGR B"
  FX_FILTER_TYPE, // Colour for "FOOT VOL"
  FX_FILTER_TYPE, // Colour for "GRAPH EQ"
  FX_PITCH_TYPE, // Colour for "HARMO"
  FX_PITCH_TYPE, // Colour for "HUMANIZR",
  FX_FILTER_TYPE, // Colour for "ISOLATOR",
  FX_DYNAMICS_TYPE, // Colour for "LIMITER"
  FX_FILTER_TYPE, // Colour for "LO-FI"
  FX_PITCH_TYPE, // Colour for "OCTAVE"
  FX_PITCH_TYPE, // Colour for "OCT BASS"
  FX_MODULATE_TYPE, // Colour for "PAN",
  FX_FILTER_TYPE, // Colour for "PARAM EQ"
  FX_PITCH_TYPE, // Colour for "PDL BEND"
  FX_MODULATE_TYPE, // Colour for "PHASER"
  FX_PITCH_TYPE, // Colour for "PITCH SH"
  FX_REVERB_TYPE, // Colour for "REVERB",
  FX_MODULATE_TYPE, // Colour for "RING MOD"
  FX_MODULATE_TYPE, // Colour for "ROTARY"
  FX_GTR_TYPE, // Colour for "SITAR SM"
  FX_MODULATE_TYPE, // Colour for "SLICER"
  FX_FILTER_TYPE, // Colour for "SLOW GR"
  FX_FILTER_TYPE, // Colour for "SLW GR B"
  FX_FILTER_TYPE, // Colour for "SND HOLD"
  FX_WAH_TYPE, // Colour for "TOUCH W"
  FX_WAH_TYPE, // Colour for "TW BASS"
  FX_MODULATE_TYPE, // Colour for "TREMOLO"
  FX_MODULATE_TYPE, // Colour for "VIBRATO"
  FX_WAH_TYPE, // Colour for "WAH"
};

FLASHMEM void MD_SY1000_class::read_parameter_name(uint16_t number, String &Output) { // Called from menu
  if (number < number_of_parameters())  Output = SY1000_parameters[number].Name;
  else Output = "?";
}

FLASHMEM void MD_SY1000_class::read_parameter_value_name(uint16_t number, uint16_t value, String &Output) {
  if (number < SY1000_NUMBER_OF_PARAMETERS)  {
    uint16_t my_sublist = SY1000_parameters[number].Sublist;
    if ((my_sublist > 0) && !(my_sublist & SUBLIST_FROM_BYTE2)) { // Check if state needs to be read
      switch (my_sublist) {
        case SHOW_NUMBER:
          Output += String(value);
          break;
        case SHOW_DOUBLE_NUMBER:
          Output += String(value * 2); //Patch level is displayed double
          break;
        case SHOW_PAN:
          if (value < 64) Output += "L" + String(50 - value);
          if (value == 64) Output += "C";
          if (value > 64) Output += "R" + String(value - 50);
          break;
        case SY1000_INST_SUBLIST:
          if (!bass_mode) my_sublist = SY1000_GTR_INST_TYPES_SUBLIST;
          else my_sublist = SY1000_BASS_INST_TYPES_SUBLIST;
        // no break!
        default:
          String type_name = SY1000_sublists[my_sublist + value - 1];
          Output += type_name;
          break;
      }
    }
    else if (value == 1) Output += "ON";
    else Output += "OFF";
  }
  else Output += "?";
}

FLASHMEM uint32_t MD_SY1000_class::read_parameter_address(uint16_t number) {
  if (number >= SY1000_NUMBER_OF_PARAMETERS) return 0;

  uint32_t address;
  uint8_t inst_type = 0;
  if (!bass_mode) {
    address = SY1000_parameters[number].Address_GM;
    if (address == ADDR_MANUAL) return 0x0001103F;
    if (address == SET_SW_MODE) return 0x00011021;
    if (address & 0x8000) address = (address & 0x7FFF) | 0x10000;
    address += 0x10000000;
  }
  else {
    address = SY1000_parameters[number].Address_BM;
    if (address == ADDR_MANUAL) return 0x0008103F;
    if (address == SET_SW_MODE) return 0x00081021;
    if (address & 0x8000) address = (address & 0x7FFF) | 0x10000;
    address += 0x10020000;
  }
  uint8_t type = SY1000_parameters[number].Type;
  if (type > 0) {
    inst_type = INST_type[(type & 0x0F) - 1];
    switch (type & 0xF0) {
      case 0x10: // Type INST1_LFO, INST2_LFO or INST3_LFO
        if (inst_type == 1) address += 0x206;
        break;
      case 0x20: // Type INST1_AMP, INST2_AMP or INST3_AMP
        if ((!bass_mode) && (inst_type == 4)) address += 0x102; // Acoustic
        if ((!bass_mode) && (inst_type == 5)) address += 0x201; // E-Bass
        if ((bass_mode) && (inst_type == 4)) address += 0xF2; // Acoustic Bass
        if ((bass_mode) && (inst_type == 5)) address += 0x1F4; // E-Gtr
        break;
    }
  }
  return address;
}

// Toggle SY1000 stompbox parameter
FLASHMEM void MD_SY1000_class::parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {
  if (number >= SY1000_NUMBER_OF_PARAMETERS) return;

  // Send sysex MIDI command to SY-1000
  uint8_t value = SCO_return_parameter_value(Sw, cmd);
  if ((SP[Sw].Latch != TGL_OFF) && (number < SY1000_NUMBER_OF_PARAMETERS)) {
    switch (SY1000_parameters[number].Address_GM) {
      case SET_SW_MODE:
        set_switch_mode(value);
        break;
      case VIA_ASSIGN:
        if (SY1000_parameters[number].Type != DO_TAP_TEMPO) {
          uint16_t target;
          if (!bass_mode) target = SY1000_parameters[number].Target_GM;
          else target = SY1000_parameters[number].Target_BM;
          set_assign_settings(15, target, SP[Sw].Latch, SY1000_SOURCE_CC95);
          delay(20);
          MIDI_send_CC(95, 127, MIDI_channel, MIDI_out_port);
          assign_return_target_timer = millis() + SY1000_ASSIGN_RETURN_TIME;
        }
        else {
          SCO_global_tap_tempo_press(Sw);
          update_page = REFRESH_PAGE;
          return;
        }
        break;
      default:
        write_parameter_value(number, value);
        break;
    }

    SP[Sw].Offline_value = value;

    // Show message
    check_update_label(Sw, value);
    String msg = "";
    if (SP[Sw].Type != ASSIGN) {
      msg = SY1000_parameters[number].Name;
      if ((SY1000_parameters[number].Sublist & SUBLIST_FROM_BYTE2) || (SY1000_parameters[number].Sublist == 0)) msg += ' ';
      else msg += ':';
    }
    msg += SP[Sw].Label;
    LCD_show_popup_label(msg, ACTION_TIMER_LENGTH);

    //PAGE_load_current(false); // To update the other parameter states, we re-load the current page
    if (SP[Sw].Latch != UPDOWN) update_page = REFRESH_FX_ONLY;
  }
}

FLASHMEM void MD_SY1000_class::parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {
  if (number >= SY1000_NUMBER_OF_PARAMETERS) return;

  // Work out state of pedal
  switch (SY1000_parameters[number].Address_GM) {
    case VIA_ASSIGN:
      MIDI_send_CC(95, 0, MIDI_channel, MIDI_out_port);
      delay(20);
      restore_assign_settings(15);
      break;
    case SET_SW_MODE:
      break;
    default:
      if (SP[Sw].Latch == MOMENTARY) {
        SP[Sw].State = 2; // Switch state off
        if (number < SY1000_NUMBER_OF_PARAMETERS) {
          write_parameter_value(number, cmd->Value2);
          SP[Sw].Offline_value = cmd->Value2;
        }

        //PAGE_load_current(false); // To update the other switch states, we re-load the current page
        update_page = REFRESH_FX_ONLY;
      }
  }

}

FLASHMEM void MD_SY1000_class::write_parameter_value(uint16_t number, uint8_t value) {
  if (number >= SY1000_NUMBER_OF_PARAMETERS) return;
  control_edit_mode();
  if (SY1000_parameters[number].Address_GM != VIA_ASSIGN) {
    switch (SY1000_parameters[number].Target_GM) {
      case PREV2:
        for (uint8_t i = 0; i < 2; i++) {
          write_sysex(read_parameter_address(number - 2 + i), value);
        }
        break;
      case PREV3:
        for (uint8_t i = 0; i < 3; i++) {
          write_sysex(read_parameter_address(number - 3 + i), value);
        }
        break;
      case PREV9:
        for (uint8_t i = 0; i < 9; i++) {
          if (SY1000_parameters[number - 9 + i].Target_GM < 0x8000) write_sysex(read_parameter_address(number - 9 + i), value);
        }
        break;
      default:
        write_sysex(read_parameter_address(number), value);
        break;
    }
  }
}

FLASHMEM void MD_SY1000_class::read_parameter_title(uint16_t number, String &Output) {
  if (number >= SY1000_NUMBER_OF_PARAMETERS) return;
  Output += SY1000_parameters[number].Name;
}

FLASHMEM bool MD_SY1000_class::request_parameter(uint8_t sw, uint16_t number) {
  if (number >= SY1000_NUMBER_OF_PARAMETERS) return true;
  if (can_request_sysex_data()) {
    switch (SY1000_parameters[number].Address_GM)  {
      case VIA_ASSIGN:
        read_parameter(sw, 0, 0);
        return true;
      case SET_SW_MODE:
        read_parameter(sw, switch_mode, 0);
        return true;
      default:
        uint32_t my_address = read_parameter_address(number);
        last_requested_sysex_address = my_address;
        last_requested_sysex_type = REQUEST_PARAMETER_TYPE;
        last_requested_sysex_switch = sw;
        request_sysex(my_address, 2); // Request the parameter state data
        return false; // Move to next switch is false. We need to read the parameter first
    }
  }
  else {
    if ((sw < TOTAL_NUMBER_OF_SWITCHES) && (SP[sw].Type == PAR_BANK)) read_parameter(sw, SP[sw].Offline_value, SP[sw + 1].Offline_value);
    else read_parameter(sw, SP[sw].Offline_value, 0);
    return true;
  }
}

FLASHMEM void MD_SY1000_class::read_parameter(uint8_t sw, uint8_t byte1, uint8_t byte2) { //Read the current SY1000 parameter
  SP[sw].Target_byte1 = byte1;
  SP[sw].Target_byte2 = byte2;

  // Set the status
  SP[sw].State = SCO_find_parameter_state(sw, byte1);

  // Set the colour
  uint16_t index = SP[sw].PP_number; // Read the parameter number (index to SY1000-parameter array)
  uint8_t my_colour = SY1000_parameters[index].Colour;

  //Check for special colours:
  if (my_colour == SY1000_FX_COLOUR) my_colour = SY1000_FX_colours[byte2]; //FX type read in byte2
  if (my_colour == SY1000_FX_TYPE_COLOUR) my_colour = SY1000_FX_colours[byte1]; //FX type read in byte1

  SP[sw].Colour =  my_colour;

  // Set the display message
  String msg = "";
  uint16_t my_sublist = SY1000_parameters[index].Sublist;
  if ((my_sublist & 0x7FFF) == SY1000_INST_SUBLIST) {
    if (!bass_mode) my_sublist = (my_sublist & SUBLIST_FROM_BYTE2) + SY1000_GTR_INST_TYPES_SUBLIST;
    else my_sublist = (my_sublist & SUBLIST_FROM_BYTE2) + SY1000_BASS_INST_TYPES_SUBLIST;
  }
  if (SP[sw].Type == ASSIGN) msg = SY1000_parameters[index].Name;
  if (my_sublist > SUBLIST_FROM_BYTE2) { // Check if a sublist exists
    String type_name = SY1000_sublists[my_sublist - SUBLIST_FROM_BYTE2 + byte2 - 1];
    msg += " (" + type_name + ')';
  }
  if ((my_sublist > 0) && !(my_sublist & SUBLIST_FROM_BYTE2)) { // Check if state needs to be read
    //String type_name = SY1000_sublists[my_sublist + byte1 - 101];
    if (SP[sw].Type == ASSIGN) msg += ':';
    read_parameter_value_name(index, byte1, msg);
  }
  //Copy it to the display name:
  LCD_set_SP_label(sw, msg);
  update_lcd = sw;
}

FLASHMEM void MD_SY1000_class::check_update_label(uint8_t Sw, uint8_t value) { // Updates the label for extended sublists
  uint16_t index = SP[Sw].PP_number; // Read the parameter number (index to SY1000-parameter array)
  if ((index != NOT_FOUND) && (index <  SY1000_NUMBER_OF_PARAMETERS)) {
    if ((SY1000_parameters[index].Sublist > 0) && !(SY1000_parameters[index].Sublist & SUBLIST_FROM_BYTE2)) { // Check if state needs to be read
      LCD_clear_SP_label(Sw);
      // Set the display message
      String msg = "";
      read_parameter_value_name(index, value, msg);

      //Copy it to the display name:
      LCD_set_SP_label(Sw, msg);

      //Update the current switch label
      update_lcd = Sw;
    }
  }
}

FLASHMEM uint16_t MD_SY1000_class::number_of_parameters() {
  return SY1000_NUMBER_OF_PARAMETERS;
}

FLASHMEM uint8_t MD_SY1000_class::number_of_values(uint16_t parameter) {
  if (parameter < SY1000_NUMBER_OF_PARAMETERS) return SY1000_parameters[parameter].NumVals;
  else return 0;
}

// ** Switch mode

FLASHMEM void MD_SY1000_class::set_switch_mode(uint8_t mode) {
  DEBUGMAIN("** Set switch mode to " + String(mode));
  prev_switch_mode = switch_mode;
  //if (mode == switch_mode) return;
  switch_mode = mode;

  uint8_t num_switch_global_type = 1; // Set state of the 1, 2, 3 and 4 switches
  uint8_t num_switch_system = 0;
  uint8_t bank_up_switch_global_type = 1; // Set state of the bank down and bank up switches
  uint8_t bank_down_switch_global_type = 1;
  uint8_t bank_switch_system = 0;
  uint8_t ctl12_switch_global_type = 1;  // Set state of the CTL1 and CTL2 switches
  uint8_t ctl12_switch_system = 0;
  uint8_t manual_mode = 0;
  uint8_t top_row_led_colour_from_VController = 0; // Set LED color pref on the SY1000 to patch (0) or system (1)
  uint8_t bottom_row_led_colour_from_VController = 0;

  switch (mode) {
    case MODE_MANUAL:
      ctl12_switch_global_type = 1;
      manual_mode = 1;
      edit_mode_always_on = true;
      break;
    case MODE_PATCH:
      num_switch_system = 1;
      bank_switch_system = 1;
      edit_mode_always_on = true;
      break;
    case MODE_SCENE:
    case MODE_SCENE_ASSIGN:
      num_switch_global_type = 0;
      num_switch_system = 1;
      bank_up_switch_global_type = 0;
      bank_down_switch_global_type = 0;
      bank_switch_system = 1;
      ctl12_switch_global_type = 0;
      ctl12_switch_system = 1;
      top_row_led_colour_from_VController = 1;
      bottom_row_led_colour_from_VController = 1;
      edit_mode_always_on = false;
      break;
    case MODE_SCENE_TOP_ROW: // Scenes on top - manual mode on bottomw row
      bank_up_switch_global_type = 0;
      bank_down_switch_global_type = 0; bank_switch_system = 1;
      ctl12_switch_global_type = 0;
      ctl12_switch_system = 1;
      top_row_led_colour_from_VController = 1;
      edit_mode_always_on = false;
      manual_mode = 1;
      break;
    case MODE_SCENE_BOTTOM_ROW: // Scenes on bottom - patch up/down + ctl 1/2 on top row
      bank_up_switch_global_type = 2;
      bank_down_switch_global_type = 3;
      bank_switch_system = 1;
      num_switch_global_type = 0;
      num_switch_system = 1;
      bottom_row_led_colour_from_VController = 1;
      edit_mode_always_on = false;
      break;
  }

  control_edit_mode();
  uint32_t address;
  if (!bass_mode) address = 0x00011000;
  else address = 0x00081000;

  // Enable or disable the num switches
  write_sysex(address, num_switch_global_type, 0, num_switch_global_type, 0, num_switch_global_type, 0, num_switch_global_type, 0);
  // Enable or disable the pbank up/down and ctl1/2 switches
  write_sysex(address + 0x08, bank_down_switch_global_type, 0, bank_up_switch_global_type, 0, ctl12_switch_global_type, 0, ctl12_switch_global_type, 0);
  // Set all switches to patch or system
  write_sysex(address + 0x21, num_switch_system, num_switch_system, num_switch_system, num_switch_system, bank_switch_system, bank_switch_system, ctl12_switch_system, ctl12_switch_system);
  // Enable or disable manual mode
  write_sysex(address + 0x3F, manual_mode); // Memory mode
  // Set LED colour pref for all eight switches
  write_sysex(address + 0x1012, bottom_row_led_colour_from_VController, bottom_row_led_colour_from_VController, bottom_row_led_colour_from_VController, bottom_row_led_colour_from_VController, top_row_led_colour_from_VController, top_row_led_colour_from_VController, top_row_led_colour_from_VController, top_row_led_colour_from_VController); // LED colours from SY1000 (patch)

  update_leds_on_SY1000();
}

FLASHMEM void MD_SY1000_class::request_switch_mode() {
  request_sysex(ADDR_MANUAL, 1);
  if (!bass_mode) {
    request_sysex(0x00012012, 8);
  }
  else {
    request_sysex(0x00082012, 8);
  }
  request_LED_state = true;
}

FLASHMEM void MD_SY1000_class::check_switch_mode(uint32_t address, const unsigned char* sxdata, short unsigned int sxlength) {
  if (address == ADDR_MANUAL) {
    manual_mode = (sxdata[sx_index(sxdata[3], 12)] == 0x01 ? true : false);
  }

  if ((address == 0x00012012) || (address == 0x00082012)) {
    if (request_LED_state) {
      uint8_t data3 = sxdata[3];
      bool top_row_on = (sxdata[sx_index(data3, 16)] == 0x01 ? true : false);
      bool bottom_row_on = (sxdata[sx_index(data3, 12)] == 0x01 ? true : false);
      DEBUGMSG("!!! LED_state top: " + String (top_row_on) + ", bottom: " + String (bottom_row_on));

#ifdef IS_VCMINI
      if ((bottom_row_on) && (top_row_on)) {
        set_switch_mode(MODE_SCENE);
      }
      else if ((!bottom_row_on) && (top_row_on)) {
        set_switch_mode(MODE_SCENE_TOP_ROW);
      }
      else if ((bottom_row_on) && (!top_row_on)) {
        set_switch_mode(MODE_SCENE_BOTTOM_ROW);
      }
      else {
        if (manual_mode) set_switch_mode(MODE_MANUAL);
        else set_switch_mode(MODE_NUM);
      }
#else
      if ((bottom_row_on) || (top_row_on)) {
        if (manual_mode) set_switch_mode(MODE_MANUAL);
        else set_switch_mode(MODE_NUM);
      }
#endif
      request_LED_state = false;
      update_page = REFRESH_FX_ONLY;
      request_next_data_item();
    }
  }
}

FLASHMEM void MD_SY1000_class::cc_operate_switch_mode(uint8_t sw, uint8_t value) {
  bool no_scene_switch = false;
  switch (switch_mode) {
    case MODE_NUM:
    case MODE_MANUAL:
    case MODE_PATCH:
      no_scene_switch = true;
    case MODE_SCENE_TOP_ROW:
      if (sw < 4) no_scene_switch = true;
      break;
    case MODE_SCENE_BOTTOM_ROW:
      if (sw >= 4) no_scene_switch = true;
      break;
  }
  DEBUGMAIN("** Operate SY1000 switch " + String(sw));

  if (no_scene_switch) {
    // Whenever a switch is pressed twice, request the tempo when edit mode is not active - this is the best way to keep the tempo on the VC-mini in sync with the SY1000
    if ((prev_switch_mode_cc == sw + 1) && (!edit_mode) && (millis() > check_ample_time_between_pc_messages_timer)) {
      send_tempo_timer = millis() + CHECK_SEND_TEMPO_TIME; // We need to send the message delayed though, as tempo is wrong right after a patch change, which the SY1000 switches often initiate.
    }
    prev_switch_mode_cc = sw + 1;
    return;
  }

  const uint8_t switch_source[8] = { 0, 1, 2, 3, 5, 6, 7, 8 };
  uint8_t my_source = switch_source[sw];

  if (value == 127) {
    if (prev_switch_mode_cc != sw + 1) { // Catching quirk: when connected through USB_MIDI the CC message is sent twice!
      prev_switch_mode_cc = sw + 1;

      // Disable any assigns for this switch by changing the source to CC95
      // It is amazing this actually works, but it looks like the SY1000 is delaying the execution of the assigns
      // so we have enough time to switch it off, before it gets executed!
      change_active_assign_sources(my_source, SY1000_SOURCE_CC95);

      String msg;
      uint8_t new_scene;
      switch (switch_mode) {
        case MODE_SCENE:
          new_scene = sw + 1;
          set_snapscene(0, new_scene);
          msg = "Scene " + String(new_scene) + ':';
          read_scene_name_from_buffer(new_scene);
          msg += scene_label_buffer;
          LCD_show_popup_label(msg, MESSAGE_TIMER_LENGTH);
          break;
        case MODE_SCENE_TOP_ROW:
          if (current_snapscene == (sw - 3)) new_scene = sw + 1; // Select scene 5 - 8
          else new_scene = sw - 3; // select scene 1 - 4
          set_snapscene(0, new_scene);
          msg = "Scene " + String(new_scene) + ':';
          read_scene_name_from_buffer(new_scene);
          msg += scene_label_buffer;
          LCD_show_popup_label(msg, MESSAGE_TIMER_LENGTH);
          delay(40);
          set_editor_mode(false);
          break;
        case MODE_SCENE_BOTTOM_ROW:
          if (current_snapscene == (sw + 1)) new_scene = sw + 5; // Select scene 5 - 8
          else new_scene = sw + 1; // Select scene 1 - 4
          set_snapscene(0, new_scene);
          msg = "Scene " + String(new_scene) + ':';
          read_scene_name_from_buffer(new_scene);
          msg += scene_label_buffer;
          LCD_show_popup_label(msg, MESSAGE_TIMER_LENGTH);
          delay(25);
          set_editor_mode(false);
          break;
        case MODE_SCENE_ASSIGN:
          toggle_scene_assign(sw);
          break;
      }
    }
  }
  else {
    // Enable assigns for this switch
    change_active_assign_sources(my_source, my_source);
    prev_switch_mode_cc = 0;
    set_editor_mode(false);
  }
}

FLASHMEM void MD_SY1000_class::auto_return_switch_mode() {
  if (switch_mode != MODE_PATCH) {
    //update_leds_on_SY1000();
    return;
  }
  if (prev_switch_mode == switch_mode) return;
  if (prev_switch_mode == MODE_PATCH) return;
  set_switch_mode(prev_switch_mode);
  update_page = REFRESH_FX_ONLY;
}

FLASHMEM void MD_SY1000_class::update_leds_on_SY1000() {
  switch (switch_mode) {
    case MODE_SCENE:
    case MODE_SCENE_TOP_ROW:
    case MODE_SCENE_BOTTOM_ROW:
      set_snapscene_number_and_LED(current_snapscene);
      break;
    case MODE_SCENE_ASSIGN:
      show_scene_assign_LEDs();
      break;
  }
}

FLASHMEM void MD_SY1000_class::set_LED_colour(uint8_t sw, uint8_t colour) {
  if (!connected) return;
  if ((switch_mode != MODE_SCENE) && (switch_mode != MODE_SCENE_ASSIGN) && (switch_mode != MODE_SCENE_TOP_ROW) && (switch_mode != MODE_SCENE_BOTTOM_ROW)) return;
  if (sw > 7) return;

  // Translate VController coloiur to SY1000 colour
  /*uint8_t SY1000_colour = 0;
    switch (colour) {
    case 1: SY1000_colour = SY1000_GREEN; break; // Colour 1 is Green
    case 2: SY1000_colour = SY1000_RED; break; // Colour 2 is Red
    case 3: SY1000_colour = SY1000_BLUE; break; // Colour 3 is Blue
    case 4: SY1000_colour = SY1000_RED; break; // Colour 4 is Orange
    case 5: SY1000_colour = SY1000_LIGHT_BLUE; break; // Colour 5 is Cyan
    case 6: SY1000_colour = SY1000_WHITE; break; // Colour 6 is White
    case 7: SY1000_colour = SY1000_YELLOW; break; // Colour 7 is Yellow
    case 8: SY1000_colour = SY1000_PURPLE; break; // Colour 8 is Purple
    case 9: SY1000_colour = SY1000_PURPLE; break; // Colour 9 is Pink
    case 10: SY1000_colour = SY1000_GREEN; break; // Colour 10 is Soft Green
    case 11: SY1000_colour = SY1000_LIGHT_BLUE; break; // Colour 11 is Light Blue
    }*/

  control_edit_mode();

  uint32_t address;
  if (!bass_mode) address = 0x00012000;
  else address = 0x00082000;

  write_sysex(address + (sw * 2), colour, colour);
  DEBUGMSG("SY1000 SW " + String(sw) + " set to colour " + String(colour));
}

FLASHMEM void MD_SY1000_class::set_all_LED_colours(uint8_t * colour) {
  if (!connected) return;
  if ((switch_mode != MODE_SCENE) && (switch_mode != MODE_SCENE_ASSIGN)) return;

  uint32_t address;
  if (!bass_mode) address = 0x00012000;
  else address = 0x00082000;

  write_sysex(address, colour[0], colour[0], colour[1], colour[1], colour[2], colour[2], colour[3], colour[3]);
  write_sysex(address + 8, colour[4], colour[4], colour[5], colour[5], colour[6], colour[6], colour[7], colour[7]);
}

FLASHMEM void MD_SY1000_class::update_switches_on_page(uint32_t address, uint8_t data) {
  for (uint8_t s = 1; s < TOTAL_NUMBER_OF_SWITCHES + 1; s++) {
    uint8_t Dev = SP[s].Device;
    if (Dev == CURRENT) Dev = Current_device;
    uint16_t index = SP[s].PP_number;
    uint32_t par_address;
    par_address = read_parameter_address(index);
    if ((Dev == my_device_number) && (address == par_address)) {
      update_page = REFRESH_FX_ONLY;
    }
  }
}

// ********************************* Section 6: SY1000 assign control ********************************************

// SY1000 has 16 general assigns, which we can be controlled with a cc-MIDI message.

// Procedures for SY1000_ASSIGN:
// 1. Load in SP array - SY1000_assign_load() below
// 2. Request - SY1000_request_current_assign()
// 3. Read assign - SY1000_read_current_assign(uint8_t sw) - also requests parameter state
// 4. Read parameter state - SY1000_read_parameter() above
// 5. Press switch - SY1000_assign_press() below
// 6. Release switch - SY1000_assign_release() below

struct SY1000_assign_struct {
  char Title[8];
  char Title_short[5];
  uint8_t Address;
  uint8_t Assign_target_number; // To control the regular assigns
  uint8_t Type;
};

#define SY1000_CTL_TYPE 1
#define SY1000_NUM_TYPE 2
#define SY1000_MAN_TYPE 3

const PROGMEM SY1000_assign_struct SY1000_assigns[] = {
  {"CTL1", "CTL1", 0x1C, 7, SY1000_CTL_TYPE },
  {"CTL2", "CTL2", 0x1E, 8, SY1000_CTL_TYPE },
  {"CTL3", "CTL3", 0x20, 9, SY1000_CTL_TYPE },
  {"CTL4", "CTL4", 0x22, 10, SY1000_CTL_TYPE },
  {"CTL5", "CTL5", 0x24, 11, SY1000_CTL_TYPE },
  {"CTL6", "CTL6", 0x26, 12, SY1000_CTL_TYPE },
  {"BANK DN", "BNK-", 0x18, 5, SY1000_NUM_TYPE },
  {"BANK UP", "BNK+", 0x1A, 6, SY1000_NUM_TYPE },
  {"NUM1", "NUM1", 0x10, 0, SY1000_NUM_TYPE },
  {"NUM2", "NUM2", 0x12, 1, SY1000_NUM_TYPE },
  {"NUM3", "NUM3", 0x14, 2, SY1000_NUM_TYPE },
  {"NUM4", "NUM4", 0x16, 3, SY1000_NUM_TYPE },
  {"MANUAL1", "MNL1", 0x31, 127, SY1000_MAN_TYPE },
  {"MANUAL2", "MNL2", 0x33, 127, SY1000_MAN_TYPE },
  {"MANUAL3", "MNL3", 0x35, 127, SY1000_MAN_TYPE },
  {"MANUAL4", "MNL4", 0x37, 127, SY1000_MAN_TYPE },
  //{"GK SW1", "GKS1", 0x2D, 15, SY1000_CTL_TYPE },
  //{"GK SW2", "GKS2", 0x2F, 16, SY1000_CTL_TYPE },
};

const uint8_t SY1000_NUMBER_OF_CTL_FUNCTIONS = sizeof(SY1000_assigns) / sizeof(SY1000_assigns[0]);
#define SY1000_TOTAL_NUMBER_OF_ASSIGNS SY1000_NUMBER_OF_CTL_FUNCTIONS + SY1000_NUMBER_OF_SCENE_ASSIGNS

FLASHMEM uint32_t MD_SY1000_class::calculate_assign_address(uint8_t number) {
  if (number >= SY1000_TOTAL_NUMBER_OF_ASSIGNS) return 0;
  if (number < SY1000_NUMBER_OF_CTL_FUNCTIONS) {
    if (!bass_mode) return 0x10000000 + SY1000_assigns[number].Address;
    else return 0x10020000 + SY1000_assigns[number].Address;
  }
  else {
    number -= SY1000_NUMBER_OF_CTL_FUNCTIONS;
    return calculate_full_assign_address(number);
  }
}

FLASHMEM void MD_SY1000_class::read_assign_name(uint8_t number, String & Output) {
  if (number < SY1000_NUMBER_OF_CTL_FUNCTIONS)  Output += SY1000_assigns[number].Title;
  else {
    number -= SY1000_NUMBER_OF_CTL_FUNCTIONS;
    if (number < SY1000_NUMBER_OF_SCENE_ASSIGNS)  Output += "SCENE ASGN " + String(number + 1);
    else Output += "--";
  }
}

FLASHMEM void MD_SY1000_class::read_assign_short_name(uint8_t number, String & Output) {
  if (number < SY1000_NUMBER_OF_CTL_FUNCTIONS)  Output += SY1000_assigns[number].Title_short;
  else {
    number -= SY1000_NUMBER_OF_CTL_FUNCTIONS;
    if (number < SY1000_NUMBER_OF_SCENE_ASSIGNS)  Output += "ASG" + String(number + 1);
    else Output += "--";
  }
}

FLASHMEM void MD_SY1000_class::read_assign_trigger(uint8_t number, String & Output) {
  if (number < SY1000_NUMBER_OF_CTL_FUNCTIONS) Output = SY1000_assigns[number].Title;
  else if (number < 96) Output = "CC#" + String(number);
  else Output = "-";
}

FLASHMEM uint8_t MD_SY1000_class::get_number_of_assigns() {
  return SY1000_TOTAL_NUMBER_OF_ASSIGNS;
}

FLASHMEM uint8_t MD_SY1000_class::trigger_follow_assign(uint8_t number) {
  if (number < SY1000_NUMBER_OF_CTL_FUNCTIONS) return number;
  else return number + SY1000_FIRST_SCENE_ASSIGN_SOURCE_CC - SY1000_NUMBER_OF_CTL_FUNCTIONS; // Scene assign cc numbers are 21 and up
}

FLASHMEM void MD_SY1000_class::assign_press(uint8_t Sw, uint8_t value) { // Switch set to SY1000_ASSIGN is pressed
  uint8_t asgn = SP[Sw].Assign_number;
  uint16_t target;
  if (asgn < SY1000_NUMBER_OF_CTL_FUNCTIONS) {
    if (SP[Sw].PP_number == NOT_FOUND) {
      LCD_show_popup_label("Cannot control", ACTION_TIMER_LENGTH);
      return;
    }
    if (SP[Sw].Assign_on) {
      // Execute this command
      uint16_t number = SP[Sw].PP_number;
      uint8_t val = (SP[Sw].State == 1 ? 1 : 0);
      if (SY1000_parameters[number].Address_GM != VIA_ASSIGN) {
        write_parameter_value(number, val);
      }
      else {
        if (!bass_mode) target = SY1000_parameters[number].Target_GM;
        else target = SY1000_parameters[number].Target_BM;
        if (SY1000_parameters[number].Type != DO_TAP_TEMPO) {
          set_assign_settings(15, target, SP[Sw].Latch, SY1000_SOURCE_CC95);
        }
        else {
          SCO_global_tap_tempo_press(Sw);
          update_page = REFRESH_PAGE;
          return;
        }
      }
      SP[Sw].Offline_value = val;
    }
    change_active_assign_sources(SY1000_assigns[asgn].Assign_target_number, SY1000_SOURCE_CC95);
    delay(20);
    MIDI_send_CC(95, 127, MIDI_channel, MIDI_out_port);
    assign_return_target_timer = millis() + SY1000_ASSIGN_RETURN_TIME;
  }
  else {
    toggle_scene_assign(asgn - SY1000_NUMBER_OF_CTL_FUNCTIONS);
  }

  if (SP[Sw].Assign_on) {
    uint8_t value = 0;
    if (SP[Sw].State == 1) value = SP[Sw].Assign_max;
    else value = SP[Sw].Assign_min;
    check_update_label(Sw, value);
    LCD_show_popup_label(SP[Sw].Label, ACTION_TIMER_LENGTH);
  }

  if (SP[Sw].Assign_on) update_page = REFRESH_PAGE; // To update the other switch states, we re-load the current page
}

FLASHMEM void MD_SY1000_class::assign_release(uint8_t Sw) { // Switch set to SY1000_ASSIGN is released
  uint8_t asgn = SP[Sw].Assign_number;
  if (asgn < SY1000_NUMBER_OF_CTL_FUNCTIONS) {
    // Execute assigns
    MIDI_send_CC(95, 0, MIDI_channel, MIDI_out_port);
    delay(20);

    if (SP[Sw].Assign_on) {
      // Execute this command
      uint16_t number = SP[Sw].PP_number;
      if (SY1000_parameters[number].Address_GM == VIA_ASSIGN) {
        restore_assign_settings(15);
      }
      SP[Sw].Offline_value = 0;
    }

    assign_return_target = SY1000_assigns[asgn].Assign_target_number;
  }

  // Update status
  if (SP[Sw].Latch == MOMENTARY) {
    if (SP[Sw].Assign_on) {
      SP[Sw].State = 2; // Switch state off
      uint16_t number = SP[Sw].PP_number;
      if (SY1000_parameters[number].Address_GM != VIA_ASSIGN) {
        write_parameter_value(number, 0);
      }
      check_update_label(Sw, SP[Sw].Assign_min);
      LCD_show_popup_label(SP[Sw].Label, ACTION_TIMER_LENGTH);
    }
    else SP[Sw].State = 0; // Assign off, so LED should be off as well

    if (SP[Sw].Assign_on) update_page = REFRESH_PAGE; // To update the other switch states, we re-load the current page
  }
}

void MD_SY1000_class::check_delayed_release_assignments() {
  if (assign_return_target > 0) {
    if (millis() > assign_return_target_timer) {
      change_active_assign_sources(assign_return_target, assign_return_target);
      assign_return_target = 0;
    }
  }
}

FLASHMEM void MD_SY1000_class::assign_load(uint8_t sw, uint8_t assign_number, uint8_t cc_number) { // Switch set to SY1000_ASSIGN is loaded in SP array
  SP[sw].Trigger = cc_number; //Save the cc_number in the Trigger variable
  SP[sw].Assign_number = assign_number;
}

FLASHMEM void MD_SY1000_class::request_current_assign(uint8_t sw) {
  uint8_t index = SP[sw].Assign_number;
  if (index < SY1000_NUMBER_OF_CTL_FUNCTIONS) {
    DEBUGMSG("Request assign " + String(index + 1));
    uint32_t my_address = calculate_assign_address(index);
    last_requested_sysex_address = my_address;
    last_requested_sysex_type = REQUEST_ASSIGN_TYPE;
    last_requested_sysex_switch = sw;
    request_sysex(my_address, 2);
  }
  else { // Scene assign
    index -= SY1000_NUMBER_OF_CTL_FUNCTIONS;
    SP[sw].Assign_on = true;
    SP[sw].Latch = TOGGLE;
    SP[sw].Assign_max = 1;
    SP[sw].Assign_min = 0;
    SP[sw].PP_number = NOT_FOUND;
    SP[sw].Colour = 2; // Red
    if (scene_assign_state[index]) SP[sw].State = 1;
    else SP[sw].State = 0;
    String msg = "CC #" + String(index + SY1000_FIRST_SCENE_ASSIGN_SOURCE_CC);
    uint8_t asgn = check_for_scene_assign_source(index + SY1000_FIRST_SCENE_ASSIGN_SOURCE_CC);
    if (asgn > 0) {
      msg += " (ASGN " + String(asgn) + ')';
      if (SP[sw].State == 0) SP[sw].State = 2;
    }
    LCD_set_SP_label(sw, msg);
    PAGE_request_next_switch(); // Wrong assign number given in Config - skip it
  }
}

FLASHMEM void MD_SY1000_class::read_current_assign(uint8_t sw, uint32_t address, const unsigned char* sxdata, short unsigned int sxlength) {
  bool found;
  String msg;
  uint8_t ctl_function_number;
  uint8_t data3 = sxdata[3];
  uint8_t asgn = SP[sw].Assign_number;

  if (asgn < SY1000_NUMBER_OF_CTL_FUNCTIONS) { // CTL assign
    ctl_function_number = sxdata[sx_index(data3, 12)];
    if ((SY1000_assigns[asgn].Type == SY1000_NUM_TYPE) && (ctl_function_number > 1)) ctl_function_number++;
    if ((SY1000_assigns[asgn].Type == SY1000_MAN_TYPE) && (ctl_function_number > 0)) ctl_function_number += 4;
    bool ctl_function_on = (ctl_function_number > 0);

    if (ctl_function_on) {
      SP[sw].Assign_on = true;
      SP[sw].Latch = (sxdata[sx_index(data3, 13)] == 0 ? TOGGLE : MOMENTARY);
      SP[sw].Assign_max = 1;
      SP[sw].Assign_min = 0;

      // Request the target
      found = ctl_target_lookup(sw, ctl_function_number); // Lookup the address of the target in the SY1000_Parameters array
      DEBUGMSG("Request target of CTL" + String(asgn + 1) + ':' + String(SP[sw].Address, HEX));
      if (found) {
        uint32_t my_address = SP[sw].Address;
        if (my_address != VIA_ASSIGN) {
          last_requested_sysex_address = my_address;
          last_requested_sysex_type = REQUEST_PARAMETER_TYPE;
          //read_assign_target = true;
          request_sysex(my_address, 2);
        }
        else {
          read_parameter(sw, SP[sw].Offline_value, 0);
          PAGE_request_next_switch();
        }
      }
      else { // parameter not in list, so it cannot be controlled
        SP[sw].Assign_on = false;
        SP[sw].PP_number = NOT_FOUND;
        SP[sw].Colour = FX_DEFAULT_TYPE;
        SP[sw].State = 0; // Switch the stompbox off
        SP[sw].Latch = MOMENTARY; // Make it momentary
        SP[sw].Colour = FX_DEFAULT_TYPE; // Set the on colour to default
        // Set the Label
        msg = '(';
        msg += SY1000_sublists[SY1000_CTL_TYPE_SUBLIST + ctl_function_number];
        msg += ')';
        LCD_set_SP_label(sw, msg);
        PAGE_request_next_switch();
      }
    }
    else { // Control is switched off.
      SP[sw].Assign_on = false; // Switch the pedal off
      SP[sw].PP_number = 0; // To make sure its value is not NOT_FOUND, which gives an occasional "cannot control" message
      SP[sw].State = 0; // Switch the stompbox off
      SP[sw].Latch = MOMENTARY; // Make it momentary
      SP[sw].Colour = FX_DEFAULT_TYPE; // Set the on colour to default
      // Set the Label
      msg = "--";
      LCD_set_SP_label(sw, msg);
      PAGE_request_next_switch();
    }
  }
}

FLASHMEM bool MD_SY1000_class::ctl_target_lookup(uint8_t sw, uint16_t target) { // Finds the target and its address in the SY1000_parameters table

  // Lookup in SY1000_parameter array
  bool found = false;
  for (uint16_t i = 0; i < SY1000_NUMBER_OF_PARAMETERS; i++) {
    if (target == SY1000_parameters[i].Ctl_target) { //Check is we've found the right target
      SP[sw].PP_number = i; // Save the index number
      SP[sw].Address = read_parameter_address(i);
      found = true;
    }
  }
  return found;
}

FLASHMEM bool MD_SY1000_class::target_lookup(uint8_t sw, uint16_t target) { // Finds the target and its address in the SY1000_parameters table

  // Lookup in SY1000_parameter array
  bool found = false;
  if (!bass_mode) {
    for (uint16_t i = 0; i < SY1000_NUMBER_OF_PARAMETERS; i++) {
      if (target == SY1000_parameters[i].Target_GM) { //Check is we've found the right target
        SP[sw].PP_number = i; // Save the index number
        SP[sw].Address = read_parameter_address(i);
        found = true;
      }
    }
  }
  else {
    for (uint16_t i = 0; i < SY1000_NUMBER_OF_PARAMETERS; i++) {
      if (target == SY1000_parameters[i].Target_BM) { //Check is we've found the right target
        SP[sw].PP_number = i; // Save the index number
        SP[sw].Address = read_parameter_address(i);
        found = true;
      }
    }
  }
  return found;
}

FLASHMEM void MD_SY1000_class::request_full_assign(uint8_t number) {
  if (number >= SY1000_NUMBER_OF_ASSIGNS) return;
  DEBUGMSG("Requesting assign " + String(number));
  uint32_t my_address = calculate_full_assign_address(number);
  read_full_assign_number = number;
  request_sysex(my_address, SY1000_NUMBER_OF_ASSIGN_BYTES_READ);
}

FLASHMEM void MD_SY1000_class::read_full_assign(uint8_t number, uint32_t address, const unsigned char* sxdata, short unsigned int sxlength) {
  if (number >= SY1000_NUMBER_OF_ASSIGNS) return;
  DEBUGMSG("Reading assign " + String(number));
  for (uint8_t i = 0; i < sizeof(assign_area[0]); i++) {
    assign_area[number][i] = sxdata[i + 12];
  }
  number++;
  if (number < SY1000_NUMBER_OF_ASSIGNS) request_full_assign(number);
  if (number == 2) auto_return_switch_mode();
}

FLASHMEM void MD_SY1000_class::change_active_assign_sources(uint8_t from_value, uint8_t to_value) {
  for (uint8_t number = 0; number < SY1000_NUMBER_OF_ASSIGNS; number++) {
    DEBUGMSG("Checking assign " + String(number) + ", byte[0]:" + String(assign_area[number][0]) + ", byte[13]:" + String(assign_area[number][13]));
    if ((assign_area[number][0] == 1) && (assign_area[number][13] == from_value)) {
      control_edit_mode();
      uint32_t my_address = calculate_full_assign_address(number);
      uint16_t sum = 0;
      uint8_t *ad = (uint8_t*)&my_address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
      uint8_t sysexmessage[57] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x00, 0x69, 0x12, ad[3], ad[2], ad[1], ad[0]};
      for (uint8_t i = 0; i < sizeof(assign_area[0]); i++) {
        sysexmessage[i + 12] = assign_area[number][i];
      }
      sysexmessage[25] = to_value;
      for (uint8_t i = 8; i < 55; i++) sum += sysexmessage[i];
      sysexmessage[55] = calc_Roland_checksum(sum);
      sysexmessage[56] = 0xF7;
      check_sysex_delay();
      MIDI_send_sysex(sysexmessage, 57, MIDI_out_port);
    }
  }
}

FLASHMEM void MD_SY1000_class::set_assign_settings(uint8_t assign, uint16_t target, uint8_t mode, uint8_t trigger) {
  DEBUGMSG("Set assign " + String(assign) + "to target " + String(target));
  control_edit_mode();
  uint32_t my_address = calculate_full_assign_address(assign);
  uint16_t sum = 0;
  uint8_t *ad = (uint8_t*)&my_address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t sysexmessage[57] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x00, 0x69, 0x12, ad[3], ad[2], ad[1], ad[0], 0x01, 0x00, 0x03, 0x0e, 0x07, 0x08, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x01, SY1000_SOURCE_CC95,
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0f, 0x0f, 0x0f, 0x00, 0x00, 0x14, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0f, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0xf7
                             };
  sysexmessage[13] = (target & 0xF000) >> 12;
  sysexmessage[14] = (target & 0x0F00) >> 8;
  sysexmessage[15] = (target & 0x00F0) >> 4;
  sysexmessage[16] = (target & 0x000F);
  sysexmessage[25] = trigger;
  sysexmessage[26] = (mode == TOGGLE ? 0x00 : 0x01);
  for (uint8_t i = 8; i < 55; i++) sum += sysexmessage[i];
  sysexmessage[55] = calc_Roland_checksum(sum);
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 57, MIDI_out_port);
}

FLASHMEM void MD_SY1000_class::restore_assign_settings(uint8_t assign) {
  DEBUGMSG("Restore assign " + String(assign));
  control_edit_mode();
  uint32_t my_address = calculate_full_assign_address(assign);
  uint16_t sum = 0;
  uint8_t *ad = (uint8_t*)&my_address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t sysexmessage[57] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x00, 0x69, 0x12, ad[3], ad[2], ad[1], ad[0]};
  for (uint8_t i = 0; i < sizeof(assign_area[0]); i++) {
    sysexmessage[i + 12] = assign_area[assign][i];
  }
  for (uint8_t i = 8; i < 55; i++) sum += sysexmessage[i];
  sysexmessage[55] = calc_Roland_checksum(sum);
  sysexmessage[56] = 0xF7;
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 57, MIDI_out_port);
}

// f0 41 10 00 00 00 69 12 10 00 05 00 01 00 02 0a 03 08 00 05 00 08 00 00 00 29 01 00 02 08 00 00 03 00 00 0a 0a 14 01 00 01 00 00 00 00 03 0f 0f 0f 00 00 00 00 00 00 35 f7
// f0 41 10 00 00 00 69 12 10 00 05 40 01 00 02 0a 04 08 00 06 04 08 00 00 00 29 01 00 02 00 00 00 02 08 00 00 00 14 01 00 01 00 00 00 00 03 0f 0f 0f 00 00 00 00 00 00 04 f7
// f0 41 10 00 00 00 69 12 10 00 06 00 01 00 02 0a 05 08 00 06 04 08 00 00 00 29 01 00 01 08 00 00 02 00 00 00 00 14 01 00 01 00 00 00 00 03 0f 0f 0f 00 00 00 00 00 00 43 f7
// f0 41 10 00 00 00 69 12 10 00 06 40 01 00 02 0a 06 08 00 06 04 08 00 00 00 29 01 00 01 00 00 00 01 08 00 00 00 14 01 00 01 00 00 00 00 03 0f 0f 0f 00 00 00 00 00 00 03 f7
// f0 41 10 00 00 00 69 12 10 00 07 00 01 00 02 0a 07 08 00 06 04 08 00 00 00 29 01 00 00 08 00 00 01 00 00 00 00 14 01 00 01 00 00 00 00 03 0f 0f 0f 00 00 00 00 00 00 42 f7

const PROGMEM uint8_t bass_mode_assigns[5][43] = {
  { 0x01, 0x00, 0x02, 0x0a, 0x03, 0x08, 0x00, 0x05, 0x00, 0x08, 0x00, 0x00, 0x00, 0x29, 0x01, 0x00, 0x02, 0x08, 0x00, 0x00, 0x03, 0x00, 0x00, 0x0a, 0x0a, 0x14, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0f, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
  { 0x01, 0x00, 0x02, 0x0a, 0x04, 0x08, 0x00, 0x06, 0x04, 0x08, 0x00, 0x00, 0x00, 0x29, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x08, 0x00, 0x00, 0x00, 0x14, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0f, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
  { 0x01, 0x00, 0x02, 0x0a, 0x05, 0x08, 0x00, 0x06, 0x04, 0x08, 0x00, 0x00, 0x00, 0x29, 0x01, 0x00, 0x01, 0x08, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x14, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0f, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
  { 0x01, 0x00, 0x02, 0x0a, 0x06, 0x08, 0x00, 0x06, 0x04, 0x08, 0x00, 0x00, 0x00, 0x29, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x00, 0x00, 0x00, 0x14, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0f, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
  { 0x01, 0x00, 0x02, 0x0a, 0x07, 0x08, 0x00, 0x06, 0x04, 0x08, 0x00, 0x00, 0x00, 0x29, 0x01, 0x00, 0x00, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x14, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0f, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
};

FLASHMEM void MD_SY1000_class::add_bass_string_assigns() {
  uint8_t item = 0;
  for (uint8_t a = 0; a < 16; a++) {
    if (assign_area[a][0] == 0) { // Assign off
      for (uint8_t i = 0; i < sizeof(assign_area[0]); i++) {
        assign_area[a][i] = bass_mode_assigns[item][i];
      }
      restore_assign_settings(a);
      item++;
      if (item > 5) return;
    }
  }
}

FLASHMEM uint8_t MD_SY1000_class::check_for_scene_assign_source(uint8_t cc) {
  uint8_t my_source = 255;
  if ((cc >= 1) && (cc <= 31)) my_source = cc + 26; // Trigger is cc01 - cc31 (source for CC#01 is 27)
  else if ((cc >= 64) && (cc <= 95)) my_source = cc - 6; // Trigger is cc64 - cc95 (source for CC#64 is 58)
  for (uint8_t a = 0; a < SY1000_NUMBER_OF_ASSIGNS; a++) { // Check if assign is on and my_source is correct
    if ((assign_area[a][0] == 0x01) && (assign_area[a][13] == my_source)) return a + 1;
  }
  return 0;
}

FLASHMEM uint32_t MD_SY1000_class::calculate_full_assign_address(uint8_t number) {
  uint32_t addr = 0x10000400 + ((number / 2) * 0x100) + ((number % 2) * 0x40);
  if (bass_mode) addr += 0x20000;
  return addr;
}

// Scene assigns - assigns used in scenes

FLASHMEM void MD_SY1000_class::initialize_scene_assigns() {
  for (uint8_t sa = 0; sa < SY1000_NUMBER_OF_SCENE_ASSIGNS; sa++) scene_assign_state[sa] = false;
}

FLASHMEM void MD_SY1000_class::toggle_scene_assign(uint8_t number) {
  if (number >= SY1000_NUMBER_OF_SCENE_ASSIGNS) return;
  uint8_t cc = number + SY1000_FIRST_SCENE_ASSIGN_SOURCE_CC;
  scene_assign_state[number] ^= 1; // Toggle state

  if (scene_assign_state[number]) MIDI_send_CC(cc, 127, MIDI_channel, MIDI_out_port);
  else MIDI_send_CC(cc, 0, MIDI_channel, MIDI_out_port);

  show_scene_assign_LEDs();
  String msg = "CC #" + String(cc);
  uint8_t asgn = check_for_scene_assign_source(cc);
  if (asgn > 0) msg += " (ASGN " + String(asgn) + ')';
  LCD_show_popup_label(msg, ACTION_TIMER_LENGTH);
}

FLASHMEM void MD_SY1000_class::set_scene_assign_states(uint8_t my_byte) {
  for (uint8_t sa = 0; sa < SY1000_NUMBER_OF_SCENE_ASSIGNS; sa++) {
    scene_assign_state[sa] = bitRead(my_byte, sa);
  }
}

FLASHMEM uint8_t MD_SY1000_class::read_scene_assign_state() {
  uint8_t my_byte = 0;
  for (uint8_t sa = 0; sa < SY1000_NUMBER_OF_SCENE_ASSIGNS; sa++) {
    bitWrite(my_byte, sa, scene_assign_state[sa]);
  }
  return my_byte;
}

FLASHMEM void MD_SY1000_class::check_scene_assigns_with_new_state(uint8_t new_byte) {
  for (uint8_t sa = 0; sa < SY1000_NUMBER_OF_SCENE_ASSIGNS; sa++) {
    bool new_state = bitRead(new_byte, sa);
    if (scene_assign_state[sa] != new_state) toggle_scene_assign(sa);
  }
}

FLASHMEM void MD_SY1000_class::show_scene_assign_LEDs() {
  if (switch_mode != MODE_SCENE_ASSIGN) return;
  for (uint8_t sa = 0; sa < SY1000_NUMBER_OF_SCENE_ASSIGNS; sa++) {
    if (scene_assign_state[sa] == 1) set_LED_colour(sa, SY1000_SCENE_ASSIGN_COLOUR);
    else set_LED_colour(sa, 0);
  }
}

// ********************************* Section 7: SY1000 Scene control *******************************************************

// Uses SY1000_scene_buffer of 192 bytes - same size as a Katana patch
// byte   0 -   7: Common data
// byte   8 -  30: Scene 1 - 23 bytes per scene
// byte  31 - 53: Scene 2
// byte  54 - 76: Scene 3
// byte  77 -  99: Scene 4
// byte 100 - 122: Scene 5
// byte 123 - 145: Scene 6
// byte 146 - 168: Scene 7
// byte 169 - 191: Scene 8

// Structure of common data:
// Byte 0: Patch type
// Byte 1, 2: Patch number
// Byte 3, 4: Patch checksum
// Byte 5: Current scene of patch
// Byte 6: Bitwise scene active

#define SY1000_PATCH_TYPE 0
#define SY1000_PATCH_NUMBER_MSB 1
#define SY1000_PATCH_NUMBER_LSB 2
#define SY1000_CURRENT_SCENE_BYTE 5
#define SY1000_SCENE_ACTIVE_BYTE 6
#define SY1000_MUTE_DURING_SCENE_CHANGE_BYTE 7

// Structure of scene data
// Scene byte 0 - 7: Store common data
// Scene byte 8 - 9: INST1 type dependant data
// Scene byte 10 - 11: INST2 type dependant data
// Scene byte 12 - 13: INST3 type dependant data
// Scene byte 14: Scene assign states
// Scene byte 15 - 22: Scene name

#define SY1000_SCENE_SIZE 23
#define SY1000_COMMON_DATA_SIZE 8

#define SY1000_INST_DATA_OFFSET 8
#define SY1000_SCENE_ASSIGN_BYTE 14
#define SY1000_SCENE_NAME_BYTE 15

struct SY1000_scene_parameter_struct { // Combines all the data we need for controlling a parameter in a device
  uint16_t Address_GM;
  uint16_t Address_BM;
  uint8_t Bit_address;
  uint8_t Bit_mask;
};

const PROGMEM SY1000_scene_parameter_struct SY1000_scene_parameters[] {
  { 0x1500, 0x1500, 0x00, B00000001 }, // INST1 sw
  { 0x1501, 0x1501, 0x01, B00000111 }, // INST1 type
  { 0x1503, 0x1503, 0x04, B00000001 }, // INST1 normal pu
  { 0x1F00, 0x1E00, 0x05, B00000001 }, // INST1 alt tune
  { 0x1F0E, 0x1E0E, 0x06, B00000001 }, // INST1 12 string
  { 0x1F28, 0x1E28, 0x07, B00000001 }, // INST1 bend
  { 0x2000, 0x1F00, 0x10, B00000001 }, // INST2 sw
  { 0x2001, 0x1F01, 0x11, B00000111 }, // INST2 type
  { 0x2003, 0x1F03, 0x14, B00000001 }, // INST2 normal pu
  { 0x2A00, 0x2800, 0x15, B00000001 }, // INST2 alt tune
  { 0x2A0E, 0x280E, 0x16, B00000001 }, // INST2 12 string
  { 0x2A28, 0x2828, 0x17, B00000001 }, // INST2 bend
  { 0x2B00, 0x2900, 0x20, B00000001 }, // INST3 sw
  { 0x2B01, 0x2901, 0x21, B00000111 }, // INST3 type
  { 0x2B03, 0x2903, 0x24, B00000001 }, // INST3 normal pu
  { 0x3500, 0x3200, 0x25, B00000001 }, // INST3 alt tune
  { 0x350E, 0x320E, 0x26, B00000001 }, // INST3 12 string
  { 0x3528, 0x3228, 0x27, B00000001 }, // INST3 bend
  { 0x1201, 0x1201, 0x30, B00000001 }, // Normal PU
  { 0x3600, 0x3300, 0x31, B00000001 }, // Comp SW
  { 0x3700, 0x3400, 0x32, B00000001 }, // DIST SW
  { 0x3707, 0x3407, 0x33, B00000001 }, // DS SOLO
  { 0x3800, 0x3500, 0x34, B00000001 }, // Amp
  { 0x380C, 0x350C, 0x35, B00000001 }, // Amp SOLO
  { 0x380B, 0x350B, 0x36, B00000011 }, // Amp GAIN SW
  { 0x380A, 0x350A, 0x40, B00000011 }, // Amp BRIGHT SW
  { 0x1221, 0x1221, 0x41, B00000001 }, // Div CH sel
  { 0x3F00, 0x3C00, 0x42, B00000001 }, // Chorus
  { 0x3900, 0x3600, 0x43, B00000001 }, // NS
  { 0x3A00, 0x3700, 0x44, B00000001 }, // EQ1
  { 0x3B00, 0x3800, 0x45, B00000001 }, // EQ2
  { 0x122E, 0x122E, 0x46, B00000001 }, // S/R loop
  { 0x3C00, 0x3900, 0x47, B00000001 }, // Delay 1
  { 0x4000, 0x3D00, 0x50, B00000001 }, // FX1
  { 0x4001, 0x3D01, 0x51, B00111111 }, // FX1 Type
  { 0x3D00, 0x3A00, 0x57, B00000001 }, // Delay 2
  { 0x6600, 0x6300, 0x60, B00000001 }, // FX2
  { 0x6601, 0x6301, 0x61, B00111111 }, // FX2 Type
  { 0x3E00, 0x3B00, 0x67, B00000001 }, // MST Delay
  { 0x8C00, 0x8900, 0x70, B00000001 }, // FX3
  { 0x8C01, 0x8901, 0x71, B00111111 }, // FX3 Type
  { 0xB200, 0xAF00, 0x77, B00000001 }, // Reverb
};

#define INST1_SW_ITEM 0
#define INST1_TYPE_ITEM 1
#define INST2_SW_ITEM 6
#define INST2_TYPE_ITEM 7
#define INST3_SW_ITEM 12
#define INST3_TYPE_ITEM 13
#define NORM_GTR_ITEM 18


const uint16_t SY1000_NUMBER_OF_SCENE_PARAMETERS = sizeof(SY1000_scene_parameters) / sizeof(SY1000_scene_parameters[0]);

struct SY1000_scene_inst_parameter_struct { // Combines all the data we need for controlling a parameter in a device
  uint16_t Address;
  uint8_t Bit_address;
  uint8_t Bit_mask;
};

#define SY1000_NUMBER_OF_INST_PARAMETERS 10

const PROGMEM SY1000_scene_inst_parameter_struct SY1000_scene_inst_parameters[][SY1000_NUMBER_OF_INST_PARAMETERS] {
  { // Dynasynth (0)
    { 0x0104, 0x00, B00001111 }, // DS Waveform (9 values)
    { 0x011C, 0x04, B00000001 }, // DS Filter on/off
    { 0x0127, 0x05, B00000001 }, // DS LFO1
    { 0x0131, 0x06, B00000001 }, // DS LFO2
    { 0x021F, 0x07, B00000001 }, // DS SEQ1
    { 0x0235, 0x10, B00000001 }, // DS SEQ2
    { 0x0223, 0x11, B00000001 }, // DS TURBO1
    { 0x0239, 0x12, B00000001 }, // DS TURBO1
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
  },
  { // OSCsynth (1)
    { 0x0304, 0x00, B00000011 }, // OSC Mode
    { 0x0304, 0x02, B00000111 }, // OSC Wav1
    { 0x0304, 0x05, B00000111 }, // OSC Wav2
    { 0x0304, 0x10, B00000001 }, // OSC MONO/poly
    { 0x0304, 0x11, B00000001 }, // OSC Porta
    { 0x0304, 0x12, B00000001 }, // OSC LFO1
    { 0x0304, 0x13, B00000001 }, // OSC LFO2
    { 0x0304, 0x14, B00000001 }, // OSC SYNC
    { 0x0304, 0x15, B00000011 }, // OSC HOLD MODE
    { 0x0304, 0x17, B00000001 }, // OSC Filter
  },
  { // GR synth (2)
    { 0x0400, 0x00, B00000011 }, // GR MODE
    { 0x0401, 0x02, B00000001 }, // GR COMP SW
    { 0x0404, 0x03, B00000011 }, // GR ENV MOD
    { 0x0407, 0x05, B00000011 }, // GR PITCH SW
    { 0x040C, 0x07, B00000001 }, // GR DUET
    { 0x040D, 0x10, B00000001 }, // GR SWEEP
    { 0x0410, 0x11, B00000001 }, // GR VIBRATO
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
  },
  { // E.GTR (3)
    { 0x0500, 0x00, B00001111 }, // E.GTR Type
    { 0x0501, 0x04, B00000111 }, // E.GTR PU select
    { 0x050A, 0x07, B00000001 }, // E.GTR amp sw
    { 0x0515, 0x10, B00000011 }, // E.GTR gain sw
    { 0x0516, 0x12, B00000001 }, // E.GTR solo
    { 0x0514, 0x13, B00000001 }, // E.GTR bright
    { 0x051E, 0x14, B00000001 }, // E.GTR ns
    { 0x0521, 0x15, B00000001 }, // E.GTR eq
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },

  },
  { // Acoustic GTR (4)
    { 0x0600, 0x00, B00001111 }, // A.GTR Type
    { 0x0603, 0x04, B00000011 }, // A.GTR PU sel
    { 0x0617, 0x06, B00000011 }, // A.GTR gain sw
    { 0x0616, 0x10, B00000001 }, // A.GTR bright
    { 0x0618, 0x11, B00000001 }, // A.GTR solo
    { 0x0623, 0x12, B00000001 }, // A.GTR eq
    { 0x0620, 0x13, B00000001 }, // A.GTR ns
    { 0x060C, 0x14, B00000001 }, // A.GTR Amp
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
  },
  { // E Bass (5)
    { 0x0700, 0x00, B00000011 }, // GM E.Bass Type
    { 0x0703, 0x02, B00000011 }, // GM E.Bass Tone type
    { 0x070B, 0x04, B00000001 }, // GM E.Bass amp sw
    { 0x0716, 0x05, B00000011 }, // GM E.Bass gain sw
    { 0x0717, 0x07, B00000001 }, // GM E.Bass solo
    { 0x0715, 0x10, B00000001 }, // GM E.Bass bright sw
    { 0x071F, 0x11, B00000001 }, // GM E.Bass ns
    { 0x0722, 0x12, B00000001 }, // GM E.Bass eq
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
  },
  { // VIO Guitar (6)
    { 0x0800, 0x00, B00001111 }, // VIO Type
    { 0x0801, 0x04, B00000111 }, // VIO PU sel
    { 0x0802, 0x10, B00000011 }, // VIO Tone type
    { 0x0819, 0x12, B00000001 }, // VIO eq
    { 0x0816, 0x13, B00000001 }, // VIO ns
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
  },
  { // POLY FX (7)
    { 0x0900, 0x00, B00000111 }, // POLYFX Type
    { 0x0906, 0x03, B00000001 }, // POLYFX Filter
    { 0x0907, 0x04, B00000001 }, // POLYFX polarity
    { 0x090C, 0x05, B00000011 }, // POLYFX Tone type
    { 0x090D, 0x07, B00000001 }, // POLYFX Comp
    { 0x0910, 0x10, B00000001 }, // POLYFX ns
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
  },
  { // Bass mode: E-Bass (8 - real value 3)
    { 0x0500, 0x00, B00001111 }, // BM E.Bass Type
    { 0x050A, 0x04, B00000011 }, // BM E.Bass PU select
    { 0x050B, 0x06, B00000001 }, // BM E.Bass Treble on
    { 0x050C, 0x07, B00000001 }, // BM E.Bass Bass on
    { 0x050D, 0x10, B00000001 }, // BM E.Bass Rhyth/solo
    { 0x050E, 0x11, B00000011 }, // BM E.Bass Tone type
    { 0x0516, 0x13, B00000001 }, // BM E.Bass amp sw
    { 0x0521, 0x14, B00000011 }, // BM E.Bass gain sw
    { 0x0522, 0x16, B00000001 }, // BM E.Bass solo
    { 0x052D, 0x17, B00000001 }, // BM E.Bass eq
  },
  { // Bass mode: Ac-Bass  (9 - real value 4)
    { 0x060B, 0x00, B00000001 }, // BM Ac.Bass amp sw
    { 0x0613, 0x01, B00000011 }, // BM Ac.Bass gain sw
    { 0x0614, 0x03, B00000001 }, // BM Ac.Bass solo
    { 0x0612, 0x04, B00000001 }, // BM Ac.Bass bright sw
    { 0x061C, 0x05, B00000001 }, // BM Ac.Bass ns
    { 0x061F, 0x06, B00000001 }, // BM Ac.Bass eq
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
  },
  { // Bass mode: E.GTR  (10 - real value 5)
    { 0x0700, 0x00, B00000011 }, // BM E.GTR Type
    { 0x0701, 0x04, B00000111 }, // BM E.GTR PU select
    { 0x070A, 0x07, B00000001 }, // BM E.GTR amp sw
    { 0x0715, 0x10, B00000011 }, // BM E.GTR gain sw
    { 0x0716, 0x12, B00000001 }, // BM E.GTR solo
    { 0x0714, 0x13, B00000001 }, // BM E.GTR bright
    { 0x071E, 0x14, B00000001 }, // BM E.GTR ns
    { 0x0721, 0x15, B00000001 }, // BM E.GTR eq
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
  },
  { // Bass mode: POLY FX  (11 - real value 6)
    { 0x0800, 0x00, B00000111 }, // BM POLYFX Type
    { 0x0806, 0x03, B00000001 }, // BM POLYFX Filter
    { 0x0807, 0x04, B00000001 }, // BM POLYFX polarity
    { 0x080C, 0x05, B00000011 }, // BM POLYFX Tone type
    { 0x080D, 0x07, B00000001 }, // BM POLYFX Comp
    { 0x0810, 0x10, B00000001 }, // BM POLYFX ns
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
  },
};

#define SY1000_NUMBER_OF_SCENE_INST_PARAMETERS (SY1000_NUMBER_OF_INST_PARAMETERS * 3)
#define SY1000_TOTAL_NUMBER_OF_SCENE_PARAMETERS SY1000_NUMBER_OF_SCENE_INST_PARAMETERS + SY1000_NUMBER_OF_SCENE_PARAMETERS

const uint32_t GM_inst_start_address[] = { 0x10001500, 0x10002000, 0x10002B00 };
const uint32_t BM_inst_start_address[] = { 0x10021500, 0x10021F00, 0x10022900 };

FLASHMEM uint32_t MD_SY1000_class::get_scene_inst_parameter_address(uint16_t number) {
  if (number >= SY1000_NUMBER_OF_SCENE_INST_PARAMETERS) return 0;
  uint8_t inst = number / SY1000_NUMBER_OF_INST_PARAMETERS;
  uint8_t par = number % SY1000_NUMBER_OF_INST_PARAMETERS;
  uint8_t inst_type = INST_type[inst];

  if (!bass_mode) {
    return GM_inst_start_address[inst] + SY1000_scene_inst_parameters[inst_type][par].Address;
  }
  else {
    if (inst_type > 2) inst_type += 5; // jump from inst 3 to inst 8 for Bass mode
    return BM_inst_start_address[inst] + SY1000_scene_inst_parameters[inst_type][par].Address;
  }
}

FLASHMEM uint32_t MD_SY1000_class::get_scene_parameter_address(uint16_t number) {
  uint32_t address;
  if (number >= SY1000_NUMBER_OF_SCENE_PARAMETERS) return 0;
  if (!bass_mode) { // Guitar mode
    address = SY1000_scene_parameters[number].Address_GM;
    if (address & 0x8000) address = (address & 0x7FFF) | 0x10000;
    address += 0x10000000;
  }
  else { // Bass mode
    address = SY1000_scene_parameters[number].Address_BM;
    if (address & 0x8000) address = (address & 0x7FFF) | 0x10000;
    address += 0x10020000;
  }
  return address;
}


FLASHMEM void MD_SY1000_class::get_snapscene_title(uint8_t number, String & Output) {
  Output += "SCENE " + String(number);
}

FLASHMEM void MD_SY1000_class::get_snapscene_label(uint8_t number, String & Output) {
  read_scene_name_from_buffer(number);
  Output += scene_label_buffer;
}

FLASHMEM bool MD_SY1000_class::request_snapscene_name(uint8_t sw, uint8_t sw1, uint8_t sw2, uint8_t sw3) {
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

FLASHMEM void MD_SY1000_class::set_snapscene(uint8_t sw, uint8_t number) {
  if (!is_on) unmute();
  if ((number < 1) || (number > 8)) return;
  bool loaded = load_scene(last_loaded_scene, number);
  set_snapscene_number_and_LED(number);
  if ((loaded) && (sw > 0)) {
    read_scene_name_from_buffer(number);
    String msg = "Scene " + String(number) + ':' + scene_label_buffer;
    LCD_show_popup_label(msg, MESSAGE_TIMER_LENGTH);
  }
  MIDI_send_current_snapscene(my_device_number, current_snapscene);
  update_main_lcd = true;
}

FLASHMEM void MD_SY1000_class::show_snapscene(uint8_t  number) {
  if ((number < 1) || (number > 8)) return;
  if (number == current_snapscene) return;
  set_snapscene_number_and_LED(number);
  update_main_lcd = true;
  DEBUGMSG("SY1000 snapscene change to scene " + String(number));
}

FLASHMEM void MD_SY1000_class::set_snapscene_number_and_LED(uint8_t number) {
  current_snapscene = number;
  uint8_t last_led = 8;
  uint8_t first_switch = 0;
  bool second_scene_active = false;
  if (switch_mode == MODE_SCENE_TOP_ROW) {
    last_led = 4;
    first_switch = 4;
    if (current_snapscene > 4) second_scene_active = true;
  }
  else if (switch_mode == MODE_SCENE_BOTTOM_ROW) {
    last_led = 4;
    if (current_snapscene > 4) second_scene_active = true;
  }
  else if (switch_mode != MODE_SCENE) return;

  for (uint8_t s = 0; s < last_led; s++) {
    if ((s + 1) == current_snapscene) {
      set_LED_colour(s + first_switch, SY1000_SCENE_SELECTED_COLOUR);
    }
    else if ((second_scene_active) && ((s + 5) == current_snapscene)) {
      set_LED_colour(s + first_switch, SY1000_SECOND_SCENE_SELECTED_COLOUR);
    }
    else {
      if (check_snapscene_active(s + 1)) set_LED_colour(s + first_switch, SY1000_ACTIVE_SCENE_COLOUR);
      else set_LED_colour(s + first_switch, 0);
    }
  }
}

FLASHMEM bool MD_SY1000_class::load_scene(uint8_t prev_scene, uint8_t new_scene) {
  uint8_t my_byte, my_shift, my_mask;
  MIDI_debug_sysex(SY1000_patch_buffer, VC_PATCH_SIZE, 255, true);
  bool load_all_data = false;
  if (prev_scene == new_scene) return false;
  if (!check_snapscene_active(new_scene)) return false;
  if (last_loaded_scene == 0) load_all_data = true;

  DEBUGMAIN("Loading scene " + String(new_scene) + " (prev scene:" + String(prev_scene) + ')');
  control_edit_mode();

  // Read inst state and types:
  DEBUGMAIN("Checking INST changes");
  INST_onoff[0] = read_scene_data(new_scene, INST1_SW_ITEM);
  INST_type[0] = read_scene_data(new_scene, INST1_TYPE_ITEM);
  INST_onoff[1] = read_scene_data(new_scene, INST2_SW_ITEM);
  INST_type[1] = read_scene_data(new_scene, INST2_TYPE_ITEM);
  INST_onoff[2] = read_scene_data(new_scene, INST3_SW_ITEM);
  INST_type[2] = read_scene_data(new_scene, INST3_TYPE_ITEM);
  normal_pu_mute = read_scene_data(new_scene, NORM_GTR_ITEM);
  uint8_t prev_inst_type[3];
  prev_inst_type[0] = read_scene_data(prev_scene, INST1_TYPE_ITEM);
  prev_inst_type[1] = read_scene_data(prev_scene, INST2_TYPE_ITEM);
  prev_inst_type[2] = read_scene_data(prev_scene, INST3_TYPE_ITEM);

  uint8_t index_new = get_scene_index(new_scene);
  uint8_t index_prev = get_scene_index(prev_scene);

  // Mute instruments
  mute_during_scene_change = check_mute_during_scene_change(new_scene);
  if (mute_during_scene_change) mute_now();

  uint8_t addr_index = 0;
  for (uint8_t inst = 0; inst < 3; inst ++) {
    DEBUGMAIN("Checking INST to turn down of off" + String(inst + 1));
    uint8_t inst_type_index = INST_type[inst];
    if ((bass_mode) && (inst_type_index > 2)) inst_type_index += 5; // jump from inst 3 to inst 8 for Bass mode
    bool inst_changed = (INST_type[inst] != prev_inst_type[inst]);
    for (uint8_t par = 0; par < SY1000_NUMBER_OF_INST_PARAMETERS; par++) {
      my_byte = (SY1000_scene_inst_parameters[inst_type_index][par].Bit_address >> 4) + (inst * 2) + SY1000_INST_DATA_OFFSET;
      my_shift = SY1000_scene_inst_parameters[inst_type_index][par].Bit_address & 0x0F;
      my_mask = SY1000_scene_inst_parameters[inst_type_index][par].Bit_mask << my_shift;

      if (my_mask != 0) {
        uint8_t new_data = SY1000_patch_buffer[index_new + my_byte] & my_mask;
        uint8_t old_data = SY1000_patch_buffer[index_prev + my_byte] & my_mask;
        if ((new_data < old_data) && (!inst_changed) && (!load_all_data)) {
          uint32_t address = get_scene_inst_parameter_address(addr_index);
          uint8_t data = new_data >> my_shift;
          write_sysex(address, data);
        }
      }
      addr_index++;
    }
  }

  DEBUGMAIN("Checking common parameters to turn down or off");
  for (uint8_t i = 0; i < SY1000_NUMBER_OF_SCENE_PARAMETERS; i++) {
    bool is_inst_sw = (i == INST1_SW_ITEM) || (i == INST2_SW_ITEM) || (i == INST3_SW_ITEM);
    if ((!mute_during_scene_change) || (!is_inst_sw)) {
      my_byte = SY1000_scene_parameters[i].Bit_address >> 4;
      my_shift = SY1000_scene_parameters[i].Bit_address & 0x0F;
      my_mask = SY1000_scene_parameters[i].Bit_mask << my_shift;

      if (my_mask != 0) {
        uint8_t new_data = SY1000_patch_buffer[index_new + my_byte] & my_mask;
        uint8_t old_data = SY1000_patch_buffer[index_prev + my_byte] & my_mask;
        if ((new_data < old_data)  && (!load_all_data)) {
          uint32_t address = get_scene_parameter_address(i);
          uint8_t data = new_data >> my_shift;
          write_sysex(address, data);
        }
      }
    }
    addr_index++;
  }

  DEBUGMAIN("Check scene assigns");
  check_scene_assigns_with_new_state(SY1000_patch_buffer[index_new + SY1000_SCENE_ASSIGN_BYTE]);
  last_loaded_scene = new_scene;

  addr_index = 0;
  for (uint8_t inst = 0; inst < 3; inst ++) {
    DEBUGMAIN("Checking INST to turn up or on" + String(inst + 1));
    uint8_t inst_type_index = INST_type[inst];
    if ((bass_mode) && (inst_type_index > 2)) inst_type_index += 5; // jump from inst 3 to inst 8 for Bass mode
    bool inst_changed = (INST_type[inst] != prev_inst_type[inst]);
    for (uint8_t par = 0; par < SY1000_NUMBER_OF_INST_PARAMETERS; par++) {
      my_byte = (SY1000_scene_inst_parameters[inst_type_index][par].Bit_address >> 4) + (inst * 2) + SY1000_INST_DATA_OFFSET;
      my_shift = SY1000_scene_inst_parameters[inst_type_index][par].Bit_address & 0x0F;
      my_mask = SY1000_scene_inst_parameters[inst_type_index][par].Bit_mask << my_shift;

      if (my_mask != 0) {
        uint8_t new_data = SY1000_patch_buffer[index_new + my_byte] & my_mask;
        uint8_t old_data = SY1000_patch_buffer[index_prev + my_byte] & my_mask;
        if ((new_data > old_data) || (inst_changed) || (load_all_data)) {
          uint32_t address = get_scene_inst_parameter_address(addr_index);
          uint8_t data = new_data >> my_shift;
          write_sysex(address, data);
        }
      }
      addr_index++;
    }
  }

  DEBUGMAIN("Checking common parameters to turn up or on");
  for (uint8_t i = 0; i < SY1000_NUMBER_OF_SCENE_PARAMETERS; i++) {
    bool is_inst_sw = (i == INST1_SW_ITEM) || (i == INST2_SW_ITEM) || (i == INST3_SW_ITEM);
    if ((!mute_during_scene_change) || (!is_inst_sw)) {
      my_byte = SY1000_scene_parameters[i].Bit_address >> 4;
      my_shift = SY1000_scene_parameters[i].Bit_address & 0x0F;
      my_mask = SY1000_scene_parameters[i].Bit_mask << my_shift;

      if (my_mask != 0) {
        uint8_t new_data = SY1000_patch_buffer[index_new + my_byte] & my_mask;
        uint8_t old_data = SY1000_patch_buffer[index_prev + my_byte] & my_mask;
        if ((new_data > old_data) || (load_all_data)) {
          uint32_t address = get_scene_parameter_address(i);
          uint8_t data = new_data >> my_shift;
          write_sysex(address, data);
        }
      }
    }
    addr_index++;
  }

  // Unmute instruments
  if (mute_during_scene_change) unmute();

  DEBUGMAIN("Done!");
  return true;
}

FLASHMEM uint8_t MD_SY1000_class::get_scene_index(uint8_t scene) {
  if (scene > 0) scene--;
  return (scene * SY1000_SCENE_SIZE) + SY1000_COMMON_DATA_SIZE;
}

FLASHMEM uint8_t MD_SY1000_class::read_scene_data(uint8_t scene, uint8_t parameter) {
  uint8_t index = get_scene_index(scene);
  uint8_t my_byte = SY1000_scene_parameters[parameter].Bit_address >> 4;
  uint8_t my_shift = SY1000_scene_parameters[parameter].Bit_address & 0x0F;
  uint8_t my_mask = SY1000_scene_parameters[parameter].Bit_mask;
  return (SY1000_patch_buffer[index + my_byte] >> my_shift) & my_mask;
}

FLASHMEM uint8_t MD_SY1000_class::read_temp_scene_data(uint8_t parameter) {
  uint8_t my_byte = SY1000_scene_parameters[parameter].Bit_address >> 4;
  uint8_t my_shift = SY1000_scene_parameters[parameter].Bit_address & 0x0F;
  uint8_t my_mask = SY1000_scene_parameters[parameter].Bit_mask;
  return (scene_data_buffer[my_byte] >> my_shift) & my_mask;
}

FLASHMEM void MD_SY1000_class::save_scene() {
  save_scene_number = current_snapscene;
  if (save_scene_number == 0) save_scene_number = 1;
  read_scene_name_from_buffer(save_scene_number);
  if ((scene_label_buffer[0] == '-') && (scene_label_buffer[1] == '-')) {
    scene_label_buffer[0] = '*';
    scene_label_buffer[1] = '*';
  }
  open_specific_menu = SY1000 + 1;
  SCO_select_page(PAGE_MENU); // Open the menu
}

FLASHMEM void MD_SY1000_class::store_scene() { // Returning from menu
  // Request the data from the SY1000
  store_scene_name_to_buffer(save_scene_number);
  set_snapscene_number_and_LED(save_scene_number);
  last_loaded_scene = save_scene_number;
  MIDI_disable_device_check();
  memset(scene_data_buffer, 0, SY1000_SCENE_DATA_BUFFER_SIZE);
  request_scene_message(1);
  update_page = 0;
}

FLASHMEM void MD_SY1000_class::update_change_on_all_scenes() {
  // Here we do the following steps:
  // 1) Read scene data to scene_data_buffer
  // 2) Compare the bytes of the scene_data_buffer to the last_loaded_scene - in check_delta_and_update_scenes()
  // 3) Write this difference to all scenes - in check_delta_and_update_scenes()
  save_scene_number = 9;
  memset(scene_data_buffer, 0, SY1000_SCENE_DATA_BUFFER_SIZE);
  request_scene_message(1);
  update_page = 0;
}

FLASHMEM void MD_SY1000_class::check_delta_and_update_scenes() {
  uint8_t my_byte, my_shift, my_mask;
  if (!check_snapscene_active(last_loaded_scene)) return;

  // Read inst state and types:
  uint8_t new_inst_type[3];
  DEBUGMAIN("Checking INST changes");
  new_inst_type[0] = read_temp_scene_data(INST1_TYPE_ITEM);
  new_inst_type[1] = read_temp_scene_data(INST2_TYPE_ITEM);
  new_inst_type[2] = read_temp_scene_data(INST3_TYPE_ITEM);

  uint8_t prev_inst_type[3];
  prev_inst_type[0] = read_scene_data(last_loaded_scene, INST1_TYPE_ITEM);
  prev_inst_type[1] = read_scene_data(last_loaded_scene, INST2_TYPE_ITEM);
  prev_inst_type[2] = read_scene_data(last_loaded_scene, INST3_TYPE_ITEM);

  uint8_t index_prev = get_scene_index(last_loaded_scene);

  uint8_t addr_index = 0;
  for (uint8_t inst = 0; inst < 3; inst ++) {
    DEBUGMAIN("Checking INST" + String(inst + 1));
    uint8_t inst_type_index = new_inst_type[inst];
    if ((bass_mode) && (inst_type_index > 2)) inst_type_index += 5; // jump from inst 3 to inst 8 for Bass mode
    bool inst_changed = (new_inst_type[inst] != prev_inst_type[inst]);
    for (uint8_t par = 0; par < SY1000_NUMBER_OF_INST_PARAMETERS; par++) {
      my_byte = (SY1000_scene_inst_parameters[inst_type_index][par].Bit_address >> 4) + (inst * 2) + SY1000_INST_DATA_OFFSET;
      my_shift = SY1000_scene_inst_parameters[inst_type_index][par].Bit_address & 0x0F;
      my_mask = SY1000_scene_inst_parameters[inst_type_index][par].Bit_mask << my_shift;

      if (my_mask != 0) {
        uint8_t new_data = scene_data_buffer[my_byte] & my_mask;
        uint8_t old_data = SY1000_patch_buffer[index_prev + my_byte] & my_mask;
        if (((new_data != old_data) && (new_inst_type[inst] == prev_inst_type[inst])) || (inst_changed)) {
          for (uint8_t s = 1; s <= 8; s++) { // Write to all active scenes
            if (check_snapscene_active(s)) {
              uint8_t index = get_scene_index(s);
              uint8_t data = SY1000_patch_buffer[index + my_byte] & ~my_mask;
              SY1000_patch_buffer[index + my_byte] = data | new_data;
            }
          }
        }
      }
      addr_index++;
    }
  }

  DEBUGMAIN("Checking common parameters");
  for (uint8_t i = 0; i < SY1000_NUMBER_OF_SCENE_PARAMETERS; i++) {

    my_byte = SY1000_scene_parameters[i].Bit_address >> 4;
    my_shift = SY1000_scene_parameters[i].Bit_address & 0x0F;
    my_mask = SY1000_scene_parameters[i].Bit_mask << my_shift;

    if (my_mask != 0) {
      uint8_t new_data = scene_data_buffer[my_byte] & my_mask;
      uint8_t old_data = SY1000_patch_buffer[index_prev + my_byte] & my_mask;
      if (new_data != old_data) {
        for (uint8_t s = 1; s <= 8; s++) { // Write to all active scenes
          if (check_snapscene_active(s)) {
            uint8_t index = get_scene_index(s);
            uint8_t data = SY1000_patch_buffer[index + my_byte] & ~my_mask;
            SY1000_patch_buffer[index + my_byte] = data | new_data;
          }
        }
      }
    }
    addr_index++;
  }

  /*DEBUGMAIN("Check scene assigns");
    check_scene_assigns_with_new_state(SY1000_patch_buffer[index_new + SY1000_SCENE_ASSIGN_BYTE]);
    last_loaded_scene = new_scene;
    DEBUGMAIN("Done!");*/
}

FLASHMEM void MD_SY1000_class::request_scene_message(uint8_t number) {
  if (number == 0) return;
  if (number == 2) LCD_show_popup_label("Reading scene...", MESSAGE_TIMER_LENGTH);
  read_scene_parameter_number = number;
  if (number < SY1000_NUMBER_OF_SCENE_INST_PARAMETERS) read_scene_parameter_address = get_scene_inst_parameter_address(number - 1);
  else read_scene_parameter_address = get_scene_parameter_address(number - SY1000_NUMBER_OF_SCENE_INST_PARAMETERS - 1);
  DEBUGMAIN("Requesting item " + String(number - 1) + " at address " + String(read_scene_parameter_address, HEX));
  request_sysex(read_scene_parameter_address, 1);
  read_scene_midi_timer = millis() + SY1000_READ_SCENE_MIDI_TIMER_LENGTH; // Set the timer
}

void MD_SY1000_class::check_read_scene_midi_timer() {
  if (read_scene_midi_timer > 0) { // Check timer is running
    if (millis() > read_scene_midi_timer) {
      DEBUGMSG("SY1000 Midi timer expired!");
      request_scene_message(read_scene_parameter_number);
    }
  }
}

FLASHMEM void MD_SY1000_class::read_scene_message(uint8_t number, uint8_t data) {
  uint8_t my_byte, my_shift, my_mask;
  if (number == 0) return;
  number--;
  if (number < SY1000_NUMBER_OF_SCENE_INST_PARAMETERS) {
    uint8_t inst = number / SY1000_NUMBER_OF_INST_PARAMETERS;
    uint8_t par = number % SY1000_NUMBER_OF_INST_PARAMETERS;
    uint8_t inst_type_index = INST_type[inst];
    if ((bass_mode) && (inst_type_index > 2)) inst_type_index += 5; // jump from inst 3 to inst 8 for Bass mode

    my_byte = (SY1000_scene_inst_parameters[inst_type_index][par].Bit_address >> 4) + (inst * 2) + SY1000_INST_DATA_OFFSET;
    my_shift = SY1000_scene_inst_parameters[inst_type_index][par].Bit_address & 0x0F;
    my_mask = SY1000_scene_inst_parameters[inst_type_index][par].Bit_mask;
    DEBUGMAIN("INST" + String(inst + 1) + ": item:" + String(par) + ", my_byte:" + String(my_byte));
  }
  else {
    uint8_t index = number - SY1000_NUMBER_OF_SCENE_INST_PARAMETERS;
    my_byte = SY1000_scene_parameters[index].Bit_address >> 4;
    my_shift = SY1000_scene_parameters[index].Bit_address & 0x0F;
    my_mask = SY1000_scene_parameters[index].Bit_mask;
    DEBUGMAIN("COMMON ITEM " + String(index) + ": my_byte:" + String(my_byte) + ", my_shift:" + String(my_shift));
  }
  if (my_mask != 0) scene_data_buffer[my_byte] |= ((data & my_mask) << my_shift);

  if (number < SY1000_TOTAL_NUMBER_OF_SCENE_PARAMETERS - 1) {
    request_scene_message(read_scene_parameter_number + 1); // Request next message
  }
  else { // Done reading messages for scene
    read_scene_parameter_number = 0;
    read_scene_midi_timer = 0;
    MIDI_enable_device_check();

    scene_data_buffer[SY1000_SCENE_ASSIGN_BYTE] = read_scene_assign_state();

    if (save_scene_number == 0) return;
    if (save_scene_number < 9) { // Copy scene_data_buffer to current_scene
      DEBUGMAIN("Patch scene read succesfully from SY1000. Storing to scene " + String(save_scene_number));
      uint8_t b = get_scene_index(save_scene_number);
      //DEBUGMAIN("Scene index for storing: " + String(b));
      for (uint8_t i = 0; i < SY1000_SCENE_DATA_BUFFER_SIZE; i++) {
        SY1000_patch_buffer[b++] = scene_data_buffer[i];
      }
      set_mute_during_scene_change(save_scene_number, mute_during_scene_change);
      set_scene_active(save_scene_number);
      update_page = RELOAD_PAGE;

      //store_patch(patch_number);

      LCD_show_popup_label("Scene read!", MESSAGE_TIMER_LENGTH);
      // Dump data (debug)
      //DEBUGMAIN("SY1000_scene_buffer dump:");
      //MIDI_debug_sysex(SY1000_scene_buffer, VC_PATCH_SIZE, 255, true);
    }
    else { // Update scene differences and write them to all scenes
      if (last_loaded_scene == 0) return;
      check_delta_and_update_scenes();
      load_scene(0, last_loaded_scene);
    }
  }
  LCD_show_bar(0, map(number, 0, SY1000_TOTAL_NUMBER_OF_SCENE_PARAMETERS, 0, 127), 0);
}

FLASHMEM void MD_SY1000_class::exchange_scene(uint8_t new_scene, uint8_t prev_scene) {
  if (new_scene == prev_scene) return;
  bool new_scene_active = check_snapscene_active(new_scene);
  bool prev_scene_active = check_snapscene_active(prev_scene);

  uint8_t index_new = get_scene_index(new_scene);
  uint8_t index_prev = get_scene_index(prev_scene);
  uint8_t temp;
  for (uint8_t i = 0; i < SY1000_SCENE_DATA_BUFFER_SIZE + 8; i++) {
    temp = SY1000_patch_buffer[index_prev];
    SY1000_patch_buffer[index_prev] = SY1000_patch_buffer[index_new];
    SY1000_patch_buffer[index_new] = temp;
    index_new++;
    index_prev++;
  }

  if (new_scene_active) set_scene_active(prev_scene);
  else clear_scene_active(prev_scene);
  if (prev_scene_active) set_scene_active(new_scene);
  else clear_scene_active(new_scene);

  set_snapscene_number_and_LED(new_scene);
  last_loaded_scene = 0;
}


FLASHMEM void MD_SY1000_class::initialize_scene(uint8_t scene) {
  DEBUGMAIN("Initializing scene " + String(scene));
  uint8_t index = get_scene_index(scene);
  for (uint8_t i = 0; i < SY1000_SCENE_DATA_BUFFER_SIZE + 8; i++) SY1000_patch_buffer[index++] = 0;

  // Set default label
  scene_label_buffer[0] = '-';
  scene_label_buffer[1] = '-';
  for (uint8_t i = 2; i < 8; i++) scene_label_buffer[i] = ' ';
  store_scene_name_to_buffer(scene);

  clear_scene_active(scene);
  set_snapscene_number_and_LED(0);
  last_loaded_scene = 0;
}

FLASHMEM bool MD_SY1000_class::check_snapscene_active(uint8_t scene) {
  if ((scene == 0) || (scene > 8)) return false;
  scene--;
  return ((SY1000_patch_buffer[SY1000_SCENE_ACTIVE_BYTE] & (1 << scene)) != 0);
}

FLASHMEM void MD_SY1000_class::set_scene_active(uint8_t scene) {
  if ((scene == 0) || (scene > 8)) return;
  scene--;
  bitSet(SY1000_patch_buffer[SY1000_SCENE_ACTIVE_BYTE], scene);
}

FLASHMEM void MD_SY1000_class::clear_scene_active(uint8_t scene) {
  if ((scene == 0) || (scene > 8)) return;
  scene--;
  bitClear(SY1000_patch_buffer[SY1000_SCENE_ACTIVE_BYTE], scene);
}

FLASHMEM bool MD_SY1000_class::check_mute_during_scene_change(uint8_t scene) {
  if ((scene == 0) || (scene > 8)) return false;
  scene--;
  return ((SY1000_patch_buffer[SY1000_MUTE_DURING_SCENE_CHANGE_BYTE] & (1 << scene)) != 0);
}

FLASHMEM void MD_SY1000_class::set_mute_during_scene_change(uint8_t scene, bool value) {
  if ((scene == 0) || (scene > 8)) return;
  scene--;
  if (value) bitSet(SY1000_patch_buffer[SY1000_MUTE_DURING_SCENE_CHANGE_BYTE], scene);
  else bitClear(SY1000_patch_buffer[SY1000_MUTE_DURING_SCENE_CHANGE_BYTE], scene);
}

FLASHMEM void MD_SY1000_class::read_scene_name_from_buffer(uint8_t scene) {
  uint8_t b = get_scene_index(scene) + SY1000_SCENE_NAME_BYTE;
  for (uint8_t c = 0; c < 8; c++) {
    scene_label_buffer[c] = SY1000_patch_buffer[b++];
  }
}

FLASHMEM void MD_SY1000_class::store_scene_name_to_buffer(uint8_t scene) {
  uint8_t b = get_scene_index(scene) + SY1000_SCENE_NAME_BYTE;
  for (uint8_t c = 0; c < 8; c++) SY1000_patch_buffer[b++] = scene_label_buffer[c];
  //SY1000_label_buffer[c] = '\n';
}

FLASHMEM void MD_SY1000_class::load_patch(uint16_t number) {

  // Read from EEPROM
  if (bass_mode) number += SY1000_BASS_MODE_NUMBER_OFFSET;
  bool loaded = EEPROM_load_device_patch(my_device_number + 1, number, SY1000_patch_buffer, VC_PATCH_SIZE);

  if (loaded) {
    last_loaded_scene = SY1000_patch_buffer[SY1000_CURRENT_SCENE_BYTE];
  }
  else {
    initialize_patch_space();
    last_loaded_scene = 0;
  }
  current_snapscene = last_loaded_scene;
  uint8_t index = get_scene_index(current_snapscene) + SY1000_SCENE_ASSIGN_BYTE;
  set_scene_assign_states(SY1000_patch_buffer[index]);
  DEBUGMSG("SY1000 patch loaded from EEPROM");
  //MIDI_debug_sysex(SY1000_patch_buffer, VC_PATCH_SIZE, 255, true); // Show contents of patch buffer
}

FLASHMEM bool MD_SY1000_class::store_patch(uint16_t number) {

  if (bass_mode) number |= SY1000_BASS_MODE_NUMBER_OFFSET;

  // Check if scenes are empty
  if (SY1000_patch_buffer[SY1000_SCENE_ACTIVE_BYTE] == 0) {
    EEPROM_initialize_device_patch(my_device_number + 1, number);
    return false;
  }

  // Add header
  SY1000_patch_buffer[SY1000_PATCH_TYPE] = my_device_number + 1;
  SY1000_patch_buffer[SY1000_PATCH_NUMBER_MSB] = number >> 8; // Patch_number_MSB
  SY1000_patch_buffer[SY1000_PATCH_NUMBER_LSB] = number & 0xFF; // Patch_number_LSB

  // Store to EEPROM
  SY1000_patch_buffer[SY1000_CURRENT_SCENE_BYTE] = last_loaded_scene;
  //MIDI_debug_sysex(SY1000_patch_buffer, VC_PATCH_SIZE, 255, true);  // Show contents of patch buffer
  return EEPROM_save_device_patch(my_device_number + 1, number, SY1000_patch_buffer, VC_PATCH_SIZE);
}

FLASHMEM bool MD_SY1000_class::exchange_patches(uint16_t number) {
  uint16_t pn = patch_number;
  if (bass_mode) {
    number |= SY1000_BASS_MODE_NUMBER_OFFSET;
    pn |= SY1000_BASS_MODE_NUMBER_OFFSET;
  }
  EEPROM_exchange_device_patch(my_device_number + 1, number, pn, VC_PATCH_SIZE);
  LCD_show_popup_label("Patches swapped", MESSAGE_TIMER_LENGTH);
  return true; // Swap succesfull
}

FLASHMEM bool MD_SY1000_class::insert_patch(uint16_t number) {
  if (bass_mode) number |= SY1000_BASS_MODE_NUMBER_OFFSET;

  // First clear the last user patch
  uint16_t last_patch = 199;
  if (bass_mode) last_patch |= SY1000_BASS_MODE_NUMBER_OFFSET;
  EEPROM_initialize_device_patch(my_device_number + 1, last_patch);

  // Then move up all patches below
  for (uint16_t p = last_patch; p -- > number; ) {
    EEPROM_exchange_device_patch(my_device_number + 1, p, p + 1, VC_PATCH_SIZE);
    DEBUGMSG("swapping " + String(p) + " and " + String(p + 1));
  }

  // Then store the patch
  return store_patch(number);
}

FLASHMEM bool MD_SY1000_class::initialize_patch(uint16_t number) {
  if (bass_mode) number += SY1000_BASS_MODE_NUMBER_OFFSET;
  EEPROM_initialize_device_patch(my_device_number + 1, number);
  return true;
}

FLASHMEM void MD_SY1000_class::initialize_patch_space() {
  memset(SY1000_patch_buffer, 0, VC_PATCH_SIZE);
  scene_label_buffer[0] = '-';
  scene_label_buffer[1] = '-';
  for (uint8_t i = 2; i < 8; i++) scene_label_buffer[i] = ' ';
  for (uint8_t s = 1; s <= 8; s++) {
    store_scene_name_to_buffer(s);
  }
  set_snapscene_number_and_LED(0);
}

// ********************************* Section 8: SY1000 Expression pedal control ********************************************

#define RANGE_DIRECT 1
#define RANGE_CUTOFF 2
#define RANGE_RESONANCE 3
#define RANGE_FX 4
#define RANGE1000 5
#define RANGE1000T 6
#define RANGE100 7
#define RANGE200 8
#define LAST_THREE 9
#define LAST_THREEx2 10

struct SY1000_exp_pedal_target_struct { // Combines all the data we need for controlling a parameter in a device
  uint16_t Address_GM; // The address of the parameter in guitar mode
  uint16_t Address_BM; // The address of the parameter in bass mode
  uint8_t Type;
};

SY1000_exp_pedal_target_struct SY1000_exp_pedal_target[] = {
  { 0, 0, 0 }, // OFF
  { 0x120E, 0x120E, RANGE1000 }, // FV1
  { 0x121B, 0x121B, RANGE1000 }, // FV2
  { 0x120E, 0x120E, RANGE1000T }, // FV1 + TUNER
  { 0x121B, 0x121B, RANGE1000T }, // FV2 + TUNER
  { 0x4000, 0x3D00, RANGE_FX }, // FX1 PEDAL POS - Base address of FX1
  { 0x6600, 0x6300, RANGE_FX }, // FX2 PEDAL POS - Base address of FX2
  { 0x8C00, 0x8900, RANGE_FX }, // FX3 PEDAL POS - Base address of FX3
  { 0x123C, 0x123C, RANGE100 }, // PATCH LEVEL 100
  { 0x123C, 0x123C, RANGE200 }, // PATCH LEVEL200
  { 0x1502, 0x1502, RANGE_DIRECT }, // INST1 LEVEL
  { 0x2002, 0x1F02, RANGE_DIRECT }, // INST2 LEVEL
  { 0x2B02, 0x2902, RANGE_DIRECT }, // INST3 LEVEL
  { 0, 0, LAST_THREE }, // INST ALL LEVEL
  { 0x1500, 0x1500, RANGE_CUTOFF }, // INST1 CUTOFF - Base address of instr1
  { 0x2000, 0x1900, RANGE_CUTOFF }, // INST2 CUTOFF - Base address of instr2
  { 0x2B00, 0x2900, RANGE_CUTOFF }, // INST3 CUTOFF - Base address of instr3
  { 0, 0, LAST_THREE }, // INST ALL CUTOFF
  { 0x1500, 0x1500, RANGE_RESONANCE }, // INST1 CUTOFF - Base address of instr1 + 1
  { 0x2000, 0x1900, RANGE_RESONANCE }, // INST2 CUTOFF - Base address of instr2 + 1
  { 0x2B00, 0x2900, RANGE_RESONANCE }, // INST3 CUTOFF - Base address of instr3 + 1
  { 0, 0, LAST_THREE }, // INST ALL RESO
  { 0x1A08, 0x1A01, RANGE_DIRECT }, // 1:GTR VOL
  { 0x2508, 0x2401, RANGE_DIRECT }, // 2:GTR VOL
  { 0x3008, 0x2E01, RANGE_DIRECT }, // 3:GTR VOL
  { 0, 0, LAST_THREE }, // ALL:GTR VOL
  { 0x1504, 0x1504, RANGE100 }, // 1:NOR MIX 100
  { 0x1504, 0x1504, RANGE200 }, // 1:NOR MIX 200
  { 0x2004, 0x1F04, RANGE100 }, // 2:NOR MIX 100
  { 0x2004, 0x1F04, RANGE200 }, // 2:NOR MIX 200
  { 0x2B04, 0x2904, RANGE100 }, // 3:NOR MIX 100
  { 0x2B04, 0x2904, RANGE200 }, // 3:NOR MIX 200
  { 0, 0, LAST_THREEx2 }, // ALL:NOR MIX100
  { 0, 0, LAST_THREEx2 }, // ALL:NOR MIX200
  { 0x1F2F, 0x1E2F, RANGE_DIRECT }, // 1:STR BEND
  { 0x2A2F, 0x282F, RANGE_DIRECT }, // 2:STR BEND
  { 0x352F, 0x322F, RANGE_DIRECT }, // 3:STR BEND
  { 0, 0, LAST_THREE }, // ALL:STR BEND
  { 0x1612, 0x1612, RANGE_DIRECT }, // 1:DYNA BEND
  { 0x2112, 0x2012, RANGE_DIRECT }, // 2:DYNA BEND
  { 0x2C12, 0x2A12, RANGE_DIRECT }, // 3:DYNA BEND
  { 0, 0, LAST_THREE }, // ALL:DYNA BEND
  { 0x1222, 0x1222, RANGE_DIRECT }, // MIXER A/B
  { 0x1225, 0x1225, RANGE_DIRECT }, // BAL1 A/B
  { 0x1228, 0x1228, RANGE_DIRECT }, // BAL2 A/B
  { 0x122B, 0x122B, RANGE_DIRECT }, // BAL3 A/B
};

#define INST1_CUTOFF_TYPE 14
#define INST2_CUTOFF_TYPE 15
#define INST3_CUTOFF_TYPE 16
#define INST1_RESONANCE_TYPE 18
#define INST2_RESONANCE_TYPE 19
#define INST3_RESONANCE_TYPE 20

const uint8_t SY1000_NUMBER_OF_EXP_PEDAL_TARGETS = sizeof(SY1000_exp_pedal_target) / sizeof(SY1000_exp_pedal_target[0]);


FLASHMEM void MD_SY1000_class::move_expression_pedal(uint8_t sw, uint8_t value, uint8_t exp_pedal) {
  uint16_t val = value;
  if (exp_pedal == 0) exp_pedal = current_exp_pedal;
  LCD_show_bar(0, value, 0); // Show it on the main display

  if ((exp_pedal == 1) && (exp1_type < SY1000_NUMBER_OF_EXP_PEDAL_TARGETS)) {
    val = send_expression_value(exp_pedal, exp1_type, value);
  }
  if ((exp_pedal == 2) && (exp2_type < SY1000_NUMBER_OF_EXP_PEDAL_TARGETS)) {
    val = send_expression_value(exp_pedal, exp2_type, value);
  }
  update_exp_label(sw);
  String msg = SP[sw].Label;
  msg = msg.trim();
  msg += ':';
  LCD_add_3digit_number(val, msg);
  LCD_show_popup_label(msg, ACTION_TIMER_LENGTH);
  update_page = REFRESH_FX_ONLY; // To update the other switch states, we re-load the current page*/
}

FLASHMEM uint16_t MD_SY1000_class::send_expression_value(uint8_t exp_pedal, uint8_t exp_type, uint8_t value) {
  uint32_t address;
  uint16_t val = value;
  if (!bass_mode) {
    address = SY1000_exp_pedal_target[exp_type].Address_GM;
    if (address & 0x8000) address = (address & 0x7FFF) | 0x10000;
    address += 0x10000000;
  }
  else {
    address = SY1000_exp_pedal_target[exp_type].Address_BM;
    if (address & 0x8000) address = (address & 0x7FFF) | 0x10000;
    address += 0x10020000;
  }
  uint8_t type = SY1000_exp_pedal_target[exp_type].Type;

  uint8_t v8;
  uint16_t v16;

  uint8_t inst_type = 255;
  control_edit_mode();
  switch (type) {
    case RANGE_DIRECT:
      v8 = map(value, 0, 127, 0, 100);
      write_sysex(address, v8);
      val = v8;
      break;
    case RANGE_CUTOFF:
      if (exp_type == INST1_CUTOFF_TYPE) inst_type = INST_type[0];
      if (exp_type == INST2_CUTOFF_TYPE) inst_type = INST_type[1];
      if (exp_type == INST3_CUTOFF_TYPE) inst_type = INST_type[2];

      v8 = map(value, 0, 127, 0, 100);
      if (inst_type == 0) write_sysex(address + 0x011F, v8); // Address of cutoff of DYN_SYNTH compared to base address of instrument
      if (inst_type == 1) write_sysex(address + 0x031D, v8); // Address of cutoff of OSC_SYNTH compared to base address of instrument
      if (inst_type == 2) write_sysex(address + 0x0402, v8); // Address of cutoff of GR-300 compared to base address of instrument
      val = v8;
      break;
    case RANGE_RESONANCE:
      if (exp_type == INST1_RESONANCE_TYPE) inst_type = INST_type[0];
      if (exp_type == INST2_RESONANCE_TYPE) inst_type = INST_type[1];
      if (exp_type == INST3_RESONANCE_TYPE) inst_type = INST_type[2];

      v8 = map(value, 0, 127, 0, 100);
      if (inst_type == 0) write_sysex(address + 0x0120, v8); // Address of resonance of DYN_SYNTH compared to base address of instrument
      if (inst_type == 1) write_sysex(address + 0x031F, v8); // Address of resonance of OSC_SYNTH compared to base address of instrument
      if (inst_type == 2) write_sysex(address + 0x0403, v8); // Address of resonance of GR-300 compared to base address of instrument
      val = v8;
      break;
    case RANGE_FX:
      v16 = map(value, 0, 127, 0, 1000);
      write_sysex(address + 0x1602, (v16 & 0xF000) >> 12, (v16 & 0x0F00) >> 8, (v16 & 0x00F0) >> 4, (v16 & 0x000F)); // Address of PEDAL BEND compared to base address of instrument
      write_sysex(address + 0x2502, (v16 & 0xF000) >> 12, (v16 & 0x0F00) >> 8, (v16 & 0x00F0) >> 4, (v16 & 0x000F)); // Address of WAH compared to base address of instrument
      write_sysex(address + 0x0B08, (v16 & 0xF000) >> 12, (v16 & 0x0F00) >> 8, (v16 & 0x00F0) >> 4, (v16 & 0x000F)); // Address of FOOT VOL compared to base address of instrument
      val = v16 / 10;
      break;
    case RANGE100:
      v8 = map(value, 0, 127, 0, 100);
      write_sysex(address, (v8 & 0xF0) >> 4, (v8 & 0x0F));
      val = v8;
      break;
    case RANGE200:
      v8 = map(value, 0, 127, 0, 200);
      write_sysex(address, (v8 & 0xF0) >> 4, (v8 & 0x0F));
      break;
    case RANGE1000T:
      if ((value == 0) && (!tuner_active)) start_tuner();
      if ((value > 2) && (tuner_active)) stop_tuner();
    // no_break!
    case RANGE1000:
      v16 = map(value, 0, 127, 0, 1000);
      write_sysex(address, (v16 & 0xF000) >> 12, (v16 & 0x0F00) >> 8, (v16 & 0x00F0) >> 4, (v16 & 0x000F));
      val = v16 / 10;
      break;
    case LAST_THREE:
      val = send_expression_value(exp_pedal, exp_type - 3, value);
      val = send_expression_value(exp_pedal, exp_type - 2, value);
      val = send_expression_value(exp_pedal, exp_type - 1, value);
      break;
    case LAST_THREEx2:
      val = send_expression_value(exp_pedal, exp_type - 6, value);
      val = send_expression_value(exp_pedal, exp_type - 4, value);
      val = send_expression_value(exp_pedal, exp_type - 2, value);
      break;
  }

  uint8_t assign_target_exp = 0;
  if (exp_pedal == 1) assign_target_exp = 13;
  else if (exp_pedal == 2) assign_target_exp = 14;
  else return val;

  if (millis() > assign_return_target_timer) {
    change_active_assign_sources(assign_target_exp, SY1000_SOURCE_CC94);
    delay(20);
    assign_return_target_timer = millis() + SY1000_ASSIGN_RETURN_TIME;
    assign_return_target = assign_target_exp;
  }

  MIDI_send_CC(94, value, MIDI_channel, MIDI_out_port);
  return val;
}

FLASHMEM void MD_SY1000_class::toggle_expression_pedal(uint8_t sw) {
  if (current_exp_pedal == 0) return;
  current_exp_pedal++;
  if (current_exp_pedal > 2) current_exp_pedal = 1;
  update_page = REFRESH_FX_ONLY;
}

FLASHMEM bool MD_SY1000_class::request_exp_pedal(uint8_t sw, uint8_t exp_pedal) { // Used for both Master_exp_pedal and toggle_exp_pedal
  if (exp_pedal == 0) exp_pedal = current_exp_pedal;
  if (exp_pedal <= 2) {
    last_requested_sysex_type = REQUEST_PEDAL_ASSIGN;
    last_requested_sysex_switch = sw;
    if (!bass_mode) request_sysex(0x1000002A, 2); // Request the target of exp1 and exp2
    else request_sysex(0x1002002A, 2);
  }
  LCD_clear_SP_label(sw);
  return false;
}

FLASHMEM void MD_SY1000_class::update_exp_label(uint8_t sw) {
  if (current_exp_pedal == 1) {
    SP[sw].PP_number = SY1000_EXP1;
    read_parameter(sw, exp1_type, 0);
  }
  if (current_exp_pedal == 2) {
    SP[sw].PP_number = SY1000_EXP2;
    read_parameter(sw, exp2_type, 0);
  }
}

FLASHMEM void MD_SY1000_class::check_patch_midi(uint8_t cc, uint8_t value) {
  if (cc == 0) return;
  DEBUGMSG("Patch MIDI CC#" + String(cc - 1) + " value " + String(value));
  switch (cc - 1) {
    case 10: inst_harmony_type[0] = value; break;
    case 11: inst_harmony_interval[0] = value; break;
    case 12: inst_harmony_type[1] = value; break;
    case 13: inst_harmony_interval[1] = value; break;
    case 14: inst_harmony_type[2] = value; break;
    case 15: inst_harmony_interval[2] = value; break;
  }
}

FLASHMEM void MD_SY1000_class::reset_special_functions() {
  DEBUGMSG("!!! Resetting special functions");
  for (uint8_t i = 0; i < 3; i++) {
    inst_harmony_type[i] = 0;
    inst_harmony_interval[i] = 0;
    for (uint8_t j = 0; j < 6; j++) current_string_interval[i][j] = 0;
  }
}


FLASHMEM void MD_SY1000_class::set_harmony_interval(uint8_t note, uint8_t velocity, uint8_t channel, uint8_t port) {
  // Update user tuning with favourite tuning
  const uint32_t user_tuning_gm[3] = {0x10001F02, 0x10002A02, 0x10003502};
  const uint32_t user_tuning_bm[3] = {0x10021E02, 0x10022802, 0x10023202};
  if (velocity > 0) {
    uint8_t my_note = (note - master_key) % 12;
    uint8_t my_string = channel - 1;

    for (uint8_t inst = 0; inst < 3; inst++) {
      uint8_t my_interval = calculate_interval(inst_harmony_type[inst], inst_harmony_interval[inst], my_note);

      if ((my_interval > 0) && (current_string_interval[inst][my_string] != my_interval)) {
        control_edit_mode();
        uint32_t address;
        if (!bass_mode) address = user_tuning_gm[inst];
        else address = user_tuning_bm[inst];
        address += my_string;
        write_sysex(address, my_interval);
        current_string_interval[inst][my_string] = my_interval;
      }
    }
  }
}

// My interval settings:
// 0: -2 oct, 1: -14th, 2: -13th, 3: -12th, 4: -11th, 5: - 10th, 6: -9th,
// 7: -1 oct, 8: -7th,  9: -6th, 10: -5th, 11: -4th, 12: -3rd, 13: -2nd
//14: TONIC, 15: +2nd, 16: +3rd, 17: +4th, 18: +5th, 19: +6th, 20: +7th
//21: +1 oct,22, +9th, 23: +10th, 24: +11th, 25: +12th, 26: +13th, 27: +14th
// 28: +2oct

const uint8_t positive_interval[7][12] = {
  { 2,  2,  2,  2,  1,  2,  2,  2,  2,  2,  2,  1},
  { 4,  3,  3,  4,  3,  4,  3,  4,  3,  3,  4,  3},
  { 5,  5,  5,  5,  5,  6,  5,  5,  5,  5,  5,  5},
  { 7,  6,  7,  6,  7,  7,  6,  7,  6,  7,  6,  6},
  { 9,  9,  9,  8,  8,  9,  8,  9,  8,  8,  8,  8},
  {11, 10, 10,  9, 10, 11, 10, 10,  9, 10,  9, 10},
  {12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12},
};

const uint8_t negative_interval[7][12] = {
  { 1,  2,  2,  3,  2,  1,  2,  2,  3,  2,  3,  2},
  { 3,  3,  3,  4,  4,  3,  4,  3,  4,  4,  4,  4},
  { 5,  6,  5,  6,  5,  5,  6,  5,  6,  5,  6,  6},
  { 7,  7,  7,  7,  7,  8,  7,  7,  7,  7,  7,  7},
  { 8,  9,  9,  9,  9,  8,  9,  8,  9,  9,  9,  9},
  {10, 10, 10, 10, 11, 10, 10, 10, 10, 10, 10, 11},
  {12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12},
};

const uint8_t sixeights_intervals[12] = {7, 7, 10, 9, 8, 7, 8, 7, 7, 7, 7, 8};

FLASHMEM uint8_t MD_SY1000_class::calculate_interval(uint8_t my_type, uint8_t my_interval, uint8_t my_note) {
  if (my_type == 1) {
    if (my_interval > 28) return 0;
    if (my_interval < 7) return 20 - negative_interval[6 - my_interval][my_note]; // More than octave down
    if (my_interval < 14) return 32 - negative_interval[13 - my_interval][my_note]; // Up to actave down
    if (my_interval == 14) return 32; // Tonic
    if (my_interval > 21) return 44 + positive_interval[my_interval - 22][my_note]; // More than octave up
    if (my_interval > 14) return 32 + positive_interval[my_interval - 15][my_note]; // Up to octave up
  }
  if (my_type == 2) {
    uint8_t i = calculate_interval(1, my_interval, my_note);
    switch ((my_note + i + 4) % 12) {
      case 11: // Move maj7 to octave
        return i + 1;
      case 10: // move min7 to octave
        return i + 2;
    }
    return i;
  }
  if (my_type == 3) { //return 32 + sixeights_intervals[my_note];
    const signed int semitones[] = { -24, -23, -21, -19, -17, -16, -14, -12, -11, -9, -7, -5, -4, -2, 0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17, 19, 21, 23, 24 };
    uint8_t i = semitones[my_interval] + 32;
    DEBUGMSG("!!Interval: " + String((my_note + i + 4) % 12));
    switch ((my_note + i + 4) % 12) {
      case 1:
      case 6:
      case 11:
        return i + 1;
      case 9:
        return i + 3;
      case 10:
        return i + 2;
    }
    return i;
  }
  if (my_type == 4) return 32 + sixeights_intervals[my_note];

  return 0;
}

/*FLASHMEM void MD_SY1000_class::control_slow_gear(uint8_t note, uint8_t velocity, uint8_t channel, uint8_t port) {
  uint8_t my_string = channel - 1;
  if (velocity > 0) {
    // Start swell for string
    slow_gear_string_updown[my_string] = true;
    slow_gear_string_level[my_string] = 0;
    update_string_level(my_string, 0);
  }
  else {
    slow_gear_string_updown[my_string] = false;
  }
  }

  FLASHMEM void MD_SY1000_class::update_slow_gear() {
  if (millis() > slow_gear_timer) {
    slow_gear_timer = millis() + 10;
    for (uint8_t s = 0; s < 6; s++) {
      if (slow_gear_string_updown[s] == true) {
        if (slow_gear_string_level[s] < 100) {
          slow_gear_string_level[s] ++;
          update_string_level(s, slow_gear_string_level[s]);
        }
      }
      else {
        if (slow_gear_string_level[s] > 0) {
          slow_gear_string_level[s] --;
          update_string_level(s, slow_gear_string_level[s]);
        }
      }
    }
  }
  }

  FLASHMEM void MD_SY1000_class::update_string_level(uint8_t string, uint8_t level) {
  control_edit_mode();
  uint32_t address = 0x10001506;
  address += string;
  write_sysex(address, level);
  }*/
