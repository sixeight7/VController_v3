// Please read VController_v3.ino for information about the license and authors

// Class structure for Fractal Audio Systems Products
// Tested with AxeFX 2

// This page has the following parts:
// Section 1: FAS Initialization
// Section 2: FAS common MIDI in functions
// Section 3: FAS common MIDI out functions
// Section 4: FAS program change
// Section 5: FAS parameter control
// Section 6: FAS scene and looper control

// ********************************* Section 1: FAS Initialization ********************************************

// Fractal AxeFX default settings:
#define FAS_MIDI_CHANNEL 1
#define FAS_MIDI_PORT MIDI1_PORT // Default port is MIDI1
#define FAS_PATCH_MIN 0
#define FAS_PATCH_MAX 383

// In the Axe-Fx's I/O / MIDI settings page, make sure that "Send Realtime Sysex” is set to ALL.

// AxeFX model numbers:
#define FAS_MODEL_STANDARD 0x00 // Axe-Fx Standard
#define FAS_MODEL_ULTRA 0x01 //Axe-Fx Ultra
#define FAS_MODEL_AF2 0x03 //Axe-Fx II
#define FAS_MODEL_FX8 0x05 //FX8
#define FAS_MODEL_XL 0x06 //Axe-Fx II XL
#define FAS_MODEL_XLP 0x07 //Axe-Fx II XL+
#define FAS_MODEL_AX8 0x08 //AX8
#define FAS_MODEL_FX8mk2 0x0A //FX8 mk2
#define FAS_MODEL_AF3 0x10 //Axe-Fx III

// Max number of patches per model - not sure if all these numbers are correct yet...
#define AXEFX1_PATCH_MAX 383
#define AXEFX1U_PATCH_MAX 383
#define AXEFX2_PATCH_MAX 383
#define FX8_PATCH_MAX 127
#define AXEFX2XL_PATCH_MAX 767
#define AXEFX2XLP_PATCH_MAX 767
#define AX8_PATCH_MAX 511
#define FX8M2_PATCH_MAX 511
#define AXEFX3_PATCH_MAX 511

// CC messages
#define FAS_BYPASS_CC 13
#define FAS_TUNER_CC 15
#define FAS_SCENE_SELECT_CC 34 // Default value for scene select - but not on AF3

// Block IDs
#define FAS_LOOPER_ID 169

// Snooping data from Axe-edit
// F0 00 01 74 7F 00 7A F7 is sent out by Axe-Edit - this will query for any Fractal device.
// The axefx answers with F0 00 01 74 03 64 00 00 62 F7 - which probably means: "Hi I am an axefx 2"
// Axe-edit: F0 00 01 74 03 08 0E F7 - request for firmware version - also puts the Axe-fx in edit mode
// AF-2: F0 00 01 74 03 08 1D 04  01 01 01 00 16 F7 - the current version
// Axe-edit: F0 00 01 74 03 47 41 F7 - some unknown request
// AF-2: F0 00 01 74 03 47 03 02  04 01 64 00 04 00 25 F7 - unknown answer.
// Axe-edit: F0 00 01 74 03 15 7F 7F 13 F7 - request for all patch names
// AF-2: F0 00 01 74 03 15 00 00  (Patch name) 00 CS F7 - the patch name of patch one - all the patches are numbered 00 00 to 02 7F (second byte is LSB)
// Axe-edit: F0 00 01 74 03 12 7F 7F  14 F7 - Request for CAB names
// AF-2: F0 00 01 74 03 32 12 68  01 4F F7 - The number of cabs (68 01)
// AF-2: F0 00 01 74 03 12 00 00 (Cab name) CS F7 - the name of CAB 01 - cabs are numbered from 00 00 to 67 01 (first byte is LSB)
// AF-2: F0 00 01 74 03 33 12 27  F7 - ???
// More data is exchanged
// On closing Axe-edit: F0 00 01 74 03 42 44 F7
// AF-2: F0 00 01 74 03 64 42 00  20 F7 - Which is probably "Bye for now"


// Sysex request commands
#define FAS_FIRMWARE_VERSION 0x08
#define FAS_FIRMWARE_VERSION_AF2 0x64
#define FAS_GET_PRESET_NUMBER 0x14
#define FAS_GET_PRESET_NAME 0x0F
#define FAS_GET_PRESET_NAME_BY_NUMBER 0x15
#define FAS_EDIT_MODE_ON 0x08
#define FAS_EDIT_MODE_OFF 0x42
#define FAS_GET_LOOPER_STATUS 0x23
#define FAS_GET_SCENE_STATUS 0x29
#define FAS_REALTIME_TEMPO 0x10
#define FAS_REALTIME_TUNER 0x0D
#define FAS_SET_PARAMETER 0x02
#define FAS_GET_FX_BLOCK_BYPASS_STATE 0x0E
#define FAS_GET_SET_BLOCK_PARAMETER_VALUE 0x02

