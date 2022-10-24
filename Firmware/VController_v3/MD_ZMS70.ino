// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: ZOOM MS70-CDR Initialization
// Section 2: ZOOM MS70-CDR common MIDI in functions
// Section 3: ZOOM MS70-CDR common MIDI out functions
// Section 4: ZOOM MS70-CDR program change
// Section 5: ZOOM MS70-CDR parameter control

// ********************************* Section 1: ZOOM MS70-CDR initialization ********************************************

// Zoom MS70CDR settings:
#define ZMS70_MIDI_CHANNEL 1
#define ZMS70_MIDI_PORT USBHMIDI_PORT
#define ZMS70_PATCH_MIN 0
#define ZMS70_PATCH_MAX 49
#define ZMS70_MIDI_TIMER_LENGTH 100 // Minimal time inbetween requests for patch data from MS70

// Documentation of Zoom sysex has been moved to http://www.vguitarforums.com/smf/index.php?topic=4329.msg131444#msg131444 (The ZOOM MS70-CDR v2 MIDI specification)
// The relevant messages are repeated here
// 1) The Zoom responds to an MIDI identity request message with F0 7E 00 (Device ID) 06 02 52 (Manufacturing ID for Zoom) 61 (model number MS70CDR) 00  00 00 32 2E 31 30 F7
#define ZMS70_MODEL_NUMBER 0x61
// 2) The editor keeps sending F0 52 00 61 50 F7. The MS70-CDR does not seem to respond to it. But it may signal editor mode on.
#define ZMS70_EDITOR_MODE_ON 0x50
// 3) If the editor sends F0 52 00 61 33 F7, the MS70-CDR responds with the current Bank number (CC00 and CC32) and the current Program number (PC)
#define ZMS70_REQUEST_CURRENT_PATCH_NUMBER 0x33
// 4) If the editor sends F0 52 00 61 29 F7, the MS70-CDR responds with the current patch in 110 bytes with command number 28. Byte 0x61 - 0x6B contain the patch name. with a strange 0 at position 0x65
#define ZMS70_REQUEST_CURRENT_PATCH 0x29
// 5) The editor then reads all individual patches by sending F0 52 00 61 09 00 00 {00-63} (patch number) F7.
//    The MS70-CDR responds with 156 bytes with command number 08. Byte 0x66 - 0x70 contain the patch name. with an "overflow byte" at position 0x6A.
// 6) At the end the editor sends F0 52 00 61 51 F7 and communication seems to stop.
#define ZMS70_EDITOR_MODE_OFF 0x51
// 7) Switch effect on/off:
//    Switch on effect 1: F0 52 00 61 31 00 00 01 (on) 00 F7, switch off: F0 52 00 61 31 00 00 00 (off) 00 F7
//    Switch on effect 2: F0 52 00 61 31 01 00 01 (on) 00 F7 switch off: F0 52 00 61 31 01 00 00 (off) 00 F7. etc. sixth byte changes consistent for the effect
//    Same message is sent for changing a knob on the MS70-CDR, but byte 7 is not 0x00
// 8) Tempo. set bpm=40: F0 52 00 61 31 03 08 28 00 F7 => 0x28 = 40, bpm=240: F0 52 00 61 31 03 08 7A 01 F7 => 0x7A = 122, 122+128 = 240, so the last two bytes are the tempo.

// Initialize device variables
// Called at startup of VController
FLASHMEM void MD_ZMS70_class::init() // Default values for variables
{
  MD_base_class::init();

  // Variables
  enabled = DEVICE_DETECT; // Default value
  strcpy(device_name, "MS70"); // This line crashes the VController Production Model for no apparent reason - memory leak somewhere? Fixed by changing "Optimize" to fast
  strcpy(full_device_name, "Zoom MS70-cdr");
  patch_min = ZMS70_PATCH_MIN;
  patch_max = ZMS70_PATCH_MAX;
  sysex_delay_length = 5; // time between sysex messages (in msec).
  max_times_no_response = MAX_TIMES_NO_RESPONSE; // The number of times the Zoom G3 does not have to respond before disconnection
  my_LED_colour = 5; // Default value
  MIDI_channel = ZMS70_MIDI_CHANNEL; // Default value
  MIDI_port_manual = MIDI_port_number(ZMS70_MIDI_PORT); // Default value
#if defined(CONFIG_VCTOUCH)
  my_device_page1 = ZMS70_DEFAULT_VCTOUCH_PAGE1; // Default value
  my_device_page2 = ZMS70_DEFAULT_VCTOUCH_PAGE2; // Default value
  my_device_page3 = ZMS70_DEFAULT_VCTOUCH_PAGE3; // Default value
  my_device_page4 = ZMS70_DEFAULT_VCTOUCH_PAGE4; // Default value
#elif defined(CONFIG_VCMINI)
  my_device_page1 = ZMS70_DEFAULT_VCMINI_PAGE1; // Default value
  my_device_page2 = ZMS70_DEFAULT_VCMINI_PAGE2; // Default value
  my_device_page3 = ZMS70_DEFAULT_VCMINI_PAGE3; // Default value
  my_device_page4 = ZMS70_DEFAULT_VCMINI_PAGE4; // Default value
#elif defined (CONFIG_CUSTOM)
  my_device_page1 = ZMS70_DEFAULT_CUSTOM_PAGE1; // Default value
  my_device_page2 = ZMS70_DEFAULT_CUSTOM_PAGE2; // Default value
  my_device_page3 = ZMS70_DEFAULT_CUSTOM_PAGE3; // Default value
  my_device_page4 = ZMS70_DEFAULT_CUSTOM_PAGE4; // Default value
#else
  my_device_page1 = ZMS70_DEFAULT_VC_PAGE1; // Default value
  my_device_page2 = ZMS70_DEFAULT_VC_PAGE2; // Default value
  my_device_page3 = ZMS70_DEFAULT_VC_PAGE3; // Default value
  my_device_page4 = ZMS70_DEFAULT_VC_PAGE4; // Default value
#endif
  CP_MEM_current = false;
  midi_timer = 0;

#ifdef IS_VCTOUCH
  device_pic = img_ZMS70;
#endif
}

