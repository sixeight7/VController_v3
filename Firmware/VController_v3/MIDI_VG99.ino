// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: VG99/FC300 Initialization
// Section 2: VG99 common MIDI in functions
// Section 3: VG99 common MIDI out functions
// Section 4: VG99 program change
// Section 5: VG99 parameter control
// Section 6: VG99 assign control

// Note: when connecting the VG-99 to the RRC connector make sure that you make the following settings:
// 1) Go to SYSTEM / MIDI / PAGE 4 on the VG99
// 2) Switch on RRC -> main (F1)
// 3) Switch off RRC <- main (F2 and F3)
// The reason is that the VG99 sends back patch changes to the VController, which make the system unresponsive.
// I cannot trace this back to the source, but it may be a firmware error on the VG-99.

// ********************************* Section 1: VG99/FC300 initialization ********************************************

//boolean VG99_FC300_mode = false; // If the FC300 is attached, patch names will be sent in FC300 SYSEX messages

//Sysex messages for the VG-99
#define VG99_REQUEST_CURRENT_PATCH_NAME 0x60000000, 16 //Request 16 bytes for current patch name
#define VG99_REQUEST_PATCH_NUMBER 0x71000100, 2 //Request current patch number

#define VG99_EDITOR_MODE_ON 0x70000100, 0x01 //Gets the VG-99 spitting out lots of sysex data. Does not have to be switched on for the tuner to work on the VG99
#define VG99_EDITOR_MODE_OFF 0x70000100, 0x00
#define VG99_TUNER_ON 0x70000000, 0x01 // Changes the running mode to tuner / multi-mode
#define VG99_TUNER_OFF 0x70000000, 0x00 // Changes the running mode to play
#define VG99_TAP_TEMPO_LED_CC 80 // If you have the D-beam switched off on the VG99, make an assign on the VG99: Source CC #80, momentary, target D BEAM - SELECT - Assignable / off on every patch and you will have a flashing LED. 
// Set to zero and VController will send no CC message. You can also set the cc on the VG99 to V-link sw, but that generates a lot of midi data.

#define VG99_PATCH_CHANGE 0x71000000 //00 00 Patch 001 and 03 0F Patch 400
#define VG99_TEMPO 0x60000015  // Accepts values from 40 bpm - 250 bpm
#define VG99_KEY 0x60000017    // Accepts values from 0 (C) - 11 (B)

#define VG99_COSM_GUITAR_A_SW 0x60003000 // The address of the COSM guitar switch
#define VG99_COSM_GUITAR_B_SW 0x60003800 // The address of the COSM guitar switch
//bool request_onoff = false;

//Sysex messages for FC300 in sysex mode:
//I am trying to fool the VG-99 to believe there is an FC300 attached, but it does not work.
#define FC300_TUNER_ON 0x1002, 0x01 // Does not work for some reason
#define FC300_TUNER_OFF 0x1002, 0x00
#define FC300_SYSEX_MODE 0x1000, 0x01 //Tell the VG-99 we are in sysex mode
#define FC300_NORMAL_MODE 0x1000, 0x00

// VG99 handshake with FC300
// 1. VG99 sends continually F0 41 7F 00 00 1F 11 00 01 7F F7 on RRC port (not on other ports)
// 2. FC300 responds with ???

// The VG99 sends a PC message back to the VController. This message arrives after a new patch change has been made on rapid selection of patches
// Therefore PC messages will be ignored for 1 second after the patch has been changed on the VController
#define VG99_PC_IGNORE_TIME 1000 // 1000ms = 1 sec

// Initialize device variables
// Called at startup of VController
void VG99_class::init() // Default values for variables
{
  // Roland VG-99 variables:
  strcpy(device_name, "VG99");
  strcpy(full_device_name, "Roland VG-99");
  current_patch_name.reserve(17);
  current_patch_name = "                ";
  patch_min = VG99_PATCH_MIN;
  patch_max = VG99_PATCH_MAX;
  bank_size = 10;
  max_times_no_response = MAX_TIMES_NO_RESPONSE; // The number of times the VG-99 does not have to respond before disconnection
  COSM_A_onoff = 0;
  COSM_B_onoff = 0;
  SYSEX_DELAY_LENGTH = 0; // time between sysex messages (in msec)
  my_LED_colour = 2; // Default value: red
  MIDI_channel = VG99_MIDI_CHANNEL; // Default value
  bank_number = 0; // Default value
  is_always_on = true; // Default value
  my_patch_page = PAGE_CURRENT_PATCH_BANK;  // Default value
  my_parameter_page = PAGE_CURRENT_PARAMETER; // Default value
  my_assign_page = PAGE_VG99_ASSIGNS; // Default value
}

// ********************************* Section 2: VG99 common MIDI in functions ********************************************

void VG99_class::check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port) {  // Check incoming sysex messages from  Called from MIDI:OnSysEx/OnSerialSysEx

  // Check if it is a message from a VG-99
  if ((port == MIDI_port) && (sxdata[1] == 0x41) && (sxdata[2] == MIDI_device_id) && (sxdata[3] == 0x00) && (sxdata[4] == 0x00) && (sxdata[5] == 0x1C) && (sxdata[6] == 0x12)) {
    uint32_t address = (sxdata[7] << 24) + (sxdata[8] << 16) + (sxdata[9] << 8) + sxdata[10]; // Make the address 32 bit

    // Check if it is the patch number
    if (address == 0x71000100) {
      if (patch_number != sxdata[12]) { //Right after a patch change the patch number is sent again. So here we catch that message.
        patch_number = sxdata[11] * 128 + sxdata[12];
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

    // Check if it is the current patch name (address: 0x60, 0x00, 0x00, 0x00)
    if ((sxdata[6] == 0x12) && (address == 0x60000000) ) {
      current_patch_name = "";
      for (uint8_t count = 11; count < 28; count++) {
        current_patch_name = current_patch_name + static_cast<char>(sxdata[count]); //Add ascii character to Patch Name String
      }
      update_main_lcd = true;
    }

    // Check if it is the guitar on/off states
    check_guitar_switch_states(sxdata, sxlength);
  }

  // Also check messages for the FC300 mode of the VG99
  check_SYSEX_in_fc300(sxdata, sxlength);
}

void VG99_class::check_SYSEX_in_fc300(const unsigned char* sxdata, short unsigned int sxlength) { // Check incoming sysex messages from VG99/FC300. Called from MIDI:OnSysEx/OnSerialSysEx

  // Check if it is a message from a VG-99 in FC300 mode.
  if ((sxdata[1] == 0x41) && (sxdata[3] == 0x00) && (sxdata[4] == 0x00) && (sxdata[5] == 0x20)) {
    uint16_t address = (sxdata[7] << 8) + sxdata[8]; // Make the address 16 bit

    // Check if it is a data request - here we have to fool the VG99 into believing there is an FC300 attached
    // I found the numbers by packet sniffing the communication between the two devices. The addresses are not mentioned in the FC300 manual.

    if (sxdata[6] == 0x11) {
      if (address == 0x0000) { // Request for address 0x0000
        //        VG99_FC300_mode = true; //We are in FC300 mode!!!
        write_sysexfc(0x0000, 0x01, 0x00, 0x00); // Answer with three numbers - 01 00 00
        DEBUGMSG("VG99 request for address 0x0000 answered");
      }
      if (address == 0x0400) { // Request for address 0x0400
        //write_sysexfc(0x0400, 0x03); // Answer with one number - 03
        DEBUGMSG("VG99 request for address 0x0400 answered");
      }
      if (address == 0x0600) { // Request for address 0x0600
        write_sysexfc(0x0600, 0x10, 0x02, 0x08); // Answer with three numbers - 10 02 08
        DEBUGMSG("VG99 request for address 0x0600 answered");
        fix_reverse_pedals();
      }
    }
  }
}

void VG99_class::check_PC_in(uint8_t program, uint8_t channel, uint8_t port) { // Check incoming PC messages from  Called from MIDI:OnProgramChange

  // Check the source by checking the channel
  if ((port == MIDI_port) && (channel == MIDI_channel)) { // VG99 outputs a program change
    uint16_t new_patch = (CC01 * 100) + program;
    if ((patch_number != new_patch) && (millis() - PC_ignore_timer > VG99_PC_IGNORE_TIME)) {
      patch_number = new_patch;
      request_sysex(VG99_REQUEST_CURRENT_PATCH_NAME); // So the main display always show the correct patch
      //page_check();
      do_after_patch_selection();
      update_page |= REFRESH_PAGE;
    }
  }
}

// Detection of VG-99

void VG99_class::identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port) {
  // Check if it is a VG-99
  if ((sxdata[5] == 0x41) && (sxdata[6] == 0x1C) && (sxdata[7] == 0x02)) {
    no_response_counter = 0;
    if (connected == false) connect(sxdata[2], port); //Byte 2 contains the correct device ID
  }
}

void VG99_class::do_after_connect() {
  //write_sysex(VG99_EDITOR_MODE_ON); // Put the VG-99 into editor mode - saves lots of messages on the VG99 display, but may also overload the MIDI buffer
  //editor_mode = true;
  //VG99_fix_reverse_pedals();
  request_sysex(VG99_REQUEST_PATCH_NUMBER);
  request_sysex(VG99_REQUEST_CURRENT_PATCH_NAME); // So the main display always show the correct patch
  do_after_patch_selection();
  update_page |= REFRESH_PAGE;
}

// ********************************* Section 3: VG99 common MIDI out functions ********************************************

void VG99_class::write_sysex(uint32_t address, uint8_t value) { // For sending one data byte

  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t checksum = calc_Roland_checksum(ad[3] + ad[2] + ad[1] + ad[0] + value); // Calculate the Roland checksum
  uint8_t sysexmessage[14] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x1C, 0x12, ad[3], ad[2], ad[1], ad[0], value, checksum, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 14, MIDI_port);
}