// Initialize device variables
// Called at startup of VController
FLASHMEM void MD_FAS_class::init() { // Default values for variables
  MD_base_class::init();

  // AXE-FX variables:
  enabled = DEVICE_DETECT; // Default value
  strcpy(device_name, "AXEFX");
  strcpy(full_device_name, "Fractal device");
  model_number = FAS_MODEL_AF2;
  patch_min = FAS_PATCH_MIN;
  patch_max = FAS_PATCH_MAX;
  max_times_no_response = MAX_TIMES_NO_RESPONSE; // The number of times the AXEFX does not have to respond before disconnection
  sysex_delay_length = 0; // time between sysex messages (in msec).
  my_LED_colour = 1; // Default value: green
  MIDI_channel = FAS_MIDI_CHANNEL; // Default value
  MIDI_port_manual = MIDI_port_number(FAS_MIDI_PORT); // Default value

#if defined(CONFIG_VCTOUCH)
  my_device_page1 = FAS_DEFAULT_VCTOUCH_PAGE1; // Default value
  my_device_page2 = FAS_DEFAULT_VCTOUCH_PAGE2; // Default value
  my_device_page3 = FAS_DEFAULT_VCTOUCH_PAGE3; // Default value
  my_device_page4 = FAS_DEFAULT_VCTOUCH_PAGE4; // Default value
#elif defined(CONFIG_VCMINI)
  my_device_page1 = FAS_DEFAULT_VCMINI_PAGE1; // Default value
  my_device_page2 = FAS_DEFAULT_VCMINI_PAGE2; // Default value
  my_device_page3 = FAS_DEFAULT_VCMINI_PAGE3; // Default value
  my_device_page4 = FAS_DEFAULT_VCMINI_PAGE4; // Default value
#elif defined (CONFIG_CUSTOM)
  my_device_page1 = FAS_DEFAULT_CUSTOM_PAGE1; // Default value
  my_device_page2 = FAS_DEFAULT_CUSTOM_PAGE2; // Default value
  my_device_page3 = FAS_DEFAULT_CUSTOM_PAGE3; // Default value
  my_device_page4 = FAS_DEFAULT_CUSTOM_PAGE4; // Default value
#else
  my_device_page1 = FAS_DEFAULT_VC_PAGE1; // Default value
  my_device_page2 = FAS_DEFAULT_VC_PAGE2; // Default value
  my_device_page3 = FAS_DEFAULT_VC_PAGE3; // Default value
  my_device_page4 = FAS_DEFAULT_VC_PAGE4; // Default value
#endif
  clear_FX_states();
  number_of_active_blocks = 0;
  looper_block_detected = false;
  //max_looper_length = 30000000; // Normal stereo looper time is 30 seconds - time given in microseconds
#ifdef IS_VCTOUCH
  device_pic = img_FAS_AXEFX2;
#endif
}

FLASHMEM void MD_FAS_class::update() {
  if (!connected) return;
  looper_timer_check();
}

// ********************************* Section 2: FAS common MIDI in functions ********************************************

FLASHMEM void MD_FAS_class::check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port) { // Check incoming sysex messages from  Called from MIDI:OnSysEx/OnSerialSysEx

  // Check if it is a message from an AXEFX
  if ((sxdata[1] == 0x00) && (sxdata[2] == 0x01) && (sxdata[3] == 0x74)) {

    // Check if it is GET_FIRMWARE_RESPONSE - used for device detection of the AxeFX
    if (((sxdata[5] == FAS_FIRMWARE_VERSION) || (sxdata[5] == FAS_FIRMWARE_VERSION_AF2)) && (!connected) && (enabled == DEVICE_DETECT)) {
      set_type(sxdata[4]); // Will change the device name after a succesful detection
      connect(sxdata[4], port, Current_MIDI_out_port | (port & 0x0F)); // Will connect to the device
    }

    else if ((port == MIDI_in_port) && (sxdata[4] == model_number)) { // Check the other messages

      uint16_t new_patch;
      bool end_reached;

      // Byte 5 of the SysEx message holds the function number, the function
      // ids are defined in AxeMidi.h
      switch (sxdata[5]) {

        case FAS_REALTIME_TEMPO: // We use the realtime tempo messages to check if the AxeFX is still connected
          no_response_counter = 0;
          break;

        case FAS_GET_PRESET_NUMBER:
          new_patch = sxdata[6] << 7 | sxdata[7];
          if (patch_number != new_patch) { //Right after a patch change the patch number is sent again. So here we catch that message.
            prev_patch_number = patch_number;
            set_patch_number(new_patch);
            //page_check();
            do_after_patch_selection();
            update_page = REFRESH_PAGE;
          }
          break;

        case FAS_GET_PRESET_NAME:
          current_patch_name = "";
          end_reached = false;
          for (uint8_t count = 6; count < 22; count++) {
            if (sxdata[count] == 0x00) end_reached = true;
            if (!end_reached) current_patch_name += static_cast<char>(sxdata[count]); //Add ascii character to Patch Name String
            else current_patch_name += ' ';
          }
          update_main_lcd = true;
          if (popup_patch_name) {
            LCD_show_popup_label(current_patch_name, ACTION_TIMER_LENGTH);
            popup_patch_name = false;
          }
          break;


        case FAS_GET_PRESET_NAME_BY_NUMBER: // Patch name of specific preset
          for (uint8_t count = 0; count < 16; count++) {
            SP[last_requested_sysex_switch].Label[count] = static_cast<char>(sxdata[count + 8]); //Add ascii character to the SP.Label String
          }
          if (SP[last_requested_sysex_switch].PP_number == patch_number) {
            current_patch_name = SP[last_requested_sysex_switch].Label; // Load patchname when it is read
            update_main_lcd = true; // And show it on the main LCD
          }
          DEBUGMSG(SP[last_requested_sysex_switch].Label);
          PAGE_request_next_switch();
          break;

        case FAS_GET_SCENE_STATUS:
          if (current_scene != sxdata[6]) {
            current_scene = sxdata[6];
            DEBUGMSG("Current scene: " + String(current_scene));
            update_page = REFRESH_PAGE;
          }
          break;

        case FAS_GET_FX_BLOCK_BYPASS_STATE: // Check if it is a SYSEX_FAS_GET_PRESET_EFFECT_BLOCKS_AND_CC_AND_BYPASS_STATE message

          // Source: https://github.com/mackatack/FCBInfinity/tree/master/Teensyduino/fcbinfinity
          // declare some vars we'll use in the loop
          uint8_t state = 0;
          uint8_t effectID = 0;
          uint8_t cc = 0;
          uint8_t cc_xy = 0;
          looper_block_detected = false;

          // Reset all the effectstates to 'not placed'
          clear_FX_states();
          number_of_active_blocks = 0;

          for (uint8_t i = 6; i < sxlength - 4; i += 5) {
            if (model_number < FAS_MODEL_AF2) {
              // Older models
              // byte+0 effect ID LS nibble
              // byte+1 effect ID MS nibble
              // byte+2 bypass CC# LS nibble
              // byte+3 bypass CC# MS nibble
              // byte+4 bypass state: 0=bypassed; 1=not bypassed
              effectID = (sxdata[i + 1] << 4) | sxdata[i];
              cc = (sxdata[i + 3] << 4) | sxdata[i + 2];
              state = sxdata[i + 4];
            }
            else {
              // AxeFx2 and up style
              // for each effect there are 5 bytes:
              // byte+0, bit 1 is bypass, bit 2 is X/Y state
              // byte+1: bit 2-7 is bypass CC LS 6 bits
              // byte+2: bit 1+2 is bypass CC MS 2 bits, bit 3 - 7 is XY CC 5 bits
              // byte+3: bit 1+2 is XY CC LS bits + bit 4 - 7 is effectID LS nibble
              // byte+4: bit 1-4 is effectID MS nibble
              effectID = ((sxdata[i + 3] & 0x78) >> 3) | ((sxdata[i + 4] & 0x0F) << 4); // block id
              cc = ((sxdata[i + 1] & 0x7E) >> 1) | ((sxdata[i + 2] & 3) << 6); // cc number of bypass state
              cc_xy = ((sxdata[i + 2] & 0x7C) >> 2) | ((sxdata[i + 3] & 3) << 5); // cc number of xy state
              state = sxdata[i]; // byp and XY state
            }

            // Update the state and cc of the effectblock to the new values.
            number_of_active_blocks++;
            set_FX_state(cc, state & 0x01); // Set the state for the FX bypass
            set_FX_state(cc_xy, ((state & 0x02) == 0) ? 1 : 0); // Set the state for the FX XY state and reverse it
            if (effectID == FAS_LOOPER_ID) {
              looper_block_detected = true;
              DEBUGMSG("Looper block detected.");
            }

            DEBUGMSG("effectID: " + String(effectID) + ", cc: " + String(cc) + ", cc_xy: " + String(cc_xy) + ", state: " + String(state));
          } // for (i)
          update_page = REFRESH_PAGE;
          break;
      } // switch (sxdata[5])
    }
  }
}


