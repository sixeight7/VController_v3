// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: GR55 Initialization
// Section 2: GR55 common MIDI in functions
// Section 3: GR55 common MIDI out functions
// Section 4: GR55 program change
// Section 5: GR55 parameter control
// Section 6: GR55 assign control
// Section 7: GR55 preset names
// Section 8: GR55 Scene control

// ********************************* Section 1: GR55 initialization ********************************************

//#define SKIP_GR55_PRESET_NAMES // Will not compile the hardcoded preset names for the preset patches - will save around 8k of space (3% on Teensy 3.2)

// Roland GR-55 settings:
#define GR55_MIDI_CHANNEL 8
#define GR55_MIDI_PORT MIDI1_PORT
#define GR55_PATCH_MIN 0
#define GR55_PATCH_MAX 656 // Assuming we are in guitar mode!!! When the GR-55 is in bass mode the number of patches is less.
#define GR55_NUMBER_OF_USER_PATCHES 297

//Sysex messages for Roland GR-55
#define GR55_REQUEST_MODE 0x18000000, 1 //Is 00 in guitar mode, 01 in bass mode (Gumtown in town :-)
#define GR55_REQUEST_CURRENT_PATCH_NAME 0x18000001, 16 // Request 16 bytes for current patch name
#define GR55_REQUEST_CURRENT_PATCH_NUMBER 0x01000000, 2 // Request current patch number
#define GR55_CTL_ADDRESS 0x18000011
#define GR55_EXP_SW_ADDRESS 0x1800004D

#define GR55_TEMPO 0x1800023C  // Accepts values from 40 bpm - 250 bpm

#define GR55_PCM1_SW 0x18002003 // The address of the synth1 switch
#define GR55_PCM2_SW 0x18002103 // The address of the synth1 switch
#define GR55_COSM_GUITAR_SW 0x1800100A // The address of the COSM guitar switch
#define GR55_NORMAL_PU_SW 0x18000232 // The address of the Normal PU switch
#define GR55_COSM_DATA 0x18001000 // We request also the guitar and bass types by reading the entire area of data

#define GR55_PCM1_TYPE 0x18002000
#define GR55_PCM2_TYPE 0x18002100

#define GR55_NUMBER_OF_FACTORY_PATCHES 360
#define GR55_PATCH_MAX_BASS_MODE 404

#define GR55_PATCH_BASE 0x18000224
#define GR55_PATCH_BASE_LENGTH (0x0247 - 0x0224)

#define GR55_SYSEX_DELAY 10 // Time between sysex messages in msec

#define GR55_FIRST_SCENE_ASSIGN_SOURCE_CC 21
#define GR55_READ_SCENE_MIDI_TIMER_LENGTH 1000
#define GR55_BASS_MODE_NUMBER_OFFSET 0x1000 // Bass mode patches are stored with higher number

#define GR55_GM_PATCH_NUMBER 0x18001127 // For storing patch number in guitar mode - is GR300 env mod sense and attack in bass mode
#define GR55_BM_PATCH_NUMBER 0x18001050 // For storing patch number in bass mode - is GR300 env mod sense and attack in guitar mode

#define GR55_SOURCE_CC95 71
#define GR55_SOURCE_CC94 70
#define GR55_SOURCE_INT_PEDAL 4

// Initialize device variables
// Called at startup of VController
FLASHMEM void MD_GR55_class::init() // Default values for variables
{
  MD_base_class::init();

  // Boss GR-55 variables:
  enabled = DEVICE_DETECT; // Default value
  strcpy(device_name, "GR55");
  strcpy(full_device_name, "Roland GR-55");
  patch_min = GR55_PATCH_MIN;
  patch_max = GR55_PATCH_MAX;
  max_times_no_response = MAX_TIMES_NO_RESPONSE; // The number of times the GR-55 does not have to respond before disconnection
  preset_banks = 40; // Default number of preset banks is 40. When we are in bass mode, there are only 12.
  gr55_pcm1_mute = 0;
  gr55_pcm2_mute = 0;
  COSM_gtr_mute = 0;
  normal_pu_mute = 0;
  sysex_delay_length = GR55_SYSEX_DELAY; // time between sysex messages (in msec)
  my_LED_colour = 3; // Default value: blue
  MIDI_channel = GR55_MIDI_CHANNEL; // Default value
  MIDI_port_manual = MIDI_port_number(GR55_MIDI_PORT); // Default value
  is_always_on = true; // Default value
  current_exp_pedal = 1;
#if defined(CONFIG_VCTOUCH)
  my_device_page1 = GR55_DEFAULT_VCTOUCH_PAGE1; // Default value
  my_device_page2 = GR55_DEFAULT_VCTOUCH_PAGE2; // Default value
  my_device_page3 = GR55_DEFAULT_VCTOUCH_PAGE3; // Default value
  my_device_page4 = GR55_DEFAULT_VCTOUCH_PAGE4; // Default value
#elif defined(CONFIG_VCMINI)
  my_device_page1 = GR55_DEFAULT_VCMINI_PAGE1; // Default value
  my_device_page2 = GR55_DEFAULT_VCMINI_PAGE2; // Default value
  my_device_page3 = GR55_DEFAULT_VCMINI_PAGE3; // Default value
  my_device_page4 = GR55_DEFAULT_VCMINI_PAGE4; // Default value
#elif defined (CONFIG_CUSTOM)
  my_device_page1 = GR55_DEFAULT_CUSTOM_PAGE1; // Default value
  my_device_page2 = GR55_DEFAULT_CUSTOM_PAGE2; // Default value
  my_device_page3 = GR55_DEFAULT_CUSTOM_PAGE3; // Default value
  my_device_page4 = GR55_DEFAULT_CUSTOM_PAGE4; // Default value
#else
  my_device_page1 = GR55_DEFAULT_VC_PAGE1; // Default value
  my_device_page2 = GR55_DEFAULT_VC_PAGE2; // Default value
  my_device_page3 = GR55_DEFAULT_VC_PAGE3; // Default value
  my_device_page4 = GR55_DEFAULT_VC_PAGE4; // Default value
#endif

  initialize_patch_space();
  initialize_scene_assigns();

#ifdef IS_VCTOUCH
  device_pic = img_GR55;
#endif
}

FLASHMEM void MD_GR55_class::update() {
  check_read_scene_midi_timer();
  check_int_pdl_cc_timer();
}

// ********************************* Section 2: GR55 common MIDI in functions ********************************************

FLASHMEM void MD_GR55_class::check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port) {  // Check incoming sysex messages from  Called from MIDI:OnSysEx/OnSerialSysEx

  // Check if it is a message from a GR-55
  if ((port == MIDI_in_port) && (sxdata[1] == 0x41) && (sxdata[2] == MIDI_device_id) && (sxdata[3] == 0x00) && (sxdata[4] == 0x00) && (sxdata[5] == 0x53) && (sxdata[6] == 0x12)) {
    uint32_t address = (sxdata[7] << 24) + (sxdata[8] << 16) + (sxdata[9] << 8) + sxdata[10]; // Make the address 32 bit

    // Check checksum
    uint16_t sum = 0;
    for (uint8_t i = 7; i < sxlength - 2; i++) sum += sxdata[i];
    bool checksum_ok = (sxdata[sxlength - 2] == calc_Roland_checksum(sum));

    // Check if it is the patch number
    if ((address == 0x01000000) && (checksum_ok)) {
      if (patch_number != sxdata[12]) { //Right after a patch change the patch number is sent again. So here we catch that message.
        uint16_t new_patch = sxdata[11] * 128 + sxdata[12];
        if (new_patch > 2047) new_patch = new_patch - 1751; // There is a gap of 1752 patches in the numbering system of the GR-55. This will close it.
        set_patch_number(new_patch);
        //page_check();
        do_after_patch_selection();
        update_page = REFRESH_PAGE;
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
          if (sxlength == 29) { // Check if we received the full message
            for (uint8_t count = 0; count < 16; count++) {
              SP[last_requested_sysex_switch].Label[count] = static_cast<char>(sxdata[count + 11]); //Add ascii character to the SP.Label String
            }
            if (SP[last_requested_sysex_switch].PP_number == patch_number) {
              current_patch_name = SP[last_requested_sysex_switch].Label; // Load patchname when it is read
              update_main_lcd = true; // And show it on the main LCD
            }
            DEBUGMSG(SP[last_requested_sysex_switch].Label);
            PAGE_request_next_switch();
          }
          else {
            PAGE_request_current_switch();
          }
          break;
        case REQUEST_PARAMETER_TYPE:
          read_parameter(last_requested_sysex_switch, sxdata[11], sxdata[12]);
          if ((SP[last_requested_sysex_switch].Type != ASSIGN) && (last_requested_sysex_address == 0x18000011)) { // if requested parameter is CTL pedal, check if latching type should be momentary
            if (((sxdata[12] == 1) && (sxdata[14] == 1)) || (sxdata[12] == 15)) { // Check for momentary switch types (Hold (1) + momentary or LED_MOMENT (15))
              SP[last_requested_sysex_switch].Latch = MOMENTARY;
            }
            else {
              SP[last_requested_sysex_switch].Latch = TOGGLE;
            }
          }
          PAGE_request_next_switch();
          break;
        case REQUEST_ASSIGN_TYPE:
          read_current_assign(last_requested_sysex_switch, address, sxdata, sxlength);
          break;
      }
    }

    // Check if it is the patch name (address: 0x18, 0x00, 0x00, 0x01)
    if ((sxdata[6] == 0x12) && (address == 0x18000001) && (checksum_ok)) {
      current_patch_name = "";
      for (uint8_t count = 11; count < 27; count++) {
        current_patch_name += static_cast<char>(sxdata[count]); //Add ascii character to Patch Name String
      }
      update_main_lcd = true;
      if (popup_patch_name) {
        LCD_show_popup_label(current_patch_name, ACTION_TIMER_LENGTH);
        popup_patch_name = false;
      }
    }

    // Check if it is the instrument on/off states
    if (checksum_ok) check_inst_switch_states(sxdata, sxlength);

    // Check if the GR55 is in bass mode (address: 0x18, 0x00, 0x00, 0x00)
    if ((sxdata[6] == 0x12) && (address == 0x18000000) && (sxdata[11] == 0x01) && (checksum_ok)) {
      bass_mode = true; // In bass mode
      preset_banks = 12;
      if (patch_max > GR55_PATCH_MAX_BASS_MODE) patch_max = GR55_PATCH_MAX_BASS_MODE;
      DEBUGMSG("GR55 is in Bass Mode");
    }

    // Check if it is the expression pedal settings
    if ((sxdata[6] == 0x12) && (address == 0x1800001F) && (checksum_ok)) {
      exp_type = sxdata[11]; // EXP type
      exp_on_type = sxdata[34]; // EXP_ON type
      exp_sw_type = sxdata[58]; // EXP SW type
      if (sxdata[57] == 0x01) current_exp_pedal = 2; // Check EXP_SW state
      else current_exp_pedal = 1;
      update_exp_label(last_requested_sysex_switch);
      PAGE_request_next_switch();
    }

    // Check for assign data
    if (read_full_assign_number < GR55_NUMBER_OF_ASSIGNS) {
      if (address == calculate_full_assign_address(read_full_assign_number)) read_full_assign(read_full_assign_number, address, sxdata, sxlength);
    }

    // Check for scene parameter data
    if (read_scene_parameter_number > 0) {
      if ((address == read_scene_parameter_address) && (checksum_ok)) read_scene_message(read_scene_parameter_number, sxdata[11]);
    }

    if (address == GR55_PCM1_TYPE) {
      read_scene_pcm_type(0, (sxdata[12] << 7) + sxdata[13]);
    }
    if (address == GR55_PCM2_TYPE) {
      read_scene_pcm_type(2, (sxdata[12] << 7) + sxdata[13]);
    }

    if ((address == GR55_GM_PATCH_NUMBER) || (address == GR55_BM_PATCH_NUMBER)) {
      check_pc_after_patch_load((sxdata[11] << 7) + sxdata[12]);
    }

    if (address > 0x20000000) {
      if (((address & 0xFFFF) == (GR55_GM_PATCH_NUMBER & 0xFFFF)) || ((address & 0xFFFF) == (GR55_BM_PATCH_NUMBER & 0xFFFF))) {
        do_pc_crosscheck((sxdata[11] << 7) + sxdata[12]);
      }
    }

  }
}


FLASHMEM void MD_GR55_class::check_PC_in(uint8_t program, uint8_t channel, uint8_t port) { // Check incoming PC messages from  Called from MIDI:OnProgramChange

  // Check the source by checking the channel
  if ((port == MIDI_in_port) && (channel == MIDI_channel)) { // GR55 sends a program change
    uint16_t new_patch = (CC00 * 128) + program;
    if (patch_number != new_patch) {
      if (new_patch > 2047) new_patch = new_patch - 1751; // There is a gap of 1752 patches in the numbering system of the GR-55. This will close it.
      set_patch_number(new_patch);
      request_sysex(GR55_REQUEST_CURRENT_PATCH_NAME); // So the main display always show the correct patch
      //page_check();
      do_after_patch_selection();
      check_ample_time_between_pc_messages_timer = millis() + CHECK_AMPLE_TIME_BETWEEN_PC_MESSAGES_TIME;
      update_page = REFRESH_PAGE;
    }
    else if (millis() > check_ample_time_between_pc_messages_timer) { // Detect WRITE pressed on GR55 by checking current PC number sent.
      bool saved = store_patch(new_patch);
      if (saved) LCD_show_popup_label("Patch saved.", MESSAGE_TIMER_LENGTH);
    }
  }
}

// Detection of GR-55

FLASHMEM void MD_GR55_class::identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t in_port, uint8_t out_port) {
  // Check if it is a GR-55
  if ((sxdata[5] == 0x41) && (sxdata[6] == 0x53) && (sxdata[7] == 0x02) && (enabled == DEVICE_DETECT)) {
    no_response_counter = 0;
    if (connected == false) connect(sxdata[2], in_port, out_port); //Byte 2 contains the correct device ID
  }
}

FLASHMEM void MD_GR55_class::do_after_connect() {
  request_sysex(GR55_REQUEST_CURRENT_PATCH_NUMBER);
  request_sysex(GR55_REQUEST_CURRENT_PATCH_NAME); // So the main display always show the correct patch
  request_sysex(GR55_REQUEST_MODE); // Check if the GR-55 is in bass mode
  do_after_patch_selection();
  update_page = RELOAD_PAGE;
}


// ********************************* Section 3: GR55 common MIDI out functions ********************************************

FLASHMEM void MD_GR55_class::write_sysex(uint32_t address, uint8_t value) { // For sending one data byte

  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t checksum = calc_Roland_checksum(ad[3] + ad[2] + ad[1] + ad[0] + value); // Calculate the Roland checksum
  uint8_t sysexmessage[14] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x53, 0x12, ad[3], ad[2], ad[1], ad[0], value, checksum, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 14, MIDI_out_port);
}

FLASHMEM void MD_GR55_class::write_sysex(uint32_t address, uint8_t value1, uint8_t value2) { // For sending two data bytes

  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t checksum = calc_Roland_checksum(ad[3] + ad[2] + ad[1] + ad[0] + value1 + value2); // Calculate the Roland checksum
  uint8_t sysexmessage[15] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x53, 0x12, ad[3], ad[2], ad[1], ad[0], value1, value2, checksum, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 15, MIDI_out_port);
}

FLASHMEM void MD_GR55_class::write_sysex(uint32_t address, uint8_t value1, uint8_t value2, uint8_t value3) { // For sending two data bytes

  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t checksum = calc_Roland_checksum(ad[3] + ad[2] + ad[1] + ad[0] + value1 + value2 + value3); // Calculate the Roland checksum
  uint8_t sysexmessage[16] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x53, 0x12, ad[3], ad[2], ad[1], ad[0], value1, value2, value3, checksum, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 16, MIDI_out_port);
}

FLASHMEM void MD_GR55_class::request_sysex(uint32_t address, uint8_t no_of_bytes) {
  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t checksum = calc_Roland_checksum(ad[3] + ad[2] + ad[1] + ad[0] +  no_of_bytes); // Calculate the Roland checksum
  uint8_t sysexmessage[17] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x53, 0x11, ad[3], ad[2], ad[1], ad[0], 0x00, 0x00, 0x00, no_of_bytes, checksum, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 17, MIDI_out_port);
}

FLASHMEM void MD_GR55_class::set_bpm() {
  if (connected) {
    write_sysex(GR55_TEMPO, Setting.Bpm >> 4, Setting.Bpm & 0x0F); // Tempo is modulus 16. It's all so very logical. NOT.
  }
}

FLASHMEM void MD_GR55_class::start_tuner() {
  if (connected) {
    was_on = is_on;
    mute_now(); // Mute the GR-55. There is no documentation on how to start the tuner on the GR55 through sysex.
  }
}