void VG99_class::write_sysex(uint32_t address, uint8_t value1, uint8_t value2) { // For sending two data bytes

  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t checksum = calc_Roland_checksum(ad[3] + ad[2] + ad[1] + ad[0] + value1 + value2); // Calculate the Roland checksum
  uint8_t sysexmessage[15] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x1C, 0x12, ad[3], ad[2], ad[1], ad[0], value1, value2, checksum, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 15, MIDI_port);
}

void VG99_class::write_sysexfc(uint16_t address, uint8_t value) { // VG99 writing to the FC300

  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into two bytes: ad[1] and ad[0]
  uint8_t checksum = calc_Roland_checksum(ad[1] + ad[0] + value); // Calculate the Roland checksum
  uint8_t sysexmessage[12] = {0xF0, 0x41, FC300_device_id, 0x00, 0x00, 0x020, 0x12, ad[1], ad[0], value, checksum, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 12, MIDI_port);
}

void VG99_class::write_sysexfc(uint16_t address, uint8_t value1, uint8_t value2, uint8_t value3) { // VG99 writing to the FC300 - 3 bytes version

  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into two bytes: ad[1] and ad[0]
  uint8_t checksum = calc_Roland_checksum(ad[1] + ad[0] + value1 + value2 + value3); // Calculate the Roland checksum
  uint8_t sysexmessage[14] = {0xF0, 0x41, FC300_device_id, 0x00, 0x00, 0x020, 0x12, ad[1], ad[0], value1, value2, value3, checksum, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 14, MIDI_port);
}

void VG99_class::request_sysex(uint32_t address, uint8_t no_of_bytes) {
  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t checksum = calc_Roland_checksum(ad[3] + ad[2] + ad[1] + ad[0] +  no_of_bytes); // Calculate the Roland checksum
  uint8_t sysexmessage[17] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x1C, 0x11, ad[3], ad[2], ad[1], ad[0], 0x00, 0x00, 0x00, no_of_bytes, checksum, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 17, MIDI_port);
}

void VG99_class::set_bpm() {
  if (connected) {
    write_sysex(VG99_TEMPO, (Setting.Bpm - 40) / 128, (Setting.Bpm - 40) % 128); // Tempo is modulus 128 on the  And sending 0 gives tempo 40.
  }
}

void VG99_class::start_tuner() {
  if (connected) {
    write_sysex(VG99_TUNER_ON); // Start tuner on VG-99
  }
}

void VG99_class::stop_tuner() {
  if (connected) {
    write_sysex(VG99_TUNER_OFF); // Stop tuner on VG-99
  }
}

// ********************************* Section 4: VG99 program change ********************************************

void VG99_class::SendProgramChange(uint16_t new_patch) {
  //if (new_patch == patch_number) unmute();
  patch_number = new_patch;

  MIDI_send_CC(0, new_patch / 100, MIDI_channel, MIDI_port);
  MIDI_send_PC(new_patch % 100, MIDI_channel, MIDI_port);
  DEBUGMSG("out(VG99) PC" + String(new_patch)); //Debug
  //mute();
  //GR55.mute();
  do_after_patch_selection();
  update_page |= REFRESH_PAGE;
}

void VG99_class::do_after_patch_selection() {
  request_onoff = false;
  is_on = connected;
  if (Setting.Send_global_tempo_after_patch_change == true) set_bpm();
  Current_patch_number = patch_number;
  update_LEDS = true;
  update_main_lcd = true;
  request_guitar_switch_states();

  if (!PAGE_check_on_page(my_device_number, patch_number)) { // Check if patch is on the page 
    update_page |= REFRESH_PAGE;
  }
  else {
    update_page = REFRESH_FX_ONLY;
  }
}

/*uint32_t VG99_class::calculate_patch_address(uint16_t number) {
  return 0x71010000 + (((number * 0x10) / 0x80) * 0x100) + ((number * 0x10) % 0x80); //Calculate the address where the patchname is stored on the VG-99
}*/

void VG99_class::request_patch_name(uint16_t number) {
  uint32_t Address = 0x71010000 + (((number * 0x10) / 0x80) * 0x100) + ((number * 0x10) % 0x80); //Calculate the address where the patchname is stored on the VG-99
  last_requested_sysex_address = Address;
  last_requested_sysex_type = PATCH_NAME;
  request_sysex(Address, 16); //Request the 16 bytes of the GP10 patchname
}

void VG99_class::request_current_patch_name() {
  request_sysex(VG99_REQUEST_CURRENT_PATCH_NAME);
}

/*void VG99_class::page_check() { // Checks if the current patch is on the page and will reload the page if not
  bool onpage = false;
  for (uint8_t s = 0; s < NUMBER_OF_SWITCHES; s++) {
    if ((SP[s].Type == PATCH_BANK) && (SP[s].PP_number == patch_number)) {
      onpage = true;
      current_patch_name = SP[s].Label; // Set patchname correctly
    }
  }
  if (!onpage) {
    update_page |= REFRESH_PAGE;
  }
}*/

void VG99_class::display_patch_number_string() {
  if (bank_selection_active() == false) {
    number_format(patch_number, Current_patch_number_string);
  }
  else {
    String start_number1, end_number1;
    number_format(bank_select_number * bank_size, start_number1);
    number_format((bank_select_number + 1) * bank_size - 1, end_number1);
    Current_patch_number_string = Current_patch_number_string + start_number1 + "-" + end_number1;
  }

}

void VG99_class::number_format(uint16_t number, String &Output) {
  // Uses patch_number as input and returns Current_patch_number_string as output in format "U001"
  // First character is U for User or P for Preset patches
  if (number > 199) Output +=  "P";
  else Output +=  "U";

  // Then add the patch number
  Output +=  String((number + 1) / 100) + String(((number + 1) / 10) % 10) + String((number + 1) % 10);
}

void VG99_class::direct_select_format(uint16_t number, String &Output) {
  if (direct_select_state == 0) Output +=  "U" + String(bank_select_number) + String(number) + "_";
  else Output +=  "U" + String(direct_select_first_digit / 10) + String(direct_select_first_digit % 10) + String(number);
}


// ** US-20 simulation
// Selecting and muting the VG99 is done by storing the settings of COSM guitar switch and Normal PU switch
// and switching both off when guitar is muted and back to original state when the VG99 is selected

void VG99_class::request_guitar_switch_states() {
  //VG99_select_LED = VG99_PATCH_COLOUR; //Switch the LED on
  request_sysex(VG99_COSM_GUITAR_A_SW, 1);
  request_sysex(VG99_COSM_GUITAR_B_SW, 1);
  request_onoff = true;
}

void VG99_class::check_guitar_switch_states(const unsigned char* sxdata, short unsigned int sxlength) {
  if (request_onoff == true) {
    uint32_t address = (sxdata[7] << 24) + (sxdata[8] << 16) + (sxdata[9] << 8) + sxdata[10]; // Make the address 32 bit

    if (address == VG99_COSM_GUITAR_A_SW) {
      COSM_A_onoff = sxdata[11];  // Store the value
    }

    if (address == VG99_COSM_GUITAR_B_SW) {
      COSM_B_onoff = sxdata[11];  // Store the value
      request_onoff = false;
    }
  }
}