FLASHMEM void MD_ZMS70_class::update() {
  if ((send_patch_change) && (millis() > midi_timer)) { // We delay the sending of a PC message if these come to close together.
    MIDI_send_PC(patch_number, MIDI_channel, MIDI_out_port);
    DEBUGMSG("out(" + String(device_name) + ") PC" + String(patch_number)); //Debug
    do_after_patch_selection();
    midi_timer = millis() + ZMS70_MIDI_TIMER_LENGTH;
    send_patch_change = false;
  }
}

// ********************************* Section 2: ZOOM MS70-CDR common MIDI in functions ********************************************

FLASHMEM void MD_ZMS70_class::check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port) { // Check incoming sysex messages from  Called from MIDI:OnSysEx/OnSerialSysEx

  // Check if it is a message from a ZOOM MS70-CDR
  if ((port == MIDI_in_port) && (sxdata[1] == 0x52) && (sxdata[2] == MIDI_device_id) && (sxdata[3] == ZMS70_MODEL_NUMBER)) {

    // Check if it is patch data for a specific patch
    if ((sxdata[4] == 0x08)  && (sxdata[7] == last_requested_sysex_patch_number)) {
      if  (sxlength == 156) { // Check if full sysex message is read...
        // Read the patch name - needs to be read in three parts, to sail around the overflow bytes.
        // Read the first character of the name
        SP[last_requested_sysex_switch].Label[0] = static_cast<char>(sxdata[0x89]); //Add first character ascii character to the SP.Label String
        for (uint8_t count = 1; count < 8; count++) { // Read the next seven characters of the name
          SP[last_requested_sysex_switch].Label[count] = static_cast<char>(sxdata[count + 0x8A]); //Add byte 2 - 8 to ascii character to the SP.Label String
        }
        for (uint8_t count = 8; count < 10; count++) { // Read the last two characters of the name
          SP[last_requested_sysex_switch].Label[count] = static_cast<char>(sxdata[count + 0x8B]); //Add byte 9 and 10 to ascii character to the SP.Label String
        }
        for (uint8_t count = 10; count < 16; count++) { // Fill the rest with spaces
          SP[last_requested_sysex_switch].Label[count] = ' ';
        }
        DEBUGMSG (SP[last_requested_sysex_switch].Label);

        if (SP[last_requested_sysex_switch].PP_number == patch_number) {
          current_patch_name = SP[last_requested_sysex_switch].Label; // Load patchname when it is read
          update_main_lcd = true; // And show it on the main LCD
        }
        PAGE_request_next_switch();
      }
      else { // Data is not complete
        PAGE_request_current_switch(); // Retry reading data for current switch
      }
    }

    // Check if it is the current patch
    if (sxdata[4] == 0x28) {
      if (sxlength == ZMS70_CURRENT_PATCH_DATA_SIZE) {
        // Copy current patch data to current patch memory - we need it later for changing specific parameters
        memcpy( CP_MEM, sxdata, ZMS70_CURRENT_PATCH_DATA_SIZE );
        CP_MEM_current = true;
        DEBUGMSG("Copied current patch to CP_MEM");

        // Here we read the FX types and states from the current patch memory (CP_MEM)
        FX[0] = FXtypeMangler(CP_MEM[6] + ((CP_MEM[5] & B01000000) << 1), CP_MEM[7], CP_MEM[9]);
        FX[1] = FXtypeMangler(CP_MEM[26] + ((CP_MEM[21] & B00000100) << 5), CP_MEM[27], CP_MEM[30]);
        FX[2] = FXtypeMangler(CP_MEM[47] + ((CP_MEM[45] & B00100000) << 2), CP_MEM[48], CP_MEM[50]);
        FX[3] = FXtypeMangler(CP_MEM[67] + ((CP_MEM[61] & B00000010) << 6), CP_MEM[68], CP_MEM[71]);
        FX[4] = FXtypeMangler(CP_MEM[88] + ((CP_MEM[85] & B00010000) << 3), CP_MEM[89], CP_MEM[91]);
        FX[5] = FXtypeMangler(CP_MEM[108] + ((CP_MEM[101] & B00000001) << 7), CP_MEM[110], CP_MEM[112]);

        // Read the current patch name - needs to be read in two parts, because the fifth byte in the patchname string is a 0.
        current_patch_name[0] = static_cast<char>(CP_MEM[0x84]); //Add first character ascii character to the SP.Label String
        for (uint8_t count = 1; count < 8; count++) { // Read the last nine characters of the name
          current_patch_name[count] = static_cast<char>(CP_MEM[count + 0x85]); //Add byte 2 - 8 to ascii character to the SP.Label String
        }
        for (uint8_t count = 8; count < 10; count++) { // Read the last nine characters of the name
          current_patch_name[count] = static_cast<char>(CP_MEM[count + 0x86]); //Add byte 9 and 10 to ascii character to the SP.Label String
        }

        MIDI_enable_device_check();
        update_main_lcd = true;
        if (popup_patch_name) {
          LCD_show_popup_label(current_patch_name, ACTION_TIMER_LENGTH);
          popup_patch_name = false;
        }
        update_page = REFRESH_FX_ONLY; // Need to update everything, otherwise displays go blank on detection of the MS70-CDR.
      }
      else { // Data is not complete
        write_sysex(ZMS70_REQUEST_CURRENT_PATCH); // Request current patch data again
      }
    }

    if ((sxdata[4] == 0x31) && (sxdata[6] == 0x00) && (sxlength == 10)) { // Check for effect switched off on the ZOOM MS70-CDR.
      uint8_t index = sxdata[5];
      if (index < NUMBER_OF_FX_SLOTS) FX[index] &= B11111110; //Make a zero of bit 1 - this will switch off the effect
      update_page = REFRESH_FX_ONLY;
    }
  }
}

