// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: Switch Action Trigger and Command Lookup
// Section 2: Command Execution
// Section 3: Parameter State Control
// Section 4: MIDI CC Commands
// Section 5: Page Selection Commands
// Section 6: Global Tuner Commands
// Section 7: Global Tap Tempo Commands
// Section 8: Bass Mode (Low/High String Priority)
// Section 9: VController Power On/Off Switching
// Section 10: Master expression pedal control

// ********************************* Section 1: Switch Action Trigger and Command Lookup ********************************************

#ifdef CONFIG_CUSTOM
#define PAGE_KPA_LOOPER PAGE_CUSTOM_KPA_LOOPER
#define PAGE_FULL_LOOPER PAGE_CUSTOM_FULL_LOOPER
#else
#ifdef CONFIG_VCTOUCH
#define PAGE_KPA_LOOPER PAGE_VCTOUCH_KPA_LOOPER
#define PAGE_FULL_LOOPER PAGE_VCTOUCH_FULL_LOOPER
#else
#ifdef CONFIG_VCMINI
#define PAGE_KPA_LOOPER PAGE_VCMINI_KPA_LOOPER
#define PAGE_FULL_LOOPER PAGE_VCMINI_FULL_LOOPER
#else
#define PAGE_KPA_LOOPER PAGE_VC_KPA_LOOPER
#define PAGE_FULL_LOOPER PAGE_VC_FULL_LOOPER
#endif
#endif
#endif

bool updown_direction_can_change;
bool master_expr_from_cc = false;

// Memory buffer for commands - because the commands have to be read from external EEPROM over i2c, we speed things up by storing up to 50 in local RAM
#define CMD_BUFFER_SIZE 50
Cmd_struct cmd_buf[CMD_BUFFER_SIZE]; // We store fifty commands to allow for faster access
uint8_t current_cmdbuf_index = 0; // Index to the cmd_buf array

uint8_t prev_switch_pressed = 0; // To allow detection of re-pressing a switch and reading the commands from the buffer instead of external EEPROM
uint8_t prev_switch_page = 0; // The page the previous command was on.
uint16_t current_cmd = 0; // The current command that is being executed
uint8_t current_cmd_switch = 0; // Placeholder for the current_switch where the action is taken on

uint8_t current_cmd_switch_action = 0; // Placeholder for the switch action of the current command. Types are below:
#define SWITCH_PRESSED 1
#define SWITCH_RELEASED 2
#define SWITCH_LONG_PRESSED 3
#define SWITCH_LONG_RELEASED 4
#define SWITCH_HELD 5
#define SWITCH_DUAL_PRESSED 6
#define SWITCH_DUAL_RELEASED 7
#define SWITCH_DUAL_LONG_PRESSED 8
#define SWITCH_DUAL_HELD 9

uint8_t arm_page_cmd_exec = 0; // Set to the number of the selected page to execute the commands for this page

void setup_switch_control()
{
  SCO_MIDI_clock_start();
}

// Take action on switch being pressed / released/ held / long pressed or extra long pressed.
// There can be any number of commands executed by one switch press. The maximum number of commands for one switch is determined by the size of the buffer (CMD_BUFFER_SIZE).
// On every loop cycle one command is executed. THe next command to be executed is set in the current_cmd variable.
// Commands read from EEPROM are stored in a memory buffer (cmd_buf) in RAM to increase performance.

void main_switch_control()  // Checks if a button has been pressed and check out which functions have to be executed
{
  if (switch_released > 0) { // When switch is released, set current_cmd_switch_action to SWITCH_RELEASED and let current_cmd point to the first command for this switch
    if (switch_released & ON_LONG_PRESS) current_cmd_switch_action = SWITCH_LONG_RELEASED;
    else if (switch_released & ON_DUAL_PRESS) current_cmd_switch_action = SWITCH_DUAL_RELEASED;
    else current_cmd_switch_action = SWITCH_RELEASED;
    current_cmd = EEPROM_first_cmd(Current_page, switch_released & SWITCH_MASK);
    current_cmd_switch = switch_released & SWITCH_MASK;
    current_cmdbuf_index = 0;
    switch_released = 0;
  }

  if (switch_pressed > 0) {
    // Check if we are in tuner mode - pressing any key will stop tuner mode
    if (global_tuner_active) {
      SCO_global_tuner_stop();
      SC_skip_release_and_hold_until_next_press(SKIP_RELEASE | SKIP_LONG_PRESS | SKIP_HOLD);
    }
    else { // When switch is pressed, set current_cmd_switch_action to SWITCH_PRESSED or SWITCH_PRESSED_REPEAT and let current_cmd point to the first command for this switch
      if (switch_pressed & ON_DUAL_PRESS) current_cmd_switch_action = SWITCH_DUAL_PRESSED;
      else current_cmd_switch_action = SWITCH_PRESSED;
      current_cmd = EEPROM_first_cmd(Current_page, switch_pressed & SWITCH_MASK); // Is always fast, because first_cmd is read from the index which is in RAM
      //DEBUGMSG("Current cmd:" + String(current_cmd) + " on page:" + String(Current_page));
      current_cmd_switch = switch_pressed;
      //DEBUGMSG("Current_cmd_switch: " + String(current_cmd_switch));
      current_cmdbuf_index = 0;
    }
    switch_pressed = 0;
  }

  if (switch_long_pressed > 0) { // When switch is long pressed, set current_cmd_switch_action to SWITCH_LONG_PRESSED and let current_cmd point to the first command for this switch
    //SCO_switch_long_pressed_commands(Current_page, switch_long_pressed);
    if (switch_pressed & ON_DUAL_PRESS) current_cmd_switch_action = SWITCH_DUAL_LONG_PRESSED;
    else current_cmd_switch_action = SWITCH_LONG_PRESSED;
    current_cmd = EEPROM_first_cmd(Current_page, switch_long_pressed & SWITCH_MASK);
    current_cmd_switch = switch_long_pressed;
    current_cmdbuf_index = 0;
    switch_long_pressed = 0;
  }

  if (switch_held > 0) { // When switch is held, set current_cmd_switch_action to SWITCH_HELD and let current_cmd point to the first command for this switch
    //SCO_switch_held_commands(Current_page, switch_held);
    if (switch_pressed & ON_DUAL_PRESS) current_cmd_switch_action = SWITCH_DUAL_HELD;
    else current_cmd_switch_action = SWITCH_HELD;
    current_cmd = EEPROM_first_cmd(Current_page, switch_held & SWITCH_MASK);
    current_cmd_switch = switch_held;
    current_cmdbuf_index = 0;
    switch_held = 0;
  }

#ifdef POWER_SWITCH_NUMBER
  if (switch_extra_long_pressed == POWER_SWITCH_NUMBER) { // When switch is extra long pressed and it is the power switch, turn off the VController
    SCO_select_page(Previous_page, Previous_device); // "Undo"
    SCO_switch_power_off();
  }
#endif
  switch_extra_long_pressed = 0;

  if ((current_cmd == 0) && (arm_page_cmd_exec > 0)) { // Execute any commands on page selection
    // We do this when current_cmd is 0, to allow running commands to finish first.
    current_cmd_switch_action = SWITCH_PRESSED;
    current_cmd = EEPROM_first_cmd(arm_page_cmd_exec, 0);
    if (current_cmd > 0) {
      DEBUGMAIN("** Trigger execution of default commands for page " + String(arm_page_cmd_exec));
    }
    current_cmd_switch = 0;
    current_cmdbuf_index = 0;
    arm_page_cmd_exec = 0;
  }

  if (current_cmd > 0) { // If current_cmd points to a command we can execute, do it. Then check if there is another command to execute.
    SCO_execute_cmd(current_cmd_switch, current_cmd_switch_action, current_cmdbuf_index);
    current_cmd = EEPROM_next_cmd(current_cmd); //Find the next command - will be executed on the next cycle
    current_cmdbuf_index++; // Point to the next command in the command buffer
    if (current_cmdbuf_index >= CMD_BUFFER_SIZE) current_cmd = 0; // Stop executing commands when the end of the buffer is reached.
  }
  else {
    prev_switch_pressed = current_cmd_switch & SWITCH_MASK; // Will stop the reading of switch commands from EEPROM
  }

  SCO_check_update_tempo();
}

// ********************************* Section 2: Command Execution ********************************************

