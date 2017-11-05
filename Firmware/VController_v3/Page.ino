// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: Page Setup
// Section 2: Page Loading Into SP array
// Section 3: Page Reading MIDI Data From Devices

// ********************************* Section 1: Page Setup ********************************************

// A page contains a set of functions for internal and external switches and expression pedals.
// Loading of a page from configuration page to active memory (SP array) and reading of the variables

#define SYSEX_WATCHDOG_LENGTH 100 // watchdog length for sysex messages (in msec).
unsigned long SysexWatchdog = 0; // This watchdog will check if a device responds. If it expires it will request the same parameter again.
boolean Sysex_watchdog_running = false;
#define SYSEX_NUMBER_OF_READ_ATTEMPS 3 // Number of times the watchdog will be restarted before moving to the next parameter
uint8_t read_attempt = 1;
uint8_t reading_type = OFF; // Will ensure that all the patch names are read!
bool default_command; //Will indicate if the command that has been read is from the default page or not...
String page_label;
bool request_next_switch = false;

void setup_page()
{
  DEBUGMSG("Number of commands stored in external EEPROM: " + String(number_of_cmds));
  if ((Current_page >= Number_of_pages) && (Current_page < FIRST_FIXED_CMD_PAGE)) Current_page = DEFAULT_PAGE;
  PAGE_load_current();
  page_label.reserve(17);
}

void main_page() {
  //if (global_tuner_active) DEBUGMSG("Tuner active");
  if ((update_page != OFF) && (reading_type == OFF)) {
    switch (update_page) {
      case RELOAD_PAGE:
        DEBUGMAIN("** Reloading page " + String(Current_page));
        reading_type = RELOAD_PAGE;
        update_page = OFF;
        PAGE_load_current(); // Load and read the page
        break;
      case REFRESH_PAGE:
        DEBUGMAIN("** Refreshing page " + String(Current_page));
        reading_type = REFRESH_PAGE;
        update_page = OFF;
        PAGE_request_first_switch();
        break;
      case REFRESH_FX_ONLY:
        DEBUGMAIN("** Refreshing fx only on page " + String(Current_page));
        reading_type = REFRESH_FX_ONLY;
        update_page = OFF;
        PAGE_request_first_switch();
        break;
    }
  }

  if (request_next_switch) {
    request_next_switch = false;
    PAGE_request_next_switch();
  }

  check_keyboard_press_expired(); // Function is on the Menu page...
}

// ********************************* Section 2: Page Loading Into SP array ********************************************

// Load a page from memory into the SP array
void PAGE_load_current() {
  //update_page = OFF; //Switch LCDs are updated here as well
  for (uint8_t s = 0; s < (NUMBER_OF_SWITCHES + NUMBER_OF_EXTERNAL_SWITCHES + 1); s++) { // Load regular switches
    PAGE_load_switch(s);
  }
  PAGE_request_first_switch(); //Now start reading in the parameters from the devices
}

