// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: EEPROM Initialization
// Section 2: Internal EEPROM functions
// Section 3: External EEPROM (24LC512) Functions
// Section 4: Command and index functions
// Section 5: Reading/writing titles to EEPROM
// Section 6: Reading/writing Katana data to EEPROM
// Section 7: HELIX forward messaging storage

#include "globals.h"

// ********************************* Section 1: EEPROM Initialization ********************************************
// The Teensy 3.2 has 2 kB of EEPROM on board - this storage memory is used for VController settings.
// The external 24LC512 chip has 64 kB of EEPROM - this storage memory is used for VController commands, BOSS Katana presets and Line6 Helix forward messaging

#include <EEPROM.h>
//#include <Wire.h>
#include <i2c_t3.h>

#define CURRENT_EEPROM_VERSION 5 // Update this value whenever there is an update of the internal EEPROM structure 
#define CURRENT_EXT_EEPROM_VERSION 5 // Update this value whenever there is an update of the external EEPROM structure - data will be overwritten!!!!
#define CURRENT_KATANA_MEMORY_VERSION 2 // Update this value whenever there is an update of the Katana EEPROM structure - data will be overwritten!!!!
#define CURRENT_EXP_EEPROM_HELIX_FORWARD_MESSAGING_VERSION 2 // Update this value whenever there is an update of the Helix EEPROM structure - data will be overwritten!!!!

// ************************ Internal EEPROM data addresses ***********************
// Total size: 2048 bytes (Teensy 3.2), 4096 bytes (Teensy 3.6)

// Common data (max 16 bytes)
#define EEPROM_VERSION_ADDR 0x00 // Address!!! Version update must be done above!!!
#define EEPROM_CURRENT_PAGE_ADDR 0x02
#define EEPROM_CURRENT_DEVICE_ADDR 0x03

// Settings data (max 112 bytes)
#define EEPROM_GENERAL_SETTINGS_BASE_ADDRESS 16 // Number of settings determined by size of settings array (see globals.h) - currently 34.

// MIDI settings (max 320 bytes)
#define EEPROM_MIDI_SWITCH_SETTINGS_BASE 128 // MIDI settings contain type, port, channel and number for max 32 switches (128 bytes)
#define EEPROM_MIDI_SWITCH_SETTINGS_DATA_SIZE 4
#define EEPROM_MIDI_SWITCH_TYPE_ADDR 0
#define EEPROM_MIDI_SWITCH_PORT_ADDR 1
#define EEPROM_MIDI_SWITCH_CHANNEL_ADDR 2
#define EEPROM_MIDI_SWITCH_CC_ADDR 3

//#define EEPROM_MIDI_FORWARD_SETTINGS_BASE 256 // MIDI port forwarding (192 bytes)

// Device data (max 1920 bytes - 50 devices)
#define EEPROM_DEVICES_BASE_ADDRESS 448
#define EEPROM_DEVICE_DATA_SIZE 32 // Reserve 32 variables for each device

#define EEPROM_DEVICE_PATCH_MSB 0
#define EEPROM_DEVICE_PATCH_LSB 1
#define EEPROM_DEVICE_SETLIST 2
#define EEPROM_DEVICE_SETTINGS 8 // The rest of the device settings - currently 10 - total of 13 bytes. There is space for 32.

// ************************ External EEPROM data addresses ***********************

// 24LC512 has 64 kByte of memory
// These are writable in pages of 128 bytes = 500 pages
// Page 0 - 249:  Commands
// Page 250 - 369 Katana presets
// Page 375 - 390 Helix messages

#define EEPROM_ADDRESS 0x50    // i2c address of 24LC512 eeprom chip
#define EEPROM_DELAY_LENGTH 5  // time between EEPROM writes (usually 5 ms is OK)
unsigned long WriteDelay = 0;

// Addressing of programmed commands for switches
#define EXT_EEP_CMD_BASE_ADDRESS 32    // Commands are stored from address 32 - 31999
#define EXT_EEP_MAX_NUMBER_OF_COMMANDS 2997
#define EXT_EEP_NUMBER_OF_COMMANDS_MSB_ADDR 0
#define EXT_EEP_NUMBER_OF_COMMANDS_LSB_ADDR 1
#define EXT_EEP_KATANA_PRESETS_INITIALIZED_ADDR 2
#define EXT_EEP_HELIX_MSG_SPACE_ADDR 3
#define EXT_EEP_VERSION_ADDR 4
#define EXP_EEP_HELIX_FORWARD_MESSAGING_ADDR 5
#define CMD_IS_FIXED 0X8000 // MSB of index is set to 1 for fixed commands

#define EXT_EEP_KATANA_PRESETS_BASE_ADDRESS 32000 // Katana presets are stored from address 320000 - 47359 (80 presets of 192 bytes)
#define EXT_MAX_NUMBER_OF_KATANA_PRESETS 80

#define EXT_EEP_HELIX_MESSAGE_BASE_ADDRESS 48000 // Katana presets are stored from address 48000 - 50047 (128 messages of 16 bytes)
#define EXT_MAX_NUMBER_OF_HELIX_MESSAGES 128
#define MIDI_HLX_MESSAGES 5 // number of messages per patch that are stored

// RAM indexes
#define MAX_NUMBER_OF_PAGES 256

// We will create a number of indexes to quickly find the commands that are assigned to a switch of the VController
uint16_t First_cmd_index[MAX_NUMBER_OF_PAGES][TOTAL_NUMBER_OF_SWITCHES + 1]; // Gives the index number of every first command for every dwitch on every page
uint16_t Next_cmd_index[EXT_EEP_MAX_NUMBER_OF_COMMANDS]; // Gives the number of the next command that needs to be read from EEPROM
uint16_t Next_internal_cmd_index[NUMBER_OF_INTERNAL_COMMANDS]; // Gives the index of the next command that needs to be read from Fixed_commands[]
uint16_t Title_index[MAX_NUMBER_OF_PAGES][TOTAL_NUMBER_OF_SWITCHES + 1]; // gives the index number of the title command for page 0 (page title) and every switch with a display
uint16_t number_of_cmds; // Contains the total number of commands stored in EEPROM.