FLASHMEM void MD_FAS_class::check_PC_in(uint8_t program, uint8_t channel, uint8_t port) {  // Check incoming PC messages from  Called from MIDI:OnProgramChange

  // Check the source by checking the channel
  if ((port == MIDI_in_port) && (channel == MIDI_channel)) { // AXEFX sends a program change
    uint16_t new_patch = (CC00 * 128) + program;
    if (patch_number != new_patch) {
      set_patch_number(new_patch);
      write_sysex(FAS_GET_PRESET_NAME); // So the main display always show the correct patch
      //page_check();
      do_after_patch_selection();
      update_page = REFRESH_PAGE;
    }
  }
}


// Detection of the Axe-FX

FLASHMEM void MD_FAS_class::send_alternative_identity_request(uint8_t check_device_no) {
  if ((enabled == DEVICE_DETECT) && (!connected)) {
    uint8_t sysexmessage[8] = {0xF0, 0x00, 0x01, 0x74, 0x7F, 0x00, 0x7A, 0xF7}; // Will get any fractal device to respond - The Fractal editor uses this command as well
    check_sysex_delay();
    MIDI_send_sysex(sysexmessage, 8, Current_MIDI_out_port);
  }
}

FLASHMEM void MD_FAS_class::set_type(uint8_t device_type) { // Is run just before we connect.
  model_number = device_type;
  switch (device_type) { // First determine which Fractal Audio device has connected...
    case FAS_MODEL_STANDARD: // Axe-Fx Standard
      strcpy(device_name, "AF1");
      strcpy(full_device_name, "Fractal AF STD");
      patch_min = FAS_PATCH_MIN;
      patch_max = AXEFX1_PATCH_MAX;
      break;
    case FAS_MODEL_ULTRA: //Axe-Fx Ultra
      strcpy(device_name, "AF1U");
      strcpy(full_device_name, "Fractal AF ULTRA");
      patch_min = FAS_PATCH_MIN;
      patch_max = AXEFX1U_PATCH_MAX;
      looper_pre = true; // So Ultra will start with delay 1
      break;
    case FAS_MODEL_AF2: //Axe-Fx II
      strcpy(device_name, "AF2");
      strcpy(full_device_name, "Fractal AF 2");
      patch_min = FAS_PATCH_MIN;
      patch_max = AXEFX2_PATCH_MAX;
      break;
    case FAS_MODEL_FX8: //FX8
      strcpy(device_name, "FX8");
      strcpy(full_device_name, "Fractal FX8");
      patch_min = FAS_PATCH_MIN;
      patch_max = FX8_PATCH_MAX;
      break;
    case FAS_MODEL_XL: //Axe-Fx II XL
      strcpy(device_name, "AF2XL");
      strcpy(full_device_name, "Fractal AF 2 XL");
      patch_min = FAS_PATCH_MIN;
      patch_max = AXEFX2XL_PATCH_MAX;
      break;
    case FAS_MODEL_XLP: //Axe-Fx II XL+
      strcpy(device_name, "AF2+");
      strcpy(full_device_name, "Fractal AF 2 XL+");
      patch_min = FAS_PATCH_MIN;
      patch_max = AXEFX2XLP_PATCH_MAX;
      break;
    case FAS_MODEL_AX8: //AX8
      strcpy(device_name, "AX8");
      strcpy(full_device_name, "Fractal AX8");
      patch_min = FAS_PATCH_MIN;
      patch_max = AX8_PATCH_MAX;
      break;
    case FAS_MODEL_FX8mk2: //FX8 mk2
      strcpy(device_name, "FX8m2");
      strcpy(full_device_name, "Fractal FX8 m2");
      patch_min = FAS_PATCH_MIN;
      patch_max = FX8M2_PATCH_MAX;
      break;
    case FAS_MODEL_AF3: //Axe-Fx III
      strcpy(device_name, "AF3");
      strcpy(full_device_name, "Fractal AF III");
      patch_min = FAS_PATCH_MIN;
      patch_max = AXEFX3_PATCH_MAX;
      break;
  }
}


FLASHMEM void MD_FAS_class::do_after_connect() {
  current_exp_pedal = 1;
  write_sysex(FAS_GET_PRESET_NUMBER);
  write_sysex(FAS_GET_PRESET_NAME); // So the main display always show the correct patch
  //write_sysex(FAS_EDITOR_MODE_ON); // Put the AXEFX in EDITOR mode
  do_after_patch_selection();
  update_page = REFRESH_PAGE;
}

