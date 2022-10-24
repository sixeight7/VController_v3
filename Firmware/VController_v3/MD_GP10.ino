// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: GP10 Initialization
// Section 2: GP10 common MIDI in functions
// Section 3: GP10 common MIDI out functions
// Section 4: GP10 program change
// Section 5: GP10 parameter control
// Section 6: GP10 assign control
// Section 7: GP10 expression pedal control

// ********************************* Section 1: GP10 Initialization ********************************************

// Boss GP-10 settings:
#define GP10_MIDI_CHANNEL 1 // Was unable to change patch when GP-10 channel was not 1. Seems to be a bug in the GP-10
#define GP10_MIDI_PORT USBHMIDI_PORT
#define GP10_PATCH_MIN 0
#define GP10_PATCH_MAX 98

//Messages are abbreviated to just the address and the data bytes. Checksum is calculated automatically
//Example: {0xF0, 0x41, 0x10, 0x00, 0x00, 0x00, 0x05, 0x12, 0x7F, 0x00, 0x00, 0x01, 0x01, 0x7F, 0xF7} is reduced to 0x7F000001, 0x01

#define GP10_EDITOR_MODE_ON 0x7F000001, 0x01 //Gets the GP-10 spitting out lots of sysex data. Should be switched on, otherwise the tuner does not work
#define GP10_EDITOR_MODE_OFF 0x7F000001, 0x00
#define GP10_REQUEST_CURRENT_PATCH_NAME 0x20000000, 12 //Request 12 bytes for current patch name
#define GP10_REQUEST_CURRENT_PATCH_NUMBER 0x00000000, 1 //Request current patch number (Yes, 0 is the correct address!)

#define GP10_TUNER_ON 0x7F000002, 0x02 // Changes the running mode of the GP-10 to Tuner - Got these terms from the VG-99 sysex manual.
#define GP10_TUNER_OFF 0x7F000002, 0x00 //Changes the running mode of the GP10 to play.
#define GP10_SOLO_ON 0x2000500B, 0x01
#define GP10_SOLO_OFF 0x2000500B, 0x00

#define GP10_TEMPO 0x20000801  // Accepts values from 40 bpm - 250 bpm
#define GP10_HARMONIST_KEY 0x20014001 // Sets the key for the harmonist, which is the only effect that is key related.

#define GP10_FOOT_VOL 0x20020803 // The address of the footvolume - values between 0 and 100
#define GP10_COSM_GUITAR_SW 0x20001000 // The address of the COSM guitar switch
#define GP10_NORMAL_PU_SW 0x20000804 // The address of the COSM guitar switch

// Initialize device variables
// Called at startup of VController
FLASHMEM void MD_GP10_class::init() { // Default values for variables
  MD_base_class::init();

  // Boss GP-10 variables:
  enabled = DEVICE_DETECT; // Default value
  strcpy(device_name, "GP10");
  strcpy(full_device_name, "Boss GP-10");
  patch_min = GP10_PATCH_MIN;
  patch_max = GP10_PATCH_MAX;
  max_times_no_response = MAX_TIMES_NO_RESPONSE; // The number of times the GP-10 does not have to respond before disconnection
  COSM_onoff = 0;
  nrml_pu_onoff = 0;
  sysex_delay_length = 5; // minimum delay time between sysex messages (in msec).
  my_LED_colour = 4; // Default value: orange
  MIDI_channel = GP10_MIDI_CHANNEL; // Default value
  MIDI_port_manual = MIDI_port_number(GP10_MIDI_PORT); // Default value
  is_always_on = true; // Default value
#if defined(CONFIG_VCTOUCH)
  my_device_page1 = GP10_DEFAULT_VCTOUCH_PAGE1; // Default value
  my_device_page2 = GP10_DEFAULT_VCTOUCH_PAGE2; // Default value
  my_device_page3 = GP10_DEFAULT_VCTOUCH_PAGE3; // Default value
  my_device_page4 = GP10_DEFAULT_VCTOUCH_PAGE4; // Default value
#elif defined(CONFIG_VCMINI)
  my_device_page1 = GP10_DEFAULT_VCMINI_PAGE1; // Default value
  my_device_page2 = GP10_DEFAULT_VCMINI_PAGE2; // Default value
  my_device_page3 = GP10_DEFAULT_VCMINI_PAGE3; // Default value
  my_device_page4 = GP10_DEFAULT_VCMINI_PAGE4; // Default value
#elif defined (CONFIG_CUSTOM)
  my_device_page1 = GP10_DEFAULT_CUSTOM_PAGE1; // Default value
  my_device_page2 = GP10_DEFAULT_CUSTOM_PAGE2; // Default value
  my_device_page3 = GP10_DEFAULT_CUSTOM_PAGE3; // Default value
  my_device_page4 = GP10_DEFAULT_CUSTOM_PAGE4; // Default value
#else
  my_device_page1 = GP10_DEFAULT_VC_PAGE1; // Default value
  my_device_page2 = GP10_DEFAULT_VC_PAGE2; // Default value
  my_device_page3 = GP10_DEFAULT_VC_PAGE3; // Default value
  my_device_page4 = GP10_DEFAULT_VC_PAGE4; // Default value
#endif

#ifdef IS_VCTOUCH
  device_pic = img_GP10;
#endif
}

