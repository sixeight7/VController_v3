// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: Switch Action Trigger and Command Lookup
// Section 2: Command Execution
// Section 3: Parameter State Control
// Section 4: MIDI CC Commands
// Section 5: Page Selection Commands
// Section 6: Global Tuner Commands
// Section 7: Global Tap Tempo Commands
// Section 8: Bass Mode (Low String Priority)
// Section 9: VController Power On/Off Switching
// Section 10: Master expression pedal control

// ********************************* Section 1: Switch Action Trigger and Command Lookup ********************************************

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
  //SCO_reset_all_switch_states();
  //PAGE_load_current(true);
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
      SC_skip_release_and_hold_until_next_press();
    }
    else { // When switch is pressed, set current_cmd_switch_action to SWITCH_PRESSED or SWITCH_PRESSED_REPEAT and let current_cmd point to the first command for this switch
      if (switch_pressed & ON_DUAL_PRESS) current_cmd_switch_action = SWITCH_DUAL_PRESSED;
      else current_cmd_switch_action = SWITCH_PRESSED;
      current_cmd = EEPROM_first_cmd(Current_page, switch_pressed & SWITCH_MASK); // Is always fast, because first_cmd is read from the index which is in RAM
      DEBUGMSG("Current cmd:" + String(current_cmd) + " on page:" + String(Current_page));
      current_cmd_switch = switch_pressed;
      DEBUGMSG("Current_cmd_switch: " + String(current_cmd_switch));
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

  if (current_cmd > 0) { // If current_cmd points to a command we can execute, do it. Then check if there is another command to execute.
    SCO_execute_cmd(current_cmd_switch, current_cmd_switch_action, current_cmdbuf_index);
    current_cmd = EEPROM_next_cmd(current_cmd); //Find the next command - will be executed on the next cycle
    current_cmdbuf_index++; // Point to the next command in the command buffer
    if (current_cmdbuf_index >= CMD_BUFFER_SIZE) current_cmd = 0; // Stop executing commands when the end of the buffer is reached.
  }
  else {
    prev_switch_pressed = current_cmd_switch & SWITCH_MASK; // Will stop the reading of switch commands from EEPROM
  }

  if ((current_cmd == 0) && (arm_page_cmd_exec > 0)) { // Execute any commands on page selection
    // We do this when current_cmd is 0, because the arm_page_exec variable is set from another command that is running.
    current_cmd_switch_action = SWITCH_PRESSED;
    prev_switch_pressed = 0;
    current_cmd = EEPROM_first_cmd(arm_page_cmd_exec, 0);
    DEBUGMSG("Trigger execution of default command for page " + String(arm_page_cmd_exec));
    current_cmd_switch = 0;
    current_cmdbuf_index = 0;
    arm_page_cmd_exec = 0;
  }

  SCO_update_tap_tempo_LED();
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
  DEBUGMSG("Checking Cmd index: " +  String(index) + ", switch type: " + String(switch_type) + ", switch: " + String(sw) + ", action: " + String(action));

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
      if (switch_type == 0) {
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
        SCO_global_tap_tempo_press(Sw);
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case SET_TEMPO:
        if (SC_switch_is_expr_pedal()) break;
        if (SC_switch_is_encoder()) SCO_set_global_tempo_with_encoder(Enc_value);
        else SCO_set_global_tempo_press(Data1);
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case PAGE:
        if (SC_switch_is_expr_pedal()) break;
        if (Data1 == SELECT) {
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
        if (SC_switch_is_encoder()) menu_encoder_turn(Sw, Enc_value);
        else menu_press(Sw, true);
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case SELECT_NEXT_DEVICE:
        if (SC_switch_is_expr_pedal()) break;
        SCO_select_next_device();
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case GLOBAL_TUNER:
        if (SC_switch_is_expr_pedal()) break;
        SCO_global_tuner_toggle();
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case MIDI_CC:
        SCO_CC_press(Data1, Data2, cmd->Value1, cmd->Value2, cmd->Value3, SCO_MIDI_port(cmd->Value4), Sw, first_cmd);
        if (!SC_switch_is_expr_pedal()) {
          update_page = REFRESH_PAGE;
        }
        break;
      case MIDI_PC:
        if (SC_switch_is_expr_pedal()) break;
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
          DEBUGMSG("***********HERE!!!!!!");
          update_main_lcd = true;
          update_page = REFRESH_PAGE;
          break;
        }
        if (SC_switch_is_encoder()) {
          if (Enc_value == 0) break;
          if (Enc_value > 0) pnumber = Device[Dev]->next_patch_number();
          if (Enc_value < 0) pnumber = Device[Dev]->prev_patch_number();
        }
        else {
          if (Data1 == SELECT) pnumber = Data1 + (Data2 * 100);
          else if (Data1 == PREV) pnumber = Device[Dev]->prev_patch_number();
          else if (Data1 == NEXT) pnumber = Device[Dev]->next_patch_number();
          else pnumber = SP[Sw].PP_number;
        }
        Device[Dev]->patch_select_pressed(pnumber);
        Device[Dev]->current_patch_name = SP[Sw].Label; // Store current patch name
        mute_all_but_me(Dev); // mute all the other devices
        if (SP[Sw].Label[0] != ' ') LCD_show_popup_label(SP[Sw].Label, ACTION_TIMER_LENGTH);
        break;
      case DIRECT_SELECT:
        if (SC_switch_is_expr_pedal()) break;
        Device[Dev]->direct_select_press(Data1);
        update_page = RELOAD_PAGE;
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case PARAMETER:
        if (Data2 == UPDOWN) break; // updown parameters are executed in SCO_command_press()
        if (first_cmd) updated = SCO_update_parameter_state(Sw, cmd->Value1, cmd->Value2, cmd->Value3); // Passing min, max and step value for STEP, RANGE and UPDOWN style pedal
        if (updated) Device[Dev]->parameter_press(Sw, cmd, Data1);
        break;
      case PAR_BANK:
        if (Data2 == UPDOWN) break; // updown parameters are executed in SCO_command_press()
        if (first_cmd) updated = SCO_update_parameter_state(Sw, 0, Device[Dev]->number_of_values(SP[Sw].PP_number) - 1, 1); // Passing min, max and step value for STEP, RANGE and UPDOWN style pedal
        if (updated) Device[Dev]->parameter_press(Sw, cmd, SP[Sw].PP_number);
        break;
      case PAR_BANK_CATEGORY:
        if (SC_switch_is_expr_pedal()) break;
        Device[Dev]->select_parameter_bank_category(SP[Sw].PP_number);
        SCO_select_page(PAGE_CURRENT_PARAMETER, Dev);
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case PAR_BANK_UP:
        if (SC_switch_is_expr_pedal()) break;
        if (SC_switch_is_encoder()) Device[Dev]->par_bank_updown(Enc_value, Data1);
        else Device[Dev]->par_bank_updown(1, Data1);
        update_main_lcd = true;
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case PAR_BANK_DOWN:
        if (SC_switch_is_expr_pedal()) break;
        if (SC_switch_is_encoder()) Device[Dev]->par_bank_updown(Enc_value, Data1);
        else Device[Dev]->par_bank_updown(-1, Data1);
        update_main_lcd = true;
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case ASSIGN:
        if ((Data1 == SELECT) || (Data1 == BANKSELECT)) {
          if (first_cmd) updated = SCO_update_parameter_state(Sw, 0, 1, 1);
          if (updated) {
            if (SC_switch_is_expr_pedal()) Device[Dev]->assign_press(Sw, Expr_ped_value);
            else Device[Dev]->assign_press(Sw, 127);
          }
        }
        if (Data1 == BANKUP) {
          if (SC_switch_is_expr_pedal()) break;
          if (SC_switch_is_encoder()) Device[Dev]->asgn_bank_updown(Enc_value, Data1);
          else Device[Dev]->asgn_bank_updown(1, Data1);
          update_main_lcd = true;
          switch_controlled_by_master_exp_pedal = 0;
        }
        if (Data1 == BANKDOWN) {
          if (SC_switch_is_expr_pedal()) break;
          if (SC_switch_is_encoder()) Device[Dev]->asgn_bank_updown(Enc_value, Data1);
          else Device[Dev]->asgn_bank_updown(-1, Data1);
          update_main_lcd = true;
          switch_controlled_by_master_exp_pedal = 0;
        }
        break;
      case MUTE:
        if (SC_switch_is_expr_pedal()) break;
        Device[Dev]->mute();
        update_LEDS = true;
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case OPEN_PAGE_DEVICE:
        if (SC_switch_is_expr_pedal()) break;
        SCO_trigger_default_page_cmds(Data1);
        SCO_select_page(Data1, Dev);
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case OPEN_NEXT_PAGE_OF_DEVICE:
        if (SC_switch_is_expr_pedal()) break;
        SCO_select_next_page_of_device(Dev);
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case TOGGLE_EXP_PEDAL:
        if (SC_switch_is_expr_pedal()) break;
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
        if (SC_switch_is_expr_pedal()) SCO_move_master_exp_pedal(Sw, Dev);
        break;
      case SNAPSCENE:
        if (SC_switch_is_expr_pedal()) break;
        if (first_cmd) Device[Dev]->set_snapscene(SP[Sw].PP_number);
        else Device[Dev]->set_snapscene(Data1);  // When not the first command, the first snapshot value set in the command is sent.
        update_page = REFRESH_PAGE;
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case LOOPER:
        if (SC_switch_is_expr_pedal()) break;
        Device[Dev]->looper_press(Data1, true);
        update_page = REFRESH_PAGE;
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case SAVE_PATCH:
        if (SC_switch_is_expr_pedal()) break;
        if (Dev == KTN) {
          My_KTN.save_patch();
        }
        switch_controlled_by_master_exp_pedal = 0;
        break;
    }
  }
}