FLASHMEM void MD_FAS_class::check_still_connected() {} // Never disconnect

// ********************************* Section 3: FAS common MIDI out functions ********************************************

FLASHMEM void MD_FAS_class::write_sysex(uint8_t byte1) {
  uint8_t sysexmessage[8] = {0xF0, 0x00, 0x01, 0x74, model_number, byte1, 0, 0xF7};
  sysexmessage[6] = calc_FS_checksum(sysexmessage, 6); // Calculate the Fractal Systems checksum over the first 6 bytes and add it to the message
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 8, MIDI_out_port);
}

FLASHMEM void MD_FAS_class::write_sysex(uint8_t byte1, uint8_t byte2) {
  uint8_t sysexmessage[9] = {0xF0, 0x00, 0x01, 0x74, model_number, byte1, byte2, 0, 0xF7};
  sysexmessage[7] = calc_FS_checksum(sysexmessage, 7); // Calculate the Fractal Systems checksum over the first 7 bytes and add it to the message
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 9, MIDI_out_port);
}

FLASHMEM void MD_FAS_class::write_sysex(uint8_t byte1, uint8_t byte2, uint8_t byte3) {
  uint8_t sysexmessage[10] = {0xF0, 0x00, 0x01, 0x74, model_number, byte1, byte2, byte3, 0, 0xF7};
  sysexmessage[8] = calc_FS_checksum(sysexmessage, 8); // Calculate the Fractal Systems checksum over the first 8 bytes and add it to the message
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 10, MIDI_out_port);
}

FLASHMEM void MD_FAS_class::write_sysex(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4, uint8_t byte5, uint8_t byte6, uint8_t byte7, uint8_t byte8, uint8_t byte9) {
  uint8_t sysexmessage[16] = {0xF0, 0x00, 0x01, 0x74, model_number, byte1, byte2, byte3, byte4, byte5, byte6, byte7, byte8, byte9, 0, 0xF7};
  sysexmessage[14] = calc_FS_checksum(sysexmessage, 14); // Calculate the Fractal Systems checksum over the first 14 bytes and add it to the message
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 16, MIDI_out_port);
}

FLASHMEM uint8_t MD_FAS_class::calc_FS_checksum(const unsigned char* sxdata, short unsigned int sxlength) {
  // In order to calculate the Checksum, you basically have to XOR every byte from the start of the SysEx message, up to the character BEFORE the terminating F7 byte. For example, to send the following SysEx message (to fetch a preset name):
  // F0 00 01 74 03 0F F7
  // We would have to XOR all the byte values from the starting 'F0' to the '0F' which is the second last byte:
  // 0xF0 ^ 0x00 ^ 0x01 ^ 0x74 ^ 0x03 ^ 0x0F = 0x89
  // Then, we would need to strip the leftmost bit from the result (by ANDing it to 0x7F):
  // 0x89 & 0x7F = 0x09
  // And, we add this byte (actually, a septet now) to the end of the SysEx string, BEFORE the terminating F7:
  // F0 00 01 74 03 0F 09 F7
  uint8_t cs = 0;
  for (uint8_t i = 0; i < sxlength; i++) cs ^= sxdata[i];
  return cs & 0x7F;
}

FLASHMEM void MD_FAS_class::set_bpm() {
  // If you want to send a BPM value to the Axe-Fx II/AX8 you can do this via Function 0x02 GET/SET_BLOCK_PARAMETER_VALUE.
  // To do this you will send the parameter to the "CONTROLLERS" Block (ID 141), Parameter ID 32, and the value would be a number between 30 and 250

  // Format of the 9 parameter bytes that make up the GET/SET_BLOCK_PARAMETER_VALUE message:
  // 0x02 Function ID (0x02)
  // 0xdd effect ID bits 6-0
  // 0xdd effect ID bits 13-7
  // 0xdd parameter ID bits 6-0
  // 0xdd parameter ID bits 13-7
  // 0xdd parameter value bits 6-0
  // 0xdd parameter value bits 13-7
  // 0xdd parameter value bits 15-14
  // 0x00 0=query value, 1=set value

#define CONTROLLERS_BLOCK_ID  141

  if (connected) {
    write_sysex(FAS_GET_SET_BLOCK_PARAMETER_VALUE, CONTROLLERS_BLOCK_ID & 0x7F, CONTROLLERS_BLOCK_ID >> 7, 32, 0, Setting.Bpm & 0x7F, Setting.Bpm >> 7, 0, 1);
  }
}

FLASHMEM void MD_FAS_class::start_tuner() {
  if (connected) {
    MIDI_send_CC(FAS_TUNER_CC, 127, MIDI_channel, MIDI_out_port);
  }
}

FLASHMEM void MD_FAS_class::stop_tuner() {
  if (connected) {
    MIDI_send_CC(FAS_TUNER_CC, 0, MIDI_channel, MIDI_out_port);
  }
}

// ********************************* Section 4: FAS program change ********************************************

FLASHMEM void MD_FAS_class::select_patch(uint16_t new_patch) {
  //if (new_patch == patch_number) unmute();
  prev_patch_number = patch_number;
  patch_number = new_patch;

  MIDI_send_CC(0, new_patch >> 7, MIDI_channel, MIDI_out_port);
  MIDI_send_PC(new_patch & 0x7F, MIDI_channel, MIDI_out_port);
  DEBUGMSG("out(AXEFX) PC" + String(new_patch)); //Debug
  do_after_patch_selection();
  update_page = REFRESH_PAGE;
}

FLASHMEM void MD_FAS_class::do_after_patch_selection() {
  request_onoff = false;
  is_on = connected;
  if (Setting.Send_global_tempo_after_patch_change == true) {
    set_bpm();
  }
  write_sysex(FAS_GET_FX_BLOCK_BYPASS_STATE); // Request the states of the effects
  write_sysex(FAS_GET_SCENE_STATUS, 0x7F); // Request the current scene
  Current_patch_number = patch_number;
  current_snapscene = 0;
  looper_reset();
  update_LEDS = true;
  update_main_lcd = true;
  //update_page = REFRESH_PAGE;
  //EEPROM.write(EEPROM_FAS_PATCH_NUMBER, patch_number);
  if (!PAGE_check_on_page(my_device_number, patch_number)) { // Check if patch is on the page
    update_page = REFRESH_PAGE;
  }
  else {
    update_page = REFRESH_FX_ONLY;
  }
}

