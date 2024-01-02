// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: ZOOM G3 Initialization
// Section 2: ZOOM G3 common MIDI in functions
// Section 3: ZOOM G3 common MIDI out functions
// Section 4: ZOOM G3 program change
// Section 5: ZOOM G3 parameter control

// ********************************* Section 1: ZOOM G3 initialization ********************************************

// Zoom G3 settings:
#define ZG3_MIDI_CHANNEL 1
#define ZG3_MIDI_PORT USBHMIDI_PORT
#define ZG3_PATCH_MIN 0
#define ZG3_PATCH_MAX 99
#define ZG3_MIDI_TIMER_LENGTH 100 // Minimal time inbetween requests for patch data from MS70

// Documentation of Zoom sysex has been moved to http://www.vguitarforums.com/smf/index.php?topic=4329.msg131444#msg131444 (The ZOOM G3 v2 MIDI specification)
// The relevant messages are repeated here
// 1) The Zoom responds to an MIDI identity request message with F0 7E 00 (Device ID) 06 02 52 (Manufacturing ID for Zoom) 5A (model number G3) 00  00 00 32 2E 31 30 F7
#define ZG3_MODEL_NUMBER 0x5A
// 2) The editor keeps sending F0 52 00 5A 50 F7. The G3 does not seem to respond to it. But it may signal editor mode on.
#define ZG3_EDITOR_MODE_ON 0x50
// 3) If the editor sends F0 52 00 5A 33 F7, the G3 responds with the current Bank number (CC00 and CC32) and the current Program number (PC)
#define ZG3_REQUEST_CURRENT_PATCH_NUMBER 0x33
// 4) If the editor sends F0 52 00 5A 29 F7, the G3 responds with the current patch in 110 bytes with comaand number 28. Byte 0x61 - 0x6B contain the patch name. with a strange 0 at position 0x65
#define ZG3_REQUEST_CURRENT_PATCH 0x29
// 5) The editor then reads all individual patches by sending F0 52 00 5A 09 00 00 {00-63} (patch number) F7.
//    The G3 responds with 120 bytes with command number 08. Byte 0x66 - 0x70 contain the patch name. with an "overflow byte" at position 0x6A.
// 6) At the end the editor sends F0 52 00 5A 51 F7 and communication seems to stop.
#define ZG3_EDITOR_MODE_OFF 0x51
// 7) Switch effect on/off:
//    Switch on effect 1: F0 52 00 5A 31 00 00 01 (on) 00 F7, switch off: F0 52 00 5A 31 00 00 00 (off) 00 F7
//    Switch on effect 2: F0 52 00 5A 31 01 00 01 (on) 00 F7 switch off: F0 52 00 5A 31 01 00 00 (off) 00 F7. etc. sixth byte changes consistent for the effect
//    Same message is sent for changing a knob on the G3, but byte 7 is not 0x00
// 8) Tempo. set bpm=40: F0 52 00 5A 31 06 08 28 00 F7 => 0x28 = 40, bpm=240: F0 52 00 5A 31 06 08 7A 01 F7 => 0x7A = 122, 122+128 = 240, so the last two bytes are the tempo.