void SCO_execute_command_press(uint8_t Sw, Cmd_struct *cmd, bool first_cmd) {
  uint8_t Dev = cmd->Device;
  if (Dev == CURRENT) Dev = Current_device;
  uint8_t Type = cmd->Type;
  uint8_t Data1 = cmd->Data1;
  uint8_t Data2 = cmd->Data2;
  bool updated = false;

  DEBUGMSG("Press -> execute command " + String(Type) + " for device " + String(Dev));

  if (Dev == COMMON) { // Check for common parameters
    switch (Type) {
      case MIDI_NOTE:
        MIDI_send_note_on(Data1, cmd->Data2, cmd->Value1, SCO_MIDI_port(cmd->Value2));
        break;
      case MIDI_CC:
        if (first_cmd) SCO_CC_press(Data1, Data2, cmd->Value1, cmd->Value2, cmd->Value3, SCO_MIDI_port(cmd->Value4), Sw, first_cmd); // Passing min, max and step value for STEP, RANGE and UPDOWN style pedal
        break;
    }
  }
  if (Dev < NUMBER_OF_DEVICES) {
    switch (Type) {
      case PARAMETER:
        if (Data2 != UPDOWN) break; // Only update updown parameters here
        if (first_cmd) updated = SCO_update_parameter_state(Sw, cmd->Value1, cmd->Value2, cmd->Value3); // Passing min, max and step value for STEP, RANGE and UPDOWN style pedal
        if (updated) Device[Dev]->parameter_press(Sw, cmd, Data1);
        break;
      case PAR_BANK:
        if (SP[Sw].Latch != UPDOWN) break; // Only update updown parameters here
        if (first_cmd) updated = SCO_update_parameter_state(Sw, 0, Device[Dev]->number_of_values(SP[Sw].PP_number) - 1, 1); // Passing min, max and step value for STEP, RANGE and UPDOWN style pedal
        if (updated) Device[Dev]->parameter_press(Sw, cmd, SP[Sw].PP_number);
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
      prev_page_shown = Previous_page; // Page to be displayed on PAGE_SELECT LEDS
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
          if (Data2 == UPDOWN) updown_direction_can_change = false;
        }
        break;
      case PAR_BANK:
        if ((SP[Sw].Latch == STEP) || (SP[Sw].Latch == UPDOWN)) {
          if (first_cmd) SCO_update_held_parameter_state(Sw, 0, Device[Dev]->number_of_values(SP[Sw].PP_number) - 1, 1); // Passing min, max and step value for STEP, RANGE and UPDOWN style pedal
          Device[Dev]->parameter_press(Sw, cmd, SP[Sw].PP_number);
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

bool SCO_update_parameter_state(uint8_t Sw, uint8_t Min, uint8_t Max, uint8_t Step) {
  // Update the current paramater state. Return true if value was updated.
  uint8_t val;
  if (SC_switch_is_expr_pedal()) {
    bool isnew = false;
    SP[Sw].State = 1;
    if (SP[Sw].Latch == RANGE) {
      if (Max >= 128) { // Checking for dual byte parameters
        uint16_t _max = Max;
        uint16_t _min = Min;
        if (Max + 1 == TIME_2000) _max = 2000;
        if (Max + 1 == TIME_1000) _max = 1000;
        if (Max + 1 == TIME_500) _max = 500;
        if (Max + 1 == TIME_300) _max = 300;
        if (Max + 1 == RPT_600) {
          _min = 40;
          _max = 600;
        }
        uint16_t new_value = map(Expr_ped_value, 0, 127, _min, _max);
        if (new_value != (SP[Sw].Target_byte1 * 128) + SP[Sw].Target_byte2) {
          SP[Sw].Target_byte1 = new_value / 128;
          SP[Sw].Target_byte2 = new_value % 128;
          isnew = true;
        }
      }
      else { // Single byte parameter
        uint8_t new_value = map(Expr_ped_value, 0, 127, Min, Max);
        if (new_value != SP[Sw].Target_byte1) {
          SP[Sw].Target_byte1 = new_value;
          isnew = true;
        }
      }
      LCD_show_bar(0, Expr_ped_value); // Show it on the main display
      if (switch_controlled_by_master_exp_pedal > 0) LCD_show_bar(switch_controlled_by_master_exp_pedal, Expr_ped_value); // Show the bar on the individual display when using the MEP
      return isnew;
    }
  }
  else { // Not an expresssion pedal
    master_expr_from_cc = false;
    signed int delta;
    if (SC_switch_is_encoder()) delta = Enc_value;
    else delta = 1;

    switch (SP[Sw].Latch) {
      case MOMENTARY:
        SP[Sw].State = 1; // Switch state on
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case TOGGLE:  // Toggle state
        SP[Sw].State += delta;;
        if (SP[Sw].State > 2) SP[Sw].State = 1;
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case TRISTATE:  // Select next state
        SP[Sw].State += delta;
        if (SP[Sw].State > 3) SP[Sw].State = 1;
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case FOURSTATE:  // Select next state
        SP[Sw].State += delta;
        if (SP[Sw].State > 4) SP[Sw].State = 1;
        switch_controlled_by_master_exp_pedal = 0;
        break;
      case STEP:
        // Update byte1 with the new value
        val = SP[Sw].Target_byte1;
        if (delta > 0) {
          for (uint8_t i = 0; i < abs(delta); i++) {
            val += Step;
            if (val > Max) val = Min;
          }
        }
        else {
          for (uint8_t i = 0; i < abs(delta); i++) {
            if (val == Min) val = Max - 1;
            else val -= Step;
          }
        }
        SP[Sw].Target_byte1 = val;
        if (Setting.MEP_control == 2) {
          if (switch_controlled_by_master_exp_pedal != Sw) update_page = REFRESH_PAGE;
          switch_controlled_by_master_exp_pedal = Sw;
        }
        else {
          switch_controlled_by_master_exp_pedal = 0;
        }
        break;
      case UPDOWN:
        if (SC_switch_is_encoder()) {
          val = SP[Sw].Target_byte1;
          if (delta > 0) {
            for (uint8_t i = 0; i < abs(delta); i++) {
              val ++;
              if (val > Max) val = Min;
            }
          }
          else {
            for (uint8_t i = 0; i < abs(delta); i++) {
              if (val == Min) val = Max - 1;
              else val --;
            }
          }
          SP[Sw].Target_byte1 = val;
          break;
        }
        else {
          updown_direction_can_change = true;
          if (Setting.MEP_control >= 1) {
            if (switch_controlled_by_master_exp_pedal != Sw) update_page = REFRESH_PAGE;
            switch_controlled_by_master_exp_pedal = Sw;
          }
          else {
            switch_controlled_by_master_exp_pedal = 0;
          }
          return false;
        }
        break;
    }
    DEBUGMSG("New state is " + String(SP[Sw].State));
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
  if (SP[Sw].Latch == STEP) {
    // Update byte1 with the new value
    val = SP[Sw].Target_byte1;
    val += Step;
    if (val > Max) val = Min;
    SP[Sw].Target_byte1 = val;
  }
  if (SP[Sw].Latch == UPDOWN) {
    if (Max >= 128) { // Need double target byte for large numbers
      val = (SP[Sw].Target_byte1 << 7) + SP[Sw].Target_byte2;
      uint16_t _max = 0;
      uint16_t _min = 0;
      if (Max + 1 == TIME_2000) _max = 2000;
      if (Max + 1 == TIME_1000) _max = 1000;
      if (Max + 1 == TIME_500) _max = 500;
      if (Max + 1 == TIME_300) _max = 300;
      if (Max + 1 == RPT_600) {
        _max = 600;
        _min = 40;
      }
      if (SP[Sw].Direction) { // Up
        if (val < _max) val++;
      }
      else { // Down
        if (val > _min) val--;
      }
      SP[Sw].Target_byte1 = val >> 7;
      SP[Sw].Target_byte2 = val & 0x7F;
    }
    else {
      // Update byte1 with the new value
      val = SP[Sw].Target_byte1;
      if (SP[Sw].Direction) { // Up
        if (val < Max) val++;
      }
      else { // Down
        if (val > Min) val--;
      }
      SP[Sw].Target_byte1 = val;
    }
  }
}

uint8_t SCO_return_parameter_value(uint8_t Sw, Cmd_struct * cmd) {
  if (SP[Sw].Latch == RANGE) return SP[Sw].Target_byte1;
  if (SP[Sw].Latch == STEP) return SP[Sw].Target_byte1;
  if (SP[Sw].Latch == UPDOWN) return SP[Sw].Target_byte1;
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
  else return ALL_PORTS;
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
      LCD_show_bar(0, Expr_ped_value); // Show it on the main display
      if (switch_controlled_by_master_exp_pedal > 0) {
        LCD_show_bar(switch_controlled_by_master_exp_pedal, Expr_ped_value); // Show it on the individual display
        update_lcd = switch_controlled_by_master_exp_pedal;
      }

      msg = "CC #";
      LCD_add_3digit_number(CC_number, msg);
      msg += ":";
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
        if (val > value1) val = value2;
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
    prev_page_shown = 255; // Clear page number to be displayed on LEDS
    if ((Current_page != PAGE_MENU) && (Current_page != PAGE_CURRENT_DIRECT_SELECT)) {
      EEPROM.write(EEPROM_CURRENT_PAGE_ADDR, Current_page);
      EEPROM.write(EEPROM_CURRENT_DEVICE_ADDR, Current_device);
    }
    if (Current_page == PAGE_MENU) menu_open();
    if ((Current_page == PAGE_CURRENT_DIRECT_SELECT) && (Current_device < NUMBER_OF_DEVICES)) {
      Device[Current_device]->direct_select_start();
    }
    my_looper_lcd = 0;
    update_page = RELOAD_PAGE;
    update_main_lcd = true;
    device_in_bank_selection = 0;
    SC_skip_release_and_hold_until_next_press(); // So no release, long press or hold commands will be triggered on the new page by the switch that is still pressed now
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
  }
  else {
    SCO_select_page(DEFAULT_PAGE);
    LCD_show_popup_label("No devices...", MESSAGE_TIMER_LENGTH);
  }
}

uint8_t SCO_get_number_of_next_device() {
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
  uint8_t New_page = 0;
  if (delta > 0) {
    for (uint8_t i = 0; i < delta; i++) {
      if (Current_page < (Number_of_pages - 1)) New_page = Current_page + 1;
      else New_page = LOWEST_USER_PAGE;
    }
  }
  if (delta < 0) {
    for (uint8_t i = 0; i < abs(delta); i++) {
      if (Current_page > LOWEST_USER_PAGE) New_page = Current_page - 1;
      else New_page = Number_of_pages - 1;
    }
  }
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
  if (delta > 0) {
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
  }

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
  update_main_lcd = true;
}

void SCO_global_tuner_stop() {
  // Stop tuner or unmute all devices
  global_tuner_active = false;
  for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
    Device[d]->stop_tuner();
  }
  update_main_lcd = true;
}

// ********************************* Section 7: Global Tap Tempo Commands ********************************************

// Call global_tap_tempo()
// We only support bpms from 40 to 250:
#define MIN_TIME 240000 // (60.000.000 / 250 bpm)
#define MAX_TIME 1500000 // (60.000.000 / 40 bpm)
#define NUMBER_OF_TAPS 4 // When tapping a new tempo, this is the number of taps that are sent
uint8_t tap = 0;

#define NUMBER_OF_TAPMEMS 5 // Tap tempo will do the average of five taps
uint32_t tap_time[NUMBER_OF_TAPMEMS];
uint8_t tap_time_index = 0;
uint32_t new_time, time_diff, avg_time;
uint32_t prev_time = 0;
bool tap_array_full = false;
bool send_new_bpm_value = false;

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

  // If time difference between two taps is too short or too long, we will start new tapping sequence
  if ((time_diff < MIN_TIME) || (time_diff > MAX_TIME)) {
    tap_time_index = 1;
    tap_array_full = false;
    tap_time[0] = new_time;
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
    //DEBUGMSG("Wrote tap_time[" + String(tap_time_index) + "] with time " + String(new_time));

    // Calculate the bpm
    Setting.Bpm = ((60000000 + (avg_time >> 1)) / avg_time); // Calculate the bpm
    //EEPROM.write(EEPROM_bpm, bpm);  // Store it in EEPROM
    //DEBUGMSG("avg time:" + String(avg_time) + " => bpm: " + String(bpm));

    // Send it to the devices
    for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
      Device[d]->set_bpm();
    }

    // Move to the next memory slot
    tap_time_index++;
    if (tap_time_index >= NUMBER_OF_TAPMEMS) { // We have reached the last tap memory
      tap_time_index = 0;
      tap_array_full = true; // So we need to calculate the average tap time in a different way
    }
  }
  LCD_show_popup_label("Tempo " + String(Setting.Bpm) + " bpm", ACTION_TIMER_LENGTH); // Show the tempo on the main display
  update_lcd = sw; // Update the LCD of the display above the tap tempo button
  SCO_reset_tap_tempo_LED(); // Reset the LED state, so it will flash in time with the new tempo
}