FLASHMEM bool MD_FAS_class::request_patch_name(uint8_t sw, uint16_t number) {
  uint8_t msb = number >> 7;
  uint8_t lsb = number & 0x7F;
  write_sysex(FAS_GET_PRESET_NAME_BY_NUMBER, msb, lsb); // F0 00 01 74 03 15 7F 7F 13 F7
  last_requested_sysex_type = REQUEST_PATCH_NAME;
  last_requested_sysex_switch = sw;
  return false;
}

FLASHMEM void MD_FAS_class::request_current_patch_name() {
  write_sysex(FAS_GET_PRESET_NAME);
}

FLASHMEM void MD_FAS_class::number_format(uint16_t number, String & Output) {
  char BankChar = 65 + (number >> 7);
  uint16_t number_plus_one = number + 1;
  Output += BankChar + String(number_plus_one / 100) + String((number_plus_one / 10) % 10) + String(number_plus_one % 10);
}

FLASHMEM void MD_FAS_class::direct_select_format(uint16_t number, String & Output) {
  if (direct_select_state == 0) {
    char BankChar = 65 + ((bank_select_number * 100 + number * 10) >> 7);
    Output +=  BankChar + String(bank_select_number) + String(number) + "_";
  }
  else {
    char BankChar = 65 + ((bank_select_number * 10) >> 7);
    Output +=  BankChar + String(bank_select_number / 10) + String(bank_select_number % 10) + String(number);
  }
}

FLASHMEM void MD_FAS_class::unmute() {
  is_on = connected;
  MIDI_send_CC(FAS_BYPASS_CC, 0, MIDI_channel, MIDI_out_port);
}

FLASHMEM void MD_FAS_class::mute() {
  if ((US20_mode_enabled()) && (!is_always_on) && (is_on)) {
    is_on = false;
    MIDI_send_CC(FAS_BYPASS_CC, 127, MIDI_channel, MIDI_out_port);
  }
}

// ********************************* Section 5: FAS parameter control ********************************************

// The AxeFX has IA (Instant Access) and XY state switches.
// These buttons can be controlled through regular MIDI cc messages.
// Whenever the patch is changed on the AxeFX, we send the FAS_GET_FX_BLOCK_BYPASS_STATE command
// The AxeFX will answer with one long message, that contains 5 bytes for every block that is loaded on the AxeFX.
// From these 5 bytes, we can pull the CC number of the IA button, the CC number of the XY button and the current state of these buttons.
// The states are saved in the effect_state array. This array has one byte for every CC number from CC#37 to CC#121. Thee bytes can be 0: not used, 1: ON/Y or 2: OFF/X

// Procedures for the FAS_PARAMETER:
// 1. Load in SP array L load_page()
// 2. Request parameter state - in PAGE_request_current_switch()
// 3. Read parameter state - FAS_read_parameter() below
// 4. Press switch - FAS_parameter_press() below - also calls check_update_label()
// 5. Release switch - FAS_parameter_release() below - also calls check_update_label()

struct FAS_parameter_struct { // Combines all the data we need for controlling a parameter in a device
  char Name[13]; // The name for the label
  uint8_t Colour; // The colour for this effect.
};