// Initialize device variables
// Called at startup of VController
void MD_ZG3_class::init() // Default values for variables
{
  MD_base_class::init();

  // Variables
  enabled = DEVICE_DETECT; // Default value
  strcpy(device_name, "ZG3");
  strcpy(full_device_name, "Zoom G3");
  patch_number_offset = 0; // First patch is numbered zero
  patch_min = ZG3_PATCH_MIN;
  patch_max = ZG3_PATCH_MAX;
  sysex_delay_length = 5; // time between sysex messages (in msec).
  max_times_no_response = MAX_TIMES_NO_RESPONSE; // The number of times the Zoom G3 does not have to respond before disconnection
  my_LED_colour = 1; // Default value: green
  MIDI_channel = ZG3_MIDI_CHANNEL; // Default value
  MIDI_port_manual = MIDI_port_number(ZG3_MIDI_PORT); // Default value
#if defined(CONFIG_VCTOUCH)
  my_device_page1 = ZG3_DEFAULT_VCTOUCH_PAGE1; // Default values for VC-touch
  my_device_page2 = ZG3_DEFAULT_VCTOUCH_PAGE2;
  my_device_page3 = ZG3_DEFAULT_VCTOUCH_PAGE3;
  my_device_page4 = ZG3_DEFAULT_VCTOUCH_PAGE4;
#elif defined(CONFIG_VCMINI)
  my_device_page1 = ZG3_DEFAULT_VCMINI_PAGE1; // Default values for VC-mini
  my_device_page2 = ZG3_DEFAULT_VCMINI_PAGE2;
  my_device_page3 = ZG3_DEFAULT_VCMINI_PAGE3;
  my_device_page4 = ZG3_DEFAULT_VCMINI_PAGE4;
#elif defined (CONFIG_CUSTOM)
  my_device_page1 = ZG3_DEFAULT_CUSTOM_PAGE1; // Default values for custom VC device
  my_device_page2 = ZG3_DEFAULT_CUSTOM_PAGE2;
  my_device_page3 = ZG3_DEFAULT_CUSTOM_PAGE3;
  my_device_page4 = ZG3_DEFAULT_CUSTOM_PAGE4;
#else
  my_device_page1 = ZG3_DEFAULT_VC_PAGE1; // Default values for VController
  my_device_page2 = ZG3_DEFAULT_VC_PAGE2;
  my_device_page3 = ZG3_DEFAULT_VC_PAGE3;
  my_device_page4 = ZG3_DEFAULT_VC_PAGE4;
#endif

#ifdef IS_VCTOUCH
  device_pic = img_ZG3;
#endif
}

void MD_ZG3_class::update() {
  if ((send_patch_change) && (millis() > midi_timer)) { // We delay the sending of a PC message if these come to close together.
    MIDI_send_PC(patch_number, MIDI_channel, MIDI_out_port);
    DEBUGMSG("out(" + String(device_name) + ") PC" + String(patch_number)); //Debug
    do_after_patch_selection();
    midi_timer = millis() + ZG3_MIDI_TIMER_LENGTH;
    send_patch_change = false;
  }
}

// ********************************* Section 2: ZOOM G3 common MIDI in functions ********************************************

