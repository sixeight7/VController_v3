// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: GP10 Initialization
// Section 2: GP10 common MIDI in functions
// Section 3: GP10 common MIDI out functions
// Section 4: GP10 program change
// Section 5: GP10 parameter control
// Section 6: GP10 assign control

// ********************************* Section 1: GP10 Initialization ********************************************

//Messages are abbreviated to just the address and the data bytes. Checksum is calculated automatically
//Example: {0xF0, 0x41, 0x10, 0x00, 0x00, 0x00, 0x05, 0x12, 0x7F, 0x00, 0x00, 0x01, 0x01, 0x7F, 0xF7} is reduced to 0x7F000001, 0x01

#define GP10_EDITOR_MODE_ON 0x7F000001, 0x01 //Gets the GP-10 spitting out lots of sysex data. Should be switched on, otherwise the tuner does not work
#define GP10_EDITOR_MODE_OFF 0x7F000001, 0x00
#define GP10_REQUEST_CURRENT_PATCH_NAME 0x20000000, 12 //Request 12 bytes for current patch name
#define GP10_REQUEST_PATCH_NUMBER 0x00000000, 1 //Request current patch number

#define GP10_TUNER_ON 0x7F000002, 0x02 // Changes the running mode of the GP-10 to Tuner - Got these terms from the VG-99 sysex manual.
#define GP10_TUNER_OFF 0x7F000002, 0x00 //Changes the running mode of the GP10 to play.
#define GP10_SOLO_ON 0x2000500B, 0x01
#define GP10_SOLO_OFF 0x2000500B, 0x00

#define GP10_TEMPO 0x20000801  // Accepts values from 40 bpm - 250 bpm
#define GP10_HARMONIST_KEY 0x20014001 // Sets the key for the harmonist, which is the only effect that is key related.

#define GP10_FOOT_VOL 0x20020803 // The address of the footvolume - values between 0 and 100
#define GP10_COSM_GUITAR_SW 0x20001000 // The address of the COSM guitar switch
#define GP10_NORMAL_PU_SW 0x20000804 // The address of the COSM guitar switch
//bool request_onoff = false;

// Initialize device variables
// Called at startup of VController
void GP10_class::init() { // Default values for variables

  // Boss GP-10 variables:
  strcpy(device_name, "GP10");
  strcpy(full_device_name, "Boss GP-10");
  current_patch_name.reserve(17);
  current_patch_name = "                ";
  patch_min = GP10_PATCH_MIN;
  patch_max = GP10_PATCH_MAX;
  bank_size = 10;
  max_times_no_response = MAX_TIMES_NO_RESPONSE; // The number of times the GP-10 does not have to respond before disconnection
  COSM_onoff = 0;
  nrml_pu_onoff = 0;
  SYSEX_DELAY_LENGTH = 0; // time between sysex messages (in msec).
  my_LED_colour = 4; // Default value: orange
  MIDI_channel = GP10_MIDI_CHANNEL; // Default value
  bank_number = 0; // Default value
  is_always_on = true; // Default value
  my_patch_page = PAGE_CURRENT_PATCH_BANK; // Default value
  my_parameter_page = PAGE_CURRENT_PARAMETER; // Default value
  my_assign_page = PAGE_GP10_ASSIGNS; // Default value
}

// ********************************* Section 2: GP10 common MIDI in functions ********************************************

void GP10_class::check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port) { // Check incoming sysex messages from  Called from MIDI:OnSysEx/OnSerialSysEx

  // Check if it is a message from a GP-10
  if ((port == MIDI_port) && (sxdata[1] == 0x41) && (sxdata[2] == MIDI_device_id) && (sxdata[3] == 0x00) && (sxdata[4] == 0x00) && (sxdata[5] == 0x00) && (sxdata[6] == 0x05) && (sxdata[7] == 0x12)) {
    uint32_t address = (sxdata[8] << 24) + (sxdata[9] << 16) + (sxdata[10] << 8) + sxdata[11]; // Make the address 32 bit

    // Check if it is the patch number
    if (address == 0x00000000) {
      if (patch_number != sxdata[12]) { //Right after a patch change the patch number is sent again. So here we catch that message.
        patch_number = sxdata[12];
        //page_check();
        do_after_patch_selection();
        update_page |= REFRESH_PAGE;
      }
    }

    // Check if it is the current parameter
    if (address == last_requested_sysex_address) {
      switch (last_requested_sysex_type) {
        case PATCH_NAME:
          if (sxlength == 26) {
            for (uint8_t count = 0; count < 12; count++) {
              SP[Current_switch].Label[count] = static_cast<char>(sxdata[count + 12]); //Add ascii character to the SP.Label String
            }
            for (uint8_t count = 12; count < 16; count++) {
              SP[Current_switch].Label[count] = ' '; //Fill the rest of the label with spaces
            }

            if (SP[Current_switch].PP_number == patch_number) {
              current_patch_name = SP[Current_switch].Label; // Load patchname when it is read
              update_main_lcd = true; // And show it on the main LCD
            }
            DEBUGMSG(SP[Current_switch].Label);
            PAGE_request_next_switch();
          }
          else {
            PAGE_request_current_switch();
          }
          break;

        case PARAMETER_TYPE:
        case ASSIGN_TYPE:
          //case GP10_ASSIGN:
          read_parameter(sxdata[12], sxdata[13]);
          PAGE_request_next_switch();
          break;
      }
    }

    // Check if it is the patch name (address: 0x20, 0x00, 0x00, 0x00)
    if (address == 0x20000000) {
      current_patch_name = "";
      for (uint8_t count = 12; count < 24; count++) {
        current_patch_name = current_patch_name + static_cast<char>(sxdata[count]); //Add ascii character to Patch Name String
      }
      current_patch_name = current_patch_name + "    ";
      update_main_lcd = true;
    }

    // Read GP10 assign area
    read_complete_assign_area(address, sxdata, sxlength);

    // Check if it is the guitar on/off states
    check_guitar_switch_states(sxdata, sxlength);

    // Check if it is some other stompbox function and copy the status to the right LED
    //GP10_check_stompbox_states(sxdata, sxlength);
  }
}