#define INTERNAL_CMD 0x8000 // MSB of an index set indicates the command is an internal command

uint8_t HLX_messages[MIDI_HLX_MESSAGES][3]; // Space to store Helix forwarding messages
uint8_t HLX_message_setlist;

uint32_t EEPROM_update_timer = 0;
#define EEPROM_UPDATE_TIMER_LENGTH 2000 // Time between reading the display boards

void setup_eeprom()
{
  if (EEPROM.read(EEPROM_VERSION_ADDR) != CURRENT_EEPROM_VERSION) EEP_initialize_internal_eeprom_data();
  if (read_ext_EEPROM(EXT_EEP_VERSION_ADDR) != CURRENT_EXT_EEPROM_VERSION) EEP_initialize_external_eeprom_data();
  if (read_ext_EEPROM(EXT_EEP_KATANA_PRESETS_INITIALIZED_ADDR) != CURRENT_KATANA_MEMORY_VERSION) EEP_update_katana_preset_memory();
  if (read_ext_EEPROM(EXP_EEP_HELIX_FORWARD_MESSAGING_ADDR) != CURRENT_EXP_EEPROM_HELIX_FORWARD_MESSAGING_VERSION) EEPROM_initialize_HELIX_message_space();

  // Read data from EEPROM memory
  EEP_read_eeprom_common_data();
  Current_page = EEPROM.read(EEPROM_CURRENT_PAGE_ADDR);
  set_current_device(EEPROM.read(EEPROM_CURRENT_DEVICE_ADDR));

  EEPROM_create_indexes();
}

void main_eeprom()
{
  if ((millis() > EEPROM_update_timer) && (EEPROM_update_timer > 0)) EEPROM_update(); 
}

void EEPROM_update_when_quiet() {
  EEPROM_update_timer = millis() + EEPROM_UPDATE_TIMER_LENGTH;
}

void EEPROM_update() {
  EEPROM_update_timer = 0;
  EEPROM.write(EEPROM_CURRENT_PAGE_ADDR, Current_page);
  EEPROM.write(EEPROM_CURRENT_DEVICE_ADDR, Current_device);
}

// ********************************* Section 2: Internal EEPROM functions ********************************************

void EEP_read_eeprom_common_data() {

  // Read the settings for the Setting array
  byte* settingbytes = (byte*)&Setting;
  for (uint8_t s = 0; s < sizeof(Setting); s++) {
    *settingbytes++ = EEPROM.read(EEPROM_GENERAL_SETTINGS_BASE_ADDRESS + s);
  }
  DEBUGMSG("Reading settings (" + String(sizeof(Setting)) + " settings)");
  SC_set_expression_pedals();

  // Read MIDI switch data
  for (uint8_t s = 0; s < TOTAL_NUMBER_OF_SWITCHES; s++) {
    uint16_t address = EEPROM_MIDI_SWITCH_SETTINGS_BASE + (EEPROM_MIDI_SWITCH_SETTINGS_DATA_SIZE * s);
    MIDI_switch[s].type = EEPROM.read(address + EEPROM_MIDI_SWITCH_TYPE_ADDR);
    MIDI_switch[s].port = EEPROM.read(address + EEPROM_MIDI_SWITCH_PORT_ADDR);
    MIDI_switch[s].channel = EEPROM.read(address + EEPROM_MIDI_SWITCH_CHANNEL_ADDR);
    MIDI_switch[s].cc = EEPROM.read(address + EEPROM_MIDI_SWITCH_CC_ADDR);
  }

  // Read device data
  for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
    uint16_t address = EEPROM_DEVICES_BASE_ADDRESS + (EEPROM_DEVICE_DATA_SIZE * d);
    Device[d]->patch_number = (EEPROM.read(address + EEPROM_DEVICE_PATCH_MSB) << 8) + EEPROM.read(address + EEPROM_DEVICE_PATCH_LSB);
    Device[d]->update_bank_number(Device[d]->patch_number);
    Device[d]->current_setlist = EEPROM.read(address + EEPROM_DEVICE_SETLIST);
    for (uint8_t var = 0; var < NUMBER_OF_DEVICE_SETTINGS; var++) {
      Device[d]->set_setting(var, EEPROM.read(address + EEPROM_DEVICE_SETTINGS + var));
    }
  }
}

void EEP_write_eeprom_common_data() {
  byte* settingbytes = (byte*)&Setting;
  for (uint8_t s = 0; s < sizeof(Setting); s++) {
    EEPROM.write(EEPROM_GENERAL_SETTINGS_BASE_ADDRESS + s, *settingbytes++);
  }
  for (uint8_t s = 0; s < TOTAL_NUMBER_OF_SWITCHES; s++) {
    uint16_t address = EEPROM_MIDI_SWITCH_SETTINGS_BASE + (EEPROM_MIDI_SWITCH_SETTINGS_DATA_SIZE * s);
    EEPROM_write(address + EEPROM_MIDI_SWITCH_TYPE_ADDR, MIDI_switch[s].type);
    EEPROM_write(address + EEPROM_MIDI_SWITCH_PORT_ADDR, MIDI_switch[s].port);
    EEPROM_write(address + EEPROM_MIDI_SWITCH_CHANNEL_ADDR, MIDI_switch[s].channel);
    EEPROM_write(address + EEPROM_MIDI_SWITCH_CC_ADDR, MIDI_switch[s].cc);
  }
  for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
    uint16_t address = EEPROM_DEVICES_BASE_ADDRESS + (EEPROM_DEVICE_DATA_SIZE * d);
    EEPROM_write(address + EEPROM_DEVICE_PATCH_MSB, (Device[d]->patch_number >> 8));
    EEPROM_write(address + EEPROM_DEVICE_PATCH_LSB, (Device[d]->patch_number & 0xFF));
    EEPROM_write(address + EEPROM_DEVICE_SETLIST, Device[d]->current_setlist);
    //EEPROM_write(address + EEPROM_DEVICE_BANK_NUMBER, Device[d]->bank_number);
    for (uint8_t var = 0; var < NUMBER_OF_DEVICE_SETTINGS; var++) {
      EEPROM_write(address + EEPROM_DEVICE_SETTINGS + var, Device[d]->get_setting(var));
    }
  }
}

