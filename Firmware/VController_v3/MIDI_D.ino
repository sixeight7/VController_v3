// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: Device Initialization
// Section 2: Device common MIDI in functions
// Section 3: Device common MIDI out functions
// Section 4: Device program change
// Section 5: Effect parameter and assign control

// ********************************* Section 1: Device Initialization ********************************************
// Types for sysex requests:
#define PATCH_NAME 1
#define PARAMETER_TYPE 2
#define ASSIGN_TYPE 3

// Constructor - called by all derived classes as well
Device_class::Device_class (uint8_t _dev_no) {
  my_device_number = _dev_no;
}

// Initialize device variables
// Called at startup of VController
void Device_class::init() { // Default values for variables
  my_patch_page = PAGE_CURRENT_PATCH_BANK;
  my_parameter_page = PAGE_CURRENT_PATCH_BANK;
  my_assign_page = PAGE_CURRENT_PATCH_BANK;
}

// Functions for EEPROM/menu to get and set values
// Make sure you update NUMBER_OF_DEVICE_SETTINGS on page Classes when you add a new setting

uint8_t Device_class::get_setting(uint8_t variable) {
  switch (variable) {
    case 0: return my_LED_colour;
    case 1: return MIDI_channel;
    case 2: return MIDI_port;
    case 3: return MIDI_device_id;
    case 4: return bank_number;
    case 5: return is_always_on;
    case 6: return my_patch_page;
    case 7: return my_parameter_page;
    case 8: return my_assign_page;
  }
  return 0;
}

void Device_class::set_setting(uint8_t variable, uint8_t value) {
  switch (variable) {
    case 0: my_LED_colour = value; break;
    case 1: MIDI_channel = value; break;
    case 2: MIDI_port = value; break;
    case 3: MIDI_device_id = value; break;
    case 4: bank_number = value; break;
    case 5: is_always_on = value; break;
    case 6: my_patch_page = value; break;
    case 7: my_parameter_page = value; break;
    case 8: my_assign_page = value; break;
  }
}

// ********************************* Section 2: Device common MIDI in functions ********************************************

void Device_class::check_SYSEX_in(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port) {}

void Device_class::check_PC_in(uint8_t program, uint8_t channel, uint8_t port) {
  // Check the source by checking the channel
  if ((port == MIDI_port) && (channel == MIDI_channel)) { // Device outputs a program change
    if (patch_number != program) {
      patch_number = program;
      //update_page |= REFRESH_PAGE;
      //page_check();
      do_after_patch_selection();
    }
  }
}

void Device_class::check_CC_in(uint8_t control, uint8_t value, uint8_t channel, uint8_t port) {  // Check incoming CC messages from GR-55
  if ((channel == MIDI_channel) && (port == MIDI_port)) {
    if (control == 0) {
      CC01 = value;
    }
  }
}

void Device_class::check_still_connected() { // Started from MIDI/MIDI_check_all_devices_still_connected()
  if (connected) {
    //DEBUGMSG(device_name + " not detected times " + String(no_response_counter));
    if (no_response_counter >= max_times_no_response) disconnect();
    no_response_counter++;
  }
}

void Device_class::identity_check(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port) {}

void Device_class::connect(uint8_t device_id, uint8_t port) {
  connected = true;
  LCD_show_status_message(String(device_name) + " connected ");
  MIDI_device_id = device_id;
  MIDI_port = port; // Set the correct MIDI port for this device
  DEBUGMAIN(String(device_name) + " connected on MIDI port " + String(port >> 4) + ":" + String(port & 0x0F));
  //bank_number = (patch_number / bank_size);
  do_after_connect();
  update_page = RELOAD_PAGE;
}

void Device_class::do_after_connect() {}

void Device_class::disconnect() {
  connected = false;
  is_on = false;
  LCD_show_status_message(String(device_name) + " offline   ");
  DEBUGMAIN(String(device_name) + " offline");
  update_page |= RELOAD_PAGE;
  update_main_lcd = true;
}

// ********************************* Section 3: Device common MIDI out functions ********************************************

void Device_class::check_sysex_delay() { // Will delay if last message was within SYSEX_DELAY_LENGTH (10 ms)
  while (millis() - sysexDelay <= SYSEX_DELAY_LENGTH) {}
  sysexDelay = millis();
}