void GP10_class::check_PC_in(uint8_t program, uint8_t channel, uint8_t port) {  // Check incoming PC messages from  Called from MIDI:OnProgramChange

  // Check the source by checking the channel
  if ((port == MIDI_port) && (channel == MIDI_channel)) { // GP10 outputs a program change
    if (patch_number != program) {
      patch_number = program;
      request_sysex(GP10_REQUEST_CURRENT_PATCH_NAME); // So the main display always show the correct patch
      //page_check();
      do_after_patch_selection();
      update_page |= REFRESH_PAGE;
    }
  }
}


// Detection of GP-10

void GP10_class::identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port) {
  // Check if it is a GP-10
  if ((sxdata[5] == 0x41) && (sxdata[6] == 0x05) && (sxdata[7] == 0x03)) {
    no_response_counter = 0;
    if (connected == false) connect(sxdata[2], port); //Byte 2 contains the correct device ID
  }
}

void GP10_class::do_after_connect() {
  request_sysex(GP10_REQUEST_PATCH_NUMBER);
  request_sysex(GP10_REQUEST_CURRENT_PATCH_NAME); // So the main display always show the correct patch
  //write_sysex(GP10_EDITOR_MODE_ON); // Put the GP10 in EDITOR mode - otherwise tuner will not work
  do_after_patch_selection();
  update_page |= REFRESH_PAGE;
  assign_read = false; // Assigns should be read again
}


// ********************************* Section 3: GP10 common MIDI out functions ********************************************

void GP10_class::write_sysex(uint32_t address, uint8_t value) { // For sending one data byte

  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t checksum = calc_Roland_checksum(ad[3] + ad[2] + ad[1] + ad[0] + value); // Calculate the Roland checksum
  uint8_t sysexmessage[15] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x00, 0x05, 0x12, ad[3], ad[2], ad[1], ad[0], value, checksum, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 15, MIDI_port);
}

void GP10_class::write_sysex(uint32_t address, uint8_t value1, uint8_t value2) { // For sending two data bytes

  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t checksum = calc_Roland_checksum(ad[3] + ad[2] + ad[1] + ad[0] + value1 + value2); // Calculate the Roland checksum
  uint8_t sysexmessage[16] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x00, 0x05, 0x12, ad[3], ad[2], ad[1], ad[0], value1, value2, checksum, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 16, MIDI_port);
}

void GP10_class::request_sysex(uint32_t address, uint8_t no_of_bytes) {
  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t no1 = no_of_bytes / 128;
  uint8_t no2 = no_of_bytes % 128;
  uint8_t checksum = calc_Roland_checksum(ad[3] + ad[2] + ad[1] + ad[0] +  no1 + no2); // Calculate the Roland checksum
  uint8_t sysexmessage[18] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x00, 0x05, 0x11, ad[3], ad[2], ad[1], ad[0], 0x00, 0x00, no1, no2, checksum, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 18, MIDI_port);
}

void GP10_class::set_bpm() {
  if (connected) {
    write_sysex(GP10_TEMPO, Setting.Bpm / 16, Setting.Bpm % 16); // Tempo is modulus 16. It's all so very logical. NOT.
  }
}

void GP10_class::start_tuner() {
  if (connected) {
    write_sysex(GP10_EDITOR_MODE_ON); // Put the GP10 in EDITOR mode - otherwise tuner will not work
    write_sysex(GP10_TUNER_ON); // Start tuner on GP-10
  }
}

void GP10_class::stop_tuner() {
  if (connected) {
    write_sysex(GP10_TUNER_OFF); // Stop tuner on GP-10
    write_sysex(GP10_EDITOR_MODE_OFF); // Switch off EDITOR mode - so the GP10 will respond faster
  }
}

// ********************************* Section 4: GP10 program change ********************************************

void GP10_class::do_after_patch_selection() {
  request_onoff = false;
  is_on = connected;
  if (Setting.Send_global_tempo_after_patch_change == true) {
    delay(5); // GP10 misses send bpm command...
    set_bpm();
  }
  Current_patch_number = patch_number; // the patch name
  update_LEDS = true;
  update_main_lcd = true;
  //update_page |= REFRESH_PAGE;
  assign_read = false; // Assigns should be read again
  request_guitar_switch_states();
  //EEPROM.write(EEPROM_GP10_PATCH_NUMBER, patch_number);
  if (!PAGE_check_on_page(my_device_number, patch_number)) { // Check if patch is on the page
    update_page |= REFRESH_PAGE;
  }
  else {
    update_page = REFRESH_FX_ONLY;
  }
}