const PROGMEM FAS_parameter_struct FAS_parameters[] = {
  {"Amp 1 Byp", FX_AMP_TYPE}, // CC #37
  {"Amp 2 Byp", FX_AMP_TYPE}, // CC #38
  {"Cabinet 1", FX_AMP_TYPE}, // CC #39
  {"Cabinet 2", FX_AMP_TYPE}, // etc.
  {"Chorus 1", FX_MODULATE_TYPE},
  {"Chorus 2", FX_MODULATE_TYPE},
  {"Compressor 1", FX_FILTER_TYPE},
  {"Compressor 2", FX_FILTER_TYPE},
  {"Crossover 1", FX_FILTER_TYPE},
  {"Crossover 2", FX_FILTER_TYPE},
  {"Delay 1", FX_DELAY_TYPE},
  {"Delay 2", FX_DELAY_TYPE},
  {"Drive 1", FX_DIST_TYPE},
  {"Drive 2", FX_DIST_TYPE},
  {"Enhancer 1", FX_FILTER_TYPE},
  {"Filter 1", FX_FILTER_TYPE},
  {"Filter 2", FX_FILTER_TYPE},
  {"Filter 3", FX_FILTER_TYPE},
  {"Filter 4", FX_FILTER_TYPE},
  {"Flanger 1", FX_MODULATE_TYPE},
  {"Flanger 2", FX_MODULATE_TYPE},
  {"Formant", FX_FILTER_TYPE},
  {"FX Loop", FX_MODULATE_TYPE},
  {"Gate 1", FX_FILTER_TYPE},
  {"Gate 2", FX_FILTER_TYPE},
  {"Graphic EQ 1", FX_FILTER_TYPE},
  {"Graphic EQ 2", FX_FILTER_TYPE},
  {"Graphic EQ 3", FX_FILTER_TYPE},
  {"Graphic EQ 4", FX_FILTER_TYPE},
  {"Megatap", FX_DELAY_TYPE},
  {"Multicomp 1", FX_FILTER_TYPE},
  {"Multicomp 2", FX_FILTER_TYPE},
  {"Multidly 1", FX_DELAY_TYPE},
  {"Multidly 2", FX_DELAY_TYPE},
  {"ParamEq 1", FX_FILTER_TYPE},
  {"ParamEq 2", FX_FILTER_TYPE},
  {"ParamEq 3", FX_FILTER_TYPE},
  {"ParamEq 4", FX_FILTER_TYPE},
  {"Phaser 1", FX_MODULATE_TYPE},
  {"Phaser 2", FX_MODULATE_TYPE},
  {"Pitch 1", FX_PITCH_TYPE},
  {"Pitch 2", FX_PITCH_TYPE},
  {"QChorus 1", FX_MODULATE_TYPE},
  {"QChorus 2", FX_MODULATE_TYPE},
  {"Resonator 1", FX_MODULATE_TYPE},
  {"Resonator 2", FX_MODULATE_TYPE},
  {"Reverb 1", FX_REVERB_TYPE},
  {"Reverb 2", FX_REVERB_TYPE},
  {"RingMod", FX_MODULATE_TYPE},
  {"Rotary 1", FX_MODULATE_TYPE},
  {"Rotary 2", FX_MODULATE_TYPE},
  {"Synth 1", FX_MODULATE_TYPE},
  {"Synth 2", FX_MODULATE_TYPE},
  {"Tremolo 1", FX_MODULATE_TYPE},
  {"Tremolo 2", FX_MODULATE_TYPE},
  {"Vocoder", FX_MODULATE_TYPE},
  {"Volume 1", FX_MODULATE_TYPE},
  {"Volume 2", FX_MODULATE_TYPE},
  {"Volume 3", FX_MODULATE_TYPE},
  {"Volume 4", FX_MODULATE_TYPE},
  {"Wah 1 Byp", FX_FILTER_TYPE},
  {"Wah 2 Byp", FX_FILTER_TYPE}, // CC #98 - last one supported by AXE-FX 1 and ultra
  {"Tonematch", FX_FILTER_TYPE},
  {"AMP 1 X/Y", FX_AMP_TYPE},
  {"AMP 2 X/Y", FX_AMP_TYPE},
  {"CAB 1 X/Y", FX_AMP_TYPE},
  {"CAB 2 X/Y", FX_AMP_TYPE},
  {"CHORUS 1 X/Y", FX_MODULATE_TYPE},
  {"CHORUS 2 X/Y", FX_MODULATE_TYPE},
  {"DELAY 1 X/Y", FX_DELAY_TYPE},
  {"DELAY 2 X/Y", FX_DELAY_TYPE},
  {"DRIVE 1 X/Y", FX_DIST_TYPE},
  {"DRIVE 2 X/Y", FX_DIST_TYPE},
  {"FLANGER1 X/Y", FX_MODULATE_TYPE},
  {"FLANGER2 X/Y", FX_MODULATE_TYPE},
  {"PHASER 1 X/Y", FX_MODULATE_TYPE},
  {"PHASER 2 X/Y", FX_MODULATE_TYPE},
  {"PITCH 1 X/Y", FX_PITCH_TYPE},
  {"PITCH 2 X/Y", FX_PITCH_TYPE},
  {"REVERB 1 X/Y", FX_REVERB_TYPE},
  {"REVERB 2 X/Y", FX_REVERB_TYPE},
  {"ROTARY 1 X/Y", FX_MODULATE_TYPE},
  {"ROTARY 2 X/Y", FX_MODULATE_TYPE},
  {"WAHWAH 1 X/Y", FX_FILTER_TYPE},
  {"WAHWAH 2 X/Y", FX_FILTER_TYPE}, // CC #121
  {"Ext 1", FX_DEFAULT_TYPE}, // CC #16 - number 85 in the list
  {"Ext 2", FX_DEFAULT_TYPE}, // CC #17
  {"Ext 3", FX_DEFAULT_TYPE}, // CC #18
  {"Ext 4", FX_DEFAULT_TYPE}, // CC #19
  {"Ext 5", FX_DEFAULT_TYPE}, // CC #20
  {"Ext 6", FX_DEFAULT_TYPE}, // CC #21
  {"Ext 7", FX_DEFAULT_TYPE}, // CC #22
  {"Ext 8", FX_DEFAULT_TYPE}, // CC #23
};

const uint16_t FAS_NUMBER_OF_PARAMETERS = sizeof(FAS_parameters) / sizeof(FAS_parameters[0]);

#define FAS_FIRST_XY_PEDAL 63
#define FAS_FIRST_EXTERNAL_PEDAL 85
#define FAS_FIRST_EXTERNAL_PEDAL_CC 16

FLASHMEM void MD_FAS_class::read_parameter_name(uint16_t number, String & Output) { // Called from menu
  if (number < number_of_parameters())  Output = FAS_parameters[number].Name;
  else Output = "?";
}

FLASHMEM void MD_FAS_class::read_parameter_value_name(uint16_t number, uint16_t value, String & Output) { // Called from menu
  if (number < FAS_FIRST_EXTERNAL_PEDAL) { // Show ON/OFF for IA and XY switches
    if (value == 1) Output += "ON";
    else Output += "OFF";
    return;
  }
  if (number < FAS_NUMBER_OF_PARAMETERS) { // External pedal
    Output += String(value); // Show value for external pedals
    return;
  }
  // Illegal value
  Output += "?";
}

FLASHMEM void MD_FAS_class::clear_FX_states() {
  memset(effect_state, 0, FAS_NUMBER_OF_FX);
}

FLASHMEM void MD_FAS_class::set_FX_state(uint8_t cc, bool state) {
  if (cc < FAS_FX_FIRST_CC) return; // Exit if the CC is smaller than FAS_FX_FIRST_CC (37)
  uint8_t index = cc - FAS_FX_FIRST_CC;
  if (index < FAS_NUMBER_OF_FX) effect_state[index] = (state == 0) ? 2 : 1; // Set state to 2 if state is zero and to 1 if state is not zero
}

// Toggle AXEFX stompbox parameter
FLASHMEM void MD_FAS_class::parameter_press(uint8_t Sw, Cmd_struct * cmd, uint16_t number) {

  if (number >= FAS_NUMBER_OF_PARAMETERS) return; // Quit if number is out of range

  uint8_t value = SCO_return_parameter_value(Sw, cmd); // Value is toggled already

  // Send cc MIDI command to AXEFX
  uint8_t cc;
  if (number < FAS_FIRST_EXTERNAL_PEDAL) { // IA or XY switch
    cc = number + FAS_FX_FIRST_CC;
    if (value == 1) MIDI_send_CC(cc, 127, MIDI_channel, MIDI_out_port);
    else  MIDI_send_CC(cc, 0, MIDI_channel, MIDI_out_port);
  }
  else { // External pedal
    cc = number - FAS_FIRST_EXTERNAL_PEDAL + FAS_FIRST_EXTERNAL_PEDAL_CC;
    MIDI_send_CC(cc, value, MIDI_channel, MIDI_out_port);
  }

  if (value == 1) effect_state[number] = 1; // Store the new effect state
  else effect_state[number] = 2;

  // Show message
  check_update_label(Sw, value);
  LCD_show_popup_label(SP[Sw].Label, ACTION_TIMER_LENGTH);

  if ((SP[Sw].Latch != RANGE) && (SP[Sw].Latch != UPDOWN)) update_page = REFRESH_PAGE;
}

