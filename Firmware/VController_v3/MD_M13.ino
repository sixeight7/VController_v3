 // Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: M13 Initialization
// Section 2: M13 common MIDI in functions
// Section 3: M13 common MIDI out functions
// Section 4: M13 program change
// Section 5: M13 parameter control
// Section 6: M13 expression pedal control
// Section 7: M13 looper control

// ********************************* Section 1: M13 Initialization ********************************************

// Line6 M13 settings:
#define M13_MIDI_CHANNEL 1
#define M13_MIDI_PORT MIDI1_PORT
#define M13_PATCH_MIN 0
#define M13_PATCH_MAX 11

// Response to universal midi message: F0 7E 7F 06 02 00 01 0C 0E 40 00 00 30 32 30 34 F7
// 00 01 0C is Line6
// 0E = M13

// Request full program
// 0xF0, 0x00, 0x01, 0x0C, 0x0E, 0x00, 0x03, number, 0xF7 - will send 16 sysex messages of 157 bytes

#define M13_NUMBER_OF_SCENES 12

// Note: the timing is crucial for getting a good read from the M13. Values have been determined by experimentation
#define M13_TIMEOUT_FOR_READING_CURRENT_SCENE 1000
#define M13_TIME_BEFORE_FIRST_SCENE_READ 500
#define M13_DELAY_AFTER_SCENE_READ 10

struct M13_data_struct {
  char Name[17]; // Scene name
  uint8_t State[4]; // States of FX1-4
  uint8_t Category[12]; // Category of each of the FX
  uint8_t Effect[12]; // The number of each of the FX
};

M13_data_struct *M13_data; // Memory will we be allocated after M13 connects.

// Initialize device variables
// Called at startup of VController
FLASHMEM void MD_M13_class::init() { // Default values for variables
  MD_base_class::init();

  // Line6 M13 variables:
  enabled = DEVICE_DETECT; // Default value
  strcpy(device_name, "M13");
  strcpy(full_device_name, "Line6 M13");
  patch_min = M13_PATCH_MIN;
  patch_max = M13_PATCH_MAX;
  max_times_no_response = MAX_TIMES_NO_RESPONSE; // The number of times the M13 does not have to respond before disconnection
  sysex_delay_length = 0; // time between sysex messages (in msec).
  my_LED_colour = 6; // Default value: white
  MIDI_channel = M13_MIDI_CHANNEL; // Default value
  MIDI_port_manual = MIDI_port_number(M13_MIDI_PORT); // Default value
#if defined(CONFIG_VCTOUCH)
  my_device_page1 = M13_DEFAULT_VCTOUCH_PAGE1; // Default value
  my_device_page2 = M13_DEFAULT_VCTOUCH_PAGE2; // Default value
  my_device_page3 = M13_DEFAULT_VCTOUCH_PAGE3; // Default value
  my_device_page4 = M13_DEFAULT_VCTOUCH_PAGE4; // Default value
#elif defined(CONFIG_VCMINI)
  my_device_page1 = M13_DEFAULT_VCMINI_PAGE1; // Default value
  my_device_page2 = M13_DEFAULT_VCMINI_PAGE2; // Default value
  my_device_page3 = M13_DEFAULT_VCMINI_PAGE3; // Default value
  my_device_page4 = M13_DEFAULT_VCMINI_PAGE4; // Default value
#elif defined (CONFIG_CUSTOM)
  my_device_page1 = M13_DEFAULT_CUSTOM_PAGE1; // Default value
  my_device_page2 = M13_DEFAULT_CUSTOM_PAGE2; // Default value
  my_device_page3 = M13_DEFAULT_CUSTOM_PAGE3; // Default value
  my_device_page4 = M13_DEFAULT_CUSTOM_PAGE4; // Default value
#else
  my_device_page1 = M13_DEFAULT_VC_PAGE1; // Default value
  my_device_page2 = M13_DEFAULT_VC_PAGE2; // Default value
  my_device_page3 = M13_DEFAULT_VC_PAGE3; // Default value
  my_device_page4 = M13_DEFAULT_VC_PAGE4; // Default value
#endif
  tuner_active = false;
  memset(par_on, 0, M13_PAR_ON_SIZE);
  //overdub = false;
  midi_timer = 0;
  max_looper_length = 30000000; // Normal stereo looper time is 30 seconds - time given in microseconds

#ifdef IS_VCTOUCH
  device_pic = img_M13;
#endif
}