void EEP_initialize_internal_eeprom_data() {
  // Initialize settings for VController and devices from Teensy EEPROM and write default configuration to external EEPROM
  LED_show_initializing_data();
  LCD_show_popup_label("Initializing...", MESSAGE_TIMER_LENGTH);
  EEPROM_write(EEPROM_VERSION_ADDR, CURRENT_EEPROM_VERSION); // Save the current eeprom version
  EEPROM_write(EEPROM_CURRENT_PAGE_ADDR, DEFAULT_PAGE);
  EEPROM_write(EEPROM_CURRENT_DEVICE_ADDR, 0); // Select first device

  byte* settingbytes = (byte*)&Default_settings;
  for (uint8_t s = 0; s < sizeof(Setting); s++) {
    EEPROM.write(EEPROM_GENERAL_SETTINGS_BASE_ADDRESS + s, *settingbytes++);
  }

  uint8_t no_of_midi_switches = TOTAL_NUMBER_OF_SWITCHES;
  if (NUMBER_OF_DEFAULT_MIDI_SWITCHES < no_of_midi_switches) no_of_midi_switches = NUMBER_OF_DEFAULT_MIDI_SWITCHES;
  for (uint8_t s = 0; s < no_of_midi_switches; s++) {
    uint16_t address = EEPROM_MIDI_SWITCH_SETTINGS_BASE + (EEPROM_MIDI_SWITCH_SETTINGS_DATA_SIZE * s);
    EEPROM_write(address + EEPROM_MIDI_SWITCH_TYPE_ADDR, MIDI_switch_default_settings[s].type);
    EEPROM_write(address + EEPROM_MIDI_SWITCH_PORT_ADDR, MIDI_switch_default_settings[s].port);
    EEPROM_write(address + EEPROM_MIDI_SWITCH_CHANNEL_ADDR, MIDI_switch_default_settings[s].channel);
    EEPROM_write(address + EEPROM_MIDI_SWITCH_CC_ADDR, MIDI_switch_default_settings[s].cc);
  }

  for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
    uint16_t address = EEPROM_DEVICES_BASE_ADDRESS + (EEPROM_DEVICE_DATA_SIZE * d);
    EEPROM_write(address + EEPROM_DEVICE_PATCH_MSB, 0);
    EEPROM_write(address + EEPROM_DEVICE_PATCH_LSB, 0);
    EEPROM_write(address + EEPROM_DEVICE_SETLIST, 0);
    Device[d]->init(); // So default values will be restored
    for (uint8_t var = 0; var < NUMBER_OF_DEVICE_SETTINGS; var++) { // Just write the settings from EEPROM
      EEPROM_write(address + EEPROM_DEVICE_SETTINGS + var, Device[d]->get_setting(var));
      DEBUGMSG("Initialized address " + String(address + var + 1) + " to value " + String(Device[d]->get_setting(var)));
    }
  }
}


void EEPROM_write(uint16_t address, uint8_t value) { // Only write data if it has changed
  if (value != EEPROM.read(address)) {
    EEPROM.write(address, value);
    DEBUGMSG("wrote value " + String(value) + " to address " + String(address));
  }
}

void EEPROM_write_patch_number(uint8_t dev, uint16_t number) {
  if (dev < NUMBER_OF_DEVICES) {
    uint16_t address = EEPROM_DEVICES_BASE_ADDRESS + (EEPROM_DEVICE_DATA_SIZE * dev);
    EEPROM_write(address + EEPROM_DEVICE_PATCH_MSB, number >> 8);
    EEPROM_write(address + EEPROM_DEVICE_PATCH_LSB, number & 0xFF);
  }
}


// ********************************* Section 3: External EEPROM (24LC512) Functions ********************************************

// We store the commands in the external EEPROM.
// The page size of the 24LC512 is 32 bytes. We can only read/write multiple bytes from within that page.
// Two commands of 10 bytes are stored on every page.
// An index is created to quickly access the correct commands from EEPROM
// There are also commands fixed in the ROM of the TEENSY
// To access these commands, the index is larger (CMD_IS_FIXED = 0x8000)

// Low level read/write of the 24LC512

void write_ext_EEPROM(unsigned int eeaddress, byte data )
// Write a single byte to 24LC512 chip
{
  if (data != read_ext_EEPROM(eeaddress)) {

    EEPROM_delay();

    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.send((int)(eeaddress >> 8));   // MSB
    Wire.send((int)(eeaddress & 0xFF)); // LSB
    Wire.send(data);
    Wire.endTransmission();
  }
}

byte read_ext_EEPROM(unsigned int eeaddress )
// Reads a single byte from 24LC512 chip
{
  EEPROM_wait_ready();
  byte rdata = 0xFF;

  Wire.beginTransmission(EEPROM_ADDRESS);
  Wire.send((int)(eeaddress >> 8));   // MSB
  Wire.send((int)(eeaddress & 0xFF)); // LSB
  Wire.endTransmission();

  Wire.requestFrom(EEPROM_ADDRESS, 1);

  if (Wire.available()) rdata = Wire.receive();

  return rdata;
}