FLASHMEM void MD_GR55_class::stop_tuner() {
  if (was_on) {
    unmute(); // Unmute the GR-55. There is no documentation on how to start the tuner on the GR55 through sysex.
  }
}

/*FLASHMEM void MD_GR55_class::sendGR55BankPatch(uint16_t patchno)
  {
  // Experimental method: https://www.vguitarforums.com/smf/index.php?topic=23298.25
  byte SysArray[135]; // Create 9 messages of 15 bytes = 135 bytes
  const uint8_t byte_eight[9] = { 0x00, 0x03, 0x06, 0x07, 0x10, 0x20, 0x21, 0x30, 0x31};

  uint8_t bank = patchno >> 7;
  uint8_t patch = patchno & 0x7F;
  int sumofdata = 0x01 + bank + patch;

  uint8_t b = 0;
  for (uint8_t m = 0; m < 9; m++) {
    SysArray[b++] = 0xF0;
    SysArray[b++] = 0x41;
    SysArray[b++] = 0x10;
    SysArray[b++] = 0x00;
    SysArray[b++] = 0x00;
    SysArray[b++] = 0x53;
    SysArray[b++] = 0x12;
    SysArray[b++] = 0x01;
    SysArray[b++] = byte_eight[m]; // This byte is unique for each message
    SysArray[b++] = 0x00;
    SysArray[b++] = 0x00;
    SysArray[b++] = bank;
    SysArray[b++] = patch;
    SysArray[b++] = (128 - ((sumofdata + byte_eight[m]) & 0x7F));
    SysArray[b++] = 0xF7;
  }
  MIDI_send_sysex(SysArray, 135, MIDI_out_port);
  }*/

// ********************************* Section 4: GR55 program change ********************************************

FLASHMEM void MD_GR55_class::select_patch(uint16_t new_patch) {

  if (new_patch == patch_number) unmute();
  prev_patch_number = patch_number;
  patch_number = new_patch;

  // Method 1: using CC/PC
  uint16_t GR55_patch_send = patch_number;
  if (patch_number >= GR55_NUMBER_OF_USER_PATCHES) {
    GR55_patch_send += 1751; // There is a gap of 1752 patches in the numbering system of the GR-55. This will recreate it.
  }
  MIDI_send_CC(0, GR55_patch_send >> 7, MIDI_channel, MIDI_out_port);
  MIDI_send_PC(GR55_patch_send & 0x7F, MIDI_channel, MIDI_out_port);

  // New method thanks to Phil Jynx, which should improve the patch change gap of the GR-55. But the difference is ever so little...
  // Method 2: Using Phil's method
  //sendGR55BankPatch(GR55_patch_send)

  DEBUGMSG("Selecting patch " + String(new_patch));
  do_after_patch_selection();
}

FLASHMEM void MD_GR55_class::do_after_patch_selection() {
  request_onoff = false;
  is_on = connected;
  if (Setting.Send_global_tempo_after_patch_change == true) set_bpm();
  Current_patch_number = patch_number;
  load_patch(patch_number);
  request_full_assign(0);
  update_LEDS = true;
  update_main_lcd = true;
  request_guitar_switch_states();
  MD_base_class::do_after_patch_selection();
  if ((flash_bank_of_three != 255) && (Current_page != Previous_page)) { // When in direct select, go back to the current_device_page
    bank_size = Previous_bank_size;
    SCO_select_page(Previous_page);
  }
  flash_bank_of_three = 255;
}

FLASHMEM bool MD_GR55_class::request_patch_name(uint8_t sw, uint16_t number) {
  if (number > patch_max) return true;
  if (number < GR55_NUMBER_OF_USER_PATCHES) { // This is a user patch - we read these from memory
    uint32_t Address = 0x20000001 + ((number / 0x80) * 0x1000000) + ((number % 0x80) * 0x10000); //Calculate the address where the patchname is stored on the GR-55
    last_requested_sysex_address = Address;
    last_requested_sysex_type = REQUEST_PATCH_NAME;
    last_requested_sysex_switch = sw;
    request_sysex(Address, 16); //Request the 16 bytes of the GP10 patchname
  }
  else {
    if (number < (GR55_NUMBER_OF_USER_PATCHES + GR55_NUMBER_OF_FACTORY_PATCHES)) { // Fixed patch - read from GR55_preset_patch_names array
      read_preset_name(sw, SP[sw].PP_number);
    }
    PAGE_request_next_switch(); // Move on to next switch
  }
  return false;
}

FLASHMEM void MD_GR55_class::request_current_patch_name() {
  request_sysex(GR55_REQUEST_CURRENT_PATCH_NAME);
}

FLASHMEM bool MD_GR55_class::flash_LEDs_for_patch_bank_switch(uint8_t sw) { // Will flash the LEDs in banks of three when coming from direct select mode.
  if (!bank_selection_active()) return false;

  if (flash_bank_of_three == 255) return true; // We are not coming from direct select, so all LEDs should flash

  bool in_right_bank_of_nine = (flash_bank_of_three / 3 == SP[sw].PP_number / 9); // Going bank up and down coming from direct select will make all LEDs flash in other banks
  if (!in_right_bank_of_nine) return true;

  bool in_right_bank_of_three = (flash_bank_of_three == SP[sw].PP_number / 3); // Only flash the three LEDs of the corresponding bank
  if (in_right_bank_of_three) return true;
  return false;
}

FLASHMEM void MD_GR55_class::number_format(uint16_t number, String &Output) {
  // Uses patch_number as input and returns Current_patch_number_string as output in format "U01-1"
  // First character is L for Lead, R for Rhythm, O for Other or U for User
  // In guitar mode preset_banks is set to 40, in bass mode it is set to 12, because there a less preset banks in bass mode.

  uint16_t patch_number_corrected = 0; // Need a corrected version of the patch number to deal with the funny numbering system of the GR-55
  uint16_t bank_number_corrected = 0; //Also needed, because with higher banks, we start counting again

  bank_number_corrected = (number / 3); // Calculate the bank number from the patch number

  if (bank_number_corrected < 99) {
    Output += 'U';
    patch_number_corrected = number;  //In the User bank all is normal
  }

  else {
    if (bank_number_corrected >= (99 + (2 * preset_banks))) {   // In the Other bank we have to adjust the bank and patch numbers so we start with O01-1
      Output += "O";
      patch_number_corrected = number - (GR55_NUMBER_OF_USER_PATCHES + (6 * preset_banks));
      bank_number_corrected = bank_number_corrected - (99 + (2 * preset_banks));
    }

    else {
      if (bank_number_corrected >= (99 + preset_banks)) {   // In the Rhythm bank we have to adjust the bank and patch numbers so we start with R01-1
        Output += "R";
        patch_number_corrected = number - (GR55_NUMBER_OF_USER_PATCHES + (3 * preset_banks));
        bank_number_corrected = bank_number_corrected - (99 + preset_banks);
      }

      else    {// In the Lead bank we have to adjust the bank and patch numbers so we start with L01-1
        Output += "L";
        patch_number_corrected = number - GR55_NUMBER_OF_USER_PATCHES;
        bank_number_corrected = bank_number_corrected - 99;
      }
    }
  }

  // Then add the bank number
  uint8_t bank_no = (patch_number_corrected / 3) + 1;

  Output += String(bank_no / 10) + String(bank_no % 10);
  // Finally add the patch number
  Output += "-" + String((patch_number_corrected % 3) + 1);
}

FLASHMEM void MD_GR55_class::direct_select_format(uint16_t number, String &Output) {
  if (direct_select_state == 0) Output += 'U' + String(number) + "_-_";
  else Output += 'U' + String(bank_select_number) + String(number) + "-_";
}

FLASHMEM void MD_GR55_class::direct_select_start() {
  Previous_bank_size = bank_size; // Remember the bank size
  device_in_bank_selection = my_device_number + 1;
  bank_size = 300;
  bank_select_number = 0; // Reset bank number
  direct_select_state = 0;
}

FLASHMEM uint16_t MD_GR55_class::direct_select_patch_number_to_request(uint8_t number) {
  uint16_t new_patch_number;
  if (direct_select_state == 0) new_patch_number = (number * 30);
  else new_patch_number = (bank_select_number * 30) + (number * 3);
  if (new_patch_number < 3) new_patch_number = 3;
  return new_patch_number - 3;
}

FLASHMEM void MD_GR55_class::direct_select_press(uint8_t number) {
  if (!valid_direct_select_switch(number)) return;
  if (direct_select_state == 0) {
    // First digit pressed
    bank_select_number = number;
    bank_size = 30;
    direct_select_state = 1;
  }
  else  {
    // Second digit pressed
    if (number > 0) number--;
    uint16_t base_patch = (bank_select_number * 30) + (number * 3);
    flash_bank_of_three = base_patch / 3;
    bank_size = 9;
    bank_select_number = (base_patch / bank_size);
    Current_page = Previous_page; // SCO_select_page will overwrite Previous_page with Current_page, now it will know the way back
    SCO_select_page(my_device_page1); // Which should give PAGE_GR55_PATCH_BANK
    device_in_bank_selection = my_device_number + 1; // Go into bank mode
  }
}

// ** US-20 simulation
// Selecting and muting the GR55 is done by storing the settings of COSM guitar switch and Normal PU switch
// and switching both off when guitar is muted and back to original state when the GR55 is selected

FLASHMEM void MD_GR55_class::request_guitar_switch_states() {
  delay(10); // Extra delay, otherwise first parameter is not read after patch change
  request_sysex(GR55_PCM1_SW, 1);
  request_sysex(GR55_PCM2_SW, 1);
  request_sysex(GR55_COSM_DATA, 11);
  request_sysex(GR55_NORMAL_PU_SW, 1);
  request_onoff = true;
}

FLASHMEM void MD_GR55_class::check_inst_switch_states(const unsigned char* sxdata, short unsigned int sxlength) {
  if (request_onoff == true) {
    uint32_t address = (sxdata[7] << 24) + (sxdata[8] << 16) + (sxdata[9] << 8) + sxdata[10]; // Make the address 32 bit

    if (address == GR55_PCM1_SW) {
      gr55_pcm1_mute = sxdata[11];  // Store the value
    }

    if (address == GR55_PCM2_SW) {
      gr55_pcm2_mute = sxdata[11];  // Store the value
    }

    if (address == GR55_COSM_DATA) {
      COSM_gtr_mute = sxdata[21];  // Store the value
      COSM_gtr_type = sxdata[11];
      COSM_bass_type = sxdata[16];
    }

    if (address == GR55_NORMAL_PU_SW) {
      normal_pu_mute = sxdata[11];  // Store the value
      request_onoff = false;
    }
  }
}

FLASHMEM void MD_GR55_class::unmute() {
  is_on = connected;
  if (is_on) {
    write_sysex(GR55_PCM1_SW, gr55_pcm1_mute);
    write_sysex(GR55_PCM2_SW, gr55_pcm2_mute);
    write_sysex(GR55_COSM_GUITAR_SW, COSM_gtr_mute);
    write_sysex(GR55_NORMAL_PU_SW, normal_pu_mute);
  }
}

FLASHMEM void MD_GR55_class::mute() {
  if ((US20_mode_enabled()) && (!is_always_on) && (is_on)) {
    mute_now();
  }
}

FLASHMEM void MD_GR55_class::mute_now() { // Also called when engaging global tuner.
  is_on = false;
  write_sysex(GR55_PCM1_SW, 0x01); // Switch synth 1 off
  write_sysex(GR55_PCM2_SW, 0x01); // Switch synth 1 off
  write_sysex(GR55_COSM_GUITAR_SW, 0x01); // Switch COSM guitar off
  write_sysex(GR55_NORMAL_PU_SW, 0x01); // Switch normal pu off
}

// ********************************* Section 5: GR55 parameter control ********************************************

// Procedures for the GR55_PARAMETER and GR55_ASSIGN commands

// Procedures for the GR55_PARAMETER:
// 1. Load in SP array - in PAGE_load_current(true)
// 2. Request parameter state - in PAGE_request_current_switch()
// 3. Read parameter state - GR55_read_parameter() below
// 4. Press switch - GR55_parameter_press() below - also calls GR55_check_update_label()
// 5. Release switch - GR55_parameter_release() below - also calls GR55_check_update_label()

// There is some weird mapping going on in the GR55 for some parameters:
// *C127 convert 0-127 to display 0-100
// *C64 convert 0-127(center 64) to display -50 - 0 - +50
// *C63 convert 1-127(center 64) to display -50 - 0 - +50
// *C63Off convert 1-127(center 64) to display -50 - 0 - +50     0(-64) to -51(OFF)
// *C64pan convert 0-127(center 64) to display L50 - center - R50
// *C200 convert 44-84(center 64) to display -200 - 0 - +200


struct GR55_parameter_struct { // Combines all the data we need for controlling a parameter in a device
  uint8_t Ctl_function; // Control function number
  uint16_t Target; // Target of the assign as given in the assignments of the GR55 / GR55
  uint32_t Address; // The address of the parameter
  uint8_t NumVals; // The number of values for this parameter
  char Name[11]; // The name for the label
  uint16_t Sublist; // Which sublist to read for the FX or amp type - 0 if second byte does not contain the type or if there is no sublist +100 Show value from sublist.
  uint8_t Colour; // The colour for this effect.
  bool Reversed; // The GR-55 has SYNTH 1 SW, SYNTH 2 SW, COSM GUITAR SW and NORMAL PICKUP reversed. For these parameters the on and off values will be read and written in reverse
};


#define GR55_MFX_COLOUR 255 // Just a colour number to pick the colour from the GR55_MFX_colours table
#define GR55_MFX_TYPE_COLOUR 254 //Another number for the MFX type
#define GR55_MOD_COLOUR 253 // Just a colour number to pick the colour from the GR55_MOD_colours table
#define GR55_MOD_TYPE_COLOUR 252 //Another number for the MOD type
#define SUBLIST_FROM_BYTE2 32768 //Bit 16 set high means read sublist frm byte 2
#define SHOW_NUMBER 31767 // Special number for showing the number (used for gain/volume/etc) - set in sublist
#define SHOW_DOUBLE_NUMBER 31766 // Special double byte number for showing the number (used for patch volume/etc) - set in sublist
#define SHOW_C64PAN 31765 // Special number for showing the number (used for gain/volume/etc) - set in sublist
#define SHOW_C127 31764 // Weird remapping of 0-127 to 0-100.

// All parameters that cannot be set from an assign have the target address at 0xFFF