void VG99_class::unmute() {
  is_on = connected;
  //VG99_select_LED = VG99_PATCH_COLOUR; //Switch the LED on
  //write_sysex(VG99_COSM_GUITAR_A_SW, COSM_A_onoff); // Switch COSM guitar on
  //write_sysex(VG99_COSM_GUITAR_B_SW, COSM_B_onoff); // Switch normal pu on
  SendProgramChange(patch_number); //Just sending the program change will put the sound back on
}

void VG99_class::mute() {
  if ((Setting.US20_emulation_active) && (!is_always_on) && (is_on)) {
    mute_now();
  }
}

void VG99_class::mute_now() {
  is_on = false;
  //  VG99_select_LED = VG99_OFF_COLOUR; //Switch the LED off
  write_sysex(VG99_COSM_GUITAR_A_SW, 0x00); // Switch COSM guitar off
  write_sysex(VG99_COSM_GUITAR_B_SW, 0x00); // Switch normal pu off
}

// ********************************* Section 5: VG99 parameter control ********************************************

// Procedures for the VG99_PARAMETER and VG99_ASSIGN commands

// Procedures for the VG99_PARAMETER:
// 1. Load in SP array - in PAGE_load_current(true)
// 2. Request parameter state - in PAGE_request_current_switch()
// 3. Read parameter state - VG99_read_parameter() below
// 4. Press switch - VG99_parameter_press() below - also calls VG99_check_update_label()
// 5. Release switch - VG99_parameter_release() below - also calls VG99_check_update_label()

struct VG99_parameter_struct { // Combines all the data we need for controlling a parameter in a device
  uint16_t Address; // The address of the parameter
  uint8_t NumVals; // The number of values this parameter may have
  char Name[17]; // The name for the label
  uint16_t Sublist; // Which sublist to read for the FX or amp type
  uint8_t Colour; // The colour for this effect.
};

//typedef struct stomper Stomper;
#define VG99_FX_COLOUR 255 // Just a colour number to pick the colour from the VG99_FX_colours table
#define VG99_FX_TYPE_COLOUR 254 //Another number for the MFX type
#define VG99_POLYFX_COLOUR 253 // Just a colour number to pick the colour from the VG99_POLY_FX_colours table
#define VG99_POLYFX_TYPE_COLOUR 252 //Another number for the MOD type
#define SUBLIST_FROM_BYTE2 32768 //Bit 16 set high means read sublist frm byte 2
#define SHOW_NUMBER 31767 // Special number for showing the number (used for gain/volume/etc) - set in sublist
#define SHOW_DOUBLE_NUMBER 31766 // Show number times 2 (used for patch volume/etc) - set in sublist
#define SHOW_PAN 31765 // Special number for showing the pan- set in sublist