// Calculate the Roland checksum
uint8_t Device_class::calc_Roland_checksum(uint16_t sum) {
  uint8_t checksum = 0x80 - (sum % 0x80);
  if (checksum == 0x80) checksum = 0;
  return checksum;
}


void Device_class::set_bpm() {}
void Device_class::start_tuner() {}
void Device_class::stop_tuner() {}

// ********************************* Section 4: Device program change ********************************************

void Device_class::SendProgramChange(uint16_t new_patch) {

  if (new_patch == patch_number) unmute();
  patch_number = new_patch;
  MIDI_send_PC(new_patch, MIDI_channel, MIDI_port);
  DEBUGMSG("out(" + String(device_name) + ") PC" + String(new_patch)); //Debug
  do_after_patch_selection();
  //update_page |= REFRESH_PAGE;
}

void Device_class::do_after_patch_selection() {
  update_page |= REFRESH_PAGE;
}

/*uint32_t Device_class::_address(uint16_t number) {
  return 0;
}*/

void Device_class::request_patch_name(uint16_t number) {}

void Device_class::request_current_patch_name() {}

uint16_t Device_class::calculate_patch_number(uint8_t bank_position, uint8_t bank_size) {
  uint16_t new_patch;
  if (bank_selection_active()) new_patch = (bank_select_number * bank_size) + bank_position;
  else new_patch = (bank_number * bank_size) + bank_position;
  if (new_patch > patch_max) new_patch = new_patch - patch_max - 1 + patch_min;
  return new_patch;
}

void Device_class::patch_select(uint16_t new_patch) {
  // Check whether the current patch needs to be switched on or whether a new patch is chosen
  if (new_patch == patch_number) select_switch(); // Not a new patch - do US20 emulation
  else {
    SendProgramChange(new_patch); //New patch - send program change
    Current_device = my_device_number;
    PC_ignore_timer = millis();
  }
  bank_number = bank_select_number; // Update the real bank number with the selected number
  device_in_bank_selection = 0; // Switch off bank selection mode
}

uint16_t Device_class::prev_patch_number() {
  if (patch_number > patch_min) return (patch_number - 1);
  else return (patch_max);
}

uint16_t Device_class::next_patch_number() {
  if (patch_number < patch_max) return (patch_number + 1);
  else return (patch_min);
}

void Device_class::bank_updown(bool updown, uint8_t my_bank_size) {

  //uint8_t calculated_bank_number = (patch_number / my_bank_size);

  if (!bank_selection_active()) {
    device_in_bank_selection = my_device_number + 1; // Use of static variable device_in_bank_selection will make sure only one device is in bank select mode.
    bank_select_number = bank_number; //Reset the bank to current patch
    bank_size = my_bank_size;
  }
  // Perform bank up:
  if (updown == UP) {
    if (bank_select_number >= (patch_max / bank_size)) bank_select_number = (patch_min / bank_size); // Check if we've reached the top
    else bank_select_number++; //Otherwise move bank up
  }
  // Perform bank down:
  if (updown == DOWN) {
    if (bank_select_number <= (patch_min / bank_size)) bank_select_number = (patch_max / bank_size); // Check if we've reached the bottom
    else bank_select_number--; //Otherwise move bank down
  }

  if (bank_select_number == bank_number) device_in_bank_selection = 0; //Check whether were back to the original bank
}

bool Device_class::bank_selection_active() {
  return (device_in_bank_selection == my_device_number + 1);
}

void Device_class::display_patch_number_string() {}

void Device_class::number_format(uint16_t number, String &Output) {
  Output = Output + String((number + 1) / 10) + String((number + 1) % 10);
}

void Device_class::direct_select_format(uint16_t number, String &Output) {
  if (direct_select_state == 0) {
    if ( (number * 10) + (bank_select_number * 100) <= patch_max ) Output = Output + String(number) + "_";
    else Output = Output + "--";
  }
  else Output = Output + String(direct_select_first_digit) + String(number);
}