// ********************************* Section 2: GP10 common MIDI in functions ********************************************

FLASHMEM void MD_GP10_class::check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port) { // Check incoming sysex messages from  Called from MIDI:OnSysEx/OnSerialSysEx

  // Check if it is a message from a GP-10
  if ((port == MIDI_in_port) && (sxdata[1] == 0x41) && (sxdata[2] == MIDI_device_id) && (sxdata[3] == 0x00) && (sxdata[4] == 0x00) && (sxdata[5] == 0x00) && (sxdata[6] == 0x05) && (sxdata[7] == 0x12)) {
    uint32_t address = (sxdata[8] << 24) + (sxdata[9] << 16) + (sxdata[10] << 8) + sxdata[11]; // Make the address 32 bit

    // Check checksum
    uint16_t sum = 0;
    for (uint8_t i = 8; i < sxlength - 2; i++) sum += sxdata[i];
    bool checksum_ok = (sxdata[sxlength - 2] == calc_Roland_checksum(sum));

    // Check if it is the patch number
    if ((address == 0x00000000) && (checksum_ok)) {
      if (patch_number != sxdata[12]) { //Right after a patch change the patch number is sent again. So here we catch that message.
        set_patch_number(sxdata[12]);
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
          if (sxlength == 26) {
            for (uint8_t count = 0; count < 12; count++) {
              SP[last_requested_sysex_switch].Label[count] = static_cast<char>(sxdata[count + 12]); //Add ascii character to the SP.Label String
            }
            for (uint8_t count = 12; count < 16; count++) {
              SP[last_requested_sysex_switch].Label[count] = ' '; //Fill the rest of the label with spaces
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
        case REQUEST_ASSIGN_TYPE:
          //case GP10_ASSIGN:
          read_parameter(last_requested_sysex_switch, sxdata[12], sxdata[13]);
          PAGE_request_next_switch();
          break;
      }
    }

    // Check if it is the patch name (address: 0x20, 0x00, 0x00, 0x00)
    if ((address == 0x20000000) && (checksum_ok)) {
      current_patch_name = "";
      for (uint8_t count = 12; count < 24; count++) {
        current_patch_name += static_cast<char>(sxdata[count]); //Add ascii character to Patch Name String
      }
      current_patch_name += "    ";
      update_main_lcd = true;
      if (popup_patch_name) {
        LCD_show_popup_label(current_patch_name, ACTION_TIMER_LENGTH);
        popup_patch_name = false;
      }
    }

    // Check if it is the exp sw, exp and exp_on state
    if ((address == 0x2002180C) && (checksum_ok)) {
      exp_sw_type = sxdata[12]; // EXP SW type
      exp_type = sxdata[14]; // EXP type
      exp_on_type = sxdata[15]; // EXP_ON type
      //if (sxdata[18] == 0x01) current_exp_pedal = 2; // EXP_LED status - does not work - correct state is not sent by the GP-10 :-(
      //else current_exp_pedal = 1;
      update_exp_label(last_requested_sysex_switch);
      PAGE_request_next_switch();
    }

    // Read GP10 assign area
    if (checksum_ok) read_complete_assign_area(last_requested_sysex_switch, address, sxdata, sxlength);

    // Check if it is the instrument on/off states
    if (checksum_ok) check_inst_switch_states(sxdata, sxlength);

    // Check if it is some other stompbox function and copy the status to the right LED
    //GP10_check_stompbox_states(sxdata, sxlength);
  }

}

FLASHMEM void MD_GP10_class::check_PC_in(uint8_t program, uint8_t channel, uint8_t port) {  // Check incoming PC messages from  Called from MIDI:OnProgramChange

  // Check the source by checking the channel
  if ((port == MIDI_in_port) && (channel == MIDI_channel)) { // GP10 sends a program change
    if (patch_number != program) {
      set_patch_number(program);
      request_sysex(GP10_REQUEST_CURRENT_PATCH_NAME); // So the main display always show the correct patch
      //page_check();
      do_after_patch_selection();
      update_page = REFRESH_PAGE;
    }
  }
}


// Detection of GP-10

FLASHMEM void MD_GP10_class::identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t in_port, uint8_t out_port) {
  // Check if it is a GP-10
  if ((sxdata[5] == 0x41) && (sxdata[6] == 0x05) && (sxdata[7] == 0x03) && (enabled == DEVICE_DETECT)) {
    no_response_counter = 0;
    if (connected == false) connect(sxdata[2], in_port, out_port); //Byte 2 contains the correct device ID
  }
}

FLASHMEM void MD_GP10_class::do_after_connect() {
  request_sysex(GP10_REQUEST_CURRENT_PATCH_NUMBER);
  request_sysex(GP10_REQUEST_CURRENT_PATCH_NAME); // So the main display always show the correct patch
  //write_sysex(GP10_EDITOR_MODE_ON); // Put the GP10 in EDITOR mode - otherwise tuner will not work
  current_exp_pedal = 1;
  do_after_patch_selection();
  update_page = REFRESH_PAGE;
  assign_read = false; // Assigns should be read again
}


// ********************************* Section 3: GP10 common MIDI out functions ********************************************

FLASHMEM void MD_GP10_class::write_sysex(uint32_t address, uint8_t value) { // For sending one data byte

  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t checksum = calc_Roland_checksum(ad[3] + ad[2] + ad[1] + ad[0] + value); // Calculate the Roland checksum
  uint8_t sysexmessage[15] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x00, 0x05, 0x12, ad[3], ad[2], ad[1], ad[0], value, checksum, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 15, MIDI_out_port, 1); // GP-10 connected via USBHost_t36 library will only supoort sysex messages via cable 1 (default 0)
}