// Make sure you edit the VG99_NUMBER_OF_STOMPS when adding new parameters
//#define VG99_NUMBER_OF_PARTS 8
//#define VG99_NUMBER_OF_PARAMETERS 28
const PROGMEM VG99_parameter_struct VG99_parameters[] = {
  //{ // part 0: 0000 - 1000 Tunings
  {0x0017, 12, "KEY", 161, FX_PITCH_TYPE},
  /*{0x0015, 250, "BPM", 0, FX_DELAY_TYPE},
  {0x0024, 2, "FC AMP CTL1", 0, FX_AMP_TYPE},
  {0x0025, 2, "FC AMP CTL2", 0, FX_AMP_TYPE},
  {0x0D1E, 4, "D BM SELECT", 0, FX_PITCH_TYPE},
  {0x0D1F, 2, "D BM PITCH TYP", 0, FX_PITCH_TYPE},
  {0x0D20, 3, "D BM T-ARM CH", 0, FX_PITCH_TYPE},
  {0x0D21, 4, "D BM T-ARM TYP", 0, FX_PITCH_TYPE},
  {0x0D26, 2, "D BM FREEZE CH", 0, FX_DEFAULT_TYPE},
  {0x0D29, 2, "D BM FRZ(A) LVL", 0, FX_DEFAULT_TYPE},
  {0x0D2D, 2, "D BM FRZ(B) LVL", 0, FX_DEFAULT_TYPE},
  {0x0D2F, 2, "D BM FILTER CH", 0, FX_FILTER_TYPE},
  {0x0D30, 2, "D BM FLTR TYPE", 0, FX_FILTER_TYPE},
  {0x0D34, 2, "D BM FILTR LVL", 0, FX_FILTER_TYPE},
  {0x0D35, 2, "RIBBON SELECT", 0, FX_DEFAULT_TYPE},
  {0x0D36, 2, "RBBN T-ARM CH", 0, FX_PITCH_TYPE},
  {0x0D37, 2, "RBBN T-ARM TYPE", 0, FX_PITCH_TYPE},
  {0x0D3C, 2, "RBBN FILTER CH", 0, FX_FILTER_TYPE},
  {0x0D3D, 2, "RBBN FILTER TYP", 0, FX_FILTER_TYPE},
  {0x0D41, 2, "RBBN FILTER LVL", 0, FX_FILTER_TYPE},*/
  //},

  //{ // part 1: 1000 - 2000 Alt tuning parameters
  {0x1001, 2, "[A]TUN", 144 | SUBLIST_FROM_BYTE2, FX_PITCH_TYPE},
  {0x1002, 13, "[A]TU TYP", 144, FX_PITCH_TYPE},
  {0x1007, 2, "[A]BEND SW", 0, FX_PITCH_TYPE},
  {0x1004, 2, "[A]12STRING SW", 0, FX_PITCH_TYPE},
  {0x1003, 2, "[A]DETUNE SW", 0, FX_PITCH_TYPE},
  {0x1005, 2, "[A]HM", 65 | SUBLIST_FROM_BYTE2, FX_PITCH_TYPE},
  {0x1006, 30, "[A]HARMO", 65, FX_PITCH_TYPE},
  {0x1009, 2, "[B]TUN", 144 | SUBLIST_FROM_BYTE2, FX_PITCH_TYPE},
  {0x100A, 13, "[B]TU TYP", 144, FX_PITCH_TYPE},
  {0x100F, 2, "[B]BEND SW", 0, FX_PITCH_TYPE},
  {0x100C, 2, "[B]12STRING SW", 0, FX_PITCH_TYPE},
  {0x100B, 2, "[B]DETUNE SW", 0, FX_PITCH_TYPE},
  {0x100D, 2, "[B]HM SW", 65 | SUBLIST_FROM_BYTE2, FX_PITCH_TYPE},
  {0x100E, 30, "[B]HARMO", 65, FX_PITCH_TYPE},
  //},

  //{ // part 2: 2000 - 3000 Common parameters
  {0x2007, 2, "DYNA SW", 0, FX_DEFAULT_TYPE},
  {0x2008, 2, "DYNA TYPE", 0, FX_DEFAULT_TYPE},
  {0x2014, 101, "[A]MIXER PAN", SHOW_PAN, FX_DEFAULT_TYPE},
  {0x2015, 101, "[A]MIXER LVL", SHOW_NUMBER, FX_DEFAULT_TYPE},
  {0x2019, 2, "[A]MIX SW", 0, FX_DEFAULT_TYPE},
  {0x201A, 101, "[B]MIXER PAN", SHOW_PAN, FX_DEFAULT_TYPE},
  {0x201B, 101, "[B]MIXER LVL", SHOW_NUMBER, FX_DEFAULT_TYPE},
  {0x201F, 2, "[B]MIX SW", 0, FX_DEFAULT_TYPE},
  {0x202F, 2, "TOTAL EQ SW", 0, FX_DEFAULT_TYPE},
  {0x2013, 101, "A/B BAL", SHOW_PAN, FX_DEFAULT_TYPE},
  {0x2000, 101, "PATCH LEVEL", SHOW_DOUBLE_NUMBER, FX_DEFAULT_TYPE},
  {0x2020, 2, "D/R RVB SW", 0, FX_REVERB_TYPE},
  {0x2021, 5, "D/R RVB TP", 241, FX_REVERB_TYPE},
  /*{0x2022, 2, "D/R REVRB TIME", 0, FX_REVERB_TYPE},
  {0x2023, 2, "D/R RVRB PREDY", 0, FX_REVERB_TYPE},
  {0x2024, 2, "D/R RVRB LWCUT", 0, FX_REVERB_TYPE},
  {0x2025, 2, "D/R RVRB HICUT", 0, FX_REVERB_TYPE},
  {0x2026, 2, "D/R REVRB DENS", 0, FX_REVERB_TYPE},*/
  {0x2027, 101, "D/R RVB LVL", SHOW_NUMBER, FX_REVERB_TYPE},
  {0x2028, 2, "D/R DELAY SW", 0, FX_DELAY_TYPE},
  //{0x2029, 2, "D/R DELAY TIME", 0, FX_DELAY_TYPE},
  //{0x202B, 2, "D/R DLAY FDBCK", 0, FX_DELAY_TYPE},
  //{0x202C, 2, "D/R DLAY HICUT", 0, FX_DELAY_TYPE},
  {0x202D, 121, "D/R DLY LVL", SHOW_NUMBER, FX_DELAY_TYPE},
  //},

  //{ // part 3: 3000 - 4000 Guitar parameters
  {0x3000, 2, "[A]COSM GTR SW", 0, FX_GTR_TYPE},
  {0x3001, 4, "[A]MODEL", 173, FX_GTR_TYPE},
  {0x301B, 10, "[A]E.GTR", 177, FX_GTR_TYPE},
  {0x301D, 5, "[A]PU SEL", 187, FX_GTR_TYPE},
  {0x301F, 101, "[A]E.GTR VOL", SHOW_NUMBER, FX_GTR_TYPE},
  {0x3022, 101, "[A]EGTR TONE", SHOW_NUMBER, FX_FILTER_TYPE},
  {0x3045, 6, "[A]AC TYPE", 193, FX_GTR_TYPE},
  {0x3046, 5, "[A]BODY", 199, FX_GTR_TYPE},
  {0x3069, 2, "[A]BASS TYPE", 204, FX_GTR_TYPE},
  {0x3400, 11, "[A]SYNTH T", 206, FX_GTR_TYPE},
  {0x3002, 2, "[A]GTR EQ SW", 0, FX_FILTER_TYPE},
  {0x3018, 101, "[A]COSM LVL", SHOW_NUMBER, FX_GTR_TYPE},
  {0x301A, 101, "[A]NPU LEVEL", SHOW_NUMBER, FX_GTR_TYPE},
  {0x3457, 2, "[A]NS SW", 0, FX_GTR_TYPE},
  {0x3800, 2, "[B]COSM GTR SW", 0, FX_GTR_TYPE},
  {0x3801, 4, "[B]MODEL", 173, FX_GTR_TYPE},
  {0x381B, 10, "[B]E.GTR", 177, FX_GTR_TYPE},
  {0x381D, 5, "[B]PU SEL", 187, FX_GTR_TYPE},
  {0x381F, 101, "[B]E.GTR VOL", SHOW_NUMBER, FX_GTR_TYPE},
  {0x3822, 101, "[B]EGTR TONE", SHOW_NUMBER, FX_FILTER_TYPE},
  {0x3845, 6, "[B]AC TYPE", 193, FX_GTR_TYPE},
  {0x3846, 5, "[B]BODY", 199, FX_GTR_TYPE},
  {0x3869, 2, "[B]BASS TYPE", 204, FX_GTR_TYPE},
  {0x3C00, 11, "[B]SYNTH T", 206, FX_GTR_TYPE},
  {0x3802, 2, "[B]GTR EQ SW", 0, FX_FILTER_TYPE},
  {0x3818, 101, "[B]COSM LVL", SHOW_NUMBER, FX_GTR_TYPE},
  {0x381A, 101, "[B]NPU LEVEL", SHOW_NUMBER, FX_GTR_TYPE},
  {0x3C57, 2, "[B]NS SW", 0, FX_FILTER_TYPE},
  //},

  //{ // part 4: 4000 - 5000 Poly FX
  {0x4000, 2, "POLY FX CHAN", 0, FX_FILTER_TYPE},
  {0x4001, 2, "POLYFX", 157 | SUBLIST_FROM_BYTE2, VG99_POLYFX_COLOUR},
  {0x4002, 4, "POLY TYPE", 157, VG99_POLYFX_TYPE_COLOUR},
  {0x4003, 2, "POLY COMP T", 217, FX_FILTER_TYPE},
  /*{0x4004, 2, "POLY COMP SUSTN", 0, FX_FILTER_TYPE},
  {0x4005, 2, "POLY COMP ATTACK", 0, FX_FILTER_TYPE},
  {0x4006, 2, "POLY COMP THRSH", 0, FX_FILTER_TYPE},
  {0x4007, 2, "POLY COMP REL", 0, FX_FILTER_TYPE},
  {0x4008, 2, "POLY COMP TONE", 0, FX_FILTER_TYPE},
  {0x4009, 2, "POLY COMP LEVEL", 0, FX_FILTER_TYPE},
  {0x400A, 2, "POLY COMP BAL", 0, FX_FILTER_TYPE},
  {0x400B, 2, "POLY DIST MODE", 0, FX_DIST_TYPE},
  {0x400C, 2, "POLY DIST DRIVE", 0, FX_DIST_TYPE},*/
  //{0x400D, 2, "POLY D HIGH CUT", 0, FX_DIST_TYPE},
  //{0x400E, 2, "POLY D POLY BAL", 0, FX_DIST_TYPE},
  //{0x400F, 2, "POLY D DRIVE BAL", 0, FX_DIST_TYPE},
  {0x4010, 101, "PDIST LVL", SHOW_NUMBER, FX_DIST_TYPE},
  //{0x402F, 2, "POLY SG RISETIME", 0, FX_MODULATE_TYPE},
  //{0x4030, 2, "POLY SG SENS", 0, FX_MODULATE_TYPE},
  //},

  //{ // part 5: 5000 - 6000 FX and amps chain A
  {0x502B, 2, "[A]COMP SW", 0, FX_FILTER_TYPE},
  {0x502C, 2, "[A]COMP TP", 217, FX_FILTER_TYPE},
  {0x5033, 2, "[A]OD", 1 | SUBLIST_FROM_BYTE2, FX_DIST_TYPE}, // Check VG99_odds_type table (1)
  {0x5034, 31, "[A]OD T", 1, FX_DIST_TYPE},
  {0x503F, 2, "[A]WAH SW", 219 | SUBLIST_FROM_BYTE2, FX_FILTER_TYPE},
  {0x5040, 7, "[A]WAH TP", 219, FX_FILTER_TYPE}, // Parameter number: 155
  {0x5041, 101, "[A]WAH POS", SHOW_NUMBER, FX_FILTER_TYPE},
  {0x5042, 101, "[A]WAH LVL", SHOW_NUMBER, FX_FILTER_TYPE},
  {0x5048, 2, "[A]EQ SW", 0, FX_FILTER_TYPE},
  {0x5054, 2, "[A]DLY SW", 226 | SUBLIST_FROM_BYTE2, FX_DELAY_TYPE},
  {0x5055, 11, "[A]DLY TP", 226, FX_DELAY_TYPE},
  {0x506B, 101, "[A]DLY LVL", SHOW_NUMBER, FX_DELAY_TYPE},
  {0x506D, 2, "[A]CHOR SW", 238 | SUBLIST_FROM_BYTE2, FX_MODULATE_TYPE},
  {0x506E, 3, "[A]CHOR MD", 238, FX_MODULATE_TYPE}, // Parameter number: 160
  {0x5074, 101, "[A]CHOR LVL", SHOW_NUMBER, FX_MODULATE_TYPE},
  {0x5075, 2, "[A]RVB SW", 241 | SUBLIST_FROM_BYTE2, FX_REVERB_TYPE},
  {0x5076, 7, "[A]RVB TP", 241, FX_REVERB_TYPE},
  {0x507C, 101, "[A]RVB LVL", SHOW_NUMBER, FX_REVERB_TYPE},
  {0x5400, 2, "[A]M1", 32 | SUBLIST_FROM_BYTE2, VG99_FX_COLOUR}, // Check VG99_mod_type table (2)
  {0x5401, 33, "[A]M1 TP", 32, VG99_FX_TYPE_COLOUR},
  {0x5800, 2, "[A]M2", 32 | SUBLIST_FROM_BYTE2, VG99_FX_COLOUR}, // Check VG99_mod_type table (2) // Parameter number: 165
  {0x5801, 33, "[A]M2 TP", 32, VG99_FX_TYPE_COLOUR},
  {0x507E, 2, "[A]NS SW", 0, FX_FILTER_TYPE},
  {0x500D, 2, "[A]AMP", 95 | SUBLIST_FROM_BYTE2, FX_AMP_TYPE},
  {0x500E, 49, "[A]AMP TP", 95, FX_AMP_TYPE},
  {0x500F, 121, "[A]AMP GAIN", SHOW_NUMBER, FX_AMP_TYPE}, // Parameter number: 170
  {0x5015, 2, "[A]AMP BRIGHT", 0, FX_AMP_TYPE},
  {0x5016, 3, "[A]GAIN SW", 258, FX_AMP_TYPE},
  {0x5017, 2, "[A]AMP(SOLO) SW", 0, FX_AMP_TYPE},
  {0x5019, 10, "[A]AMP SP", 248, FX_AMP_TYPE},
  {0x5102, 101, "[A]FOOT VOL", SHOW_NUMBER, FX_DEFAULT_TYPE},
  //},

  //{ // part 6: 6000 - 7000 FX and amps chain B
  {0x602B, 2, "[B]COMP SW", 0, FX_FILTER_TYPE}, // Parameter number: 180
  {0x602C, 2, "[B]COMP TP", 217, FX_FILTER_TYPE},
  {0x6033, 2, "[B]OD", 1 | SUBLIST_FROM_BYTE2, FX_DIST_TYPE}, // Check VG99_odds_type table (1)
  {0x6034, 31, "[B]OD T", 1, FX_DIST_TYPE},
  {0x603F, 2, "[B]WAH SW", 219 | SUBLIST_FROM_BYTE2, FX_FILTER_TYPE},
  {0x6040, 7, "[B]WAH TP", 219, FX_FILTER_TYPE}, // Parameter number: 185
  {0x6041, 101, "[B]WAH POS", SHOW_NUMBER, FX_FILTER_TYPE},
  {0x6042, 101, "[B]WAH LVL", SHOW_NUMBER, FX_FILTER_TYPE},
  {0x6048, 2, "[B]EQ SW", 0, FX_FILTER_TYPE},
  {0x6054, 2, "[B]DLY SW", 226 | SUBLIST_FROM_BYTE2, FX_DELAY_TYPE},
  {0x6055, 11, "[B]DLY TP", 226, FX_DELAY_TYPE},
  {0x606B, 101, "[B]DLY LVL", SHOW_NUMBER, FX_DELAY_TYPE},
  {0x606D, 2, "[B]CHOR SW", 238 | SUBLIST_FROM_BYTE2, FX_MODULATE_TYPE},
  {0x606E, 3, "[B]CHOR M", 238, FX_MODULATE_TYPE}, // Parameter number: 190
  {0x6074, 101, "[B]CHOR LVL", SHOW_NUMBER, FX_MODULATE_TYPE},
  {0x6075, 2, "[B]RVB SW", 241 | SUBLIST_FROM_BYTE2, FX_REVERB_TYPE},
  {0x6076, 7, "[B]RVB TP", 241, FX_REVERB_TYPE},
  {0x607C, 101, "[B]RVB LVL", SHOW_NUMBER, FX_REVERB_TYPE},
  {0x6400, 2, "[B]M1", 32 | SUBLIST_FROM_BYTE2, VG99_FX_COLOUR}, // Check VG99_mod_type table (2)
  {0x6401, 33, "[B]M1 TYPE", 32, VG99_FX_TYPE_COLOUR},
  {0x6800, 2, "[B]M2", 32 | SUBLIST_FROM_BYTE2, VG99_FX_COLOUR}, // Check VG99_mod_type table (2) // Parameter number: 195
  {0x6801, 33, "[B]M2 TYPE", 32, VG99_FX_TYPE_COLOUR},
  {0x607E, 2, "[B]NS SW", 0, FX_FILTER_TYPE},
  {0x600D, 2, "[B]AMP", 95 | SUBLIST_FROM_BYTE2, FX_AMP_TYPE}, // Sublist amps
  {0x600E, 49, "[B]AMP TP", 95, FX_AMP_TYPE},
  {0x600F, 121, "[B]AMP GAIN", SHOW_NUMBER, FX_AMP_TYPE}, // Parameter number: 200
  {0x6015, 2, "[B]AMP BRIGHT", 0, FX_AMP_TYPE},
  {0x6016, 3, "[B]GAIN SW", 258, FX_AMP_TYPE},
  {0x6017, 2, "[B]AMP(SOLO) SW", 0, FX_AMP_TYPE},
  {0x6019, 10, "[B]AMP SP", 248, FX_AMP_TYPE},
  {0x6102, 101, "[B]FOOT VOL", SHOW_NUMBER, FX_DEFAULT_TYPE},
  //},

  //{ // part 7: 7000 - 8000 Special functions - these work only from an assign - not from a parameter - update VG99_NON_PARAMETER_TARGETS below when adding parameters here
  {0x7600, 2, "[A]BEND", 0, FX_PITCH_TYPE}, // Parameter number: 210
  {0x7601, 2, "[B]BEND", 0, FX_PITCH_TYPE},
  {0x7602, 2, "DB T-ARM CONTROL", 0, FX_PITCH_TYPE},
  {0x7603, 2, "DB T-ARM SW", 0, FX_PITCH_TYPE},
  {0x7604, 2, "DB FREEZE SW", 0, FX_DEFAULT_TYPE},
  {0x7606, 2, "DB FILTER CONTRL", 0, FX_FILTER_TYPE}, // Parameter number: 215
  {0x7607, 2, "DB FILTER SW", 0, FX_FILTER_TYPE},
  {0x7608, 2, "RB T-ARM CONTROL", 0, FX_PITCH_TYPE},
  {0x7609, 2, "RB T-ARM SW", 0, FX_PITCH_TYPE},
  {0x760A, 2, "RB FILTER CONTRL", 0, FX_FILTER_TYPE},
  {0x760B, 2, "RB FILTER SW", 0, FX_FILTER_TYPE}, // Parameter number: 220
  {0x760C, 2, "[A]FX DLY REC", 0, FX_DELAY_TYPE},
  {0x760D, 2, "[A]FX DLY STOP", 0, FX_DELAY_TYPE},
  {0x760E, 2, "[B]FX DLY REC", 0, FX_DELAY_TYPE},
  {0x760F, 2, "[B]FX DLY STOP", 0, FX_DELAY_TYPE},
  {0x7611, 2, "BPM TAP", 0, FX_DELAY_TYPE}, // Parameter number: 225
  {0x7610, 2, "V-LINK SW", 0, FX_AMP_TYPE},
  {0x7F7F, 0, "OFF", 0, 0},
  //}
};