void SCO_execute_cmd(uint8_t sw, uint8_t action, uint8_t index) {
  if ((prev_switch_pressed != (sw & SWITCH_MASK)) || (prev_switch_page != Current_page)) {
    // Here we read the switch and store the value in the command buffer. This is only done on first press. After that the commands are executed from the buffer.
    // This allows for smoother expression pedal operation, where we trigger the same commands in quick succesion
    prev_switch_page = Current_page; // Remember the page the switch was on, so repressing it after changing page will cause a re-read from EEPROM.
    read_cmd_EEPROM(current_cmd, &cmd_buf[index]);
    DEBUGMSG("Cmd buffer read from EEPROM. Switch number is " + String(cmd_buf[index].Switch));
  }

  uint8_t switch_type = cmd_buf[index].Switch & SWITCH_TYPE_MASK;
  //DEBUGMSG("Checking Cmd index: " +  String(index) + ", switch type: " + String(switch_type) + ", switch: " + String(sw) + ", action: " + String(action));

  switch (action) {
    case SWITCH_PRESSED:
      if ((switch_type & ON_DUAL_PRESS) == 0) SCO_execute_command_press(sw & SWITCH_MASK, &cmd_buf[index], (index == 0));
      if (switch_type == 0) {
        DEBUGMSG("Pressed");
        SCO_execute_command(sw, &cmd_buf[index], (index == 0));
      }
      break;
    case SWITCH_RELEASED:
      if (switch_type == ON_RELEASE) {
        DEBUGMSG("Released");
        SCO_execute_command(sw & SWITCH_MASK, &cmd_buf[index], (index == 0));
      }
      if ((switch_type & ON_DUAL_PRESS) == 0) SCO_execute_command_release(sw & SWITCH_MASK, &cmd_buf[index], (index == 0));
      break;
    case SWITCH_LONG_PRESSED:
      //SCO_execute_command_long_pressed(sw & SWITCH_MASK, &cmd_buf[index], (index == 0));
      if (switch_type == ON_LONG_PRESS) {
        DEBUGMSG("Long Pressed");
        SCO_execute_command(sw & SWITCH_MASK, &cmd_buf[index], (index == 0));
      }
      break;
    case SWITCH_LONG_RELEASED:
      if ((switch_type & ON_DUAL_PRESS) == 0) {
        DEBUGMSG("Long Released");
        SCO_execute_command_release(sw & SWITCH_MASK, &cmd_buf[index], (index == 0));
      }
      break;
    case SWITCH_HELD:
      if ((switch_type == 0) || (switch_type == ON_RELEASE)) {
        DEBUGMSG("Switch Held");
        SCO_execute_command_held(sw & SWITCH_MASK, &cmd_buf[index], (index == 0));
      }
      break;
    case SWITCH_DUAL_PRESSED:
      if (switch_type == ON_DUAL_PRESS) {
        DEBUGMSG("Dual pressed");
        SCO_execute_command_press(sw & SWITCH_MASK, &cmd_buf[index], (index == 0));
        SCO_execute_command(sw & SWITCH_MASK, &cmd_buf[index], (index == 0));
      }
      break;
    case SWITCH_DUAL_RELEASED:
      if (switch_type == (ON_DUAL_PRESS | ON_RELEASE)) {
        DEBUGMSG("Dual released");
        SCO_execute_command_release(sw & SWITCH_MASK, &cmd_buf[index], (index == 0));
        SCO_execute_command(sw & SWITCH_MASK, &cmd_buf[index], (index == 0));
      }
      break;
    case SWITCH_DUAL_LONG_PRESSED:
      if (switch_type == (ON_DUAL_PRESS | ON_LONG_PRESS)) {
        DEBUGMSG("Dual long pressed");
        //SCO_execute_command_long_pressed(sw & SWITCH_MASK, &cmd_buf[index], (index == 0));
        SCO_execute_command(sw & SWITCH_MASK, &cmd_buf[index], (index == 0));
      }
      break;
    case SWITCH_DUAL_HELD:
      if (switch_type == ON_DUAL_PRESS) {
        DEBUGMSG("Dual held");
        SCO_execute_command_held(sw & SWITCH_MASK, &cmd_buf[index], (index == 0));
      }
      break;
  }
}

void SCO_trigger_default_page_cmds(uint8_t Pg) {
  arm_page_cmd_exec = Pg;
}

void SCO_execute_command(uint8_t Sw, Cmd_struct *cmd, bool first_cmd) {

  uint8_t Dev = cmd->Device;
  if (Dev == CURRENT) Dev = Current_device;
  uint8_t Type = cmd->Type;
  uint8_t Data1 = cmd->Data1;
  uint8_t Data2 = cmd->Data2;

  DEBUGMAIN("Execute command " + String(Type) + " for device " + String(Dev));

  if (Dev == COMMON) { // Check for common parameters
    switch (Type) {
      case TAP_TEMPO:
        if (SC_switch_is_expr_pedal()) break;
        if (SC_switch_triggered_by_PC()) break;
        SCO_global_tap_tempo_press(Sw);
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case SET_TEMPO:
        if (SC_switch_is_expr_pedal()) break;
        if (SC_switch_is_encoder()) {
          SCO_set_global_tempo_with_encoder(Enc_value);
          break;
        }
        if (SC_switch_triggered_by_PC()) SCO_set_global_tempo_press(PC_value);
        else SCO_set_global_tempo_press(Data1);
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case PAGE:
        if (SC_switch_is_expr_pedal()) break;
        if (SC_switch_triggered_by_PC()) {
          SCO_trigger_default_page_cmds(PC_value);
          SCO_select_page(PC_value);
          update_main_lcd = true;
          update_page = REFRESH_PAGE;
          switch_controlled_by_master_exp_pedal = 0;
          break;
        }
        if (Data1 == SELECT) {
          if (Data2 == 0) Data2 = Previous_page;
          SCO_trigger_default_page_cmds(Data2);
          SCO_select_page(Data2);
        }
        if (Data1 == NEXT) {
          if (SC_switch_is_encoder()) SCO_page_up_down(Enc_value);
          else SCO_page_up_down(1);
        }
        if (Data1 == PREV) {
          if (SC_switch_is_encoder()) SCO_page_up_down(Enc_value);
          else SCO_page_up_down(-1);
        }
        if (Data1 == BANKSELECT) {
          SCO_trigger_default_page_cmds(SP[Sw].PP_number);
          SCO_select_page(SP[Sw].PP_number);
          page_last_selected = SP[Sw].PP_number;
        }
        if (Data1 == BANKUP) {
          if (SC_switch_is_encoder()) SC_page_bank_updown(Enc_value, Data2);
          else SC_page_bank_updown(1, Data2);
          update_main_lcd = true;
          update_page = REFRESH_PAGE;
        }
        if (Data1 == BANKDOWN) {
          if (SC_switch_is_encoder()) SC_page_bank_updown(Enc_value, Data2);
          else SC_page_bank_updown(-1, Data2);
          update_main_lcd = true;
          update_page = REFRESH_PAGE;
        }
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case MENU:
        if (SC_switch_is_expr_pedal()) break;
        if (SC_switch_triggered_by_PC()) break;
        if (SC_switch_is_encoder()) menu_encoder_turn(Sw, Enc_value);
        else menu_press(Sw, true);
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case SELECT_NEXT_DEVICE:
        if (SC_switch_is_expr_pedal()) break;
        if (SC_switch_triggered_by_PC()) break;
        SCO_select_next_device();
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case GLOBAL_TUNER:
        if (SC_switch_is_expr_pedal()) break;
        if (SC_switch_triggered_by_PC()) break;
        SCO_global_tuner_toggle();
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case MIDI_CC:
        SCO_CC_press(Data1, Data2, cmd->Value1, cmd->Value2, cmd->Value3, SCO_MIDI_port(cmd->Value4), Sw, first_cmd);
        if (SC_switch_triggered_by_PC()) break;
        if (!SC_switch_is_expr_pedal()) {
          update_page = REFRESH_PAGE;
        }
        break;
      case MIDI_PC:
        if (SC_switch_is_expr_pedal()) break;
        if (SC_switch_triggered_by_PC()) {
          MIDI_send_PC(Data1, Data2, SCO_MIDI_port(PC_value));
          MIDI_remember_PC(Data1, Data2, SCO_MIDI_port(PC_value));
          break;
        }
        MIDI_send_PC(Data1, Data2, SCO_MIDI_port(cmd->Value1));
        MIDI_remember_PC(Data1, Data2, SCO_MIDI_port(cmd->Value1));
        switch_controlled_by_master_exp_pedal = 0;
        break;
    }
  }
  if (Dev < NUMBER_OF_DEVICES) { // Check for device specific parameters
    bool updated = false;
    uint16_t pnumber = 0;
    switch (Type) {
      case PATCH:
        if (SC_switch_is_expr_pedal()) break;
        if (SC_switch_triggered_by_PC()) {
          Device[Dev]->patch_select_pressed(PC_value);
          Device[Dev]->update_bank_number(PC_value); // Update the bank number
          //Device[Dev]->current_patch_name = SP[Sw].Label; // Store current patch name
          mute_all_but_me(Dev); // mute all the other devices
          //if (SP[Sw].Label[0] != ' ') LCD_show_popup_label(SP[Sw].Label, ACTION_TIMER_LENGTH);
          update_page = RELOAD_PAGE;
          break;
        }
        switch_controlled_by_master_exp_pedal = 0;
        if (Data1 == BANKUP) {
          if (SC_switch_is_encoder()) Device[Dev]->bank_updown(Enc_value, Data2);
          else Device[Dev]->bank_updown(1, Data2);
          update_main_lcd = true;
          update_page = REFRESH_PAGE;
          break;
        }
        if (Data1 == BANKDOWN) {
          if (SC_switch_is_encoder()) Device[Dev]->bank_updown(Enc_value, Data2);
          else Device[Dev]->bank_updown(-1, Data2);
          update_main_lcd = true;
          update_page = REFRESH_PAGE;
          break;
        }
        if (SC_switch_is_encoder()) {
          if (Enc_value == 0) break;
          if (Enc_value > 0) pnumber = Device[Dev]->calculate_next_patch_number();
          if (Enc_value < 0) pnumber = Device[Dev]->calculate_prev_patch_number();
        }
        else {
          if (Data1 == SELECT) pnumber = (cmd->Value1 * 100) + Data2;
          else if (Data1 == PREV) pnumber = Device[Dev]->calculate_prev_patch_number();
          else if (Data1 == NEXT) pnumber = Device[Dev]->calculate_next_patch_number();
          else pnumber = SP[Sw].PP_number;
        }
        Device[Dev]->patch_select_pressed(pnumber);
        Device[Dev]->current_patch_name = SP[Sw].Label; // Store current patch name
        mute_all_but_me(Dev); // mute all the other devices
        update_page = RELOAD_PAGE;
        break;
      case DIRECT_SELECT:
        if (SC_switch_is_expr_pedal()) break;
        if (SC_switch_triggered_by_PC()) {
          Device[Dev]->direct_select_press(PC_value % 10);
          update_page = RELOAD_PAGE;
          switch_controlled_by_master_exp_pedal = 0;
          break;
        }
        Device[Dev]->direct_select_press(Data1);
        update_page = RELOAD_PAGE;
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case PARAMETER:
        if (SC_switch_triggered_by_PC()) break;
        //if (Data2 == UPDOWN) break; // updown parameters are executed in SCO_command_press()
        if (first_cmd) updated = SCO_update_parameter_state(Sw, cmd->Value1, cmd->Value2, cmd->Value3); // Passing min, max and step value for STEP, RANGE and UPDOWN style pedal
        if (updated) Device[Dev]->parameter_press(Sw, cmd, Data1);
        break;
      case PAR_BANK:
        if (SC_switch_triggered_by_PC()) break;
        //if (Data2 == UPDOWN) break; // updown parameters are executed in SCO_command_press()
        if (first_cmd) updated = SCO_update_parameter_state(Sw, 0, Device[Dev]->number_of_values(SP[Sw].PP_number) - 1, 1); // Passing min, max and step value for STEP, RANGE and UPDOWN style pedal
        if (updated) Device[Dev]->parameter_press(Sw, cmd, SP[Sw].PP_number);
        break;
      case PAR_BANK_CATEGORY:
        if (SC_switch_is_expr_pedal()) break;
        if (SC_switch_triggered_by_PC()) break;
        Device[Dev]->select_parameter_bank_category(SP[Sw].PP_number);
        SCO_select_page(PAGE_CURRENT_PARAMETER, Dev);
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case PAR_BANK_UP:
        if (SC_switch_is_expr_pedal()) break;
        if (SC_switch_triggered_by_PC()) break;
        if (SC_switch_is_encoder()) Device[Dev]->par_bank_updown(Enc_value, Data1);
        else Device[Dev]->par_bank_updown(1, Data1);
        update_main_lcd = true;
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case PAR_BANK_DOWN:
        if (SC_switch_is_expr_pedal()) break;
        if (SC_switch_triggered_by_PC()) break;
        if (SC_switch_is_encoder()) Device[Dev]->par_bank_updown(Enc_value, Data1);
        else Device[Dev]->par_bank_updown(-1, Data1);
        update_main_lcd = true;
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case ASSIGN:
        if (SC_switch_triggered_by_PC()) break;
        if ((Data1 == SELECT) || (Data1 == BANKSELECT)) {
          if (first_cmd) updated = SCO_update_parameter_state(Sw, 0, 1, 1);
          if (updated) {
            if (SC_switch_is_expr_pedal()) Device[Dev]->assign_press(Sw, Expr_ped_value);
            else Device[Dev]->assign_press(Sw, 127);
          }
        }
        if (Data1 == BANKUP) {
          if (SC_switch_is_expr_pedal()) break;
          if (SC_switch_is_encoder()) Device[Dev]->asgn_bank_updown(Enc_value, Data2);
          else Device[Dev]->asgn_bank_updown(1, Data2);
          update_main_lcd = true;
          switch_controlled_by_master_exp_pedal = 0;
        }
        if (Data1 == BANKDOWN) {
          if (SC_switch_is_expr_pedal()) break;
          if (SC_switch_is_encoder()) Device[Dev]->asgn_bank_updown(Enc_value, Data2);
          else Device[Dev]->asgn_bank_updown(-1, Data2);
          update_main_lcd = true;
          switch_controlled_by_master_exp_pedal = 0;
        }
        break;
      case MUTE:
        if (SC_switch_is_expr_pedal()) break;
        if (SC_switch_triggered_by_PC()) break;
        Device[Dev]->mute();
        update_LEDS = true;
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case OPEN_PAGE_DEVICE:
        if (SC_switch_is_expr_pedal()) break;
        if (SC_switch_triggered_by_PC()) break;
        SCO_trigger_default_page_cmds(Data1);
        SCO_select_page(Data1, Dev);
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case OPEN_NEXT_PAGE_OF_DEVICE:
        if (SC_switch_is_expr_pedal()) break;
        if (SC_switch_triggered_by_PC()) break;
        SCO_select_next_page_of_device(Dev);
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case TOGGLE_EXP_PEDAL:
        if (SC_switch_is_expr_pedal()) break;
        if (SC_switch_triggered_by_PC()) break;
        if (switch_controlled_by_master_exp_pedal > 0) { // If MEP is currently set to control some other parameter, undo that.
          update_page = REFRESH_FX_ONLY;
        }
        else {
          Device[Dev]->toggle_expression_pedal(Sw);
        }
        switch_controlled_by_master_exp_pedal = 0;
#ifdef IS_VCMINI
        LCD_show_popup_label(SP[Sw].Title, ACTION_TIMER_LENGTH);
#endif
        break;
      case MASTER_EXP_PEDAL:
        if (SC_switch_triggered_by_PC()) break;
        if (SC_switch_is_expr_pedal()) SCO_move_master_exp_pedal(Sw, Dev);
        break;
      case SNAPSCENE:
        if (SC_switch_is_expr_pedal()) { // Is used when controlling snapscene via MIDI CC command
          Device[Dev]->set_snapscene(Sw, Expr_ped_value + 1);
          update_page = REFRESH_PAGE;
          switch_controlled_by_master_exp_pedal = 0;
          break;
        }
        mute_all_but_me(Dev); // mute all the other devices
        if (SC_switch_triggered_by_PC()) {
          Device[Dev]->set_snapscene(Sw, PC_value + 1);
          update_page = REFRESH_PAGE;
          switch_controlled_by_master_exp_pedal = 0;
          break;
        }
        if (first_cmd) Device[Dev]->set_snapscene(Sw, SP[Sw].PP_number);
        else Device[Dev]->set_snapscene(Sw, Data1);  // When not the first command, the first snapshot value set in the command is sent.
        update_page = REFRESH_PAGE;
        update_main_lcd = true;
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case LOOPER:
        if (SC_switch_is_expr_pedal()) break;
        if (SC_switch_triggered_by_PC()) break;
        Device[Dev]->looper_press(Data1, true);
        MIDI_send_looper_state(Dev, Data1);
        update_page = REFRESH_PAGE;
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case SAVE_PATCH:
        if (SC_switch_is_expr_pedal()) break;
        if (SC_switch_triggered_by_PC()) break;
        if (Dev == KTN) {
          My_KTN.save_patch();
        }
        if (Dev == SY1000) {
          My_SY1000.save_scene();
        }
        switch_controlled_by_master_exp_pedal = 0;
        break;
    }
  }
}

