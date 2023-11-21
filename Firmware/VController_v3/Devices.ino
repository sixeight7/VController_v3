// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: Common settings
// Section 2: Common device settings
// Section 3: Device Object Creation and Initialization

#if defined(__IMXRT1062__) // Teensy 4.0 and 4.1
#include <Entropy.h>
#endif

// ********************************* Section 1: Common settings ********************************************

// Here are variables from various parts of the code that have been moved here, because they are used early on.

#if defined (CONFIG_VCTOUCH)
#define DEFAULT_PAGE DEFAULT_VCTOUCH_PAGE
#define PAGE_FOR_SONG_MODE PAGE_VCTOUCH_SONG_MODE
#define PAGE_FOR_PAGE_MODE PAGE_VCTOUCH_PAGE_MODE
#define PAGE_FOR_DEVICE_MODE PAGE_VCTOUCH_DEVICE_MODE
#define PAGE_CURRENT_PATCH_BANK PAGE_VCTOUCH_CURRENT_PATCH_BANK
#define PAGE_MENU PAGE_VCTOUCH_MENU
#define PAGE_CURRENT_DIRECT_SELECT PAGE_VCTOUCH_CURRENT_DIRECT_SELECT
#define PAGE_DEFAULT PAGE_VCTOUCH_DEFAULT
#define PAGE_SELECT PAGE_VCTOUCH_DEVICE_MODE
#define PAGE_SETLIST_SELECT PAGE_VCTOUCH_SETLIST_SELECT
#define PAGE_CURRENT_PARAMETER PAGE_VCTOUCH_EDIT_PARAMETER
#define FIRST_FIXED_CMD_PAGE FIRST_FIXED_CMD_PAGE_VCTOUCH
#define FIRST_SELECTABLE_FIXED_CMD_PAGE FIRST_SELECTABLE_FIXED_CMD_PAGE_VCTOUCH
#define LAST_FIXED_CMD_PAGE LAST_FIXED_CMD_PAGE_VCTOUCH

#elif defined (CONFIG_VCMINI)
#define DEFAULT_PAGE DEFAULT_VCMINI_PAGE
#define PAGE_FOR_SONG_MODE PAGE_VCMINI_SONG_MODE
#define PAGE_FOR_PAGE_MODE PAGE_VCMINI_PAGE_MODE
#define PAGE_FOR_DEVICE_MODE PAGE_VCMINI_DEVICE_MODE
#define PAGE_CURRENT_PATCH_BANK PAGE_VCMINI_CURRENT_PATCH_BANK
#define PAGE_MENU PAGE_VCMINI_MENU
#define PAGE_CURRENT_DIRECT_SELECT PAGE_VCMINI_CURRENT_DIRECT_SELECT
#define PAGE_DEFAULT PAGE_VCMINI_DEFAULT
#define PAGE_SELECT PAGE_VCMINI_DEVICE_MODE
#define PAGE_CURRENT_PARAMETER PAGE_VCMINI_CURRENT_PARAMETER
#define PAGE_SETLIST_SELECT PAGE_VCMINI_SETLIST_SELECT
#define FIRST_FIXED_CMD_PAGE FIRST_FIXED_CMD_PAGE_VCMINI
#define FIRST_SELECTABLE_FIXED_CMD_PAGE FIRST_SELECTABLE_FIXED_CMD_PAGE_VCMINI
#define LAST_FIXED_CMD_PAGE LAST_FIXED_CMD_PAGE_VCMINI

#elif defined (CONFIG_CUSTOM)
#define DEFAULT_PAGE DEFAULT_CUSTOM_PAGE
#define PAGE_FOR_SONG_MODE PAGE_CUSTOM_SONG_MODE
#define PAGE_FOR_PAGE_MODE PAGE_CUSTOM_PAGE_MODE
#define PAGE_FOR_DEVICE_MODE PAGE_CUSTOM_DEVICE_MODE
#define PAGE_CURRENT_PATCH_BANK PAGE_CUSTOM_CURRENT_PATCH_BANK
#define PAGE_MENU PAGE_CUSTOM_MENU
#define PAGE_CURRENT_DIRECT_SELECT PAGE_CUSTOM_CURRENT_DIRECT_SELECT
#define PAGE_DEFAULT PAGE_CUSTOM_DEFAULT
#define PAGE_SELECT PAGE_CUSTOM_DEVICE_MODE
#define PAGE_SETLIST_SELECT PAGE_CUSTOM_SETLIST_SELECT
#define PAGE_CURRENT_PARAMETER PAGE_CUSTOM_CURRENT_PARAMETER
#define FIRST_FIXED_CMD_PAGE FIRST_FIXED_CMD_PAGE_CUSTOM
#define FIRST_SELECTABLE_FIXED_CMD_PAGE FIRST_SELECTABLE_FIXED_CMD_PAGE_CUSTOM
#define LAST_FIXED_CMD_PAGE LAST_FIXED_CMD_PAGE_CUSTOM

