// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: GR55 Initialization
// Section 2: GR55 common MIDI in functions
// Section 3: GR55 common MIDI out functions
// Section 4: GR55 program change
// Section 5: GR55 parameter control
// Section 6: GR55 assign control
// Section 7: GR55 preset names

// ********************************* Section 1: GR55 initialization ********************************************

//Sysex messages for Roland GR-55
#define GR55_REQUEST_MODE 0x18000000, 1 //Is 00 in guitar mode, 01 in bass mode (Gumtown in town :-)
#define GR55_REQUEST_CURRENT_PATCH_NAME 0x18000001, 16 //Request 16 bytes for current patch name
#define GR55_REQUEST_PATCH_NUMBER 0x01000000, 2 //Request current patch number
#define GR55_CTL_LED_ON 0x18000011, 0x01
#define GR55_CTL_LED_OFF 0x18000011, 0x00

#define GR55_TEMPO 0x1800023C  // Accepts values from 40 bpm - 250 bpm

#define GR55_SYNTH1_SW 0x18002003 // The address of the synth1 switch
#define GR55_SYNTH2_SW 0x18002103 // The address of the synth1 switch
#define GR55_COSM_GUITAR_SW 0x1800100A // The address of the COSM guitar switch
#define GR55_NORMAL_PU_SW 0x18000232 // The address of the COSM guitar switch

#define GR55_NUMBER_OF_FACTORY_PATCHES 360
#define GR55_PATCH_MAX_BASS_MODE 404

// Initialize device variables
// Called at startup of VController
void GR55_class::init() // Default values for variables
{
  // Boss GR-55 variables:
  strcpy(device_name, "GR55");
  strcpy(full_device_name, "Roland GR-55");
  current_patch_name.reserve(17);
  current_patch_name = "                ";
  patch_min = GR55_PATCH_MIN;
  patch_max = GR55_PATCH_MAX;
  bank_size = 3;
  max_times_no_response = MAX_TIMES_NO_RESPONSE; // The number of times the GP-10 does not have to respond before disconnection
  preset_banks = 40; // Default number of preset banks is 40. When we are in bass mode, there are only 12.
  synth1_onoff = 0;
  synth2_onoff = 0;
  COSM_onoff = 0;
  nrml_pu_onoff = 0;
  SYSEX_DELAY_LENGTH = 10; // time between sysex messages (in msec)
  my_LED_colour = 3; // Default value: blue
  MIDI_channel = GR55_MIDI_CHANNEL; // Default value
  bank_number = 0; // Default value
  is_always_on = true; // Default value
  my_patch_page = PAGE_GR55_PATCH_BANK; // Default value
  my_parameter_page = PAGE_CURRENT_PARAMETER; // Default value
  my_assign_page = PAGE_GR55_ASSIGNS; // Default value
}

// ********************************* Section 2: GR55 common MIDI in functions ********************************************

void GR55_class::check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port) {  // Check incoming sysex messages from  Called from MIDI:OnSysEx/OnSerialSysEx

  // Check if it is a message from a GR-55
  if ((port == MIDI_port) && (sxdata[1] == 0x41) && (sxdata[2] == MIDI_device_id) && (sxdata[3] == 0x00) && (sxdata[4] == 0x00) && (sxdata[5] == 0x53) && (sxdata[6] == 0x12)) {
    uint32_t address = (sxdata[7] << 24) + (sxdata[8] << 16) + (sxdata[9] << 8) + sxdata[10]; // Make the address 32 bit

    // Check if it is the patch number
    if (address == 0x01000000) {
      if (patch_number != sxdata[12]) { //Right after a patch change the patch number is sent again. So here we catch that message.
        patch_number = sxdata[11] * 128 + sxdata[12];
        if (patch_number > 2047) patch_number = patch_number - 1751; // There is a gap of 1752 patches in the numbering system of the GR-55. This will close it.
        //page_check();
        do_after_patch_selection();
        update_page |= REFRESH_PAGE;
      }
    }

    // Check if it is the current parameter
    if (address == last_requested_sysex_address) {
      switch (last_requested_sysex_type) {
        case PATCH_NAME:
          for (uint8_t count = 0; count < 16; count++) {
            SP[Current_switch].Label[count] = static_cast<char>(sxdata[count + 11]); //Add ascii character to the SP.Label String
          }
          if (SP[Current_switch].PP_number == patch_number) {
            current_patch_name = SP[Current_switch].Label; // Load patchname when it is read
            update_main_lcd = true; // And show it on the main LCD
          }
          DEBUGMSG(SP[Current_switch].Label);
          PAGE_request_next_switch();
          break;
        case PARAMETER_TYPE:
          read_parameter(sxdata[11], sxdata[12]);
          PAGE_request_next_switch();
          break;
        case ASSIGN_TYPE:
          read_current_assign(address, sxdata, sxlength);
          break;
      }
    }

    // Check if it is the patch name (address: 0x18, 0x00, 0x00, 0x01)
    if ((sxdata[6] == 0x12) && (address == 0x18000001) ) {
      current_patch_name = "";
      for (uint8_t count = 11; count < 27; count++) {
        current_patch_name = current_patch_name + static_cast<char>(sxdata[count]); //Add ascii character to Patch Name String
      }
      update_main_lcd = true;
    }

    // Check if it is the guitar on/off states
    check_guitar_switch_states(sxdata, sxlength);

    // Check if the GR55 is in bass mode (address: 0x18, 0x00, 0x00, 0x00)
    if ((sxdata[6] == 0x12) && (address == 0x18000000) && (sxdata[11] == 0x01)) {
      bass_mode = true; // In bass mode
      preset_banks = 12;
      if (patch_max > GR55_PATCH_MAX_BASS_MODE) patch_max = GR55_PATCH_MAX_BASS_MODE;
      DEBUGMSG("GR55 is in Bass Mode");
    }
  }
}