FLASHMEM void MD_M13_class::update() {
  if (!connected) return;
  if (midi_timer > 0) { // Check timer is running
    if (millis() > midi_timer) {
      DEBUGMSG("M13 Midi timer expired!");
      send_midi_request_for_scene(read_scene);
    }
  }

  looper_timer_check();
}

// ********************************* Section 2: M13 common MIDI in functions ********************************************

FLASHMEM void MD_M13_class::check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port) { // Check incoming sysex messages from  Called from MIDI:OnSysEx/OnSerialSysEx

  // Check if it is a message from a M13
  if ((port == MIDI_in_port) && (sxdata[1] == 0x00) && (sxdata[2] == 0x01) && (sxdata[3] == 0x0C) && (sxdata[4] == 0x0E) && (sxdata[5] == 0x01)) {

    if (sxdata[6] <= read_scene_byte) { // Check if we are reading the proper part of the scene
      read_scene_byte++;
      no_response_counter = 0; // So the connection will not be dropped during this large sysex dump
      switch (sxdata[6]) {
        case 0x02:
          for (uint8_t count = 0; count < 7; count++) {
            M13_data[read_scene].Name[count] = static_cast<char>(sxdata[count + 9]); //Add ascii character to the SP.Label String
          }
          for (uint8_t count = 7; count < 14; count++) {
            M13_data[read_scene].Name[count] = static_cast<char>(sxdata[count + 10]); //Add ascii character to the SP.Label String
          }
          for (uint8_t count = 14; count < 16; count++) {
            M13_data[read_scene].Name[count] = static_cast<char>(sxdata[count + 11]); //Add ascii character to the SP.Label String
          }

          if (read_scene == patch_number) {
            current_patch_name = M13_data[read_scene].Name; // Set current patchname when it is read
            update_main_lcd = true; // And show it on the main LCD
          }

          // Store FX1 state
          if (sxdata[0x56] == 1) M13_data[read_scene].State[0] = sxdata[0x57] + 1; // For switch A,B or C active
          else M13_data[read_scene].State[0] = 0;

          // Store FX type and category for FX 1A
          M13_data[read_scene].Effect[0] = sxdata[0x5F];
          M13_data[read_scene].Category[0] = sxdata[0x62];
          break;

        case 0x03:
          // Store FX type and category for FX 1B
          M13_data[read_scene].Effect[1] = sxdata[0x44];
          M13_data[read_scene].Category[1] = sxdata[0x46];
          break;

        case 0x04:
          // Store FX type and category for FX 1C
          M13_data[read_scene].Effect[2] = sxdata[0x29];
          M13_data[read_scene].Category[2] = sxdata[0x2B];
          break;

        case 0x05:
          // Store FX2 state
          if (sxdata[0x0D] == 1) M13_data[read_scene].State[1] = sxdata[0x0E] + 1; // For switch A,B or C active
          else M13_data[read_scene].State[1] = 0;

          // Store FX type and category for FX 2A
          M13_data[read_scene].Effect[3] = sxdata[0x16];
          M13_data[read_scene].Category[3] = sxdata[0x19];

          // Store FX type and category for FX 2B
          M13_data[read_scene].Effect[4] = sxdata[0x8D];
          M13_data[read_scene].Category[4] = sxdata[0x8F];
          break;

        case 0x06:
          // Store FX type and category for FX 2C
          M13_data[read_scene].Effect[5] = sxdata[0x72];
          M13_data[read_scene].Category[5] = sxdata[0x74];
          break;

        case 0x07:
          // Store FX3 state
          if (sxdata[0x56] == 1) M13_data[read_scene].State[2] = sxdata[0x57] + 1; // For switch A,B or C active
          else M13_data[read_scene].State[2] = 0;

          // Store FX type and category for FX 3A
          M13_data[read_scene].Effect[6] = sxdata[0x5F];
          M13_data[read_scene].Category[6] = sxdata[0x62];
          break;

        case 0x08:
          // Store FX type and category for FX 3B
          M13_data[read_scene].Effect[7] = sxdata[0x44];
          M13_data[read_scene].Category[7] = sxdata[0x46];
          break;

        case 0x09:
          // Store FX type and category for FX 3C
          M13_data[read_scene].Effect[8] = sxdata[0x29];
          M13_data[read_scene].Category[8] = sxdata[0x2B];
          break;

        case 0x0A:
          // Store FX4 state
          if (sxdata[0x0D] == 1) M13_data[read_scene].State[3] = sxdata[0x0E] + 1; // For switch A,B or C active
          else M13_data[read_scene].State[3] = 0;

          // Store FX type and category for FX 4A
          M13_data[read_scene].Effect[9] = sxdata[0x16];
          M13_data[read_scene].Category[9] = sxdata[0x19];

          // Store FX type and category for FX 4B
          M13_data[read_scene].Effect[10] = sxdata[0x8D];
          M13_data[read_scene].Category[10] = sxdata[0x8F];
          break;

        case 0x0B:
          // Store FX type and category for FX 4C
          M13_data[read_scene].Effect[11] = sxdata[0x72];
          M13_data[read_scene].Category[11] = sxdata[0x74];
          break;

        case 0x0F: // The last message of the patch dump
          LCD_show_popup_label("Scene " + String(read_scene + 1) + "/12 read", MESSAGE_TIMER_LENGTH);
          if (read_scene < 11) { // Request the next scene
            read_scene++;
            //read_scene_byte = 0;
            //delay(10); // Essential for getting correct read from the M13!
            request_scene(read_scene);

          }
          else {
            MIDI_enable_device_check();
            midi_timer = 0; // Stop timer
            if (Setting.Send_global_tempo_after_patch_change == true) { // Retapping tempo messes with the scene reading - it should be done after scene reading is done
              SCO_retap_tempo();
            }
            update_page = REFRESH_PAGE;
          }
          break;
      }
    }
  }
}