void GP10_class::request_patch_name(uint16_t number) {
  number++;
  uint32_t Address = 0x20000000 + ((number / 0x20) * 0x1000000) + ((number % 0x20) * 0x40000); //Calculate the address where the patchname is stored on the GP-10
  last_requested_sysex_address = Address;
  last_requested_sysex_type = PATCH_NAME;
  request_sysex(Address, 12); //Request the 12 bytes of the GP10 patchname
}

void GP10_class::request_current_patch_name() {
  request_sysex(GP10_REQUEST_CURRENT_PATCH_NAME);
}

void GP10_class::display_patch_number_string() {
  // Uses patch_number as input and returns Current_patch_number_string as output in format "P01"
  if (bank_selection_active() == false) {
    number_format(patch_number, Current_patch_number_string);
  }
  else {
    //Current_patch_number_string = "P" + String(bank_number) + "-";
    String start_number, end_number;
    number_format(bank_select_number * bank_size, start_number);
    number_format((bank_select_number + 1) * bank_size - 1, end_number);
    Current_patch_number_string = Current_patch_number_string + start_number + "-" + end_number;
  }
}

void GP10_class::number_format(uint16_t number, String &Output) {
  Output = Output + "P" + String((number + 1) / 10) + String((number + 1) % 10);
}

void GP10_class::direct_select_format(uint16_t number, String &Output) {
  if (direct_select_state == 0) Output = Output + "P" + String(number) + "_";
  else Output = Output + "P" + String(direct_select_first_digit) + String(number);
}

// ** US-20 simulation
// Selecting and muting the GP10 is done by storing the settings of COSM guitar switch and Normal PU switch
// and switching both off when guitar is muted and back to original state when the GP10 is selected

void GP10_class::request_guitar_switch_states() {
  //GP10_select_LED = GP10_PATCH_COLOUR; //Switch the LED on
  request_sysex(GP10_COSM_GUITAR_SW, 1);
  request_sysex(GP10_NORMAL_PU_SW, 1);
  request_onoff = true;
  //no_device_check = true;
}

void GP10_class::check_guitar_switch_states(const unsigned char* sxdata, short unsigned int sxlength) {
  if (request_onoff == true) {
    uint32_t address = (sxdata[8] << 24) + (sxdata[9] << 16) + (sxdata[10] << 8) + sxdata[11]; // Make the address 32 bit
    if (address == GP10_COSM_GUITAR_SW) {
      COSM_onoff = sxdata[12];  // Store the value
    }

    if (address == GP10_NORMAL_PU_SW) {
      nrml_pu_onoff = sxdata[12];  // Store the value
      request_onoff = false;
      //no_device_check = false;
    }
  }
}

void GP10_class::unmute() {
  is_on = connected;
  //GP10_select_LED = GP10_PATCH_COLOUR; //Switch the LED on
  //write_sysex(GP10_FOOT_VOL, 100); // Switching guitars does not work - the wrong values are read from the GP-10. ?????
  write_sysex(GP10_COSM_GUITAR_SW, COSM_onoff); // Switch COSM guitar on
  write_sysex(GP10_NORMAL_PU_SW, nrml_pu_onoff); // Switch normal pu on
}

void GP10_class::mute() {
  if ((Setting.US20_emulation_active) && (!is_always_on) && (is_on)) {
    is_on = false;
    //    GP10_select_LED = GP10_OFF_COLOUR; //Switch the LED off
    //write_sysex(GP10_FOOT_VOL, 0);
    write_sysex(GP10_COSM_GUITAR_SW, 0x00); // Switch COSM guitar off
    write_sysex(GP10_NORMAL_PU_SW, 0x00); // Switch normal pu off
  }
}

// ********************************* Section 5: GP10 parameter control ********************************************

// Procedures for the GP10_PARAMETER:
// 1. Load in SP array L load_page()
// 2. Request parameter state - in PAGE_request_current_switch()
// 3. Read parameter state - GP10_read_parameter() below
// 4. Press switch - GP10_parameter_press() below - also calls check_update_label()
// 5. Release switch - GP10_parameter_release() below - also calls check_update_label()

struct GP10_parameter_struct { // Combines all the data we need for controlling a parameter in a device
  uint16_t Target; // Target of the assign as given in the assignments of the GP10 / GR55
  uint32_t Address; // The address of the parameter
  uint8_t NumVals; // The number of values for this parameter
  char Name[17]; // The name for the label
  uint16_t Sublist; // Which sublist to read for the FX or amp type - 0 if second byte does not contain the type or if there is no sublist +100 Show value from sublist.
  uint8_t Colour; // The colour for this effect.
};