const PROGMEM GR55_parameter_struct GR55_parameters[] = {
  {6, 0x12B, 0x18000304,   2, "MFX", 1 | SUBLIST_FROM_BYTE2, GR55_MFX_COLOUR, false}, // 0
  {0, 0x12C, 0x18000305,  20, "MFX TYPE", 1, GR55_MFX_TYPE_COLOUR, false},
  {5, 0x0E6, 0x18000715,   2, "MOD", 63 | SUBLIST_FROM_BYTE2, GR55_MOD_COLOUR, false},
  {0, 0x0E7, 0x18000716,  14, "MOD TYPE", 63, GR55_MOD_TYPE_COLOUR, false},
  {10, 0x000, 0x18002003,   2, "SYNTH1 SW", 0, FX_GTR_TYPE, true},
  {0, 0x003, 0x18002005,  68, "PCM1 OCT", 92, FX_GTR_TYPE, false}, // Not a perfect solution, as there is no minimal value
  {11, 0x03B, 0x18002103,   2, "SYNTH2 SW", 0, FX_GTR_TYPE, true},
  {0, 0x03E, 0x18002105,  68, "PCM2 OCT", 92, FX_GTR_TYPE, false}, // Not a perfect solution, as there is no minimal value
  {12, 0x076, 0x1800100A,   2, "COSM GT SW", 0, FX_GTR_TYPE, true},
  {13, 0xFFF, 0x18000232,   2, "NRML PU SW", 0, FX_GTR_TYPE, true},
  {0, 0x081, 0x1800101D,   2, "12STR SW", 0, FX_PITCH_TYPE, false},
  {4, 0x0D6, 0x18000700,   2, "AMP", 21 | SUBLIST_FROM_BYTE2, FX_AMP_TYPE, false}, // 10
  {0, 0xFFF, 0x18000701,  42, "AMP TP", 21, FX_AMP_TYPE, false},
  {0, 0x0D7, 0x18000702, 121, "AMP GAIN", SHOW_NUMBER, FX_AMP_TYPE, false},
  {0, 0x0D8, 0x18000703, 101, "AMP LVL", SHOW_NUMBER, FX_AMP_TYPE, false},
  {0, 0x0D9, 0x18000704,   3, "AMP GAIN", 141, FX_AMP_TYPE, false},
  {0, 0x0DA, 0x18000705,   2, "AMP SOLO", 0, FX_AMP_TYPE, false},
  {0, 0x0DC, 0x18000707, 101, "AMP BASS", SHOW_NUMBER, FX_AMP_TYPE, false},
  {0, 0x0DD, 0x18000708, 101, "AMP MID", SHOW_NUMBER, FX_AMP_TYPE, false},
  {0, 0x0DE, 0x18000709, 101, "AMP TREBLE", SHOW_NUMBER, FX_AMP_TYPE, false},
  {0, 0x0DF, 0x1800070A, 101, "AMP PRESC", SHOW_NUMBER, FX_AMP_TYPE, false},
  {0, 0x0E0, 0x1800070B,   2, "AMP BRIGHT", 0, FX_AMP_TYPE, false}, // 20
  {0, 0x0E1, 0x1800070C,   9, "SPKR TYPE", 144, FX_AMP_TYPE, false},
  {0, 0x128, 0x1800075A,   2, "NS SWITCH", 0, FX_DYNAMICS_TYPE, false},
  {7, 0x1EC, 0x18000605,   2, "DLY SW", 129 | SUBLIST_FROM_BYTE2, FX_DELAY_TYPE, false},
  {0, 0x1ED, 0x18000606,   7, "DLY TYPE", 129, FX_DELAY_TYPE, false},
  {0, 0x1F4, 0x1800060C,   2, "RVRB SW", 136 | SUBLIST_FROM_BYTE2, FX_REVERB_TYPE, false},
  {0, 0x1F5, 0x1800060D,   5, "RVRB TYPE", 136, FX_REVERB_TYPE, false},
  {8, 0x1FC, 0x18000600,   2, "CHOR SW", 125 | SUBLIST_FROM_BYTE2, FX_MODULATE_TYPE,  false},
  {0, 0x1FD, 0x18000601,   4, "CHOR TYPE", 125, FX_MODULATE_TYPE,  false},
  {0, 0x204, 0x18000611,   2, "EQ SWITCH", 0, FX_FILTER_TYPE, false},
  {0, 0x213, 0x18000234,   2, "TUN SW", 160 | SUBLIST_FROM_BYTE2, FX_PITCH_TYPE, false}, // 30
  {0, 0x214, 0x18000235,  13, "TUNING", 160, FX_PITCH_TYPE, false},
  {0, 0x216, 0x18000230, 201, "PATCH LVL", SHOW_DOUBLE_NUMBER, FX_FILTER_TYPE, false},
  {100, 0xFFF, 0x18000011,   2, "CTL", 94 | SUBLIST_FROM_BYTE2, FX_DEFAULT_TYPE, false},
  {0, 0xFFF, 0x18000012,  16, "CTL SW", 94, FX_DEFAULT_TYPE, false},
  {0, 0xFFF, 0x1800001F,  10, "EXP", 84, FX_DEFAULT_TYPE, false},
  {0, 0xFFF, 0x18000036,  10, "EXP ON", 84, FX_DEFAULT_TYPE, false},
  {101, 0xFFF, 0x1800004D,   2, "EXP SW", 111 | SUBLIST_FROM_BYTE2, FX_DEFAULT_TYPE, false},
  {0, 0xFFF, 0x1800004E,  14, "EXP SW", 111, FX_DEFAULT_TYPE, false},
  {0, 0xFFF, 0x1800005B,  10, "GK VOL", 84, FX_DEFAULT_TYPE, false},
  {0, 0xFFF, 0x18000072,  14, "GK S1", 111, FX_DEFAULT_TYPE, false}, // 40
  {0, 0xFFF, 0x1800007F,  14, "GK S2", 111, FX_DEFAULT_TYPE, false},
  {0, 0xFFF, 0x02000007,   2, "GTR2MIDI", 0, FX_DEFAULT_TYPE, false},
  {0, 0xFFF, 0x02000008,   2, "G2M MODE", 77, FX_DEFAULT_TYPE, false},
  {0, 0xFFF, 0x02000009,   2, "G2M CHOM", 0, FX_DEFAULT_TYPE, false},
  {0, 0xFFF, 0x02000016,   5, "GTR OUT", 79, FX_DEFAULT_TYPE, false},
  {0, 238, 0x1800071E,  14, "WAH PEDAL", 0, GR55_MOD_TYPE_COLOUR, false},
};

#define GR55_EXP_SW 38
#define GR55_EXP 36
#define GR55_EXP_ON 37

const uint16_t GR55_NUMBER_OF_PARAMETERS = sizeof(GR55_parameters) / sizeof(GR55_parameters[0]);

//#define GR55_SIZE_OF_SUBLISTS 76
const PROGMEM char GR55_sublists[][9] = {
  // Sublist 1 - 20: MFX FX types
  "EQ", "S FILTR", "PHASER", "STEP PHR", "RING MOD", "TREMOLO", "AUTO PAN", "SLICER", "K ROTARY", "HEXA-CHS",
  "SPACE-D", "FLANGER", "STEP FLR", "AMP SIM", "COMPRESS", "LIMITER", "3TAP DLY", "TIME DLY", "LOFI CPS", "PITCH SH",

  // Sublist 21 - 62: Amp types
  "BOSS CLN", "JC-120", "JAZZ CBO", "FULL RNG", "CLN TWIN", "PRO CRCH", "TWEED", "DELUX CR", "BOSS CRH", "BLUES",
  "WILD CRH", "STACK CR", "VO DRIVE", "VO LEAD", "VO CLEAN", "MATCH DR", "FAT MTCH", "MATCH LD", "BG LEAD", "BG DRIVE",
  "BG RHYTH", "MS'59 I", "MS'59 II", "MS HIGN", "MS SCOOP", "R-FIER V", "R-FIER M", "R-FIER C", "T-AMP LD", "T-AMP CR",
  "T-AMP CL", "BOSS DRV", "SLDN", "LEAD STK", "HEAVY LD", "BOSS MTL", "5150 DRV", "METAL LD", "EDGE LD", "BASS CLN",
  "BASS CRH", "BASS HIG",

  // Sublist 63 - 76: MOD FX types
  "OD/DS", "WAH", "COMP", "LIMITER", "OCTAVE", "PHASER", "FLANGER", "TREMOLO", "ROTARY", "UNI-V",
  "PAN", "DELAY", "CHORUS", "EQ",

  // Sublist 77-78: G2M Mode
  "MONO", "POLY",

  // Sublist 79-83: Guitar out source select
  "PATCH", "OFF", "NRML PU", "MODEL", "BOTH",

  // Sublist 84-93: Assign function
  "OFF", "PTCH VOL", "TONE VOL", "PITCH BD", "MOD", "X FADER", "DLY LVL", "RVRB LVL", "CHOR LVL", "MOD CTL",

  // Sublist 94-110: Button assign function
  "OFF", "HOLD", "TAPTEMPO", "TONE SW", "AMP SW", "MOD SW", "MFX SW", "DLY SW", "RVRB SW", "CHOR SW", "AP PLAY", "AP SONG+", "AP SONG-", "AP SW", "VLINK SW", "LED MOMT", "LED TOGL",

  // Sublist 111-124: Button assign function
  "OFF", "TAPTEMPO", "TONE SW", "AMP SW", "MOD SW", "MFX SW", "DLY SW", "RVRB SW", "CHOR SW", "AP PLAY", "AP SONG+", "AP SONG-", "AP SW", "VLINK SW",

  // Sublist 125-128: Chorus Type
  "MONO", "STEREO", "MON MILD", "ST MILD",

  // Sublist 129 - 135: Delay type
  "SINGLE", "PAN", "REVERSE", "ANALOG", "TAPE", "MODULATE", "HICUT",

  // Sublist 136 - 140: Reverb type
  "AMBIENCE", "ROOM", "HALL1", "HALL2", "PLATE",

  // Sublist 141 - 143: Amp gain switch
  "LOW", "MID", "HIGH",

  //Sublist 144 - 152: Amp speaker type
  "OFF", "ORIGIN", "1x8\"", "1x10\"", "1x12\"", "2x12\"", "4x10\"", "4x12\"", "8x12\"",

  // Sublist 153 - 159: Part Octave shift - this sublist has to be called from 92 because the minimum value of the data is 61
  "-3", "-2", "-1", "0", "+1", "+2", "+3",

  // Sublist 160 - 172: Tunings
  "OPEN-D", "OPEN-E", "OPEN-G", "OPEN-A", "DROP-D", "D-MODAL", "-1 STEP", "-2 STEP", "BARITONE", "NASHVL", "-1 OCT", "+1 OCT", "USER",
};

const uint16_t GR55_SIZE_OF_SUBLIST = sizeof(GR55_sublists) / sizeof(GR55_sublists[0]);

const PROGMEM uint8_t GR55_MFX_colours[20] = {
  FX_FILTER_TYPE, // Colour for"EQ",
  FX_FILTER_TYPE, // Colour for"S FILTR",
  FX_MODULATE_TYPE, // Colour for"PHASER",
  FX_MODULATE_TYPE, // Colour for"STEP PHR",
  FX_MODULATE_TYPE, // Colour for"RING MOD",
  FX_MODULATE_TYPE, // Colour for"TREMOLO",
  FX_MODULATE_TYPE, // Colour for"AUTO PAN",
  FX_MODULATE_TYPE, // Colour for"SLICER",
  FX_MODULATE_TYPE, // Colour for"K ROTARY",
  FX_MODULATE_TYPE, // Colour for"HEXA-CHS",
  FX_MODULATE_TYPE, // Colour for"SPACE-D",
  FX_MODULATE_TYPE, // Colour for"FLANGER",
  FX_MODULATE_TYPE, // Colour for"STEP FLR",
  FX_AMP_TYPE, // Colour for"AMP SIM",
  FX_DYNAMICS_TYPE, // Colour for"COMPRESS",
  FX_DYNAMICS_TYPE, // Colour for"LIMITER",
  FX_DELAY_TYPE, // Colour for"3TAP DLY",
  FX_DELAY_TYPE, // Colour for"TIME DLY",
  FX_FILTER_TYPE, // Colour for"LOFI CPS",
  FX_PITCH_TYPE // Colour for"PITCH SH"
};

const PROGMEM uint8_t GR55_MOD_colours[14] = {
  FX_DIST_TYPE, // Colour for"OD/DS",
  FX_WAH_TYPE, // Colour for"WAH",
  FX_DYNAMICS_TYPE, // Colour for"COMP",
  FX_DYNAMICS_TYPE, // Colour for"LIMITER",
  FX_PITCH_TYPE, // Colour for"OCTAVE",
  FX_MODULATE_TYPE, // Colour for"PHASER",
  FX_MODULATE_TYPE, // Colour for"FLANGER",
  FX_MODULATE_TYPE, // Colour for"TREMOLO",
  FX_MODULATE_TYPE, // Colour for"ROTARY",
  FX_MODULATE_TYPE, // Colour for"UNI-V",
  FX_MODULATE_TYPE, // Colour for"PAN",
  FX_DELAY_TYPE, // Colour for"DELAY",
  FX_MODULATE_TYPE, // Colour for"CHORUS",
  FX_FILTER_TYPE // Colour for"EQ"
};

FLASHMEM void MD_GR55_class::read_parameter_name(uint16_t number, String &Output) { // Called from menu
  if (number < number_of_parameters())  Output = GR55_parameters[number].Name;
  else Output = "?";
}

FLASHMEM void MD_GR55_class::read_parameter_value_name(uint16_t number, uint16_t value, String &Output) {
  if (number < number_of_parameters())  {
    uint16_t my_sublist = GR55_parameters[number].Sublist;
    if ((my_sublist > 0) && !(my_sublist & SUBLIST_FROM_BYTE2)) { // Check if state needs to be read
      switch (my_sublist) {
        case SHOW_NUMBER:
        case SHOW_DOUBLE_NUMBER:
          //if (GR55_parameters[number].Address == 0x2000) Output += String(value * 2); //Patch level is displayed double
          Output += String(value);
          break;
        case SHOW_C64PAN:
          if (value < 64) Output += "L" + String(50 - value);
          if (value == 64) Output += "C";
          if (value > 64) Output += "R" + String(value - 50);
          break;
        default:
          String type_name = GR55_sublists[my_sublist + value - 1];
          Output += type_name;
          break;
      }
    }
    else if (value == 1) Output += "ON";
    else Output += "OFF";
  }
  else Output += "?";
}

// Toggle GR55 stompbox parameter
FLASHMEM void MD_GR55_class::parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {

  uint8_t value = SCO_return_parameter_value(Sw, cmd);

  if (GR55_parameters[number].Sublist == 92) { // Create minimum value for tone octave
    if (value < 61) value = 61;
  }

  if (GR55_parameters[number].Reversed) value ^= 1; // Reverse the value

  if ((SP[Sw].Latch != TGL_OFF) && (number < GR55_NUMBER_OF_PARAMETERS)) {
    if (GR55_parameters[number].Sublist == SHOW_DOUBLE_NUMBER) write_sysex(GR55_parameters[number].Address, value >> 4, value & 0x0F);
    else write_sysex(GR55_parameters[number].Address, value);
    SP[Sw].Offline_value = value;
    // Show message
    check_update_label(Sw, value);
    String msg = "";
    if (SP[Sw].Type != ASSIGN) {
      msg = GR55_parameters[number].Name;
      msg += ':';
    }
    msg += SP[Sw].Label;
    LCD_show_popup_label(msg, ACTION_TIMER_LENGTH);
    if (SP[Sw].Latch != UPDOWN) update_page = REFRESH_FX_ONLY; // To update the other switch states, we re-load the current page
  }
}

FLASHMEM void MD_GR55_class::parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {
  // Work out state of pedal
  if (SP[Sw].Latch == MOMENTARY) {
    SP[Sw].State = 2; // Switch state off
    if (number < GR55_NUMBER_OF_PARAMETERS) {
      write_sysex(GR55_parameters[number].Address, cmd->Value2);
      SP[Sw].Offline_value = cmd->Value2;
    }
    update_page = REFRESH_FX_ONLY; // To update the other switch states, we re-load the current page
  }
}

FLASHMEM void MD_GR55_class::read_parameter_title(uint16_t number, String &Output) {
  Output += GR55_parameters[number].Name;
}

FLASHMEM bool MD_GR55_class::request_parameter(uint8_t sw, uint16_t number) {
  if (can_request_sysex_data()) {
    uint32_t my_address = GR55_parameters[number].Address;
    last_requested_sysex_address = my_address;
    last_requested_sysex_type = REQUEST_PARAMETER_TYPE;
    last_requested_sysex_switch = sw;
    if (my_address == 0x18000011) request_sysex(my_address, 4); // Request four parameters to include hold type
    else request_sysex(my_address, 2);
    return false; // Move to next switch is false. We need to read the parameter first
  }
  else {
    if ((sw < TOTAL_NUMBER_OF_SWITCHES) && (SP[sw].Type == PAR_BANK)) read_parameter(sw, SP[sw].Offline_value, SP[sw + 1].Offline_value);
    else read_parameter(sw, SP[sw].Offline_value, 0);
    return true;
  }
}

FLASHMEM void MD_GR55_class::read_parameter(uint8_t sw, uint8_t byte1, uint8_t byte2) { //Read the current GR55 parameter
  SP[sw].Target_byte1 = byte1;
  SP[sw].Target_byte2 = byte2;

  uint16_t index = SP[sw].PP_number; // Read the parameter number (index to GR55-parameter array)

  if (GR55_parameters[index].Sublist == SHOW_DOUBLE_NUMBER) { // Don't you just love all these weird exceptions!
    byte1 = (byte1 * 16) + byte2; // Change byte1 so the parameter updates correctly on the display
    SP[sw].Target_byte1 = byte1;
  }

  // Set the status
  SP[sw].State = SCO_find_parameter_state(sw, byte1);

  // Reverse state 1 and 2 if neccesary
  if (GR55_parameters[index].Reversed) {
    if (SP[sw].State == 1) SP[sw].State = 2;
    else if (SP[sw].State == 2) SP[sw].State = 1;
  }

  // Set the colour
  uint8_t my_colour = GR55_parameters[index].Colour;

  //Check for special colours:
  switch (my_colour) {
    case GR55_MFX_COLOUR:
      SP[sw].Colour = GR55_MFX_colours[byte2]; //MFX type read in byte2
      break;
    case GR55_MFX_TYPE_COLOUR:
      SP[sw].Colour = GR55_MFX_colours[byte1]; //MFX type read in byte1
      break;
    case GR55_MOD_COLOUR:
      SP[sw].Colour = GR55_MOD_colours[byte2]; //MOD type read in byte2
      break;
    case GR55_MOD_TYPE_COLOUR:
      SP[sw].Colour = GR55_MOD_colours[byte1]; //MOD type read in byte1
      break;
    default:
      SP[sw].Colour =  my_colour;
      break;
  }

  // Check for exception on CTL pedal - LED momentary - not that it does anything, because assigns to CTL pedals are ignored when controlled through sysex!!!
  /*if (GR55_parameters[index].Address == 0x18000011) && (byte2 == 0x0F)) {
    SP[sw].Latch = MOMENTARY;
    }*/

  // Set the display message
  String msg = "";
  if (SP[sw].Type == ASSIGN) msg = GR55_parameters[index].Name;
  if (GR55_parameters[index].Sublist > SUBLIST_FROM_BYTE2) { // Check if a sublist exists
    String type_name = GR55_sublists[GR55_parameters[index].Sublist - SUBLIST_FROM_BYTE2 + byte2 - 1];
    msg += '(' + type_name + ')';
  }
  if ((GR55_parameters[index].Sublist > 0) && !(GR55_parameters[index].Sublist & SUBLIST_FROM_BYTE2)) { // Check if state needs to be read
    if (SP[sw].Type == ASSIGN) msg += ':';
    read_parameter_value_name(index, byte1, msg);
    //  String type_name = GR55_sublists[GR55_parameters[index].Sublist + byte1 - 1];
    //  msg += ':' + type_name;
  }
  //Copy it to the display name:
  LCD_set_SP_label(sw, msg);
}