void MD_ZG3_class::check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port) { // Check incoming sysex messages from  Called from MIDI:OnSysEx/OnSerialSysEx

  // Check if it is a message from a ZOOM G3
  if ((port == MIDI_in_port) && (sxdata[1] == 0x52) && (sxdata[2] == MIDI_device_id) && (sxdata[3] == ZG3_MODEL_NUMBER)) {

    // Check if it is the patch data of a specific patch
    if ((sxdata[4] == 0x08) && (sxdata[7] == last_requested_sysex_patch_number)) {
      if (sxlength == 120) {
        //if (sxdata[4] == 0x08) {
        //DEBUGMSG("!!!!Length sysex message: " + String(sxlength));
        // Read the patch name - needs to be read in two parts, because the fifth byte in the patchname string is a 0.
        for (uint8_t count = 0; count < 4; count++) { // Read the first four characters of the name
          SP[last_requested_sysex_switch].Label[count] = static_cast<char>(sxdata[count + 0x66]); //Add ascii character to the SP.Label String
        }
        for (uint8_t count = 4; count < 10; count++) { // Read the last six characters of the name
          SP[last_requested_sysex_switch].Label[count] = static_cast<char>(sxdata[count + 0x67]); //Add ascii character to the SP.Label String
        }
        for (uint8_t count = 10; count < 17; count++) { // Fill the rest with spaces
          SP[last_requested_sysex_switch].Label[count] = ' ';
        }

        if (SP[last_requested_sysex_switch].PP_number == patch_number) {
          current_patch_name = SP[last_requested_sysex_switch].Label; // Load current patchname when it is read
          update_main_lcd = true; // And show it on the main LCD
        }
        DEBUGMSG(SP[last_requested_sysex_switch].Label);
        PAGE_request_next_switch();
      }
      else {
        PAGE_request_current_switch(); // Retry reading
      }
    }

    // Check if it is the current patch data
    if (sxdata[4] == 0x28) {
      if (sxlength == ZG3_CURRENT_PATCH_DATA_SIZE) {

        // These codes are the same as above. Only the sxdata postitions are all minus 5.
        FX[0] = ((sxdata[5] & B01000000) << 1) | sxdata[6];
        FX[1] = ((sxdata[13] & B00000010) << 6) | sxdata[19];
        FX[2] = ((sxdata[29] & B00001000) << 4) | sxdata[33];
        FX[3] = ((sxdata[45] & B00100000) << 2) | sxdata[47];
        FX[4] = ((sxdata[53] & B00000001) << 7) | sxdata[60];
        FX[5] = ((sxdata[69] & B00000100) << 5) | sxdata[74];

        // Read the patch name - needs to be read in two parts, because the fifth byte in the patchname string is a 0.
        for (uint8_t count = 0; count < 4; count++) { // Read the first four characters of the name
          current_patch_name[count] = static_cast<char>(sxdata[count + 0x61]); //Add ascii character to the SP.Label String
        }
        for (uint8_t count = 4; count < 10; count++) { // Read the last six characters of the name
          current_patch_name[count] = static_cast<char>(sxdata[count + 0x62]); //Add ascii character to the SP.Label String
        }

        update_main_lcd = true;
        if (popup_patch_name) {
          if (LCD_check_popup_allowed(0)) LCD_show_popup_label(current_patch_name, ACTION_TIMER_LENGTH);
          popup_patch_name = false;
        }
        update_page = REFRESH_FX_ONLY; // Need to update everything, otherwise displays go blank on detection of the G3.
      }
      else {
        write_sysex(ZG3_REQUEST_CURRENT_PATCH); //Re-request the patch data
      }
    }

    if ((sxdata[4] == 0x31) && (sxdata[6] == 0x00) && (sxlength == 10)) { // Check for effect switched off on the ZOOM G5.
      uint8_t index = sxdata[5];
      if (index < NUMBER_OF_FX_SLOTS) FX[index] &= B11111110; //Make a zero of bit 1 - this will switch off the effect
      MIDI_enable_device_check();
      update_page = REFRESH_FX_ONLY;
    }
  }
}

// Detection of Zoom G3

void MD_ZG3_class::identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t in_port, uint8_t out_port) {
  // Check if it is a Zoom G3
  if ((sxdata[5] == 0x52) && (sxdata[6] == ZG3_MODEL_NUMBER) && (sxdata[7] == 0x00) && (enabled == DEVICE_DETECT)) {
    no_response_counter = 0;
    if (connected == false) connect(sxdata[2], in_port, out_port); //Byte 2 contains the correct device ID
  }
}

void MD_ZG3_class::do_after_connect() {
  is_on = true;
  write_sysex(ZG3_EDITOR_MODE_ON); // Put the Zoom G3 in EDITOR mode
  write_sysex(ZG3_REQUEST_CURRENT_PATCH_NUMBER);
  write_sysex(ZG3_REQUEST_CURRENT_PATCH); //This will update the FX buttons
  update_page = REFRESH_PAGE;
}

// ********************************* Section 3: ZOOM G3 common MIDI out functions ********************************************

void MD_ZG3_class::write_sysex(uint8_t message) {
  uint8_t sysexmessage[6] = {0xF0, 0x52, MIDI_device_id, ZG3_MODEL_NUMBER, message, 0xF7};
  //check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 6, MIDI_out_port);
}

void MD_ZG3_class::request_patch(uint8_t number) { //Will request the complete patch information from the Zoom G3 (will receive 120 bytes as an answer)
  uint8_t sysexmessage[9] = {0xF0, 0x52, MIDI_device_id, ZG3_MODEL_NUMBER, 0x09, 0x00, 0x00, number, 0xF7};
  //check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 9, MIDI_out_port);
}

