// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: SY1000 Initialization
// Section 2: SY1000 common MIDI in functions
// Section 3: SY1000 common MIDI out functions
// Section 4: SY1000 program change
// Section 5: SY1000 parameter control
// Section 6: SY1000 assign control
// Section 7: SY1000 expression pedal control

// ********************************* Section 1: SY1000 Initialization ********************************************

// Boss SY-1000 settings:
#define SY1000_MIDI_CHANNEL 1
#define SY1000_PATCH_MIN 0
#define SY1000_PATCH_MAX 399

//Messages are abbreviated to just the address and the data bytes. Checksum is calculated automatically
//Example: {0xF0, 0x41, 0x10, 0x00, 0x00, 0x00, 0x05, 0x12, 0x7F, 0x00, 0x00, 0x01, 0x01, 0x7F, 0xF7} is reduced to 0x7F000001, 0x01

#define SY1000_EDITOR_MODE_ON 0x7F000001, 0x01 //Gets the SY-1000 spitting out lots of sysex data. Should be switched on, otherwise the tuner does not work
#define SY1000_EDITOR_MODE_OFF 0x7F000001, 0x00

#define SY1000_GM_REQUEST_CURRENT_PATCH_NAME 0x10000000 // Address for current patch name (16 bytes)
#define SY1000_GM_REQUEST_CURRENT_PATCH_NUMBER 0x00010000 // Address for current patch number (4 bytes)

#define SY1000_BM_REQUEST_CURRENT_PATCH_NAME 0x10020000 // Address for current Bass patch name (16 bytes)
#define SY1000_BM_REQUEST_CURRENT_PATCH_NUMBER 0x00080000 // Address for current Bass patch number (4 bytes)

#define SY1000_TUNER_ON 0x7F000002, 0x02 // Changes the running mode of the SY-1000 to Tuner - Got these terms from the VG-99 sysex manual.
#define SY1000_TUNER_OFF 0x7F000002, 0x00 //Changes the running mode of the SY1000 to play.
//#define SY1000_SOLO_ON 0x2000500B, 0x01
//#define SY1000_SOLO_OFF 0x2000500B, 0x00

#define SY1000_GM_TEMPO 0x1000123E  // Accepts values from 400 bpm - 2500 bpm
#define SY1000_BM_TEMPO 0x1002123E
#define SY1000_MASTER_KEY 0x10001242 // Sets the master key

#define SY1000_BASS_MODE 0x7F000004

#define SY1000_FOOT_VOL 0x20020803 // The address of the footvolume - values between 0 and 100

#define SY1000_GM_INST1_SW 0x10001500 // The address of the INST1 switch (guitar mode)
#define SY1000_GM_INST2_SW 0x10002000 // The address of the INST2 switch (guitar mode)
#define SY1000_GM_INST3_SW 0x10002B00 // The address of the INST3 switch (guitar mode)
#define SY1000_GM_NORMAL_PU_SW 0x10001201 // The address of the COSM guitar switch (guitar mode)

#define SY1000_BM_INST1_SW 0x10021500 // The address of the INST1 switch (bass mode)
#define SY1000_BM_INST2_SW 0x10021F00 // The address of the INST2 switch (bass mode)
#define SY1000_BM_INST3_SW 0x10022900 // The address of the INST3 switch (bass mode)
#define SY1000_BM_NORMAL_PU_SW 0x10021201 // The address of the COSM guitar switch (bass mode)

// Initialize device variables
// Called at startup of VController
void MD_SY1000_class::init() { // Default values for variables
  MD_base_class::init();

  // Boss SY-1000 variables:
  enabled = DEVICE_DETECT; // Default value
  //enabled = DEVICE_ON; // Default value
  //MIDI_port_manual = 4;
  strcpy(device_name, "SY1000");
  strcpy(full_device_name, "Boss SY-1000");
  patch_min = SY1000_PATCH_MIN;
  patch_max = SY1000_PATCH_MAX;
  max_times_no_response = MAX_TIMES_NO_RESPONSE; // The number of times the SY-1000 does not have to respond before disconnection
  INST1_onoff = 0;
  INST2_onoff = 0;
  INST3_onoff = 0;
  nrml_pu_onoff = 0;
  sysex_delay_length = 5; // minimum delay time between sysex messages (in msec).
  my_LED_colour = 5; // Default value: cyan
  MIDI_channel = SY1000_MIDI_CHANNEL; // Default value
  //bank_number = 0; // Default value
  my_device_page1 = SY1000_DEFAULT_PAGE1; // Default value
  my_device_page2 = SY1000_DEFAULT_PAGE2; // Default value
  my_device_page3 = SY1000_DEFAULT_PAGE3; // Default value
  my_device_page4 = SY1000_DEFAULT_PAGE4; // Default value
  //bass_mode = true;
}

// ********************************* Section 2: SY1000 common MIDI in functions ********************************************