FLASHMEM void MD_GR55_class::check_update_label(uint8_t Sw, uint8_t value) { // Updates the label for extended sublists
  uint16_t index = SP[Sw].PP_number; // Read the parameter number (index to GR55-parameter array)
  if (index != NOT_FOUND) {
    if ((GR55_parameters[index].Sublist > 0) && !(GR55_parameters[index].Sublist & SUBLIST_FROM_BYTE2)) { // Check if state needs to be read
      LCD_clear_SP_label(Sw);
      // Set the display message
      String msg = "";
      //String type_name = GR55_sublists[GR55_parameters[index].Sublist + value - 1];
      //msg += ':' + type_name;
      //msg += ':';
      read_parameter_value_name(index, value, msg);

      //Copy it to the display name:
      LCD_set_SP_label(Sw, msg);

      //Update the current switch label
      update_lcd = Sw;
    }
  }
}

FLASHMEM uint8_t MD_GR55_class::find_target(uint16_t target) {
  for (uint8_t i = 0; i < GR55_NUMBER_OF_PARAMETERS; i++) {
    if (GR55_parameters[i].Target == target) return i;
  }
  return NOT_FOUND;
}

FLASHMEM uint16_t MD_GR55_class::number_of_parameters() {
  return GR55_NUMBER_OF_PARAMETERS;
}

FLASHMEM uint8_t MD_GR55_class::number_of_values(uint16_t parameter) {
  if (parameter < GR55_NUMBER_OF_PARAMETERS) return GR55_parameters[parameter].NumVals;
  else return 0;
}

// ********************************* Section 6: GR55 assign control ********************************************

// GR55 has eight general assigns, which we can be controlled with a cc-MIDI message.

// Procedures for GR55_ASSIGN:
// 1. Load in SP array - GR55_assign_load() below
// 2. Request - GR55_request_current_assign()
// 3. Read assign - GR55_read_current_assign(uint8_t sw) - also requests parameter state
// 4. Read parameter state - GR55_read_parameter() above
// 5. Press switch - GR55_assign_press() below
// 6. Release switch - GR55_assign_release() below

const PROGMEM char GR55_ctl_pedal_title[GR55_NUMBER_OF_CONTROL_PEDALS][7] = { "CTL", "EXP SW", "GK S1", "GK S2"};
const PROGMEM char GR55_ctl_pedal_short_title[GR55_NUMBER_OF_CONTROL_PEDALS][7] = { "CTL", "EXPS", "GKS1", "GKS2"};
const PROGMEM uint32_t GR55_ctl_pedal_address[GR55_NUMBER_OF_CONTROL_PEDALS] = { 0x18000012, 0x1800004E, 0x18000072, 0x1800007F};
const PROGMEM uint8_t GR55_ctl_pedal_assign_target[GR55_NUMBER_OF_CONTROL_PEDALS] = { 0, 3, 6, 7 };
const PROGMEM uint8_t GR55_int_pdl_assign_target[GR55_NUMBER_OF_CONTROL_PEDALS] = { 1, 8, 9, 10 };
const PROGMEM char GR55_ctl_pedal_functions_title[][16] = {
  "--", "HOLD", "TAP TEMPO", "TONE SW", "AMP SW", "MOD SW", "MFX SW", "DELAY SW", "REVERB SW", "CHORUS SW", "PLAY/STOP",
  "SONG INC", "SONG DEC", "AUDIO PLAYER SW", "V_LINK SW", "LED MOMENT", "LED TOGGLE"
};
const PROGMEM uint32_t GR55_ctl_pedal_function_address[] = { 0, 0, 0, 0x18002003, };

FLASHMEM void MD_GR55_class::read_assign_name(uint8_t number, String & Output) {
  if (number < GR55_NUMBER_OF_CONTROL_PEDALS) Output += GR55_ctl_pedal_title[number];
  else {
    number -= GR55_NUMBER_OF_CONTROL_PEDALS;
    if (number < GR55_NUMBER_OF_SCENE_ASSIGNS) Output += "SCENE ASGN " + String(number + 1);
    else Output += "--";
  }
}

FLASHMEM void MD_GR55_class::read_assign_short_name(uint8_t number, String & Output) {
  if (number < GR55_NUMBER_OF_CONTROL_PEDALS) Output += GR55_ctl_pedal_short_title[number];
  else {
    number -= GR55_NUMBER_OF_CONTROL_PEDALS;
    if (number < GR55_NUMBER_OF_SCENE_ASSIGNS)  Output += "ASG" + String(number + 1);
    else Output += "--";
  }
}

FLASHMEM void MD_GR55_class::read_assign_trigger(uint8_t number, String & Output) {
  if (number < GR55_NUMBER_OF_CONTROL_PEDALS) Output += GR55_ctl_pedal_title[number];
  else if (number < 128) Output += "CC#" + String(number);
  else Output += "-";
}

FLASHMEM uint8_t MD_GR55_class::get_number_of_assigns() {
  return GR55_NUMBER_OF_CONTROL_PEDALS + GR55_NUMBER_OF_SCENE_ASSIGNS;
}

FLASHMEM uint8_t MD_GR55_class::trigger_follow_assign(uint8_t number) {
  if (number < GR55_NUMBER_OF_CONTROL_PEDALS) return number;
  number -= GR55_NUMBER_OF_CONTROL_PEDALS;
  if (number < GR55_NUMBER_OF_SCENE_ASSIGNS) return number + GR55_FIRST_SCENE_ASSIGN_SOURCE_CC;
  else return 0;
}

FLASHMEM void MD_GR55_class::assign_press(uint8_t Sw, uint8_t value) { // Switch set to GR55_ASSIGN is pressed
  uint8_t asgn = SP[Sw].Assign_number;

  if (SP[Sw].Assign_on) {
    uint8_t value;
    if (SP[Sw].State == 1) value = 1;
    else value = 0;

    if (asgn == 0) write_sysex(GR55_CTL_ADDRESS, value);
    else if (asgn == 1) write_sysex(GR55_EXP_SW_ADDRESS, value);
    else if (asgn < GR55_NUMBER_OF_CONTROL_PEDALS) {
      if (SP[Sw].Value4 == 3) { // Tone sw
        DEBUGMSG("FULL TONE STATE: " + String(SP[Sw].Step));
        if (SP[Sw].State == 1) set_tone_state(SP[Sw].Step & 0x0F);
        if (SP[Sw].State == 2) set_tone_state(SP[Sw].Step >> 4);
      }
      else {
        write_sysex(SP[Sw].Address, value);
      }
    }

    if (asgn < GR55_NUMBER_OF_CONTROL_PEDALS) {
      bool int_pedal_source_found = change_active_assign_sources(GR55_ctl_pedal_assign_target[asgn], GR55_SOURCE_CC95, GR55_int_pdl_assign_target[asgn], GR55_SOURCE_CC94);
      delay(20);
      MIDI_send_CC(95, 127, MIDI_channel, MIDI_out_port);
      if (int_pedal_source_found) {
        int_pdl_cc_val = 0;
        MIDI_send_CC(94, int_pdl_cc_val, MIDI_channel, MIDI_out_port);
        int_pdl_timer = millis();
      }
      //assign_return_target_timer = millis() + SY1000_ASSIGN_RETURN_TIME;
    }
    else {
      toggle_scene_assign(asgn - GR55_NUMBER_OF_CONTROL_PEDALS);
    }
    check_update_label(Sw, value);
    LCD_show_popup_label(SP[Sw].Label, ACTION_TIMER_LENGTH);
  }

  if (SP[Sw].Assign_on) update_page = REFRESH_PAGE; // To update the other switch states, we re-load the current page
}

FLASHMEM void MD_GR55_class::assign_release(uint8_t Sw) { // Switch set to GR55_ASSIGN is released
  uint8_t asgn = SP[Sw].Assign_number;
  if ((asgn == 0) && (SP[Sw].Latch == MOMENTARY)) {
    write_sysex(GR55_CTL_ADDRESS, 0);
    SP[Sw].State = 2;
  }
  if (asgn < GR55_NUMBER_OF_CONTROL_PEDALS) {
    MIDI_send_CC(95, 0, MIDI_channel, MIDI_out_port);
    MIDI_send_CC(94, 127, MIDI_channel, MIDI_out_port);
    int_pdl_cc_val = 127;
    change_active_assign_sources(GR55_ctl_pedal_assign_target[asgn], GR55_ctl_pedal_assign_target[asgn], GR55_int_pdl_assign_target[asgn], GR55_SOURCE_INT_PEDAL);
  }
}

FLASHMEM void MD_GR55_class::assign_load(uint8_t sw, uint8_t assign_number, uint8_t cc_number) { // Switch set to GR55_ASSIGN is loaded in SP array
  SP[sw].Trigger = cc_number; //Save the cc_number in the Trigger variable
  SP[sw].Assign_number = assign_number;
}

FLASHMEM void MD_GR55_class::request_current_assign(uint8_t sw) {
  uint8_t asgn = SP[sw].Assign_number;
  if (asgn < GR55_NUMBER_OF_CONTROL_PEDALS) {
    uint32_t my_address = GR55_ctl_pedal_address[asgn];
    request_sysex(my_address , 13);
    last_requested_sysex_address = my_address;
    last_requested_sysex_type = REQUEST_ASSIGN_TYPE;
    last_requested_sysex_switch = sw;
  }
  else {
    asgn -= GR55_NUMBER_OF_CONTROL_PEDALS;
    if (asgn < GR55_NUMBER_OF_SCENE_ASSIGNS) {
      SP[sw].Assign_on = true;
      SP[sw].Latch = TOGGLE;
      SP[sw].Assign_max = 1;
      SP[sw].Assign_min = 0;
      SP[sw].PP_number = NOT_FOUND;
      SP[sw].Colour = 2; // Red
      if (scene_assign_state[asgn]) SP[sw].State = 1;
      else SP[sw].State = 0;
      String msg = "CC #" + String(asgn + GR55_FIRST_SCENE_ASSIGN_SOURCE_CC);
      uint8_t asgn_active = check_for_scene_assign_source(asgn + GR55_FIRST_SCENE_ASSIGN_SOURCE_CC);
      if (asgn_active > 0) {
        msg += " (ASGN " + String(asgn_active) + ')';
        if (SP[sw].State == 0) SP[sw].State = 2;
      }
      LCD_set_SP_label(sw, msg);
      PAGE_request_next_switch();
    }
  }
}

