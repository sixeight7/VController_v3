// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: Common settings
// Section 2: Common device settings
// Section 3: Device Object Creation and Initialization

// ********************************* Section 1: Common settings ********************************************

// Here are variables from various parts of the code that have been moved here, because they are used early on.
// As the code develops and is improved, these variables should disappear from here.

bool global_tuner_active = false;

// States and variable for updaing page
#define OFF 0
#define REFRESH_FX_ONLY 1
#define REFRESH_PAGE 2
#define RELOAD_PAGE 3
uint8_t update_page = OFF;
uint8_t Number_of_pages = 0; // Real value is read in EEPROM_create_indexes()
uint8_t Current_page = PAGE_CURRENT_PATCH_BANK;
uint8_t Current_device = 255;                     // The device that is currently selected
uint8_t Previous_page = DEFAULT_PAGE;
uint8_t Previous_device = 0;
uint8_t Previous_bank_size = 0; // Used for direct select

uint8_t calibrate_exp_pedal = 0; // The selected expression pedal in the menu

bool update_main_lcd = false; // True if main display needs updating
uint8_t update_lcd = 0; // Set to the number of the LCD that needs updating

bool open_menu_for_Katana_edit = false;

#define UP true
#define DOWN false

#define NOT_FOUND 255 // Special state for SP[].PP_number in case an assign is not listed

#define DEVICE_OFF 0
#define DEVICE_ON 1
#define DEVICE_DETECT 2

bool menu_active = false;

// ********************************* Section 2: Common device settings ********************************************
// Default values have been moved to the device classes. All values should be set in the VController menu or VC-edit

// ********************************* Section 3: Device Object Creation and Initialization ********************************************

GP10_class My_GP10 = GP10_class(GP10);
GR55_class My_GR55 = GR55_class(GR55);
VG99_class My_VG99 = VG99_class(VG99);
ZG3_class My_ZG3 = ZG3_class(ZG3);
ZMS70_class My_ZMS70 = ZMS70_class(ZMS70);
M13_class My_M13 = M13_class(M13);
HLX_class My_HLX = HLX_class(HLX);
AXEFX_class My_AXEFX = AXEFX_class(AXEFX);
KTN_class My_KTN = KTN_class(KTN);

// Here we create an array for the devices, so we can access them by pointer reference
//Device_class * Device[NUMBER_OF_DEVICES] = {&My_GP10, &My_GR55, &My_VG99, &My_ZG3, &My_ZMS70, &My_M13};
Device_class * Device[NUMBER_OF_DEVICES] = {&My_GP10, &My_GR55, &My_VG99, &My_ZG3, &My_ZMS70, &My_M13, &My_HLX, &My_AXEFX, &My_KTN};

void setup_devices() { // Trigger the initialization of  the devices
  for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
    Device[d]->init();
  }
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