void SCO_execute_command_press(uint8_t Sw, Cmd_struct *cmd, bool first_cmd) {
  uint8_t Dev = cmd->Device;
  if (Dev == CURRENT) Dev = Current_device;

  //DEBUGMSG("Press -> execute command " + String(Type) + " for device " + String(Dev));

  if (Dev == COMMON) { // Check for common parameters
    switch (cmd->Type) {
      case MIDI_NOTE:
        MIDI_send_note_on(cmd->Data1, cmd->Data2, cmd->Value1, SCO_MIDI_port(cmd->Value2));
        break;
    }
  }
}

void SCO_execute_command_release(uint8_t Sw, Cmd_struct *cmd, bool first_cmd) {
  uint8_t Dev = cmd->Device;
  if (Dev == CURRENT) Dev = Current_device;
  uint8_t Type = cmd->Type;
  uint8_t Data1 = cmd->Data1;
  uint8_t Data2 = cmd->Data2;

  DEBUGMSG("Release -> execute command " + String(Type) + " for device " + String(Dev));

  if (Dev == COMMON) { // Check for common parameters
    switch (Type) {
      case MIDI_NOTE:
        MIDI_send_note_off(Data1, cmd->Data2, cmd->Value1, SCO_MIDI_port(cmd->Value2));
        break;
      case MIDI_CC:
        if (first_cmd) SCO_CC_release(Data1, Data2, cmd->Value1, cmd->Value2, cmd->Value3, SCO_MIDI_port(cmd->Value4), Sw, first_cmd); // Passing min, max and step value for STEP, RANGE and UPDOWN style pedal
        break;
    }
  }
  if (Dev < NUMBER_OF_DEVICES) {
    switch (Type) {
      case PARAMETER:
        if (first_cmd) SCO_update_released_parameter_state(Sw, 0, Device[Dev]->number_of_values(SP[Sw].PP_number) - 1, 1); // Passing min, max and step value for STEP, RANGE and UPDOWN style pedal
        Device[Dev]->parameter_release(Sw, cmd, Data1);
        break;
      case PAR_BANK:
        if (first_cmd) SCO_update_released_parameter_state(Sw, 0, Device[Dev]->number_of_values(SP[Sw].PP_number) - 1, 1); // Passing min, max and step value for STEP, RANGE and UPDOWN style pedal
        Device[Dev]->parameter_release(Sw, cmd, SP[Sw].PP_number);
        break;
      case ASSIGN:
        if ((Data1 == SELECT) || (Data1 == BANKSELECT)) {
          if (first_cmd) SCO_update_released_parameter_state(Sw, 0, Device[Dev]->number_of_values(SP[Sw].PP_number) - 1, 1); // Passing min, max and step value for STEP, RANGE and UPDOWN style pedal
          Device[Dev]->assign_release(Sw);
        }
        break;
      case LOOPER:
        Device[Dev]->looper_release();
        break;

    }
  }
}