FLASHMEM void MD_M13_class::check_PC_in(uint8_t program, uint8_t channel, uint8_t port) {  // Check incoming PC messages from  Called from MIDI:OnProgramChange

  // Check the source by checking the channel
  if ((port == MIDI_in_port) && (channel == MIDI_channel)) { // M13 sends a program change
    if (patch_number != program) {
      set_patch_number(program);
      do_after_patch_selection();
      update_page = REFRESH_PAGE;
    }
  }
}

FLASHMEM void MD_M13_class::check_CC_in(uint8_t control, uint8_t value, uint8_t channel, uint8_t port) {
  // Check the source by checking the channel
  if ((port == MIDI_in_port) && (channel == MIDI_channel)) { // M13 sends a CC message
    if ((control >= 11) && (control <= 22)) { // FX on M13 switched on or off
      uint8_t number = control - 11;
      uint8_t FX = number / 3;
      uint8_t state = (number % 3) + 1;
      if (value == 0) state = 0;
      M13_data[patch_number].State[FX] = state; // update state on VController
      update_page = REFRESH_PAGE;
    }
    if (control == 64) SCO_global_tap_external(); // Tap tempo on M13 pressed
    if (control == 69) tuner_active = (value > 64); // Tuner mode on M13 engaged
  }
}

// Detection of M13

FLASHMEM void MD_M13_class::check_still_connected() { // Started from MIDI/MIDI_check_all_devices_still_connected()
  if ((connected) && (enabled == DEVICE_DETECT) && (tuner_active == false)) {
    //DEBUGMSG(device_name + " not detected times " + String(no_response_counter));
    if (no_response_counter >= max_times_no_response) disconnect();
    no_response_counter++;
  }
}


