// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: Page Setup
// Section 2: Page Loading Into SP array
// Section 3: Page Reading MIDI Data From Devices

// ********************************* Section 1: Page Setup ********************************************

// A page contains a set of functions for internal and external switches and expression pedals.
// Every switch has a record in the SP[] array. From this array the VController can determine what data to show on the display and LED of this switch
// Every switch is first loaded and after that the data will be read.

// The page data van be updated in three levels:
// update_page = RELOAD_PAGE will reload and reread all the data from the devices
// update_page = REFRESH_PAGE will reread all the data from the devices, but not reload
// update_page = REFRESH_FX_ONLY will only reread the effects (where SP[].Refresh_with_FX_only is set to true)

// Reading data from the devices takes time. Therefore it can happen that a REFRESH_FX_ONLY is given while RELOAD_PAGE is still in progress. In that cas a RELOAD_PAGE will be
// executed for the entire page. That way all the displays will be updated with the correct data.

#define MINIMAL_PAGE_UPDATE_INTERVAL 200 // Minimal time between page updates
unsigned long PageUpdateTimer = 0;

#define SYSEX_WATCHDOG_LENGTH 300 // watchdog length for sysex messages (in msec).
unsigned long SysexWatchdog = 0; // This watchdog will check if a device responds. If it expires it will request the same parameter again.
boolean Sysex_watchdog_running = false;
#define SYSEX_NUMBER_OF_READ_ATTEMPS 3 // Number of times the watchdog will be restarted before moving to the next parameter

uint8_t read_attempt = 1;
uint8_t active_update_type = OFF; // Will ensure that all the patch names are read!
bool default_command; //Will indicate if the command that has been read is from the default page or not...
String page_label;
bool request_next_switch = false;
uint8_t switch_controlled_by_master_exp_pedal = 0;
uint8_t Current_switch = 255; // The parameter that is being read (pointer in the SP array)
uint8_t number_of_connected_devices = 0;

void setup_page()
{
  DEBUGMSG("Number of commands stored in external EEPROM: " + String(number_of_cmds));
  if ((Current_page >= Number_of_pages) && (Current_page < FIRST_FIXED_CMD_PAGE)) Current_page = DEFAULT_PAGE;
  PAGE_load_current();
  page_label.reserve(17);
}