void SCO_set_global_tempo_press(uint8_t new_bpm) {
  Setting.Bpm = new_bpm;
  // Send it to the devices
  for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
    Device[d]->set_bpm();
  }
  tap = 0; // So the tempo will be retapped (done from SCO_update_tap_tempo_LED)
  update_page = REFRESH_PAGE; //Refresh the page, so any present tap tempo button display will also be updated.
}

void SCO_set_global_tempo_with_encoder(signed int delta) {
  if (delta > 0) {
    Setting.Bpm += delta;
    if (Setting.Bpm > 240) Setting.Bpm = 40;
  }
  if (delta <= 0) {
    Setting.Bpm -= abs(delta);
    if (Setting.Bpm < 40) Setting.Bpm = 240;
  }
  send_new_bpm_value = true;
  for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
    Device[d]->set_bpm();
  }
  tap = 0; // So the tempo will be retapped (done from SCO_update_tap_tempo_LED)
  update_page = REFRESH_PAGE; //Refresh the page, so any present tap tempo button display will also be updated.
}

#define BPM_LED_ON_TIME 100 // The time the bpm LED is on in msec. 50 for real LED, 100 for virtual LED
#define BPM_LED_ADJUST 1   // LED is running a little to slow. This is an adjustment of a few msecs
uint32_t bpm_LED_timer = 0;
uint32_t bpm_LED_timer_length = BPM_LED_ON_TIME;