FLASHMEM void MD_M13_class::identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t in_port, uint8_t out_port) {
  // Check if it is a M13
  if ((sxdata[5] == 0x00) && (sxdata[6] == 0x01) && (sxdata[7] == 0x0C) && (sxdata[8] == 0x0E) && (enabled == DEVICE_DETECT)) {
    no_response_counter = 0;
    if (connected == false) connect(sxdata[2], in_port, out_port); //Byte 2 contains the correct device ID
  }
}

FLASHMEM void MD_M13_class::do_after_connect() {
  // Allocate memory to the M13 data struct
  if (M13_data == NULL) M13_data = (struct M13_data_struct *) malloc(M13_NUMBER_OF_SCENES * sizeof(struct M13_data_struct));
  memset(M13_data, 0, M13_NUMBER_OF_SCENES * sizeof(struct M13_data_struct));

  // Start reading scenes
  request_scene(0); // Request the first scene
  MIDI_disable_device_check();
  do_after_patch_selection();
  current_exp_pedal = 1;
  update_page = REFRESH_PAGE;
}

FLASHMEM void MD_M13_class::do_after_disconnect() {
  free(M13_data);
  M13_data = NULL;
}


// ********************************* Section 3: M13 common MIDI out functions ********************************************
FLASHMEM void MD_M13_class::request_scene(uint8_t number) {
  read_scene = number;
  read_scene_byte = 0;
  if (number == 0) midi_timer = millis() + M13_TIME_BEFORE_FIRST_SCENE_READ;
  else midi_timer = millis() + M13_DELAY_AFTER_SCENE_READ;
}

FLASHMEM void MD_M13_class::send_midi_request_for_scene(uint8_t number) {
  uint8_t sysexmessage[9] = {0xF0, 0x00, 0x01, 0x0C, 0x0E, 0x00, 0x03, number, 0xF7};
  DEBUGMSG("M13 request scene " + String(number));
  MIDI_send_sysex(sysexmessage, 9, MIDI_out_port);
  midi_timer = millis() + M13_TIMEOUT_FOR_READING_CURRENT_SCENE; // Set the timer
  MIDI_disable_device_check();
}

FLASHMEM void MD_M13_class::bpm_tap() {
  if (connected) {
    MIDI_send_CC(64, 127, MIDI_channel, MIDI_out_port); // Tap tempo on M13
  }
}

FLASHMEM void MD_M13_class::start_tuner() {
  if (connected) {
    MIDI_send_CC(69, 127, MIDI_channel, MIDI_out_port); // Start tuner on M13
    tuner_active = true;
  }
}

FLASHMEM void MD_M13_class::stop_tuner() {
  if (connected) {
    MIDI_send_CC(69, 0, MIDI_channel, MIDI_out_port); // Stop tuner on M13
    tuner_active = false;
  }
}

// ********************************* Section 4: M13 program change ********************************************

FLASHMEM void MD_M13_class::do_after_patch_selection() {
  is_on = connected;
  if ((Setting.Send_global_tempo_after_patch_change == true) && (read_scene >= 11)) { // Retapping tempo messes with the scene reading - it should be done after scene reading is done
    SCO_retap_tempo();
  }
  Current_patch_number = patch_number;
  update_LEDS = true;
  update_main_lcd = true;
  MD_base_class::do_after_patch_selection();
}

FLASHMEM bool MD_M13_class::request_patch_name(uint8_t sw, uint16_t number) {
  if (M13_data == NULL) {
    LCD_clear_SP_label(sw);
    return true;
  }
  String msg = M13_data[number].Name;
  if (number < M13_NUMBER_OF_SCENES) LCD_set_SP_label(sw, msg);
  //PAGE_request_next_switch();
  return true;
}

// ********************************* Section 5: M13 parameter control ********************************************
// Define array for M13 effeect names and colours
struct M13_parameter_struct { // Combines all the data we need for controlling a parameter in a device
  char Name[17]; // The name for the label
  char Short_name[5]; // The short label for this effect
  uint8_t CC; // The colour for this effect
};