void main_page() {
  //if (global_tuner_active) DEBUGMSG("Tuner active");
  if ((update_page != OFF) && (millis() > PageUpdateTimer)) {
    PageUpdateTimer = millis() + MINIMAL_PAGE_UPDATE_INTERVAL;
    if (active_update_type < update_page) active_update_type = update_page;
    switch (active_update_type) {
      case RELOAD_PAGE:
        DEBUGMAIN("** Reloading page " + String(Current_page));
        update_page = OFF;
        PAGE_load_current(); // Load and read the page
        break;
      case REFRESH_PAGE:
        DEBUGMAIN("** Refreshing page " + String(Current_page));
        update_page = OFF;
        PAGE_request_first_switch();
        break;
      case REFRESH_FX_ONLY:
        DEBUGMAIN("** Refreshing fx only on page " + String(Current_page));
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
  on_looper_page = false;
  for (uint8_t s = 0; s < (TOTAL_NUMBER_OF_SWITCHES + 1); s++) { // Load regular switches
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
  SP[sw].Refresh_with_FX_only = false; // Must be set, otherwise we will get unpredictable results
  DEBUGMSG("Loading switch " + String(sw) + " for device " + String(Dev));
  if (Dev < NUMBER_OF_DEVICES) {
    switch (SP[sw].Type) {
      case PATCH:
        {
          SP[sw].Sel_type = Data1;
          SP[sw].Colour = Device[Dev]->my_LED_colour; // Set the colour
          if (Data1 == SELECT) {
            SP[sw].PP_number = (cmd.Value1 * 100) + Data2;
          }
          if (Data1 == BANKSELECT) {
            SP[sw].Bank_position = Data2;
            SP[sw].Bank_size = cmd.Value1;
            Device[Dev]->update_bank_size(cmd.Value1);
          }
          if ((Data1 == PREV) || (Data1 == NEXT)) {
            SP[sw].Refresh_with_FX_only = true;
          }
        }
        break;
      case PAR_BANK_UP:
      case PAR_BANK_DOWN:
      case SAVE_PATCH:
        SP[sw].Colour = Device[Dev]->my_LED_colour;
        break;
      case DIRECT_SELECT:
        SP[sw].Refresh_with_FX_only = true;
        SP[sw].Colour = Device[Dev]->my_LED_colour; // Set the colour
        SP[sw].PP_number = Data1;
        break;
      case PARAMETER:
        SP[sw].Refresh_with_FX_only = true; // Parameters always need re-reading
        SP[sw].PP_number = Data1; //Store parameter number
        SP[sw].Latch = Data2;
        SP[sw].Assign_min = cmd.Value1;
        SP[sw].Assign_max = cmd.Value2 + 1;
        SP[sw].Value1 = cmd.Value1;
        SP[sw].Value2 = cmd.Value2;
        SP[sw].Value3 = cmd.Value3;
        SP[sw].Value4 = cmd.Value4;
        break;
      case PAR_BANK:
        SP[sw].Refresh_with_FX_only = true; // Parameters always need re-reading
        SP[sw].Bank_position = Data1;
        SP[sw].Bank_size = Data2;
        break;
      case PAR_BANK_CATEGORY:
        SP[sw].Colour = Device[Dev]->my_LED_colour;
        SP[sw].PP_number = Data1;
        break;
      case ASSIGN:
        SP[sw].Refresh_with_FX_only = true; // Parameters always need re-reading
        SP[sw].Sel_type = Data1;
        if (Data1 == SELECT) {
          Device[Dev]->assign_load(sw, Data2, cmd.Value1);
        }
        else if (Data1 == BANKSELECT) {
          SP[sw].Bank_position = Data2;
          SP[sw].Bank_size = cmd.Value1;
        }
        else {
          SP[sw].Colour = Device[Dev]->my_LED_colour;
        }
        break;
      case OPEN_PAGE_DEVICE:
        SP[sw].PP_number = Data1; //Store page number
        break;
      case OPEN_NEXT_PAGE_OF_DEVICE:
        SP[sw].PP_number = Device[Dev]->read_current_device_page(); //Store page number
        break;
      case MUTE:
        SP[sw].Colour = Device[Dev]->my_LED_colour; // Set the colour
        break;
      case TOGGLE_EXP_PEDAL:
        SP[sw].Refresh_with_FX_only = true;
        SP[sw].Colour = Device[Dev]->my_LED_colour; // Set the colour
        break;
      case MASTER_EXP_PEDAL:
        SP[sw].Refresh_with_FX_only = true;
        SP[sw].Exp_pedal = Data1;
        break;
      case SNAPSCENE:
        SP[sw].Value1 = Data1;
        SP[sw].Value2 = Data2;
        SP[sw].Value3 = cmd.Value1;
        SP[sw].PP_number = Data1;
        SP[sw].Colour = Device[Dev]->my_LED_colour; // Set the colour
        break;
      case LOOPER:
        SP[sw].PP_number = Data1;
        SP[sw].Colour = FX_LOOPER_TYPE; // Set the colour
        SP[sw].Refresh_with_FX_only = true;
        on_looper_page = true;
        break;
    }
  }
  else if (Dev == COMMON) {
    switch (SP[sw].Type) {
      case MENU:
      case SET_TEMPO:
        SP[sw].Colour = Setting.LED_global_colour;
        SP[sw].PP_number = Data1; //Store page number
        SP[sw].Refresh_with_FX_only = true;
        break;
      case PAGE:
        SP[sw].Sel_type = Data1;
        if (Data1 == SELECT) {
          SP[sw].PP_number = Data2; //Store page number
        }
        if (Data1 == BANKSELECT) {
          SP[sw].Bank_position = Data2;
          SP[sw].Bank_size = cmd.Value1;
        }
        SP[sw].Colour = Setting.LED_global_colour;
        SP[sw].Refresh_with_FX_only = true;
        break;
      case MIDI_NOTE:
        SP[sw].PP_number = Data1;
        LCD_number_to_note(Data1, msg);
        SP[sw].Colour = Setting.MIDI_note_colour;
        LCD_set_SP_label(sw, msg);
        break;
      case MIDI_PC:
        SP[sw].PP_number = Data1;
        SP[sw].Value1 = cmd.Data2; // Value1 stores MIDI channel
        SP[sw].Value2 = cmd.Value1; // value2 stores MIDI port
        SP[sw].Colour = Setting.MIDI_PC_colour;
        LCD_clear_SP_label(sw);
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
        LCD_clear_SP_label(sw);
        break;
    }
  }
}

// ********************************* Section 3: Page Reading MIDI Data From Devices ********************************************

// This will read patch numbers and names and parameter states and names that are on the current page (SP array)

void PAGE_request_first_switch() {
  Current_switch = 0; //After the name is read, the assigns can be read
  //update_lcd = 1; //update first LCD before moving on...
  read_attempt = 1;
  DEBUGMSG("Start reading switch parameters");
  PAGE_stop_sysex_watchdog();
  PAGE_request_current_switch();
}

void PAGE_request_next_switch() {
  if ((active_update_type != REFRESH_FX_ONLY) || (SP[Current_switch].Refresh_with_FX_only)) LCD_update(Current_switch, true);
  PAGE_stop_sysex_watchdog();
  Current_switch++;
  read_attempt = 1;
  PAGE_request_current_switch();
}

void PAGE_request_current_switch() { //Will request the data for the next switch
  if (Current_switch < TOTAL_NUMBER_OF_SWITCHES + 1) {
    request_next_switch = true; // By default we move on to the next page.
    //DEBUGMSG("Switch " + String(Current_switch) + ": active_update_type: " + String(active_update_type) + ", Always read: " + String(SP[Current_switch].Refresh_with_FX_only));
    if ((active_update_type != REFRESH_FX_ONLY) || (SP[Current_switch].Refresh_with_FX_only)) {
      uint8_t Dev = SP[Current_switch].Device;
      if (Dev == CURRENT) Dev = Current_device;
      uint8_t Type = SP[Current_switch].Type;
      uint16_t my_patch_number;
      uint8_t page;
      uint8_t asgn_num;
      String msg;
      DEBUGMAIN("Request data for switch " + String(Current_switch) + ", device " + String(Dev) + ", type: " + String(Type));
      //update_lcd = Current_switch; //update LCD before moving on...
      MIDI_disable_device_check(); // Do not check for devices now

      if (Dev < NUMBER_OF_DEVICES) {
        switch (Type) {
          case PATCH:
            if ((SP[Current_switch].Sel_type == BANKUP) || (SP[Current_switch].Sel_type == BANKDOWN)) break;
            if (SP[Current_switch].Sel_type == BANKSELECT) {
              my_patch_number = Device[Dev]->calculate_patch_number(SP[Current_switch].Bank_position - 1, SP[Current_switch].Bank_size);
              SP[Current_switch].PP_number = my_patch_number;
            }
            if (SP[Current_switch].Sel_type == NEXT) {
              my_patch_number = Device[Dev]->calculate_next_patch_number();
              SP[Current_switch].PP_number = my_patch_number;
            }
            if (SP[Current_switch].Sel_type == PREV) {
              my_patch_number = Device[Dev]->calculate_prev_patch_number();
              SP[Current_switch].PP_number = my_patch_number;
            }
            if ((Device[Dev]->can_request_sysex_data()) || (Dev == KPA)) {
              request_next_switch = Device[Dev]->request_patch_name(Current_switch, SP[Current_switch].PP_number);  //Request the patch name
              if (!request_next_switch) PAGE_start_sysex_watchdog();
              DEBUGMSG("Requesting patch name #" + String(SP[Current_switch].PP_number));
            }
            else LCD_clear_SP_label(Current_switch);
            break;
          case DIRECT_SELECT:
            if ((Device[Dev]->can_request_sysex_data()) && (Device[Dev]->valid_direct_select_switch(SP[Current_switch].PP_number))) {
              my_patch_number = Device[Dev]->direct_select_patch_number_to_request(SP[Current_switch].PP_number);
              request_next_switch = Device[Dev]->request_patch_name(Current_switch, my_patch_number);  //Request the patch name
              if (!request_next_switch) PAGE_start_sysex_watchdog(); // Start the watchdog
            }
            else {
              LCD_clear_SP_label(Current_switch);
            }
            break;
          case PARAMETER:
            request_next_switch = Device[Dev]->request_parameter(Current_switch, SP[Current_switch].PP_number);  //Request the parameter for this device
            if (!request_next_switch) PAGE_start_sysex_watchdog(); // Start the watchdog
            break;
          case PAR_BANK:
            {
              uint16_t par_num = (Device[Dev]->parameter_bank_number * SP[Current_switch].Bank_size) + SP[Current_switch].Bank_position - 1;
              uint16_t par_id = Device[Dev]->get_parbank_parameter_id(par_num); // If the device supports parameter categories, this will point to the parameter within the selected category
              //DEBUGMSG("****PARNUM: " + String(par_num) + ", PAR: " + String(par_id));
              if (par_num < Device[Dev]->number_of_parbank_parameters()) {

                // Determine if the pedal type: TOGGLE, STEP or UPDOWN
                SP[Current_switch].PP_number = par_id;
                if (Device[Dev]->number_of_values(par_id) == 2) { // If a certain parameter has two values, it will be a toggle switch
                  SP[Current_switch].Latch = TOGGLE;
                }
                else {
                  if (Device[Dev]->number_of_values(par_id) > 50) SP[Current_switch].Latch = UPDOWN; // If a certain parameter has more than 50 parameters, it will be an updown switch
                  else SP[Current_switch].Latch = STEP; // Otherwise it will be a step switch
                  SP[Current_switch].Assign_min = 0;
                  SP[Current_switch].Assign_max = Device[Dev]->number_of_values(par_id);
                }

                // Request the parameter name from the Device
                if (Device[Dev]->number_of_values(par_id) >= 1) {
                  request_next_switch = Device[Dev]->request_parameter(Current_switch, par_id);  //Request the bytes for this parameter
                  if (!request_next_switch) PAGE_start_sysex_watchdog(); // Start the watchdog
                }
                else { // Number_of_values for this parameter is zero
                  SP[Current_switch].PP_number = 0;
                  SP[Current_switch].Latch = TGL_OFF;
                  SP[Current_switch].Colour = 0;
                  LCD_clear_SP_label(Current_switch);
                  request_next_switch = true;
                }
              }
              else { // Parameter out of range
                SP[Current_switch].PP_number = 0;
                SP[Current_switch].Latch = TGL_OFF;
                SP[Current_switch].Colour = 0;
                LCD_clear_SP_label(Current_switch);
                //request_next_switch = true;
              }
            }
            break;
          case PAR_BANK_CATEGORY:
            Device[Dev]->request_par_bank_category_name(Current_switch);
            break;
          case ASSIGN:
            if (SP[Current_switch].Sel_type == SELECT) {
              if (Device[Dev]->can_request_sysex_data()) {
                Device[Dev]->request_current_assign(Current_switch);
                PAGE_start_sysex_watchdog(); // Start the watchdog
                request_next_switch = false; // Do not read the next switch yet
              }
              else LCD_clear_SP_label(Current_switch);
            }
            if (SP[Current_switch].Sel_type == BANKSELECT) {
              asgn_num = (Device[Dev]->assign_bank_number * SP[Current_switch].Bank_size) + SP[Current_switch].Bank_position - 1;
              if (asgn_num < Device[Dev]->get_number_of_assigns()) {
                SP[Current_switch].PP_number = asgn_num;
                Device[Dev]->assign_load(Current_switch, asgn_num, Device[Dev]->trigger_follow_assign(asgn_num));
                if (Device[Dev]->can_request_sysex_data()) {
                  Device[Dev]->request_current_assign(Current_switch);
                  PAGE_start_sysex_watchdog(); // Start the watchdog
                  request_next_switch = false; // Do not read the next switch yet
                }
                else LCD_clear_SP_label(Current_switch);
              }
              else { // Assign number out of range
                SP[Current_switch].PP_number = 0;
                SP[Current_switch].Latch = TGL_OFF;
                SP[Current_switch].Colour = 0;
                LCD_clear_SP_label(Current_switch);
              }
            }
            break;
          case OPEN_PAGE_DEVICE:
          case OPEN_NEXT_PAGE_OF_DEVICE:
            page = SP[Current_switch].PP_number;
            //PAGE_lookup_title(page, page_label);
            EEPROM_read_title(page, 0, page_label);
            LCD_set_SP_label(Current_switch, page_label);
            break;
          case TOGGLE_EXP_PEDAL:
            if (switch_controlled_by_master_exp_pedal > 0) { // If we are controlling an UPDOWN or STEP switch.
              LCD_set_SP_title(Current_switch, "[SWITCH " + String(switch_controlled_by_master_exp_pedal) + ']');
              msg = "";
              Device[Dev]->read_parameter_title(SP[switch_controlled_by_master_exp_pedal].PP_number, msg);
              msg += ':';
              msg += SP[switch_controlled_by_master_exp_pedal].Label;
              LCD_set_SP_label(Current_switch, msg); // Copy the title and label from that switch
              break;
            }
            Device[Dev]->set_expr_title(Current_switch);
            if (Device[Dev]->can_request_sysex_data()) { // Otherwise request the label from the device
              request_next_switch = Device[Dev]->request_exp_pedal(Current_switch, 0);  //Request the parameter for this device
              if (!request_next_switch) PAGE_start_sysex_watchdog();
            }
            else LCD_clear_SP_label(Current_switch);
            break;
          case SNAPSCENE:
            if ((Device[Dev]->current_snapscene == SP[Current_switch].Value2) && (SP[Current_switch].Value3 > 0)) {
              SP[Current_switch].PP_number = SP[Current_switch].Value3;
            }
            else if ((Device[Dev]->current_snapscene == SP[Current_switch].Value1) && (SP[Current_switch].Value2 > 0)) {
              SP[Current_switch].PP_number = SP[Current_switch].Value2;
            }
            else {
              SP[Current_switch].PP_number = SP[Current_switch].Value1;
            }
            request_next_switch = Device[Dev]->request_snapscene_name(Current_switch, SP[Current_switch].PP_number);  //Request the parameter for this device
            if (!request_next_switch) PAGE_start_sysex_watchdog(); // Start the watchdog
            //LCD_clear_SP_label(Current_switch);
            break;
          case LOOPER:
            Device[Dev]->request_looper_label(Current_switch);
            break;
          case MASTER_EXP_PEDAL:
            if (Device[Dev]->can_request_sysex_data()) {
              request_next_switch = Device[Dev]->request_exp_pedal(Current_switch, SP[Current_switch].Exp_pedal);  //Request the parameter for this device
              if (!request_next_switch) PAGE_start_sysex_watchdog(); // Start the watchdog
            }
            else LCD_clear_SP_label(Current_switch);
            break;
        }
        // Set main backlight colour
        if (Current_switch == 1) {
          if (on_looper_page) {
            uint8_t c = Device[Dev]->request_looper_backlight_colour();
            if (c > 0) Main_backlight_show_colour(c);
            else Main_backlight_show_colour(6); // show white
          }
          else Main_backlight_show_colour(Device[Current_device]->my_LED_colour);
        }
      }
      else if (Dev == COMMON) {
        switch (SP[Current_switch].Type) {
          case MENU:
            menu_load(Current_switch);
            break;
          case PAGE:
            if ((SP[Current_switch].Sel_type == BANKUP) || (SP[Current_switch].Sel_type == BANKDOWN)) break;
            if (SP[Current_switch].Sel_type == BANKSELECT) {
              page = (page_bank_select_number * SP[Current_switch].Bank_size) + SP[Current_switch].Bank_position;
              if (page >= Number_of_pages) page += FIRST_SELECTABLE_FIXED_CMD_PAGE - Number_of_pages; // Jump over the gap in the pages to reach the fixed ones
              SP[Current_switch].PP_number = page;
              SP[Current_switch].Colour = Setting.LED_global_colour;
            }
            else {
              page = SP[Current_switch].PP_number;
              if (page == 0) page = Previous_page;
            }
            if (SCO_valid_page(page)) {
              EEPROM_read_title(page, 0, page_label);
              page_label.trim();
              LCD_set_SP_label(Current_switch, page_label);
            }
            else {
              SP[Current_switch].Colour = 0; // Switch colour off if this page does not exist.
              LCD_clear_SP_label(Current_switch);
            }
            break;
        }
        // Set main backlight colour
        if (Current_switch == 1) Main_backlight_show_colour(6); // Main display colour white
      }
    }
  }
  else {
    // Reading page is ready
    active_update_type = OFF;
    PAGE_stop_sysex_watchdog(); // Stop the watchdog
    MIDI_enable_device_check(); // Checking devices is OK again
    DEBUGMAIN("Done reading page");
  }
}

// Sysex watchdog will request the data for the current switch again in case the device did not respond in time.
void PAGE_start_sysex_watchdog() {
  SysexWatchdog = millis() + SYSEX_WATCHDOG_LENGTH;
  Sysex_watchdog_running = true;
  //DEBUGMSG("Sysex watchdog started");
}

void PAGE_stop_sysex_watchdog() {
  if (Sysex_watchdog_running) {
    Sysex_watchdog_running = false;
    //DEBUGMSG("Sysex watchdog stopped");
  }
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
  number_of_connected_devices++;
  if (number_of_connected_devices == 1) {
    set_current_device(dev);
    if ((Current_device < NUMBER_OF_DEVICES) && (Current_page != PAGE_MENU)) SCO_select_page(Device[dev]->read_current_device_page()); // Load the patch page associated to this device
  }
}

void PAGE_check_disconnect(uint8_t dev) {
  number_of_connected_devices--;
  if ((number_of_connected_devices == 0) || (Current_device == dev)) SCO_select_next_device();
}

bool PAGE_check_on_page(uint8_t dev, uint16_t patch) { // Will check if the patch mentioned is currently on the page
  bool dev_on_page = false;
  for (uint8_t s = 1; s < NUMBER_OF_SWITCHES + 1; s++) { // Run through the switches on the current page
    if ((SP[s].Type == PATCH) && (SP[s].Device == dev)) {
      dev_on_page = true;
      if (SP[s].PP_number == patch) return true;
    }
  }

  if (dev < NUMBER_OF_DEVICES) { // Update the bank number of this device
    Device[dev]->update_bank_number(patch);
  }
  return !dev_on_page;
}