void MD_ZG3_class::set_FX_state(uint8_t number, uint8_t state) { //Will set an effect on or off
  uint8_t sysexmessage[10] = {0xF0, 0x52, MIDI_device_id, ZG3_MODEL_NUMBER, 0x31, number, 0x00, state, 0x00, 0xF7}; // F0 52 00 5A 31 00 (FX) 00 01 (on) 00 F7
  //check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 10, MIDI_out_port);
}

void MD_ZG3_class::set_bpm() { //Will change the bpm to the specified value
  if (connected) {
    uint8_t sysexmessage[10] = {0xF0, 0x52, MIDI_device_id, ZG3_MODEL_NUMBER, 0x31, 0x06, 0x08, (uint8_t)(Setting.Bpm & 0x7F), (uint8_t)(Setting.Bpm >> 7), 0xF7}; // F0 52 00 5A 31 06 08 7A 01 F7
    //check_sysex_delay();
    MIDI_send_sysex(sysexmessage, 10, MIDI_out_port);
  }
}

void MD_ZG3_class::start_tuner() {
  if (connected) {
    MIDI_send_CC(0x4A, 0x40, MIDI_channel, MIDI_out_port); // Only works for v1 firmware on the G3
  }
}

void MD_ZG3_class::stop_tuner() {
  if (connected) {
    MIDI_send_CC(0x4A, 0x00, MIDI_channel, MIDI_out_port); // Only works for v1 firmware on the G3
  }
}

// ********************************* Section 4: G3 program change ********************************************

void MD_ZG3_class::select_patch(uint16_t new_patch) {

  if (new_patch == patch_number) unmute();
  else prev_patch_number = patch_number;
  patch_number = new_patch;
  send_patch_change = true;
  update_LEDS = true;
  update_main_lcd = true;
}

void MD_ZG3_class::do_after_patch_selection() {
  is_on = connected;
  Current_patch_number = patch_number;
  update_LEDS = true;
  update_main_lcd = true;
  write_sysex(ZG3_REQUEST_CURRENT_PATCH); // Request current patch, so the FX buttons will be updated
  MIDI_disable_device_check();
  MD_base_class::do_after_patch_selection();
}

void MD_ZG3_class::page_check() { // Checks if the current patch is on the page
  bool onpage = false;
  for (uint8_t s = 0; s < NUMBER_OF_SWITCHES; s++) {
    if ((SP[s].Type == PATCH) && (SP[s].Sel_type == BANKSELECT) && (SP[s].PP_number == patch_number)) {
      onpage = true;
      current_patch_name = SP[s].Label; // Set patchname correctly
      ZG3_Recall_FXs(s); // Copy the FX button states from memory
      update_page = REFRESH_PAGE;
    }
  }
  if (!onpage) {
    write_sysex(ZG3_REQUEST_CURRENT_PATCH); // Request FX button states
    MIDI_disable_device_check();
  }
}

FLASHMEM bool MD_ZG3_class::request_patch_name(uint8_t sw, uint16_t number) {
  DEBUGMSG("Requesting patch " + String(number));
  if (number > patch_max) return true;
  last_requested_sysex_switch = sw;
  last_requested_sysex_patch_number = number;
  request_patch(number);
  return false;
}

void MD_ZG3_class::number_format(uint16_t number, String &Output) {
  char BankChar = char(65 + (number / 10));
  Output.append(BankChar);
  Output.append(String(number % 10));
}

void MD_ZG3_class::direct_select_format(uint16_t number, String &Output) {
  char BankChar;
  if (direct_select_state == 0) {
    BankChar = 65 + number;
    Output.append(BankChar);
    Output.append("_");
  }
  else {
    BankChar = 65 + bank_select_number;
    Output.append(BankChar);
    Output.append(String(number));
  }
}

// ********************************* Section 5: ZOOM G3 parameter control ********************************************