FLASHMEM void MD_GR55_class::read_current_assign(uint8_t sw, uint32_t address, const unsigned char* sxdata, short unsigned int sxlength) {
  bool found;
  String msg;
  uint8_t ctl_function_number;
  uint8_t asgn = SP[sw].Assign_number;

  if (asgn < GR55_NUMBER_OF_CONTROL_PEDALS) { // CTL assign
    ctl_function_number = sxdata[11];
    if ((asgn > 0) && (ctl_function_number > 0)) ctl_function_number++; // Skip hold function for all but first ctl pedal
    bool ctl_function_on = ((ctl_function_number > 0) || (asgn == 1));

    if (ctl_function_on) {
      SP[sw].Assign_on = true;
      SP[sw].Latch = TOGGLE;
      if (((ctl_function_number == 1) && (sxdata[14] == 1)) || (ctl_function_number == 2) || (ctl_function_number == 10) || (ctl_function_number == 11)
          || (ctl_function_number == 12) || (ctl_function_number == 15)) SP[sw].Latch = MOMENTARY;
      SP[sw].Assign_max = 1;
      SP[sw].Assign_min = 0;

      SP[sw].Value4 = ctl_function_number;

      if ((ctl_function_number <= 2) || (ctl_function_number >= 9)) {
        if (asgn == 0) ctl_function_number = 100; // Special case for CTL
        if (asgn == 1) ctl_function_number = 101; // Special case for EXP SW
      }

      // Save tone on/off states bitwise in Target_byte2 for GKS1 en GKS2
      if (ctl_function_number == 3) {
        uint8_t on_off_state = 0;
        for (uint8_t b = 16; b < 24; b++) {
          on_off_state <<= 1;
          on_off_state |= (sxdata[b] & 1);
        }
        SP[sw].Step = on_off_state;
        DEBUGMSG("FULL_ONOFF_STATE:" + String(on_off_state));

        // find the tone switch that is different
        if (sxdata[16] != sxdata[20]) {
          ctl_function_number = 10;
          SP[sw].Assign_max = sxdata[20];
          SP[sw].Assign_min = sxdata[16];
        }
        else if (sxdata[17] != sxdata[21]) {
          ctl_function_number = 11;
          SP[sw].Assign_max = sxdata[21];
          SP[sw].Assign_min = sxdata[17];
        }
        else if (sxdata[18] != sxdata[22]) {
          ctl_function_number = 12;
          SP[sw].Assign_max = sxdata[22];
          SP[sw].Assign_min = sxdata[18];
        }
        else if (sxdata[19] != sxdata[23]) {
          ctl_function_number = 13;
          SP[sw].Assign_max = sxdata[23];
          SP[sw].Assign_min = sxdata[17];
        }
      }

      LCD_set_SP_label(sw, GR55_ctl_pedal_functions_title[ctl_function_number]);

      // Request the target state
      found = ctl_target_lookup(sw, ctl_function_number); // Lookup the address of the target in the SY1000_Parameters array
      DEBUGMSG("Request target of CTL" + String(asgn + 1) + ':' + String(SP[sw].Address, HEX));
      if (found) {
        last_requested_sysex_address = SP[sw].Address;
        last_requested_sysex_type = REQUEST_PARAMETER_TYPE;
        request_sysex(last_requested_sysex_address, 2);
      }
      else { // parameter not in list, so it cannot be controlled
        SP[sw].Assign_on = false;
        SP[sw].PP_number = NOT_FOUND;
        SP[sw].Colour = FX_DEFAULT_TYPE;
        SP[sw].State = 0; // Switch the stompbox off
        SP[sw].Latch = MOMENTARY; // Make it momentary
        SP[sw].Colour = FX_DEFAULT_TYPE; // Set the on colour to default

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

FLASHMEM bool MD_GR55_class::ctl_target_lookup(uint8_t sw, uint8_t target) {  // Finds the target and its address in the GR55_parameters table

  // Lookup in GR55_parameter array
  bool found = false;
  for (uint16_t i = 0; i < GR55_NUMBER_OF_PARAMETERS; i++) {
    if (target == GR55_parameters[i].Ctl_function) { //Check is we've found the right target
      SP[sw].PP_number = i; // Save the index number
      SP[sw].Address = GR55_parameters[i].Address;
      found = true;
    }
  }
  return found;
}

FLASHMEM void MD_GR55_class::set_tone_state(uint8_t state) {
  DEBUGMSG("TONE STATE: " + String(state));
  gr55_pcm1_mute = ((state & 0x08) == 0);
  gr55_pcm2_mute = ((state & 0x04) == 0);
  COSM_gtr_mute = ((state & 0x02) == 0);
  normal_pu_mute = ((state & 0x01) == 0);
  unmute();
}


// Read full assign area
FLASHMEM void MD_GR55_class::request_full_assign(uint8_t number) {
  if (number >= GR55_NUMBER_OF_ASSIGNS) return;
  DEBUGMSG("Requesting assign " + String(number));
  uint32_t my_address = calculate_full_assign_address(number);
  read_full_assign_number = number;
  request_sysex(my_address, GR55_NUMBER_OF_ASSIGN_BYTES_READ);
}

FLASHMEM void MD_GR55_class::read_full_assign(uint8_t number, uint32_t address, const unsigned char* sxdata, short unsigned int sxlength) {
  if (number >= GR55_NUMBER_OF_ASSIGNS) return;
  DEBUGMSG("Reading assign " + String(number));
  for (uint8_t i = 0; i < sizeof(assign_area[0]); i++) {
    assign_area[number][i] = sxdata[i + 11];
  }
  number++;
  if (number < GR55_NUMBER_OF_ASSIGNS) request_full_assign(number);
  else update_page = REFRESH_PAGE;
}

FLASHMEM uint32_t MD_GR55_class::calculate_full_assign_address(uint8_t number) {
  // 19 bytes per assign
  uint16_t addr = 0x8c + (number * 19);
  return 0x18000000 + ((addr >> 7) << 8) + (addr & 0x7F) ;
}


FLASHMEM bool MD_GR55_class::change_active_assign_sources(uint8_t from_value, uint8_t to_value, uint8_t int_pdl_source_value, uint8_t asgn_source_value) {
  bool int_pedal_source_found = false;
  for (uint8_t number = 0; number < GR55_NUMBER_OF_ASSIGNS; number++) {
    DEBUGMSG("Checking assign " + String(number) + ", byte[0]:" + String(assign_area[number][0]) + ", byte[13]:" + String(assign_area[number][10]));
    bool assign_change = (assign_area[number][0] == 1) && (assign_area[number][10] == from_value);
    bool int_pdl_change = (assign_area[number][0] == 1) && (assign_area[number][10] == GR55_SOURCE_INT_PEDAL) && (assign_area[number][14] == int_pdl_source_value);
    if (assign_change || int_pdl_change) {
      uint32_t my_address = calculate_full_assign_address(number);
      //write_sysex(my_address + 21, to_value);
      uint16_t sum = 0;
      uint8_t *ad = (uint8_t*)&my_address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
      uint8_t sysexmessage[GR55_NUMBER_OF_ASSIGN_BYTES_READ + 13] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x53, 0x12, ad[3], ad[2], ad[1], ad[0]};
      for (uint8_t i = 0; i < sizeof(assign_area[0]); i++) {
        sysexmessage[i + 11] = assign_area[number][i];
      }
      if (assign_change) sysexmessage[21] = to_value;
      if (int_pdl_change) {
        sysexmessage[21] = asgn_source_value;
        int_pedal_source_found = true;
        int_pdl_speed = pow(assign_area[number][15] + 10, 3) / 40;
      }
      for (uint8_t i = 7; i < GR55_NUMBER_OF_ASSIGN_BYTES_READ + 11; i++) sum += sysexmessage[i];
      sysexmessage[GR55_NUMBER_OF_ASSIGN_BYTES_READ + 11] = calc_Roland_checksum(sum);
      sysexmessage[GR55_NUMBER_OF_ASSIGN_BYTES_READ + 12] = 0xF7;
      check_sysex_delay();
      MIDI_send_sysex(sysexmessage, GR55_NUMBER_OF_ASSIGN_BYTES_READ + 13, MIDI_out_port);
    }
  }
  return int_pedal_source_found;
}

FLASHMEM uint16_t MD_GR55_class::find_active_assign_sources(uint8_t source) {
  for (uint8_t number = 0; number < GR55_NUMBER_OF_ASSIGNS; number++) {
    if ((assign_area[number][0] == 1) && (assign_area[number][10] == source)) {
      return (assign_area[number][1] << 8) + (assign_area[number][2] << 4) + assign_area[number][3];
    }
  }
  return NOT_FOUND;
}

// Measured time of internal pedal
// Speed 100 takes 25 sec to go from min to max value
// Speed 75 takes 10 sec, Speed 50 takes 2,5 sec and speed 25 takes 0.5 sec
// Formulae for total time has been found by trial and error

FLASHMEM void MD_GR55_class::check_int_pdl_cc_timer() {
  if (int_pdl_cc_val == 127) return;
  uint32_t used_time = millis() - int_pdl_timer;
  uint8_t new_val = 127 * used_time / int_pdl_speed;
  if (new_val != int_pdl_cc_val) {
    int_pdl_cc_val = new_val;
    MIDI_send_CC(94, int_pdl_cc_val, MIDI_channel, MIDI_out_port);
    delay(3);
  }
}

// Scene assigns - assigns used in scenes

FLASHMEM void MD_GR55_class::initialize_scene_assigns() {
  for (uint8_t sa = 0; sa < GR55_NUMBER_OF_SCENE_ASSIGNS; sa++) scene_assign_state[sa] = false;
}

FLASHMEM void MD_GR55_class::toggle_scene_assign(uint8_t number) {
  if (number >= GR55_NUMBER_OF_SCENE_ASSIGNS) return;
  uint8_t cc = number + GR55_FIRST_SCENE_ASSIGN_SOURCE_CC;
  scene_assign_state[number] ^= 1; // Toggle state

  if (scene_assign_state[number]) MIDI_send_CC(cc, 127, MIDI_channel, MIDI_out_port);
  else MIDI_send_CC(cc, 0, MIDI_channel, MIDI_out_port);

  String msg = "CC #" + String(cc);
  uint8_t asgn = check_for_scene_assign_source(cc);
  if (asgn > 0) msg += " (ASGN " + String(asgn) + ')';
  LCD_show_popup_label(msg, ACTION_TIMER_LENGTH);
}

FLASHMEM void MD_GR55_class::set_scene_assign_states(uint8_t my_byte) {
  for (uint8_t sa = 0; sa < GR55_NUMBER_OF_SCENE_ASSIGNS; sa++) {
    scene_assign_state[sa] = bitRead(my_byte, sa);
  }
}

FLASHMEM uint8_t MD_GR55_class::read_scene_assign_state() {
  uint8_t my_byte = 0;
  for (uint8_t sa = 0; sa < GR55_NUMBER_OF_SCENE_ASSIGNS; sa++) {
    bitWrite(my_byte, sa, scene_assign_state[sa]);
  }
  return my_byte;
}

FLASHMEM void MD_GR55_class::check_scene_assigns_with_new_state(uint8_t new_byte) {
  for (uint8_t sa = 0; sa < GR55_NUMBER_OF_SCENE_ASSIGNS; sa++) {
    bool new_state = bitRead(new_byte, sa);
    if (scene_assign_state[sa] != new_state) toggle_scene_assign(sa);
  }
}

FLASHMEM uint8_t MD_GR55_class::check_for_scene_assign_source(uint8_t cc) {
  uint8_t my_source = 255;
  if ((cc >= 1) && (cc <= 31)) my_source = cc + 8; // Trigger is cc01 - cc31 (source for CC#01 is 9)
  else if ((cc >= 64) && (cc <= 95)) my_source = cc - 23; // Trigger is cc64 - cc95 (source for CC#64 is 41)
  for (uint8_t a = 0; a < GR55_NUMBER_OF_ASSIGNS; a++) { // Check if assign is on and my_source is correct
    if ((assign_area[a][0] == 0x01) && (assign_area[a][10] == my_source)) return a + 1;
  }
  return 0;
}

// ********************************* Section 7: GR55 preset names ********************************************

#ifndef SKIP_GR55_PRESET_NAMES
const PROGMEM char GR55_preset_patch_names[GR55_NUMBER_OF_FACTORY_PATCHES][17] = {
  // LEAD
  "Metal Synth Lead", "Rock Lead Organ", "GR-300 Ctl:+1oct", "Nice Tenor", "Flute Solo", "Jazz Guitar Vibe", "Legato Solo", "SlowAttack Solo",
  "Synth Brass Lead", "Drive Blues Harp", "Tp Section", "Mellow Cello", "Strange Whistle", "Emotional Lead", "Wave Synth Solo", "Dual Sync Lead",
  "Funky Syn Lead", "SqrPipe For You", "Concert Grand", "Mute Trumpet/Exp", "Epf + 335 Unison", "P90 & Organ Bell", "Feedback Guitar", "CTL=DLY/EXP=WAH",
  "More Blacklord", "Pdl Bend Guitar", "Poly Distortion", "NaturalResoLead", "Organ Syn Lead", "Crims-O-Tron", "Dist Sync Lead", "5th Layer",
  "Screamin Lead", "Portamento Lead", "Dist Sine Solo", "Dist Square Lead", "Buzz Lead", "Metal Saw Lead", "BrassyLead", "Long Echo Lead",
  "RockyOrgan", "Mild Saw Lead", "Simple Square", "+1oct Mild Lead", "Unison Lead", "Lead Beast", "Dream Bell", "Female Chorus",
  "70s Unison", "Comfortable Solo", "Wah Feedback", "Gtr+Organ Unison", "Vibraphone", "Dark Trumpet", "High Note Tp", "Fat Brass Sec",
  "Solo Fr.Horn", "SGT Fr Horn", "Solo Trombone", "Super Low Brass", "Clarinet>EXP Vib", "Oboe", "Soprano Sax", "Alto Sax",
  "Moody Sax", "Guitar+SaxUnison", "Flute+Gtr Unison", "Pan Flute", "Piccolo", "Flutey GT", "Heaven Ocarina", "LofiFlute&Glockn",
  "Recorder", "Chromatic Harmo", "Filter Harp", "Gt + Harmonica", "Heavy Harmonica", "Lead Violin", "Dist Violin", "Drive+Vln+Cello",
  "Double Cello", "Glass Cello", "Overdrive+Cello", "Smooth Lead+Vln", "Brass + Drive", "Organ,Pf & OD Gt", "Shamisen", "for Normal PU L1",
  "for Normal PU L2", "for Normal PU L3", "Heavy PdlBend", "Hard St/Syn FX", "StackOfSoloSynth", "Captain Nylon", "HarpNylon&String", "Sync Key Vox Gt",
  "Liquid Baritone", "String Quartet", "Sax over Organ", "FullBeardBoogie", "MahoganyTones", "EuropeanFeedback", "Funkenstein Bass", "Fuzz Bass&Syn Bs",
  "Weather Forecast", "FlyingJuno Brass", "Drop D Trance", "Soft Syn Lead", "Soft Res Lead", "Octa Sync Mix", "Filtered PolySyn", "Fat Power Note",
  "Anthem Approved", "MILD CLEAN", "Mild Octive", "Bright+1octive", "+1 OctModulation", "Lipstack Dly", "Heavy Gt", "Saturated Dreams",

  // RHYTHM
  "12st AG & Ch Org", "DoubleFlatHeavy", "SoftBrightPad+L4", "Rich Strings", "Poly Sitar", "HeavyBrassRock", "Syn Str.Pdl Reso", "TB-303 Bass",
  "AG+Bell Pad", "Double Low Piano", "E.Piano", "Xylophone Plus", "30 String Guitar", "ST + Tweed", "LP + Stack", "AcGt12st+Strings",
  "Jazz Guitar", "TL&Rotary Organ", "Ballade Wurly", "RnB Section", "Nylon Gt+Strings", "Symphonic Rock!", "GR Brass+Strings", "RockInCathedral",
  "DADGAD Phaser", "Asian DADGAD", "TL+StFlanger Pad", "Heavy Gt W/Sweep", "Fat Drive Mix", "Bright Gtr + Pad", "Electric 12str", "AC->12stAC(CTL)",
  "Nylon String Gtr", "Pedal Wah", "Stolling Rones", "Flat Tuned Drive", "BlueGrass 12-St", "Bell Clean", "AG & Epf", "HnkyTonk Piano",
  "Phaser E.Pf", "Piano + Anlg Pad", "Dyno Epf w/Pad", "ST+FM Epf+Voice", "Drive Wurly", "80s Piano", "Analog Clav S&H", "E.Piano/AcPiano",
  "Pipe Organ", "Cheap Organ", "3xOrganPower", "Simple Clavi", "R12st+Clavi+Xylo", "Harpsichord CTL", "Celesta", "Accordion",
  "Bell&Mallet+(Bs)", "TE+FM Bell Pad", "Marimba", "SteelDrums/Ethno", "Voice Pad SL", "AG+Voice", "Rotary G & Pad", "Gt & Vo Unison",
  "Vox+Pf+Crystal", "Crunch & Voice", "80s Stack Piano", "Like 60s", "Reed Organ(+LP)", "Full Section", "Real & Syn Brass", "Edge Brass",
  "Orchestra", "Pizzicato Gt", "Flange Strings", "Phase Strings", "SynthBrass", "Blade Running", "Seychelles Tour", "EmotionalBallad",
  "Analog Voice Pad", "-2 Tubular & LP", "Bridge of Sy's", "Faded Cherry", "Acid Bass", "Acoustic Bass", "Heavy P-Funk BS", "for Normal PU R1",
  "for Normal PU R2", "for Normal PU R3", "DreaminResonator", "NashvilleRoads", "MoodyBaritoneGTR", "Rotary Poly Key", "Syn Brs&Ana Bell", "ClnCho EXP>Bell",
  "OpenE Repeater", "ES335 Bright", "Dynamic TL!", "Reggae Ricky", "Heavy EXPsw Up 5", "Tight Tele Tack", "Tele Tastic", "Bright ST R+C",
  "Power Ac.Guitar", "Mild Nylon Gt", "Sitar", "Mandlin&AG+Acord", "Kalimba Pad", "AsianOpenG-Slide", "HarpsiOrch+12stG", "Open G Dulcimers",
  "Rotary Wurly Pls", "80s Analog Mix", "Crunch LP&St Pad", "DADGAD Crunch", "Gt->Rock Bass", "Asian Edge", "MostBeautifulGTR", "Stack Of Blues",

  // Other
  "Ultimate Pulse", "Heavy Hit&Groove", "Jazz Trio", "Seq*Tempo Dly+EG", "DarkSideOfTheSun", "Koto Dreams", "Voice Hit", "Heavens Bells",
  "Sine Air Bend", "Question+Answer", "Metamorphosis", "HighlanderGTR", "Sitar Fantasy", "GR-300 Triplet", "Noize Mix Drive", "Scat & Guitar",
  "SE Pad & LP+MS", "DancingAcoustic", "Heavy Pulse", "New Waves", "FourthOfFifth", "E Sitar& Dly Toy", "Trio Concerto", "Paradise Lost",
  "Trademark Riff", "Touchy 5th", "Scuba-Diving", "Big Syn Drum", "Sequence Clean", "Acoustic Heaven", "SparkleBellGTR", "Metal Timpani",
  "Cheezy Movie", "Stalker Violin", "OverblownClnGTR", "MotionBuilder", "Pulsing Bell+EG", "Flying Tremolo", "Trance Organ", "Sequence Trio",
  "Extreme FX", "Rhythmic Pulse", "Scared Score", "EasternFluteGT", "Odd Guitar", "DissonantBeauty", "PluckdBaritoned", "GroovePusher",
  "JazzEP/BassSplit", "Metal Scat", "Quantum Physics", "Enigmatic Rick", "Euro Beat Slicer", "Fuzz Heaven", "Arabian Nights", "Morpheus",
  "Unison+5thPower", "BassFluteSaxTrio", "Exorbitanz", "Armageddon", "Grinder", "EmoCarillion", "Unbelievable", "FAB 4 Together",
  "Esoteric Vibe", "Deja Vu Bass", "GK Paradise", "Is Dis Fat?", "Gladiator", "SlowGearSynth", "Oxygen Lead", "SteelPan + Agogo",
  "Ghostly", "Sneaking Up", "Big Ben", "AggroClav", "Cinematic Art", "Strictly E", "Beat Provider", "Shanai+Rhythm",
  "BackToDaCrib", "Hyper TE Beat", "House Fire", "Trance Groove", "Rainstorm", "Scary Scream", "Comedian", "for Normal PU O1",
  "for Normal PU O2", "for Normal PU O3", "Fantasy E.Guitar", "SpaceAltar", "ElectroG&Passing", "Sweep & Mod", "Tremolo Morphin", "Fairy Jazz GT",
  "Fine Wine(DropD)", "DeepWater(OpenE)", "Bubble in Heaven", "Lo B Rush Hour", "Trance Mission", "Ultraslow Groove", "80`s Kraftgroove", "Trancy CTL=BPM",
  "Trancy Riff BPM", "Slicer Change", "Drop-D Slices", "Bell&SynBrass Gt", "GR-Wonderland", "FallDown(ExpPdl)", "GtrBell (+ExpSw)", "ReverseGt+St Pad",
  "SingleNoteOrch.", "Shadow Crunch Gt", "Dbl Crystal Bell", "Analog Seq & Dly", "Hold Bass>Wah LD", "Tap Dance Guitar", "CompuRhythm", "BritishRaceTrack"
};

#define GR55_NUMBER_OF_FACTORY_BASS_PATCHES 108

const PROGMEM char GR55_preset_bass_patch_names[GR55_NUMBER_OF_FACTORY_BASS_PATCHES][17] = {
  // LEAD
  "Double Sting Bs", "Oct Unison Lead", "Cotton Harp", "Jazz Trio", "Mond MG Lead", "Pipe & Organ", "Indian Fretless", "EP Unison", "Mellow Fretless",
  "AnalogBass+Pedal", "OrgBass+PedalSyn", "ModBass+PedalSyn", "Deep Ensmble", "Rock Organic", "Pedal Synth Bend", "Soft Lead", "70s Mond Org", "Flange GR-500",
  "Solo Cello", "Trumpet&Strings", "OctaPiano", "Strings&FL Sound", "Ska Melody", "Spacy Jazz Bass", "Delayed Nylon", "Experience", "Extreme Dist",
  "for Normal PU L1", "for Normal PU L2", "for Normal PU L3", "Saxy Bass", "Fat SynthBass 8+", "SINE UNISON BASS", "COOL JUMP BASS", "LOW OCT SAW", "Heavy Dst Organ",

  // RHYTHM
  "Super Saw Bass", "M-Man Brass", "Fat Upright", "Organ ViolinBass", "Bell Sweep Bass", "Heavy E.Piano", "Shaker Synth", "FilterBassSynth", "MM & Fat Poly",
  "FastTrackin'Bass", "Soft Bass", "BRightJB+SynBass", "Fat Synth Bass", "Big Synth", "DecayFilterBass", "Bass Synth", "Reso Fuzz Bass", "ACID CLAV",
  "SPace Funk", "Trem E.Piano", "Bass + Clav", "OctaClavz", "High Strings", "Brass Mix", "Organ Bass", "Octave M-Man", "P-Bass Crunch",
  "for Normal PU R1", "for Normal PU R2", "for Normal PU R3", "Rhds Piano Bass", "Analog-y", "Fat Alpha Bass", "Synth Reso Bass", "MONSTER BASS SAW", "SlapSynthCTL+EXP",

  // Other
  "Ultimate Pulse", "Ambient Sparkle", "Auto Groove", "Avalon", "Bollywood Stack", "Gel Sequence", "Seq.Str.Hit", "Vint Seq.Bass", "Techno Sequence",
  "Tubular Strings", "TIME>TRAVELER", "STRINGTHEORY", "Ambient Organ", "RingLoop&E.Piano", "Unknown Kingdom", "Arrival Of King", "Ringing Bell", "TOKYO LIGHTS",
  "Sad Memory", "Wandering Pipe", "LUNAR LANDING", "Techno Opening", "Inner Journey", "HOUSE PARTY", "Compu-Strings", "5th & Groovin'", "Shamisen Beat",
  "for Normal PU O1", "for Normal PU O2", "for Normal PU O3", "WINE-N-BASSDELAY", "AEROPLANESUSTAIN", "Space SAW", "Gtr Hi/Bass Lo", "CompuRhythm", "S&H Groove/CTL"
};
#endif

FLASHMEM void MD_GR55_class::read_preset_name(uint8_t number, uint16_t patch) {
  String lbl;
#ifndef SKIP_GR55_PRESET_NAMES
  if (!bass_mode) lbl = GR55_preset_patch_names[patch - GR55_NUMBER_OF_USER_PATCHES]; //Read the label from the guitar array
  else lbl = GR55_preset_bass_patch_names[patch - GR55_NUMBER_OF_USER_PATCHES]; //Read the label from the bass array
#else
  LCD_clear_string(lbl);
#endif
  LCD_set_SP_label(number, lbl);
  if (patch == patch_number) current_patch_name = lbl; //Keeps the main display name updated
}

// ********************************* Section 7: GR55 Expression pedal control ********************************************

FLASHMEM void MD_GR55_class::move_expression_pedal(uint8_t sw, uint8_t value, uint8_t exp_pedal) {
  LCD_show_bar(0, value, 0); // Show it on the main display
  uint8_t cc;
  if (exp_pedal == 0) exp_pedal = current_exp_pedal;
  if (exp_pedal == 2) cc = 10;
  else cc = 7;
  MIDI_send_CC(cc, value, MIDI_channel, MIDI_out_port);
  LCD_show_popup_label(SP[sw].Label, MESSAGE_TIMER_LENGTH);
  //update_page = REFRESH_FX_ONLY; // To update the other switch states, we re-load the current page*/
}

FLASHMEM void MD_GR55_class::toggle_expression_pedal(uint8_t sw) {
  if (current_exp_pedal == 0) return;
  current_exp_pedal++;
  if (current_exp_pedal > 2) current_exp_pedal = 1;
  // Send toe switch message
  uint8_t value = 0x00;
  if (current_exp_pedal == 2) value = 0x01;
  write_sysex(0x1800004D, value);
  SP[sw].PP_number = GR55_EXP_SW;
  read_parameter(sw, exp_sw_type, 0); // Update label temporary to show switch state
  LCD_show_popup_label(SP[sw].Label, ACTION_TIMER_LENGTH);
  update_exp_label(sw);
  update_page = REFRESH_FX_ONLY;
}

FLASHMEM bool MD_GR55_class::request_exp_pedal(uint8_t sw, uint8_t exp_pedal) { // Used for both Master_exp_pedal and toggle_exp_pedal
  if (exp_pedal == 0) exp_pedal = current_exp_pedal;
  if (exp_pedal <= 2) {
    // Check if has been assigned
    uint8_t cc_asgn;
    if (exp_pedal == 1) cc_asgn = 15; // CC #7
    else cc_asgn = 18; // CC #10
    uint16_t target = find_active_assign_sources(cc_asgn);
    uint8_t par = find_target(target);
    DEBUGMSG("!!! Exp pedal:" + String(exp_pedal) + " target:" + String(target) + " par:" + String(par));
    if (par == NOT_FOUND) {
      if (exp_pedal == 1) LCD_set_SP_label(sw, "CC #7");
      else LCD_set_SP_label(sw, "CC #10");
    }
    else {
      LCD_set_SP_label(sw, GR55_parameters[par].Name);
    }
    return true;
  }
  LCD_clear_SP_label(sw);
  return true;
}

FLASHMEM void MD_GR55_class::update_exp_label(uint8_t sw) {
  if (current_exp_pedal == 1) {
    SP[sw].PP_number = GR55_EXP;
    read_parameter(sw, exp_type, 0);
  }
  if (current_exp_pedal == 2) {
    SP[sw].PP_number = GR55_EXP_ON;
    read_parameter(sw, exp_on_type, 0);
  }
}

// ********************************* Section 8: GR55 Scene control *******************************************************

// Uses GR55_scene_buffer of 192 bytes - same size as a Katana patch
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

#define GR55_PATCH_TYPE 0
#define GR55_PATCH_NUMBER_MSB 1
#define GR55_PATCH_NUMBER_LSB 2
#define GR55_CURRENT_SCENE_BYTE 5
#define GR55_SCENE_ACTIVE_BYTE 6
#define GR55_MUTE_DURING_SCENE_CHANGE_BYTE 7

// Structure of scene data
// Scene byte 0 - 11: Store common data
// Scene byte 12 - 13: COSM gtr type dependant data
// Scene byte 14: Scene assign states
// Scene byte 15 - 22: Scene name

#define GR55_SCENE_SIZE 23
#define GR55_COMMON_DATA_SIZE 8

#define GR55_INST_DATA_OFFSET 12
#define GR55_SCENE_MOMENTARY_BYTE 11
#define GR55_SCENE_ASSIGN_BYTE 14
#define GR55_SCENE_NAME_BYTE 15

struct GR55_scene_parameter_struct { // Combines all the data we need for controlling a parameter in a device
  uint16_t Address;
  uint8_t Bit_address;
  uint8_t Bit_mask;
};

const PROGMEM GR55_scene_parameter_struct GR55_scene_parameters[] {
  //{ 0x2001, 0x00, B11111111 }, // PCM 1 tone (part 1) // Space is reserved, but not read here
  //{ 0x2002, 0x10, B00000011 }, // PCM 1 tone (part 2)
  { 0x2003, 0x12, B00000001 }, // PCM 1 mute
  { 0x2005, 0x13, B00000111 }, // PCM 1 octave shift
  { 0x2006, 0x16, B00000001 }, // PCM 1 chromatic
  { 0x2007, 0x17, B00000001 }, // PCM 1 legato
  //{ 0x2101, 0x20, B11111111 }, // PCM 2 tone (part 1) // Space is reserved, but not read here
  //{ 0x2102, 0x30, B00000011 }, // PCM 2 tone (part 2)
  { 0x2103, 0x32, B00000001 }, // PCM 2 mute
  { 0x2105, 0x33, B00000111 }, // PCM 2 octave shift
  { 0x2106, 0x36, B00000001 }, // PCM 2 chromatic
  { 0x2107, 0x37, B00000001 }, // PCM 2 legato
  { 0x2008, 0x40, B00000001 }, // PCM 1 nuance
  { 0x2108, 0x41, B00000001 }, // PCM 2 nuance
  { 0x100A, 0x42, B00000001 }, // COSM gt sw
  { 0x1000, 0x43, B00000011 }, // COSM gt tone category
  { 0x1005, 0x45, B00000011 }, // COSM bass tone category
  { 0x0232, 0x47, B00000001 }, // Normal PU mute
  { 0x0234, 0x50, B00000001 }, // Alt tune sw
  { 0x0235, 0x51, B00001111 }, // Alt tune type
  { 0x0600, 0x55, B00000001 }, // CH sw
  { 0x0601, 0x56, B00000011 }, // CH type
  { 0x0605, 0x60, B00000001 }, // DLY sw
  { 0x0606, 0x61, B00000111 }, // DLY type
  { 0x060C, 0x64, B00000001 }, // RVB sw
  { 0x060D, 0x65, B00000111 }, // RVB type
  { 0x0611, 0x70, B00000001 }, // EQ sw
  { 0x0304, 0x71, B00000001 }, // MFX sw
  { 0x0305, 0x72, B00011111 }, // MFX type
  { 0x0700, 0x77, B00000001 }, // Amp sw
  { 0x0701, 0x80, B00111111 }, // Amp type
  { 0x070B, 0x86, B00000001 }, // Amp bright sw
  { 0x0704, 0x90, B00000011 }, // amp gain sw
  { 0x0705, 0x87, B00000001 }, // amp solo
  { 0x0715, 0x92, B00000001 }, // MOD sw
  { 0x0716, 0x93, B00001111 }, // MOD type
  { 0x101D, 0x97, B00000001 }, // 12 str sw
  { 0x075A, 0xA0, B00000001 }, // ns sw
  { 0x070C, 0xA1, B00001111 }, // amp speaker type
  { 0x070D, 0xA5, B00000111 }, // amp mic type
  // 0xB0 - B3 = Scene momentary state
  { 0x8007, 0xB3, B00000001 }, // G2M switch
  { 0x8008, 0xB4, B00000001 }, // G2M mode
  { 0x8009, 0xB5, B00000001 }, // G2M chrom
};

#define PCM1_SW_ITEM 0
#define PCM1_OCTAVE_SHIFT 1
#define PCM2_SW_ITEM 4
#define PCM2_OCTAVE_SHIFT 5
#define COSM_GTR_SW_ITEM 10
#define COSM_GTR_TYPE_ITEM 11
#define COSM_BASS_TYPE_ITEM 12
#define NORM_PU_ITEM 13

const uint16_t GR55_NUMBER_OF_SCENE_PARAMETERS = sizeof(GR55_scene_parameters) / sizeof(GR55_scene_parameters[0]);

struct GR55_scene_inst_parameter_struct { // Combines all the data we need for controlling a parameter in a device
  uint16_t Address;
  uint8_t Bit_address;
  uint8_t Bit_mask;
};

#define GR55_NUMBER_OF_INST_PARAMETERS 10

const PROGMEM GR55_scene_inst_parameter_struct GR55_scene_inst_parameters[][GR55_NUMBER_OF_INST_PARAMETERS] {
  { // EGTR (0)
    { 0x1001, 0x00, B00001111 }, // EGTR type
    { 0x102E, 0x04, B00000011 }, // Pickup sw 1
    { 0x102F, 0x10, B00000111 }, // Pickup sw 2
    { 0x1030, 0x13, B00000111 }, // Pickup sw 3
    { 0x102B, 0x16, B00000001 }, // NS switch
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
  },
  { // AC (1)
    { 0x1002, 0x00, B00000111 }, // AC type
    { 0x1033, 0x04, B00000111 }, // Steel type
    { 0x1039, 0x10, B00000011 }, // Sitar pickup
    { 0x102B, 0x12, B00000001 }, // NS switch
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
  },
  { // E.BASS (2)
    { 0x1003, 0x00, B00000001 }, // BASS TYPE
    { 0x102B, 0x01, B00000001 }, // NS switch
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
  },
  { // SYNTH (3)
    { 0x1004, 0x00, B00000111 }, // SYNTH Type
    { 0x104B, 0x03, B00000011 }, // GR300 mode
    { 0x104C, 0x05, B00000001 }, // GR300 comp
    { 0x104F, 0x16, B00000011 }, // GR300 env mod sw
    { 0x1052, 0x20, B00000011 }, // GR300 pitch sw
    { 0x1057, 0x22, B00000001 }, // GR300 pitch duet
    { 0x1058, 0x23, B00000001 }, // GR300 sweep sw
    { 0x105B, 0x24, B00000001 }, // GR300 vibrato
    { 0x105E, 0x25, B00000001 }, // WAVE synth type
    { 0x102B, 0x26, B00000001 }, // NS switch
  },
  { // BM:EBASS(4)
    { 0x1006, 0x00, B00001111 }, // BASS type
    { 0x111E, 0x04, B00000011 }, // BASS Pickup select
    { 0x111F, 0x07, B00000001 }, // Bass treble sw
    { 0x1120, 0x10, B00000001 }, // Bass bass sw
    { 0x1121, 0x11, B00000001 }, // Bass rhythm/solo sw
    { 0x102B, 0x12, B00000001 }, // NS switch
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
  },
  { // BM:SYNTH (5)
    { 0x1007, 0x00, B00000111 }, // SYNTH Type
    { 0x1122, 0x03, B00000011 }, // GR300 mode
    { 0x1123, 0x05, B00000001 }, // GR300 comp
    { 0x1126, 0x06, B00000011 }, // GR300 env mod sw
    { 0x1129, 0x10, B00000001 }, // GR300 pitch sw
    { 0x112E, 0x11, B00000001 }, // GR300 pitch duet
    { 0x112F, 0x12, B00000001 }, // GR300 sweep sw
    { 0x1132, 0x13, B00000001 }, // GR300 vibrato
    { 0x1135, 0x14, B00000001 }, // WAVE synth type
    { 0x102B, 0x15, B00000001 }, // NS switch
  },
  { // BM:E.GTR (6)
    { 0x1003, 0x00, B00000001 }, // GTR TYPE
    { 0x1110, 0x01, B00000011 }, // GTR Pickup select 3
    { 0x1111, 0x03, B00000111 }, // GTR Pickup select 5
    { 0x102B, 0x06, B00000001 }, // NS switch
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
    { 0x0000, 0x00, 0,        },
  },
};

#define GR55_TOTAL_NUMBER_OF_SCENE_PARAMETERS GR55_NUMBER_OF_INST_PARAMETERS + GR55_NUMBER_OF_SCENE_PARAMETERS

FLASHMEM uint32_t MD_GR55_class::get_scene_inst_parameter_address(uint16_t number) {
  uint32_t address;
  if (number < GR55_NUMBER_OF_INST_PARAMETERS) {
    uint8_t inst_type;
    if (!bass_mode) inst_type = COSM_gtr_type;
    else inst_type = COSM_bass_type + 4;
    address = GR55_scene_inst_parameters[inst_type][number].Address;
    return 0x18000000 + address;
  }
  else return 0;
}

FLASHMEM uint32_t MD_GR55_class::get_scene_parameter_address(uint16_t number) {
  uint32_t address;
  if (number < GR55_NUMBER_OF_SCENE_PARAMETERS) {
    address = GR55_scene_parameters[number].Address;
    if (address < 0x8000) return 0x18000000 + address;
    else return 0x02000000 + (address & 0x7FFF);
  }
  else return 0;
}


FLASHMEM void MD_GR55_class::get_snapscene_title(uint8_t number, String & Output) {
  Output += "SCENE " + String(number);
}

FLASHMEM void MD_GR55_class::get_snapscene_label(uint8_t number, String & Output) {
  read_scene_name_from_buffer(number);
  Output += scene_label_buffer;
}

FLASHMEM bool MD_GR55_class::request_snapscene_name(uint8_t sw, uint8_t sw1, uint8_t sw2, uint8_t sw3) {
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

FLASHMEM void MD_GR55_class::set_snapscene(uint8_t sw, uint8_t number) {
  if (!is_on) unmute();
  if ((number < 1) || (number > 8)) return;
  bool loaded = load_scene(last_loaded_scene, number);
  //set_snapscene_number_and_LED(number);
  current_snapscene = number;
  if ((loaded) && (sw > 0)) {
    read_scene_name_from_buffer(number);
    String msg = "Scene " + String(number) + ':' + scene_label_buffer;
    LCD_show_popup_label(msg, MESSAGE_TIMER_LENGTH);
  }
  MIDI_send_current_snapscene(my_device_number, current_snapscene);
  switch_scene_momentary_inst(false);

  int_pdl_speed = 60000 * 8 / Setting.Bpm; // Send cc#94 internal pedal emulation on scene change which goes from 0 - 127 in 8 beats of the current bpm
  int_pdl_cc_val = 0;
  MIDI_send_CC(94, int_pdl_cc_val, MIDI_channel, MIDI_out_port);
  int_pdl_timer = millis();
  update_main_lcd = true;
}

FLASHMEM void MD_GR55_class::release_snapscene(uint8_t  sw, uint8_t  number) {
  switch_scene_momentary_inst(true);
}

FLASHMEM void MD_GR55_class::show_snapscene(uint8_t  number) {
  if ((number < 1) || (number > 8)) return;
  if (number == current_snapscene) return;
  current_snapscene = number;
  update_main_lcd = true;
  DEBUGMSG("GR55 snapscene change to scene " + String(number));
}

FLASHMEM bool MD_GR55_class::load_scene(uint8_t prev_scene, uint8_t new_scene) {
  uint8_t my_byte, my_shift, my_mask;
  MIDI_debug_sysex(GR55_patch_buffer, VC_PATCH_SIZE, 255, true);
  bool load_all_data = false;
  if (prev_scene == new_scene) return false;
  if (!check_snapscene_active(new_scene)) return false;
  if (last_loaded_scene == 0) load_all_data = true;

  DEBUGMAIN("Loading scene " + String(new_scene) + " (prev scene:" + String(prev_scene) + ')');
  //control_edit_mode();

  // Read inst state and types:
  DEBUGMAIN("Checking INST changes");
  gr55_pcm1_mute = read_scene_data(new_scene, PCM1_SW_ITEM);
  gr55_pcm2_mute = read_scene_data(new_scene, PCM2_SW_ITEM);
  COSM_gtr_mute = read_scene_data(new_scene, COSM_GTR_SW_ITEM);
  COSM_gtr_type = read_scene_data(new_scene, COSM_GTR_TYPE_ITEM);
  COSM_bass_type = read_scene_data(new_scene, COSM_BASS_TYPE_ITEM);
  normal_pu_mute = read_scene_data(new_scene, NORM_PU_ITEM);

  uint8_t prev_cosm_gtr_type, prev_cosm_bass_type;
  prev_cosm_gtr_type = read_scene_data(prev_scene, COSM_GTR_TYPE_ITEM);
  prev_cosm_bass_type = read_scene_data(prev_scene, COSM_BASS_TYPE_ITEM);

  uint8_t index_new = get_scene_index(new_scene);
  uint8_t index_prev = get_scene_index(prev_scene);

  // Mute instruments
  mute_during_scene_change = check_mute_during_scene_change(new_scene);
  if (mute_during_scene_change) mute_now();

  uint8_t addr_index = 0;
  DEBUGMAIN("Checking INST to turn down of off");
  uint8_t inst_type_index;
  bool inst_changed;
  if (!bass_mode) {
    inst_type_index = COSM_gtr_type;
    inst_changed = (COSM_gtr_type != prev_cosm_gtr_type);
  }
  else {
    inst_type_index = COSM_bass_type + 4;
    inst_changed = (COSM_bass_type != prev_cosm_bass_type);
  }
  for (uint8_t par = 0; par < GR55_NUMBER_OF_INST_PARAMETERS; par++) {
    my_byte = (GR55_scene_inst_parameters[inst_type_index][par].Bit_address >> 4) + GR55_INST_DATA_OFFSET;
    my_shift = GR55_scene_inst_parameters[inst_type_index][par].Bit_address & 0x0F;
    my_mask = GR55_scene_inst_parameters[inst_type_index][par].Bit_mask << my_shift;

    if (my_mask != 0) {
      uint8_t new_data = GR55_patch_buffer[index_new + my_byte] & my_mask;
      uint8_t old_data = GR55_patch_buffer[index_prev + my_byte] & my_mask;
      if ((new_data < old_data) && (!inst_changed) && (!load_all_data)) {
        uint32_t address = get_scene_inst_parameter_address(addr_index);
        uint8_t data = new_data >> my_shift;
        write_sysex(address, data);
      }
    }
    addr_index++;
  }

  DEBUGMAIN("Checking common parameters to turn down or off");
  for (uint8_t i = 0; i < GR55_NUMBER_OF_SCENE_PARAMETERS; i++) {
    bool is_inst_sw = (i == PCM1_SW_ITEM) || (i == PCM2_SW_ITEM) || (i == COSM_GTR_SW_ITEM);
    if ((!mute_during_scene_change) || (!is_inst_sw)) {
      my_byte = GR55_scene_parameters[i].Bit_address >> 4;
      my_shift = GR55_scene_parameters[i].Bit_address & 0x0F;
      my_mask = GR55_scene_parameters[i].Bit_mask << my_shift;

      if (my_mask != 0) {
        uint8_t new_data = GR55_patch_buffer[index_new + my_byte] & my_mask;
        uint8_t old_data = GR55_patch_buffer[index_prev + my_byte] & my_mask;
        if ((new_data < old_data)  && (!load_all_data)) {
          uint32_t address = get_scene_parameter_address(i);
          uint8_t data = new_data >> my_shift;
          if ((i == PCM1_OCTAVE_SHIFT) || (i == PCM2_OCTAVE_SHIFT)) data += 61; // Octave shift is sent as 61 - 67.
          write_sysex(address, data);
        }
      }
    }
    addr_index++;
  }

  DEBUGMAIN("Checking synth types");
  uint16_t new_pcm1_type = (GR55_patch_buffer[index_new] << 2) + (GR55_patch_buffer[index_new + 1] & 3);
  uint16_t old_pcm1_type = (GR55_patch_buffer[index_prev] << 2) + (GR55_patch_buffer[index_prev + 1] & 3);
  DEBUGMSG("PCM1 old_type: " + String(old_pcm1_type) + ", new type: " + String(new_pcm1_type));
  if (new_pcm1_type != old_pcm1_type) write_sysex(GR55_PCM1_TYPE, 0x58, (new_pcm1_type >> 7), (new_pcm1_type & 0x7F));

  uint16_t new_pcm2_type = (GR55_patch_buffer[index_new + 2] << 2) + (GR55_patch_buffer[index_new + 3] & 3);
  uint16_t old_pcm2_type = (GR55_patch_buffer[index_prev + 2] << 2) + (GR55_patch_buffer[index_prev + 3] & 3);
  DEBUGMSG("PCM2 old_type: " + String(old_pcm2_type) + ", new type: " + String(new_pcm2_type));
  if (new_pcm2_type != old_pcm2_type) write_sysex(GR55_PCM2_TYPE, 0x58, (new_pcm2_type >> 7), (new_pcm2_type & 0x7F));


  DEBUGMAIN("Check scene assigns");
  check_scene_assigns_with_new_state(GR55_patch_buffer[index_new + GR55_SCENE_ASSIGN_BYTE]);
  scene_momentary_inst_state = GR55_patch_buffer[index_new + GR55_SCENE_MOMENTARY_BYTE] & 0x07;
  last_loaded_scene = new_scene;

  addr_index = 0;
  DEBUGMAIN("Checking INST to turn up or on");
  for (uint8_t par = 0; par < GR55_NUMBER_OF_INST_PARAMETERS; par++) {
    my_byte = (GR55_scene_inst_parameters[inst_type_index][par].Bit_address >> 4) + GR55_INST_DATA_OFFSET;
    my_shift = GR55_scene_inst_parameters[inst_type_index][par].Bit_address & 0x0F;
    my_mask = GR55_scene_inst_parameters[inst_type_index][par].Bit_mask << my_shift;

    if (my_mask != 0) {
      uint8_t new_data = GR55_patch_buffer[index_new + my_byte] & my_mask;
      uint8_t old_data = GR55_patch_buffer[index_prev + my_byte] & my_mask;
      if ((new_data > old_data) || (inst_changed) || (load_all_data)) {
        uint32_t address = get_scene_inst_parameter_address(addr_index);
        uint8_t data = new_data >> my_shift;
        write_sysex(address, data);
      }
    }
    addr_index++;
  }

  DEBUGMAIN("Checking common parameters to turn up or on");
  for (uint8_t i = 0; i < GR55_NUMBER_OF_SCENE_PARAMETERS; i++) {
    bool is_inst_sw = (i == PCM1_SW_ITEM) || (i == PCM2_SW_ITEM) || (i == COSM_GTR_SW_ITEM);
    if ((!mute_during_scene_change) || (!is_inst_sw)) {
      my_byte = GR55_scene_parameters[i].Bit_address >> 4;
      my_shift = GR55_scene_parameters[i].Bit_address & 0x0F;
      my_mask = GR55_scene_parameters[i].Bit_mask << my_shift;

      if (my_mask != 0) {
        uint8_t new_data = GR55_patch_buffer[index_new + my_byte] & my_mask;
        uint8_t old_data = GR55_patch_buffer[index_prev + my_byte] & my_mask;
        if ((new_data > old_data) || (load_all_data)) {
          uint32_t address = get_scene_parameter_address(i);
          uint8_t data = new_data >> my_shift;
          if ((i == PCM1_OCTAVE_SHIFT) || (i == PCM2_OCTAVE_SHIFT)) data += 61; // Octave shift is sent as 61 - 67.
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

FLASHMEM uint8_t MD_GR55_class::get_scene_index(uint8_t scene) {
  if (scene > 0) scene--;
  return (scene * GR55_SCENE_SIZE) + GR55_COMMON_DATA_SIZE;
}

FLASHMEM uint8_t MD_GR55_class::read_scene_data(uint8_t scene, uint8_t parameter) {
  uint8_t index = get_scene_index(scene);
  uint8_t my_byte = GR55_scene_parameters[parameter].Bit_address >> 4;
  uint8_t my_shift = GR55_scene_parameters[parameter].Bit_address & 0x0F;
  uint8_t my_mask = GR55_scene_parameters[parameter].Bit_mask;
  return (GR55_patch_buffer[index + my_byte] >> my_shift) & my_mask;
}

FLASHMEM uint8_t MD_GR55_class::read_temp_scene_data(uint8_t parameter) {
  uint8_t my_byte = GR55_scene_parameters[parameter].Bit_address >> 4;
  uint8_t my_shift = GR55_scene_parameters[parameter].Bit_address & 0x0F;
  uint8_t my_mask = GR55_scene_parameters[parameter].Bit_mask;
  return (scene_data_buffer[my_byte] >> my_shift) & my_mask;
}

FLASHMEM void MD_GR55_class::save_scene() {
  save_scene_number = current_snapscene;
  if (save_scene_number == 0) save_scene_number = 1;
  read_scene_name_from_buffer(save_scene_number);
  if ((scene_label_buffer[0] == '-') && (scene_label_buffer[1] == '-')) {
    scene_label_buffer[0] = '*';
    scene_label_buffer[1] = '*';
  }
  open_specific_menu = GR55 + 1;
  SCO_select_page(PAGE_MENU); // Open the menu
}

FLASHMEM void MD_GR55_class::store_scene() { // Returning from menu
  // Request the data from the GR55
  store_scene_name_to_buffer(save_scene_number);
  current_snapscene = save_scene_number;
  //set_snapscene_number_and_LED(save_scene_number);
  last_loaded_scene = save_scene_number;
  MIDI_disable_device_check();
  memset(scene_data_buffer, 0, GR55_SCENE_DATA_BUFFER_SIZE);
  request_sysex(GR55_PCM1_TYPE, 3);
  request_sysex(GR55_PCM2_TYPE, 3);
  request_scene_message(1);
  update_page = 0;
}

FLASHMEM void MD_GR55_class::update_change_on_all_scenes() {
  // Here we do the following steps:
  // 1) Read scene data to scene_data_buffer
  // 2) Compare the bytes of the scene_data_buffer to the last_loaded_scene - in check_delta_and_update_scenes()
  // 3) Write this difference to all scenes - in check_delta_and_update_scenes()
  save_scene_number = 9;
  memset(scene_data_buffer, 0, GR55_SCENE_DATA_BUFFER_SIZE);
  request_scene_message(1);
  update_page = 0;
}

FLASHMEM void MD_GR55_class::check_delta_and_update_scenes() {
  uint8_t my_byte, my_shift, my_mask;
  if (!check_snapscene_active(last_loaded_scene)) return;

  // Read inst state and types:
  uint8_t new_inst_type;
  DEBUGMAIN("Checking INST changes");
  if (!bass_mode) new_inst_type = read_temp_scene_data(COSM_GTR_TYPE_ITEM);
  else new_inst_type = read_temp_scene_data(COSM_BASS_TYPE_ITEM);


  uint8_t prev_inst_type;
  if (!bass_mode) prev_inst_type = read_scene_data(last_loaded_scene, COSM_GTR_TYPE_ITEM);
  else prev_inst_type = read_scene_data(last_loaded_scene, COSM_BASS_TYPE_ITEM);

  uint8_t index_prev = get_scene_index(last_loaded_scene);

  uint8_t addr_index = 0;
  DEBUGMAIN("Checking INST");
  uint8_t inst_type_index;
  bool inst_changed;
  if (!bass_mode) {
    inst_type_index = COSM_gtr_type;
    inst_changed = (COSM_gtr_type != prev_inst_type);
  }
  else {
    inst_type_index = COSM_bass_type + 4;
    inst_changed = (COSM_bass_type != prev_inst_type);
  }
  for (uint8_t par = 0; par < GR55_NUMBER_OF_INST_PARAMETERS; par++) {
    my_byte = (GR55_scene_inst_parameters[inst_type_index][par].Bit_address >> 4) + GR55_INST_DATA_OFFSET;
    my_shift = GR55_scene_inst_parameters[inst_type_index][par].Bit_address & 0x0F;
    my_mask = GR55_scene_inst_parameters[inst_type_index][par].Bit_mask << my_shift;

    if (my_mask != 0) {
      uint8_t new_data = scene_data_buffer[my_byte] & my_mask;
      uint8_t old_data = GR55_patch_buffer[index_prev + my_byte] & my_mask;
      if (((new_data != old_data) && (new_inst_type == prev_inst_type)) || (inst_changed)) {
        for (uint8_t s = 1; s <= 8; s++) { // Write to all active scenes
          if (check_snapscene_active(s)) {
            uint8_t index = get_scene_index(s);
            uint8_t data = GR55_patch_buffer[index + my_byte] & ~my_mask;
            GR55_patch_buffer[index + my_byte] = data | new_data;
          }
        }
      }
    }
    addr_index++;
  }

  DEBUGMAIN("Checking common parameters");
  for (uint8_t i = 0; i < GR55_NUMBER_OF_SCENE_PARAMETERS; i++) {

    my_byte = GR55_scene_parameters[i].Bit_address >> 4;
    my_shift = GR55_scene_parameters[i].Bit_address & 0x0F;
    my_mask = GR55_scene_parameters[i].Bit_mask << my_shift;

    if (my_mask != 0) {
      uint8_t new_data = scene_data_buffer[my_byte] & my_mask;
      uint8_t old_data = GR55_patch_buffer[index_prev + my_byte] & my_mask;
      if (new_data != old_data) {
        for (uint8_t s = 1; s <= 8; s++) { // Write to all active scenes
          if (check_snapscene_active(s)) {
            uint8_t index = get_scene_index(s);
            uint8_t data = GR55_patch_buffer[index + my_byte] & ~my_mask;
            GR55_patch_buffer[index + my_byte] = data | new_data;
          }
        }
      }
    }
    addr_index++;
  }
}

FLASHMEM void MD_GR55_class::request_scene_message(uint8_t number) {
  if (number == 0) return;
  if (number == 2) LCD_show_popup_label("Reading scene...", MESSAGE_TIMER_LENGTH);
  read_scene_parameter_number = number;
  if (number < GR55_NUMBER_OF_INST_PARAMETERS) {
    read_scene_parameter_address = get_scene_inst_parameter_address(number - 1);
    DEBUGMAIN("Requesting cosm item " + String(number - 1) + " at address " + String(read_scene_parameter_address, HEX));
  }
  else {
    read_scene_parameter_address = get_scene_parameter_address(number - GR55_NUMBER_OF_INST_PARAMETERS - 1);
    DEBUGMAIN("Requesting normal item " + String(number) + " at address " + String(read_scene_parameter_address, HEX));
  }
  if (read_scene_parameter_address != 0) {
    request_sysex(read_scene_parameter_address, 1);
    read_scene_midi_timer = millis() + GR55_READ_SCENE_MIDI_TIMER_LENGTH; // Set the timer
  }
  else request_scene_message(GR55_NUMBER_OF_INST_PARAMETERS + 1); // Continue to read the  normal scene parameters
}

void MD_GR55_class::check_read_scene_midi_timer() {
  if (read_scene_midi_timer > 0) { // Check timer is running
    if (millis() > read_scene_midi_timer) {
      DEBUGMSG("GR55 Midi timer expired!");
      request_scene_message(read_scene_parameter_number);
    }
  }
}

FLASHMEM void MD_GR55_class::read_scene_message(uint8_t number, uint8_t data) {
  uint8_t my_byte, my_shift, my_mask;
  if (number == 0) return;
  number--;

  if (number < GR55_NUMBER_OF_INST_PARAMETERS) {
    uint8_t inst_type_index;
    if (!bass_mode) {
      inst_type_index = COSM_gtr_type;
    }
    else {
      inst_type_index = COSM_bass_type + 4;
    }

    my_byte = (GR55_scene_inst_parameters[inst_type_index][number].Bit_address >> 4) + GR55_INST_DATA_OFFSET;
    my_shift = GR55_scene_inst_parameters[inst_type_index][number].Bit_address & 0x0F;
    my_mask = GR55_scene_inst_parameters[inst_type_index][number].Bit_mask;
    DEBUGMAIN("COSM gtr item:" + String(number) + ", my_byte:" + String(my_byte));
  }
  else {
    uint8_t index = number - GR55_NUMBER_OF_INST_PARAMETERS;
    if ((index == PCM1_OCTAVE_SHIFT) || (index == PCM2_OCTAVE_SHIFT)) data -= 61; // Octave shift is sent as 61 - 67.
    my_byte = GR55_scene_parameters[index].Bit_address >> 4;
    my_shift = GR55_scene_parameters[index].Bit_address & 0x0F;
    my_mask = GR55_scene_parameters[index].Bit_mask;
    DEBUGMAIN("COMMON ITEM " + String(index) + ": my_byte:" + String(my_byte) + ", my_shift:" + String(my_shift));
  }
  if (my_mask != 0) scene_data_buffer[my_byte] |= ((data & my_mask) << my_shift);

  if (number < GR55_TOTAL_NUMBER_OF_SCENE_PARAMETERS - 1) {
    request_scene_message(read_scene_parameter_number + 1); // Request next message
  }
  else { // Done reading messages for scene
    read_scene_parameter_number = 0;
    read_scene_midi_timer = 0;
    MIDI_enable_device_check();

    scene_data_buffer[GR55_SCENE_ASSIGN_BYTE] = read_scene_assign_state();
    scene_data_buffer[GR55_SCENE_MOMENTARY_BYTE] |= (scene_momentary_inst_state & 0x07);

    if (save_scene_number == 0) return;
    if (save_scene_number < 9) { // Copy scene_data_buffer to current_scene
      DEBUGMAIN("Patch scene read succesfully from GR55. Storing to scene " + String(save_scene_number));
      uint8_t b = get_scene_index(save_scene_number);
      //DEBUGMAIN("Scene index for storing: " + String(b));
      for (uint8_t i = 0; i < GR55_SCENE_DATA_BUFFER_SIZE; i++) {
        GR55_patch_buffer[b++] = scene_data_buffer[i];
      }
      set_mute_during_scene_change(save_scene_number, mute_during_scene_change);
      set_scene_active(save_scene_number);
      update_page = RELOAD_PAGE;

      //store_patch(patch_number);

      LCD_show_popup_label("Scene read!", MESSAGE_TIMER_LENGTH);
      // Dump data (debug)
      //DEBUGMAIN("GR55_scene_buffer dump:");
      //MIDI_debug_sysex(GR55_scene_buffer, VC_PATCH_SIZE, 255, true);
      //store_patch(patch_number);
      save_pc_after_patch_save();
    }
    else { // Update scene differences and write them to all scenes
      if (last_loaded_scene == 0) return;
      check_delta_and_update_scenes();
      load_scene(0, last_loaded_scene);
      //store_patch(patch_number);
    }
  }
  LCD_show_bar(0, map(number, 0, GR55_TOTAL_NUMBER_OF_SCENE_PARAMETERS - 1, 0, 127), 0);
}

FLASHMEM void MD_GR55_class::read_scene_pcm_type(uint8_t index, uint16_t data) {
  scene_data_buffer[index] = data >> 2;
  scene_data_buffer[index + 1] = data & 3;
  DEBUGMSG("Writing PCM data with index " + String(index) + " and data " + String(data));
}

FLASHMEM void MD_GR55_class::exchange_scene(uint8_t new_scene, uint8_t prev_scene) {
  if (new_scene == prev_scene) return;
  bool new_scene_active = check_snapscene_active(new_scene);
  bool prev_scene_active = check_snapscene_active(prev_scene);

  uint8_t index_new = get_scene_index(new_scene);
  uint8_t index_prev = get_scene_index(prev_scene);
  uint8_t temp;
  for (uint8_t i = 0; i < GR55_SCENE_DATA_BUFFER_SIZE + 8; i++) {
    temp = GR55_patch_buffer[index_prev];
    GR55_patch_buffer[index_prev] = GR55_patch_buffer[index_new];
    GR55_patch_buffer[index_new] = temp;
    index_new++;
    index_prev++;
  }

  if (new_scene_active) set_scene_active(prev_scene);
  else clear_scene_active(prev_scene);
  if (prev_scene_active) set_scene_active(new_scene);
  else clear_scene_active(new_scene);

  //set_snapscene_number_and_LED(new_scene);
  current_snapscene = new_scene;
  last_loaded_scene = 0;
}


FLASHMEM void MD_GR55_class::initialize_scene(uint8_t scene) {
  DEBUGMAIN("Initializing scene " + String(scene));
  uint8_t index = get_scene_index(scene);
  for (uint8_t i = 0; i < GR55_SCENE_DATA_BUFFER_SIZE + 8; i++) GR55_patch_buffer[index++] = 0;

  // Set default label
  scene_label_buffer[0] = '-';
  scene_label_buffer[1] = '-';
  for (uint8_t i = 2; i < 8; i++) scene_label_buffer[i] = ' ';
  store_scene_name_to_buffer(scene);

  clear_scene_active(scene);
  //set_snapscene_number_and_LED(0);
  current_snapscene = 0;
  last_loaded_scene = 0;
}

FLASHMEM bool MD_GR55_class::check_snapscene_active(uint8_t scene) {
  if ((scene == 0) || (scene > 8)) return false;
  scene--;
  return ((GR55_patch_buffer[GR55_SCENE_ACTIVE_BYTE] & (1 << scene)) != 0);
}

FLASHMEM void MD_GR55_class::set_scene_active(uint8_t scene) {
  if ((scene == 0) || (scene > 8)) return;
  scene--;
  bitSet(GR55_patch_buffer[GR55_SCENE_ACTIVE_BYTE], scene);
}

FLASHMEM void MD_GR55_class::clear_scene_active(uint8_t scene) {
  if ((scene == 0) || (scene > 8)) return;
  scene--;
  bitClear(GR55_patch_buffer[GR55_SCENE_ACTIVE_BYTE], scene);
}

FLASHMEM bool MD_GR55_class::check_mute_during_scene_change(uint8_t scene) {
  if ((scene == 0) || (scene > 8)) return false;
  scene--;
  return ((GR55_patch_buffer[GR55_MUTE_DURING_SCENE_CHANGE_BYTE] & (1 << scene)) != 0);
}

FLASHMEM void MD_GR55_class::set_mute_during_scene_change(uint8_t scene, bool value) {
  if ((scene == 0) || (scene > 8)) return;
  scene--;
  if (value) bitSet(GR55_patch_buffer[GR55_MUTE_DURING_SCENE_CHANGE_BYTE], scene);
  else bitClear(GR55_patch_buffer[GR55_MUTE_DURING_SCENE_CHANGE_BYTE], scene);
}

FLASHMEM void MD_GR55_class::read_scene_name_from_buffer(uint8_t scene) {
  uint8_t b = get_scene_index(scene) + GR55_SCENE_NAME_BYTE;
  for (uint8_t c = 0; c < 8; c++) {
    scene_label_buffer[c] = GR55_patch_buffer[b++];
  }
}

FLASHMEM void MD_GR55_class::store_scene_name_to_buffer(uint8_t scene) {
  uint8_t b = get_scene_index(scene) + GR55_SCENE_NAME_BYTE;
  for (uint8_t c = 0; c < 8; c++) GR55_patch_buffer[b++] = scene_label_buffer[c];
  //GR55_label_buffer[c] = '\n';
}

FLASHMEM void MD_GR55_class::load_patch(uint16_t number) {

  // Read from EEPROM
  if (bass_mode) number += GR55_BASS_MODE_NUMBER_OFFSET;
  bool loaded = EEPROM_load_device_patch(my_device_number + 1, number, GR55_patch_buffer, VC_PATCH_SIZE);

  if (loaded) {
    last_loaded_scene = GR55_patch_buffer[GR55_CURRENT_SCENE_BYTE];
  }
  else {
    initialize_patch_space();
    last_loaded_scene = 0;
  }
  current_snapscene = last_loaded_scene;
  uint8_t index = get_scene_index(current_snapscene);
  set_scene_assign_states(GR55_patch_buffer[index + GR55_SCENE_ASSIGN_BYTE]);
  scene_momentary_inst_state = GR55_patch_buffer[index + GR55_SCENE_MOMENTARY_BYTE] & 0x07;
  request_pc_after_patch_load();
  DEBUGMSG("GR55 patch loaded from EEPROM");
  //MIDI_debug_sysex(GR55_patch_buffer, VC_PATCH_SIZE, 255, true); // Show contents of patch buffer
}

FLASHMEM bool MD_GR55_class::store_patch(uint16_t number) {

  if (bass_mode) number |= GR55_BASS_MODE_NUMBER_OFFSET;

  // Check if scenes are empty
  if (GR55_patch_buffer[GR55_SCENE_ACTIVE_BYTE] == 0) {
    EEPROM_initialize_device_patch(my_device_number + 1, number);
    return false;
  }

  // Add header
  GR55_patch_buffer[GR55_PATCH_TYPE] = my_device_number + 1;
  GR55_patch_buffer[GR55_PATCH_NUMBER_MSB] = number >> 8; // Patch_number_MSB
  GR55_patch_buffer[GR55_PATCH_NUMBER_LSB] = number & 0xFF; // Patch_number_LSB

  // Store to EEPROM
  GR55_patch_buffer[GR55_CURRENT_SCENE_BYTE] = last_loaded_scene;
  //MIDI_debug_sysex(GR55_patch_buffer, VC_PATCH_SIZE, 255, true);  // Show contents of patch buffer
  return EEPROM_save_device_patch(my_device_number + 1, number, GR55_patch_buffer, VC_PATCH_SIZE);
}

FLASHMEM bool MD_GR55_class::exchange_patches(uint16_t number) {
  uint16_t pn = patch_number;
  if (bass_mode) {
    number |= GR55_BASS_MODE_NUMBER_OFFSET;
    pn |= GR55_BASS_MODE_NUMBER_OFFSET;
  }
  EEPROM_exchange_device_patch(my_device_number + 1, number, pn, VC_PATCH_SIZE);
  LCD_show_popup_label("Patches swapped", MESSAGE_TIMER_LENGTH);
  return true; // Swap succesfull
}

FLASHMEM bool MD_GR55_class::insert_patch(uint16_t number) {
  if (bass_mode) number |= GR55_BASS_MODE_NUMBER_OFFSET;

  // First clear the last user patch
  uint16_t last_patch = 199;
  if (bass_mode) last_patch |= GR55_BASS_MODE_NUMBER_OFFSET;
  EEPROM_initialize_device_patch(my_device_number + 1, last_patch);

  // Then move up all patches below
  for (uint16_t p = last_patch; p -- > number; ) {
    EEPROM_exchange_device_patch(my_device_number + 1, p, p + 1, VC_PATCH_SIZE);
    DEBUGMSG("swapping " + String(p) + " and " + String(p + 1));
  }

  // Then store the patch
  return store_patch(number);
}

FLASHMEM bool MD_GR55_class::initialize_patch(uint16_t number) {
  if (bass_mode) number += GR55_BASS_MODE_NUMBER_OFFSET;
  EEPROM_initialize_device_patch(my_device_number + 1, number);
  return true;
}

FLASHMEM void MD_GR55_class::initialize_patch_space() {
  memset(GR55_patch_buffer, 0, VC_PATCH_SIZE);
  scene_label_buffer[0] = '-';
  scene_label_buffer[1] = '-';
  for (uint8_t i = 2; i < 8; i++) scene_label_buffer[i] = ' ';
  for (uint8_t s = 1; s <= 8; s++) {
    store_scene_name_to_buffer(s);
  }
  //set_snapscene_number_and_LED(0);
  current_snapscene = 0;
}

// Here we check if a patch that has a scene has been moved or copied
// We store the patch number in GR55_GM_PATCH_NUMBER or GR55_BM_PATCH_NUMBER, which are only used in the other guitar/bass mode.
// If the patch_number stored in the patch is the same as the patch nubmer, then nothing has changed.
// If they are different, the patch has been copied or exchanged.
// We now do a cross check to determine whether the patch has been copied or exchanged.

FLASHMEM void MD_GR55_class::save_pc_after_patch_save() {
  if (!bass_mode) write_sysex(GR55_GM_PATCH_NUMBER, patch_number >> 7, patch_number & 0x7F);
  else write_sysex(GR55_BM_PATCH_NUMBER, patch_number >> 7, patch_number & 0x7F);
}

FLASHMEM void MD_GR55_class::request_pc_after_patch_load() {
  if (millis() > cross_check_timer) {
    if (!bass_mode) request_sysex(GR55_GM_PATCH_NUMBER, 2);
    else request_sysex(GR55_BM_PATCH_NUMBER, 2);
  }
}

FLASHMEM void MD_GR55_class::check_pc_after_patch_load(uint16_t number) {
  if (patch_number != number) { // patch has been swapped or copied
    if (number < GR55_NUMBER_OF_USER_PATCHES) {
      // Check the patch_number in the other patch
      cross_check_number = number;
      uint32_t Address;
      if (!bass_mode) Address = GR55_GM_PATCH_NUMBER & 0xFFFF;
      else Address = GR55_BM_PATCH_NUMBER & 0xFFFF;
      Address += 0x20000000 + ((number / 0x80) * 0x1000000) + ((number % 0x80) * 0x10000); //Calculate the address where the patchname is stored on the GR-55
      request_sysex(Address, 2);
    }
  }
}

FLASHMEM void MD_GR55_class::do_pc_crosscheck (uint16_t number) {
  if ((EEPROM_find_patch_data_index(my_device_number + 1, number) == PATCH_INDEX_NOT_FOUND) || (patch_number == number)) { // patches have been swapped
    exchange_patches(cross_check_number);
    fix_pc_in_patch(cross_check_number);
    fix_pc_in_patch(patch_number);
    cross_check_timer = millis() + CROSS_CHECK_TIME; // The fixed data is not settled yet in the EEPROM of the GR55, so another check will still find the old number and mess it all up.
    load_patch(patch_number);
  }
  else if (number == cross_check_number) { // patches have been copied
    load_patch(number);
    store_patch(patch_number);
    fix_pc_in_patch(patch_number);
    LCD_show_popup_label("Patch copied", MESSAGE_TIMER_LENGTH);
  }
}

FLASHMEM void MD_GR55_class::fix_pc_in_patch(uint16_t number) {
  uint32_t Address;
  if (!bass_mode) Address = GR55_GM_PATCH_NUMBER & 0xFFFF;
  else Address = GR55_BM_PATCH_NUMBER & 0xFFFF;
  Address += 0x20000000 + ((number / 0x80) * 0x1000000) + ((number % 0x80) * 0x10000); //Calculate the address where the patchname is stored on the GR-55
  write_sysex(Address, number >> 7, number & 0x7F);
}

FLASHMEM void MD_GR55_class::fix_pc_in_all_patches() {
  for (uint16_t p = 0; p < GR55_NUMBER_OF_USER_PATCHES; p++) {
    fix_pc_in_patch(p);
    LCD_show_bar(0, map(p, 0, GR55_NUMBER_OF_USER_PATCHES, 0, 127), 0);

  }
}

FLASHMEM void MD_GR55_class::switch_scene_momentary_inst(bool state) {
  switch (scene_momentary_inst_state) {
    case 1: // PCM1
      gr55_pcm1_mute = state;
      break;
    case 2: // PCM2
      gr55_pcm2_mute = state;
      break;
    case 3: // PCM1 + PCM2
      gr55_pcm1_mute = state;
      gr55_pcm2_mute = state;
      break;
    case 4: // COSM GTR
      COSM_gtr_mute = state;
      break;
    case 5: // NORMAL PU
      normal_pu_mute = state;
      break;
    case 6: // PCM2 + COSM
      gr55_pcm2_mute = state;
      COSM_gtr_mute = state;
      break;
    case 7: // COSM + NORMAL PU
      COSM_gtr_mute = state;
      normal_pu_mute = state;
      break;
  }
  unmute();
}