void write_cmd_EEPROM(uint16_t memloc, const Cmd_struct* cmd ) // Write a command (11 bytes) to 24LC512 chip
{
  if (memloc < EXT_EEP_MAX_NUMBER_OF_COMMANDS) {
    uint8_t cmdsize = sizeof(*cmd);
    uint16_t eeaddress = EXT_EEP_CMD_BASE_ADDRESS + ((memloc / 3) * 32) + ((memloc % 3) * 10);  // We write three commands in one block of 32 bytes, to stay within the page size of the memory chip
    const byte* cmdbytes = (const byte*)cmd;

    // First check if the command we write is different from the existing data
    bool changed = false;
    Cmd_struct existing_cmd;
    const byte* existingbytes = (const byte*)&existing_cmd;

    read_cmd_EEPROM(memloc, &existing_cmd);

    for (uint8_t c = 0; c < cmdsize; c++) {
      if (*cmdbytes++ != *existingbytes++) changed = true;
    }

    if (changed) {
      EEPROM_delay();

      Wire.beginTransmission(EEPROM_ADDRESS);
      Wire.send((int)(eeaddress >> 8));   // MSB
      Wire.send((int)(eeaddress & 0xFF)); // LSB

      cmdbytes = (const byte*)cmd; // Reset the pointer
      for (uint8_t c = 0; c < cmdsize; c++) {
        Wire.send(*cmdbytes++);
      }
      Wire.endTransmission();
    }
  }
}

void read_cmd_EEPROM(uint16_t memloc, Cmd_struct* cmd)
// Read a command (11 bytes) from the 24LC512 chip
{
  if (memloc & INTERNAL_CMD) { // Copy from Fixed_commands[] array
    copy_cmd(&Fixed_commands[memloc & 0x7FFF], cmd);
    //DEBUGMSG("Copy internal cmd no:" + String(memloc &0x7FFF) + " Type:" + String(cmd->Type));
  }
  else { // Read from EEPROM
    uint16_t eeaddress = EXT_EEP_CMD_BASE_ADDRESS + ((memloc / 3) * 32) + ((memloc % 3) * 10);  // We write three commands in one block of 32 bytes, to stay within the page size of the memory chip
    byte* cmdbytes = (byte*)cmd;
    uint8_t cmdsize = sizeof(*cmd);

    EEPROM_wait_ready();

    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.send((int)(eeaddress >> 8));   // MSB
    Wire.send((int)(eeaddress & 0xFF)); // LSB
    Wire.endTransmission();

    Wire.requestFrom (EEPROM_ADDRESS, (int) cmdsize);
    for (uint8_t c = 0; c < cmdsize; c++) {
      uint8_t newbyte = Wire.receive();
      *cmdbytes++ = newbyte;
    }
  }
}

void EEPROM_delay() {
  // Will delay if last message was within EEPROM_DELAY_LENGTH (5 ms)
  while (millis() - WriteDelay <= EEPROM_DELAY_LENGTH) {}
  WriteDelay = millis();
}

void EEPROM_wait_ready() { // Pause until delay has expired
  while (millis() - WriteDelay <= EEPROM_DELAY_LENGTH) {};
}

void copy_cmd(const Cmd_struct* source, Cmd_struct* dest) {
  byte* sourcebytes = (byte*)source;
  byte* destbytes = (byte*)dest;
  uint8_t cmdsize = sizeof(*source);

  for (uint8_t c = 0; c < cmdsize; c++) {
    *destbytes++ = *sourcebytes++;
  }
}

// ********************************* Section 4: Command and index functions ********************************************

void EEP_initialize_external_eeprom_data() {
  write_ext_EEPROM(EXT_EEP_VERSION_ADDR, CURRENT_EXT_EEPROM_VERSION); // Save the current eeprom version
  LED_show_initializing_data();
  LCD_show_popup_label("Initializing...", MESSAGE_TIMER_LENGTH);

  // Write commands to external EEPROM
  for (uint16_t c = 0; c < NUMBER_OF_INIT_COMMANDS; c++) {
    write_cmd_EEPROM(c, &Default_commands[c]);
  }
  EEP_write_number_of_commands(NUMBER_OF_INIT_COMMANDS);
  delay(30); // Delay 30 ms to allow the data om the EEPROM to settle

  EEPROM_create_indexes();
  LED_turn_all_off();
}

void EEP_write_number_of_commands(uint16_t number) {
  write_ext_EEPROM(EXT_EEP_NUMBER_OF_COMMANDS_MSB_ADDR, number >> 8);
  write_ext_EEPROM(EXT_EEP_NUMBER_OF_COMMANDS_LSB_ADDR, number & 0xFF);
}