//typedef struct stomper Stomper;
#define GP10_FX_COLOUR 255 // Just a colour number to pick the colour from the GP10_FX_colours table
#define GP10_FX_TYPE_COLOUR 254 //Another number for the FX type
#define SUBLIST_FROM_BYTE2 32768 //Bit 16 set high means read sublist frm byte 2
#define SHOW_NUMBER 31767 // Special number for showing the number (used for gain/volume/etc) - set in sublist
#define SHOW_DOUBLE_NUMBER 31766 // Show number times 2 (used for patch volume/etc) - set in sublist
#define SHOW_PAN 31765 // Special number for showing the pan- set in sublist

// Make sure you edit the GP10_NUMBER_OF_STOMPS when adding new parameters
//#define GP10_NUMBER_OF_PARAMETERS 19

const PROGMEM GP10_parameter_struct GP10_parameters[] = {
  {0xFFF, 0x20000800, 101, "PATCH LVL", SHOW_DOUBLE_NUMBER, FX_DEFAULT_TYPE},
  {0x0F1, 0x20005800, 2, "FX", 1 | SUBLIST_FROM_BYTE2, GP10_FX_COLOUR},
  {0x0F2, 0x20005801, 16, "FX Type", 1, GP10_FX_TYPE_COLOUR},
  {0x166, 0x20016000, 2, "WAH SW", 180 | SUBLIST_FROM_BYTE2, FX_FILTER_TYPE},
  {0x167, 0x20016001, 6, "WAH TP", 180, FX_FILTER_TYPE},
  {0x168, 0x20016002, 101, "WAH POS", SHOW_NUMBER, FX_FILTER_TYPE},
  {0x16D, 0x20016800, 2, "Chorus", 17 | SUBLIST_FROM_BYTE2, FX_MODULATE_TYPE},
  {0x161, 0x20016801, 3, "CHS Type", 17, FX_MODULATE_TYPE},
  {0x176, 0x20017000, 2, "DLY", 20 | SUBLIST_FROM_BYTE2, FX_DELAY_TYPE},
  {0x177, 0x20017001, 10, "DLY TYPE", 20, FX_DELAY_TYPE},
  {0x188, 0x20017800, 2, "RVB", 30 | SUBLIST_FROM_BYTE2, FX_REVERB_TYPE},
  {0x189, 0x20017801, 7, "RVB TYPE", 30, FX_REVERB_TYPE},
  {0x192, 0x20020000, 2, "EQ SW", 0, FX_FILTER_TYPE},
  {0x000, 0x20001000, 2, "COSM GUITAR", 0, FX_GTR_TYPE},
  {0x001, 0x20001001, 4, "COSM Type", 115, FX_GTR_TYPE},
  {0x0A5, 0x20000804, 2, "NORMAL PU", 0, FX_GTR_TYPE},
  {0x002, 0x20001800, 12, "E.GTR TP", 119, FX_GTR_TYPE},
  {0x003, 0x20001801, 101, "E.GTR LVL", SHOW_NUMBER, FX_GTR_TYPE},
  {0xFFF, 0x20002001, 101, "A.GTR LVL", SHOW_NUMBER, FX_GTR_TYPE},
  {0xFFF, 0x20002801, 101, "BASS LVL", SHOW_NUMBER, FX_GTR_TYPE},
  {0xFFF, 0x20003001, 101, "SYNTH LVL", SHOW_NUMBER, FX_GTR_TYPE},
  {0xFFF, 0x20003801, 101, "POLYFX LVL", SHOW_NUMBER, FX_GTR_TYPE},
  {0x0CF, 0x20001002, 2, "COSM NS SW", 0, FX_GTR_TYPE},
  {0x09F, 0x20004000, 2, "TUN SW", 143  | SUBLIST_FROM_BYTE2, FX_PITCH_TYPE},
  {0x0A0, 0x20004001, 37, "TUNING", 143, FX_PITCH_TYPE},
  {0x0AD, 0x2000400E, 2, "12 STRING SW", 0, FX_PITCH_TYPE},
  {0x0DE, 0x20005000, 2, "Amp SW", 37 | SUBLIST_FROM_BYTE2, FX_AMP_TYPE},
  {0x0DF, 0x20005001, 30, "Amp Type", 37, FX_AMP_TYPE},
  {0x0E0, 0x20005002, 101, "Amp Gain", SHOW_NUMBER, FX_AMP_TYPE},
  {0x0E2, 0x20005004, 101, "Amp LVL", SHOW_NUMBER, FX_AMP_TYPE},
  {0x0E8, 0x2000500B, 2, "Amp solo", 0, FX_AMP_TYPE},
  {0x0EC, 0x2000500A, 3, "Amp Gain SW", 131, FX_AMP_TYPE},
  {0xFFF, 0x20005028, 2, "Bend SW", 131, FX_AMP_TYPE},
  {0xFFF, 0x2000500E, 9, "Amp SP TP", 134, FX_AMP_TYPE},
  {0xFFF, 0x20020803, 101, "Foot Vol", SHOW_NUMBER, FX_AMP_TYPE},
  {0xFFF, 0x20021800, 18, "CTL1", 67, FX_DEFAULT_TYPE},
  {0xFFF, 0x20021802, 18, "CTL2", 67, FX_DEFAULT_TYPE},
  {0xFFF, 0x20021804, 17, "CTL3", 67, FX_DEFAULT_TYPE},
  {0xFFF, 0x20021806, 17, "CTL4", 67, FX_DEFAULT_TYPE},
  {0xFFF, 0x20021808, 17, "GK S1", 67, FX_DEFAULT_TYPE},
  {0xFFF, 0x2002180A, 17, "GK S2", 67, FX_DEFAULT_TYPE},
  {0xFFF, 0x2002180C, 15, "EXP SW", 85, FX_DEFAULT_TYPE},
  {0xFFF, 0x2002180E, 15, "EXP", 100, FX_DEFAULT_TYPE},
  {0xFFF, 0x2002180F, 15, "EXP ON", 100, FX_DEFAULT_TYPE},
  {0xFFF, 0x20021810, 15, "EXP2", 100, FX_DEFAULT_TYPE},
  {0xFFF, 0x20021811, 15, "GK VOL", 100, FX_DEFAULT_TYPE},
  {0xFFF, 0x10001000, 2, "Guitar2MIDI", 0, FX_DEFAULT_TYPE}, //Can not be controlled from assignment, but can be from GP10_PARAMETER!!!
};