FLASHMEM void MD_GP10_class::write_sysex(uint32_t address, uint8_t value1, uint8_t value2) { // For sending two data bytes

  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t checksum = calc_Roland_checksum(ad[3] + ad[2] + ad[1] + ad[0] + value1 + value2); // Calculate the Roland checksum
  uint8_t sysexmessage[16] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x00, 0x05, 0x12, ad[3], ad[2], ad[1], ad[0], value1, value2, checksum, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 16, MIDI_out_port, 1);
}

FLASHMEM void MD_GP10_class::request_sysex(uint32_t address, uint8_t no_of_bytes) {
  uint8_t *ad = (uint8_t*)&address; //Split the 32-bit address into four bytes: ad[3], ad[2], ad[1] and ad[0]
  uint8_t no1 = no_of_bytes >> 7;
  uint8_t no2 = no_of_bytes & 0x7F;
  uint8_t checksum = calc_Roland_checksum(ad[3] + ad[2] + ad[1] + ad[0] +  no1 + no2); // Calculate the Roland checksum
  uint8_t sysexmessage[18] = {0xF0, 0x41, MIDI_device_id, 0x00, 0x00, 0x00, 0x05, 0x11, ad[3], ad[2], ad[1], ad[0], 0x00, 0x00, no1, no2, checksum, 0xF7};
  check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 18, MIDI_out_port, 1);
}

FLASHMEM void MD_GP10_class::set_bpm() {
  if (connected) {
    write_sysex(GP10_TEMPO, Setting.Bpm >> 4, Setting.Bpm & 0x0F); // Tempo is modulus 16. It's all so very logical. NOT.
  }
}

FLASHMEM void MD_GP10_class::start_tuner() {
  if (connected) {
    write_sysex(GP10_EDITOR_MODE_ON); // Put the GP10 in EDITOR mode - otherwise tuner will not work
    write_sysex(GP10_TUNER_ON); // Start tuner on GP-10
  }
}

FLASHMEM void MD_GP10_class::stop_tuner() {
  if (connected) {
    write_sysex(GP10_TUNER_OFF); // Stop tuner on GP-10
    write_sysex(GP10_EDITOR_MODE_OFF); // Switch off EDITOR mode - so the GP10 will respond faster
  }
}

// ********************************* Section 4: GP10 program change ********************************************

FLASHMEM void MD_GP10_class::do_after_patch_selection() {
  request_onoff = false;
  is_on = connected;
  if (Setting.Send_global_tempo_after_patch_change == true) {
    delay(5); // GP10 misses send bpm command...
    set_bpm();
  }
  Current_patch_number = patch_number;
  update_LEDS = true;
  update_main_lcd = true;
  assign_read = false; // Assigns should be read again
  request_guitar_switch_states();
  request_current_patch_name();
  //EEPROM.write(EEPROM_GP10_PATCH_NUMBER, patch_number);
  MD_base_class::do_after_patch_selection();
}

FLASHMEM bool MD_GP10_class::request_patch_name(uint8_t sw, uint16_t number) {
  if (number > patch_max) return true;
  number++;
  uint32_t Address = 0x20000000 + ((number / 0x20) * 0x1000000) + ((number % 0x20) * 0x40000); //Calculate the address where the patchname is stored on the GP-10
  last_requested_sysex_address = Address;
  last_requested_sysex_type = REQUEST_PATCH_NAME;
  last_requested_sysex_switch = sw;
  request_sysex(Address, 12); //Request the 12 bytes of the GP10 patchname
  return false;
}

FLASHMEM void MD_GP10_class::request_current_patch_name() {
  request_sysex(GP10_REQUEST_CURRENT_PATCH_NAME);
}

FLASHMEM void MD_GP10_class::number_format(uint16_t number, String &Output) {
  Output += 'P' + String((number + 1) / 10) + String((number + 1) % 10);
}

FLASHMEM void MD_GP10_class::direct_select_format(uint16_t number, String &Output) {
  if (direct_select_state == 0) Output += 'P' + String(number) + "_";
  else Output += 'P' + String(bank_select_number) + String(number);
}