FLASHMEM uint16_t MD_ZMS70_class::FXtypeMangler(uint8_t byte1, uint8_t byte2, uint8_t byte4) {
  // We will read three bytes from the sysex stream. All of them contain some bits that together make the FX type
  // Byte1 is the first byte of every fx slot
  // Bit one of Byte1 signals whether the effect is on or off
  // Bit 5-8 are all relevant for the effect type

  // Byte2 is the second byte of every fx slot
  // AFAIK only the first three bits are relevant for the effect type

  // Byte4 is the fourth byte of every fx stream - this one contain the effect type. The following values are known
  // 02: Reverb?
  // 04: Eq effects
  // 12: Chorus effects
  // 14: Roto effects
  // 16: Delay effects
  // 18: Reverb effects

  // We will search for the right effect type
  uint16_t my_type = FXsearch((byte4 & 0x1F), (byte1 & 0xF8) + (byte2 & 0x0F)); // We include bit 4 from byte1, so the difference between HD Hall and HD reverb is clear.
  DEBUGMSG("Input: type:" + String(byte4 & 0x1F) + ", fx:" + String((byte1 & 0xF8) + (byte2 & 0x0F)));
  // Then we will return the effect index + the effect state
  return (my_type << 1) + (byte1 & 1);
}

// Detection of Zoom MS70-CDR

FLASHMEM void MD_ZMS70_class::identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t in_port, uint8_t out_port) {
  // Check if it is a Zoom MS70-CDR
  if ((sxdata[5] == 0x52) && (sxdata[6] == ZMS70_MODEL_NUMBER) && (sxdata[7] == 0x00) && (enabled == DEVICE_DETECT)) {
    no_response_counter = 0;
    if (connected == false) connect(sxdata[2], in_port, out_port); //Byte 2 contains the correct device ID
  }
}

FLASHMEM void MD_ZMS70_class::do_after_connect() {
  is_on = true;
  write_sysex(ZMS70_EDITOR_MODE_ON); // Put the Zoom MS70-CDR in EDITOR mode
  write_sysex(ZMS70_REQUEST_CURRENT_PATCH_NUMBER); // Receiving the current patch number will trigger a request for the current patch as well.
  CP_MEM_current = false;
  write_sysex(ZMS70_REQUEST_CURRENT_PATCH); //This will update the FX buttons ->
}

// ********************************* Section 3: ZOOM MS70-CDR common MIDI out functions ********************************************

FLASHMEM void MD_ZMS70_class::write_sysex(uint8_t message) {
  uint8_t sysexmessage[6] = {0xF0, 0x52, MIDI_device_id, ZMS70_MODEL_NUMBER, message, 0xF7};
  //check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 6, MIDI_out_port);
}

FLASHMEM void MD_ZMS70_class::request_patch(uint8_t number) { //Will request the complete patch information from the Zoom MS70-CDR (will receive 120 bytes as an answer)
  uint8_t sysexmessage[9] = {0xF0, 0x52, MIDI_device_id, ZMS70_MODEL_NUMBER, 0x09, 0x00, 0x00, number, 0xF7};
  //check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 9, MIDI_out_port);
}

FLASHMEM void MD_ZMS70_class::set_FX_state(uint8_t number, uint8_t state) { //Will set an effect on or off
  uint8_t sysexmessage[10] = {0xF0, 0x52, MIDI_device_id, ZMS70_MODEL_NUMBER, 0x31, number, 0x00, state, 0x00, 0xF7}; // F0 52 00 61 31 00 (FX) 00 01 (on) 00 F7
  //check_sysex_delay();
  MIDI_send_sysex(sysexmessage, 10, MIDI_out_port);
}