const uint16_t GP10_NUMBER_OF_PARAMETERS = sizeof(GP10_parameters) / sizeof(GP10_parameters[0]);

//#define GP10_SIZE_OF_SUBLIST 66
const PROGMEM char GP10_sublists[][9] = {
  // Sublist 1 - 16: FX types
  "OD/DS", "COMPRSR", "LIMITER", "EQ", "T.WAH", "P.SHIFT", "HARMO", "P. BEND", "PHASER", "FLANGER", "TREMOLO", "PAN", "ROTARY", "UNI-V", "CHORUS", "DELAY",

  // Sublist 17 - 19: Chorus types
  "MONO", "ST1", "ST2",

  // Sublist 20 - 29: Delay types
  "SINGLE", "PAN", "STEREO", "DUAL-S", "DUAL-P", "DU L/R", "REVRSE", "ANALOG", "TAPE", "MODLTE",

  // Sublist 30 - 36: Reverb types
  "AMBNCE", "ROOM", "HALL1", "HALL2", "PLATE", "SPRING", "MODLTE",

  // Sublist 37 - 66 : Amp types
  "NAT CLN", "FULL RG", "COMBO C", "STACK C", "HiGAIN", "POWER D", "EXTREME", "CORE MT", "JC-120", "CL TWIN",
  "PRO CR", "TWEED", "DELUXE", "VO DRVE", "VO LEAD", "MTCH DR", "BG LEAD", "BG DRVE", "MS1959I", "M1959II",
  "RFIER V", "RFIER M", "T-AMP L", "SLDN", "5150 DR", "BGNR UB", "ORNG RR", "BASS CL", "BASS CR", "BASS Hi",

  // Sublist 67 - 84: CTL1-4 + GK assign function:
  "OFF", "PUSEL UP", "PUSEL DN", "12-STRNG", "ALT TN", "STR BEND", "MODELING", "N PU SW", "AMP SOLO", "FX SW", "EQ SW", "WAH SW", "CHOR SW", "DLY SW", "RVRB", "OSC HOLD", "TAPTEMPO", "LED TGL",

  // Sublist 85 - 99: CTL1-4 + GK assign function:
  "OFF", "12-STRNG", "ALT TN", "STR BEND", "MODELING", "N PU SW", "AMP SOLO", "FX SW", "EQ SW", "WAH SW", "CHOR SW", "DLY SW", "RVRB", "OSC HOLD", "LED TGL",

  // Sublist 100 - 114: EXP assign function
  "OFF", "FOOT VOL", "PTCH LVL", "MLNG VOL", "N PU VOL", "MIXER", "STG BEND", "MODELING", "FX", "AMP", "WAH", "CHORUS", "DELAY", "REVERB", "EQ",

  // Sublist 115 - 118: COSM types
  "E.GTR", "ACOUSTIC", "E.BASS", "SYNTH",

  // Sublist 119 - 130: E.GTR types
  "CLA ST", "MOD ST", "TE", "LP", "P90", "335", "L4", "RICK", "LIPS", "WIDE RNG", "BRI HUM", "FRETLESS",

  // Sublist 131 - 133: Amp Gain switch types
  "LOW", "MID", "HIGH",

  // Sublist 134 - 142: Amp speaker types
  "OFF", "ORIG", "1x8", "1x10", "1x12", "2x12", "4x10", "4x12", "8x12",

  // Sublist 143 - 179: Alternative tuning types
  "OPEN-D", "OPEN-E", "OPEN-G", "OPEN-A", "DROP-D", "DROP-Db", "DROP-C", "DROP-B", "DROP-Bb", "DROP-A", "D-MODAL", "NASHVL", "-12 STEP", "-11 STEP", "-10 STEP", "-9 STEP", "-8 STEP", "-7 STEP",
  "-6 STEP", "-5 STEP", "-4 STEP", "-3 STEP", "-2 STEP", "-1 STEP", "+1 STEP", "+2 STEP", "+3 STEP", "+4 STEP", "+5 STEP", "+6 STEP", "+7 STEP", "+8 STEP", "+9 STEP", "+10 STEP", "+11 STEP", "+12 STEP", "USER",

  // Sublist 180 - 185: Amp Gain switch types
  "CRY", "VO", "FAT", "LIGHT", "7-STRING", "RESO",

};