void EEPROM_create_indexes() {
  memset(First_cmd_index, 0, sizeof(First_cmd_index));
  memset(Next_cmd_index, 0, sizeof(Next_cmd_index));
  memset(Next_internal_cmd_index, 0, sizeof(Next_internal_cmd_index));
  memset(Title_index, 0, sizeof(Title_index));

  Cmd_struct cmd;

  DEBUGMSG("Creating indexes");

  // Fill the indexes with external commands
  DEBUGMSG("Creating index internal commands");
  //LCD_show_popup_label("Index internal");
  number_of_cmds = (read_ext_EEPROM(EXT_EEP_NUMBER_OF_COMMANDS_MSB_ADDR) << 8) + read_ext_EEPROM(EXT_EEP_NUMBER_OF_COMMANDS_LSB_ADDR);
  DEBUGMSG("Number of commands is " + String(number_of_cmds));
  Number_of_pages = 0;
  for (uint16_t c = number_of_cmds; c-- > 0; ) { //Run backwards through the EEPROM command array
    read_cmd_EEPROM(c, &cmd); // read the command from EEPROM
    if (cmd.Page >= Number_of_pages) Number_of_pages = cmd.Page + 1; //update the number of pages
    if (EEPROM_is_label(cmd.Switch)) { // Check if it is a name label
      uint16_t first_title = Title_index[cmd.Page][cmd.Switch & SWITCH_MASK];
      if (first_title != 0) {// Title array is already filled!!!
        Next_cmd_index[c] = first_title; // Move first title to the Next_cmd_index
      }
      Title_index[cmd.Page][cmd.Switch & SWITCH_MASK] = c; // Add to the Title index
    }
    else { // It is a command
      uint16_t first_cmd = First_cmd_index[cmd.Page][cmd.Switch & SWITCH_MASK];
      if (first_cmd != 0) {// First command array is already filled!!!
        Next_cmd_index[c] = first_cmd; // Move first command to the Next_cmd_index
      }
      First_cmd_index[cmd.Page][cmd.Switch & SWITCH_MASK] = c; // Store the first command
    }
  }

  // Fill the indexes with internal commands
  DEBUGMSG("Creating index internal commands");
  DEBUGMSG("Number of internal commands is " + String(NUMBER_OF_INTERNAL_COMMANDS));
  //LCD_show_popup_label("Index internal");
  for (uint16_t c = NUMBER_OF_INTERNAL_COMMANDS; c-- > 0; ) { //Run backwards through the EEPROM command array
    if (EEPROM_is_label(Fixed_commands[c].Switch)) { // Check if it is a name label
      uint16_t first_title = Title_index[Fixed_commands[c].Page][Fixed_commands[c].Switch & SWITCH_MASK];
      if (first_title != 0) {// Title array is already filled!!!
        Next_internal_cmd_index[c] = first_title; // Move first title to the Next_internral_cmd_index
      }
      if (Fixed_commands[c].Page >= FIRST_FIXED_CMD_PAGE) Title_index[Fixed_commands[c].Page][Fixed_commands[c].Switch & SWITCH_MASK] = c | INTERNAL_CMD; // Add to the Title index
    }
    else { // It is a command
      //DEBUGMSG("indexing pg:" + String(Fixed_commands[c].Page) + " sw:" + String(Fixed_commands[c].Switch));
      uint16_t first_cmd = First_cmd_index[Fixed_commands[c].Page][Fixed_commands[c].Switch & SWITCH_MASK];
      if (first_cmd != 0) {// First command array is already filled!!!
        Next_internal_cmd_index[c] = first_cmd; // Move first command to the Next_internal_cmd_index
      }
      if (Fixed_commands[c].Page >= FIRST_FIXED_CMD_PAGE) First_cmd_index[Fixed_commands[c].Page][Fixed_commands[c].Switch & SWITCH_MASK] = c | INTERNAL_CMD; // Store the first command
    }
  }
  DEBUGMSG("Done indexing..");
}

void EEPROM_purge_cmds() { // Will delete any empty commands
  Cmd_struct cmd;
  uint16_t number_of_deleted_cmds = 0;
  DEBUGMSG("Purging cmds. Number of commands is " + String(number_of_cmds));
  for (uint16_t c = 0; c < number_of_cmds; c++ ) { //Run through the EEPROM command array
    read_cmd_EEPROM(c, &cmd); // read the command from EEPROM
    if ((cmd.Page == 0) && (cmd.Switch == 0) && (cmd.Type == 0)) {
      //DEBUGMSG("Command " + String(c) + " will be purged");
      number_of_deleted_cmds++;
    }
    else if (number_of_deleted_cmds > 0) { // Write the cmds back if previous have been deleted
      write_cmd_EEPROM(c - number_of_deleted_cmds, &cmd);
      //DEBUGMSG("Wrote command " + String(c) + " to address " + String(c - number_of_deleted_cmds));
    }
  }
  if (number_of_deleted_cmds > 0) { // Update the number of cmds if neccesary
    // Clear the commands at the end
    const Cmd_struct empty_cmd = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    for (uint16_t c = number_of_cmds - number_of_deleted_cmds; c < number_of_cmds; c++ ) {
      write_cmd_EEPROM(c, &empty_cmd);
    }
    number_of_cmds -= number_of_deleted_cmds;
    EEP_write_number_of_commands(number_of_cmds);
    DEBUGMSG("New number of commands is " + String(number_of_cmds));
    EEPROM_create_indexes();
  }
}

inline bool EEPROM_is_label(uint8_t sw) {
  return ((sw & SWITCH_TYPE_MASK) == LABEL);
}

void EEPROM_clear_all_commands() {
  number_of_cmds = 0;
}

void EEPROM_write_command_from_editor(Cmd_struct *cmd) {
  write_cmd_EEPROM(number_of_cmds, cmd); // Write this command
  number_of_cmds++;
}

void EEPROM_check_data_received(uint8_t check_number_of_pages, uint16_t check_number_of_cmds) {
  if (check_number_of_cmds != number_of_cmds) {
    DEBUGMAIN("MIDI read error: number of commands (" + String(number_of_cmds) + ") is different from the number of received commands (" + String(check_number_of_cmds) + ")");
    LCD_show_popup_label("Receive failed!", MESSAGE_TIMER_LENGTH);
  }
  else {
    LCD_show_popup_label("Data received OK", MESSAGE_TIMER_LENGTH);
  }
  EEP_write_number_of_commands(number_of_cmds);

  EEPROM_create_indexes();
}

uint8_t EEPROM_count_cmds(uint8_t pg, uint8_t sw) { // Counts the number of commands for this switch. Excludes the commands from the defaut bank, unless pg is the default bank
  if (pg >= Number_of_pages) return 0; // Just in case we are creating a new page

  uint16_t i = First_cmd_index[pg][sw & SWITCH_MASK];

  if (i == 0) return 0; // No command for this switch

  uint8_t number = 1;
  while (Next_cmd_index[i] != 0) {
    i = Next_cmd_index[i];
    number++;
  }

  return number;
}

uint16_t EEPROM_new_command_index() { // Create new command
  number_of_cmds++; // we now have a new command
  EEP_write_number_of_commands(number_of_cmds);
  return number_of_cmds - 1; // Return the index of the new command
}

uint16_t EEPROM_first_cmd(uint8_t pg, uint8_t sw) { // Find the first command - called when button is pressed/released/held/etc.
  //if (pg >= Number_of_pages) return 0; // So running this command for fixed page does no harm.
  sw &= SWITCH_MASK;
  if (First_cmd_index[pg][sw] == 0) return First_cmd_index[PAGE_DEFAULT][sw]; // If no command is found for this switch, return the one of the default page
  return First_cmd_index[pg][sw];
}