FLASHMEM void MD_FAS_class::parameter_release(uint8_t Sw, Cmd_struct * cmd, uint16_t number) {
  // Work out state of pedal
  if (SP[Sw].Latch == MOMENTARY) {
    effect_state[number] = 2; // Switch state off
    SP[Sw].State = 2;
    uint8_t cc = number + FAS_FX_FIRST_CC;
    MIDI_send_CC(cc, 0, MIDI_channel, MIDI_out_port);

    update_page = REFRESH_PAGE;
  }
}

FLASHMEM void MD_FAS_class::read_parameter_title(uint16_t number, String &Output) {
  if (number < FAS_NUMBER_OF_PARAMETERS) {
    if (number < FAS_FIRST_EXTERNAL_PEDAL) { // IA or XY switch
      Output += FAS_parameters[number].Name; // Set the label
    }
    else { // External pedal
      // How to read the value of an external parameter from the AxeFX ???? Now they are just made zero...
      // Anyway: it doesn't matter when we use these just for expression pedals.
      Output += device_name;
      Output += ' ';
      Output += FAS_parameters[number].Name;
    }
  }
}

FLASHMEM bool MD_FAS_class::request_parameter(uint8_t sw, uint16_t number) {
  //Effect type and state are stored in the effect_state array
  //Effect can have three states: 0 = no effect, 1 = on, 2 = off

  String msg;
  if (number < FAS_NUMBER_OF_PARAMETERS) {
    if (number < FAS_FIRST_EXTERNAL_PEDAL) { // IA or XY switch
      SP[sw].State = effect_state[number]; // Read the effect state from the array
      if (number >= FAS_FIRST_XY_PEDAL) {
        if (effect_state[number] == 1) LCD_set_SP_label(sw, " X [Y]"); // Set the label
        else LCD_set_SP_label(sw, "[X] Y ");
      }
      else LCD_clear_SP_label(sw);
    }
    else { // External pedal
      // How to read the value of an external parameter from the AxeFX ???? Now they are just made zero...
      // Anyway: it doesn't matter when we use these just for expression pedals.
      String msg = "";
      LCD_add_3digit_number(effect_state[number], msg);
      LCD_set_SP_label(sw, msg);
    }

    SP[sw].Colour = FAS_parameters[number].Colour; // Set the FX colour
  }

  return true; // Move to next switch is true
}

FLASHMEM void MD_FAS_class::check_update_label(uint8_t Sw, uint8_t value) { // Updates the label for extended sublists
  uint16_t index = SP[Sw].PP_number; // Read the parameter number (index to AXEFX-parameter array)
  if ((index >= FAS_FIRST_EXTERNAL_PEDAL) && (index < FAS_NUMBER_OF_PARAMETERS)) {
    String msg = "";
    LCD_add_3digit_number(value, msg);
    LCD_set_SP_label(Sw, msg);

    //Update the current switch label
    update_lcd = Sw;
  }
}


FLASHMEM uint16_t MD_FAS_class::number_of_parameters() {
  return FAS_NUMBER_OF_PARAMETERS;
}

FLASHMEM uint8_t MD_FAS_class::number_of_values(uint16_t parameter) {
  if (parameter < FAS_FIRST_EXTERNAL_PEDAL) return 2; // IA or XY switch
  if (parameter < FAS_NUMBER_OF_PARAMETERS) return 128; // External pedal
  return 0; // Out of range
}

FLASHMEM uint16_t MD_FAS_class::number_of_parbank_parameters() {
  if (enabled == DEVICE_DETECT) return number_of_active_blocks * 2; // Every active block has a bypass and an XY state
  else return FAS_NUMBER_OF_PARAMETERS; // When not connected bi-directionally, just show all the parameters
}

FLASHMEM uint16_t MD_FAS_class::get_parbank_parameter_id(uint16_t par_number) {
  //Find the correct parameter number for this parameter number
  if (enabled == DEVICE_DETECT) {
    uint8_t active_fx_number = 0;
    for (uint8_t i = 0; i < FAS_NUMBER_OF_FX; i++) { // We go through the effect_state array and look for effects that are enabled
      if (effect_state[i] != 0) { // If an effect is enabled (state is 1 or 2 in that case)
        if (active_fx_number == par_number) return i; // And we have found the right effect, return the index number to this effect
        active_fx_number++;
      }
    }
    return NO_RESULT; // An effect number that is way out of range - not all FX have XY states, so this can happen
  }
  else { // When not bi-directional, show all the effects
    return par_number;
  }
}

FLASHMEM void MD_FAS_class::move_expression_pedal(uint8_t sw, uint8_t value, uint8_t exp_pedal) {
  if (exp_pedal == 0) exp_pedal = current_exp_pedal;
  if (exp_pedal == 0) return;
  LCD_show_bar(0, value, 0); // Show it on the main display
  MIDI_send_CC(FAS_FIRST_EXTERNAL_PEDAL_CC + exp_pedal - 1, value, MIDI_channel, MIDI_out_port);
  check_update_label(sw, value);
  LCD_show_popup_label(SP[sw].Label, ACTION_TIMER_LENGTH);
  update_page = REFRESH_PAGE; // To update the other switch states, we re-load the current page
}

FLASHMEM bool MD_FAS_class::request_exp_pedal(uint8_t sw, uint8_t exp_pedal) {
  if (exp_pedal == 0) exp_pedal = current_exp_pedal;
  if (exp_pedal > 0) {
    uint8_t number = FAS_FIRST_EXTERNAL_PEDAL + exp_pedal - 1;
    SP[sw].PP_number = number;
    String msg;
    msg = device_name;
    msg += ' ';
    msg += FAS_parameters[number].Name;
    LCD_set_SP_label(sw, msg);
  }
  return true;
}
// ********************************* Section 6: FAS scene and looper control ********************************************