const uint16_t GP10_SIZE_OF_SUBLIST = sizeof(GP10_sublists) / sizeof(GP10_sublists[0]);

const uint8_t GP10_FX_colours[17] = { // Table with the LED colours for the different FX states
  FX_DIST_TYPE, // Colour for "OD/DS"
  FX_FILTER_TYPE, // Colour for "COMPRSR"
  FX_FILTER_TYPE, // Colour for "LIMITER"
  FX_FILTER_TYPE, // Colour for "EQ"
  FX_FILTER_TYPE, // Colour for "T.WAH"
  FX_PITCH_TYPE, // Colour for "P.SHIFT"
  FX_PITCH_TYPE, // Colour for  "HARMO"
  FX_PITCH_TYPE, // Colour for "P. BEND"
  FX_MODULATE_TYPE, // Colour for "PHASER"
  FX_MODULATE_TYPE, // Colour for "FLANGER"
  FX_MODULATE_TYPE, // Colour for "TREMOLO"
  FX_MODULATE_TYPE, // Colour for "PAN"
  FX_MODULATE_TYPE, // Colour for  "ROTARY"
  FX_MODULATE_TYPE, // Colour for "UNI-V"
  FX_MODULATE_TYPE, // Colour for "CHORUS"
  FX_DELAY_TYPE, // Colour for "DELAY"
};

void GP10_class::read_parameter_name(uint16_t number, String &Output) { // Called from menu
  if (number < number_of_parameters())  Output = GP10_parameters[number].Name;
  else Output = "?";
}

void GP10_class::read_parameter_state(uint16_t number, uint8_t value, String &Output) {
  if (number < number_of_parameters())  {
    uint16_t my_sublist = GP10_parameters[number].Sublist;
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
        default:
          String type_name = GP10_sublists[my_sublist + value - 1];
          Output += type_name;
          break;
      }
    }
    else if (value == 1) Output += "ON";
    else Output += "OFF";
  }
  else Output += "?";
}

// Toggle GP10 stompbox parameter
void GP10_class::parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {

  // Send sysex MIDI command to GP-10
  uint8_t value = SCO_return_parameter_value(Sw, cmd);
  if (SP[Sw].Latch != TGL_OFF) {
    write_sysex(GP10_parameters[number].Address, value);

    // Show message
    check_update_label(Sw, value);
    LCD_show_status_message(SP[Sw].Label);

    //PAGE_load_current(false); // To update the other parameter states, we re-load the current page
    if (SP[Sw].Latch != UPDOWN) update_page |= REFRESH_PAGE;
  }
}

void GP10_class::parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {
  // Work out state of pedal
  if (SP[Sw].Latch == MOMENTARY) {
    //SP[Sw].State = 2; // Switch state off
    write_sysex(GP10_parameters[number].Address, cmd->Value1);

    //PAGE_load_current(false); // To update the other switch states, we re-load the current page
    update_page |= REFRESH_PAGE;
  }
}

bool GP10_class::request_parameter(uint16_t number) {
  uint32_t my_address = GP10_parameters[number].Address;
  last_requested_sysex_address = my_address;
  last_requested_sysex_type = PARAMETER_TYPE;
  request_sysex(my_address, 2); // Request the parameter state data
  return false; // We have not read the parameter yet
}

void GP10_class::read_parameter(uint8_t byte1, uint8_t byte2) { //Read the current GP10 parameter
  SP[Current_switch].Target_byte1 = byte1;
  SP[Current_switch].Target_byte2 = byte2;

  // Set the status
  SP[Current_switch].State = SCO_find_parameter_state(Current_switch, byte1);

  // Set the colour
  uint16_t index = SP[Current_switch].PP_number; // Read the parameter number (index to GP10-parameter array)
  uint8_t my_colour = GP10_parameters[index].Colour;

  //Check for special colours:
  if (my_colour == GP10_FX_COLOUR) SP[Current_switch].Colour = GP10_FX_colours[byte2]; //FX type read in byte2
  else if (my_colour == GP10_FX_TYPE_COLOUR) SP[Current_switch].Colour = GP10_FX_colours[byte1]; //FX type read in byte1
  else SP[Current_switch].Colour =  my_colour;

  // Set the display message
  String msg = GP10_parameters[index].Name;
  if (GP10_parameters[index].Sublist > SUBLIST_FROM_BYTE2) { // Check if a sublist exists
    String type_name = GP10_sublists[GP10_parameters[index].Sublist - SUBLIST_FROM_BYTE2 + byte2 - 1];
    msg = msg + " (" + type_name + ")";
  }
  if ((GP10_parameters[index].Sublist > 0) && !(GP10_parameters[index].Sublist & SUBLIST_FROM_BYTE2)) { // Check if state needs to be read
    //String type_name = GP10_sublists[GP10_parameters[index].Sublist + byte1 - 101];
    msg += ": ";
    read_parameter_state(index, byte1, msg);
  }
  //Copy it to the display name:
  LCD_set_label(Current_switch, msg);
}