void SCO_execute_command_long_pressed(uint8_t Sw, Cmd_struct *cmd, bool first_cmd) {

  uint8_t Dev = cmd->Device;
  if (Dev == CURRENT) Dev = Current_device;
  uint8_t Type = cmd->Type;

  DEBUGMSG("Long press -> execute command " + String(Type) + " for device " + String(Dev));

  switch (Type) {
    case PAGE: // Go to user select page on long press of any page command
    case OPEN_PAGE_DEVICE:
    case SELECT_NEXT_DEVICE:
    case OPEN_NEXT_PAGE_OF_DEVICE:
      Current_page = Previous_page; // "Undo" for pressing PAGE or SELECT_NEXT_DEVICE
      set_current_device(Previous_device);
      SCO_select_page(PAGE_SELECT);
      //prev_page_shown = Previous_page; // Page to be displayed on PAGE_SELECT LEDS
      break;
    case PATCH: // Go to direct select on long press of bank up/down or patch up/down
      if ((cmd->Data1 == SELECT) || (cmd->Data1 == BANKSELECT)) break;
      if (Dev < NUMBER_OF_DEVICES) Device[Dev]->direct_select_start();
      SCO_select_page(PAGE_CURRENT_DIRECT_SELECT); // Jump to the direct select page
      break;
    case TAP_TEMPO:
      SCO_global_tuner_toggle(); //Start global tuner
      break;
    case LOOPER: // Holding a looper button will open the full looper page
#ifndef IS_VCMINI
      if (Current_device == KPA) SCO_select_page(PAGE_KPA_LOOPER);
      else SCO_select_page(PAGE_FULL_LOOPER);
#endif
      break;
  }
}

void SCO_execute_command_held(uint8_t Sw, Cmd_struct *cmd, bool first_cmd) {
  uint8_t Dev = cmd->Device;
  if (Dev == CURRENT) Dev = Current_device;
  uint8_t Type = cmd->Type;
  uint8_t Data1 = cmd->Data1;
  uint8_t Data2 = cmd->Data2;

  DEBUGMSG("Switch held -> execute command " + String(Type) + " for device " + String(Dev));

  if (Dev < NUMBER_OF_DEVICES) {
    switch (Type) {
      case PARAMETER:
        if ((Data2 == STEP) || (Data2 == UPDOWN)) {
          if (first_cmd) SCO_update_held_parameter_state(Sw, 0, Device[Dev]->number_of_values(SP[Sw].PP_number) - 1, 1); // Passing min, max and step value for STEP, RANGE and UPDOWN style pedal
          Device[Dev]->parameter_press(Sw, cmd, Data1);
          SC_skip_release_and_hold_until_next_press(SKIP_LONG_PRESS);
          if (Data2 == UPDOWN) updown_direction_can_change = false;
        }
        break;
      case PAR_BANK:
        if ((SP[Sw].Latch == STEP) || (SP[Sw].Latch == UPDOWN)) {
          if (first_cmd) SCO_update_held_parameter_state(Sw, 0, Device[Dev]->number_of_values(SP[Sw].PP_number) - 1, 1); // Passing min, max and step value for STEP, RANGE and UPDOWN style pedal
          Device[Dev]->parameter_press(Sw, cmd, SP[Sw].PP_number);
          SC_skip_release_and_hold_until_next_press(SKIP_LONG_PRESS);
          if (SP[Sw].Latch == UPDOWN) updown_direction_can_change = false;
        }
        break;
    }
  }
  if (Dev == COMMON) {
    switch (Type) {
      case MENU:
        menu_press_hold(Sw);
        break;
      case MIDI_CC:
        if (first_cmd) SCO_CC_held(Data1, Data2, cmd->Value1, cmd->Value2, cmd->Value3, SCO_MIDI_port(cmd->Value4), Sw, first_cmd); // Passing min, max and step value for STEP, RANGE and UPDOWN style pedal
        SC_skip_release_and_hold_until_next_press(SKIP_LONG_PRESS);
        break;
    }
  }
}

void mute_all_but_me(uint8_t my_device) {
  for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
    if (d != my_device) Device[d]->mute();
  }
}

// ********************************* Section 3: Parameter State Control ********************************************
uint16_t SCO_find_max_value(uint8_t Max) {
  if (Max < 128) return Max;
  switch (Max + 1) {
    case TIME_2000: return 2000;
    case TIME_1000: return 1000;
    case TIME_500: return 500;
    case TIME_300: return 300;
    case RPT_600: return 600;
  }
  return 0;
}

bool SCO_update_parameter_state(uint8_t Sw, uint8_t Min, uint8_t Max, uint8_t Step) {
  // Update the current paramater state. Return true if value was updated.
  uint16_t _max = SCO_find_max_value(Max);
  uint16_t _min = Min;
  if ((Max + 1) == RPT_600) _min = 40;
  master_expr_from_cc = false;

  if (SC_switch_is_expr_pedal()) return SCO_update_parameter_state_exp_pedal(Sw, _min, _max, Step);
  if (SC_switch_is_encoder()) return SCO_update_parameter_state_encoder(Sw, _min, _max, Step);
  return SCO_update_parameter_state_switch(Sw, _min, _max, Step);
}

bool SCO_update_parameter_state_switch(uint8_t Sw, uint16_t _min, uint16_t _max, uint8_t Step) {
  uint16_t val;
  switch (SP[Sw].Latch) {
    case MOMENTARY:
      SP[Sw].State = 1; // Switch state on
      switch_controlled_by_master_exp_pedal = 0;
      break;
    case TOGGLE:  // Toggle state
      SP[Sw].State ++;
      if (SP[Sw].State > 2) SP[Sw].State = 1;
      switch_controlled_by_master_exp_pedal = 0;
      break;
    case TRISTATE:  // Select next state
      SP[Sw].State ++;
      if (SP[Sw].State > 3) SP[Sw].State = 1;
      switch_controlled_by_master_exp_pedal = 0;
      break;
    case FOURSTATE:  // Select next state
      SP[Sw].State ++;
      if (SP[Sw].State > 4) SP[Sw].State = 1;
      switch_controlled_by_master_exp_pedal = 0;
      break;
    case STEP:
      // Update byte1 with the new value
      if (_max >= 128) val = (SP[Sw].Target_byte1 * 128) + SP[Sw].Target_byte2;
      else val = SP[Sw].Target_byte1;
      if (val >= _max) val = _min;
      else val += Step;
      if (_max > 128) {
        SP[Sw].Target_byte1 = val / 128;
        SP[Sw].Target_byte2 = val % 128;
      }
      else SP[Sw].Target_byte1 = val;
      if (Setting.MEP_control == 2) {
        if (switch_controlled_by_master_exp_pedal != Sw) update_page = REFRESH_PAGE;
        switch_controlled_by_master_exp_pedal = Sw;
      }
      else {
        switch_controlled_by_master_exp_pedal = 0;
      }
      break;
    case UPDOWN:
      updown_direction_can_change = true;
      if (Setting.MEP_control >= 1) {
        if (switch_controlled_by_master_exp_pedal != Sw) update_page = REFRESH_PAGE;
        switch_controlled_by_master_exp_pedal = Sw;
      }
      else {
        switch_controlled_by_master_exp_pedal = 0;
      }
      return false;
      break;
  }
  return true;
}

bool SCO_update_parameter_state_encoder(uint8_t Sw, uint16_t _min, uint16_t _max, uint8_t Step) {
  uint16_t val;
  switch_controlled_by_master_exp_pedal = 0;
  switch (SP[Sw].Latch) {
    case TOGGLE:  // Toggle state
      if (Enc_value > 0) SP[Sw].State = 1;
      if (Enc_value < 0) SP[Sw].State = 2;
      break;
    case TRISTATE:  // Select next state
      if ((Enc_value > 0) && (SP[Sw].State < 2)) SP[Sw].State ++;
      if ((Enc_value < 0) && (SP[Sw].State > 0)) SP[Sw].State --;
      break;
    case FOURSTATE:  // Select next state
      if ((Enc_value > 0) && (SP[Sw].State < 3)) SP[Sw].State ++;
      if ((Enc_value < 0) && (SP[Sw].State > 0)) SP[Sw].State --;

      break;
    case STEP:
    case UPDOWN:
      // Update byte1 with the new value
      if (_max >= 128) val = (SP[Sw].Target_byte1 * 128) + SP[Sw].Target_byte2;
      else val = SP[Sw].Target_byte1;
      val = update_encoder_value(Enc_value, val, _min, _max);
      /*if (Enc_value > 0) {
        for (uint8_t i = 0; i < abs(Enc_value); i++) {
          if (val >= _max) {
            if ((Enc_value > 1) || (_max < 4)) val = _max;
            else val = _min;
          }
          else val += Step;
        }
        }
        if (Enc_value < 0)
        for (uint8_t i = 0; i < abs(Enc_value); i++) {
          if (val <= _min) {
            if ((Enc_value < -1) || (_max < 4)) val = _min;
            else val = _max;
          }
          else val -= Step;
        }*/

      if (_max > 128) {
        SP[Sw].Target_byte1 = val / 128;
        SP[Sw].Target_byte2 = val % 128;
      }
      else SP[Sw].Target_byte1 = val;
      break;
  }
  return true;
}

bool SCO_update_parameter_state_exp_pedal(uint8_t Sw, uint16_t _min, uint16_t _max, uint8_t Step) {
  bool isnew = false;
  SP[Sw].State = 1;
  if (SP[Sw].Latch == RANGE) {
    if (_max >= 128) { // Checking for dual byte parameters
      uint16_t new_value = map(Expr_ped_value, 0, 127, _min, _max);
      if (new_value != (SP[Sw].Target_byte1 * 128) + SP[Sw].Target_byte2) {
        SP[Sw].Target_byte1 = new_value / 128;
        SP[Sw].Target_byte2 = new_value % 128;
        isnew = true;
      }
    }
    else { // Single byte parameter
      uint8_t new_value = map(Expr_ped_value, 0, 127, _min, _max);
      if (new_value != SP[Sw].Target_byte1) {
        SP[Sw].Target_byte1 = new_value;
        isnew = true;
      }
    }
    LCD_show_bar(0, Expr_ped_value, 0); // Show it on the main display
    if (switch_controlled_by_master_exp_pedal > 0) LCD_show_bar(switch_controlled_by_master_exp_pedal, Expr_ped_value, 0); // Show the bar on the individual display when using the MEP
    return isnew;
  }
  return true;
}