FLASHMEM void MD_ZMS70_class::send_current_patch() { //  Send the previously saved data back to the Zoom unit
  if (CP_MEM_current) MIDI_send_sysex(CP_MEM, 146, MIDI_out_port);
}

FLASHMEM void MD_ZMS70_class::set_bpm() { //Will change the bpm to the specified value

  if ((connected) && (CP_MEM_current)) {
    write_tempo_to_cpmem();
    send_current_patch(); // Write the updated patch memory to the MS70-CDR
  }
}

FLASHMEM void MD_ZMS70_class::write_tempo_to_cpmem() {
  // We write the tempo in the full patch memory. Here is where it is stored:
  // Tempo bit 4 - 8 is bit 1-5 from second tempo byte (index 131)
  // Tempo bit 3 is bit 3 of the overflow byte! (index 125)
  // Tempo bit 2 and 1 are first byte bit 2 and 3 (index 130)
  CP_MEM[131] = (CP_MEM[131] & B11100000) | ((Setting.Bpm & B11111000) >> 3);
  CP_MEM[125] = (CP_MEM[125] & B11111011) | (Setting.Bpm & B00000100);
  CP_MEM[130] = (CP_MEM[130] & B10011111) | ((Setting.Bpm & B00000011) << 5);
}

FLASHMEM void MD_ZMS70_class::start_tuner() {
  if (connected) {
    MIDI_send_CC(0x4A, 0x40, MIDI_channel, MIDI_out_port);
  }
}

FLASHMEM void MD_ZMS70_class::stop_tuner() {
  if (connected) {
    MIDI_send_CC(0x4A, 0x00, MIDI_channel, MIDI_out_port);
  }
}

// ********************************* Section 4: MS70-CDR program change ********************************************

FLASHMEM void MD_ZMS70_class::select_patch(uint16_t new_patch) {

  if (new_patch == patch_number) unmute();
  else {
    prev_patch_number = patch_number;
    patch_number = new_patch;
    send_patch_change = true; 
  }
  update_LEDS = true;
  update_main_lcd = true;
}

FLASHMEM void MD_ZMS70_class::do_after_patch_selection() {
  is_on = connected;
  Current_patch_number = patch_number;
  update_LEDS = true;
  update_main_lcd = true;
  delay(5); // short delay after patch change...
  CP_MEM_current = false;
  write_sysex(ZMS70_REQUEST_CURRENT_PATCH); // Request current patch, so the FX buttons will be updated
  MIDI_disable_device_check();
  MD_base_class::do_after_patch_selection();
}

FLASHMEM bool MD_ZMS70_class::request_patch_name(uint8_t sw, uint16_t number) {
  DEBUGMSG("Requesting patch " + String(number));
  if (number > patch_max) return true;
  last_requested_sysex_switch = sw;
  last_requested_sysex_patch_number = number;
  request_patch(number);
  return false;
}

FLASHMEM void MD_ZMS70_class::number_format(uint16_t number, String &Output) {
  Output += String((number + 1) / 10) + String((number + 1) % 10);
}

// ********************************* Section 5: MS70-CDR parameter control ********************************************

// Define array for MS70-CDR effeect names and colours
struct ZMS70_FX_type_struct { // Combines all the data we need for controlling a parameter in a device
  uint8_t Type; // The MS70CDR has a byte for the FX type
  uint8_t Number; // And a number
  char Name[17]; // The name for the label
  uint8_t Colour; // The colour for this effect.
};