uint16_t EEPROM_next_cmd(uint16_t cmd) { // Find the next command - called when the next command is executed
  if (cmd & INTERNAL_CMD) return Next_internal_cmd_index[cmd - INTERNAL_CMD];
  else return Next_cmd_index[cmd];
}

bool EEPROM_read_cmd(uint8_t pg, uint8_t sw, uint8_t number, Cmd_struct *cmd) { // Reads the command from EEPROM
  sw &= SWITCH_MASK;
  uint16_t i = First_cmd_index[pg][sw];
  uint16_t prev_i = i;
  if (i == 0) i = First_cmd_index[PAGE_DEFAULT][sw]; // Read from default page if no command is found for this switch

  if (i & INTERNAL_CMD) { // Read internal cmd
    for (uint8_t n = 0; n < number; n++) {
      i = Next_internal_cmd_index[i - INTERNAL_CMD]; // Find the right command
      if (i == 0) i = prev_i; // Oh dear, command is empty - revert to the previous command
      else prev_i = i;
    }
    read_cmd_EEPROM(i, cmd); // Read this command from EEPROM

    return (Next_internal_cmd_index[i - INTERNAL_CMD] != 0); // Return true if next command exists
  }

  else { // Read external cmd
    for (uint8_t n = 0; n < number; n++) {
      i = Next_cmd_index[i]; // Find the right command
      if (i == 0) i = prev_i; // Oh dear, command is empty - revert to the previous command
      else prev_i = i;
    }
    read_cmd_EEPROM(i, cmd); // Read this command from EEPROM

    return (Next_cmd_index[i] != 0); // Return true if next command exists
  }
}

void EEPROM_write_cmd(uint8_t pg, uint8_t sw, uint8_t number, Cmd_struct *cmd) {
  uint16_t cmd_index = 0;
  bool new_command = false;

  // Check if we do not have an internal page number
  if (pg >= FIRST_FIXED_CMD_PAGE) return;

  // Check if we have a new page or a new command
  if (pg >= Number_of_pages) {
    DEBUGMSG("Adding new page");
    Number_of_pages = pg + 1;
    // Add default name for the new page
    EEPROM_write_title(pg, 0, "Page " + String(pg) + "           ");
  }

  // Check if this is a new command
  if (number >= EEPROM_count_cmds(pg, sw & SWITCH_MASK)) { // Create new command index number
    cmd_index = EEPROM_new_command_index();
    DEBUGMSG("Adding new command with index " + String(cmd_index));
    new_command = true;
  }
  else { // Find the existing command index number
    cmd_index = First_cmd_index[pg][sw & SWITCH_MASK];
    for (uint8_t n = 0; n < number; n++) cmd_index = Next_cmd_index[cmd_index];
  }

  write_cmd_EEPROM(cmd_index, cmd); // Write this command

  if (new_command) EEPROM_create_indexes();
}

void EEPROM_clear_page(uint8_t pg) { // Will delete all commands for this page
  Cmd_struct cmd;

  if (pg >= FIRST_FIXED_CMD_PAGE) return;

  Cmd_struct empty_cmd = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  for (uint16_t c = 0; c < number_of_cmds; c++ ) { //Run through the EEPROM command array
    read_cmd_EEPROM(c, &cmd); // read the command from EEPROM
    if (cmd.Page == pg) write_cmd_EEPROM(c, &empty_cmd); // Delete it if it is on the selected page
  }
}

void EEPROM_clear_switch(uint8_t pg, uint8_t sw) { // Will delete all commands for this switch
  Cmd_struct cmd;

  if (pg >= FIRST_FIXED_CMD_PAGE) return;

  Cmd_struct empty_cmd = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  for (uint16_t c = 0; c < number_of_cmds; c++ ) { //Run through the EEPROM command array
    read_cmd_EEPROM(c, &cmd); // read the command from EEPROM
    if ((cmd.Page == pg) && (cmd.Switch == (sw & SWITCH_MASK))) write_cmd_EEPROM(c, &empty_cmd); // Delete it if it is on the selected page
  }
}

// ********************************* Section 5: Reading/writing titles to EEPROM ********************************************

bool EEPROM_check4label(uint8_t pg, uint8_t sw) { // Checks if a label exists for this switch
  return (Title_index[pg][sw & SWITCH_MASK] > 0);
}

void EEPROM_read_title(uint8_t pg, uint8_t sw, String &title) {
  Cmd_struct cmd;
  uint16_t i = Title_index[pg][sw & SWITCH_MASK];
  if (i > 0) {
    read_cmd_EEPROM(i, &cmd); // Read this command

    title = static_cast<char>(cmd.Type);
    title += static_cast<char>(cmd.Device);
    title += static_cast<char>(cmd.Data1);
    title += static_cast<char>(cmd.Data2);
    title += static_cast<char>(cmd.Value1);
    title += static_cast<char>(cmd.Value2);
    title += static_cast<char>(cmd.Value3);
    title += static_cast<char>(cmd.Value4);

    if (i & INTERNAL_CMD) i = Next_internal_cmd_index[i - INTERNAL_CMD];
    else i = Next_cmd_index[i];

    if (i != 0) { // Read second part of title
      read_cmd_EEPROM(i, &cmd); // Read this command

      title += static_cast<char>(cmd.Type);
      title += static_cast<char>(cmd.Device);
      title += static_cast<char>(cmd.Data1);
      title += static_cast<char>(cmd.Data2);
      title += static_cast<char>(cmd.Value1);
      title += static_cast<char>(cmd.Value2);
      title += static_cast<char>(cmd.Value3);
      title += static_cast<char>(cmd.Value4);
    }
  }
  else title = "Page " + String(pg); // Use default page name

  for (uint8_t i = title.length(); i < LCD_DISPLAY_SIZE; i++) { // Fill the remaining chars with spaces
    title += ' ';
  }
}