void MD_SY1000_class::check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port) { // Check incoming sysex messages from  Called from MIDI:OnSysEx/OnSerialSysEx

  // Check if it is a message from a SY-1000
  if ((port == MIDI_port) && (sxdata[1] == 0x41) && (sxdata[2] == MIDI_device_id) && (sxdata[3] == 0x00) && (sxdata[4] == 0x00) && (sxdata[5] == 0x00) && (sxdata[6] == 0x69) && (sxdata[7] == 0x12)) {
    uint32_t address = (sxdata[8] << 24) + (sxdata[9] << 16) + (sxdata[10] << 8) + sxdata[11]; // Make the address 32 bit

    // Check checksum
    uint16_t sum = 0;
    for (uint8_t i = 8; i < sxlength - 2; i++) sum += sxdata[i];
    bool checksum_ok = (sxdata[sxlength - 2] == calc_Roland_checksum(sum));

    // Check if it is the current patch number
    if (((address == SY1000_GM_REQUEST_CURRENT_PATCH_NUMBER) || (address == SY1000_BM_REQUEST_CURRENT_PATCH_NUMBER)) && (checksum_ok)) {
      uint16_t new_patch = (sxdata[12] << 12) + (sxdata[13] << 8) + (sxdata[14] << 4) + sxdata[15];
      if (patch_number != new_patch) { // Right after a patch change the patch number is sent again. So here we catch that message.
        patch_number = new_patch;
        prev_patch_number = patch_number;
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
          for (uint8_t count = 0; count < 16; count++) {
            SP[last_requested_sysex_switch].Label[count] = static_cast<char>(sxdata[count + 12]); //Add ascii character to the SP.Label String
          }

          if (SP[last_requested_sysex_switch].PP_number == patch_number) {
            current_patch_name = SP[last_requested_sysex_switch].Label; // Load patchname when it is read
            update_main_lcd = true; // And show it on the main LCD
          }
          DEBUGMSG(SP[last_requested_sysex_switch].Label);
          PAGE_request_next_switch();
          return;

        case REQUEST_PARAMETER_TYPE:
          read_parameter(last_requested_sysex_switch, sxdata[12], sxdata[13]);
          PAGE_request_next_switch();
          return;

        case REQUEST_ASSIGN_TYPE:
          read_current_assign(last_requested_sysex_switch, address, sxdata, sxlength);
          return;
      }
    }

    // Check if it is the current patch name
    if (((address == SY1000_GM_REQUEST_CURRENT_PATCH_NAME) || (address == SY1000_BM_REQUEST_CURRENT_PATCH_NAME)) && (checksum_ok)) {
      current_patch_name = "";
      for (uint8_t count = 12; count < 28; count++) {
        current_patch_name += static_cast<char>(sxdata[count]); //Add ascii character to Patch Name String
      }
      update_main_lcd = true;
      if (popup_patch_name) {
        LCD_show_popup_label(current_patch_name, ACTION_TIMER_LENGTH);
        popup_patch_name = false;
      }
    }

    // Check if it is the exp sw, exp and exp_on state
    if (((address == 0x1000002A) || (address == 0x1002002A)) && (checksum_ok)) {
      exp1_type = sxdata[12]; // EXP1 type
      exp2_type = sxdata[13]; // EXP2 type
      update_exp_label(last_requested_sysex_switch);
      PAGE_request_next_switch();
    }

    // Check for bass mode
    if ((address == SY1000_BASS_MODE) && (checksum_ok)) {
      if (sxdata[12] == 0x01) {
        bass_mode = true;
        DEBUGMAIN("BASS MODE DETECTED");
      }
    }

    // Check if it is the guitar on/off states
    if (checksum_ok) check_guitar_switch_states(sxdata, sxlength);

    // Check if it is some other stompbox function and copy the status to the right LED
    //SY1000_check_stompbox_states(sxdata, sxlength);
  }

}

void MD_SY1000_class::check_PC_in(uint8_t program, uint8_t channel, uint8_t port) {  // Check incoming PC messages from  Called from MIDI:OnProgramChange

  // Check the source by checking the channel
  if ((port == MIDI_port) && (channel == MIDI_channel)) { // SY1000 sends a program change
    uint16_t new_patch = (CC00 * 100) + program;
    if (patch_number != new_patch) {
      prev_patch_number = patch_number;
      patch_number = new_patch;
      request_current_patch_name(); // So the main display always show the correct patch
      //page_check();
      do_after_patch_selection();
      update_page = REFRESH_PAGE;
    }
  }
}


// Detection of SY-1000

void MD_SY1000_class::identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port) {
  // Check if it is a SY-1000
  if ((sxdata[5] == 0x41) && (sxdata[6] == 0x69) && (sxdata[7] == 0x03)) {
    no_response_counter = 0;
    if (connected == false) connect(sxdata[2], port); //Byte 2 contains the correct device ID
  }
}

void MD_SY1000_class::do_after_connect() {
  request_sysex(SY1000_BASS_MODE, 1);
  request_current_patch_number();
  request_current_patch_name(); // So the main display always show the correct patch
  //write_sysex(SY1000_EDITOR_MODE_ON); // Put the SY1000 in EDITOR mode - otherwise tuner will not work
  current_exp_pedal = 1;
  do_after_patch_selection();
  update_page = REFRESH_PAGE;
}


// ********************************* Section 3: SY1000 common MIDI out functions ********************************************

void MD_SY1000_class::write_sysex(uint32_t address, uint8_t value) { // For sending one data byte

  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t checksum = calc_Roland_checksum(ad[3] + ad[2] + ad[1] + ad[0] + value); // Calculate the Roland checksum
  uint8_t sysexmessage[15] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x00, 0x69, 0x12, ad[3], ad[2], ad[1], ad[0], value, checksum, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 15, MIDI_port, 1); // SY-1000 connected via USBHost_t36 library will only supoort sysex messages via cable 1 (default 0)
}

void MD_SY1000_class::write_sysex(uint32_t address, uint8_t value1, uint8_t value2) { // For sending two data bytes

  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t checksum = calc_Roland_checksum(ad[3] + ad[2] + ad[1] + ad[0] + value1 + value2); // Calculate the Roland checksum
  uint8_t sysexmessage[16] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x00, 0x69, 0x12, ad[3], ad[2], ad[1], ad[0], value1, value2, checksum, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 16, MIDI_port, 1);
}

void MD_SY1000_class::write_sysex(uint32_t address, uint8_t value1, uint8_t value2, uint8_t value3, uint8_t value4) { // For sending two data bytes

  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t checksum = calc_Roland_checksum(ad[3] + ad[2] + ad[1] + ad[0] + value1 + value2 + value3 + value4); // Calculate the Roland checksum
  uint8_t sysexmessage[18] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x00, 0x69, 0x12, ad[3], ad[2], ad[1], ad[0], value1, value2, value3, value4, checksum, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 18, MIDI_port, 1);
}

void MD_SY1000_class::request_sysex(uint32_t address, uint8_t no_of_bytes) {
  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t no1 = no_of_bytes >> 7;
  uint8_t no2 = no_of_bytes & 0x7F;
  uint8_t checksum = calc_Roland_checksum(ad[3] + ad[2] + ad[1] + ad[0] +  no1 + no2); // Calculate the Roland checksum
  uint8_t sysexmessage[18] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x00, 0x69, 0x11, ad[3], ad[2], ad[1], ad[0], 0x00, 0x00, no1, no2, checksum, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 18, MIDI_port, 1);
}

void MD_SY1000_class::set_bpm() {
  if (connected) {
    uint16_t t = Setting.Bpm * 10;
    if (!bass_mode) write_sysex(SY1000_GM_TEMPO, (t & 0xF000) >> 12, (t & 0x0F00) >> 8, (t & 0x00F0) >> 4, (t & 0x000F)); // Tempo is modulus 16
    else write_sysex(SY1000_BM_TEMPO, (t & 0xF000) >> 12, (t & 0x0F00) >> 8, (t & 0x00F0) >> 4, (t & 0x000F));
  }
}