void GR55_class::check_PC_in(uint8_t program, uint8_t channel, uint8_t port) { // Check incoming PC messages from  Called from MIDI:OnProgramChange

  // Check the source by checking the channel
  if ((port == MIDI_port) && (channel == MIDI_channel)) { // GR55 outputs a program change
    uint16_t new_patch = (CC01 * 128) + program;
    if (patch_number != new_patch) {
      patch_number = new_patch;
      if (patch_number > 2047) patch_number = patch_number - 1751; // There is a gap of 1752 patches in the numbering system of the GR-55. This will close it.
      request_sysex(GR55_REQUEST_CURRENT_PATCH_NAME); // So the main display always show the correct patch
      //page_check();
      do_after_patch_selection();
      update_page |= REFRESH_PAGE;
    }
  }
}

/*void check_CC_in_GR55(byte channel, byte control, byte value) {  // Check incoming CC messages from GR-55
  if (channel == MIDI_channel) { // GR55 outputs a control change message
    if (control == 0) {
      CC01 = value;
    }
  }
}*/

// Detection of GR-55

void GR55_class::identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port) {
  // Check if it is a GR-55
  if ((sxdata[5] == 0x41) && (sxdata[6] == 0x53) && (sxdata[7] == 0x02)) {
    no_response_counter = 0;
    if (connected == false) connect(sxdata[2], port); //Byte 2 contains the correct device ID
  }
}

void GR55_class::do_after_connect() {
  request_sysex(GR55_REQUEST_PATCH_NUMBER);
  request_sysex(GR55_REQUEST_CURRENT_PATCH_NAME); // So the main display always show the correct patch
  request_sysex(GR55_REQUEST_MODE); // Check if the GR-55 is in bass mode
  do_after_patch_selection();
  update_page |= RELOAD_PAGE;
}


// ********************************* Section 3: GR55 common MIDI out functions ********************************************

void GR55_class::write_sysex(uint32_t address, uint8_t value) { // For sending one data byte

  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t checksum = calc_Roland_checksum(ad[3] + ad[2] + ad[1] + ad[0] + value); // Calculate the Roland checksum
  uint8_t sysexmessage[14] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x53, 0x12, ad[3], ad[2], ad[1], ad[0], value, checksum, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 14, MIDI_port);
}

void GR55_class::write_sysex(uint32_t address, uint8_t value1, uint8_t value2) { // For sending two data bytes

  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t checksum = calc_Roland_checksum(ad[3] + ad[2] + ad[1] + ad[0] + value1 + value2); // Calculate the Roland checksum
  uint8_t sysexmessage[15] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x53, 0x12, ad[3], ad[2], ad[1], ad[0], value1, value2, checksum, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 15, MIDI_port);
}

void GR55_class::request_sysex(uint32_t address, uint8_t no_of_bytes) {
  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t checksum = calc_Roland_checksum(ad[3] + ad[2] + ad[1] + ad[0] +  no_of_bytes); // Calculate the Roland checksum
  uint8_t sysexmessage[17] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x53, 0x11, ad[3], ad[2], ad[1], ad[0], 0x00, 0x00, 0x00, no_of_bytes, checksum, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 17, MIDI_port);
}

void GR55_class::set_bpm() {
  if (connected) {
    write_sysex(GR55_TEMPO, Setting.Bpm / 16, Setting.Bpm % 16); // Tempo is modulus 16. It's all so very logical. NOT.
  }
}

void GR55_class::start_tuner() {
  if (connected) {
    was_on = is_on;
    mute_now(); // Mute the GR-55. There is no documentation on how to start the tuner on the GR55 through sysex.
  }
}

void GR55_class::stop_tuner() {
  if (was_on) {
    unmute(); // Unmute the GR-55. There is no documentation on how to start the tuner on the GR55 through sysex.
  }
}

// ********************************* Section 4: GR55 program change ********************************************

void GR55_class::SendProgramChange(uint16_t new_patch) {

  if (new_patch == patch_number) unmute();
  patch_number = new_patch;

  uint16_t GR55_patch_send = 0; // Temporary value
  if (patch_number > 296) {
    GR55_patch_send = patch_number + 1751; // There is a gap of 1752 patches in the numbering system of the GR-55. This will recreate it.
  }
  else {
    GR55_patch_send = patch_number;
  }

  MIDI_send_CC(0, GR55_patch_send / 128, MIDI_channel, MIDI_port);
  MIDI_send_PC(GR55_patch_send % 128, MIDI_channel, MIDI_port);
  DEBUGMSG("out(GR55) PC" + String(new_patch)); //Debug
  //mute();
  //VG99.mute();
  do_after_patch_selection();
  update_page |= REFRESH_PAGE;
}