const PROGMEM M13_parameter_struct M13_parameters[] = {
  {"FX 1A", "1A", 11},
  {"FX 1B", "1B", 12},
  {"FX 1C", "1C", 13},
  {"FX 2A", "2A", 14},
  {"FX 2B", "2B", 15},
  {"FX 2C", "2C", 16},
  {"FX 3A", "3A", 17},
  {"FX 3B", "3B", 18},
  {"FX 3C", "3C", 19},
  {"FX 4A", "4A", 20},
  {"FX 4B", "4B", 21},
  {"FX 4C", "4C", 22},
  {"BYPASS ALL+LOOP", "BY+L", 23},
  {"BYPASS ALL-LOOP", "BY-L", 24},
  {"M13 EXP1", "EXP1", 1},
  {"M13 EXP2", "EXP2", 2},
};

const uint16_t M13_NUMBER_OF_PARAMETERS = sizeof(M13_parameters) / sizeof(M13_parameters[0]);

#define M13_SW_BYPASS 12
#define M13_SW_BYPASS_LOOP 13
#define M13_SW_EXP1 14
#define M13_SW_EXP2 15

struct M13_FX_type_struct { // Combines all the data we need for controlling a parameter in a device
  uint8_t Type; // The M13 FX type
  uint8_t Number; // And the FX number
  char Name[17]; // The name for the label
};

const PROGMEM M13_FX_type_struct M13_FX_types[] = { // Table with the name of every effect of the M13
  {0, 0, "---"},
  {2, 22, "Tube Echo" },
  {2, 24, "Tape Echo"  },
  {2, 23, "Tape EchoDryThru"  },
  {2, 25, "Tape EchoDryThru"  }, // Why 2?
  {2, 26, "Multi Head" },
  {2, 20, "Analog Echo"  },
  {2, 21, "Analog W/Mod" },
  {2, 27, "Sweep Echo"  },
  {2, 28, "SweepEchoDryThru"  },
  {2, 29, "Lo Res Delay" },
  {2, 17, "Digital Delay"  },
  {2, 18, "Digital Dy w/mod" },
  {2, 19, "Stereo Delay"  },
  {2, 32, "Ping Pong" },
  {2, 33, "Reverse"  },
  {2, 34, "Dynamic Delay" },
  {2, 35, "Auto Volume Dly"  },
  {2, 30, "Echo Platter" },
  {3, 32, "Opto Tremolo"  },
  {3, 61, "Opto Tremolo"  }, // Why 2
  {3, 62, "Bias Tremolo" },
  {3, 68, "Pattern Tremolo"  },
  {3, 52, "Phaser" },
  {3, 34, "Phaser" }, // Why 2?
  {3, 59, "Dual Phaser"  },
  {3, 58, "Panned Phaser" },
  {3, 65, "Barberpole Phsr"  },
  {3, 70, "Script Phase" },
  {3, 56, "U-Vibe"  },
  {3, 53, "Analog Flanger" },
  {3, 54, "Jet Flanger" },
  {3, 72, "AC Flanger " },
  {3, 74, "80A flanger"  },
  {3, 42, "Analog Chorus" },
  {3, 55, "Analog Chorus" }, // Why 2?
  {3, 44, "Tri Chorus" },
  {3, 57, "Tri Chorus" }, // Why 2?
  {3, 43, "Dimension" },
  {3, 60, "Pitch Vibrato" },
  {3, 63, "Panner" },
  {3, 38, "Rotary Drum" },
  {3, 39, "Rotary Drm/Hrn" },
  {3, 46, "Ring Modulator" },
  {3, 66, "Freq. Shifter" },
  {5, 13, "Tube Drive" },
  {5, 14, "Screamer" },
  {5, 16, "Overdrive" },
  {5, 11, "Classic Dist" },
  {5, 22, "Heavy Dist" },
  {5, 24, "Colordrive" },
  {5, 23, "Buzzsaw" },
  {5, 17, "Facial Fuzz" },
  {5, 23, "Jumbo Fuzz" },
  {5, 15, "Fuzz Pi" },
  {5, 10, "Jet Fuzz" },
  {5, 19, "Line6 Drive" },
  {5, 24, "Line6 Distortion" },
  {5, 20, "Sub Octave Fuzz" },
  {5, 16, "Bass octaver" },
  {5, 12, "Octave fuzz" },
  {5, 26, "Boost Comp" },
  {5, 25, "Volume pedal" },
  {0, 11, "Red Comp" },
  {0, 12, "Blue Comp" },
  {0, 13, "Blue Comp Treble" },
  {0, 15, "Vetta Comp" },
  {0, 16, "Vetta Juice" },
  {0, 14, "Tube Comp" },
  {0, 17, "Noise gate" },
  {12, 4, "Graphic EQ" },
  {12, 5, "Studio EQ" },
  {12, 6, "Parametric EQ" },
  {12, 7, "4band shift EQ" },
  {12, 8, "Mid focus EQ, " },
  {10, 15, "Tron up" },
  {10, 16, "Tron down" },
  {10, 17, "Seeker" },
  {10, 18, "Obi Wah" },
  {10, 29, "Voice box" },
  {10, 30, "V tron" },
  {10, 21, "Throbber" },
  {10, 22, "Spin Cycle" },
  {10, 23, "Comet trails" },
  {10, 19, "Slow Filter" },
  {10, 24, "Octisynth" },
  {10, 26, "Synthomatic" },
  {10, 27, "Attack synth" },
  {10, 28, "Synth String" },
  {10, 25, "Growler" },
  {10, 20, "Q Filter" },
  {6, 11, "Vetta Wah" },
  {6, 12, "Fassel" },
  {6, 13, "Weeper" },
  {6, 14, "Chrome" },
  {6, 15, "Chrome custom" },
  {6, 16, "Throaty" },
  {6, 17, "Conductor" },
  {6, 18, "Colorful" },
  {9, 2,  "Smart Harmony" },
  {9, 3 , "Pitch Glide" },
  {4, 28, "63 spring" },
  {4, 29, "Spring" },
  {4, 30, "Plate" },
  {4, 31, "Room" },
  {4, 32, "Chamber" },
  {4, 33, "Hall" },
  {4, 34, "Ducking" },
  {4, 35, "Octo" },
  {4, 36, "Cave" },
  {4, 37, "Tile" },
  {4, 38, "Echo" },
  {4, 39, "Particle verb" },
};