#else
#define DEFAULT_PAGE DEFAULT_VC_PAGE
#define PAGE_CURRENT_PATCH_BANK PAGE_VC_CURRENT_PATCH_BANK
#define PAGE_FOR_SONG_MODE PAGE_VC_SONG_MODE
#define PAGE_FOR_PAGE_MODE PAGE_VC_PAGE_MODE
#define PAGE_FOR_DEVICE_MODE PAGE_VC_DEVICE_MODE
#define PAGE_MENU PAGE_VC_MENU
#define PAGE_CURRENT_DIRECT_SELECT PAGE_VC_CURRENT_DIRECT_SELECT
#define PAGE_DEFAULT PAGE_VC_DEFAULT
#define PAGE_SELECT PAGE_VC_DEVICE_MODE
#define PAGE_SETLIST_SELECT PAGE_VC_SETLIST_SELECT
#define PAGE_CURRENT_PARAMETER PAGE_VC_CURRENT_PARAMETER
#define FIRST_FIXED_CMD_PAGE FIRST_FIXED_CMD_PAGE_VC
#define FIRST_SELECTABLE_FIXED_CMD_PAGE FIRST_SELECTABLE_FIXED_CMD_PAGE_VC
#define LAST_FIXED_CMD_PAGE LAST_FIXED_CMD_PAGE_VC
#endif

bool global_tuner_active = false;

#define DEFAULT_MODE DEVICE_MODE
uint8_t Current_mode = DEFAULT_MODE;

// States and variable for updating page
#define OFF 0
#define REFRESH_FX_ONLY 1
#define REFRESH_PATCH_BANK_ONLY 2
#define REFRESH_PAGE 3
#define RELOAD_PAGE 4
uint8_t update_page = OFF;
uint8_t Number_of_pages = 0; // Real value is read in EEPROM_create_command_indexes()
uint8_t Current_page = PAGE_CURRENT_PATCH_BANK;
uint8_t Current_page_setlist_item = 0;
#define ON_PAGE_SELECT_SWITCH 0
uint8_t page_bank_number = 0;
uint8_t page_bank_select_number = 0;
uint8_t page_last_selected = 1;
uint8_t Current_device = 255;
uint8_t Previous_page = DEFAULT_PAGE;
uint8_t Previous_device = 0;
uint8_t Previous_bank_size = 0; // Used for direct select
uint8_t Current_setlist = 0;
#define MAX_NUMBER_OF_SETLISTS 99
uint16_t Selected_setlist_item = 0;
#define GLOBAL_TEMPO 39
uint8_t Current_setlist_target = 0; // DMAMEM fixes weird error in the menu - looks like something is reading outside the edges.
uint16_t Selected_setlist_tempo = GLOBAL_TEMPO;
uint16_t Current_setlist_position = 0;
uint16_t Number_of_setlist_items = 0;
#define MAX_NUMBER_OF_SETLIST_ITEMS 50
#define MAX_NUMBER_OF_SETLIST_TARGETS NUMBER_OF_DEVICES + 2
uint8_t setlist_bank_number, setlist_bank_select_number;
DMAMEM uint8_t Current_setlist_buffer[VC_PATCH_SIZE];
uint8_t Current_song = 0;
uint8_t Current_song_setlist_item = 0; // The item in the setlist
#define MAX_NUMBER_OF_SONGS 99
uint8_t Current_part = 0;
#define NUMBER_OF_PARTS 8
uint8_t song_bank_number, song_bank_select_number;
DMAMEM uint8_t Current_song_buffer[VC_PATCH_SIZE];
#define NUMBER_OF_SONG_TARGETS 5
uint16_t Current_song_item[NUMBER_OF_SONG_TARGETS] = { 0 };
uint8_t Current_song_midi_port[NUMBER_OF_SONG_TARGETS] = { 0 };
uint8_t Current_song_midi_channel[NUMBER_OF_SONG_TARGETS] = { 1 };

#define SETLIST_ID 255
#define SONG_ID 254

uint8_t Current_MIDI_switch = 1; // The selected MIDI switch in the menu
uint8_t calibrate_exp_pedal = 0; // The selected expression pedal in the menu