void GR55_class::do_after_patch_selection() {
  request_onoff = false;
  is_on = connected;
  if (Setting.Send_global_tempo_after_patch_change == true) set_bpm();
  Current_patch_number = patch_number; // the patch name
  update_LEDS = true;
  update_main_lcd = true;
  //update_page |= REFRESH_PAGE;
  request_guitar_switch_states();
  //EEPROM.write(EEPROM_GR55_PATCH_MSB, (patch_number / 256));
  //EEPROM.write(EEPROM_GR55_PATCH_LSB, (patch_number % 256));
  update_page = REFRESH_FX_ONLY;
}

/*void GR55_patch_bank_load(uint8_t sw, uint8_t bank_position, uint8_t bank_size) {
  if (bank_selection_active == false) bank_select_number = (patch_number / bank_size);
  uint16_t new_patch = (bank_select_number * bank_size) + bank_position;
  if (new_patch > patch_max) new_patch = new_patch - patch_max - 1 + patch_min;
  GR55_patch_load(sw, new_patch); // Calculate the patch number for this switch
}

void GR55_patch_load(uint8_t sw, uint16_t number) {
  SP[sw].Colour = GR55_PATCH_COLOUR;
  SP[sw].PP_number = number;
  //SP[sw].Address = calculate_patch_address(number);
}*/

/*uint32_t GR55_class::calculate_patch_address(uint16_t number) {
  return 0x20000001 + ((number / 0x80) * 0x1000000) + ((number % 0x80) * 0x10000); //Calculate the address where the patchname is stored on the GR-55;
}*/

void GR55_class::request_patch_name(uint16_t number) {
  if (number < 297) { // This is a user patch - we read these from memory
    uint32_t Address = 0x20000001 + ((number / 0x80) * 0x1000000) + ((number % 0x80) * 0x10000); //Calculate the address where the patchname is stored on the GR-55
    last_requested_sysex_address = Address;
    last_requested_sysex_type = PATCH_NAME;
    request_sysex(Address, 16); //Request the 16 bytes of the GP10 patchname
  }
  else {
    if (number < (297 + GR55_NUMBER_OF_FACTORY_PATCHES)) { // Fixed patch - read from GR55_preset_patch_names array
      read_preset_name(Current_switch, SP[Current_switch].PP_number);
    }
    PAGE_request_next_switch(); // Move on to next switch
  }
}

void GR55_class::request_current_patch_name() {
  request_sysex(GR55_REQUEST_CURRENT_PATCH_NAME);
}

void GR55_class::display_patch_number_string() {
  if (bank_selection_active() == false) {
    number_format(patch_number, Current_patch_number_string);
  }
  else {
    number_format(bank_select_number * bank_size, Current_patch_number_string);
  }
}

void GR55_class::number_format(uint16_t number, String &Output) {
  // Uses patch_number as input and returns Current_patch_number_string as output in format "U01-1"
  // First character is L for Lead, R for Rhythm, O for Other or U for User
  // In guitar mode preset_banks is set to 40, in bass mode it is set to 12, because there a less preset banks in bass mode.

  uint16_t patch_number_corrected = 0; // Need a corrected version of the patch number to deal with the funny numbering system of the GR-55
  uint16_t bank_number_corrected = 0; //Also needed, because with higher banks, we start counting again

  bank_number_corrected = (number / 3); // Calculate the bank number from the patch number

  if (bank_number_corrected < 99) {
    Output = Output + "U";
    patch_number_corrected = number;  //In the User bank all is normal
  }

  else {
    if (bank_number_corrected >= (99 + (2 * preset_banks))) {   // In the Other bank we have to adjust the bank and patch numbers so we start with O01-1
      Output = Output + "O";
      patch_number_corrected = number - (297 + (6 * preset_banks));
      bank_number_corrected = bank_number_corrected - (99 + (2 * preset_banks));
    }

    else {
      if (bank_number_corrected >= (99 + preset_banks)) {   // In the Rhythm bank we have to adjust the bank and patch numbers so we start with R01-1
        Output = Output + "R";
        patch_number_corrected = number - (297 + (3 * preset_banks));
        bank_number_corrected = bank_number_corrected - (99 + preset_banks);
      }

      else    {// In the Lead bank we have to adjust the bank and patch numbers so we start with L01-1
        Output = Output + "L";
        patch_number_corrected = number - 297;
        bank_number_corrected = bank_number_corrected - 99;
      }
    }
  }

  // Then add the bank number
  Output = Output + String(((patch_number_corrected / 3) + 1) / 10) + String(((patch_number_corrected / 3) + 1) % 10);
  // Finally add the patch number
  Output = Output + "-" + String((patch_number_corrected % 3) + 1);
}

void GR55_class::direct_select_format(uint16_t number, String &Output) {
  if (direct_select_state == 0) Output = Output + "U" + String(number) + "_-_";
  else Output = Output + "U" + String(direct_select_first_digit) + String(number) + "-_";
}