void PAGE_load_switch(uint8_t sw) {
  // Read the command for this switch
  Cmd_struct cmd;
  EEPROM_read_cmd(Current_page, sw, 0, &cmd); // Read from EEPROM
  //DEBUGMSG("Read Pg:" + String(cmd.Page) + " Sw:" + String(cmd.Switch) + " Type:" + String(cmd.Type));

  uint8_t Dev = cmd.Device;
  if (Dev == CURRENT)  Dev = Current_device;
  uint8_t Type = cmd.Type;
  uint8_t Data1 = cmd.Data1;
  uint8_t Data2 = cmd.Data2;
  String msg;
  uint8_t val;

  SP[sw].Device = Dev;
  SP[sw].Type = Type;
  SP[sw].Always_read = false; // Must be set, otherwise we will get unpredictable results
  DEBUGMSG("Loading switch " + String(sw) + " for device " + String(Dev));
  if (Dev < NUMBER_OF_DEVICES) {
    switch (SP[sw].Type) {
      case PATCH_SEL:
        {
          SP[sw].Colour = Device[Dev]->my_LED_colour; // Set the colour
          uint16_t this_patch = (Data2 * 100) + Data1;
          //if ((prev_device != Dev) || (prev_type != Type) || (prev_number != this_patch)) {
          SP[sw].PP_number = this_patch;
        }
        break;
      case PATCH_BANK:
        {
          SP[sw].Colour = Device[Dev]->my_LED_colour; // Set the colour
          SP[sw].Bank_position = Data1;
          SP[sw].Bank_size = Data2;
        }
        break;
      case BANK_UP:
      case BANK_DOWN:
      case PAR_BANK_UP:
      case PAR_BANK_DOWN:
        //case ASG_BANK_UP:
        //case ASG_BANK_DOWN:
        SP[sw].Colour = Device[Dev]->my_LED_colour;
        if (cmd.Device == CURRENT) SP[sw].Colour = CURRENT_DEVICE_COLOUR;
        break;
      case PREV_PATCH:
        SP[sw].Always_read = true; // Always needs re-reading after update
        SP[sw].Colour = Device[Dev]->my_LED_colour; // Set the colour
        break;
      case NEXT_PATCH:
        SP[sw].Always_read = true; // Always needs re-reading after update
        SP[sw].Colour = Device[Dev]->my_LED_colour; // Set the colour
        break;
      case DIRECT_SELECT:
        SP[sw].Colour = Device[Dev]->my_LED_colour; // Set the colour
        SP[sw].PP_number = Data1;
        break;
      case PARAMETER:
        SP[sw].Always_read = true; // Parameters always need re-reading
        SP[sw].PP_number = Data1; //Store parameter number
        SP[sw].Latch = Data2;
        SP[sw].Assign_min = cmd.Value1;
        SP[sw].Assign_max = cmd.Value2 + 1;
        SP[sw].Value1 = cmd.Value1;
        SP[sw].Value2 = cmd.Value2;
        SP[sw].Value3 = cmd.Value3;
        SP[sw].Value4 = cmd.Value4;
        //SP[sw].Value5 = cmd.Value5;
        break;
      case PAR_BANK:
        SP[sw].Always_read = true; // Parameters always need re-reading
        SP[sw].Bank_position = Data1;
        SP[sw].Bank_size = Data2;
        break;
      case ASSIGN:
        SP[sw].Always_read = true; // Parameters always need re-reading
        Device[Dev]->assign_load(sw, Data1, Data2);
        break;
      case OPEN_PAGE_DEVICE:
        SP[sw].PP_number = Data1; //Store page number
        break;
      case OPEN_PAGE_PATCH:
        SP[sw].PP_number = Device[Dev]->my_patch_page; //Store page number
        break;
      case OPEN_PAGE_PARAMETER:
        SP[sw].PP_number = Device[Dev]->my_parameter_page; //Store page number
        break;
      case OPEN_PAGE_ASSIGN:
        SP[sw].PP_number = Device[Dev]->my_assign_page; //Store page number
        break;
      case MUTE:
        SP[sw].Colour = Device[Dev]->my_LED_colour; // Set the colour
        break;
    }
  }
  else if (Dev == COMMON) {
    switch (SP[sw].Type) {
      case OPEN_PAGE:
      case MENU:
      case SET_TEMPO:
        SP[sw].Colour = Setting.LED_global_colour;
        SP[sw].PP_number = Data1; //Store page number
        SP[sw].Always_read = true;
        break;
      case MIDI_NOTE:
        SP[sw].PP_number = Data1;
        LCD_number_to_note(Data1, msg);
        SP[sw].Colour = Setting.MIDI_note_colour;
        LCD_set_label(sw, msg);
        break;
      case MIDI_PC:
        SP[sw].PP_number = Data1;
        SP[sw].Value1 = cmd.Data2; // Value1 stores MIDI channel
        SP[sw].Value2 = cmd.Value1; // value2 stores MIDI port
        SP[sw].Colour = Setting.MIDI_PC_colour;
        LCD_clear_label(sw);
        break;
      case MIDI_CC:
        SP[sw].PP_number = Data1;
        val = MIDI_recall_CC(Data1, cmd.Value3, SCO_MIDI_port(cmd.Value4));
        if (val == NOT_FOUND) SP[sw].Target_byte1 = cmd.Value2; // Default value for STEP/UPDOWN - starts with minimum value
        else SP[sw].Target_byte1 = val;
        SP[sw].Assign_max = cmd.Value1;
        SP[sw].Direction = UP;
        if (Data2 == CC_TOGGLE) SP[sw].State = 1;
        else SP[sw].State = 0;
        //DEBUGMSG("!!!MIDI_CC: val:" + String(val) + ", cmd.Value1:" + String(cmd.Value1) + ", cmd.Value2:" + String(cmd.Value2));
        if (val == cmd.Value1) SP[sw].State = 0;
        if (val == cmd.Value2) SP[sw].State = 1;
        SP[sw].Latch = Data2;
        SP[sw].Colour = Setting.MIDI_CC_colour;
        LCD_clear_label(sw);
        break;
    }
  }
  //if (SP[sw].Always_read) LCD_clear_label(sw); //Clear the label of this switch if it needs reading
}