bool update_main_lcd = false; // True if main display needs updating
uint8_t update_lcd = 0; // Set to the number of the LCD that needs updating
#define MESSAGE_TIMER_LENGTH 1500 // time that status messages are shown (in msec)
#define ACTION_TIMER_LENGTH 600 // for messages that are shown on press of buttons
#define LEDBAR_TIMER_LENGTH 500 // time that status messages are shown (in msec)

uint8_t USER_selected_device = USER1;
uint8_t USER_selected_parameter = 0;
uint8_t selected_parameter_colour = 0;
uint8_t USER_selected_scene_number = 1;
DMAMEM User_device_struct USER_current_device_data;
uint16_t USER_current_device_data_patch_min;
uint16_t USER_current_device_data_patch_max;
#define USER_DATA_SIZE sizeof(USER_current_device_data)

String Text_entry; // The string we use for entering a text in the menu
uint8_t Text_entry_length = 16;
bool on_screen_keyboard_active = false;
uint8_t open_specific_menu = 0;
bool do_not_forward_after_Helix_PC_message = false;

uint32_t last_looper_cmd_sent_timer = 0;
#define LAST_LOOPER_CMD_SENT_TIME 300

#define UP true
#define DOWN false

#define NOT_FOUND 255 // Special state for SP[].PP_number in case an assign is not listed
#define NO_RESULT 65535
#define NEW_PATCH 65534

#define DEVICE_OFF 0
#define DEVICE_ON 1
#define DEVICE_DETECT 2

// For H_Switch.ino
#define SKIP_RELEASE 1
#define SKIP_LONG_PRESS 2
#define SKIP_HOLD 4

bool on_looper_page = false;
uint8_t Expr_ped_value = 0;

// ********************************* Section 2: Common device settings ********************************************
// Default values have been moved to the device classes. All values should be set in the VController menu or VC-edit

// ********************************* Section 3: Device Object Creation and Initialization ********************************************

MD_GP10_class My_GP10 = MD_GP10_class(GP10);
MD_GR55_class My_GR55 = MD_GR55_class(GR55);
MD_VG99_class My_VG99 = MD_VG99_class(VG99);
MD_ZG3_class My_ZG3 = MD_ZG3_class(ZG3);
MD_ZMS_class My_ZMS = MD_ZMS_class(ZMS);
MD_M13_class My_M13 = MD_M13_class(M13);
MD_HLX_class My_HLX = MD_HLX_class(HLX);
MD_FAS_class My_AXEFX = MD_FAS_class(AXEFX);
MD_KTN_class My_KTN = MD_KTN_class(KTN);
MD_KPA_class My_KPA = MD_KPA_class(KPA);
MD_SVL_class My_SVL = MD_SVL_class(SVL);
MD_SY1000_class My_SY1000 = MD_SY1000_class(SY1000);
MD_GM2_class My_GM2 = MD_GM2_class(GM2);
MD_MG300_class My_MG300 = MD_MG300_class(MG300);
MD_USER_class My_USER1 = MD_USER_class(USER1);
MD_USER_class My_USER2 = MD_USER_class(USER2);
MD_USER_class My_USER3 = MD_USER_class(USER3);
MD_USER_class My_USER4 = MD_USER_class(USER4);
MD_USER_class My_USER5 = MD_USER_class(USER5);
MD_USER_class My_USER6 = MD_USER_class(USER6);
MD_USER_class My_USER7 = MD_USER_class(USER7);
MD_USER_class My_USER8 = MD_USER_class(USER8);
MD_USER_class My_USER9 = MD_USER_class(USER9);
MD_USER_class My_USER10 = MD_USER_class(USER10);

// Here we create an array for the devices, so we can access them by pointer reference
MD_base_class * Device[NUMBER_OF_DEVICES] = {&My_GP10, &My_GR55, &My_VG99, &My_ZG3, &My_ZMS, &My_M13, &My_HLX, &My_AXEFX, &My_KTN, &My_KPA, &My_SVL, &My_SY1000, &My_GM2, &My_MG300, &My_USER1, &My_USER2, &My_USER3, &My_USER4, &My_USER5, &My_USER6, &My_USER7, &My_USER8, &My_USER9, &My_USER10};
MD_USER_class * USER_device[NUMBER_OF_USER_DEVICES] = {&My_USER1, &My_USER2, &My_USER3, &My_USER4, &My_USER5, &My_USER6, &My_USER7, &My_USER8, &My_USER9, &My_USER10};