#define VG99_NON_PARAMETER_TARGETS 18

const uint16_t VG99_NUMBER_OF_PARAMETERS = sizeof(VG99_parameters) / sizeof(VG99_parameters[0]);

const PROGMEM char VG99_sublists[][8] = {
  // Sublist 1 - 31 from the "Control assign target" table on page 57 of the VG99 MIDI impementation guide
  "BOOST", "BLUES", "CRUNCH", "NATURAL", "TURBO", "FAT OD", "OD-1", "TSCREAM", "WARM OD", "DIST",
  "MILD DS", "DRIVE", "RAT", "GUV DS", "DST+", "SOLID", "MID DS", "STACK", "MODERN", "POWER", "R-MAN",
  "METAL", "HVY MTL", "LEAD", "LOUD", "SHARP", "MECHA", "60 FUZZ", "OCTFUZZ", "BIGMUFF", "CUSTOM",

  // Sublist 32 - 64 from the "FX mod type" table on page 71 of the VG99 MIDI impementation guide
  "COMPRSR", "LIMITER", "T. WAH", "AUTOWAH", "T_WAH", "---", "TREMOLO", "PHASER", //00 - 07
  "FLANGER", "PAN", "VIB", "UNI-V", "RINGMOD", "SLOW GR", "DEFRET", "", //08 - 0F
  "FEEDBKR", "ANTI FB", "HUMANZR", "SLICER", "---", "SUB EQ", "HARMO", "PITCH S", //10 - 17
  "P. BEND", "OCTAVE", "ROTARY", "2x2 CHS", "---", "---", "---", "---", //18 - 1F
  "S DELAY",

  // Sublist 65 - 94 from the "Harmony" table on page 56 of the VG99 MIDI impementation guide
  "-2oct", "-14th", "-13th", "-12th", "-11th", "-10th", "-9th",
  "-1oct", "-7th", "-6th", "-5th", "-4th", "-3rd", "-2nd", "TONIC",
  "+2nd", "+3rd", "+4th", "+5th", "+6th", "+7th", "+1oct", "+9th", "+10th", "+11th",
  "+12th", "+13th", "+14th", "+2oct", "USER",

  // Sublist 95 - 143 from the "COSM AMP" table on page 71 of the VG99 MIDI impementation guide
  "JC-120", "JC WRM", "JC JZZ", "JC FLL", "JC BRT", "CL TWN", "PRO CR", "TWEED", "WRM CR", "CRUNCH",
  "BLUES", "WILD C", "C STCK", "VXDRIV", "VXLEAD", "VXCLN", "MTCH D", "MTCH F", "MTCH L", "BGLEAD",
  "BGDRIV", "BRHYTM", "SMOOTH", "BGMILD", "MS1959", "MS1959", "MS1959", "MS HI", "MS PWR", "RF-CLN",
  "RF-RAW", "RF-VT1", "RF-MN1", "RF-VT2", "RF-MN1", "T-CLN", "T-CNCH", "T-LEAD", "T-EDGE", "SLDANO",
  "HI-DRV", "HI-LD", "HI-HVY", "5150", "MODERN", "M LEAD", "CUSTOM", "BASS V", "BASS M",

  // Sublist 144 - 156 from page 17 of the VG99 MIDI impementation guide
  "OPEN-D", "OPEN-E", "OPEN-G", "OPEN-A", "DROP-D", "D-MODAL", "-1 STEP", "-2 STEP", "BARITON", "NASHVL", "-1 OCT", "+1 OCT", "USER",

  // Sublist 157 - 160 for poly FX
  "COMPR", "DISTORT", "OCTAVE", "SLOW GR",

  // Sublist 161 - 172 for key
  "C", "Db", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B",

  // Sublist 173 - 176 for Modeling type
  "E.GTR", "AC", "BASS", "SYNTH",

  // Sublist 177 - 186 for E.GTR type
  "CLA-ST", "MOD-ST", "TE", "LP", "P-90", "LIPS", "RICK", "335", "L4", "VAR",

  // Sublist 187 - 192: Pickup position
  "REAR", "R+C", "CENTER", "C+F", "FRONT", "ALL",

  // Sublist 193 - 198: AC. type
  "STEEL", "NYLON", "SITAR", "BANJO", "RESO", "VARI",

  // Sublist 199 - 203: Ac body type
  "MA28", "TRP-0", "GB45", "GB SML", "GLD40",

  //Sublist 204,205: Bass Type
  "JB", "PB",

  // Sublist 206 - 216: Synth type
  "GR-300", "BOWED", "DUAL", "FBASS", "PIPE", "SOLO", "PWM", "CRYSTL", "ORGAN", "BRASS", "WAVE",

  // Sublist 217, 218: Comp type
  "COMP", "LIMITR",

  // Sublist 219 - 225: Wah type
  "CRY", "VO", "FAT", "LIGHT", "7STR", "RESO", "CUSTOM",

  // Sublist 226 - 237: Delay type
  "SINGLE", "PAN", "STEREO", "DUAL-S", "DUAL-P", "DU L/R", "REVRSE", "ANALOG", "TAPE", "WARP", "MOD", "HOLD",

  // Sublist 238 -240: Chorus type
  "MONO", "ST1", "ST2",

  // Sublist 241 - 247: Reverb Type
  "AMB", "ROOM", "HALL1", "HALL2", "PLATE", "SPRING", "MOD",

  // Sublist 248 - 257: Speaker type
  "OFF", "ORIG", "1x8\"", "1x10\"", "1x12\"", "2x12\"", "4x10\"", "4x12\"", "8x12\"", "CUSTOM",

  // Sublist 258 - 260: Amp gain switch
  "LOW", "MID", "HIGH",
};