void SCO_update_tap_tempo_LED() {

  // Check if timer needs to be set
  if (bpm_LED_timer == 0) {
    bpm_LED_timer = millis();
  }

  // Check if timer runs out
  if (millis() - bpm_LED_timer > bpm_LED_timer_length) {
    bpm_LED_timer = millis(); // Reset the timer

    // If LED is currently on
    if (global_tap_tempo_LED == Setting.LED_bpm_colour) {
      global_tap_tempo_LED = 0;  // Turn the LED off
      bpm_LED_timer_length = (60000 / Setting.Bpm) - BPM_LED_ON_TIME - BPM_LED_ADJUST; // Set the time for the timer
    }
    else {
      global_tap_tempo_LED = Setting.LED_bpm_colour;   // Turn the LED on
      bpm_LED_timer_length = BPM_LED_ON_TIME; // Set the time for the timer

      if (send_new_bpm_value) { // Send updated tempo to the devices
        send_new_bpm_value = false;
        for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
          Device[d]->set_bpm();
        }
      }

      if (tap < NUMBER_OF_TAPS) { // Send automatic cc tap message
        SCO_tap_on_device();
        tap++;
      }
    }
    update_LEDS = true;
  }
}

void SCO_reset_tap_tempo_LED() {
  bpm_LED_timer = millis();
  global_tap_tempo_LED = Setting.LED_bpm_colour;    // Turn the LED on
  //VG99_TAP_TEMPO_LED_ON();
  bpm_LED_timer_length = BPM_LED_ON_TIME;  // Set the time for the timer
  update_LEDS = true;
}