void setup_devices() { // Trigger the initialization of  the devices
  DEBUGMAIN("Initializing devices");
  for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
    DEBUGMSG("Init device " + String(d));
    Device[d]->init();
  }
  setup_random_number_generator();
  device_sequencer_start();
}

void main_devices() {
  for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
    Device[d]->update();
  }
}

void check_all_devices_for_manual_connection() { // Trigger the manual connection of all devices
  for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
    Device[d]->check_manual_connection();
  }
  update_page = RELOAD_PAGE;
}

void set_current_device(uint8_t dev) {
  if (dev < NUMBER_OF_DEVICES) {
    Current_device = dev;
    Main_backlight_show_colour(Device[Current_device]->my_LED_colour);
  }
}

void reinitialize_user_devices_from_memory() {
  for (uint8_t ud = 0; ud < NUMBER_OF_USER_DEVICES; ud++) {
    USER_device[ud]->init_from_device_data();
  }
}

/*uint8_t get_device_number(uint8_t programmed_device_number) {
  if (programmed_device_number < NUMBER_OF_PROGRAMMED_DEVICES) return programmed_device_number;
  if (programmed_device_number >= CURRENT) return programmed_device_number;
  if ((programmed_device_number >= USER1) && (programmed_device_number < USER10)) return programmed_device_number - USER1 + NUMBER_OF_PROGRAMMED_DEVICES;
  return 0;
}

uint8_t get_programmed_device_number(uint8_t device_number) {
  if (evice_number < NUMBER_OF_PROGRAMMED_DEVICES) return device_number;
  if (device_number >= CURRENT) return programmed_device_number;
  if (device_number >= NUMBER_OF_PROGRAMMED_DEVICES)  return device_number - NUMBER_OF_PROGRAMMED_DEVICES + USER1;
  return 0;
}*/

IntervalTimer Sequencer_timer;

void device_sequencer_start() {
  long timer_interval = 60000000 / (24 * 120);
  Sequencer_timer.begin(device_sequencer_timer_expired, timer_interval);
  DEBUGMAIN("device sequencer started at " + String(timer_interval));
}

void device_sequencer_update(uint8_t steps, uint8_t divider) {
  if (steps == 0) return;
  if (divider == 0) return;
  long timer_interval = 60000000 * divider / (steps * Setting.Bpm);
  Sequencer_timer.update(timer_interval);
  DEBUGMSG("Sequence timer update: " + String(timer_interval));
}

void device_sequencer_timer_expired() {
  __disable_irq();
  My_HLX.update_sequencer = true;
  __enable_irq();
}

// Random number generator

// T3.6/T3.5 Random Number Generator
//==================================
//Thanks and acknowledgement to "manitou" for original test code
//Modifications by "TelephoneBill" dated 23 FEB 2017
#define RNG_CR_GO_MASK          0x1u
#define RNG_CR_HA_MASK          0x2u
#define RNG_CR_INTM_MASK        0x4u
#define RNG_CR_CLRI_MASK        0x8u
#define RNG_CR_SLP_MASK         0x10u
#define RNG_SR_OREG_LVL_MASK    0xFF00u
#define RNG_SR_OREG_LVL_SHIFT   8
#define RNG_SR_OREG_LVL(x)      (((uint32_t)(((uint32_t)(x))<<RNG_SR_OREG_LVL_SHIFT))&RNG_SR_OREG_LVL_MASK)
#define SIM_SCGC6_RNGA          ((uint32_t)0x00000200)

void setup_random_number_generator() {

#if defined(__IMXRT1062__) // Teensy 4.0 and 4.1
  Entropy.Initialize();
#elif defined(__MK64FX512__) || defined(__MK66FX1M0__) // Teensy 3.5 or 3.6
  SIM_SCGC6 |= SIM_SCGC6_RNGA;                  //enable RNG
  RNG_CR &= ~RNG_CR_SLP_MASK;
  RNG_CR |= RNG_CR_HA_MASK;                     //high assurance, not needed
#else
  // Teensy 3.2
  // A10 is an unconnected pad on Teensy 3.2, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(analogRead(A10));
#endif
}

uint8_t generate_random_number() {
#if defined(__IMXRT1062__) // Teensy 4.0 and 4.1
  return Entropy.random(127);
#elif defined(__MK64FX512__) || defined(__MK66FX1M0__) // Teensy 3.5 or 3.6
  RNG_CR |= RNG_CR_GO_MASK;
  while ((RNG_SR & RNG_SR_OREG_LVL(0xF)) == 0); //wait for RN to be generated
  return RNG_OR & 0x7F;                                //return RN
#else
  return random(127);
#endif
}