void SCO_update_released_parameter_state(uint8_t Sw, uint8_t Min, uint8_t Max, uint8_t Step) {
  if ((SP[Sw].Latch == UPDOWN) && (updown_direction_can_change)) {
    SP[Sw].Direction ^= 1; // Toggle direction
    update_lcd = Sw;
  }
  if (SP[Sw].Latch == MOMENTARY) SP[Sw].State = 2;
}

void SCO_update_held_parameter_state(uint8_t Sw, uint8_t Min, uint8_t Max, uint8_t Step) {
  uint16_t val;
  uint16_t _max = SCO_find_max_value(Max);
  uint16_t _min = Min;
  if ((Max + 1) == RPT_600) {
    _min = 40;
  }
  if (SP[Sw].Latch == STEP) {
    if (Max >= 128) val = (SP[Sw].Target_byte1 * 128) + SP[Sw].Target_byte2;
    else val = SP[Sw].Target_byte1;
    val += Step;
    if (val > _max) val = _min;
    if (Max > 128) {
      SP[Sw].Target_byte1 = val / 128;
      SP[Sw].Target_byte2 = val % 128;
    }
    else SP[Sw].Target_byte1 = val;
  }
  if (SP[Sw].Latch == UPDOWN) {
    if (Max >= 128) val = (SP[Sw].Target_byte1 * 128) + SP[Sw].Target_byte2;
    else val = SP[Sw].Target_byte1;

    if (SP[Sw].Direction) { // Up
      if (val < _max) val++;
    }
    else { // Down
      if (val > _min) val--;
    }
    if (Max > 128) {
      SP[Sw].Target_byte1 = val / 128;
      SP[Sw].Target_byte2 = val % 128;
    }
    else SP[Sw].Target_byte1 = val;
  }
}

uint16_t SCO_return_parameter_value(uint8_t Sw, Cmd_struct * cmd) {
  if ((SP[Sw].Latch == RANGE) || (SP[Sw].Latch == STEP) || (SP[Sw].Latch == UPDOWN)) {
    return SP[Sw].Target_byte1;
  }
  // Latch is MOMENTARY or TOGGLE
  if (SP[Sw].Type == PARAMETER) { //Parameters are read directly from the switch config.
    if (SP[Sw].State == 1) return cmd->Value1;
    if (SP[Sw].State == 2) return cmd->Value2;
    if (SP[Sw].State == 3) return cmd->Value3;
    if (SP[Sw].State == 4) return cmd->Value4;
    //if (SP[Sw].State == 5) return cmd->Value5;
  }
  // Type is PAR_BANK or ASSIGN, so we return 1 or 0.
  if (SP[Sw].State == 1) return 1;
  return 0;
}

uint8_t SCO_find_parameter_state(uint8_t Sw, uint8_t value) {
  //Cmd_struct cmd;
  //EEPROM_read_cmd(Current_page, Sw, 0, &cmd);
  switch (SP[Sw].Type) {
    case PARAMETER:
      if (value == SP[Sw].Value1) return 1;
      if (value == SP[Sw].Value2) return 2;
      if (value == SP[Sw].Value3) return 3;
      if (value == SP[Sw].Value4) return 4;
      return 0;

    case PAR_BANK:
      if (value == 0) return 2;
      if (value == 1) return 1;
      return 0;

    case ASSIGN:
      if (value == SP[Sw].Assign_min) return 2;
      if (value == SP[Sw].Assign_max) return 1;
      return 0;
  }
  return 0;
}

// ********************************* Section 4: MIDI CC Commands ********************************************

uint8_t SCO_MIDI_port(uint8_t port) { // Converts the port number from the menu to the proper port number
  if (port < NUMBER_OF_MIDI_PORTS) return port << 4;
  else return ALL_MIDI_PORTS;
}

void SCO_CC_press(uint8_t CC_number, uint8_t CC_toggle, uint8_t value1, uint8_t value2, uint8_t channel, uint8_t port, uint8_t Sw, bool first_cmd) {

  uint8_t val;
  String msg;

  if (SC_switch_is_expr_pedal()) {
    if (SP[Sw].Latch == CC_RANGE) {
      val = map(Expr_ped_value, 0, 127, value2, value1);
      if (val != SP[Sw].Target_byte1) { // Check if we have a new value
        MIDI_send_CC(CC_number, val, channel, port); // Controller, Value, Channel, Port;
        MIDI_remember_CC(CC_number, val, channel, port);
        SP[Sw].Target_byte1 = val;
      }
      LCD_show_bar(0, Expr_ped_value, 0); // Show it on the main display
      if (switch_controlled_by_master_exp_pedal > 0) {
        LCD_show_bar(switch_controlled_by_master_exp_pedal, Expr_ped_value, 0); // Show it on the individual display
        update_lcd = switch_controlled_by_master_exp_pedal;
      }

      msg = "CC #";
      LCD_add_3digit_number(CC_number, msg);
      msg += ':';
      LCD_add_3digit_number(val, msg);
      LCD_show_popup_label(msg, ACTION_TIMER_LENGTH);
    }
  }
  else {
    master_expr_from_cc = true;
    switch (CC_toggle) {
      case CC_ONE_SHOT:
      case CC_MOMENTARY:
        MIDI_send_CC(CC_number, value1, channel, port); // Controller, Value, Channel, Port
        MIDI_remember_CC(CC_number, value1, channel, port);
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case CC_TOGGLE:
      case CC_TOGGLE_ON:
        if (first_cmd) {
          SP[Sw].State++;
          if (SP[Sw].State >= 2) SP[Sw].State = 0;
        }
        if (SP[Sw].State == 0) {
          MIDI_send_CC(CC_number, value1, channel, port); // Controller, Value, Channel, Port
          MIDI_remember_CC(CC_number, value1, channel, port);
        }
        else {
          MIDI_send_CC(CC_number, value2, channel, port); // Controller, Value, Channel, Port
          MIDI_remember_CC(CC_number, value2, channel, port);
        }
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case CC_STEP:
        // Update byte1 with the new value
        val = SP[Sw].Target_byte1;
        val += 1;
        if (val > value1) val = value2; // Go from max to min value.
        SP[Sw].Target_byte1 = val;
        MIDI_send_CC(CC_number, val, channel, port); // Controller, Value, Channel, Port
        MIDI_remember_CC(CC_number, val, channel, port);
        switch_controlled_by_master_exp_pedal = Sw;
        break;
      case CC_UPDOWN:
        updown_direction_can_change = true;
        switch_controlled_by_master_exp_pedal = Sw;
        break;
    }
  }
}

void SCO_CC_release(uint8_t CC_number, uint8_t CC_toggle, uint8_t value1, uint8_t value2, uint8_t channel, uint8_t port, uint8_t Sw, bool first_cmd) {
  if (CC_toggle == CC_MOMENTARY) {
    MIDI_send_CC(CC_number, value2, channel, port); // Controller, Value, Channel, Port
    MIDI_remember_CC(CC_number, value2, channel, port);
  }
  if ((CC_toggle == CC_UPDOWN) && (updown_direction_can_change)) {
    SP[Sw].Direction ^= 1; // Toggle direction
    //DEBUGMSG("Toggle direction: " + String(SP[Sw].Direction));
    update_lcd = Sw;
  }
}

void SCO_CC_held(uint8_t CC_number, uint8_t CC_toggle, uint8_t Max, uint8_t Min, uint8_t channel, uint8_t port, uint8_t Sw, bool first_cmd) {
  if (CC_toggle == CC_UPDOWN) {
    if (SP[Sw].Direction) {
      if (SP[Sw].Target_byte1 < Max) SP[Sw].Target_byte1++;
    }
    else {
      if (SP[Sw].Target_byte1 > Min) SP[Sw].Target_byte1--;
    }
    MIDI_send_CC(CC_number, SP[Sw].Target_byte1, channel, port); // Controller, Value, Channel, Port
    MIDI_remember_CC(CC_number, SP[Sw].Target_byte1, channel, port);
    update_lcd = Sw;
    updown_direction_can_change = false;
  }
}

// ********************************* Section 5: Page Selection Commands ********************************************

void SCO_select_page(uint8_t new_page) {
  if (SCO_valid_page(new_page)) {
    Previous_page = Current_page; // Store the page we come from...
    Current_page = new_page;
    //prev_page_shown = 255; // Clear page number to be displayed on LEDS
    if ((Current_page != PAGE_MENU) && (Current_page != PAGE_CURRENT_DIRECT_SELECT)) {
      EEPROM_update_when_quiet();
    }
    if (Current_page == PAGE_MENU) menu_open();
    if ((Current_page == PAGE_CURRENT_DIRECT_SELECT) && (Current_device < NUMBER_OF_DEVICES)) {
      Device[Current_device]->direct_select_start();
    }
    else device_in_bank_selection = 0;
    my_looper_lcd = 0;
    update_page = RELOAD_PAGE;
    update_main_lcd = true;
    SC_skip_release_and_hold_until_next_press(SKIP_RELEASE | SKIP_HOLD); // So no release or hold commands will be triggered on the new page by the switch that is still pressed now
    if (Current_page != PAGE_MENU) LCD_show_page_name(); // Temporary show page name on main display
    DEBUGMAIN("*** SCO_select_page: " + String(new_page));
  }
}

bool SCO_valid_page(uint8_t page) {
  if (page < Number_of_pages) return true;
  if ((page >= FIRST_FIXED_CMD_PAGE) && (page <= LAST_FIXED_CMD_PAGE)) return true;
  return false;
}

void SCO_select_page(uint8_t new_page, uint8_t device) {
  if (device < NUMBER_OF_DEVICES) {
    Previous_device = Current_device;
    set_current_device(device);
  }
  SCO_select_page(new_page);
}