const PROGMEM ZMS70_FX_type_struct ZMS70_FX_types[] = { // Table with the name and colour for every effect of the Zoom MS70-CDR
  {0, 0,    "---", FX_TYPE_OFF}, // 01
  {12, 195, "CoronaCho", FX_MODULATE_TYPE}, // 02
  {12, 224, "Chorus", FX_MODULATE_TYPE}, // 03
  {12, 33,  "VintageCE", FX_MODULATE_TYPE}, // 04
  {12, 227, "ANA 234Cho", FX_MODULATE_TYPE}, // 05
  {12, 3,   "CE-Cho5", FX_MODULATE_TYPE}, // 06
  {12, 35,  "Clone Chorus", FX_MODULATE_TYPE}, // 07
  {12, 161, "Super Cho", FX_MODULATE_TYPE}, // 08
  {12, 163, "Mirage Chorus", FX_MODULATE_TYPE}, // 09
  {12, 65,  "Stereo Cho", FX_MODULATE_TYPE}, // 10
  {12, 4,   "Corona Tri", FX_MODULATE_TYPE}, // 11
  {12, 97,  "Ensemble", FX_MODULATE_TYPE}, // 12
  {12, 131, "Silky Chorus", FX_MODULATE_TYPE}, // 13
  {12, 1,   "Detune", FX_MODULATE_TYPE}, // 14
  {12, 240, "Bass Chorus", FX_MODULATE_TYPE}, // 15
  {12, 17,  "Bass Detune", FX_MODULATE_TYPE}, // 16
  {12, 113, "Bass Ensemble", FX_MODULATE_TYPE}, // 17
  {12, 193, "Flanger", FX_MODULATE_TYPE}, // 18
  {12, 209, "Bass Flanger", FX_MODULATE_TYPE}, // 19
  {12, 2,   "Vibrato", FX_MODULATE_TYPE}, // 20
  {12, 96,  "Phaser", FX_MODULATE_TYPE}, // 21
  {12, 128, "The Vibe", FX_MODULATE_TYPE}, // 22
  {12, 66,  "Pitch SHFT", FX_MODULATE_TYPE}, // 23
  {12, 130, "HPS", FX_MODULATE_TYPE}, // 24
  {12, 114, "Bass Pitch", FX_MODULATE_TYPE}, // 25
  {12, 32,  "Duo-Trem", FX_MODULATE_TYPE}, // 26
  {14, 160, "Auto-Pan", FX_MODULATE_TYPE}, // 27
  {14, 192, "Roto Closet", FX_MODULATE_TYPE}, // 28
  {16, 193, "Duo Digital Dly", FX_DELAY_TYPE}, // 29
  {16, 16,  "Delay", FX_DELAY_TYPE}, // 30
  {16, 129, "Stomp Dly", FX_DELAY_TYPE}, // 31
  {16, 33,  "Stereo Delay", FX_DELAY_TYPE}, // 32
  {16, 225, "Carbon Delay", FX_DELAY_TYPE}, // 33
  {16, 96,  "Analog DLY", FX_DELAY_TYPE}, // 34
  {16, 32,  "Tape Echo", FX_DELAY_TYPE}, // 35
  {16, 161, "Tape Echo3", FX_DELAY_TYPE}, // 36
  {16, 2,   "Drive Echo", FX_DELAY_TYPE}, // 37
  {16, 34,  "Slapback Delay", FX_DELAY_TYPE}, // 38
  {16, 66,  "Smooth Delay", FX_DELAY_TYPE}, // 39
  {16, 160, "Multi Tap Delay", FX_DELAY_TYPE}, // 40
  {16, 128, "Reverse Delay", FX_DELAY_TYPE}, // 41
  {16, 98,  "Lo-Fi Delay", FX_DELAY_TYPE}, // 42
  {16, 130, "Slow Attack Dly", FX_DELAY_TYPE}, // 43
  {16, 192, "Dyna Delay", FX_DELAY_TYPE}, // 44
  {16, 162, "Tremolo Delay", FX_DELAY_TYPE}, // 45
  {16, 64,  "ModDelay", FX_DELAY_TYPE}, // 46
  {16, 97,  "Trigger Hold Dly", FX_DELAY_TYPE}, // 47
  {16, 194, "Filter PP-Delay", FX_DELAY_TYPE}, // 48
  {16, 224, "Filter Dly", FX_DELAY_TYPE}, // 49
  {16, 65,  "Phase Delay", FX_DELAY_TYPE}, // 50
  {16, 226, "Auto Pan Delay", FX_DELAY_TYPE}, // 51
  {16, 1,   "Pitch Delay", FX_DELAY_TYPE}, // 52
  {16, 3,   "ICE Delay", FX_DELAY_TYPE}, // 53
  {16, 80,  "ModDelay2", FX_DELAY_TYPE}, // 54
  {18, 24,  "HD Reverb", FX_REVERB_TYPE}, // 55
  {18, 128, "Spring", FX_REVERB_TYPE}, // 56
  {18, 129, "Spring63", FX_REVERB_TYPE}, // 57
  {18, 33,  "Plate", FX_REVERB_TYPE}, // 58
  {18, 64,  "Room", FX_REVERB_TYPE}, // 59
  {18, 96,  "Tiled Room", FX_REVERB_TYPE}, // 60
  {18, 161, "Chamber", FX_REVERB_TYPE}, // 61
  {18, 193, "LoFi Reverb", FX_REVERB_TYPE}, // 62
  {18, 32,  "Hall", FX_REVERB_TYPE}, // 63
  {18, 16,  "HD Hall", FX_REVERB_TYPE}, // 64
  {18, 225, "Church", FX_REVERB_TYPE}, // 65
  {18, 160, "Arena Reverb", FX_REVERB_TYPE}, // 66
  {18, 2,   "Cave", FX_REVERB_TYPE}, // 67
  {18, 34,  "Ambience", FX_REVERB_TYPE}, // 68
  {18, 224, "AIR", FX_REVERB_TYPE}, // 69
  {18, 192, "Early Reflection", FX_REVERB_TYPE}, // 70
  {18, 66,  "Gate Reverb", FX_REVERB_TYPE}, // 71
  {18, 98,  "Revere Reverb", FX_REVERB_TYPE}, // 72
  {18, 97,  "Slap Back Reverb", FX_REVERB_TYPE}, // 73
  {18, 130, "Echo", FX_REVERB_TYPE}, // 74
  {18, 65,  "Mod Reverb", FX_REVERB_TYPE}, // 75
  {18, 162, "Tremelo Reverb", FX_REVERB_TYPE}, // 76
  {18, 194, "Holy Flerb", FX_REVERB_TYPE}, // 77
  {18, 226, "Dynamic Reverb", FX_REVERB_TYPE}, // 78
  {18, 3,   "Shimmer Reverb", FX_REVERB_TYPE}, // 79
  {18, 35,  "Particle Reverb", FX_REVERB_TYPE}, // 80
  {18, 67,  "Space Hole", FX_REVERB_TYPE}, // 81
  {18, 99,  "Mangled Space", FX_REVERB_TYPE}, // 82
  {18, 131, "Dual Reverb", FX_REVERB_TYPE}, // 83
  {2, 160,  "ZNR", FX_DYNAMICS_TYPE}, // 84
  {4, 193,  "Stereo Gtr GEQ", FX_FILTER_TYPE}, // 85
  {4, 2,    "Stereo Bass GEQ", FX_FILTER_TYPE}, // 86
  {4, 16,   "LineSel", FX_GTR_TYPE}, // 87
  {12, 16,  "Tremolo", FX_MODULATE_TYPE}, // New FX from firmware version 2.00
  {12, 64,  "Slicer", FX_MODULATE_TYPE},
  {12, 104, "Duo-Phase", FX_MODULATE_TYPE},
  {12, 112,  "Warp Phaser", FX_MODULATE_TYPE},
  {12, 177, "VinFLNGR", FX_MODULATE_TYPE},
  {12, 225, "DynaFLNGR", FX_MODULATE_TYPE},
  {12, 34,  "Octave", FX_MODULATE_TYPE},
  {12, 50,  "Bass Octave", FX_MODULATE_TYPE},
  {12, 98,  "Mono Pitch", FX_MODULATE_TYPE},
  {12, 162, "Bend Cho", FX_MODULATE_TYPE},
  {12, 194, "Mojo Roller", FX_MODULATE_TYPE},
  {12, 226, "Ring Mod", FX_MODULATE_TYPE},
  {12, 67,  "Stone Phase", FX_MODULATE_TYPE},
  {12, 99,  "BF FLG 2", FX_MODULATE_TYPE},
  {14, 32,  "Bit Crush", FX_MODULATE_TYPE},
  {14, 64,  "Bomber", FX_MODULATE_TYPE},
  {14, 96,  "Mono Synth", FX_MODULATE_TYPE},
  {14, 128, "Z-Organ", FX_MODULATE_TYPE},
  {2, 16,   "COMP", FX_DYNAMICS_TYPE},
  {2, 32,   "Rock Comp", FX_DYNAMICS_TYPE},
  {2, 64,   "M Comp", FX_DYNAMICS_TYPE},
  {2, 96,   "Opto Comp", FX_DYNAMICS_TYPE},
  {2, 104,  "160 COMP", FX_DYNAMICS_TYPE},
  {2, 112,  "Limiter", FX_DYNAMICS_TYPE},
  {2, 128,  "Slow Attack", FX_FILTER_TYPE},
  {2, 192,  "Noise Gate", FX_DYNAMICS_TYPE},
  {2, 224,  "Dirty Gate", FX_DYNAMICS_TYPE},
  {2, 1,    "Orange Lim", FX_DYNAMICS_TYPE},
  {2, 33,   "Gray Comp", FX_DYNAMICS_TYPE},
  {2, 65,   "Dual Comp", FX_DYNAMICS_TYPE},
  {4, 32,   "Graphic EQ", FX_FILTER_TYPE},
  {4, 48,   "Bass Graphic EQ", FX_FILTER_TYPE},
  {4, 64,   "Para EQ", FX_FILTER_TYPE},
  {4, 72,   "Bass Para EQ", FX_FILTER_TYPE},
  {4, 80,   "Splitter", FX_FILTER_TYPE},
  {4, 88,   "Bottom B", FX_FILTER_TYPE},
  {4, 96,   "Exciter", FX_FILTER_TYPE},
  {4, 128,  "Comb FLTR", FX_FILTER_TYPE},
  {4, 160,  "Auto Wah", FX_WAH_TYPE},
  {4, 176,  "Bass Auto Wah", FX_WAH_TYPE},
  {4, 192,  "Resonance", FX_WAH_TYPE},
  {4, 224,  "Cry", FX_WAH_TYPE},
  {4, 1,    "Slow Filter", FX_FILTER_TYPE},
  {4, 17,   "Z Tron", FX_FILTER_TYPE},
  {4, 33,   "M-Filter", FX_FILTER_TYPE},
  {4, 41,   "A-Filter", FX_FILTER_TYPE},
  {4, 49,   "Bass Cry", FX_WAH_TYPE},
  {4, 65,   "STEP", FX_WAH_TYPE},
  {4, 97,   "SEQ Filter", FX_FILTER_TYPE},
  {4, 129,  "Random Filter", FX_FILTER_TYPE},
  {4, 161,  "FCycle", FX_FILTER_TYPE},
  // Unique effects from MS-50G
  {8, 16,   "FD Combo", FX_AMP_TYPE},
  {8, 32,   "Deluxe-R", FX_AMP_TYPE},
  {8, 64,   "FD Vibro", FX_AMP_TYPE},
  {8, 96,   "US Blues", FX_AMP_TYPE},
  {8, 128,  "VX Combo", FX_AMP_TYPE},
  {8, 160,  "VX Jimi", FX_AMP_TYPE},
  {8, 192,  "BG Crunch", FX_AMP_TYPE},
  {8, 224,  "Match30", FX_AMP_TYPE},
  {8, 1,    "Car Drive", FX_AMP_TYPE},
  {8, 33,   "TW Rock", FX_AMP_TYPE},
  {8, 65,   "Tone City", FX_AMP_TYPE},
  {8, 97,   "HW Stack", FX_AMP_TYPE},
  {8, 129,  "Tangerine", FX_AMP_TYPE},
  {8, 161,  "B-Breaker", FX_AMP_TYPE},
  {8, 193,  "MS Crunch", FX_AMP_TYPE},
  {8, 225,  "MS 1959", FX_AMP_TYPE},
  {8, 2,    "MS Drive", FX_AMP_TYPE},
  {8, 34,   "BGN DRV", FX_AMP_TYPE},
  {8, 66,   "BG Drive", FX_AMP_TYPE},
  {8, 98,   "DZ Drive", FX_AMP_TYPE},
  {8, 130,  "Alien", FX_AMP_TYPE},
  {8, 162,  "Revo T", FX_AMP_TYPE},
  {6, 16,   "Booster", FX_DIST_TYPE},
  {6, 32,   "Overdrive", FX_DIST_TYPE},
  {6, 64,   "T Scream", FX_DIST_TYPE},
  {6, 96,   "Governor", FX_DIST_TYPE},
  {6, 128,  "Dist +", FX_DIST_TYPE},
  {6, 160,  "Dist 1", FX_DIST_TYPE},
  {6, 192,  "Squeak", FX_DIST_TYPE},
  {6, 224,  "Fuzz Smile", FX_DIST_TYPE},
  {6, 1,    "Great Muff", FX_DIST_TYPE},
  {6, 65,   "HotBox", FX_DIST_TYPE},
  {6, 97,   "Z Clean", FX_DIST_TYPE},
  {6, 129,  "Z MP1", FX_DIST_TYPE},
  {6, 161,  "Z Bottom", FX_DIST_TYPE},
  {6, 193,  "Z Dream", FX_DIST_TYPE},
  {6, 225,  "Z Scream", FX_DIST_TYPE},
  {6, 2,    "Z Neos", FX_DIST_TYPE},
  {6, 34,   "Z Wild", FX_DIST_TYPE},
  {6, 66,   "Zoom 9002 Lead", FX_DIST_TYPE},
  {6, 98,   "Extreme Dist", FX_DIST_TYPE},
  {6, 130,  "Acoustic", FX_DIST_TYPE},
  {6, 162,  "CentaGold", FX_DIST_TYPE},
  {6, 194,  "NYC Muff", FX_DIST_TYPE},
  {6, 226,  "TS Drive", FX_DIST_TYPE},
  {6, 3,    "BG Throttle", FX_DIST_TYPE},
  {6, 35,   "Oct Fuzz", FX_DIST_TYPE},
  {6, 67,   "BG Grid", FX_DIST_TYPE},
  {6, 99,   "Red Crunch", FX_DIST_TYPE},
  {6, 131,  "TB MK 1.5", FX_DIST_TYPE},
  {6, 163,  "Sweet Drive", FX_DIST_TYPE},
  {6, 227,  "RC Boost", FX_DIST_TYPE},
  {6, 36,   "Dynamic Drive", FX_DIST_TYPE},
  // Unique effects from MS-60B that do not crash the MS70-cdr
  {24, 65,  "T Scream", FX_DIST_TYPE},
  {10, 128, "SMR", FX_AMP_TYPE},
  {10, 160, "Flip Top", FX_AMP_TYPE},
  {10, 1,   "Monotone", FX_AMP_TYPE},
  {10, 33,  "Super B", FX_AMP_TYPE},
  {10, 65,  "G-Krueger", FX_AMP_TYPE},
  {10, 97,  "The Heaven", FX_AMP_TYPE},
  {14, 112, "Bass Synth", FX_FILTER_TYPE},
  {14, 224, "Std Syn", FX_FILTER_TYPE},
  {14, 1,   "Syn Tlk", FX_FILTER_TYPE},
  {14, 65,  "Defret", FX_FILTER_TYPE},
  {14, 129, "V-Syn", FX_FILTER_TYPE},
  {14, 161, "4VoiceSyn", FX_FILTER_TYPE},
  {2, 48,   "D Comp", FX_DYNAMICS_TYPE},
};