const uint16_t VG99_SIZE_OF_SUBLIST = sizeof(VG99_sublists) / sizeof(VG99_sublists[0]);

const PROGMEM uint8_t VG99_FX_colours[33] = {
  FX_FILTER_TYPE, // Colour for "COMPRSR"
  FX_FILTER_TYPE, // Colour for "LIMITER"
  FX_FILTER_TYPE, // Colour for "T. WAH"
  FX_FILTER_TYPE, // Colour for "AUTOWAH"
  FX_FILTER_TYPE, // Colour for "T_WAH"
  FX_DEFAULT_TYPE, // Colour for "GUITAR SIM" - not implemented in VG99
  FX_MODULATE_TYPE, // Colour for "TREMOLO"
  FX_MODULATE_TYPE, // Colour for "PHASER"
  FX_MODULATE_TYPE, // Colour for "FLANGER"
  FX_MODULATE_TYPE, // Colour for "PAN"
  FX_MODULATE_TYPE, // Colour for "VIB"
  FX_MODULATE_TYPE, // Colour for "UNI - V"
  FX_MODULATE_TYPE, // Colour for "RINGMOD"
  FX_MODULATE_TYPE, // Colour for "SLOW GR"
  FX_DIST_TYPE, // Colour for "DEFRET"
  FX_DEFAULT_TYPE, // Colour for ""
  FX_FILTER_TYPE, // Colour for "FEEDBKR"
  FX_FILTER_TYPE, // Colour for "ANTI FB"
  FX_FILTER_TYPE, // Colour for "HUMANZR"
  FX_FILTER_TYPE, // Colour for "SLICER"
  FX_DEFAULT_TYPE, // Colour for "SITAR" - not implemented in VG99
  FX_FILTER_TYPE, // Colour for "SUB EQ"
  FX_PITCH_TYPE, // Colour for "HARMO"
  FX_PITCH_TYPE, // Colour for "PITCH S"
  FX_PITCH_TYPE, // Colour for "P. BEND"
  FX_PITCH_TYPE, // Colour for "OCTAVE"
  FX_MODULATE_TYPE, // Colour for "ROTARY"
  FX_MODULATE_TYPE, // Colour for "2x2 CHORUS"
  FX_DEFAULT_TYPE, // Colour for "AUTO RIFF" - not implemented in VG99
  FX_DEFAULT_TYPE, // Colour for "GUITAR SYNTH" - not implemented in VG99
  FX_DEFAULT_TYPE, // Colour for "AC. PROC" - not implemented in VG99
  FX_DEFAULT_TYPE, // Colour for "SOUND HOLD"  - not implemented in VG99
  FX_DELAY_TYPE // Colour for "S DELAY"
};

const PROGMEM uint8_t VG99_polyFX_colours[4] = {
  FX_FILTER_TYPE, // Colour for "COMPR"
  FX_DIST_TYPE, // Colour for "DISTORT"
  FX_PITCH_TYPE, // Colour for "OCTAVE"
  FX_FILTER_TYPE // Colour for "SLOW GR"
};

void VG99_class::read_parameter_name(uint16_t number, String &Output) { // Called from menu
  if (number < number_of_parameters())  Output = VG99_parameters[number].Name;
  else Output = "?";
}

void VG99_class::read_parameter_state(uint16_t number, uint8_t value, String &Output) {
  if (number < number_of_parameters())  {
    //Output += VG99_parameters[number].Name;
    if ((VG99_parameters[number].Sublist > 0) && !(VG99_parameters[number].Sublist & SUBLIST_FROM_BYTE2)) { // Check if state needs to be read
      switch (VG99_parameters[number].Sublist) {
        case SHOW_NUMBER:
          Output += String(value);
          break;
        case SHOW_DOUBLE_NUMBER:
          Output += String(value * 2); //Patch level is displayed double
          break;
        case SHOW_PAN:
          if (value < 50) Output += "L" + String(50 - value);
          if (value == 50) Output += "C";
          if (value > 50) Output += "R" + String(value - 50);
          break;
        default:
          String type_name = VG99_sublists[VG99_parameters[number].Sublist + value - 1];
          Output += type_name;
          break;
      }
    }
    else if (value == 1) Output += "ON";
    else Output += "OFF";
  }
  else Output += "?";
}

// Toggle VG99 stompbox parameter
void VG99_class::parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {

  // Send sysex MIDI command to VG-99
  uint8_t value = SCO_return_parameter_value(Sw, cmd);
  
  // Skip over non implemented FX types
  if (VG99_parameters[number].Colour == VG99_FX_TYPE_COLOUR) {
    if (value == 5) value = 6; // Skip over guitar sim
    if (value == 15) value = 16; // Skip over guitar sim
    if (value == 20) value = 21; // Skip over sitar
    if (value == 28) value = 33; // Skip over the last five FX
  }
  
  if (SP[Sw].Latch != TGL_OFF) {
    write_sysex(0x60000000 + VG99_parameters[number].Address, value);

    // Show message
    check_update_label(Sw, value);
    LCD_show_status_message(SP[Sw].Label);

    if (SP[Sw].Latch != UPDOWN) update_page |= REFRESH_PAGE; // To update the other switch states, we re-load the current page
  }
}

void VG99_class::parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {

  if (SP[Sw].Latch == MOMENTARY) {
    //SP[Sw].State = 2; // Switch state off
    write_sysex(0x60000000 + VG99_parameters[number].Address, cmd->Value1);

    update_page |= REFRESH_PAGE; // To update the other switch states, we re-load the current page
  }
}