void SCO_toggle_page(uint8_t page1, uint8_t page2) {
  Previous_page = Current_page; // Store the mode we come from...
  if (Current_page == page1) SCO_select_page(page2);
  else SCO_select_page(page1);
}

void SCO_select_next_device() { // Will select the next device that is connected
  if (Current_device >= NUMBER_OF_DEVICES) return;

  device_in_bank_selection = 0;

  // Go to the page of the current device if we are on some other page (often coming from menu)
  if (Current_page != Device[Current_device]->read_current_device_page()) {
    SCO_select_page(Device[Current_device]->read_current_device_page());
    return;
  }

  // Check the devices to find the next connected one
  uint8_t current_selected_device = Current_device;
  bool device_connected = false;
  uint8_t tries = NUMBER_OF_DEVICES; // Limited the number of tries for the loop, in case no device is
  while (tries > 0) {
    tries--;
    current_selected_device++;
    if (current_selected_device >= NUMBER_OF_DEVICES) current_selected_device = 0;
    if (Device[current_selected_device]->connected) { // device is selected
      device_connected = true;
      tries = 0; //And we are done
    }
  }
  if (device_connected) {
    Previous_device = Current_device;
    set_current_device(current_selected_device);
    SCO_select_page(Device[current_selected_device]->read_current_device_page()); // Load the current page associated to this device
    if (Setting.Main_display_show_top_right != 0) {
      String Name = Device[current_selected_device]->device_name;
      LCD_show_popup_label("Selecting " + Name, MESSAGE_TIMER_LENGTH);
    }
  }
  else {
    SCO_select_page(DEFAULT_PAGE);
    LCD_show_popup_label("No devices...", MESSAGE_TIMER_LENGTH);
  }
}

uint8_t SCO_get_number_of_next_device() {
  // Return the number of the current device if we are on some other page.
  if (Current_page != Device[Current_device]->read_current_device_page()) {
    return Current_device;
  }

  uint8_t current_selected_device = Current_device;
  uint8_t tries = NUMBER_OF_DEVICES; // Limited the number of tries for the loop, in case no device is
  while (tries > 0) {
    tries--;
    current_selected_device++;
    if (current_selected_device >= NUMBER_OF_DEVICES) current_selected_device = 0;
    if (Device[current_selected_device]->connected) { // device is selected
      tries = 0; //And we are done
    }
  }
  return current_selected_device;
}

void SCO_select_next_page_of_device(uint8_t Dev) { // Will select the patch page of the current device. These can be set in programmed on the unit. Defaults are in init() of the device class
  uint8_t new_page;
  if (Dev < NUMBER_OF_DEVICES) {
    set_current_device(Dev);

    // Move to next device page
    new_page = Device[Dev]->select_next_device_page();
    //Device[Dev]->read_current_device_page();

    // Select this page
    SCO_trigger_default_page_cmds(new_page);
    SCO_select_page(new_page);

    device_in_bank_selection = 0;
  }
}

void SCO_page_up_down(signed int delta) {
  uint8_t New_page = update_encoder_value(delta, Current_page, LOWEST_USER_PAGE, Number_of_pages - 1);
  SCO_trigger_default_page_cmds(New_page);
  SCO_select_page(New_page);
}

void SC_page_bank_updown(signed int delta, uint8_t bank_size) {

  uint16_t page_max = Number_of_pages + LAST_FIXED_CMD_PAGE - FIRST_SELECTABLE_FIXED_CMD_PAGE;

  if (device_in_bank_selection != PAGE_BANK_SELECTION_IN_PROGRESS) {
    device_in_bank_selection = PAGE_BANK_SELECTION_IN_PROGRESS; // Use of static variable device_in_bank_selection will make sure only one device is in bank select mode.
    page_bank_select_number = page_bank_number; //Reset the bank to current patch
  }
  // Perform bank up:
  /*if (delta > 0) {
    for (uint8_t i = 0; i < delta; i++) {
      if (page_bank_select_number >= (page_max / bank_size)) page_bank_select_number = 0; // Check if we've reached the top
      else page_bank_select_number ++;
    }
    }
    if (delta < 0) {
    for (uint8_t i = 0; i < abs(delta); i++) {
      if (page_bank_select_number <= 0) page_bank_select_number = (page_max / bank_size); // Check if we've reached the bottom
      else page_bank_select_number --;
    }
    }*/
  page_bank_select_number = update_encoder_value(delta, page_bank_select_number, 0, page_max / bank_size);

  if (page_bank_select_number == page_bank_number) device_in_bank_selection = 0; //Check whether were back to the original bank
}

// ********************************* Section 6: Global Tuner Commands ********************************************

void SCO_global_tuner_toggle() {
  if (!global_tuner_active) SCO_global_tuner_start();
  else SCO_global_tuner_stop();
}

void SCO_global_tuner_start() {
  // Start tuner or mute all devices
  DEBUGMSG("*** Activating Global tuner");
  global_tuner_active = true;
  for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
    Device[d]->start_tuner();
  }
  MIDI_send_tuner_mode();
  update_main_lcd = true;
}

void SCO_global_tuner_stop() {
  // Stop tuner or unmute all devices
  global_tuner_active = false;
  for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
    Device[d]->stop_tuner();
  }
  MIDI_send_tuner_mode();
  update_main_lcd = true;
}

// ********************************* Section 7: Global Tap Tempo Commands ********************************************

// Call global_tap_tempo()
// We only support bpms from 40 to 250:
#define MIN_BPM 40
#define MAX_BPM 250
#define MAX_BPM_TIME 60000000 / MIN_BPM
#define NUMBER_OF_TAPS 4 // When tapping a new tempo, this is the number of taps that are sent
uint8_t tap = 0;

#define NUMBER_OF_TAPMEMS 5
uint32_t tap_time[NUMBER_OF_TAPMEMS];
uint8_t tap_time_index = 0;
uint32_t new_time, time_diff, avg_time;
uint32_t prev_time = 0;
bool tap_array_full = false;
bool send_new_bpm_value = false;
bool tap_tempo_pressed_once = false;
#define SWITCH_TEMPO_FOLLOW_MODE_TIME 2000000 // Time before switching tap tempo follow mode

#define NUMBER_OF_MIDI_CLOCK_MEMS 10
uint8_t do_not_tap_port = 255;
uint32_t ignore_midi_clock_timer = 0;
#define IGNORE_MIDI_CLOCK_TIMER_LENGTH 500000 // Time midi clock messages are ignored after tapping tap tempo
uint8_t MIDI_CLOCK_BPM_MEMS[NUMBER_OF_MIDI_CLOCK_MEMS] = { 0 };
uint8_t MIDI_clock_bpm_mem_index = 0;
uint32_t previous_midi_clock_time = 0;
bool MIDI_clock_received = false;
bool update_tempo = false;
IntervalTimer MIDI_clock_timer;
uint8_t bpm_LED_tick = 0;

#define NUMBER_OF_BPM_FOLLOW_MEMS 6
uint32_t tap_follow_mems[NUMBER_OF_BPM_FOLLOW_MEMS];
uint8_t tap_follow_bpm_index = 0;
bool tap_follow_buffer_full = false;

void SCO_MIDI_clock_start() {
  long timer_interval = 60000000 / (24 * Setting.Bpm);
  MIDI_clock_timer.begin(SCO_MIDI_clock_timer_expired, timer_interval);
}

void SCO_MIDI_clock_update() {
  long timer_interval = 60000000 / (24 * Setting.Bpm);
  MIDI_clock_timer.update(timer_interval);
}

void SCO_check_update_tempo() {
  if (update_tempo) {
    SCO_update_tap_tempo_LED();
    update_tempo = false;
  }
  if (tap_tempo_pressed_once) {
    if ((Setting.Follow_tempo_from_G2M > 0) && (micros() > tap_time[0] + SWITCH_TEMPO_FOLLOW_MODE_TIME)) {
      if (Setting.Follow_tempo_from_G2M == 1) {
        Setting.Follow_tempo_from_G2M = 2;
        LCD_show_popup_label("Tempo follow on", ACTION_TIMER_LENGTH);
      }
      else {
        Setting.Follow_tempo_from_G2M = 1;
        LCD_show_popup_label("Tempo follow off", ACTION_TIMER_LENGTH);
      }
      tap_tempo_pressed_once = false;
    }
  }
}

void SCO_MIDI_clock_timer_expired() {
  __disable_irq();
  MIDI_send_clock();
  update_tempo = true;
  bpm_LED_tick++;
  if (bpm_LED_tick >= 24) { // 24 ticks per beat
    bpm_LED_tick = 0;
  }
  __enable_irq();
}

void SCO_global_tap_external() { // For external tapping sources
  time_switch_pressed = micros();
  SCO_global_tap_tempo_press(0);
  update_page = REFRESH_PAGE;
}