/*void PAGE_read_internal_cmd(uint8_t Pg, uint8_t Sw, Cmd_struct* Cmd) {
  bool found = false;
  for (uint8_t c = 0; c < NUMBER_OF_INTERNAL_COMMANDS; c++) { // Run through the internal commands
    if ((Fixed_commands[c].Page == Pg) && (Fixed_commands[c].Switch == Sw)) { // Command found
      copy_cmd(&Fixed_commands[c], Cmd);
      SP[Sw].Cmd = c; // Remember the position of this number
      found = true;
      break;
    }
  }

  if (!found) {
    EEPROM_read_cmd(0, Sw, 0, Cmd); // Read the commands from the default bank
    SP[Sw].Cmd_from_default_bank = true;
  }
}*/

// ********************************* Section 3: Page Reading MIDI Data From Devices ********************************************

// This will read patch numbers and names and parameter states and names that are on the current page (SP array)

void PAGE_request_first_switch() {
  Current_switch = 0; //After the name is read, the assigns can be read
  //update_lcd = 1; //update first LCD before moving on...
  read_attempt = 1;
  DEBUGMSG("Start reading switch parameters");
  PAGE_request_current_switch();
}

void PAGE_request_next_switch() {
  //update_lcd = Current_switch; //update LCD before moving on...
  if ((reading_type != REFRESH_FX_ONLY) || (SP[Current_switch].Always_read)) LCD_update(Current_switch);
  Current_switch++;
  read_attempt = 1;
  PAGE_request_current_switch();
}