void EEPROM_write_title(uint8_t pg, uint8_t sw, String title) {

  if (pg >= FIRST_FIXED_CMD_PAGE) return;

  Cmd_struct cmd;
  cmd.Page = pg;
  cmd.Switch = sw | LABEL;
  cmd.Type = static_cast<byte>(title[0]);
  cmd.Device = static_cast<byte>(title[1]);
  cmd.Data1 = static_cast<byte>(title[2]);
  cmd.Data2 = static_cast<byte>(title[3]);
  cmd.Value1 = static_cast<byte>(title[4]);
  cmd.Value2 = static_cast<byte>(title[5]);
  cmd.Value3 = static_cast<byte>(title[6]);
  cmd.Value4 = static_cast<byte>(title[7]);

  uint16_t cmd_index = Title_index[pg][sw];
  if (cmd_index == 0) cmd_index = EEPROM_new_command_index();
  write_cmd_EEPROM(cmd_index, &cmd);

  if (title.length() > 8) { //Add second part if string is longer than 8 characters
    cmd.Type = static_cast<byte>(title[8]);
    cmd.Device = static_cast<byte>(title[9]);
    cmd.Data1 = static_cast<byte>(title[10]);
    cmd.Data2 = static_cast<byte>(title[11]);
    cmd.Value1 = static_cast<byte>(title[12]);
    cmd.Value2 = static_cast<byte>(title[13]);
    cmd.Value3 = static_cast<byte>(title[14]);
    cmd.Value4 = static_cast<byte>(title[15]);

    cmd_index = Next_cmd_index[cmd_index]; // Find next cmd
    if (cmd_index == 0) cmd_index = EEPROM_new_command_index();
    write_cmd_EEPROM(cmd_index, &cmd);
  }
}

void EEPROM_delete_title(uint8_t pg, uint8_t sw) {

  if (pg >= FIRST_FIXED_CMD_PAGE) return;

  Cmd_struct empty_cmd = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  uint16_t cmd_index = Title_index[pg][sw & SWITCH_MASK];
  if (cmd_index == 0) return;
  write_cmd_EEPROM(cmd_index, &empty_cmd);
  cmd_index = Next_cmd_index[cmd_index]; // Find next cmd
  if (cmd_index == 0) return;
  write_cmd_EEPROM(cmd_index, &empty_cmd);
}

// ********************************* Section 6: Reading/writing Katana data to EEPROM ********************************************

void EEP_update_katana_preset_memory() {
  if (read_ext_EEPROM(EXT_EEP_KATANA_PRESETS_INITIALIZED_ADDR) != 1) {
    EEP_initialize_katana_preset_memory();
    return;
  }

  for (uint8_t i = 0; i < EXT_MAX_NUMBER_OF_KATANA_PRESETS; i++) {
    EEPROM_load_KTN_patch(i, My_KTN.KTN_patch_buffer, KTN_PATCH_SIZE);
    My_KTN.update_patch();
    EEPROM_save_KTN_patch(i, My_KTN.KTN_patch_buffer, KTN_PATCH_SIZE);
  }
  write_ext_EEPROM(EXT_EEP_KATANA_PRESETS_INITIALIZED_ADDR, CURRENT_KATANA_MEMORY_VERSION);
}

void EEP_initialize_katana_preset_memory() {
  write_ext_EEPROM(EXT_EEP_KATANA_PRESETS_INITIALIZED_ADDR, CURRENT_KATANA_MEMORY_VERSION);
  LED_show_initializing_data();
  LCD_show_popup_label("Initializing...", MESSAGE_TIMER_LENGTH);

  My_KTN.load_patch_buffer_with_default_patch();

  for (uint8_t i = 0; i < EXT_MAX_NUMBER_OF_KATANA_PRESETS; i++) {
    EEPROM_save_KTN_patch(i, My_KTN.KTN_patch_buffer, KTN_PATCH_SIZE);
  }

  delay(30); // Delay 30 ms to allow the data om the EEPROM to settle
  LED_turn_all_off();
}

void EEPROM_read_KTN_title(uint8_t number, String &title) {
  // Data is stored on pages of 128 bytes. We use 3 pages for 2 patches.
  // Page 1 contains the first 128 bytes of patch 1
  // Page 2 contains the first 128 bytes of patch 2
  // Page 3 contains 64 bytes of patch 1 and 64 bytes of patch 2
  // The patch name is stored in the first 16 bytes of a patch.
  uint16_t base_address = EXT_EEP_KATANA_PRESETS_BASE_ADDRESS + ((number / 2) * 384); // Point to the base address of 2 patches
  uint16_t part1_address = base_address + ((number % 2) * 128); // Point to the first or second page

  DEBUGMSG("Reading patch name for number " + String(number) + " at Addr1:" + String(part1_address));

  Wire.beginTransmission(EEPROM_ADDRESS);
  Wire.send((int)(part1_address >> 8));   // MSB
  Wire.send((int)(part1_address & 0xFF)); // LSB
  Wire.endTransmission();

  Wire.requestFrom (EEPROM_ADDRESS, (int) 16);
  for (uint8_t c = 0; c < 16; c++) {
    uint8_t newbyte = Wire.receive();
    if ((newbyte > 31) && (newbyte < 128)) title += static_cast<char>(newbyte);
    else title += ' ';
  }
  DEBUGMSG("Read title from memory: " + String(title));
}