const uint8_t ZMS70_NUMBER_OF_FX = sizeof(ZMS70_FX_types) / sizeof(ZMS70_FX_types[0]);

FLASHMEM uint8_t MD_ZMS70_class::FXsearch(uint8_t type, uint8_t number) {
  uint8_t my_type = 0;
  for (uint8_t i = 0; i < ZMS70_NUMBER_OF_FX; i++) {
    if ((ZMS70_FX_types[i].Type == type) && (ZMS70_FX_types[i].Number == number)) {
      my_type = i;
      break; // break loop when found
    }
  }
  return my_type; // Return index to the effect type
}

FLASHMEM void MD_ZMS70_class::parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {
  // Send sysex MIDI command to Zoom G3
  uint8_t value = SCO_return_parameter_value(Sw, cmd);
  bool FX_on = (value & 1);
  if (CP_MEM_current) { // Here we change the on/off bit in the patch data memory and write it back to the unit
    switch (SP[Sw].PP_number) {
      case 0: bitWrite(CP_MEM[6], 0, FX_on); break;
      case 1: bitWrite(CP_MEM[26], 0, FX_on); break;
      case 2: bitWrite(CP_MEM[47], 0, FX_on); break;
      case 3: bitWrite(CP_MEM[67], 0, FX_on); break;
      case 4: bitWrite(CP_MEM[88], 0, FX_on); break;
      case 5: bitWrite(CP_MEM[108], 0, FX_on); break;
    }
    write_active_fx_to_cpmem(SP[Sw].PP_number);
    write_tempo_to_cpmem();
    send_current_patch(); // Write the updated patch memory to the MS70-CDR
  }
  else if (SP[Sw].PP_number < 3) { // This command will only work for the first three FX on the MS70-CDR
    set_FX_state(SP[Sw].PP_number, FX_on);
  }

  LCD_show_popup_label(SP[Sw].Label, ACTION_TIMER_LENGTH);
}