void PAGE_request_current_switch() { //Will request the data for the next switch
  if (Current_switch < NUMBER_OF_SWITCHES + NUMBER_OF_EXTERNAL_SWITCHES + 1) {
    request_next_switch = true;
    //DEBUGMSG("Switch " + String(Current_switch) + ": Reading_type: " + String(reading_type) + ", Always read: " + String(SP[Current_switch].Always_read));
    if ((reading_type != REFRESH_FX_ONLY) || (SP[Current_switch].Always_read)) {
      uint8_t Dev = SP[Current_switch].Device;
      if (Dev == CURRENT) Dev = Current_device;
      uint8_t Type = SP[Current_switch].Type;
      uint16_t new_patch;
      uint16_t prev_patch;
      uint8_t page;
      bool read_ok;
      DEBUGMAIN("Request parameter for switch " + String(Current_switch));
      //update_lcd = Current_switch; //update LCD before moving on...
      no_device_check = true; // Do not check for devices now

      if (Dev < NUMBER_OF_DEVICES) {
        switch (Type) {
          case PATCH_SEL:
            if (Device[Dev]->connected) {
              Device[Dev]->request_patch_name(SP[Current_switch].PP_number);  //Request the patch name
              PAGE_start_sysex_watchdog(); // Start the watchdog
              request_next_switch = false; // Do not read the next switch yet
            }
            else LCD_clear_label(Current_switch);
            break;
          case PATCH_BANK:
            new_patch = Device[Dev]->calculate_patch_number(SP[Current_switch].Bank_position - 1, SP[Current_switch].Bank_size);
            //if ((prev_device != Dev) || (prev_type != Type) || (prev_number != patch_bank_patch)) {
            prev_patch = SP[Current_switch].PP_number;
            SP[Current_switch].PP_number = new_patch;
            if (Device[Dev]->connected) {
              if ((reading_type) || (new_patch != prev_patch)) {
                Device[Dev]->request_patch_name(new_patch);  //Request the patch name
                PAGE_start_sysex_watchdog(); // Start the watchdog
                request_next_switch = false; // Do not read the next switch yet
              }
            }
            else LCD_clear_label(Current_switch);
            break;
          case PREV_PATCH:
            new_patch = Device[Dev]->prev_patch_number();
            SP[Current_switch].PP_number = new_patch;
            if (Device[Dev]->connected) {
              Device[Dev]->request_patch_name(new_patch);  //Request the patch name
              PAGE_start_sysex_watchdog(); // Start the watchdog
              request_next_switch = false; // Do not read the next switch yet
            }
            else LCD_clear_label(Current_switch);
            break;
          case NEXT_PATCH:
            new_patch = Device[Dev]->next_patch_number();
            SP[Current_switch].PP_number = new_patch;
            if (Device[Dev]->connected) {
              Device[Dev]->request_patch_name(new_patch);  //Request the patch name
              PAGE_start_sysex_watchdog(); // Start the watchdog
              request_next_switch = false; // Do not read the next switch yet
            }
            break;
          case PARAMETER:
            if (Device[Dev]->connected) {
              read_ok = Device[Dev]->request_parameter(SP[Current_switch].PP_number);  //Request the parameter for this device
              if (read_ok) { // Parameter has been read
                request_next_switch = true;
              }
              else { // Request has been sent - wait for the device to respond
                PAGE_start_sysex_watchdog(); // Start the watchdog
                request_next_switch = false;
              }
            }
            else LCD_clear_label(Current_switch);
            break;
          case PAR_BANK:
            if (Device[Dev]->connected) {
              uint16_t par = (Device[Dev]->parameter_bank_number * SP[Current_switch].Bank_size) + SP[Current_switch].Bank_position - 1;
              DEBUGMSG("****PAR NUMBER: " + String(Device[Dev]->parameter_bank_number) + " * " + String(SP[Current_switch].Bank_size) + " + " + String(SP[Current_switch].Bank_position) + " - 1 = " + String(par));
              if (par < Device[Dev]->number_of_parameters()) {
                SP[Current_switch].PP_number = par;
                if (Device[Dev]->number_of_values(par) == 2) {
                  SP[Current_switch].Latch = TOGGLE;
                }
                else {
                  if (Device[Dev]->number_of_values(par) > 50) SP[Current_switch].Latch = UPDOWN;
                  else SP[Current_switch].Latch = STEP;
                  SP[Current_switch].Assign_min = 0;
                  SP[Current_switch].Assign_max = Device[Dev]->number_of_values(par);
                }

                if (Device[Dev]->number_of_values(par) >= 1) {
                  Device[Dev]->request_parameter(par);  //Request the bytes for this parameter
                  PAGE_start_sysex_watchdog(); // Start the watchdog
                  request_next_switch = false; // Do not read the next switch yet
                }
                else {
                  SP[Current_switch].PP_number = 0;
                  SP[Current_switch].Latch = TGL_OFF;
                  SP[Current_switch].Colour = 0;
                  LCD_clear_label(Current_switch);
                  request_next_switch = true;
                }
              }
              else {
                SP[Current_switch].PP_number = 0;
                SP[Current_switch].Latch = TGL_OFF;
                SP[Current_switch].Colour = 0;
                LCD_clear_label(Current_switch);
                request_next_switch = true;
              }
            }
            else LCD_clear_label(Current_switch);
            break;
          case ASSIGN:
            //case ASG_BANK:
            if (Device[Dev]->connected) {
              Device[Dev]->request_current_assign();
              PAGE_start_sysex_watchdog(); // Start the watchdog
              request_next_switch = false; // Do not read the next switch yet
            }
            else LCD_clear_label(Current_switch);
            break;
          case OPEN_PAGE_DEVICE:
          case OPEN_PAGE_PATCH:
          case OPEN_PAGE_PARAMETER:
          case OPEN_PAGE_ASSIGN:
            page = SP[Current_switch].PP_number;
            //PAGE_lookup_title(page, page_label);
            EEPROM_read_title(page, 0, page_label);
            LCD_set_label(Current_switch, page_label);
            request_next_switch = true;
            break;
        }
      }
      if (Dev == COMMON) {
        switch (SP[Current_switch].Type) {
          case OPEN_PAGE:
            page = SP[Current_switch].PP_number;
            //PAGE_lookup_title(page, page_label);
            EEPROM_read_title(page, 0, page_label);
            page_label.trim();
            //page_label += Blank_line;
            LCD_set_label(Current_switch, page_label);
            request_next_switch = true;
            if (!SCO_valid_page(page) ) SP[Current_switch].Colour = 0; // Switch colour off if this page does not exist.
            break;
          case MENU:
            //DEBUGMSG(": Parameter start read at " + String(deltaT) + " us");
            menu_load(Current_switch);
            //DEBUGMSG("TIMING: Parameter done read at " + String(deltaT) + " us");
            request_next_switch = true;
            break;
        }
      }
    }
  }
  else {
    // Reading page is ready
    reading_type = OFF;
    PAGE_stop_sysex_watchdog(); // Stop the watchdog
    no_device_check = false; // Checking devices is OK again
    DEBUGMAIN("Done reading page");
  }
}