// Define array for G3 effeect names and colours
struct ZG3_FX_type_struct { // Combines all the data we need for controlling a parameter in a device
  char Name[17]; // The name for the label
  uint8_t Colour; // The colour for this effect.
};

const PROGMEM ZG3_FX_type_struct ZG3_FX_types[] = { // Table with the name and colour for every effect of the Zoom G3
  {"M-FILTER", FX_FILTER_TYPE}, // 01
  {"THE VIBE", FX_MODULATE_TYPE}, // 02
  {"Z-ORGAN", FX_MODULATE_TYPE}, // 03
  {"SLICER", FX_MODULATE_TYPE}, // 04
  {"PHASE DELAY", FX_DELAY_TYPE}, // 05
  {"FILTER DELAY", FX_DELAY_TYPE}, // 06
  {"PITCH DELAY", FX_DELAY_TYPE}, // 07
  {"STEREO DELAY", FX_DELAY_TYPE}, // 08
  {"BIT CRUSH", FX_FILTER_TYPE}, // 09
  {"BOMBER", FX_FILTER_TYPE}, // 10
  {"DUO-PHASE", FX_MODULATE_TYPE}, // 11
  {"MONOSYNTH", FX_MODULATE_TYPE}, // 12
  {"SEQ FILTER", FX_MODULATE_TYPE}, // 13
  {"RANDOM FLTR", FX_MODULATE_TYPE}, // 14
  {"WARP PHASER", FX_MODULATE_TYPE}, // 15
  {"TRIGGER HOLD DLY", FX_DELAY_TYPE}, // 16
  {"CHOR+DLY", FX_DELAY_TYPE}, // 17
  {"CHOR+REV", FX_REVERB_TYPE}, // 18
  {"DLY+REV", FX_REVERB_TYPE}, // 19
  {"COMP PHASER", FX_MODULATE_TYPE}, // 20
  {"COMP AWAH", FX_MODULATE_TYPE}, // 21
  {"FLANGER V-CHO", FX_MODULATE_TYPE}, // 22
  {"COMP OD", FX_DYNAMICS_TYPE}, // 23
  {"COMP", FX_DYNAMICS_TYPE}, // 24 -Start
  {"Rock COMP", FX_DYNAMICS_TYPE}, // 25
  {"M COMP", FX_DYNAMICS_TYPE}, // 26
  {"SLOW ATTACK", FX_FILTER_TYPE}, // 27
  {"ZNR", FX_DYNAMICS_TYPE}, // 28
  {"NOISE GATE", FX_DYNAMICS_TYPE}, // 29
  {"DIRTY GATE", FX_AMP_TYPE}, // 30
  {"GRAPHIC EQ", FX_FILTER_TYPE}, // 31
  {"PARA EQ", FX_FILTER_TYPE}, // 32
  {"COMB FILTER", FX_FILTER_TYPE}, // 33
  {"AUTO WAH", FX_WAH_TYPE}, // 34
  {"RESONANCE", FX_WAH_TYPE}, // 35
  {"STEP", FX_WAH_TYPE}, // 36
  {"CRY", FX_WAH_TYPE}, // 37
  {"OCTAVE", FX_PITCH_TYPE}, // 38
  {"TREMOLO", FX_MODULATE_TYPE}, // 39
  {"PHASER", FX_MODULATE_TYPE}, // 40
  {"RING MOD", FX_MODULATE_TYPE}, // 41
  {"CHORUS", FX_MODULATE_TYPE}, // 42
  {"DETUNE", FX_MODULATE_TYPE}, // 43
  {"VINTAGE CE", FX_MODULATE_TYPE}, // 44
  {"STEREO CHORUS", FX_MODULATE_TYPE}, // 45
  {"ENSEMBLE", FX_MODULATE_TYPE}, // 46
  {"VINTAGE FLANGER", FX_MODULATE_TYPE}, // 47
  {"DYNO FLANGER", FX_MODULATE_TYPE}, // 48
  {"VIBRATO", FX_MODULATE_TYPE}, // 49
  {"PITH SHIFT", FX_MODULATE_TYPE}, // 50
  {"BEND CHORUS", FX_MODULATE_TYPE}, // 51
  {"MONO PITCH", FX_PITCH_TYPE}, // 52
  {"HPS", FX_PITCH_TYPE}, // 53
  {"DELAY", FX_DELAY_TYPE}, // 54
  {"TAPE ECHO", FX_DELAY_TYPE}, // 55
  {"MOD DELAY", FX_DELAY_TYPE}, // 56
  {"ANALOG DELAY", FX_DELAY_TYPE}, // 57
  {"REVERSE DELAY", FX_DELAY_TYPE}, // 58
  {"MULTI TAP DELAY", FX_DELAY_TYPE}, // 59
  {"DYNA DELAY", FX_DELAY_TYPE}, // 60
  {"HALL", FX_REVERB_TYPE}, // 61
  {"ROOM", FX_REVERB_TYPE}, // 62
  {"TILED ROOM", FX_REVERB_TYPE}, // 63
  {"SPRING", FX_REVERB_TYPE}, // 64
  {"ARENA REVERB", FX_REVERB_TYPE}, // 65
  {"EARLY REFLECTION", FX_REVERB_TYPE}, // 66
  {"AIR", FX_REVERB_TYPE}, // 67
  {"PEDAL VX", FX_WAH_TYPE}, // 68
  {"PEDAL CRY", FX_WAH_TYPE}, // 69
  {"PEDAL PITCH", FX_PITCH_TYPE}, // 70
  {"PEDAL MN PIT", FX_PITCH_TYPE}, // 71
  {"BOOSTER", FX_DIST_TYPE}, // 72
  {"OVERDRIVE", FX_DIST_TYPE}, // 73
  {"TUBE SCREAM", FX_DIST_TYPE}, // 74
  {"GOVERNOR", FX_DIST_TYPE}, // 75
  {"DIST+", FX_DIST_TYPE}, // 76
  {"DIST 1", FX_DIST_TYPE}, // 77
  {"SQUEAK", FX_DIST_TYPE}, // 78
  {"FUZZ SMILE", FX_DIST_TYPE}, // 79
  {"GREAT MUFF", FX_DIST_TYPE}, // 80
  {"METAL WORLD", FX_DIST_TYPE}, // 81
  {"HOTBOX", FX_DIST_TYPE}, // 82
  {"Z WILD", FX_DIST_TYPE}, // 83
  {"LEAD ZOOM9002", FX_DIST_TYPE}, // 84
  {"EXTREME DIST", FX_DIST_TYPE}, // 85
  {"ACOUSTIC", FX_AMP_TYPE}, // 86
  {"Z CLEAN", FX_AMP_TYPE}, // 87
  {"Z MP1", FX_AMP_TYPE}, // 88
  {"BOTTOM", FX_AMP_TYPE}, // 89
  {"Z DREAM", FX_AMP_TYPE}, // 90
  {"Z SCREAM", FX_AMP_TYPE}, // 91
  {"Z NEOS", FX_AMP_TYPE}, // 92
  {"FD COMBO", FX_AMP_TYPE}, // 93
  {"VX COMBO", FX_AMP_TYPE}, // 94
  {"US BLUES", FX_AMP_TYPE}, // 95
  {"BG CRUNCH", FX_AMP_TYPE}, // 96
  {"HW STACK", FX_AMP_TYPE}, // 97
  {"TANGERINE", FX_AMP_TYPE}, // 98
  {"MS CRUNCH", FX_AMP_TYPE}, // 99
  {"MS DRIVE", FX_AMP_TYPE}, // 100
  {"BG DRIVE", FX_AMP_TYPE}, // 101
  {"DZ DRIVE", FX_AMP_TYPE}, // 102
  {"TW ROCK", FX_AMP_TYPE}, // 103
  {"MATCH30", FX_AMP_TYPE}, // 104
  {"FD VIBRO", FX_AMP_TYPE}, // 105
  {"HD REVERB", FX_AMP_TYPE}, // 106
  {"FLANGER", FX_MODULATE_TYPE}, // 107
  {"---", FX_TYPE_OFF}, // 108
  {"TONE CITY", FX_AMP_TYPE}, // 109
  {"B-BREAKER", FX_AMP_TYPE}, // 110
  {"BGN DRIVE", FX_AMP_TYPE}, // 111
  {"DELUXE R", FX_AMP_TYPE}, // 112
  {"ALIEN", FX_AMP_TYPE}, // 113
  {"REVO 1", FX_AMP_TYPE}, // 114
  {"CAR DRIVE", FX_AMP_TYPE}, // 115
  {"MS 1959", FX_AMP_TYPE}, // 116
  {"VX JIMI", FX_AMP_TYPE}, // 117
  {"118", FX_TYPE_OFF}, // 118
  {"119", FX_TYPE_OFF}, // 119
  {"120", FX_TYPE_OFF}, // 120
};