void GP10_class::check_update_label(uint8_t Sw, uint8_t value) { // Updates the label for extended sublists
  uint16_t index = SP[Sw].PP_number; // Read the parameter number (index to GP10-parameter array)
  if (index != NOT_FOUND) {
    if ((GP10_parameters[index].Sublist > 0) && !(GP10_parameters[index].Sublist & SUBLIST_FROM_BYTE2)) { // Check if state needs to be read
      LCD_clear_label(Sw);
      // Set the display message
      String msg = GP10_parameters[index].Name;
      msg += ":";
      read_parameter_state(index, value, msg);

      //Copy it to the display name:
      LCD_set_label(Sw, msg);

      //Update the current switch label
      update_lcd = Sw;
    }
  }
}

uint16_t GP10_class::number_of_parameters() {
  return GP10_NUMBER_OF_PARAMETERS;
}

uint8_t GP10_class::number_of_values(uint16_t parameter) {
  if (parameter < GP10_NUMBER_OF_PARAMETERS) return GP10_parameters[parameter].NumVals;
  else return 0;
}

// ********************************* Section 6: GP10 assign control ********************************************

// GP10 assigns are organised different from other Roland devices. The settings are spread out, which is a pain:
// 0x20022000 - 0x20022007: Assign switch 1 - 8
// 0x20022008 - 0x2002201F: Assign target 1 - 8
// 0x20022020 - 0x2002203F: Assign target min 1 - 8
// 0x20022040 - 0x2002205F: Assign target max 1 - 8
// 0x20022060 - 0x20022067: Assign source 1 - 8
// 0x20022068 - 0x2002206F: Assign latch (momentary/toggle) 1 - 8
// The rest we do not need, so we won't read it

// Whenever there is a GP10 assign on the page, the entire 112 bytes of the  GP10 assigns are read from the device
// Because the MIDI buffer of the VController is limited to 60 bytes including header and footer bytes, we read them in three turns

// Procedures for GP10_ASSIGN:
// 1. Load in SP array - GP10_assign_load() below
// 2. Request - GP10_request_complete_assign_area() (first time only)
// 3. Read assign area - GP10_read_complete_assign_area() (first time only)
// 4. Request parameter state - GP10_assign_request() below, also uses GP10_target_lookup()
// 5. Read parameter state - GP10_read_parameter() above
// 6. Press switch - GP10_assign_press() below
// 7. Release switch - GP10_assign_release() below

#define GP10_assign_address_set1 0x20022000
#define GP10_assign_address_set2 0x20022028
#define GP10_assign_address_set3 0x20022050
#define GP10_NUMBER_OF_ASSIGNS 8

void GP10_class::read_assign_name(uint8_t number, String & Output) {
  //if (number < GP10_NUMBER_OF_ASSIGNS)  Output = GP10_assign_title[number];
  if (number < GP10_NUMBER_OF_ASSIGNS)  Output = "ASSIGN " + String(number + 1);
  else Output = "?";
}

void GP10_class::read_assign_trigger(uint8_t number, String & Output) {
  if ((number > 0) && (number < 128)) Output = "CC#" + String(number);
  else Output = "-";
}

uint8_t GP10_class::get_number_of_assigns() {
  return GP10_NUMBER_OF_ASSIGNS;
}

uint8_t GP10_class::trigger_follow_assign(uint8_t number) {
  return number + 20; // Default cc numbers are 20 and up
}


void GP10_class::assign_press(uint8_t Sw, uint8_t value) { // Switch set to GP10_ASSIGN is pressed
  // Send cc MIDI command to GP-10
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

  if (SP[Sw].Assign_on) update_page |= REFRESH_PAGE; //PAGE_load_current(false); // To update the other switch states, we re-load the current page
}

void GP10_class::assign_release(uint8_t Sw) { // Switch set to GP10_ASSIGN is released
  // Send cc MIDI command to GP-10
  uint8_t cc_number = SP[Sw].Trigger;
  MIDI_send_CC(cc_number, 0, MIDI_channel, MIDI_port);

  // Update status
  if (SP[Sw].Latch == MOMENTARY) {
    if (SP[Sw].Assign_on) SP[Sw].State = 2; // Switch state off
    else SP[Sw].State = 0; // Assign off, so LED should be off as well

    if (SP[Sw].Assign_on) update_page |= REFRESH_PAGE; //PAGE_load_current(false); // To update the other switch states, we re-load the current page
  }
}

void GP10_class::assign_load(uint8_t sw, uint8_t assign_number, uint8_t cc_number) { // Switch set to GP10_ASSIGN is loaded in SP array
  SP[sw].Trigger = cc_number; //Save the cc_number in the Trigger variable
  SP[sw].Assign_number = assign_number;
}

void GP10_class::request_current_assign() {
  if (!assign_read) { //Check if the assign area needs to be read first
    request_complete_assign_area();
  }
  else {
    assign_request(); //otherwise just request the individual assign target
  }
}

void GP10_class::request_complete_assign_area() {
  request_sysex(GP10_assign_address_set1, 40);  //Request the first 40 bytes of the GP10 assign area
  request_sysex(GP10_assign_address_set2, 40);  //Request the second 40 bytes of the GP10 assign area
  request_sysex(GP10_assign_address_set3, 32);  //Request the last 32 bytes of the GP10 assign area
}