const uint8_t M13_NUMBER_OF_FX = sizeof(M13_FX_types) / sizeof(M13_FX_types[0]);

FLASHMEM uint8_t MD_M13_class::FXsearch(uint8_t type, uint8_t number) {
  for (uint8_t i = 0; i < M13_NUMBER_OF_FX; i++) {
    if ((M13_FX_types[i].Type == type) && (M13_FX_types[i].Number == number)) {
      return i;
    }
  }
  return 0; // Return default
}

FLASHMEM void MD_M13_class::parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {
  if (M13_data == NULL) return;
  // Send cc command to Line6 M13
  uint8_t value = 0; // = SCO_return_parameter_value(Sw, cmd);
  if (number < 12) { // FX UNITS buttons
    uint8_t u = number / 3; // determine the FX unit number (0 - 3)
    if (M13_data[patch_number].State[u] == (number % 3) + 1) { // if FX button was previously switched on
      value = 0;
      M13_data[patch_number].State[u] = 0;
    }
    else {
      value = 1;
      M13_data[patch_number].State[u] = (number % 3) + 1; // Remember the state of this FX unit (A(1), B(2) or C(3))
    }
  }
  else if (number <= M13_PAR_ON_SIZE + 12) {
    par_on[number - 12] = !par_on[number - 12]; // Toggle value

    // Check bypass states
    if (number == M13_SW_BYPASS) par_on[M13_PAR_BYPASS_LOOP] = false;
    else if (number == M13_SW_BYPASS_LOOP) par_on[M13_PAR_BYPASS] = false;

    // Set value according to par_on status
    if (par_on[number - 12]) value = 1;
    else value = 2;
  }

  // Send the CC message
  if (SP[Sw].Latch == RANGE) MIDI_send_CC(M13_parameters[number].CC, SP[Sw].Target_byte1, MIDI_channel, MIDI_out_port);
  else if (value == 1) MIDI_send_CC(M13_parameters[number].CC, 127, MIDI_channel, MIDI_out_port);
  else MIDI_send_CC(M13_parameters[number].CC, 0, MIDI_channel, MIDI_out_port);
  if (LCD_check_popup_allowed(Sw)) LCD_show_popup_label(SP[Sw].Label, ACTION_TIMER_LENGTH);
  update_page = REFRESH_FX_ONLY; // To update the other switch states, we re-load the current page
}