void MD_SY1000_class::start_tuner() {
  if (connected) {
    write_sysex(SY1000_EDITOR_MODE_ON); // Put the SY1000 in EDITOR mode - otherwise tuner will not work
    write_sysex(SY1000_TUNER_ON); // Start tuner on SY-1000
    tuner_active = true;
  }
}

void MD_SY1000_class::stop_tuner() {
  if (connected) {
    write_sysex(SY1000_TUNER_OFF); // Stop tuner on SY-1000
    write_sysex(SY1000_EDITOR_MODE_OFF); // Switch off EDITOR mode - so the SY1000 will respond faster
    tuner_active = false;
  }
}

// ********************************* Section 4: SY1000 program change ********************************************

void MD_SY1000_class::select_patch(uint16_t new_patch) {
  //if (new_patch == patch_number) unmute();
  prev_patch_number = patch_number;
  patch_number = new_patch;

  MIDI_send_CC(0, new_patch / 100, MIDI_channel, MIDI_port);
  MIDI_send_PC(new_patch % 100, MIDI_channel, MIDI_port);
  DEBUGMSG("out(SY1000) PC" + String(new_patch)); //Debug
  do_after_patch_selection();
}

void MD_SY1000_class::do_after_patch_selection() {
  request_onoff = false;
  is_on = connected;
  if (Setting.Send_global_tempo_after_patch_change == true) {
    delay(5); // SY1000 misses send bpm command...
    set_bpm();
  }
  Current_patch_number = patch_number;
  update_LEDS = true;
  update_main_lcd = true;
  request_guitar_switch_states();
  request_current_patch_name();
  //EEPROM.write(EEPROM_SY1000_PATCH_NUMBER, patch_number);
  MD_base_class::do_after_patch_selection();
}

bool MD_SY1000_class::request_patch_name(uint8_t sw, uint16_t number) {
  if (number > patch_max) return true;
  uint32_t Address = 0x52000000 + (((number * 0x10) / 0x80) * 0x100) + ((number * 0x10) % 0x80); //Calculate the address where the patchname is stored on the SY-1000
  if (bass_mode) Address += 0x01000000; // Start address is 0x53000000 in bass mode
  last_requested_sysex_address = Address;
  last_requested_sysex_type = REQUEST_PATCH_NAME;
  last_requested_sysex_switch = sw;
  request_sysex(Address, 16); //Request the 16 bytes of the SY1000 patchname
  return false;
}

void MD_SY1000_class::request_current_patch_name() {
  if (!bass_mode) request_sysex(SY1000_GM_REQUEST_CURRENT_PATCH_NAME, 16);
  else request_sysex(SY1000_BM_REQUEST_CURRENT_PATCH_NAME, 16);
}

void MD_SY1000_class::request_current_patch_number() {
  if (!bass_mode) request_sysex(SY1000_GM_REQUEST_CURRENT_PATCH_NUMBER, 4);
  else request_sysex(SY1000_BM_REQUEST_CURRENT_PATCH_NUMBER, 4);
}

bool MD_SY1000_class::flash_LEDs_for_patch_bank_switch(uint8_t sw) { // Will flash the LEDs in banks of three when coming from direct select mode.
  if (!bank_selection_active()) return false;

  if (flash_bank_of_four == 255) return true; // We are not coming from direct select, so all LEDs should flash

  bool in_right_bank_of_eight = (flash_bank_of_four / 2 == SP[sw].PP_number / 8); // Going bank up and down coming from direct select will make all LEDs flash in other banks
  if (!in_right_bank_of_eight) return true;

  bool in_right_bank_of_four = (flash_bank_of_four == SP[sw].PP_number / 4); // Only flash the three LEDs of the corresponding bank
  if (in_right_bank_of_four) return true;
  return false;
}

void MD_SY1000_class::number_format(uint16_t number, String &Output) {
  if (number < 200) Output += 'U';
  else Output += 'P';
  uint8_t bank_no = ((number % 200) / 4) + 1;
  Output += String(bank_no / 10) + String(bank_no % 10);
  Output += "-" + String((number % 4) + 1);
}

void MD_SY1000_class::direct_select_format(uint16_t number, String &Output) {

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

bool MD_SY1000_class::valid_direct_select_switch(uint8_t number) {
  bool result = false;
  if (direct_select_state == 0) { // Show all switches on first digit
    result = ((number * 40) <= (patch_max - patch_min + 1));
  }
  else {
    if ((bank_select_number == 0) && (number == 0)) return false;
    if ((bank_select_number * 40) + (number * 4) <= (patch_max - patch_min + 1)) result = true;
  }
  return result;
}

void MD_SY1000_class::direct_select_start() {
  Previous_bank_size = bank_size; // Remember the bank size
  device_in_bank_selection = my_device_number + 1;
  bank_size = 400;
  bank_select_number = 0; // Reset bank number
  direct_select_state = 0;
}

uint16_t MD_SY1000_class::direct_select_patch_number_to_request(uint8_t number) {
  uint16_t new_patch_number;
  if (direct_select_state == 0) new_patch_number = (number * 40);
  else new_patch_number = (bank_select_number * 40) + (number * 4);
  if (new_patch_number < 4) new_patch_number = 4;
  return new_patch_number - 4;
}

void MD_SY1000_class::direct_select_press(uint8_t number) {
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
    SCO_select_page(SY1000_DEFAULT_PAGE1); // Which should give PAGE_SY1000_PATCH_BANK
    device_in_bank_selection = my_device_number + 1; // Go into bank mode
  }
}

// ** US-20 simulation
// Selecting and muting the SY1000 is done by storing the settings of COSM guitar switch and Normal PU switch
// and switching both off when guitar is muted and back to original state when the SY1000 is selected