void GR55_class::direct_select_press(uint8_t number) {
  if (direct_select_state == 0) {
    // First digit pressed
    direct_select_first_digit = number;
    direct_select_state = 1;
  }
  else  {
    // Second digit pressed
    device_in_bank_selection = my_device_number + 1; // Go into bank mode
    uint16_t base_patch = (direct_select_first_digit * 30) + (number * 3);
    bank_select_number = (base_patch / Previous_bank_size);
    DEBUGMSG("PREVIOUS BANK_SIZE: " + String(Previous_bank_size));
    direct_select_state = 0;
    SCO_select_page(Previous_page);
    /*uint16_t bank_mod3 = (direct_select_first_digit * 10) + number;
    if (bank_mod3 == 0) bank_mod3 = 1;
    bank_select_number = (bank_mod3 - 1) / 3; // Banknumber is modulus 9 like the main page...
    direct_select_state = 0;
    SCO_select_page(PAGE_GR55_PATCH_BANK);*/
  }
}

// ** US-20 simulation
// Selecting and muting the GR55 is done by storing the settings of COSM guitar switch and Normal PU switch
// and switching both off when guitar is muted and back to original state when the GR55 is selected

void GR55_class::request_guitar_switch_states() {
  delay(10); // Extra delay, otherwise first parameter is not read after patch change
  request_sysex(GR55_SYNTH1_SW, 1);
  request_sysex(GR55_SYNTH2_SW, 1);
  request_sysex(GR55_COSM_GUITAR_SW, 1);
  request_sysex(GR55_NORMAL_PU_SW, 1);
  request_onoff = true;
}

void GR55_class::check_guitar_switch_states(const unsigned char* sxdata, short unsigned int sxlength) {
  if (request_onoff == true) {
    uint32_t address = (sxdata[7] << 24) + (sxdata[8] << 16) + (sxdata[9] << 8) + sxdata[10]; // Make the address 32 bit

    if (address == GR55_SYNTH1_SW) {
      synth1_onoff = sxdata[11];  // Store the value
    }

    if (address == GR55_SYNTH2_SW) {
      synth2_onoff = sxdata[11];  // Store the value
    }

    if (address == GR55_COSM_GUITAR_SW) {
      COSM_onoff = sxdata[11];  // Store the value
    }

    if (address == GR55_NORMAL_PU_SW) {
      nrml_pu_onoff = sxdata[11];  // Store the value
      request_onoff = false;
    }
  }
}

void GR55_class::unmute() {
  is_on = connected;
  //GR55_select_LED = GR55_PATCH_COLOUR; //Switch the LED on
  write_sysex(GR55_SYNTH1_SW, synth1_onoff); // Switch synth 1 off
  write_sysex(GR55_SYNTH2_SW, synth2_onoff); // Switch synth 1 off
  write_sysex(GR55_COSM_GUITAR_SW, COSM_onoff); // Switch COSM guitar on
  write_sysex(GR55_NORMAL_PU_SW, nrml_pu_onoff); // Switch normal pu on
}

void GR55_class::mute() {
  if ((Setting.US20_emulation_active) && (!is_always_on) && (is_on)) {
    mute_now();
  }
}

void GR55_class::mute_now() { // Also called when engaging global tuner.
  is_on = false;
  //  GR55_select_LED = GR55_OFF_COLOUR; //Switch the LED off
  write_sysex(GR55_SYNTH1_SW, 0x01); // Switch synth 1 off
  write_sysex(GR55_SYNTH2_SW, 0x01); // Switch synth 1 off
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
  uint16_t Target; // Target of the assign as given in the assignments of the GR55 / GR55
  uint32_t Address; // The address of the parameter
  uint8_t NumVals; // The number of values for this parameter
  char Name[17]; // The name for the label
  uint16_t Sublist; // Which sublist to read for the FX or amp type - 0 if second byte does not contain the type or if there is no sublist +100 Show value from sublist.
  uint8_t Colour; // The colour for this effect.
  bool Reversed; // The GR-55 has SYNTH 1 SW, SYNTH 2 SW, COSM GUITAR SW and NORMAL PICKUP reversed. For these parameters the on and off values will be read and written in reverse
};