FLASHMEM void MD_ZMS70_class::parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {
  if (SP[Sw].Latch == MOMENTARY) {
    if (CP_MEM_current) { // Here we change the on/off bit in the patch data memory and write it back to the unit
      switch (SP[Sw].PP_number) {
        case 0: bitWrite(CP_MEM[6], 0, cmd->Value2); break;
        case 1: bitWrite(CP_MEM[26], 0, cmd->Value2); break;
        case 2: bitWrite(CP_MEM[47], 0, cmd->Value2); break;
        case 3: bitWrite(CP_MEM[67], 0, cmd->Value2); break;
        case 4: bitWrite(CP_MEM[88], 0, cmd->Value2); break;
        case 5: bitWrite(CP_MEM[108], 0, cmd->Value2); break;
      }
      write_active_fx_to_cpmem(SP[Sw].PP_number);
      write_tempo_to_cpmem();
      send_current_patch(); // Write the updated patch memory to the MS70-CDR
    }
    else if (SP[Sw].PP_number < 3) { // This command will only work for the first three FX on the MS70-CDR
      set_FX_state(SP[Sw].PP_number, cmd->Value2);
    }
  }
}

FLASHMEM void MD_ZMS70_class::write_active_fx_to_cpmem(uint8_t fx_no) {
  //Byte 125, 129 en 130 are changing for block selection:
  //Block 1: 125, bit 4 = 0, 129 bit 7 = 1, 130 bit 0 = 1
  //Block 2: 125, bit 4 = 0, 129 bit 7 = 0, 130 bit 0 = 1
  //Block 3: 125, bit 4 = 1, 129 bit 7 = 1, 130 bit 0 = 0
  //Block 4: 125, bit 4 = 1, 129 bit 7 = 0, 130 bit 0 = 0
  //Block 5: 125, bit 4 = 0, 129 bit 7 = 1, 130 bit 0 = 0
  //Block 6: 125, bit 4 = 0, 129 bit 7 = 0, 130 bit 0 = 0
  const uint8_t bitState[6] = {B011, B001, B110, B100, B010, B000 };
  if (fx_no < 6) {
    bitWrite(CP_MEM[125], 3, (bitState[fx_no] & B100) >> 2);
    bitWrite(CP_MEM[129], 6, (bitState[fx_no] & B010) >> 1);
    bitWrite(CP_MEM[130], 0, bitState[fx_no] & B001);
  }
}