void GP10_class::read_complete_assign_area(uint32_t address, const unsigned char* sxdata, short unsigned int sxlength) {
  if (address == GP10_assign_address_set1) { // Read assign byte 0 - 39 from GP10 memory
    for (uint8_t count = 0; count < 40; count++) {
      assign_mem[count] = sxdata[count + 12]; //Add byte to the assign memory
    }
  }

  if (address == GP10_assign_address_set2) { // Read assign byte 40 - 79 from GP10 memory
    for (uint8_t count = 0; count < 40; count++) {
      assign_mem[count + 40] = sxdata[count + 12]; //Add byte to the assign memory
    }
  }

  if (address == GP10_assign_address_set3) { // Read assign byte 80 - 111 from GP10 memory
    for (uint8_t count = 0; count < 32; count++) {
      assign_mem[count + 80] = sxdata[count + 12]; //Add byte to the assign memory
    }
    assign_read = true;
    assign_request();
  }
}

void GP10_class::assign_request() { //Request the current assign
  bool found, assign_on;
  String msg;
  uint8_t assign_switch, assign_source, assign_latch;
  uint16_t assign_target, assign_target_min, assign_target_max;

  // First we read out the assign_mem. We calculate the bytes we need to read from an index.
  uint8_t index = SP[Current_switch].Assign_number - 1; //index is between 0 (assign 1) and 7 (assign 8)
  if (index < 8) {
    assign_switch = assign_mem[index];
    assign_target = (assign_mem[8 + (index * 3)] << 8) + (assign_mem[9 + (index * 3)] << 4) + assign_mem[10 + (index * 3)];
    assign_target_min = (assign_mem[0x21 + (index * 4)] << 8) + (assign_mem[0x22 + (index * 4)] << 4) + assign_mem[0x23 + (index * 4)];
    assign_target_max = (assign_mem[0x41 + (index * 4)] << 8) + (assign_mem[0x42 + (index * 4)] << 4) + assign_mem[0x43 + (index * 4)];
    assign_source = assign_mem[0x60 + index];
    assign_latch = assign_mem[0x68 + index];

    uint8_t my_trigger = SP[Current_switch].Trigger;
    if ((my_trigger >= 1) && (my_trigger <= 31)) my_trigger = my_trigger + 12; // Trigger is cc01 - cc31
    if ((my_trigger >= 64) && (my_trigger <= 95)) my_trigger = my_trigger - 20; // Trigger is cc64 - cc95
    assign_on = ((assign_switch == 0x01) && (my_trigger == assign_source)); // Check if assign is on by checking assign switch and source

    DEBUGMSG("GP-10 Assign_switch: 0x" + String(assign_switch, HEX));
    DEBUGMSG("GP-10 Assign_target 0x:" + String(assign_target, HEX));
    DEBUGMSG("GP-10 Assign_min: 0x" + String(assign_target_min, HEX));
    DEBUGMSG("GP-10 Assign_max: 0x" + String(assign_target_max, HEX));
    DEBUGMSG("GP-10 Assign_source: 0x" + String(assign_source, HEX));
    DEBUGMSG("GP-10 Assign_latch: 0x" + String(assign_latch, HEX));
    DEBUGMSG("GP-10 Assign_trigger-check:" + String(my_trigger) + "==" + String(assign_source));

  }
  else {
    assign_on = false; // Wrong assign number - switch off the assign
    assign_source = 0;
    assign_target = 0;
    assign_latch = MOMENTARY;
  }

  if (assign_on) {
    SP[Current_switch].Assign_on = true; // Switch the pedal on
    SP[Current_switch].Latch = assign_latch;
    SP[Current_switch].Assign_max = assign_target_max;
    SP[Current_switch].Assign_min = assign_target_min;

    // Request the target
    DEBUGMSG("Request target of assign " + String(index + 1) + ": " + String(SP[Current_switch].Address, HEX));
    found = target_lookup(assign_target); // Lookup the address of the target in the GP10_Parameters array
    if (found) {
      uint32_t my_address = SP[Current_switch].Address;
      last_requested_sysex_address = my_address;
      last_requested_sysex_type = ASSIGN_TYPE;
      request_sysex(my_address, 2);
    }
    else {  // Pedal set to a parameter which we do not have in the list.
      SP[Current_switch].PP_number = NOT_FOUND;
      SP[Current_switch].Colour = FX_DEFAULT_TYPE;
      SP[Current_switch].Latch = MOMENTARY; // Because we cannot read the state, it is best to make the pedal momentary
      // Set the Label
      msg = "ASGN" + String(SP[Current_switch].Assign_number) + ": Unknown";
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

bool GP10_class::target_lookup(uint16_t target) { // Finds the target and its address in the GP10_parameters table

  // Lookup in GP10_parameter array
  bool found = false;
  for (uint16_t i = 0; i < GP10_NUMBER_OF_PARAMETERS; i++) {
    if (target == GP10_parameters[i].Target) { //Check is we've found the right target
      SP[Current_switch].PP_number = i; // Save the index number
      SP[Current_switch].Address = GP10_parameters[i].Address;
      found = true;
    }
  }
  return found;
}