bool VG99_class::request_parameter(uint16_t number) {
  uint32_t my_address = 0x60000000 + VG99_parameters[number].Address;
  last_requested_sysex_address = my_address;
  last_requested_sysex_type = PARAMETER_TYPE;
  request_sysex(my_address, 2);
  return false; // We have not read the parameter yet
}

void VG99_class::read_parameter(uint8_t byte1, uint8_t byte2) { //Read the current VG99 parameter
  SP[Current_switch].Target_byte1 = byte1;
  SP[Current_switch].Target_byte2 = byte2;

  //uint8_t part = SP[Current_switch].PP_number / 30; // Split the parameter number in part and index
  //uint8_t index = SP[Current_switch].PP_number % 30;
  uint16_t index = SP[Current_switch].PP_number;

  // Set the status
  SP[Current_switch].State = SCO_find_parameter_state(Current_switch, byte1);
  DEBUGMSG("###Switch: " + String(Current_switch) + " state: " + String(SP[Current_switch].State));

  // Set the colour
  uint8_t my_colour = VG99_parameters[index].Colour;

  //Check for special colours:
  switch (my_colour) {
    case VG99_FX_COLOUR:
      SP[Current_switch].Colour = VG99_FX_colours[byte2]; //MFX type read in byte2
      break;
    case VG99_FX_TYPE_COLOUR:
      SP[Current_switch].Colour = VG99_FX_colours[byte1]; //MFX type read in byte1
      break;
    case VG99_POLYFX_COLOUR:
      SP[Current_switch].Colour = VG99_polyFX_colours[byte2]; //MOD type read in byte2
      break;
    case VG99_POLYFX_TYPE_COLOUR:
      SP[Current_switch].Colour = VG99_polyFX_colours[byte1]; //MOD type read in byte1
      break;
    default:
      SP[Current_switch].Colour =  my_colour;
      break;
  }

  // Set the display message
  String msg = VG99_parameters[index].Name;
  if (VG99_parameters[index].Sublist > SUBLIST_FROM_BYTE2) { // Check if a sublist exists
    String type_name = VG99_sublists[VG99_parameters[index].Sublist - SUBLIST_FROM_BYTE2 + byte2 - 1];
    msg = msg + " (" + type_name + ")";
  }
  if ((VG99_parameters[index].Sublist > 0) && !(VG99_parameters[index].Sublist & SUBLIST_FROM_BYTE2)) {
    msg += ":";
    read_parameter_state(index, byte1, msg);
  }

  //Copy it to the display name:
  LCD_set_label(Current_switch, msg);
}

void VG99_class::check_update_label(uint8_t Sw, uint8_t value) { // Updates the label for extended sublists
  uint16_t index = SP[Sw].PP_number;
  if (index != NOT_FOUND) {

    if ((VG99_parameters[index].Sublist > 0) && !(VG99_parameters[index].Sublist & SUBLIST_FROM_BYTE2)) { // Check if state needs to be read
      LCD_clear_label(Sw);

      // Set the display message
      String msg = VG99_parameters[index].Name;
      msg += ":";
      read_parameter_state(index, value, msg);

      //Copy it to the display name:
      LCD_set_label(Sw, msg);

      //Update the current switch label
      update_lcd = Sw;
    }
  }
}

uint16_t VG99_class::number_of_parameters() {
  return VG99_NUMBER_OF_PARAMETERS - VG99_NON_PARAMETER_TARGETS;
}

uint8_t VG99_class::number_of_values(uint16_t parameter) {
  if (parameter < VG99_NUMBER_OF_PARAMETERS) {
    //uint8_t part = parameter / 30; // Split the parameter number in part and index
    //uint8_t index = parameter % 30;
    return VG99_parameters[parameter].NumVals;
  }
  else return 0;
}

// ********************************* Section 6: VG99 assign control ********************************************

// VG99 has 16 general assigns, which we can control with a cc-MIDI message.
// Also we can control 12 FC300 assigns, which need a special sysex message to function
// If you set the cc number to 1-12, the VController will control the FC300 CTL 1-8, EXP1, EXP SW1, EXP2 or EXP SW2 !!!
// GK S1/S2, GK VOL, EXP, CTL1, CTL2, CTL3, CTL4, DBEAM-V, DBEAM H, RIBBON ACT and RIBBON POS assigns can be read but not controlled from the VController!

// Procedures for VG99_ASSIGN:
// 1. Load in SP array - VG99_class::assign_load() below
// 2. Request - VG99_class::request_current_assign()
// 3. Read assign - VG99_class::read_current_assign() - also requests parameter state
// 4. Read parameter state - VG99_class::read_parameter() above
// 5. Press switch - VG99_class::assign_press() below
// 6. Release switch - VG99_class::assign_release() below

#define VG99_NUMBER_OF_ASSIGNS 39
const PROGMEM char VG99_assign_title[VG99_NUMBER_OF_ASSIGNS][14] = {
  "ASSIGN 1", "ASSIGN 2", "ASSIGN 3", "ASSIGN 4", "ASSIGN 5", "ASSIGN 6", "ASSIGN 7", "ASSIGN 8",
  "ASSIGN 9", "ASSIGN 10", "ASSIGN 11", "ASSIGN 12", "ASSIGN 13", "ASSIGN 14", "ASSIGN 15", "ASSIGN 16",
  "FC300 CTL1", "FC300 CTL2", "FC300 CTL3", "FC300 CTL4", "FC300 CTL5", "FC300 CTL6", "FC300 CTL7", "FC300 CTL8",
  "FC300 EXP1", "FC300 EXP SW1", "FC300 EXP2", "FC300 EXP SW2", "GK S1/S2", "GK VOL", "EXP",
  "CTL1", "CTL2", "CTL3", "CTL4", "DBEAM-V", "DBEAM-H", "RIBBON ACT", "RIBBON POS"
};

const PROGMEM uint32_t VG99_assign_address[VG99_NUMBER_OF_ASSIGNS] = {
  0x60007000, 0x6000701C, 0x60007038, 0x60007054, 0x60007100, 0x6000711C, 0x60007138, 0x60007154, // ASSIGN 1 - 8
  0x60007200, 0x6000721C, 0x60007238, 0x60007254, 0x60007300, 0x6000731C, 0x60007338, 0x60007354, // ASSIGN 9 - 16
  0x60000600, 0x60000614, 0x60000628, 0x6000063C, 0x60000650, 0x60000664, 0x60000678, 0x6000070C, // FC300 CTL ASSIGN 1 - 8
  0x60000500, 0x60000514, 0x60000528, 0x6000053C, 0x60000114, 0x60000100, 0x60000150, // FC300 EXP1, EXP SW1, EXP2, EXP SW2, GK S1/S2, GK VOL, EXP
  0x60000128, 0x6000013C, 0x60000164, 0x60000178, 0x60000300, 0x60000314, 0x60000328, 0x6000033C  // CTL1, CTL2, CTL3, CTL4, DBEAM-V, DBEAM H, RIBBON ACT, RIBBON POS
};

const PROGMEM uint16_t FC300_CTL[12] = {0x2100, 0x2101, 0x2402, 0x2102, 0x2403, 0x2103, 0x2404, 0x2104, 0x2400, 0x2200, 0x2401, 0x2201}; //CTL 1-8, EXP1, EP SW1, EXP2, EP SW2

//const PROGMEM char FC300_ASGN_NAME[12][8] = {"CTL1", "CTL2", "CTL3", "CTL4", "CTL5", "CTL6", "CTL7", "CTL8", "EXP1", "EXP SW1", "EXP2", "EXP SW2",};

void VG99_class::read_assign_name(uint8_t number, String & Output) {
  if (number < VG99_NUMBER_OF_ASSIGNS)  Output = VG99_assign_title[number];
  else Output = "?";
}

void VG99_class::read_assign_trigger(uint8_t number, String & Output) {
  if ((number > 0) && (number <= 8))  Output = "FC300 CTL" + String(number);
  else if (number == 9) Output = "FC300 EXP1";
  else if (number == 10) Output = "FC300 EXP SW1";
  else if (number == 11) Output = "FC300 EXP2";
  else if (number == 12) Output = "FC300 EXP SW2";
  else if ((number > 12) && (number < 128)) Output = "CC#" + String(number);
  else Output = "-";
}

uint8_t VG99_class::get_number_of_assigns() {
  return VG99_NUMBER_OF_ASSIGNS;
}