// Parameters are the 6 FX buttons
FLASHMEM void MD_ZMS70_class::read_parameter_title(uint16_t number, String &Output) {
  Output += "FX " + String(number + 1);
}

FLASHMEM void MD_ZMS70_class::read_parameter_title_short(uint16_t number, String &Output) {
  uint8_t FX_type = FX[number] >> 1; //The FX type is stored in bit 1-7.
  Output += String(number + 1) + ':' + ZMS70_FX_types[FX_type].Name;
}

FLASHMEM bool MD_ZMS70_class::request_parameter(uint8_t sw, uint16_t number) {
  //Effect type and state are stored in the ZMS70_FX array
  //Effect can have three states: 0 = no effect, 1 = on, 2 = off
  if (number < NUMBER_OF_FX_SLOTS) {
    if (FX[number] & 0x01) SP[sw].State = 1; //Effect on
    else SP[sw].State = 2; // Effect off
  }

  uint8_t FX_type = FX[number] >> 1; //The FX type is stored in bit 1-7.
  LCD_set_SP_label(sw, ZMS70_FX_types[FX_type].Name); //Find the patch name in the ZMS70_FX_types array
  SP[sw].Colour = ZMS70_FX_types[FX_type].Colour; //Find the LED colour in the ZMS70_FX_types array
  return true; // Move to next switch is true
}

// Menu options for FX states
FLASHMEM void MD_ZMS70_class::read_parameter_name(uint16_t number, String &Output) { // Called from menu
  if (number < number_of_parameters())  Output = "FX" + String(number + 1) + " SW";
  else Output = "?";
}

FLASHMEM uint16_t MD_ZMS70_class::number_of_parameters() {
  return 6;
}

FLASHMEM uint8_t MD_ZMS70_class::number_of_values(uint16_t parameter) {
  if (parameter < number_of_parameters()) return 2; // So far all parameters have two states: on and off
  else return 0;
}

FLASHMEM void MD_ZMS70_class::read_parameter_value_name(uint16_t number, uint16_t value, String &Output) {
  if (number < number_of_parameters())  {
    if (value == 1) Output += "ON";
    else Output += "OFF";
  }
  else Output += "?";
}