FLASHMEM void MD_FAS_class::get_snapscene_title(uint8_t number, String & Output) {
  Output += "SCENE " + String(number);
}

/*FLASHMEM bool MD_FAS_class::request_snapscene_name(uint8_t sw, uint8_t sw1, uint8_t sw2, uint8_t sw3) {

  return true;
  }*/


/*FLASHMEM void MD_FAS_class::set_snapscene_name(uint8_t number, String &Output) {
  Output += "SCENE " + String(number);
  }*/

FLASHMEM void MD_FAS_class::set_snapscene(uint8_t sw, uint8_t number) {
  if (!is_on) unmute();
  if ((number < 1) || (number > 8)) return;
  current_snapscene = number;
  MIDI_send_CC(FAS_SCENE_SELECT_CC, number - 1, MIDI_channel, MIDI_out_port);
  MIDI_send_current_snapscene(my_device_number, current_snapscene);
}

FLASHMEM void MD_FAS_class::show_snapscene(uint8_t  number) {
  if ((number < 1) || (number > 8)) return;
  if (number == current_snapscene) return;
  current_snapscene = number;
}

FLASHMEM bool MD_FAS_class::looper_active() { // Is there a way to detect if the current patch contains a looper block?
  return looper_block_detected;
}

struct FAS_looper_cc_struct { // Combines all the data we need for controlling a parameter in a device
  uint8_t cc; // The cc number
  uint8_t value; // The value.
};

const PROGMEM FAS_looper_cc_struct FAS_Ultra_looper1_cc[] = { // Table with the cc messages for looper 1 of the AxeFX ultra
  {0, 0},    // HIDE
  {0, 0},    // SHOW
  {25, 0},   // STOP
  {25, 127}, // PLAY
  {24, 127}, // REC
  {27, 127}, // OVERDUB
  {28, 0},   // FORWARD
  {28, 127}, // REVERSE
  {0, 0},    // FULL_SPEED
  {0, 127},  // HALF_SPEED
  {0, 127},  // UNDO
  {0, 127},  // REDO
  {26, 127}, // PLAY_ONCE
  {0, 0},    // PRE
  {0, 0},    // POST
};

const uint8_t AXEFXU_LOOPER1_NUMBER_OF_CCS = sizeof(FAS_Ultra_looper1_cc) / sizeof(FAS_Ultra_looper1_cc[0]);

const PROGMEM FAS_looper_cc_struct FAS_Ultra_looper2_cc[] = { // Table with the cc messages for looper 2 of the AxeFX ultra
  {0, 0},    // HIDE
  {0, 0},    // SHOW
  {30, 0},   // STOP
  {30, 127}, // PLAY
  {29, 127}, // REC
  {32, 127}, // OVERDUB
  {33, 0},   // FORWARD
  {33, 127}, // REVERSE
  {0, 0},    // FULL_SPEED
  {0, 127},  // HALF_SPEED
  {0, 127},  // UNDO
  {0, 127},  // REDO
  {31, 127}, // PLAY_ONCE
  {0, 0},    // PRE
  {0, 0},    // POST
};

const uint8_t AXEFXU_LOOPER2_NUMBER_OF_CCS = sizeof(FAS_Ultra_looper2_cc) / sizeof(FAS_Ultra_looper2_cc[0]);

const PROGMEM FAS_looper_cc_struct AXEFX2_looper_cc[] = { // Table with the cc messages for the AxeFX 2 and up, FX8 and AX8
  {33, 0},    // HIDE
  {33, 127},  // SHOW
  {29, 0},    // STOP
  {29, 127},  // PLAY
  {28, 127},  // REC
  {31, 127},  // OVERDUB
  {32, 0},    // FORWARD
  {32, 127},  // REVERSE
  {120, 0},   // FULL_SPEED
  {120, 127}, // HALF_SPEED
  {121, 127}, // UNDO
  {121, 127}, // REDO
  {30, 127},  // PLAY_ONCE
  {0, 0},     // PRE
  {0, 0},     // POST
};

const uint8_t AXEFX2_LOOPER_NUMBER_OF_CCS = sizeof(AXEFX2_looper_cc) / sizeof(AXEFX2_looper_cc[0]);

// The Axe-Fx III does not have pre-set MIDI CCs. These can be selected in the MIDI menu.

FLASHMEM void MD_FAS_class::send_looper_cmd(uint8_t cmd) {
  // Execute commands for looper of the AxeFX2 and up, FX8 and AX8
  if (model_number >= FAS_MODEL_AF2) {
    if (cmd < AXEFX2_LOOPER_NUMBER_OF_CCS) {
      if (AXEFX2_looper_cc[cmd].cc > 0) MIDI_send_CC(AXEFX2_looper_cc[cmd].cc, AXEFX2_looper_cc[cmd].value, MIDI_channel, MIDI_out_port);
    }
  }
  // Execute commands for looper 1 of the AxeFX Ultra if looper_pre is on
  if ((model_number == FAS_MODEL_ULTRA) && (looper_pre)) {
    if (cmd < AXEFXU_LOOPER1_NUMBER_OF_CCS) {
      if (FAS_Ultra_looper1_cc[cmd].cc > 0) MIDI_send_CC(FAS_Ultra_looper1_cc[cmd].cc, FAS_Ultra_looper1_cc[cmd].value, MIDI_channel, MIDI_out_port);
    }
  }
  // Execute commands for looper 2 of the AxeFX Ultra if looper_pre is off
  if ((model_number == FAS_MODEL_ULTRA) && (!looper_pre)) {
    if (cmd < AXEFXU_LOOPER2_NUMBER_OF_CCS) {
      if (FAS_Ultra_looper2_cc[cmd].cc > 0) MIDI_send_CC(FAS_Ultra_looper2_cc[cmd].cc, FAS_Ultra_looper2_cc[cmd].value, MIDI_channel, MIDI_out_port);
    }
  }
}