FLASHMEM void MD_M13_class::parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {}

FLASHMEM void MD_M13_class::read_parameter_title(uint16_t number, String &Output) {
  Output += M13_parameters[number].Name;
}

FLASHMEM void MD_M13_class::read_parameter_title_short(uint16_t number, String &Output) {
  if (M13_data == NULL) return;
  Output += M13_parameters[number].Short_name;
  if (number < 12) {
    Output += ':';
    uint8_t FX_type = FXsearch(M13_data[patch_number].Category[number], M13_data[patch_number].Effect[number]);
    if (FX_type > 0) {
      Output += M13_FX_types[FX_type].Name;
    }
    else {
      Output += "--";
    }
  }
}

FLASHMEM bool MD_M13_class::request_parameter(uint8_t sw, uint16_t number) {
  if (M13_data == NULL) return true;
  //Effect type and state are stored in the M13_FX array
  //Effect can have three states: 0 = no effect, 1 = on, 2 = off
  DEBUGMSG("M13 parameter request number: " + String(number));

  String msg = "";
  if (number < 12) {
    SP[sw].State = 2; // Effect off
    //uint8_t FX_no = M13_parameters[number].CC - 11; // Calculate the FX number from the CC number.
    uint8_t FX_type = FXsearch(M13_data[patch_number].Category[number], M13_data[patch_number].Effect[number]);
    DEBUGMSG("FX-type: " + String(FX_type));
    if (FX_type > 0) {
      msg += M13_FX_types[FX_type].Name;

      // Determine the state of the current switch
      uint8_t M13_state = M13_data[patch_number].State[number / 3];
      uint8_t my_state = (number % 3) + 1;
      DEBUGMSG("M13_state: " + String(M13_state) + ", my_state: " + String(my_state));
      if (M13_state == my_state) SP[sw].State = 1; // Effect on

      // Determine colour by category number
      switch (M13_data[patch_number].Category[number]) {
        case 2: SP[sw].Colour = FX_DELAY_TYPE; break;
        case 3: SP[sw].Colour = FX_MODULATE_TYPE; break;
        case 4: SP[sw].Colour = FX_REVERB_TYPE; break;
        case 5: SP[sw].Colour = FX_DIST_TYPE; break;
        case 9: SP[sw].Colour = FX_MODULATE_TYPE; break;
        default: SP[sw].Colour = FX_FILTER_TYPE; break; // Case 0, 10 and 12
      }
    }
    else { // Effect not found in table
      if ((M13_data[patch_number].Effect[number] == 0) || (!connected)) msg = "--"; // Type zero - not read properly
      else msg += String(M13_data[patch_number].Category[number]) + ": " + String(M13_data[patch_number].Effect[number]); // New type
      SP[sw].Colour = 0;
    }
  }
  else  { // Parameter is FX bypass or looper function
    //msg = M13_parameters[number].Name;
    SP[sw].Colour = my_LED_colour;
    if (par_on[number - 12]) SP[sw].State = 1; // Effect on
    else SP[sw].State = 0; // Effect off
  }
  LCD_set_SP_label(sw, msg);
  return true; // Move to next switch is true
}