void MD_SY1000_class::request_guitar_switch_states() {
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

void MD_SY1000_class::check_guitar_switch_states(const unsigned char* sxdata, short unsigned int sxlength) {
  if (request_onoff == true) {
    uint32_t address = (sxdata[8] << 24) + (sxdata[9] << 16) + (sxdata[10] << 8) + sxdata[11]; // Make the address 32 bit
    if ((address == SY1000_GM_INST1_SW) || (address == SY1000_BM_INST1_SW)) {
      INST1_onoff = sxdata[12];
      INST1_type = sxdata[13];
    }
    if ((address == SY1000_GM_INST2_SW) || (address == SY1000_BM_INST2_SW)) {
      INST2_onoff = sxdata[12];
      INST2_type = sxdata[13];
    }
    if ((address == SY1000_GM_INST3_SW) || (address == SY1000_BM_INST3_SW)) {
      INST3_onoff = sxdata[12];
      INST3_type = sxdata[13];
    }
    if ((address == SY1000_GM_NORMAL_PU_SW) || (address == SY1000_BM_NORMAL_PU_SW)) {
      nrml_pu_onoff = sxdata[12];  // Store the value
      request_onoff = false;
    }
  }
}

void MD_SY1000_class::unmute() {
  is_on = connected;
  if (!bass_mode) {
    write_sysex(SY1000_GM_INST1_SW, INST1_onoff); // Switch INST1 guitar on
    write_sysex(SY1000_GM_INST2_SW, INST2_onoff); // Switch INST2 guitar on
    write_sysex(SY1000_GM_INST3_SW, INST3_onoff); // Switch INST3 guitar on
    write_sysex(SY1000_GM_NORMAL_PU_SW, nrml_pu_onoff); // Switch normal pu on
  }
  else {
    write_sysex(SY1000_BM_INST1_SW, INST1_onoff); // Switch INST1 guitar on
    write_sysex(SY1000_BM_INST2_SW, INST2_onoff); // Switch INST2 guitar on
    write_sysex(SY1000_BM_INST3_SW, INST3_onoff); // Switch INST3 guitar on
    write_sysex(SY1000_BM_NORMAL_PU_SW, nrml_pu_onoff); // Switch normal pu on
  }
}

void MD_SY1000_class::mute() {
  if ((US20_mode_enabled()) && (!is_always_on) && (is_on)) {
    is_on = false;
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
}

// ********************************* Section 5: SY1000 parameter control ********************************************

// Procedures for the SY1000_PARAMETER:
// 1. Load in SP array L load_page()
// 2. Request parameter state - in PAGE_request_current_switch()
// 3. Read parameter state - SY1000_read_parameter() below
// 4. Press switch - SY1000_parameter_press() below - also calls check_update_label()
// 5. Release switch - SY1000_parameter_release() below - also calls check_update_label()

struct SY1000_parameter_struct { // Combines all the data we need for controlling a parameter in a device
  uint16_t Target_GM; // Target of the assign as given in the assignments of the SY1000
  uint16_t Target_BM; // Target of the assign as given in the assignments of the SY1000
  uint32_t Address_GM; // The address of the parameter in guitar mode
  uint32_t Address_BM; // The address of the parameter in bass mode
  uint8_t NumVals; // The number of values for this parameter
  char Name[17]; // The name for the label
  uint16_t Sublist; // Which sublist to read for the FX or amp type - 0 if second byte does not contain the type or if there is no sublist +100 Show value from sublist.
  uint8_t Colour; // The colour for this effect.
};


#define SY1000_FX_COLOUR 255 // Just a colour number to pick the colour from the SY1000_FX_colours table
#define SY1000_FX_TYPE_COLOUR 254 //Another number for the FX type
#define SUBLIST_FROM_BYTE2 32768 //Bit 16 set high means read sublist from byte 2
#define SHOW_NUMBER 31767 // Special number for showing the number (used for gain/volume/etc) - set in sublist
#define SHOW_DOUBLE_NUMBER 31766 // Show number times 2 (used for patch volume/etc) - set in sublist
#define SHOW_PAN 31765 // Special number for showing the pan- set in sublist
#define SY1000_INST_SUBLIST 31750

const PROGMEM SY1000_parameter_struct SY1000_parameters[] = {
  {  11,   11, 0x10001500, 0x10021500, 2, "INST1", SY1000_INST_SUBLIST | SUBLIST_FROM_BYTE2, FX_GTR_TYPE}, // 00
  { 339,  339, 0x10002000, 0x10021F00, 2, "INST2", SY1000_INST_SUBLIST | SUBLIST_FROM_BYTE2, FX_GTR_TYPE},
  { 669,  669, 0x10002B00, 0x10022900, 2, "INST3", SY1000_INST_SUBLIST | SUBLIST_FROM_BYTE2, FX_GTR_TYPE},
  {1014,  657, 0x10001201, 0x10021201, 2, "NORMAL PU", 0, FX_GTR_TYPE},
  {1967, 1910, 0x10003600, 0x10023300, 2, "COMP", 16 | SUBLIST_FROM_BYTE2, FX_GTR_TYPE},
  {1848, 1791, 0x10003700, 0x10023400, 2, "DIST", 23 | SUBLIST_FROM_BYTE2, FX_GTR_TYPE},
  {1828, 1771, 0x10003800, 0x10023500, 2, "AMP", 58 | SUBLIST_FROM_BYTE2, FX_AMP_TYPE},
  {1840, 1783, 0x1000380C, 0x1002350C, 2, "AMP SOLO SW", 0, FX_AMP_TYPE},
  { 316,  297, 0x10001A16, 0x10021A22, 2, "AMP1 SOLO SW", 0, FX_AMP_TYPE},
  { 646,  608, 0x10002516, 0x10022422, 2, "AMP2 SOLO SW", 0, FX_AMP_TYPE},
  { 976,  919, 0x10003016, 0x10022E22, 2, "AMP3 SOLO SW", 0, FX_AMP_TYPE},
  {1930, 1873, 0x10003900, 0x10023600, 2, "NS", 0, FX_GTR_TYPE},
  {1857, 1800, 0x10003A00, 0x10023700, 2, "EQ1", 0, FX_FILTER_TYPE},
  {1869, 1812, 0x10003B00, 0x10023800, 2, "EQ2", 0, FX_FILTER_TYPE},
  {1905, 1848, 0x10003C00, 0x10023900, 2, "DELAY 1", 0, FX_DELAY_TYPE},
  {1912, 1855, 0x10003D00, 0x10023A00, 2, "DELAY 2", 0, FX_DELAY_TYPE},
  {1881, 1824, 0x10003E00, 0x10023B00, 2, "MST DLY", 89 | SUBLIST_FROM_BYTE2, FX_DELAY_TYPE},
  {1942, 1885, 0x10003F00, 0x10023C00, 2, "CHORUS", 101 | SUBLIST_FROM_BYTE2, FX_MODULATE_TYPE},
  {1018,  961, 0x10004000, 0x10023D00, 2, "FX1", 105 | SUBLIST_FROM_BYTE2, SY1000_FX_COLOUR},
  {1288, 1231, 0x10006600, 0x10026300, 2, "FX2", 105 | SUBLIST_FROM_BYTE2, SY1000_FX_COLOUR},
  {1558, 1501, 0x10010C00, 0x10030900, 2, "FX3", 105 | SUBLIST_FROM_BYTE2, SY1000_FX_COLOUR},
  {1919, 1862, 0x10013200, 0x10032F00, 2, "RVB", 142 | SUBLIST_FROM_BYTE2, FX_REVERB_TYPE},
  {0xFFFF, 0xFFFF, 0x1000002A, 0x1002002A, 46, "EXP1 TP", 149, FX_DEFAULT_TYPE},
  {0xFFFF, 0xFFFF, 0x1000002B, 0x1002002B, 46, "EXP2 TP", 149, FX_DEFAULT_TYPE},
};

#define SY1000_EXP1 22
#define SY1000_EXP2 23

const uint16_t SY1000_NUMBER_OF_PARAMETERS = sizeof(SY1000_parameters) / sizeof(SY1000_parameters[0]);

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

  // Sublist 149 - 203: EXP pedal types
  "OFF", "FV1", "FV2", "FV1+TUNR", "FV2+TUNR", "FX1 P.PS", "FX2 P.PS", "FX3 P.PS", "P.LVL100", "P.LVL200",
  "INST1 LV", "INST2 LV", "INST3 LV", "INST ALL", "I1 CUTOF", "I2 CUTOF", "I3 CUTOF", "ALL CUTF", "I1 RESO", "I2 RESO",
  "I3 RESO", "ALL RESO", "1:GTR VL", "2:GTR VL", "3:GTR VL", "ALL:G VL", "1:NOR100", "1:NOR200", "2:NOR100", "2:NOR200",
  "3:NOR100", "3:NOR200", "AL:NM100", "AL:NM200", "1:STR B", "2:STR B", "3:STR B", "AL:STR B", "1:DYNA B", "2:DYNA B",
  "3:DYNA B", "A:DYNA B", "MIXR A/B", "BAL1 A/B", "BAL2 A/B", "BAL3 A/B",
};

const uint16_t SY1000_SIZE_OF_SUBLIST = sizeof(SY1000_sublists) / sizeof(SY1000_sublists[0]);

#define SY1000_GTR_INST_TYPES_SUBLIST 1
#define SY1000_BASS_INST_TYPES_SUBLIST 9

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

void MD_SY1000_class::read_parameter_name(uint16_t number, String &Output) { // Called from menu
  if (number < number_of_parameters())  Output = SY1000_parameters[number].Name;
  else Output = "?";
}

void MD_SY1000_class::read_parameter_value_name(uint16_t number, uint16_t value, String &Output) {
  if (number < number_of_parameters())  {
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

// Toggle SY1000 stompbox parameter
void MD_SY1000_class::parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {

  // Send sysex MIDI command to SY-1000
  uint8_t value = SCO_return_parameter_value(Sw, cmd);
  if ((SP[Sw].Latch != TGL_OFF) && (number < SY1000_NUMBER_OF_PARAMETERS)) {
    if (!bass_mode) write_sysex(SY1000_parameters[number].Address_GM, value);
    else write_sysex(SY1000_parameters[number].Address_BM, value);
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

void MD_SY1000_class::parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {
  // Work out state of pedal
  if (SP[Sw].Latch == MOMENTARY) {
    SP[Sw].State = 2; // Switch state off
    if (number < SY1000_NUMBER_OF_PARAMETERS) {
      if (!bass_mode) write_sysex(SY1000_parameters[number].Address_GM, cmd->Value2);
      else write_sysex(SY1000_parameters[number].Address_BM, cmd->Value2);
      SP[Sw].Offline_value = cmd->Value2;
    }

    //PAGE_load_current(false); // To update the other switch states, we re-load the current page
    update_page = REFRESH_FX_ONLY;
  }
}

void MD_SY1000_class::read_parameter_title(uint16_t number, String &Output) {
  Output += SY1000_parameters[number].Name;
}

bool MD_SY1000_class::request_parameter(uint8_t sw, uint16_t number) {
  if (can_request_sysex_data()) {
    uint32_t my_address;
    if (!bass_mode) my_address = SY1000_parameters[number].Address_GM;
    else my_address = SY1000_parameters[number].Address_BM;
    last_requested_sysex_address = my_address;
    last_requested_sysex_type = REQUEST_PARAMETER_TYPE;
    last_requested_sysex_switch = sw;
    request_sysex(my_address, 2); // Request the parameter state data
    return false; // Move to next switch is false. We need to read the parameter first
  }
  else {
    if ((sw < TOTAL_NUMBER_OF_SWITCHES) && (SP[sw].Type == PAR_BANK)) read_parameter(sw, SP[sw].Offline_value, SP[sw + 1].Offline_value);
    else read_parameter(sw, SP[sw].Offline_value, 0);
    return true;
  }
}

void MD_SY1000_class::read_parameter(uint8_t sw, uint8_t byte1, uint8_t byte2) { //Read the current SY1000 parameter
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
    msg += '(' + type_name + ')';
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

void MD_SY1000_class::check_update_label(uint8_t Sw, uint8_t value) { // Updates the label for extended sublists
  uint16_t index = SP[Sw].PP_number; // Read the parameter number (index to SY1000-parameter array)
  if (index != NOT_FOUND) {
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

uint16_t MD_SY1000_class::number_of_parameters() {
  return SY1000_NUMBER_OF_PARAMETERS;
}

uint8_t MD_SY1000_class::number_of_values(uint16_t parameter) {
  if (parameter < SY1000_NUMBER_OF_PARAMETERS) return SY1000_parameters[parameter].NumVals;
  else return 0;
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

#define SY1000_NUMBER_OF_ASSIGNS 16

uint32_t MD_SY1000_class::calculate_assign_address(uint8_t number) {
  if (number >= SY1000_NUMBER_OF_ASSIGNS) return 0;
  if (!bass_mode) return 0x10000400 + ((number / 2) * 0x100) + ((number % 2) * 0x40);
  else return 0x10020400 + ((number / 2) * 0x100) + ((number % 2) * 0x40);
}

void MD_SY1000_class::read_assign_name(uint8_t number, String & Output) {
  //if (number < SY1000_NUMBER_OF_ASSIGNS)  Output += SY1000_assign_title[number];
  if (number < SY1000_NUMBER_OF_ASSIGNS)  Output += "ASSIGN " + String(number + 1);
  else Output += "?";
}

void MD_SY1000_class::read_assign_short_name(uint8_t number, String & Output) {
  //if (number < SY1000_NUMBER_OF_ASSIGNS)  Output += SY1000_assign_title[number];
  if (number < SY1000_NUMBER_OF_ASSIGNS)  Output += "ASG" + String(number + 1);
  else Output += "?";
}

void MD_SY1000_class::read_assign_trigger(uint8_t number, String & Output) {
  if ((number > 0) && (number < 128)) Output = "CC#" + String(number);
  else Output = "-";
}

uint8_t MD_SY1000_class::get_number_of_assigns() {
  return SY1000_NUMBER_OF_ASSIGNS;
}

uint8_t MD_SY1000_class::trigger_follow_assign(uint8_t number) {
  return number + 21; // Default cc numbers are 21 and up
}

void MD_SY1000_class::assign_press(uint8_t Sw, uint8_t value) { // Switch set to SY1000_ASSIGN is pressed
  // Send cc MIDI command to SY1000
  uint8_t cc_number = SP[Sw].Trigger;
  MIDI_send_CC(cc_number, value, MIDI_channel, MIDI_port);

  // Display the patch function
  if (SP[Sw].Assign_on) {
    uint8_t value = 0;
    if (SP[Sw].State == 1) value = SP[Sw].Assign_max;
    else value = SP[Sw].Assign_min;
    check_update_label(Sw, value);
  }
  LCD_show_popup_label(SP[Sw].Label, ACTION_TIMER_LENGTH);

  if (SP[Sw].Assign_on) update_page = REFRESH_PAGE; // To update the other switch states, we re-load the current page
}

void MD_SY1000_class::assign_release(uint8_t Sw) { // Switch set to SY1000_ASSIGN is released
  // Send cc MIDI command to SY1000
  uint8_t cc_number = SP[Sw].Trigger;
  MIDI_send_CC(cc_number, 0, MIDI_channel, MIDI_port);

  // Update status
  if (SP[Sw].Latch == MOMENTARY) {
    if (SP[Sw].Assign_on) {
      SP[Sw].State = 2; // Switch state off
      check_update_label(Sw, SP[Sw].Assign_min);
      LCD_show_popup_label(SP[Sw].Label, ACTION_TIMER_LENGTH);
    }
    else SP[Sw].State = 0; // Assign off, so LED should be off as well

    if (SP[Sw].Assign_on) update_page = REFRESH_PAGE; // To update the other switch states, we re-load the current page
  }
}

void MD_SY1000_class::assign_load(uint8_t sw, uint8_t assign_number, uint8_t cc_number) { // Switch set to SY1000_ASSIGN is loaded in SP array
  SP[sw].Trigger = cc_number; //Save the cc_number in the Trigger variable
  SP[sw].Assign_number = assign_number;
}

void MD_SY1000_class::request_current_assign(uint8_t sw) {
  uint8_t index = SP[sw].Assign_number;  //index should be between 0 and 7
  //SP[sw].Address = SY1000_assign_address[index];

  if (index < SY1000_NUMBER_OF_ASSIGNS) {
    DEBUGMSG("Request assign " + String(index + 1));
    //read_assign_target = false;
    uint32_t my_address = calculate_assign_address(index);
    last_requested_sysex_address = my_address;
    last_requested_sysex_type = REQUEST_ASSIGN_TYPE;
    last_requested_sysex_switch = sw;
    request_sysex(my_address, 15);  //Request 15 bytes for the SY1000 assign
  }
  else PAGE_request_next_switch(); // Wrong assign number given in Config - skip it
}

void MD_SY1000_class::read_current_assign(uint8_t sw, uint32_t address, const unsigned char* sxdata, short unsigned int sxlength) {
  bool found;
  String msg;
  uint8_t assign_switch = sxdata[12];
  uint16_t assign_target = (sxdata[13] << 12) + (sxdata[14] << 8) + (sxdata[15] << 4) + sxdata[16];
  uint16_t assign_target_min = (sxdata[18] << 8) + (sxdata[19] << 4) + sxdata[20]; // Byte 19 is not read as it adds 0x8000 to the assign min value
  uint16_t assign_target_max = (sxdata[22] << 8) + (sxdata[23] << 4) + sxdata[24]; // Same for byte 21
  uint8_t assign_source = sxdata[25];
  uint8_t assign_latch = sxdata[26];

  uint8_t my_trigger = SP[sw].Trigger;
  if ((my_trigger >= 1) && (my_trigger <= 31)) my_trigger = my_trigger + 26; // Trigger is cc01 - cc31 (source for CC#01 is 27)
  if ((my_trigger >= 64) && (my_trigger <= 95)) my_trigger = my_trigger - 6; // Trigger is cc64 - cc95 (source for CC#64 is 58)

  bool assign_on = ((assign_switch == 0x01) && (my_trigger == assign_source)); // Check if assign is on by checking assign switch and source

  DEBUGMSG("SY1000 Assign_switch: 0x" + String(assign_switch, HEX));
  DEBUGMSG("SY1000 Assign_target 0x:" + String(assign_target, HEX));
  DEBUGMSG("SY1000 Assign_min: 0x" + String(assign_target_min, HEX));
  DEBUGMSG("SY1000 Assign_max: 0x" + String(assign_target_max, HEX));
  DEBUGMSG("SY1000 Assign_source: 0x" + String(assign_source, HEX));
  DEBUGMSG("SY1000 Assign_latch: 0x" + String(assign_latch, HEX));
  DEBUGMSG("SY1000 Assign_trigger-check:" + String(my_trigger) + "==" + String(assign_source));

  if (assign_on) {
    SP[sw].Assign_on = true; // Switch the pedal off
    SP[sw].Latch = assign_latch;

    // Allow for SY1000 assign min and max swap. Is neccesary, because the SY1000 will not save a parameter in the on-state, unless you swap the assign min and max values
    if (assign_target_max > assign_target_min) {
      // Store values the normal way around
      SP[sw].Assign_max = assign_target_max;
      SP[sw].Assign_min = assign_target_min;
    }
    else {
      // Reverse the values
      SP[sw].Assign_max = assign_target_min;
      SP[sw].Assign_min = assign_target_max;
    }

    // Request the target
    found = target_lookup(sw, assign_target); // Lookup the address of the target in the SY1000_Parameters array
    DEBUGMSG("Request target of assign" + String(SP[sw].Assign_number + 1) + ':' + String(SP[sw].Address, HEX));
    if (found) {
      uint32_t my_address = SP[sw].Address;
      last_requested_sysex_address = my_address;
      last_requested_sysex_type = REQUEST_PARAMETER_TYPE;
      //read_assign_target = true;
      request_sysex(my_address, 2);
    }
    else {
      SP[sw].PP_number = NOT_FOUND;
      SP[sw].Colour = FX_DEFAULT_TYPE;
      // Set the Label
      msg = "CC#" + String(SP[sw].Trigger) + " (ASGN" + String(SP[sw].Assign_number + 1) + ')';
      LCD_set_SP_label(sw, msg);
      PAGE_request_next_switch();
    }
  }
  else { // Assign is off
    SP[sw].Assign_on = false; // Switch the pedal off
    SP[sw].State = 0; // Switch the stompbox off
    SP[sw].Latch = MOMENTARY; // Make it momentary
    SP[sw].Colour = FX_DEFAULT_TYPE; // Set the on colour to default
    // Set the Label
    msg = "CC#" + String(SP[sw].Trigger);
    LCD_set_SP_label(sw, msg);
    PAGE_request_next_switch();
  }
}

bool MD_SY1000_class::target_lookup(uint8_t sw, uint16_t target) { // Finds the target and its address in the SY1000_parameters table

  // Lookup in SY1000_parameter array
  bool found = false;
  if (!bass_mode) {
    for (uint16_t i = 0; i < SY1000_NUMBER_OF_PARAMETERS; i++) {
      if (target == SY1000_parameters[i].Target_GM) { //Check is we've found the right target
        SP[sw].PP_number = i; // Save the index number
        SP[sw].Address = SY1000_parameters[i].Address_GM;
        found = true;
      }
    }
  }
  else {
    for (uint16_t i = 0; i < SY1000_NUMBER_OF_PARAMETERS; i++) {
      if (target == SY1000_parameters[i].Target_BM) { //Check is we've found the right target
        SP[sw].PP_number = i; // Save the index number
        SP[sw].Address = SY1000_parameters[i].Address_BM;
        found = true;
      }
    }
  }
  return found;
}

uint32_t MD_SY1000_class::check_address_for_bass_mode(uint32_t address) {
  if (!bass_mode) return address;

  if (address < 0x10001D00) return address + 0x00020000;
  if (address < 0x10002800) return address + 0x00020000 - 100;
  if (address < 0x10003300) return address + 0x00020000 - 200;
  else return address + 0x00020000 - 300;
}

// ********************************* Section 7: SY1000 Expression pedal control ********************************************

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
  uint32_t Address_GM; // The address of the parameter in guitar mode
  uint32_t Address_BM; // The address of the parameter in bass mode
  uint8_t Type;
};

SY1000_exp_pedal_target_struct SY1000_exp_pedal_target[] = {
  { 0, 0, 0 }, // OFF
  { 0x1000120E, 0x1002120E, RANGE1000 }, // FV1
  { 0x1000121B, 0x1002121B, RANGE1000 }, // FV2
  { 0x1000120E, 0x1002120E, RANGE1000T }, // FV1 + TUNER
  { 0x1000121B, 0x1002121B, RANGE1000T }, // FV2 + TUNER
  { 0x10004000, 0x10023D00, RANGE_FX }, // FX1 PEDAL POS - Base address of FX1
  { 0x10006600, 0x10026300, RANGE_FX }, // FX2 PEDAL POS - Base address of FX2
  { 0x10010C00, 0x10030900, RANGE_FX }, // FX3 PEDAL POS - Base address of FX3
  { 0x1000123C, 0x1002123C, RANGE100 }, // PATCH LEVEL 100
  { 0x1000123C, 0x1002123C, RANGE200 }, // PATCH LEVEL200
  { 0x10001502, 0x10021502, RANGE_DIRECT }, // INST1 LEVEL
  { 0x10002002, 0x10021F02, RANGE_DIRECT }, // INST2 LEVEL
  { 0x10002B02, 0x10022902, RANGE_DIRECT }, // INST3 LEVEL
  { 0, 0, LAST_THREE }, // INST ALL LEVEL
  { 0x1001500, 0x10021500, RANGE_CUTOFF }, // INST1 CUTOFF - Base address of instr1
  { 0x1002000, 0x10021900, RANGE_CUTOFF }, // INST2 CUTOFF - Base address of instr2
  { 0x1002B00, 0x10022900, RANGE_CUTOFF }, // INST3 CUTOFF - Base address of instr3
  { 0, 0, LAST_THREE }, // INST ALL CUTOFF
  { 0x1001500, 0x10021500, RANGE_RESONANCE }, // INST1 CUTOFF - Base address of instr1 + 1
  { 0x1002000, 0x10021900, RANGE_RESONANCE }, // INST2 CUTOFF - Base address of instr2 + 1
  { 0x1002B00, 0x10022900, RANGE_RESONANCE }, // INST3 CUTOFF - Base address of instr3 + 1
  { 0, 0, LAST_THREE }, // INST ALL RESO
  { 0x10001A08, 0x10021A01, RANGE_DIRECT }, // 1:GTR VOL
  { 0x10002508, 0x10022401, RANGE_DIRECT }, // 2:GTR VOL
  { 0x10003008, 0x10022E01, RANGE_DIRECT }, // 3:GTR VOL
  { 0, 0, LAST_THREE }, // ALL:GTR VOL
  { 0x10001504, 0x10021504, RANGE100 }, // 1:NOR MIX 100
  { 0x10001504, 0x10021504, RANGE200 }, // 1:NOR MIX 200
  { 0x10002004, 0x10021F04, RANGE100 }, // 2:NOR MIX 100
  { 0x10002004, 0x10021F04, RANGE200 }, // 2:NOR MIX 200
  { 0x10002B04, 0x10022904, RANGE100 }, // 3:NOR MIX 100
  { 0x10002B04, 0x10022904, RANGE200 }, // 3:NOR MIX 200
  { 0, 0, LAST_THREEx2 }, // ALL:NOR MIX100
  { 0, 0, LAST_THREEx2 }, // ALL:NOR MIX200
  { 0x10001F2F, 0x10021E2F, RANGE_DIRECT }, // 1: STR BEND
  { 0x10002A2F, 0x1002282F, RANGE_DIRECT }, // 2: STR BEND
  { 0x1000352F, 0x1002322F, RANGE_DIRECT }, // 3: STR BEND
  { 0, 0, LAST_THREE }, // ALL: STR BEND
  { 0x10001612, 0x10021612, RANGE_DIRECT }, // 1: DYNA BEND
  { 0x10002112, 0x10022012, RANGE_DIRECT }, // 2: DYNA BEND
  { 0x10002C12, 0x10022A12, RANGE_DIRECT }, // 3: DYNA BEND
  { 0, 0, LAST_THREE }, // ALL: DYNA BEND
  { 0x10001222, 0x10021222, RANGE_DIRECT }, // MIXER A/B
  { 0x10001225, 0x10021225, RANGE_DIRECT }, // BAL1 A/B
  { 0x10001228, 0x10021228, RANGE_DIRECT }, // BAL2 A/B
  { 0x1000122B, 0x1002122B, RANGE_DIRECT }, // BAL3 A/B
};

#define INST1_CUTOFF_TYPE 14
#define INST2_CUTOFF_TYPE 15
#define INST3_CUTOFF_TYPE 16
#define INST1_RESONANCE_TYPE 18
#define INST2_RESONANCE_TYPE 19
#define INST3_RESONANCE_TYPE 20

const uint8_t SY1000_NUMBER_OF_EXP_PEDAL_TARGETS = sizeof(SY1000_exp_pedal_target) / sizeof(SY1000_exp_pedal_target[0]);


void MD_SY1000_class::move_expression_pedal(uint8_t sw, uint8_t value, uint8_t exp_pedal) {
  if (exp_pedal == 0) exp_pedal = current_exp_pedal;
  LCD_show_bar(0, value); // Show it on the main display

  if ((exp_pedal == 1) && (exp1_type < SY1000_NUMBER_OF_EXP_PEDAL_TARGETS)) {
    send_expression_value(exp1_type, value);
  }
  if ((exp_pedal == 2) && (exp2_type < SY1000_NUMBER_OF_EXP_PEDAL_TARGETS)) {
    send_expression_value(exp2_type, value);
  }
  update_exp_label(sw);
  LCD_show_popup_label(SP[sw].Label, ACTION_TIMER_LENGTH);
  update_page = REFRESH_FX_ONLY; // To update the other switch states, we re-load the current page*/
}

void MD_SY1000_class::send_expression_value(uint8_t exp_type, uint8_t value) {
  uint32_t address;
  if (!bass_mode) address = SY1000_exp_pedal_target[exp_type].Address_GM;
  else address = SY1000_exp_pedal_target[exp_type].Address_BM;
  uint8_t type = SY1000_exp_pedal_target[exp_type].Type;

  uint8_t v8;
  uint16_t v16;

  uint8_t inst_type = 255;
  switch (type) {
    case RANGE_DIRECT:
      v8 = map(value, 0, 127, 0, 100);
      write_sysex(address, v8);
      break;
    case RANGE_CUTOFF:
      if (exp_type == INST1_CUTOFF_TYPE) inst_type = INST1_type;
      if (exp_type == INST2_CUTOFF_TYPE) inst_type = INST2_type;
      if (exp_type == INST3_CUTOFF_TYPE) inst_type = INST3_type;

      v8 = map(value, 0, 127, 0, 100);
      if (inst_type == 0) write_sysex(address + 0x011F, v8); // Address of cutoff of DYN_SYNTH compared to base address of instrument
      if (inst_type == 1) write_sysex(address + 0x031D, v8); // Address of cutoff of OSC_SYNTH compared to base address of instrument
      if (inst_type == 2) write_sysex(address + 0x0402, v8); // Address of cutoff of GR-300 compared to base address of instrument
      break;
    case RANGE_RESONANCE:
      if (exp_type == INST1_RESONANCE_TYPE) inst_type = INST1_type;
      if (exp_type == INST2_RESONANCE_TYPE) inst_type = INST2_type;
      if (exp_type == INST3_RESONANCE_TYPE) inst_type = INST3_type;

      v8 = map(value, 0, 127, 0, 100);
      if (inst_type == 0) write_sysex(address + 0x0120, v8); // Address of resonance of DYN_SYNTH compared to base address of instrument
      if (inst_type == 1) write_sysex(address + 0x031F, v8); // Address of resonance of OSC_SYNTH compared to base address of instrument
      if (inst_type == 2) write_sysex(address + 0x0403, v8); // Address of resonance of GR-300 compared to base address of instrument
      break;
    case RANGE_FX:
      v8 = map(value, 0, 127, 0, 100);
      write_sysex(address + 0x1605, v8); // Address of PEDAL BEND compared to base address of instrument
      v16 = map(value, 0, 127, 0, 1000);
      write_sysex(address + 0x2502, (v16 & 0xF000) >> 12, (v16 & 0x0F00) >> 8, (v16 & 0x00F0) >> 4, (v16 & 0x000F)); // Address of WAH compared to base address of instrument
      write_sysex(address + 0x0B08, (v16 & 0xF000) >> 12, (v16 & 0x0F00) >> 8, (v16 & 0x00F0) >> 4, (v16 & 0x000F)); // Address of FOOT VOL compared to base address of instrument
      break;
    case RANGE100:
      v8 = map(value, 0, 127, 0, 100);
      write_sysex(address, (v8 & 0xF0) >> 4, (v8 & 0x0F));
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
      break;
    case LAST_THREE:
      send_expression_value(exp_type - 3, value);
      send_expression_value(exp_type - 2, value);
      send_expression_value(exp_type - 1, value);
      break;
    case LAST_THREEx2:
      send_expression_value(exp_type - 6, value);
      send_expression_value(exp_type - 4, value);
      send_expression_value(exp_type - 2, value);
      break;
  }
}

void MD_SY1000_class::toggle_expression_pedal(uint8_t sw) {
  if (current_exp_pedal == 0) return;
  current_exp_pedal++;
  if (current_exp_pedal > 2) current_exp_pedal = 1;
  update_page = REFRESH_FX_ONLY;
}

bool MD_SY1000_class::request_exp_pedal(uint8_t sw, uint8_t exp_pedal) { // Used for both Master_exp_pedal and toggle_exp_pedal
  if (exp_pedal == 0) exp_pedal = current_exp_pedal;
  if (exp_pedal <= 2) {
    last_requested_sysex_switch = sw;
    if (!bass_mode) request_sysex(0x1000002A, 2); // Request the target of exp1 and exp2
    else request_sysex(0x1002002A, 2);
  }
  LCD_clear_SP_label(sw);
  return false;
}

void MD_SY1000_class::update_exp_label(uint8_t sw) {
  if (current_exp_pedal == 1) {
    SP[sw].PP_number = SY1000_EXP1;
    read_parameter(sw, exp1_type, 0);
  }
  if (current_exp_pedal == 2) {
    SP[sw].PP_number = SY1000_EXP2;
    read_parameter(sw, exp2_type, 0);
  }
}