const uint16_t ZG3_NUMBER_OF_FX = sizeof(ZG3_FX_types) / sizeof(ZG3_FX_types[0]);

void MD_ZG3_class::parameter_press(uint8_t Sw, Cmd_struct * cmd, uint16_t number) {
  // Send sysex MIDI command to Zoom G3
  uint8_t value = SCO_return_parameter_value(Sw, cmd);
  set_FX_state(SP[Sw].PP_number, value & 0x01);

  if (LCD_check_popup_allowed(Sw)) LCD_show_popup_label(SP[Sw].Label, ACTION_TIMER_LENGTH);
}

void MD_ZG3_class::parameter_release(uint8_t Sw, Cmd_struct * cmd, uint16_t number) {
  if (SP[Sw].Latch == MOMENTARY) {
    set_FX_state(SP[Sw].PP_number, cmd->Value2);
  }
}

// Parameters are the 6 FX buttons
void MD_ZG3_class::read_parameter_title(uint16_t number, String &Output) {
  Output += "FX " + String(number + 1);
}

void MD_ZG3_class::read_parameter_title_short(uint16_t number, String &Output) {
  uint8_t FX_type = FX[number] >> 1; //The FX type is stored in bit 1-7.
  Output += String(number + 1) + ':' + ZG3_FX_types[FX_type].Name;
}