// Menu options for FX states
FLASHMEM void MD_M13_class::read_parameter_name(uint16_t number, String &Output) { // Called from menu
  if (number < number_of_parameters())  Output = M13_parameters[number].Name;
  else Output = "?";
}

FLASHMEM uint16_t MD_M13_class::number_of_parameters() {
  return M13_NUMBER_OF_PARAMETERS;
}

FLASHMEM uint8_t MD_M13_class::number_of_values(uint16_t parameter) {
  if ((parameter == M13_SW_EXP1) || (parameter == M13_SW_EXP2)) return 128; // Return 128 for the expression pedals
  if (parameter < number_of_parameters()) return 2; // So far all parameters have two states: on and bypass
  else return 0;
}

FLASHMEM void MD_M13_class::read_parameter_value_name(uint16_t number, uint16_t value, String &Output) {
  if ((number == M13_SW_EXP1) || (number == M13_SW_EXP2)) Output += String(value); // Return the number for the expression pedals
  else if (number < number_of_parameters())  {
    if (value == 1) Output += "ON";
    else Output += "BYPASS";
  }
  else Output += " ? "; // Unknown parameter
}

// ********************************* Section 6: M13 expression pedal control ********************************************

FLASHMEM void MD_M13_class::move_expression_pedal(uint8_t sw, uint8_t value, uint8_t exp_pedal) {
  if (exp_pedal == 0) exp_pedal = current_exp_pedal;
  if (exp_pedal > 0) {
    LCD_show_bar(0, value, 0); // Show it on the main display
    SP[sw].Latch = RANGE;
    SCO_update_parameter_state(sw, 0, 127, 1);
    parameter_press(sw, 0, SP[sw].PP_number);
    update_page = REFRESH_FX_ONLY;
  }
}

FLASHMEM void MD_M13_class::toggle_expression_pedal(uint8_t sw) {
  //uint8_t value;
  if (current_exp_pedal == 0) return;
  current_exp_pedal++;
  if (current_exp_pedal > 2) current_exp_pedal = 1;
  update_page = REFRESH_FX_ONLY;
}

FLASHMEM bool MD_M13_class::request_exp_pedal(uint8_t sw, uint8_t exp_pedal) {
  uint8_t number = 0;
  if (exp_pedal == 0) exp_pedal = current_exp_pedal;
  if (exp_pedal == 1) number = M13_SW_EXP1;
  if (exp_pedal == 2) number = M13_SW_EXP2;
  if (number > 0) {
    SP[sw].PP_number = number;
    return request_parameter(sw, number);
  }
  LCD_clear_SP_label(sw);
  return true;
}

// ********************************* Section 7: M13 looper control ********************************************

FLASHMEM bool MD_M13_class::looper_active() { // M13 has a looper and it is always on
  return true;
}

struct M13_looper_cc_struct { // Combines all the data we need for controlling a parameter in a device
  uint8_t cc; // The cc number
  uint8_t value; // The value.
};

const PROGMEM M13_looper_cc_struct M13_looper_cc[] = { // Table with the cc messages
  {86, 0},   // HIDE
  {86, 127}, // SHOW
  {28, 0},   // STOP
  {28, 127}, // PLAY
  {50, 127}, // REC
  {50, 0},   // OVERDUB
  {85, 0},   // FORWARD
  {85, 127}, // REVERSE
  {36, 0},   // FULL_SPEED
  {36, 127}, // HALF_SPEED
  {82, 127}, // UNDO
  {82, 127}, // REDO
  {80, 127}, // PLAY_ONCE
  {84, 127}, // PRE
  {84, 0},   // POST
};

const uint8_t M13_LOOPER_NUMBER_OF_CCS = sizeof(M13_looper_cc) / sizeof(M13_looper_cc[0]);

FLASHMEM bool MD_M13_class::send_looper_cmd(uint8_t cmd) {
  if (cmd < M13_LOOPER_NUMBER_OF_CCS) MIDI_send_CC(M13_looper_cc[cmd].cc, M13_looper_cc[cmd].value, MIDI_channel, MIDI_out_port);
  return true;
}