// ** US-20 simulation
// Selecting and muting the GP10 is done by storing the settings of COSM guitar switch and Normal PU switch
// and switching both off when guitar is muted and back to original state when the GP10 is selected

FLASHMEM void MD_GP10_class::request_guitar_switch_states() {
  //GP10_select_LED = GP10_PATCH_COLOUR; //Switch the LED on
  request_sysex(GP10_COSM_GUITAR_SW, 1);
  request_sysex(GP10_NORMAL_PU_SW, 1);
  request_onoff = true;
}

FLASHMEM void MD_GP10_class::check_inst_switch_states(const unsigned char* sxdata, short unsigned int sxlength) {
  if (request_onoff == true) {
    uint32_t address = (sxdata[8] << 24) + (sxdata[9] << 16) + (sxdata[10] << 8) + sxdata[11]; // Make the address 32 bit
    if (address == GP10_COSM_GUITAR_SW) {
      COSM_onoff = sxdata[12];  // Store the value
    }

    if (address == GP10_NORMAL_PU_SW) {
      nrml_pu_onoff = sxdata[12];  // Store the value
      request_onoff = false;
    }
  }
}

FLASHMEM void MD_GP10_class::unmute() {
  is_on = connected;
  if (is_on) {
    write_sysex(GP10_COSM_GUITAR_SW, COSM_onoff); // Switch COSM guitar on
    write_sysex(GP10_NORMAL_PU_SW, nrml_pu_onoff); // Switch normal pu on
  }
}