void SCO_global_tap_tempo_press(uint8_t sw) {

#ifdef INTA_PIN // When using display boards, use the time from the inta interrupt, otherwise bpm timing is not correct
  new_time = time_switch_pressed;
#else
  new_time = micros(); //Store the current time
#endif

  SCO_tap_on_device(); // Send out tap tempo to devices where tempo can not be set directly

  time_diff = new_time - prev_time;
  prev_time = new_time;
  //DEBUGMSG("*** Tap no:" + String(tap_time_index) + " with difference " + String(time_diff));

  // If time difference between two taps is too long, we will start new tapping sequence
  if (time_diff > MAX_BPM_TIME) {
    tap_time_index = 1;
    tap_array_full = false;
    tap_time[0] = new_time;
    tap_tempo_pressed_once = true;
    //DEBUGMSG("!!! STARTED NEW TAP SEQUENCE");
  }
  else {
    //Calculate the average time depending on if the tap_time array is full or not
    if (tap_array_full) {
      avg_time = (new_time - tap_time[tap_time_index]) / (NUMBER_OF_TAPMEMS);
    }
    else {
      avg_time = (new_time - tap_time[0]) / tap_time_index;
    }

    // Store new time in memory
    tap_time[tap_time_index] = new_time;
    
    // Calculate the bpm
    uint16_t new_bpm = ((60000000 + (avg_time >> 1)) / avg_time); // Calculate the bpm
    SCO_update_bpm(new_bpm);

    // Move to the next memory slot
    tap_time_index++;
    if (tap_time_index >= NUMBER_OF_TAPMEMS) { // We have reached the last tap memory
      tap_time_index = 0;
      tap_array_full = true; // So we need to calculate the average tap time in a different way
    }
    tap_tempo_pressed_once = false;
  }
  if (Setting.Main_display_show_top_right != 1) LCD_show_popup_label("Tempo " + String(Setting.Bpm) + " bpm", ACTION_TIMER_LENGTH); // Show the tempo on the main display
  else update_main_lcd = true;
  if (sw > 0) update_lcd = sw; // Update the LCD of the display above the tap tempo button
  else update_page = REFRESH_PAGE; // Running tap tempo from CURNUM - update entire page
  do_not_tap_port = 255;
  SCO_delay_receiving_MIDI_clock();
  SCO_MIDI_clock_update();
  SCO_reset_tap_tempo_LED(); // Reset the LED state, so it will flash in time with the new tempo
  reset_tempo_following();
}

bool SCO_update_bpm(uint8_t new_bpm) {
  if (new_bpm > MAX_BPM) new_bpm = MAX_BPM;
  if (Setting.Bpm == new_bpm) return false;
  Setting.Bpm = new_bpm;

  // Send it to the devices
  for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
    Device[d]->set_bpm();
  }
  if (VC_device_connected) MIDI_send_current_bpm();
  EEPROM_update_when_quiet();
  return true;
}

void SCO_set_global_tempo_press(uint8_t new_bpm) {
  if (new_bpm < MIN_BPM) return;
  if (new_bpm > MAX_BPM) return;
  if (new_bpm == Setting.Bpm) return;
  Setting.Bpm = new_bpm;
  // Send it to the devices
  send_new_bpm_value = true; // Will delay sending the data, but also hangs the VC-mini when using the encoder for tempo change
  /*for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
    Device[d]->set_bpm();
    } */
  tap = 0; // So the tempo will be retapped (done from SCO_update_tap_tempo_LED)
  do_not_tap_port = 255;
  SCO_delay_receiving_MIDI_clock();
  SCO_MIDI_clock_update();
  reset_tempo_following();
  update_page = REFRESH_PAGE; //Refresh the page, so any present tap tempo button display will also be updated.
}

void SCO_set_global_tempo_with_encoder(signed int delta) {
  if (delta > 15) delta = 15;
  uint8_t new_bpm = update_encoder_value(delta, Setting.Bpm, MIN_BPM, MAX_BPM);
  SCO_set_global_tempo_press(new_bpm);
  if (Setting.Main_display_show_top_right != 1) LCD_show_popup_label("Set tempo: " + String(Setting.Bpm), ACTION_TIMER_LENGTH);
}

void SCO_receive_MIDI_clock_pulse(uint8_t port) {

  // 24 pulses are sent, so bpm = 2500000 / deltatime
  uint32_t current_time = micros();

  if (!MIDI_clock_received) {
    DEBUGMSG("Clock received from port " + String(Current_MIDI_in_port >> 4));
  }

  if (millis() < ignore_midi_clock_timer) return;
    

  if (previous_midi_clock_time > 0) {
    if (current_time - previous_midi_clock_time < 9999) return; // time too short
    uint16_t bpm_course = 25000000 / (current_time - previous_midi_clock_time);
    uint8_t bpm_new = (bpm_course + 5) / 10; // Calculate a correct average
    MIDI_CLOCK_BPM_MEMS[MIDI_clock_bpm_mem_index] = bpm_new;
    MIDI_clock_bpm_mem_index++;
    if (MIDI_clock_bpm_mem_index >= NUMBER_OF_MIDI_CLOCK_MEMS) {
      MIDI_clock_bpm_mem_index = 0;
      uint16_t total = 0;
      
      uint8_t bpm = MIDI_CLOCK_BPM_MEMS[0];
      total = bpm;
      uint8_t dev = bpm / 5;
      for (uint8_t i = 1; i < NUMBER_OF_MIDI_CLOCK_MEMS; i++) {
        if ((MIDI_CLOCK_BPM_MEMS[i] < bpm - dev) || (MIDI_CLOCK_BPM_MEMS[i] > bpm + dev)) {
          DEBUGMSG("Too much deviation in MIDI clock");
          return; // Quit if tempo deviates too much
        }
        bpm = MIDI_CLOCK_BPM_MEMS[i];
        total += bpm;
      }
      uint8_t avg_bpm = total / NUMBER_OF_MIDI_CLOCK_MEMS;
      if ((avg_bpm < Setting.Bpm - 1) || (avg_bpm > Setting.Bpm + 1)) { // Check if tempo has changed
        Setting.Bpm = avg_bpm;
        SCO_MIDI_clock_update();
        MIDI_clock_received = true;
        for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
          if (Device[d]->MIDI_out_port != port) Device[d]->set_bpm();
        }
        if (VC_device_connected) MIDI_send_current_bpm();
        tap = 0;
        do_not_tap_port = port;
        update_page = REFRESH_PAGE; //Refresh the page, so any present tap tempo button display will also be updated.
      }
    }
  }
  previous_midi_clock_time = current_time;
}

void SCO_delay_receiving_MIDI_clock() {
  ignore_midi_clock_timer = IGNORE_MIDI_CLOCK_TIMER_LENGTH + millis();
  MIDI_clock_bpm_mem_index = 0;
}

void SCO_update_tap_tempo_LED() {

  // If this is the first tick
  if ((bpm_LED_tick < 6) && (global_tap_tempo_LED == 0)) {
    // Check if we lost the bpm clock
    if (micros() - previous_midi_clock_time > MAX_BPM_TIME) MIDI_clock_received = false;

    if (!Setting.Hide_tap_tempo_LED) {// Turn the LED on
      if (MIDI_clock_received) global_tap_tempo_LED = Setting.LED_bpm_synced_colour;
      else if (Setting.Follow_tempo_from_G2M < 2) global_tap_tempo_LED = Setting.LED_bpm_colour;
      else global_tap_tempo_LED = Setting.LED_bpm_follow_colour;
      update_LEDS = true;
    }

    if (send_new_bpm_value) { // Send updated tempo to the devices
      send_new_bpm_value = false;
      for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
        Device[d]->set_bpm();
      }
      if (VC_device_connected) MIDI_send_current_bpm();
      EEPROM_update_when_quiet();
    }

    if (tap < NUMBER_OF_TAPS) { // Send automatic cc tap message
      SCO_tap_on_device();
      tap++;
    }
    else do_not_tap_port = 255;
  }

  if ((bpm_LED_tick >= 6) && (global_tap_tempo_LED != 0) && (!Setting.Hide_tap_tempo_LED)) { // The sixth tick is at a quarter of 24 ticks
    global_tap_tempo_LED = 0;  // Turn the LED off
    update_LEDS = true;
  }
}

void SCO_reset_tap_tempo_LED() {
  bpm_LED_tick = 23;
}

void SCO_retap_tempo() { // Retap the tempo on all external devices (that support this method)
  tap = 0;
}

void SCO_tap_on_device() {
  for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) { // Tap this tempo on the device
    if (Device[d]->MIDI_out_port != do_not_tap_port) Device[d]->bpm_tap();
  }
}

// Automatic tempo following from Guitar2MIDI
uint32_t prev_note_on_time = 0;
uint32_t start_measurement_time = 0;
uint8_t number_of_intervals = 0;
#define MSEC_TIME_BETWEEN_CHORD_PLAYING 100000

void SCO_tempo_following_receive_note_on(uint8_t note, uint8_t velocity, uint8_t channel, uint8_t port) {
  if (Setting.Follow_tempo_from_G2M != 2) return;
  if (velocity == 0) return; // filter out note off
  if ((channel >= Setting.Bass_mode_G2M_channel) && (channel <= Setting.Bass_mode_G2M_channel + 6)) {
    // First check if time delta is long enough to make sure we are not strumming
    uint32_t delta = MIDI_note_on_time - prev_note_on_time;
    prev_note_on_time = MIDI_note_on_time;
    if (delta < MSEC_TIME_BETWEEN_CHORD_PLAYING) return;

    // Now calculate the real delta
    delta = MIDI_note_on_time - start_measurement_time;
    uint32_t average_delta_from_tempo = 60000000 / Setting.Bpm;
    if (delta > (average_delta_from_tempo * 17 / 20)) {
      start_measurement_time = MIDI_note_on_time;
      number_of_intervals = 0;
    }
    number_of_intervals++;
    /*if (delta < (average_delta_from_tempo * 11 / 10)) {
      tap_follow_bpm_index = 0; // Restart getting tempo
      tap_follow_buffer_full = false;
      }*/

    DEBUGMSG("Delta: " + String(delta) + ", avg delta: " + String(average_delta_from_tempo));
    if (number_of_intervals > 2) return;
    if ((delta > (average_delta_from_tempo * 17 / 20)) && (delta < (average_delta_from_tempo * 23 / 20))) {
      tap_follow_mems[tap_follow_bpm_index] = delta;
      tap_follow_bpm_index++;
      if (tap_follow_bpm_index >= NUMBER_OF_BPM_FOLLOW_MEMS) {
        tap_follow_bpm_index = 0;
        tap_follow_buffer_full = true;
      }

      if (tap_follow_buffer_full) {
        uint32_t total = 0;
        for (uint8_t i = 0; i < NUMBER_OF_BPM_FOLLOW_MEMS; i++) total += tap_follow_mems[i];

        uint32_t avg_time = total / NUMBER_OF_BPM_FOLLOW_MEMS;
        //if ((avg_time < (average_delta_from_tempo * 9 / 10)) || (avg_time > (average_delta_from_tempo * 11 / 10))) { // If outside 1% deviation
        uint8_t new_tempo = 60000000 / avg_time;
        if (SCO_update_bpm(new_tempo)) {
          tap = 0;
          do_not_tap_port = port;
          update_page = REFRESH_PAGE;
        }
      }
    }
  }
}