//typedef struct stomper Stomper;
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
  {0x12B, 0x18000304, 2, "MFX", 1 | SUBLIST_FROM_BYTE2, GR55_MFX_COLOUR, false},
  {0x12C, 0x18000305, 20, "MFX TYPE", 1, GR55_MFX_TYPE_COLOUR, false},
  {0x0E6, 0x18000715, 2, "MOD", 63 | SUBLIST_FROM_BYTE2, GR55_MOD_COLOUR, false},
  {0x0E7, 0x18000716, 14, "MOD TYPE", 63, GR55_MOD_TYPE_COLOUR, false},
  {0x000, 0x18002003, 2, "SYNTH1 SW", 0, FX_GTR_TYPE, true},
  {0x003, 0x18002005, 68, "PCM1 TONE OCT", 92, FX_GTR_TYPE, false}, // Not a perfect solution, as there is no minimal value
  {0x03B, 0x18002103, 2, "SYNTH2 SW", 0, FX_GTR_TYPE, true},
  {0x03E, 0x18002105, 68, "PCM2 TONE OCT", 92, FX_GTR_TYPE, false}, // Not a perfect solution, as there is no minimal value
  {0x076, 0x1800100A, 2, "COSM GT SW", 0, FX_GTR_TYPE, true},
  {0x081, 0x1800101D, 2, "12STRING SW", 0, FX_PITCH_TYPE, false},
  {0x0D6, 0x18000700, 2, "AMP", 21 | SUBLIST_FROM_BYTE2, FX_AMP_TYPE, false},
  {0xFFF, 0x18000701, 42, "AMP TP", 21, FX_AMP_TYPE, false},
  {0x0D7, 0x18000702, 121, "AMP GAIN", SHOW_NUMBER, FX_AMP_TYPE, false},
  {0x0D8, 0x18000703, 101, "AMP LVL", SHOW_NUMBER, FX_AMP_TYPE, false},
  {0x0D9, 0x18000704, 3, "AMP GAIN SW", 141, FX_AMP_TYPE, false},
  {0x0DA, 0x18000705, 2, "AMP SOLO SW", 0, FX_AMP_TYPE, false},
  {0x0DC, 0x18000707, 101, "AMP BASS", SHOW_NUMBER, FX_AMP_TYPE, false},
  {0x0DD, 0x18000708, 101, "AMP MID", SHOW_NUMBER, FX_AMP_TYPE, false},
  {0x0DE, 0x18000709, 101, "AMP TREBLE", SHOW_NUMBER, FX_AMP_TYPE, false},
  {0x0DF, 0x1800070A, 101, "AMP PRESC", SHOW_NUMBER, FX_AMP_TYPE, false},
  {0x0E0, 0x1800070B, 2, "AMP BRIGHT", 0, FX_AMP_TYPE, false},
  {0x0E1, 0x1800070C, 9, "SPKR TYPE", 144, FX_AMP_TYPE, false},
  {0x128, 0x1800075A, 2, "NS SWITCH", 0, FX_FILTER_TYPE, false},
  {0x1EC, 0x18000605, 2, "DLY SW", 129 | SUBLIST_FROM_BYTE2, FX_DELAY_TYPE, false},
  {0x1ED, 0x18000606, 7, "DLY TYPE", 129, FX_DELAY_TYPE, false},
  {0x1F4, 0x1800060C, 2, "RVRB SW", 136 | SUBLIST_FROM_BYTE2, FX_REVERB_TYPE, false},
  {0x1F5, 0x1800060D, 5, "RVRB TYPE", 136, FX_REVERB_TYPE, false},
  {0x1FC, 0x18000600, 2, "CHOR SW", 125 | SUBLIST_FROM_BYTE2, FX_MODULATE_TYPE,  false},
  {0x1FD, 0x18000601, 4, "CHOR TYPE", 125, FX_MODULATE_TYPE,  false},
  {0x204, 0x18000611, 2, "EQ SWITCH", 0, FX_FILTER_TYPE, false},
  {0x213, 0x18000234, 2, "TUN SW", 160 | SUBLIST_FROM_BYTE2, FX_PITCH_TYPE, false},
  {0x214, 0x18000235, 13, "TUNING", 160, FX_PITCH_TYPE, false},
  {0x216, 0x18000230, 201, "PATCH LVL", SHOW_DOUBLE_NUMBER, FX_FILTER_TYPE, false},
  {0xFFF, 0x18000011, 2, "CTL", 94 | SUBLIST_FROM_BYTE2, FX_DEFAULT_TYPE, false},
  {0xFFF, 0x18000012, 16, "CTL SW", 94, FX_DEFAULT_TYPE, false},
  {0xFFF, 0x1800001F, 10, "EXP", 84, FX_DEFAULT_TYPE, false},
  {0xFFF, 0x18000036, 10, "EXP ON", 84, FX_DEFAULT_TYPE, false},
  {0xFFF, 0x1800004D, 2, "EXP SW", 111 | SUBLIST_FROM_BYTE2, FX_DEFAULT_TYPE, false},
  {0xFFF, 0x1800004E, 14, "EXP SW", 111, FX_DEFAULT_TYPE, false},
  {0xFFF, 0x1800005B, 10, "GK VOL", 84, FX_DEFAULT_TYPE, false},
  {0xFFF, 0x18000072, 14, "GK S1", 111, FX_DEFAULT_TYPE, false},
  {0xFFF, 0x1800007F, 14, "GK S2", 111, FX_DEFAULT_TYPE, false},
  {0xFFF, 0x02000007, 2, "GTR2MIDI", 0, FX_DEFAULT_TYPE, false},
  {0xFFF, 0x02000008, 2, "G2M MODE", 77, FX_DEFAULT_TYPE, false},
  {0xFFF, 0x02000009, 2, "G2M CHOMATIC", 0, FX_DEFAULT_TYPE, false},
  {0xFFF, 0x02000016, 5, "GTR OUT", 79, FX_DEFAULT_TYPE, false},
};

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
  FX_FILTER_TYPE, // Colour for"COMPRESS",
  FX_FILTER_TYPE, // Colour for"LIMITER",
  FX_DELAY_TYPE, // Colour for"3TAP DLY",
  FX_DELAY_TYPE, // Colour for"TIME DLY",
  FX_FILTER_TYPE, // Colour for"LOFI CPS",
  FX_PITCH_TYPE // Colour for"PITCH SH"
};