FLASHMEM void MD_GP10_class::mute() {
  if ((US20_mode_enabled()) && (!is_always_on) && (is_on)) {
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
  char Name[11]; // The name for the label
  uint16_t Sublist; // Which sublist to read for the FX or amp type - 0 if second byte does not contain the type or if there is no sublist +100 Show value from sublist.
  uint8_t Colour; // The colour for this effect.
};


#define GP10_FX_COLOUR 255 // Just a colour number to pick the colour from the GP10_FX_colours table
#define GP10_FX_TYPE_COLOUR 254 //Another number for the FX type
#define SUBLIST_FROM_BYTE2 32768 //Bit 16 set high means read sublist frm byte 2
#define SHOW_NUMBER 31767 // Special number for showing the number (used for gain/volume/etc) - set in sublist
#define SHOW_DOUBLE_NUMBER 31766 // Show number times 2 (used for patch volume/etc) - set in sublist
#define SHOW_PAN 31765 // Special number for showing the pan- set in sublist

const PROGMEM GP10_parameter_struct GP10_parameters[] = {
  {0x166, 0x20016000,   2, "WAH SW", 180 | SUBLIST_FROM_BYTE2, FX_WAH_TYPE}, // 00
  {0x0F1, 0x20005800,   2, "FX SW", 1 | SUBLIST_FROM_BYTE2, GP10_FX_COLOUR},
  {0x16D, 0x20016800,   2, "Chorus SW", 17 | SUBLIST_FROM_BYTE2, FX_MODULATE_TYPE},
  {0x176, 0x20017000,   2, "DLY SW", 20 | SUBLIST_FROM_BYTE2, FX_DELAY_TYPE},
  {0x188, 0x20017800,   2, "RVB SW", 30 | SUBLIST_FROM_BYTE2, FX_REVERB_TYPE},
  {0x192, 0x20020000,   2, "EQ SW", 0, FX_FILTER_TYPE},
  {0x0F2, 0x20005801,  16, "FX Type", 1, GP10_FX_TYPE_COLOUR},
  {0x167, 0x20016001,   6, "WAH TP", 180, FX_WAH_TYPE},
  {0x168, 0x20016002, 101, "WAH POS", SHOW_NUMBER, FX_WAH_TYPE},
  {0x161, 0x20016801,   3, "CHS Type", 17, FX_MODULATE_TYPE},
  {0x177, 0x20017001,  10, "DLY TYPE", 20, FX_DELAY_TYPE}, // 10
  {0x189, 0x20017801,   7, "RVB TYPE", 30, FX_REVERB_TYPE},
  {0xFFF, 0x20000800, 101, "PATCH LVL", SHOW_DOUBLE_NUMBER, FX_DEFAULT_TYPE},
  {0x000, 0x20001000,   2, "COSM GTR", 0, FX_GTR_TYPE},
  {0x001, 0x20001001,   4, "COSM Type", 115, FX_GTR_TYPE},
  {0x0A5, 0x20000804,   2, "NORMAL PU", 0, FX_GTR_TYPE},
  {0x002, 0x20001800, 12, "E.GTR TP", 119, FX_GTR_TYPE},
  {0x003, 0x20001801, 101, "E.GTR LVL", SHOW_NUMBER, FX_GTR_TYPE},
  {0xFFF, 0x20002001, 101, "A.GTR LVL", SHOW_NUMBER, FX_GTR_TYPE},
  {0xFFF, 0x20002801, 101, "BASS LVL", SHOW_NUMBER, FX_GTR_TYPE},
  {0xFFF, 0x20003001, 101, "SYNTH LVL", SHOW_NUMBER, FX_GTR_TYPE}, // 20
  {0xFFF, 0x20003801, 101, "POLYFX LVL", SHOW_NUMBER, FX_GTR_TYPE},
  {0x0CF, 0x20001002,   2, "COSM NS SW", 0, FX_GTR_TYPE},
  {0x09F, 0x20004000,   2, "TUN SW", 143  | SUBLIST_FROM_BYTE2, FX_PITCH_TYPE},
  {0x0A0, 0x20004001,  37, "TUNING", 143, FX_PITCH_TYPE},
  {0x0AD, 0x2000400E,   2, "12 STR SW", 0, FX_PITCH_TYPE},
  {0x0DE, 0x20005000,   2, "Amp SW", 37 | SUBLIST_FROM_BYTE2, FX_AMP_TYPE},
  {0x0DF, 0x20005001,  30, "Amp Type", 37, FX_AMP_TYPE},
  {0x0E0, 0x20005002, 101, "Amp Gain", SHOW_NUMBER, FX_AMP_TYPE},
  {0x0E2, 0x20005004, 101, "Amp LVL", SHOW_NUMBER, FX_AMP_TYPE},
  {0x0E8, 0x2000500B,   2, "Amp solo", 0, FX_AMP_TYPE}, // 30
  {0x0EC, 0x2000500A,   3, "Amp GainSW", 131, FX_AMP_TYPE},
  {0xFFF, 0x20005028,   2, "Bend SW", 131, FX_AMP_TYPE},
  {0xFFF, 0x2000500E,   9, "Amp SP TP", 134, FX_AMP_TYPE},
  {0xFFF, 0x20020803, 101, "Foot Vol", SHOW_NUMBER, FX_AMP_TYPE}, // 34
  {0xFFF, 0x20021800,  18, "CTL1", 67, FX_DEFAULT_TYPE},
  {0xFFF, 0x20021802,  18, "CTL2", 67, FX_DEFAULT_TYPE},
  {0xFFF, 0x20021804,  17, "CTL3", 67, FX_DEFAULT_TYPE},
  {0xFFF, 0x20021806,  17, "CTL4", 67, FX_DEFAULT_TYPE},
  {0xFFF, 0x20021808,  17, "GK S1", 67, FX_DEFAULT_TYPE},
  {0xFFF, 0x2002180A,  17, "GK S2", 67, FX_DEFAULT_TYPE}, // 40
  {0xFFF, 0x2002180C,  15, "EXP SW", 85, FX_DEFAULT_TYPE}, // 41
  {0xFFF, 0x2002180E,  15, "EXP", 100, FX_DEFAULT_TYPE}, // 42
  {0xFFF, 0x2002180F,  15, "EXP ON", 100, FX_DEFAULT_TYPE}, // 43
  {0xFFF, 0x20021810,  15, "EXP2", 100, FX_DEFAULT_TYPE},
  {0xFFF, 0x20021811,  15, "GK VOL", 100, FX_DEFAULT_TYPE},
  {0xFFF, 0x10001000,   2, "Gtr2MIDI", 0, FX_DEFAULT_TYPE},  // 46. Can not be controlled from assignment, but can be from GP10_PARAMETER!!!
};

#define GP10_EXP_SW 41
#define GP10_EXP 42
#define GP10_EXP_ON 43

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
  FX_DYNAMICS_TYPE, // Colour for "COMPRSR"
  FX_DYNAMICS_TYPE, // Colour for "LIMITER"
  FX_FILTER_TYPE, // Colour for "EQ"
  FX_WAH_TYPE, // Colour for "T.WAH"
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

FLASHMEM void MD_GP10_class::read_parameter_name(uint16_t number, String &Output) { // Called from menu
  if (number < number_of_parameters())  Output = GP10_parameters[number].Name;
  else Output = "?";
}

FLASHMEM void MD_GP10_class::read_parameter_value_name(uint16_t number, uint16_t value, String &Output) {
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
FLASHMEM void MD_GP10_class::parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {

  // Send sysex MIDI command to GP-10
  uint8_t value = SCO_return_parameter_value(Sw, cmd);
  if ((SP[Sw].Latch != TGL_OFF) && (number < GP10_NUMBER_OF_PARAMETERS)) {
    write_sysex(GP10_parameters[number].Address, value);
    SP[Sw].Offline_value = value;

    // Show message
    check_update_label(Sw, value);
    String msg = "";
    if (SP[Sw].Type != ASSIGN) {
      msg = GP10_parameters[number].Name;
      if ((GP10_parameters[number].Sublist & SUBLIST_FROM_BYTE2) || (GP10_parameters[number].Sublist == 0)) msg += ' ';
      else msg += ':';
    }
    msg += SP[Sw].Label;
    LCD_show_popup_label(msg, ACTION_TIMER_LENGTH);

    //PAGE_load_current(false); // To update the other parameter states, we re-load the current page
    if (SP[Sw].Latch != UPDOWN) update_page = REFRESH_FX_ONLY;
  }
}

FLASHMEM void MD_GP10_class::parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {
  // Work out state of pedal
  if (SP[Sw].Latch == MOMENTARY) {
    SP[Sw].State = 2; // Switch state off
    if (number < GP10_NUMBER_OF_PARAMETERS) {
      write_sysex(GP10_parameters[number].Address, cmd->Value2);
      SP[Sw].Offline_value = cmd->Value2;
    }

    //PAGE_load_current(false); // To update the other switch states, we re-load the current page
    update_page = REFRESH_FX_ONLY;
  }
}

FLASHMEM void MD_GP10_class::read_parameter_title(uint16_t number, String &Output) {
  Output += GP10_parameters[number].Name;
}

FLASHMEM bool MD_GP10_class::request_parameter(uint8_t sw, uint16_t number) {
  if (can_request_sysex_data()) {
    uint32_t my_address = GP10_parameters[number].Address;
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

FLASHMEM void MD_GP10_class::read_parameter(uint8_t sw, uint8_t byte1, uint8_t byte2) { //Read the current GP10 parameter
  SP[sw].Target_byte1 = byte1;
  SP[sw].Target_byte2 = byte2;

  // Set the status
  SP[sw].State = SCO_find_parameter_state(sw, byte1);

  // Set the colour
  uint16_t index = SP[sw].PP_number; // Read the parameter number (index to GP10-parameter array)
  uint8_t my_colour = GP10_parameters[index].Colour;

  //Check for special colours:
  if (my_colour == GP10_FX_COLOUR) my_colour = GP10_FX_colours[byte2]; //FX type read in byte2
  if (my_colour == GP10_FX_TYPE_COLOUR) my_colour = GP10_FX_colours[byte1]; //FX type read in byte1

  SP[sw].Colour =  my_colour;

  // Set the display message
  String msg = "";
  if (SP[sw].Type == ASSIGN) msg = GP10_parameters[index].Name;
  if (GP10_parameters[index].Sublist > SUBLIST_FROM_BYTE2) { // Check if a sublist exists
    String type_name = GP10_sublists[GP10_parameters[index].Sublist - SUBLIST_FROM_BYTE2 + byte2 - 1];
    msg += '(' + type_name + ')';
  }
  if ((GP10_parameters[index].Sublist > 0) && !(GP10_parameters[index].Sublist & SUBLIST_FROM_BYTE2)) { // Check if state needs to be read
    //String type_name = GP10_sublists[GP10_parameters[index].Sublist + byte1 - 101];
    if (SP[sw].Type == ASSIGN) msg += ':';
    read_parameter_value_name(index, byte1, msg);
  }
  //Copy it to the display name:
  LCD_set_SP_label(sw, msg);
  update_lcd = sw;
}

FLASHMEM void MD_GP10_class::check_update_label(uint8_t Sw, uint8_t value) { // Updates the label for extended sublists
  uint16_t index = SP[Sw].PP_number; // Read the parameter number (index to GP10-parameter array)
  if (index != NOT_FOUND) {
    if ((GP10_parameters[index].Sublist > 0) && !(GP10_parameters[index].Sublist & SUBLIST_FROM_BYTE2)) { // Check if state needs to be read
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

FLASHMEM uint16_t MD_GP10_class::number_of_parameters() {
  return GP10_NUMBER_OF_PARAMETERS;
}

FLASHMEM uint8_t MD_GP10_class::number_of_values(uint16_t parameter) {
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

FLASHMEM void MD_GP10_class::read_assign_name(uint8_t number, String & Output) {
  if (number < GP10_NUMBER_OF_ASSIGNS)  Output += "ASSIGN " + String(number + 1);
  else Output += "--";
}

FLASHMEM void MD_GP10_class::read_assign_short_name(uint8_t number, String & Output) {
  if (number < GP10_NUMBER_OF_ASSIGNS)  Output += "ASG" + String(number + 1);
  else Output += "--";
}

FLASHMEM void MD_GP10_class::read_assign_trigger(uint8_t number, String & Output) {
  if ((number > 0) && (number < 128)) Output = "CC#" + String(number);
  else Output = "-";
}

FLASHMEM uint8_t MD_GP10_class::get_number_of_assigns() {
  return GP10_NUMBER_OF_ASSIGNS;
}

FLASHMEM uint8_t MD_GP10_class::trigger_follow_assign(uint8_t number) {
  return number + 21; // Default cc numbers are 21 and up
}


FLASHMEM void MD_GP10_class::assign_press(uint8_t Sw, uint8_t value) { // Switch set to GP10_ASSIGN is pressed
  // Send cc MIDI command to GP-10
  uint8_t cc_number = SP[Sw].Trigger;
  if (cc_number > 0) MIDI_send_CC(cc_number, value, MIDI_channel, MIDI_out_port);

  // Display the patch function
  if (SP[Sw].Assign_on) {
    uint8_t value = 0;
    if (SP[Sw].State == 1) value = SP[Sw].Assign_max;
    else value = SP[Sw].Assign_min;
    check_update_label(Sw, value);
  }
  LCD_show_popup_label(SP[Sw].Label, ACTION_TIMER_LENGTH);

  if (SP[Sw].Assign_on) update_page = REFRESH_FX_ONLY; //PAGE_load_current(false); // To update the other switch states, we re-load the current page
}

FLASHMEM void MD_GP10_class::assign_release(uint8_t Sw) { // Switch set to GP10_ASSIGN is released
  // Send cc MIDI command to GP-10
  uint8_t cc_number = SP[Sw].Trigger;
  if (cc_number > 0) MIDI_send_CC(cc_number, 0, MIDI_channel, MIDI_out_port);

  // Update status
  if (SP[Sw].Latch == MOMENTARY) {
    if (SP[Sw].Assign_on) {
      SP[Sw].State = 2; // Switch state off
      check_update_label(Sw, SP[Sw].Assign_min);
      LCD_show_popup_label(SP[Sw].Label, ACTION_TIMER_LENGTH);
    }
    else {
      SP[Sw].State = 0; // Assign off, so LED should be off as well
    }

    if (SP[Sw].Assign_on) update_page = REFRESH_FX_ONLY; //PAGE_load_current(false); // To update the other switch states, we re-load the current page
  }
}

FLASHMEM void MD_GP10_class::assign_load(uint8_t sw, uint8_t assign_number, uint8_t cc_number) { // Switch set to GP10_ASSIGN is loaded in SP array
  SP[sw].Trigger = cc_number; //Save the cc_number in the Trigger variable
  SP[sw].Assign_number = assign_number;
}

FLASHMEM void MD_GP10_class::request_current_assign(uint8_t sw) {
  if (!assign_read) { //Check if the assign area needs to be read first
    request_complete_assign_area(sw);
  }
  else {
    assign_request(sw); //otherwise just request the individual assign target
  }
}

FLASHMEM void MD_GP10_class::request_complete_assign_area(uint8_t sw) {
  request_sysex(GP10_assign_address_set1, 40);  //Request the first 40 bytes of the GP10 assign area
  request_sysex(GP10_assign_address_set2, 40);  //Request the second 40 bytes of the GP10 assign area
  request_sysex(GP10_assign_address_set3, 32);  //Request the last 32 bytes of the GP10 assign area
  last_requested_sysex_switch = sw;
}

FLASHMEM void MD_GP10_class::read_complete_assign_area(uint8_t sw, uint32_t address, const unsigned char* sxdata, short unsigned int sxlength) {
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
    assign_request(sw);
  }
}

FLASHMEM void MD_GP10_class::assign_request(uint8_t sw) { //Request the current assign
  bool found, assign_on;
  String msg;
  uint8_t assign_switch, assign_source, assign_latch;
  uint16_t assign_target, assign_target_min, assign_target_max;

  // First we read out the assign_mem. We calculate the bytes we need to read from an index.
  uint8_t index = SP[sw].Assign_number; //index is between 0 (assign 1) and 7 (assign 8)
  if (index < 8) {
    assign_switch = assign_mem[index];
    assign_target = (assign_mem[8 + (index * 3)] << 8) + (assign_mem[9 + (index * 3)] << 4) + assign_mem[10 + (index * 3)];
    assign_target_min = (assign_mem[0x21 + (index * 4)] << 8) + (assign_mem[0x22 + (index * 4)] << 4) + assign_mem[0x23 + (index * 4)];
    assign_target_max = (assign_mem[0x41 + (index * 4)] << 8) + (assign_mem[0x42 + (index * 4)] << 4) + assign_mem[0x43 + (index * 4)];
    assign_source = assign_mem[0x60 + index];
    assign_latch = assign_mem[0x68 + index];

    uint8_t my_trigger = SP[sw].Trigger;
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
    SP[sw].Assign_on = true; // Switch the pedal on
    SP[sw].Latch = assign_latch;
    SP[sw].Assign_max = assign_target_max;
    SP[sw].Assign_min = assign_target_min;

    // Request the target
    DEBUGMSG("Request target of assign " + String(index + 1) + ": " + String(SP[sw].Address, HEX));
    found = target_lookup(sw, assign_target); // Lookup the address of the target in the GP10_Parameters array
    if (found) {
      uint32_t my_address = SP[sw].Address;
      last_requested_sysex_address = my_address;
      last_requested_sysex_type = REQUEST_ASSIGN_TYPE;
      last_requested_sysex_switch = sw;
      request_sysex(my_address, 2);
    }
    else {  // Pedal set to a parameter which we do not have in the list.
      SP[sw].PP_number = NOT_FOUND;
      SP[sw].Colour = FX_DEFAULT_TYPE;
      SP[sw].Latch = MOMENTARY; // Because we cannot read the state, it is best to make the pedal momentary
      // Set the Label
      msg = "ASGN" + String(SP[sw].Assign_number + 1) + ": Unknown";
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

FLASHMEM bool MD_GP10_class::target_lookup(uint8_t sw, uint16_t target) { // Finds the target and its address in the GP10_parameters table

  // Lookup in GP10_parameter array
  bool found = false;
  for (uint16_t i = 0; i < GP10_NUMBER_OF_PARAMETERS; i++) {
    if (target == GP10_parameters[i].Target) { //Check is we've found the right target
      SP[sw].PP_number = i; // Save the index number
      SP[sw].Address = GP10_parameters[i].Address;
      found = true;
    }
  }
  return found;
}

// ********************************* Section 7: GP10 Expression pedal control ********************************************

uint32_t GP10_exp_pedal_target[] = {
  0, //OFF
  0x20020803, //FOOT VOL
  0x20000800, //PTCH LVL
  0x20001801, //MLNG VOL
  0x20000805, //N PU VOL
  0x20000809, //MIXER
  0x2000402F, //STG BEND",
  0x20001802, //MODELING
  0x20006001, //FX
  0x20005002, //AMP
  0x20016002, //WAH
  0x20016807, //CHORUS
  0x20017007, //DELAY"
  0x2001780A, //REVERB
  0x20020007, //EQ
};

const uint8_t GP10_NUMBER_OF_EXP_PEDAL_TARGETS = sizeof(GP10_exp_pedal_target) / sizeof(GP10_exp_pedal_target[0]);

uint32_t GP10_exp_switch_target[] = {
  0, //OFF
  0x2000400e, //12-STRING ON/OFF
  0x20004000, //ALT TUNE ON/OFF
  0x20004028, // STR BEND ON/OFF
  0x20001000, //MODELING ON/OFF
  0x20000804, //NORMAL PU ON/OFF
  0x2000500B, //AMP SOLO SW
  0x20005800, //FX ON/OFF
  0x20020000, //EQ ON/OFF
  0x20016000, //WAH ON/OFF
  0x20016800, //CHORUS ON/OFF
  0x20017000, //DELAY ON/OFF
  0x20017800, //REVERB ON/OFF
  0, //HOLD - Not possible to control!
  0x20021812, //LED ON/OFF
};

const uint8_t GP10_NUMBER_OF_EXP_SWITCH_TARGETS = sizeof(GP10_exp_switch_target) / sizeof(GP10_exp_switch_target[0]);


FLASHMEM void MD_GP10_class::move_expression_pedal(uint8_t sw, uint8_t value, uint8_t exp_pedal) {
  if (exp_pedal == 0) exp_pedal = current_exp_pedal;
  LCD_show_bar(0, value, 0); // Show it on the main display
  uint8_t new_value = map(value, 0, 127, 0, 100);
  if ((exp_pedal == 1) && (exp_type < GP10_NUMBER_OF_EXP_PEDAL_TARGETS)) {
    if (GP10_exp_pedal_target[exp_type] > 0) write_sysex(GP10_exp_pedal_target[exp_type], new_value);
  }
  if ((exp_pedal == 2) && (exp_on_type < GP10_NUMBER_OF_EXP_PEDAL_TARGETS)) {
    if (GP10_exp_pedal_target[exp_on_type] > 0) write_sysex(GP10_exp_pedal_target[exp_on_type], new_value);
  }
  update_exp_label(sw);
  LCD_show_popup_label(SP[sw].Label, ACTION_TIMER_LENGTH);
  update_page = REFRESH_FX_ONLY; // To update the other switch states, we re-load the current page*/
}

FLASHMEM void MD_GP10_class::toggle_expression_pedal(uint8_t sw) {
  if (current_exp_pedal == 0) return;
  current_exp_pedal++;
  if (current_exp_pedal > 2) current_exp_pedal = 1;
  // Send toe switch message
  uint8_t value = 0x00;
  if (current_exp_pedal == 2) value = 0x01;
  if (exp_sw_type < GP10_NUMBER_OF_EXP_SWITCH_TARGETS) {
    if (GP10_exp_switch_target[exp_sw_type] > 0) write_sysex(GP10_exp_switch_target[exp_sw_type], value);
  }
  SP[sw].PP_number = GP10_EXP_SW;
  read_parameter(sw, exp_sw_type, 0); // Update label temporary to show switch state
  LCD_show_popup_label(SP[sw].Label, ACTION_TIMER_LENGTH);
  update_exp_label(sw);
  update_page = REFRESH_FX_ONLY;
}

FLASHMEM bool MD_GP10_class::request_exp_pedal(uint8_t sw, uint8_t exp_pedal) { // Used for both Master_exp_pedal and toggle_exp_pedal
  if (exp_pedal == 0) exp_pedal = current_exp_pedal;
  if (exp_pedal <= 2) {
    last_requested_sysex_switch = sw;
    request_sysex(0x2002180C, 7); // Request the states of exp sw, exp, exp_on and exp sw led status
  }
  LCD_clear_SP_label(sw);
  return false;
}

FLASHMEM void MD_GP10_class::update_exp_label(uint8_t sw) {
  if (current_exp_pedal == 1) {
    SP[sw].PP_number = GP10_EXP;
    read_parameter(sw, exp_type, 0);
  }
  if (current_exp_pedal == 2) {
    SP[sw].PP_number = GP10_EXP_ON;
    read_parameter(sw, exp_on_type, 0);
  }
}