void reset_tempo_following() {
  tap_follow_bpm_index = 0;
  tap_follow_buffer_full = false;
}

// ********************************* Section 8: Bass Mode (Low/High String Priority) ********************************************

// Bass mode: sends a CC message with the number of the lowest string that is being played.
// By making smart assigns on a device, you can hear just the bass note played

uint8_t bass_string = 0; //remembers the lowest string played
uint8_t top_string = 5; // remebers the highest string played

// Method 1:
/*
  void SCO_bass_mode_note_on(uint8_t note, uint8_t velocity, uint8_t channel, uint8_t port) {
  if ((channel >= Setting.Bass_mode_G2M_channel) && (channel <= Setting.Bass_mode_G2M_channel + 6)) {
    uint8_t string_played = channel - Setting.Bass_mode_G2M_channel + 1;
    if ((string_played == bass_string) && (velocity == 0)) bass_string = 0; // VG-99 sends NOTE ON with velocity 0 instead of NOTE OFF

    if ((string_played > bass_string) && (velocity >= Setting.Bass_mode_min_velocity)) {
      bass_string = string_played; //Set the bass play channel to the current channel
      if (Setting.Bass_mode_device < NUMBER_OF_DEVICES)
        MIDI_send_CC(Setting.Bass_mode_cc_number , bass_string, Device[Setting.Bass_mode_device]->MIDI_channel, Device[Setting.Bass_mode_device]->MIDI_port);
    }
  }
  }

  void SCO_bass_mode_note_off(uint8_t note, uint8_t velocity, uint8_t channel, uint8_t port) {
  if ((channel >= Setting.Bass_mode_G2M_channel) && (channel <= Setting.Bass_mode_G2M_channel + 6)) {
    uint8_t string_played = channel - Setting.Bass_mode_G2M_channel + 1;
    if (string_played == bass_string) {
      bass_string = 0; //Reset the bass play channel
      //if (Setting.Bass_mode_device < NUMBER_OF_DEVICES)
      //  MIDI_send_CC(Setting.Bass_mode_cc_number , bass_string, Device[Setting.Bass_mode_device]->MIDI_channel, Device[Setting.Bass_mode_device]->MIDI_port);
    }
  }
  }
*/
// Method 2:
bool string_on[6] = { false }; // remember the current state of every string

void SCO_bass_mode_note_on(uint8_t note, uint8_t velocity, uint8_t channel, uint8_t port) {
  if ((channel >= Setting.Bass_mode_G2M_channel) && (channel <= Setting.Bass_mode_G2M_channel + 5)) {
    uint8_t string_played = channel - Setting.Bass_mode_G2M_channel;

    if (velocity >= Setting.Bass_mode_min_velocity) {
      string_on[string_played] = true;
      SCO_bass_mode_check_string();
      SCO_bass_mode_check_high_string();
    }

    else { // string level below minimum threshold or string off on VG99
      string_on[string_played] = false;
      SCO_bass_mode_check_string();
      //SCO_bass_mode_check_high_string();
    }
  }
}

void SCO_bass_mode_note_off(uint8_t note, uint8_t velocity, uint8_t channel, uint8_t port) {
  if ((channel >= Setting.Bass_mode_G2M_channel) && (channel <= Setting.Bass_mode_G2M_channel + 5)) {
    uint8_t string_played = channel - Setting.Bass_mode_G2M_channel;
    string_on[string_played] = false;
    SCO_bass_mode_check_string();
    //SCO_bass_mode_check_high_string();
  }
}

void SCO_bass_mode_check_string() {
  uint8_t lowest_string_played = 0;
  for (uint8_t s = 0; s < 6; s++) { // Find the lowest string that is played (has highest string number)
    if (string_on[s]) lowest_string_played = s + 1;
  }
  if (lowest_string_played != bass_string) {
    bass_string = lowest_string_played;
    if (Setting.Bass_mode_device < NUMBER_OF_DEVICES)
      MIDI_send_CC(Setting.Bass_mode_cc_number , bass_string, Device[Setting.Bass_mode_device]->MIDI_channel, Device[Setting.Bass_mode_device]->MIDI_out_port);
    DEBUGMAIN("Set lowest string: " + String(bass_string));
  }
}

void SCO_bass_mode_check_high_string() {
  uint8_t highest_string_played = 7;
  for (uint8_t s = 5; s -- > 0;) { // Find the lowest string that is played (has highest string number)
    if (string_on[s]) highest_string_played = s + 1;
  }
  if (highest_string_played != top_string) {
    top_string = highest_string_played;
    if (Setting.Bass_mode_device < NUMBER_OF_DEVICES)
      MIDI_send_CC(Setting.HNP_mode_cc_number, top_string, Device[Setting.Bass_mode_device]->MIDI_channel, Device[Setting.Bass_mode_device]->MIDI_out_port);
    DEBUGMAIN("Set highest string: " + String(top_string));
  }
}

// ********************************* Section 9: VController Power On/Off Switching ********************************************

void SCO_switch_power_on() {
  // Switch power on
#ifdef POWER_PIN
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, HIGH);
#endif
}

void SCO_switch_power_off() {
  DEBUGMAIN("Switching off VController...");
  EEP_write_eeprom_common_data(); // Save current settings
  LCD_clear_all_displays();
  LED_turn_all_off();
  LCD_show_popup_label("Bye bye...", MESSAGE_TIMER_LENGTH);

#ifdef POWER_PIN
  // Lower the power pin
  delay(600);
  digitalWrite(POWER_PIN, LOW);
  // VController of VC-touch will have switched off, but VC-touch on USB power will not switch off, so we simulate this.
#ifdef MAIN_TFT_DISPLAY
  TFT_switch_off_main_window(); // Clear main TFT screen
#else
  delay(1000);
#endif

#else
  // Simulate power down as there is no power pin
  // Here we start a temporary loop to emulate being switched off in case there is no poer switching
  LCD_show_popup_label("", MESSAGE_TIMER_LENGTH); // Clear Bye bye
  LCD_backlight_off();
#endif

  while (switch_pressed == 0) { // Wait for switch being pressed
    main_switch_check();
  }

  //LCD_backlight_on();
  reboot(); // Do a proper reboot!

}

bool SCO_are_you_sure() {

  switch_pressed = 0;

  // Wait for new switch to be pressed
  while (switch_pressed == 0) { // Wait for switch being pressed
    main_switch_check();
    main_MIDI_common(); // So we can press remote as well
#ifdef IS_VCTOUCH
    touch.loop();
#endif

    // Update the SP.Pressed variable
    if (switch_pressed > 0) {
      SP[switch_pressed].Pressed = true;
    }
    if (switch_released > 0) {
      SP[switch_released].Pressed = false;
      switch_released = 0;
    }

    LED_update_pressed_state_only(); // Get the LEDs to respond
  }
  bool pressed_yes = (switch_pressed == YES_SWITCH);
  switch_pressed = 0;
  multi_switch_booleans = 0;

  if (pressed_yes) {
    DEBUGMSG("Pressed YES");
  }
  else {
    DEBUGMSG("Pressed NO");
  }

  return pressed_yes;
}

// ********************************* Section 10: Master expression pedal control ********************************************

void SCO_move_master_exp_pedal(uint8_t Sw, uint8_t Dev) {

  if (SCO_change_menu_or_parameter_switch()) return;

  // Go to the current device and operate the correct function there
  if (Dev < NUMBER_OF_DEVICES) {
    Device[Dev]->move_expression_pedal(Sw, Expr_ped_value, SP[Sw].Exp_pedal);
  }
}

void SCO_move_touch_screen_parameter(uint8_t sw, uint8_t val) { // Dragging a touch screen switch takes you here.
  switch_controlled_by_master_exp_pedal = sw;
  Expr_ped_value = val;
  switch_type = SW_TYPE_EXPRESSION_PEDAL;
  SCO_change_menu_or_parameter_switch();
}



bool SCO_change_menu_or_parameter_switch() { // Also used from touch menu
  if (Current_page == PAGE_MENU) { // if menu active, change the selected field
    menu_move_expr_pedal(Expr_ped_value);
    return true;
  }

  if (switch_controlled_by_master_exp_pedal > 0) { // If updown or step switch is pressed last, update this switch with the expression pedal
    uint8_t prev_latch_type = SP[switch_controlled_by_master_exp_pedal].Latch; // We temporary change the Latch type of this switch and run the main switch control command

    if (master_expr_from_cc == false) SP[switch_controlled_by_master_exp_pedal].Latch = RANGE;
    else SP[switch_controlled_by_master_exp_pedal].Latch = CC_RANGE;
    prev_switch_pressed = current_cmd_switch & SWITCH_MASK; // To trigger a re-read from EEPROM of the commands
    switch_pressed = switch_controlled_by_master_exp_pedal;
    DEBUGMSG("MEP triggers switch " + String(switch_pressed));
    main_switch_control();

    SP[switch_controlled_by_master_exp_pedal].Latch = prev_latch_type;
    prev_switch_pressed = switch_controlled_by_master_exp_pedal; // To trigger a re-read again
    update_page = REFRESH_FX_ONLY;
    return true;
  }

  return false;
}