FLASHMEM bool MD_ZG3_class::request_parameter(uint8_t sw, uint16_t number) {
  //Effect type and state are stored in the ZG3_FX array
  //Effect can have three states: 0 = no effect, 1 = on, 2 = off
  if (number < NUMBER_OF_FX_SLOTS) {
    if (FX[number] & 0x01) SP[sw].State = 1; //Effect on
    else SP[sw].State = 2; // Effect off
  }

  uint8_t FX_type = FX[number] >> 1; //The FX type is stored in bit 1-7.
  LCD_set_SP_label(sw, ZG3_FX_types[FX_type].Name); //Find the patch name in the ZG3_FX_types array
  SP[sw].Colour = ZG3_FX_types[FX_type].Colour; //Find the LED colour in the ZG3_FX_types array
  return true; // Move to next switch is true
}

// Menu options for FX states
void MD_ZG3_class::read_parameter_name(uint16_t number, String & Output) { // Called from menu
  if (number < number_of_parameters())  Output = "FX" + String(number + 1) + " SW";
  else Output = "?";
}

FLASHMEM uint16_t MD_ZG3_class::number_of_parameters() {
  return 6;
}

FLASHMEM uint8_t MD_ZG3_class::number_of_values(uint16_t parameter) {
  if (parameter < number_of_parameters()) return 2; // So far all parameters have two states: on and off
  else return 0;
}

void MD_ZG3_class::read_parameter_value_name(uint16_t number, uint16_t value, String & Output) {
  if (number < number_of_parameters())  {
    if (value == 1) Output += "ON";
    else Output += "OFF";
  }
  else Output += "?";
}