void Device_class::direct_select_press(uint8_t number) {
  if (direct_select_state == 0) {
    // First digit pressed
    direct_select_first_digit = number + (bank_select_number * 10);
    direct_select_state = 1;
    device_in_bank_selection = 0;
  }
  else {
    // Second digit pressed
    direct_select_state = 0;
    device_in_bank_selection = 0;
    uint16_t new_patch = (direct_select_first_digit * 10) + number;
    if (new_patch <= patch_min) new_patch = patch_min + 1;
    if (new_patch > patch_max) new_patch = patch_max;
    bank_number = ((new_patch - 1) / Previous_bank_size); // Set bank number to the new patch
    bank_select_number = bank_number;
    SendProgramChange(new_patch - 1);
    SCO_select_page(Previous_page);
  }
}

// ** US-20 simulation

void Device_class::unmute() {}

void Device_class::mute() {}

void Device_class::select_switch() {
  //Current_device = my_device_number;
  if (Current_device == my_device_number) {
    is_always_on_toggle();
  }
  else {
    unmute();
    Current_device = my_device_number;
    update_main_lcd = true;
    update_LEDS = true;
  }
}

void Device_class::is_always_on_toggle() {
  if (Setting.US20_emulation_active) {
    is_always_on = !is_always_on; // Toggle is_always_on
    if (is_always_on) {
      unmute();
      LCD_show_status_message(String(device_name) + " always ON");
    }
    else {
      //mute();
      LCD_show_status_message(String(device_name) + " can be muted");
    }
  }
}

// ********************************* Section 5: Effect parameter and assign control ********************************************
void Device_class::read_parameter_name(uint16_t number, String &Output) { // Called from menu
  Output = "no parameters";
}

void Device_class::read_parameter_state(uint16_t number, uint8_t value, String &Output) {
  Output = "-";
}

void Device_class::parameter_press(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {}

void Device_class::parameter_release(uint8_t Sw, Cmd_struct *cmd, uint16_t number) {}

bool Device_class::request_parameter(uint16_t number) {
  // Not implemented in base class - move on to next switch
  return true;
}

uint16_t Device_class::number_of_parameters() {
  return 0;
}

uint8_t Device_class::number_of_values(uint16_t parameter) {
  return 0;
}

void Device_class::par_bank_updown(bool updown, uint8_t my_bank_size) {

  // Perform bank up:
  if (updown == UP) {
    DEBUGMSG("!!!NUMBER_OF_PARAMETERS: " + String(number_of_parameters()));
    if (parameter_bank_number >= (number_of_parameters() / my_bank_size)) parameter_bank_number = 0; // Check if we've reached the top
    else parameter_bank_number++; //Otherwise move bank up
  }
  // Perform bank down:
  if (updown == DOWN) {
    if (parameter_bank_number <= 0) parameter_bank_number = (number_of_parameters() / my_bank_size); // Check if we've reached the bottom
    else parameter_bank_number--; //Otherwise move bank down
  }

  LCD_show_status_message("Par bank " + String(parameter_bank_number + 1) + "/" + String((number_of_parameters() / my_bank_size) + 1));
  DEBUGMSG("***NEW BANK NUMBER: " + String(parameter_bank_number));

  //if (bank_number == calculated_bank_number) device_in_bank_selection = 0; //Check whether were back to the original bank

  update_page |= REFRESH_PAGE; //Re-read the patchnames for this bank
}

void Device_class::read_assign_name(uint8_t number, String &Output) {
  Output = "no assigns";
}

void Device_class::read_assign_trigger(uint8_t number, String &Output) {
  Output = "-";
}

uint8_t Device_class::get_number_of_assigns() {
  return 0;
}

uint8_t Device_class::trigger_follow_assign(uint8_t number) {
  return 0;
}

void Device_class::assign_press(uint8_t Sw, uint8_t value) {}

void Device_class::assign_release(uint8_t Sw) {}

void Device_class::assign_load(uint8_t sw, uint8_t assign_number, uint8_t cc_number) {}

void Device_class::request_current_assign() {
  // Not implemented in base class - move on to next switch
  PAGE_request_next_switch();
}

void Device_class::FX_press(uint8_t Sw, Cmd_struct *cmd, uint8_t number) {}  // Method for ZOOM pedals
void Device_class::FX_set_type_and_state(uint8_t Sw) {}  // Method for ZOOM pedals