void SCO_retap_tempo() { // Retap the tempo on all external devices (that support this method)
  tap = 0;
}

void SCO_tap_on_device() {
  for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) { // Tap this tempo on the device
    Device[d]->bpm_tap();
  }
}

// ********************************* Section 8: Bass Mode (Low String Priority) ********************************************

// Bass mode: sends a CC message with the number of the lowest string that is being played.
// By making smart assigns on a device, you can hear just the bass note played
uint8_t bass_string = 0; //remembers the current midi channel


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
  if ((channel >= Setting.Bass_mode_G2M_channel) && (channel <= Setting.Bass_mode_G2M_channel + 6)) {
    uint8_t string_played = channel - Setting.Bass_mode_G2M_channel;

    if (velocity >= Setting.Bass_mode_min_velocity) {
      string_on[string_played] = true;
      SCO_bass_mode_check_string();
    }

    else { // string level below minimum threshold or string off on VG99
      string_on[string_played] = false;
      SCO_bass_mode_check_string();
    }
  }
}

void SCO_bass_mode_note_off(uint8_t note, uint8_t velocity, uint8_t channel, uint8_t port) {
  if ((channel >= Setting.Bass_mode_G2M_channel) && (channel <= Setting.Bass_mode_G2M_channel + 6)) {
    uint8_t string_played = channel - Setting.Bass_mode_G2M_channel;
    string_on[string_played] = false;
    SCO_bass_mode_check_string();
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
      MIDI_send_CC(Setting.Bass_mode_cc_number , bass_string, Device[Setting.Bass_mode_device]->MIDI_channel, Device[Setting.Bass_mode_device]->MIDI_port);
    DEBUGMAIN("Set lowest string: " + String(bass_string));
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
  digitalWrite(POWER_PIN, LOW);
  delay(10000); // Wait forever

#else
  // Simulate power down as there is no power pin
  // Here we start a temporary loop to emulate being switched off in case there is no poer switching
  LCD_show_popup_label("", MESSAGE_TIMER_LENGTH); // Clear Bye bye
  LCD_backlight_off();

  while (switch_pressed == 0) { // Wait for switch being pressed
    main_switch_check();
  }

  //LCD_backlight_on();
  reboot(); // Do a proper reboot!
#endif
}

#ifdef IS_VCMINI
#define YES_SWITCH 7
#else
#define YES_SWITCH 10
#endif

bool SCO_are_you_sure() {

  switch_pressed = 0;

  // Wait for new switch to be pressed
  while (switch_pressed == 0) { // Wait for switch being pressed
    main_switch_check();
    main_MIDI_common(); // So we can press remote as well

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

  if (Current_page == PAGE_MENU) { // if menu active, change the selected field
    menu_move_expr_pedal(Expr_ped_value);
    return;
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
    return;
  }

  // Go to the current device and operate the correct function there
  if (Dev < NUMBER_OF_DEVICES) {
    Device[Dev]->move_expression_pedal(Sw, Expr_ped_value, SP[Sw].Exp_pedal);
  }
}
