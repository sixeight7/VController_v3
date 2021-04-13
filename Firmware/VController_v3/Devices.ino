// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: Common settings
// Section 2: Common device settings
// Section 3: Device Object Creation and Initialization

// ********************************* Section 1: Common settings ********************************************

// Here are variables from various parts of the code that have been moved here, because they are used early on.
// As the code develops and is improved, these variables should disappear from here.

bool global_tuner_active = false;

// States and variable for updating page
#define OFF 0
#define REFRESH_FX_ONLY 1
#define REFRESH_PAGE 2
#define RELOAD_PAGE 3
uint8_t update_page = OFF;
uint8_t Number_of_pages = 0; // Real value is read in EEPROM_create_command_indexes()
uint8_t Current_page = PAGE_CURRENT_PATCH_BANK;
uint8_t page_bank_number = 0;
uint8_t page_bank_select_number = 0;
uint8_t page_last_selected = 1;
uint8_t Current_device = 255;                     // The device that is currently selected
uint8_t Previous_page = DEFAULT_PAGE;
uint8_t Previous_device = 0;
uint8_t Previous_bank_size = 0; // Used for direct select

uint8_t calibrate_exp_pedal = 0; // The selected expression pedal in the menu

bool update_main_lcd = false; // True if main display needs updating
uint8_t update_lcd = 0; // Set to the number of the LCD that needs updating
#define MESSAGE_TIMER_LENGTH 1500 // time that status messages are shown (in msec)
#define ACTION_TIMER_LENGTH 600 // for messages that are shown on press of buttons
#define LEDBAR_TIMER_LENGTH 500 // time that status messages are shown (in msec)

bool open_menu_for_Katana_patch_save = false;
bool open_menu_for_SY1000_scene_save = false;
bool do_not_forward_after_Helix_PC_message = false;

#define UP true
#define DOWN false

#define NOT_FOUND 255 // Special state for SP[].PP_number in case an assign is not listed

#define DEVICE_OFF 0
#define DEVICE_ON 1
#define DEVICE_DETECT 2

bool menu_active = false;
bool on_looper_page = false;

// ********************************* Section 2: Common device settings ********************************************
// Default values have been moved to the device classes. All values should be set in the VController menu or VC-edit

// ********************************* Section 3: Device Object Creation and Initialization ********************************************

MD_GP10_class My_GP10 = MD_GP10_class(GP10);
MD_GR55_class My_GR55 = MD_GR55_class(GR55);
MD_VG99_class My_VG99 = MD_VG99_class(VG99);
MD_ZG3_class My_ZG3 = MD_ZG3_class(ZG3);
MD_ZMS70_class My_ZMS70 = MD_ZMS70_class(ZMS70);
MD_M13_class My_M13 = MD_M13_class(M13);
MD_HLX_class My_HLX = MD_HLX_class(HLX);
MD_FAS_class My_AXEFX = MD_FAS_class(AXEFX);
MD_KTN_class My_KTN = MD_KTN_class(KTN);
MD_KPA_class My_KPA = MD_KPA_class(KPA);
MD_SVL_class My_SVL = MD_SVL_class(SVL);
MD_SY1000_class My_SY1000 = MD_SY1000_class(SY1000);
MD_GM2_class My_GM2 = MD_GM2_class(GM2);
MD_MG300_class My_MG300 = MD_MG300_class(MG300);

// Here we create an array for the devices, so we can access them by pointer reference
MD_base_class * Device[NUMBER_OF_DEVICES] = {&My_GP10, &My_GR55, &My_VG99, &My_ZG3, &My_ZMS70, &My_M13, &My_HLX, &My_AXEFX, &My_KTN, &My_KPA, &My_SVL, &My_SY1000, &My_GM2, &My_MG300};

void setup_devices() { // Trigger the initialization of  the devices
  for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
    DEBUGMSG("Init device " + String(d));
    Device[d]->init();
  }
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

IntervalTimer Sequencer_timer;

void device_sequencer_start() {
  long timer_interval = 60000000 / (24 * 120);
  Sequencer_timer.begin(device_sequencer_timer_expired, timer_interval);
  Serial.println("device sequencer started at " + String(timer_interval));
}

void device_sequencer_update(uint8_t steps, uint8_t divider) {
  if (steps == 0) return;
  if (divider == 0) return;
  long timer_interval = 60000000 * divider / (steps * Setting.Bpm);
  Sequencer_timer.update(timer_interval);
  Serial.println("Sequence timer update: " + String(timer_interval));
}

void device_sequencer_timer_expired() {
  __disable_irq();
  My_HLX.update_sequencer = true;
  __enable_irq();
  
}