void EEPROM_load_KTN_patch(uint8_t number, uint8_t *patch_data, uint8_t data_length) {
  // Data is stored on pages of 128 bytes. We use 3 pages for 2 patches.
  // Page 1 contains the first 128 bytes of patch 1
  // Page 2 contains the first 128 bytes of patch 2
  // Page 3 contains 64 bytes of patch 1 and 64 bytes of patch 2
  uint16_t base_address = EXT_EEP_KATANA_PRESETS_BASE_ADDRESS + ((number / 2) * 384); // Point to the base address of 2 patches
  uint16_t part1_address = base_address + ((number % 2) * 128); // Point to the first or second page
  uint16_t part2_address = base_address + 256 + ((number % 2) * 64); // Point to the first or second half of the third page

  DEBUGMSG("Loading patch number " + String(number) + " from Addr1:" + String(part1_address) + "Addr2:" + String(part2_address));

  Wire.beginTransmission(EEPROM_ADDRESS);
  Wire.send((int)(part1_address >> 8));   // MSB
  Wire.send((int)(part1_address & 0xFF)); // LSB
  Wire.endTransmission();

  Wire.requestFrom (EEPROM_ADDRESS, (int) 128);
  for (uint8_t i = 0; i < 128; i++) {
    uint b = Wire.receive();
    if (b & 0x80) b = 0; // Check for uninitialized values
    patch_data[i] = b;
  }

  Wire.beginTransmission(EEPROM_ADDRESS);
  Wire.send((int)(part2_address >> 8));   // MSB
  Wire.send((int)(part2_address & 0xFF)); // LSB
  Wire.endTransmission();

  Wire.requestFrom (EEPROM_ADDRESS, (int) (data_length - 128));
  for (uint8_t i = 128; i < data_length; i++) {
    patch_data[i] = Wire.receive();
  }
}

void EEPROM_save_KTN_patch(uint8_t number, uint8_t *patch_data, uint8_t data_length) {
  // Data is stored on pages of 128 bytes. We use 3 pages for 2 patches.
  // Page 1 contains the first 128 bytes of patch 1
  // Page 2 contains the first 128 bytes of patch 2
  // Page 3 contains 64 bytes of patch 1 and 64 bytes of patch 2

  uint16_t base_address = EXT_EEP_KATANA_PRESETS_BASE_ADDRESS + ((number / 2) * 384); // Point to the base address of 2 patches
  uint16_t part1_address = base_address + ((number % 2) * 128); // Point to the first or second page
  uint16_t part2_address = base_address + 256 + ((number % 2) * 64); // Point to the first or second half of the third page

  DEBUGMSG("Storing patch number " + String(number) + " at Addr1:" + String(part1_address) + "Addr2:" + String(part2_address));

  EEPROM_delay();
  Wire.beginTransmission(EEPROM_ADDRESS);
  Wire.send((int)(part1_address >> 8));   // MSB
  Wire.send((int)(part1_address & 0xFF)); // LSB
  for (uint8_t i = 0; i < 128; i++) {
    Wire.send(patch_data[i]);
  }
  Wire.endTransmission();

  EEPROM_delay();
  Wire.beginTransmission(EEPROM_ADDRESS);
  Wire.send((int)(part2_address >> 8));   // MSB
  Wire.send((int)(part2_address & 0xFF)); // LSB
  for (uint8_t i = 128; i < data_length; i++) {
    Wire.send(patch_data[i]);
  }
  Wire.endTransmission();

  EEPROM_delay();
}

// ********************************* Section 7: HELIX forward messaging storage ********************************************

void EEPROM_store_HELIX_message(uint8_t number, uint8_t setlist) { // Store HLX_messages[5][3] array to location number.
  if (number >= EXT_MAX_NUMBER_OF_HELIX_MESSAGES) return; // Exit if number is too large
  uint16_t eeaddress = EXT_EEP_HELIX_MESSAGE_BASE_ADDRESS + (number * 16);
  const uint8_t* msg_ptr = (const uint8_t*)HLX_messages;
  uint8_t msgsize = sizeof(HLX_messages);

  EEPROM_delay();

  Wire.beginTransmission(EEPROM_ADDRESS);
  Wire.send((int)(eeaddress >> 8));   // MSB
  Wire.send((int)(eeaddress & 0xFF)); // LSB

  Wire.send(setlist); // Send the setlist number first
  for (uint8_t c = 0; c < msgsize; c++) {
    Wire.send(*msg_ptr++);
  }
  Wire.endTransmission();

  DEBUGMSG("EEPROM: Wrote Helix program " + String(number) + ", setlist " + String(setlist) + ", size: " + String(msgsize));
}

void EEPROM_load_HELIX_message(uint8_t number) { // Read HLX_messages[5][3] array from location number.
  if (number >= EXT_MAX_NUMBER_OF_HELIX_MESSAGES) return; // Exit if number is too large
  uint16_t eeaddress = EXT_EEP_HELIX_MESSAGE_BASE_ADDRESS + (number * 16);
  uint8_t* msg_ptr = (uint8_t*)HLX_messages;
  uint8_t msgsize = sizeof(HLX_messages) + 1;

  EEPROM_wait_ready();

  Wire.beginTransmission(EEPROM_ADDRESS);
  Wire.send((int)(eeaddress >> 8));   // MSB
  Wire.send((int)(eeaddress & 0xFF)); // LSB
  Wire.endTransmission();

  Wire.requestFrom (EEPROM_ADDRESS, (int) msgsize);
  HLX_message_setlist = Wire.receive();
  for (uint8_t c = 0; c < msgsize; c++) {
    uint8_t newbyte = Wire.receive();
    *msg_ptr++ = newbyte;
  }

  DEBUGMSG("EEPROM: Read Helix program " + String(number) + " and setlist " + String(HLX_message_setlist));
}

void EEPROM_initialize_HELIX_message_space() {
  LED_show_initializing_data();
  LCD_show_popup_label("Initializing...", MESSAGE_TIMER_LENGTH);
  write_ext_EEPROM(EXP_EEP_HELIX_FORWARD_MESSAGING_ADDR, CURRENT_EXP_EEPROM_HELIX_FORWARD_MESSAGING_VERSION); // Save the current eeprom version
  EEPROM_clear_HLX_message_array();
  for (uint8_t i = 0; i < EXT_MAX_NUMBER_OF_HELIX_MESSAGES; i++) {
    EEPROM_store_HELIX_message(i, 0);
  }
}

void EEPROM_clear_HLX_message_array() {
  memset(HLX_messages, 0, sizeof(HLX_messages));
}