const PROGMEM uint8_t GR55_MOD_colours[14] = {
  FX_DIST_TYPE, // Colour for"OD/DS",
  FX_FILTER_TYPE, // Colour for"WAH",
  FX_FILTER_TYPE, // Colour for"COMP",
  FX_FILTER_TYPE, // Colour for"LIMITER",
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

void GR55_class::read_parameter_name(uint16_t number, String &Output) { // Called from menu
  if (number < number_of_parameters())  Output = GR55_parameters[number].Name;
  else Output = "?";
}

void GR55_class::read_parameter_state(uint16_t number, uint8_t value, String &Output) {
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
void GR55_class::parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {

  uint8_t value = SCO_return_parameter_value(Sw, cmd);
  
  if (GR55_parameters[number].Sublist == 92) { // Create minimum value for tone octave
    if (value < 61) value = 61;
  }
  
  if (GR55_parameters[number].Reversed) value ^= 1; // Reverse the value

  if (SP[Sw].Latch != TGL_OFF) {
    if (GR55_parameters[number].Sublist == SHOW_DOUBLE_NUMBER) write_sysex(GR55_parameters[number].Address, value / 16, value % 16);
    else write_sysex(GR55_parameters[number].Address, value);
    // Show message
    check_update_label(Sw, value);
    LCD_show_status_message(SP[Sw].Label);
    if (SP[Sw].Latch != UPDOWN) update_page |= REFRESH_PAGE; // To update the other switch states, we re-load the current page
  }
}

void GR55_class::parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {
  // Work out state of pedal
  if (SP[Sw].Latch == MOMENTARY) {
    //SP[Sw].State = 2; // Switch state off
    write_sysex(GR55_parameters[number].Address, cmd->Value1);

    update_page |= REFRESH_PAGE; // To update the other switch states, we re-load the current page
  }
}

bool GR55_class::request_parameter(uint16_t number) {
  uint32_t my_address = GR55_parameters[number].Address;
  last_requested_sysex_address = my_address;
  last_requested_sysex_type = PARAMETER_TYPE;
  request_sysex(my_address, 2);
  return false; // We have not read the parameter yet
}

void GR55_class::read_parameter(uint8_t byte1, uint8_t byte2) { //Read the current GR55 parameter
  SP[Current_switch].Target_byte1 = byte1;
  SP[Current_switch].Target_byte2 = byte2;

  uint16_t index = SP[Current_switch].PP_number; // Read the parameter number (index to GR55-parameter array)

  if (GR55_parameters[index].Sublist == SHOW_DOUBLE_NUMBER) { // Don't you just love all these weird exceptions!
    byte1 = (byte1 * 16) + byte2; // Change byte1 so the parameter updates correctly on the display
    SP[Current_switch].Target_byte1 = byte1;
  }

  // Set the status
  SP[Current_switch].State = SCO_find_parameter_state(Current_switch, byte1);

  // Reverse state 1 and 2 if neccesary
  if (GR55_parameters[index].Reversed) {
    if (SP[Current_switch].State == 1) SP[Current_switch].State = 2;
    else if (SP[Current_switch].State == 2) SP[Current_switch].State = 1;
  }

  // Set the colour
  uint8_t my_colour = GR55_parameters[index].Colour;

  //Check for special colours:
  switch (my_colour) {
    case GR55_MFX_COLOUR:
      SP[Current_switch].Colour = GR55_MFX_colours[byte2]; //MFX type read in byte2
      break;
    case GR55_MFX_TYPE_COLOUR:
      SP[Current_switch].Colour = GR55_MFX_colours[byte1]; //MFX type read in byte1
      break;
    case GR55_MOD_COLOUR:
      SP[Current_switch].Colour = GR55_MOD_colours[byte2]; //MOD type read in byte2
      break;
    case GR55_MOD_TYPE_COLOUR:
      SP[Current_switch].Colour = GR55_MOD_colours[byte1]; //MOD type read in byte1
      break;
    default:
      SP[Current_switch].Colour =  my_colour;
      break;
  }

  // Set the display message
  String msg = GR55_parameters[index].Name;
  if (GR55_parameters[index].Sublist > SUBLIST_FROM_BYTE2) { // Check if a sublist exists
    String type_name = GR55_sublists[GR55_parameters[index].Sublist - SUBLIST_FROM_BYTE2 + byte2 - 1];
    msg = msg + " (" + type_name + ")";
  }
  if ((GR55_parameters[index].Sublist > 0) && !(GR55_parameters[index].Sublist & SUBLIST_FROM_BYTE2)) { // Check if state needs to be read
    msg += ":";
    read_parameter_state(index, byte1, msg);
    //  String type_name = GR55_sublists[GR55_parameters[index].Sublist + byte1 - 1];
    //  msg = msg + ":" + type_name;
  }
  //Copy it to the display name:
  LCD_set_label(Current_switch, msg);
}

void GR55_class::check_update_label(uint8_t Sw, uint8_t value) { // Updates the label for extended sublists
  uint16_t index = SP[Sw].PP_number; // Read the parameter number (index to GR55-parameter array)
  if (index != NOT_FOUND) {
    if ((GR55_parameters[index].Sublist > 0) && !(GR55_parameters[index].Sublist & SUBLIST_FROM_BYTE2)) { // Check if state needs to be read
      LCD_clear_label(Sw);
      // Set the display message
      String msg = GR55_parameters[index].Name;
      //String type_name = GR55_sublists[GR55_parameters[index].Sublist + value - 1];
      //msg = msg + ":" + type_name;
      msg += ":";
      read_parameter_state(index, value, msg);

      //Copy it to the display name:
      LCD_set_label(Sw, msg);

      //Update the current switch label
      update_lcd = Sw;
    }
  }
}

uint16_t GR55_class::number_of_parameters() {
  return GR55_NUMBER_OF_PARAMETERS;
}

uint8_t GR55_class::number_of_values(uint16_t parameter) {
  if (parameter < GR55_NUMBER_OF_PARAMETERS) return GR55_parameters[parameter].NumVals;
  else return 0;
}

// ********************************* Section 6: GR55 assign control ********************************************

// GR55 has eight general assigns, which we can control with a cc-MIDI message.

// Procedures for GR55_ASSIGN:
// 1. Load in SP array - GR55_assign_load() below
// 2. Request - GR55_request_current_assign()
// 3. Read assign - GR55_read_current_assign() - also requests parameter state
// 4. Read parameter state - GR55_read_parameter() above
// 5. Press switch - GR55_assign_press() below
// 6. Release switch - GR55_assign_release() below

#define GR55_NUMBER_OF_ASSIGNS 8
/*const PROGMEM char GR55_assign_title[GR55_NUMBER_OF_ASSIGNS][14] = {
  "ASSIGN 1", "ASSIGN 2", "ASSIGN 3", "ASSIGN 4", "ASSIGN 5", "ASSIGN 6", "ASSIGN 7", "ASSIGN 8",
  //"CTL", "EXP", "EXPON", "EXP SW", "GK S1", "GK S2", "GK VOL"
};*/
const PROGMEM uint32_t GR55_assign_address[GR55_NUMBER_OF_ASSIGNS] = { 0x1800010C, 0x1800011F, 0x18000132, 0x18000145, 0x18000158, 0x1800016B, 0x1800017E, 0x18000211 };

void GR55_class::read_assign_name(uint8_t number, String & Output) {
  //if (number < GR55_NUMBER_OF_ASSIGNS)  Output = GR55_assign_title[number];
  if (number < GR55_NUMBER_OF_ASSIGNS)  Output = "ASSIGN " + String(number + 1);
  else Output = "?";
}

void GR55_class::read_assign_trigger(uint8_t number, String & Output) {
  if ((number > 0) && (number < 128)) Output = "CC#" + String(number);
  else Output = "-";
}

uint8_t GR55_class::get_number_of_assigns() {
  return GR55_NUMBER_OF_ASSIGNS;
}

uint8_t GR55_class::trigger_follow_assign(uint8_t number) {
  return number + 20; // Default cc numbers are 20 and up
}

void GR55_class::assign_press(uint8_t Sw, uint8_t value) { // Switch set to GR55_ASSIGN is pressed
  // Send cc MIDI command to GR-55
  uint8_t cc_number = SP[Sw].Trigger;
  MIDI_send_CC(cc_number, value, MIDI_channel, MIDI_port);

  // Display the patch function
  if (SP[Sw].Assign_on) {
    uint8_t value = 0;
    if (SP[Sw].State == 1) value = SP[Sw].Assign_max;
    else value = SP[Sw].Assign_min;
    check_update_label(Sw, value);
  }
  LCD_show_status_message(SP[Sw].Label);

  if (SP[Sw].Assign_on) update_page |= REFRESH_PAGE; // To update the other switch states, we re-load the current page
}

void GR55_class::assign_release(uint8_t Sw) { // Switch set to GR55_ASSIGN is released
  // Send cc MIDI command to GR-55
  uint8_t cc_number = SP[Sw].Trigger;
  MIDI_send_CC(cc_number, 0, MIDI_channel, MIDI_port);

  // Update status
  if (SP[Sw].Latch == MOMENTARY) {
    if (SP[Sw].Assign_on) SP[Sw].State = 2; // Switch state off
    else SP[Sw].State = 0; // Assign off, so LED should be off as well

    if (SP[Sw].Assign_on) update_page |= REFRESH_PAGE; // To update the other switch states, we re-load the current page
  }
}

void GR55_class::assign_load(uint8_t sw, uint8_t assign_number, uint8_t cc_number) { // Switch set to GR55_ASSIGN is loaded in SP array
  SP[sw].Trigger = cc_number; //Save the cc_number in the Trigger variable
  SP[sw].Assign_number = assign_number;
}

void GR55_class::request_current_assign() {
  uint8_t index = SP[Current_switch].Assign_number - 1;  //index should be between 0 and 7
  //SP[Current_switch].Address = GR55_assign_address[index];

  if (index < GR55_NUMBER_OF_ASSIGNS) {
    DEBUGMSG("Request assign " + String(index + 1));
    //read_assign_target = false;
    uint32_t my_address = GR55_assign_address[index];
    last_requested_sysex_address = my_address;
    last_requested_sysex_type = ASSIGN_TYPE;
    request_sysex(my_address, 12);  //Request 12 bytes for the GR55 assign
  }
  else PAGE_request_next_switch(); // Wrong assign number given in Config - skip it
}

void GR55_class::read_current_assign(uint32_t address, const unsigned char* sxdata, short unsigned int sxlength) {
  bool found;
  String msg;
  uint8_t assign_switch = sxdata[11];
  uint8_t assign_target = (sxdata[12] << 8) + (sxdata[13] << 4) + sxdata[14];
  uint8_t assign_target_min = ((sxdata[15] - 0x04) << 8) + (sxdata[16] << 4) + sxdata[17];
  uint8_t assign_target_max = ((sxdata[18] - 0x04) << 8) + (sxdata[19] << 4) + sxdata[20];
  uint8_t assign_source = sxdata[21];
  uint8_t assign_latch = sxdata[22];

  uint8_t my_trigger = SP[Current_switch].Trigger;
  if ((my_trigger >= 1) && (my_trigger <= 31)) my_trigger = my_trigger + 8; // Trigger is cc01 - cc31
  if ((my_trigger >= 64) && (my_trigger <= 95)) my_trigger = my_trigger - 24; // Trigger is cc64 - cc95

  bool assign_on = ((assign_switch == 0x01) && (my_trigger == assign_source)); // Check if assign is on by checking assign switch and source

  DEBUGMSG("GR-55 Assign_switch: 0x" + String(assign_switch, HEX));
  DEBUGMSG("GR-55 Assign_target 0x:" + String(assign_target, HEX));
  DEBUGMSG("GR-55 Assign_min: 0x" + String(assign_target_min, HEX));
  DEBUGMSG("GR-55 Assign_max: 0x" + String(assign_target_max, HEX));
  DEBUGMSG("GR-55 Assign_source: 0x" + String(assign_source, HEX));
  DEBUGMSG("GR-55 Assign_latch: 0x" + String(assign_latch, HEX));
  DEBUGMSG("GR-55 Assign_trigger-check:" + String(my_trigger) + "==" + String(assign_source));

  if (assign_on) {
    SP[Current_switch].Assign_on = true; // Switch the pedal off
    SP[Current_switch].Latch = assign_latch;

    // Allow for GR-55 assign min and max swap. Is neccesary, because the GR-55 will not save a parameter in the on-state, unless you swap the assign min and max values
    if (assign_target_max > assign_target_min) {
      // Store values the normal way around
      SP[Current_switch].Assign_max = assign_target_max;
      SP[Current_switch].Assign_min = assign_target_min;
    }
    else {
      // Reverse the values
      SP[Current_switch].Assign_max = assign_target_min;
      SP[Current_switch].Assign_min = assign_target_max;
    }

    // Request the target
    found = target_lookup(assign_target); // Lookup the address of the target in the GR55_Parameters array
    DEBUGMSG("Request target of assign" + String(SP[Current_switch].Assign_number) + ":" + String(SP[Current_switch].Address, HEX));
    if (found) {
      uint32_t my_address = SP[Current_switch].Address;
      last_requested_sysex_address = my_address;
      last_requested_sysex_type = PARAMETER_TYPE;
      //read_assign_target = true;
      request_sysex(my_address, 2);
    }
    else {
      SP[Current_switch].PP_number = NOT_FOUND;
      SP[Current_switch].Colour = FX_DEFAULT_TYPE;
      // Set the Label
      msg = "CC#" + String(SP[Current_switch].Trigger) + " (ASGN" + String(SP[Current_switch].Assign_number) + ")";
      LCD_set_label(Current_switch, msg);
      PAGE_request_next_switch();
    }
  }
  else { // Assign is off
    SP[Current_switch].Assign_on = false; // Switch the pedal off
    SP[Current_switch].State = 0; // Switch the stompbox off
    SP[Current_switch].Latch = MOMENTARY; // Make it momentary
    SP[Current_switch].Colour = FX_DEFAULT_TYPE; // Set the on colour to default
    // Set the Label
    msg = "CC#" + String(SP[Current_switch].Trigger);
    LCD_set_label(Current_switch, msg);
    PAGE_request_next_switch();
  }
}

bool GR55_class::target_lookup(uint16_t target) {  // Finds the target and its address in the GR55_parameters table

  // Lookup in GR55_parameter array
  bool found = false;
  for (uint16_t i = 0; i < GR55_NUMBER_OF_PARAMETERS; i++) {
    if (target == GR55_parameters[i].Target) { //Check is we've found the right target
      SP[Current_switch].PP_number = i; // Save the index number
      SP[Current_switch].Address = GR55_parameters[i].Address;
      found = true;
    }
  }
  return found;
}

// ********************************* Section 7: GR55 preset names ********************************************

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

void GR55_class::read_preset_name(uint8_t number, uint16_t patch) {
  String lbl;
  if (!bass_mode) lbl = GR55_preset_patch_names[patch - 297]; //Read the label from the guitar array
  else lbl = GR55_preset_bass_patch_names[patch - 297]; //Read the label from the bass array
  LCD_set_label(number, lbl);
  if (patch == patch_number) current_patch_name = lbl; //Keeps the main display name updated
}


