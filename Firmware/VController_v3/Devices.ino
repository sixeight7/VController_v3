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
#define RELOAD_PAGE 1
#define REFRESH_PAGE 2
#define REFRESH_FX_ONLY 3
uint8_t update_page = OFF;
uint8_t Number_of_pages = 0; // Real value is read in EEPROM_create_indexes()
uint8_t Current_page = PAGE_CURRENT_PATCH_BANK;
uint8_t Current_device = 0;                     // The device that is currently selected
uint8_t Previous_page = DEFAULT_PAGE;
uint8_t Previous_device = 0;
uint8_t Previous_bank_size = 0; // Used for direct select

uint8_t Current_switch = 255; // The parameter that is being read (pointer in the SP array)

bool update_main_lcd = false; // True if main display needs updating
uint8_t update_lcd = 0; // Set to the number of the LCD that needs updating

#define UP true
#define DOWN false

#define NOT_FOUND 255 // Special state for SP[].PP_number in case an assign is not listed

bool menu_active = false;

// ********************************* Section 2: Common device settings ********************************************

// Boss GP-10 settings:
#define GP10_MIDI_CHANNEL 1 // Was unable to change patch when GP-10 channel was not 1. Seems to be a bug in the GP-10
#define GP10_PATCH_MIN 0
#define GP10_PATCH_MAX 98

// Roland GR-55 settings:
#define GR55_MIDI_CHANNEL 8
#define GR55_PATCH_MIN 0
#define GR55_PATCH_MAX 656 // Assuming we are in guitar mode!!!

// Roland VG-99 settings:
#define VG99_MIDI_CHANNEL 9
#define VG99_PATCH_MIN 0
#define VG99_PATCH_MAX 399

// Zoom G3 settings:
#define ZG3_MIDI_CHANNEL 1
#define ZG3_PATCH_MIN 0
#define ZG3_PATCH_MAX 99

// Zoom MS70CDR settings:
#define ZMS70_MIDI_CHANNEL 1
#define ZMS70_PATCH_MIN 0
#define ZMS70_PATCH_MAX 49

// ********************************* Section 3: Device Object Creation and Initialization ********************************************

GP10_class My_GP10 = GP10_class(GP10);
GR55_class My_GR55 = GR55_class(GR55);
VG99_class My_VG99 = VG99_class(VG99);
ZG3_class My_ZG3 = ZG3_class(ZG3);
ZMS70_class My_ZMS70 = ZMS70_class(ZMS70);

// Here we create an array for the devices, so we can access them by pointer reference
Device_class * Device[NUMBER_OF_DEVICES] = {&My_GP10, &My_GR55, &My_VG99, &My_ZG3, &My_ZMS70};

void setup_devices() { // Trigger the initialization of  the devices
  for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
    Device[d]->init();
  }
}