// Sysex watchdog will request the data for the current switch again in case the device did not respond in time.
void PAGE_start_sysex_watchdog() {
  SysexWatchdog = millis() + SYSEX_WATCHDOG_LENGTH;
  Sysex_watchdog_running = true;
  DEBUGMSG("Sysex watchdog started");
}

void PAGE_stop_sysex_watchdog() {
  Sysex_watchdog_running = false;
  DEBUGMSG("Sysex watchdog stopped");
}

void PAGE_check_sysex_watchdog() {
  if ((millis() > SysexWatchdog) && (Sysex_watchdog_running)) {
    DEBUGMSG("Sysex watchdog expired");
    read_attempt++;
    if (read_attempt > SYSEX_NUMBER_OF_READ_ATTEMPS) PAGE_request_next_switch();
    else PAGE_request_current_switch(); // Try reading the current parameter again
  }
}

void PAGE_check_first_connect(uint8_t dev) { // Will check if last connected device is the first that connects
  if (Current_device < NUMBER_OF_DEVICES) { // Quit if the current device is already connected
    if (Device[Current_device]->connected) return;
  }
  
  if (Current_device != dev) {
    Current_device = dev;
    SCO_select_page(Device[dev]->my_patch_page); // Select this device
  }
}

bool PAGE_check_on_page(uint8_t dev, uint16_t patch) { // Will check if the patch mentioned is currently on the page
  uint8_t bsize = 0; // We will also read the bank size in the process
  for (uint8_t s = 0; s < NUMBER_OF_SWITCHES; s++) { // Run through the switches on the current page
    if (((SP[s].Type == PATCH_SEL) || (SP[s].Type == PATCH_BANK)) && (SP[s].Device == dev) && (SP[s].PP_number == patch)) return true;
    if ((SP[s].Type == PATCH_BANK) && (SP[s].Device == dev)) bsize = SP[s].Bank_size;
  }

  if ((dev < NUMBER_OF_DEVICES) && (bsize > 0)) { // Update the bank number of this device
    uint16_t bnumber = patch / bsize;
    Device[dev]->bank_number = bnumber;
    Device[dev]->bank_select_number = bnumber;
  }
  return false;
}