uint8_t VG99_class::trigger_follow_assign(uint8_t number) {
  if (number < 16) return number + 20; // Default cc numbers are 20 and up
  if ((number >= 16) && (number < 28)) return number - 15; // Return the trigger for the FC300 pedals
  return 0;
}

void VG99_class::assign_press(uint8_t Sw, uint8_t value) { // Switch set to VG99_ASSIGN is pressed

  // Send cc MIDI command to VG-99. If cc is 1 - 8, send the FC300 CTL sysex code
  uint8_t cc_number = SP[Sw].Trigger;
  if ((cc_number >= 1) && (cc_number <= 12)) write_sysexfc(FC300_CTL[cc_number - 1], value);
  else if (cc_number < 128) MIDI_send_CC(cc_number, value, MIDI_channel, MIDI_port);

  // Display the patch function
  if (SP[Sw].Assign_on) {
    uint8_t new_val = 0;
    if (SP[Sw].State == 1) new_val = (((SP[Sw].Assign_max - SP[Sw].Assign_min) * value) / 127) + SP[Sw].Assign_min;
    else new_val = SP[Sw].Assign_min;
    check_update_label(Sw, new_val);
  }
  LCD_show_status_message(SP[Sw].Label);

  if (SP[Sw].Assign_on) update_page |= REFRESH_PAGE; // To update the other switch states, we re-load the current page
}

void VG99_class::assign_release(uint8_t Sw) { // Switch set to VG99_ASSIGN is released

  // Send cc MIDI command to VG-99. If cc is 1 - 8, send the FC300 CTL sysex code
  uint8_t cc_number = SP[Sw].Trigger;
  if ((cc_number >= 1) && (cc_number <= 12)) write_sysexfc(FC300_CTL[cc_number - 1], 0);
  else MIDI_send_CC(cc_number, 0, MIDI_channel, MIDI_port);

  // Update status
  if (SP[Sw].Latch == MOMENTARY) {
    if (SP[Sw].Assign_on) SP[Sw].State = 2; // Switch state off
    else SP[Sw].State = 0; // Assign off, so LED should be off as well

    if (SP[Sw].Assign_on) update_page |= REFRESH_PAGE; // To update the other switch states, we re-load the current page
  }
}

void VG99_class::fix_reverse_pedals() {
  // Pedal 3,5 and 7 are reversed. The VG99 also ignores the first press on them
  // But by operating them once the VG99 will respond correctly form here-on.
  write_sysexfc(FC300_CTL[3], 0x7F); // Press CTL-3
  write_sysexfc(FC300_CTL[3], 0x00); // Release CTL-3
  write_sysexfc(FC300_CTL[5], 0x7F); // Press CTL-5
  write_sysexfc(FC300_CTL[5], 0x00); // Release CTL-5
  write_sysexfc(FC300_CTL[7], 0x7F); // Press CTL-7
  write_sysexfc(FC300_CTL[7], 0x00); // Release CTL-7
}

void VG99_class::assign_load(uint8_t sw, uint8_t assign_number, uint8_t my_trigger) { // Switch set to VG99_ASSIGN is loaded in SP array
  SP[sw].Trigger = my_trigger; //Save the cc_number / FC300 pedal number in the Trigger variable
  SP[sw].Assign_number = assign_number;
}

void VG99_class::request_current_assign() {
  uint8_t index = SP[Current_switch].Assign_number - 1;  //index should be between 0 and 7
  SP[Current_switch].Address = VG99_assign_address[index];
  if (index < VG99_NUMBER_OF_ASSIGNS) {
    DEBUGMSG("Request assign " + String(index + 1));
    //read_assign_target = false;
    uint32_t my_address = VG99_assign_address[index];
    last_requested_sysex_address = my_address;
    last_requested_sysex_type = ASSIGN_TYPE;
    request_sysex(my_address, 14);  //Request 14 bytes for the VG99 assign
  }
  else PAGE_request_next_switch(); // Wrong assign number given in Config - skip it
}

void VG99_class::read_current_assign(uint32_t address, const unsigned char* sxdata, short unsigned int sxlength) {
  bool assign_on, found;
  String msg;
  uint8_t assign_switch = sxdata[11];
  uint16_t assign_target = (sxdata[12] << 8) + sxdata[13];
  uint16_t assign_target_min = (sxdata[14] << 8) + sxdata[15];
  uint16_t assign_target_max = (sxdata[16] << 8) + sxdata[17];
  uint8_t assign_latch = sxdata[18];
  uint8_t assign_source = sxdata[24]; // As sxdata[23] is always 0, we will not bother reading it.

  // Check for valid assign. We have three options
  // 1) CTL assign with FC300 CTL as source
  // 2) CTL assign with cc09 - cc31 or cc64 - cc95 as source
  // 3) FC300 CTL1-8 assign with fixed source
  uint8_t my_trigger = SP[Current_switch].Trigger;
  if ((my_trigger >= 1) && (my_trigger <= 8)) my_trigger = my_trigger + 14; // Trigger if FC300 CTRL 1 - 8. Add 14 to match the VG99 implemntation of these sources (0x0F - 0x16)
  else if ((my_trigger >= 9) && (my_trigger <= 12)) my_trigger = my_trigger + 2; // Trigger for EXP1, EXP SW1, EP2, EP SW2
  else if ((my_trigger >= 9) && (my_trigger <= 31)) my_trigger = my_trigger + 24; // Trigger is cc09 - cc31 Add 24 to match the VG99 implemntation of these sources (0x19 - 0x37)
  else if ((my_trigger >= 64) && (my_trigger <= 95)) my_trigger = my_trigger - 8; // Trigger is cc64 - cc95 Add 24 to match the VG99 implemntation of these sources (0x38 - 0x57)

  if (SP[Current_switch].Assign_number <= 16) assign_on = ((assign_switch == 0x01) && (my_trigger == assign_source)); // Check if assign is on by checking assign switch and source
  else assign_on = (assign_switch == 0x01); // Assign is FC300 CTL1-8 type, so we do not need to check the source

  //DEBUGMSG("VG-99 Assign_switch: 0x" + String(assign_switch, HEX));
  //DEBUGMSG("VG-99 Assign_target 0x:" + String(assign_target, HEX));
  //DEBUGMSG("VG-99 Assign_min: 0x" + String(assign_target_min, HEX));
  //DEBUGMSG("VG-99 Assign_max: 0x" + String(assign_target_max, HEX));
  //DEBUGMSG("VG-99 Assign_source: 0x" + String(assign_source, HEX));
  //DEBUGMSG("VG-99 Assign_latch: 0x" + String(assign_latch, HEX));
  //DEBUGMSG("VG-99 Assign_trigger-check:" + String(my_trigger) + "==" + String(assign_source));

  if (assign_on) {
    SP[Current_switch].Assign_on = true; // Switch the pedal off
    SP[Current_switch].Latch = assign_latch;

    // Allow for VG-99 assign min and max swap. Is neccesary, because the VG-99 will not save a parameter in the on-state, unless you swap the assign min and max values
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

    // Request the target - on the VG99 target and the address of the target are directly related
    SP[Current_switch].Address = 0x60000000 + assign_target;

    // We have to check the table to find the location of the target there
    found = target_lookup(assign_target); // Lookup the address of the target in the VG99_Parameters array

    DEBUGMSG("Request target of assign " + String(SP[Current_switch].Assign_number) + ": " + String(SP[Current_switch].Address, HEX));
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
      if (SP[Current_switch].Assign_number <= 16) msg = "CC#" + String(SP[Current_switch].Trigger) + " (ASGN" + String(SP[Current_switch].Assign_number) + ")";
      else msg = VG99_assign_title[SP[Current_switch].Assign_number];
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
    if (SP[Current_switch].Assign_number <= 16) msg = "CC#" + String(SP[Current_switch].Trigger);
    else msg = "--"; //"FC300 ASGN" + String(SP[Current_switch].Assign_number - 16);
    LCD_set_label(Current_switch, msg);
    PAGE_request_next_switch();
  }
}

bool VG99_class::target_lookup(uint16_t target) {  // Finds the target and its address in the VG99_parameters table

  // Lookup in VG99_parameter array
  //uint8_t part = (target / 0x1000); // As the array is divided in addresses by 1000, it is easy to find the right part
  bool found = false;
  for (uint16_t i = 0; i < VG99_NUMBER_OF_PARAMETERS; i++) {
    if (VG99_parameters[i].Address == 0) break; //Break the loop if there is no more useful data
    if (target == VG99_parameters[i].Address) { //Check is we've found the right target
      SP[Current_switch].PP_number = i; // Save the index number
      found = true;
    }
  }
  return found;
}



