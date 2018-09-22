// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: Switch Initialization
// Section 2: Internal Switch Reading
// Section 3: External Switch and Expression Pedal Reading
// Section 4: Switch Long Press / Extra Long Press and Hold Detection

// ********************************* Section 1: Switch Initialization ********************************************

// Setup and reading of internal and external switches.
// Check for switch pressed and output the result in the switch_pressed, switch_released, switch_long_pressed and switch_long_pressed variable.
// Internal switches can be connected in a keypad or to dedicated display boards controlled by an MCP23017.
// Keypad switches are triggered by interrupts.
// Display board switches are triggered through an interrupt from the MCP23017 (int a). Because reading of the MCP23017 over i2c is time consuming (around 12 ms),
// we keep track of the last read display board.
// External switches can be of type normally open, normally closed or an expression pedal.
// The type of external switch is read during startup of the VController.
// The choice of hardware must be set in hardware.h

// Create object for power switch
#ifdef POWER_SWITCH_PIN
#include <Bounce.h>
Bounce power_switch = Bounce(POWER_SWITCH_PIN, 50);
#endif

// Create objects for switchpad (max 1) if defined in hardware.h
// Functionality of the switchpad is described in switchpad.h
#ifdef ROWPINS
#include "switchpad_lib.h"
byte rowPins[ROWS] = {ROWPINS}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {COLUMNPINS}; //connect to the column pinouts of the keypad

Switchpad switchpad = Switchpad(rowPins, colPins, ROWS, COLS, 50);
#endif

// Create objects for external switches (max 8) if defined in hardware.h
#ifdef JACK1_PINS
#include "switchext_lib.h"
SwitchExt ctl_jack[NUMBER_OF_CTL_JACKS] = {
  SwitchExt (JACK1_PINS, 50, DETECT, JACK_PORT_PULLUP),
#endif
#ifdef JACK2_PINS
  SwitchExt (JACK2_PINS, 50, DETECT, JACK_PORT_PULLUP),
#endif
#ifdef JACK3_PINS
  SwitchExt (JACK3_PINS, 50, DETECT, JACK_PORT_PULLUP),
#endif
#ifdef JACK4_PINS
  SwitchExt (JACK4_PINS, 50, DETECT, JACK_PORT_PULLUP),
#endif
#ifdef JACK5_PINS
  SwitchExt (JACK5_PINS, 50, DETECT, JACK_PORT_PULLUP),
#endif
#ifdef JACK6_PINS
  SwitchExt (JACK6_PINS, 50, DETECT, JACK_PORT_PULLUP),
#endif
#ifdef JACK7_PINS
  SwitchExt (JACK7_PINS, 50, DETECT, JACK_PORT_PULLUP),
#endif
#ifdef JACK8_PINS
  SwitchExt (JACK8_PINS, 50, DETECT, JACK_PORT_PULLUP),
#endif
#ifdef JACK1_PINS
};
#endif

bool switch_is_expression_pedal = false;
uint8_t switch_pressed = 0; //Variable set when switch is pressed
uint8_t switch_released = 0; //Variable set when switch is released
uint8_t switch_long_pressed = 0; //Variable set when switch is pressed long (check LONG_PRESS_TIMER_LENGTH for when this will happen)
uint8_t switch_extra_long_pressed = 0; //Variable set when switch is pressed long (check LONG_PRESS_TIMER_LENGTH for when this will happen)
uint8_t last_switch_pressed;
uint8_t Expr_ped_value = 0;
uint8_t previous_switch_pressed = 0;
uint8_t previous_switch_released = 0;
uint8_t switch_held = 0;
uint8_t switch_held_times_triggered = 0;
uint32_t Hold_timer = 0;
uint32_t Hold_time;
bool inta_triggered = false;

#define DEBOUNCE_TIME 20 // Time between reading the display boards
uint32_t Debounce_timer = 0;
bool Debounce_timer_active = false;
bool Debounce_normal_state_timer_active = false;

#define LONG_PRESS_TIMER_LENGTH 1000 // Timer used for detecting long-pressing switch. Time is in milliseconds
#define EXTRA_LONG_PRESS_TIMER_LENGTH 3000
uint32_t Long_press_timer = 0;
uint32_t Extra_long_press_timer = 0;
uint8_t Current_board = 0; // The current display board that is read for switches pressed
uint32_t time_switch_pressed;

// ********************************* Section 2: Internal Switch Reading ********************************************

void inta_pin_interrupt() {
  time_switch_pressed = micros(); // Store the time switch was pressed. It will be used for tap tempo if the switch is programmed that way.
}

void setup_switch_check() {

#ifdef POWER_SWITCH_PIN
  pinMode(POWER_SWITCH_PIN, INPUT_PULLUP);
#endif

  // Setup interrupt pin for INT_A:
#ifdef INTA_PIN
  pinMode(INTA_PIN, INPUT_PULLUP);
  attachInterrupt(INTA_PIN, inta_pin_interrupt, FALLING);
#endif

  // initialize switchpad
#ifdef ROWPINS
  switchpad.init();
#endif

  // Initialize external jacks
#ifdef JACK1_PINS
  for (uint8_t j = 0; j < NUMBER_OF_CTL_JACKS; j++) {
    ctl_jack[j].init();
  }
#endif
}

void main_switch_check() {
  // Reset switch variables

#ifdef INTA_PIN
  SC_check_display_board_switch();
#endif

#ifdef ROWPINS
  // Check for switch pressed on switchpad
  if (switchpad.update() ) {
    switch_pressed = switchpad.pressed();
    switch_released = switchpad.released();
    switch_is_expression_pedal = false;
  }
#endif

#ifdef JACK1_PINS
  SC_check_external_switches();
#endif

#ifdef POWER_PIN
  SC_update_power_switch();
#endif

  SC_update_long_presses_and_hold();
}

#ifdef POWER_SWITCH_PIN
void SC_update_power_switch() {
  power_switch.update();
  if (power_switch.fallingEdge()) { // Check if power switch is pressed
    switch_pressed = POWER_SWITCH_NUMBER;
  }

  if (power_switch.risingEdge()) { // Check if power switch is released
    switch_released = POWER_SWITCH_NUMBER;
  }
}
#endif


#ifdef INTA_PIN
void SC_check_display_board_switch() {
  // Check for a switch pressed which is connected to a display board
  if (millis() - Debounce_timer > DEBOUNCE_TIME) { // Check if switch is debounced..
    if (digitalRead(INTA_PIN) == LOW) { // Check if interrupt is triggered
      check_switches_on_current_board(false); // Read the state as it was when the interrupt was triggered - TEST: what is we check the "normal state" here? Does that fix the false triggers?
      if (digitalRead(INTA_PIN) == LOW) { // Check if INT_A pin is still LOW
        // we did not find the source of the interrupt and we will need to read the next board
        Select_next_board();
        // And we do not restart the timer here.
      }
      else Debounce_timer = millis(); // Restart the timer
    }
    else { // Do "regular" check
      check_switches_on_current_board(false); //read the current state
      Debounce_timer = millis(); // Restart the timer
    }
  }
}

void check_switches_on_current_board(bool check_interrupt_state) {
  uint8_t new_switch_pressed = 0;
  uint8_t new_switch_released = 0;
  bool updated;

  // Read the buttons on this board
  updated = lcd[Current_board].buttonProcess(check_interrupt_state);
  
  if (updated) {
    uint8_t button_state = lcd[Current_board].buttonPressed();
    //DEBUGMAIN("  Button_state pressed board" + String(Current_board) + ": " + String(button_state) + " at " + String(micros() - time_switch_pressed));
    if (button_state & 1) new_switch_pressed = Current_board + 1; // Switch is in bottom row (1-4)
    if (button_state & 2) new_switch_pressed = Current_board + NUMBER_OF_DISPLAY_BOARDS + 1; // Switch is in second row (5-8)
    if (button_state & 4) new_switch_pressed = Current_board + (NUMBER_OF_DISPLAY_BOARDS * 2) + 1; // Switch is in third row (9-12)
    if (button_state & 8) new_switch_pressed = Current_board + (NUMBER_OF_DISPLAY_BOARDS * 3) + 1; //Switch is in top row (13 - 16)

    if (new_switch_pressed != previous_switch_pressed) { // Check for state change
      previous_switch_pressed = new_switch_pressed; // Need to store the previous version, because switch_pressed can only be active for one cycle!
      switch_pressed = new_switch_pressed;
      switch_is_expression_pedal = false;
    }

    button_state = lcd[Current_board].buttonReleased();
    //DEBUGMAIN("  Button_state released board" + String(Current_board) + ": " + String(button_state) + " at " + String(micros() - time_switch_pressed));
    if (button_state & 1) new_switch_released = Current_board + 1; // Switch is in bottom row (1-4)
    if (button_state & 2) new_switch_released = Current_board + NUMBER_OF_DISPLAY_BOARDS + 1; // Switch is in second row (5-8)
    if (button_state & 4) new_switch_released = Current_board + (NUMBER_OF_DISPLAY_BOARDS * 2) + 1; // Switch is in third row (9-12)
    if (button_state & 8) new_switch_released = Current_board + (NUMBER_OF_DISPLAY_BOARDS * 3) + 1; //Switch is in top row (13 - 16)

    if (new_switch_released != previous_switch_released) { // Check for state change
      previous_switch_released = new_switch_released; // Need to store the previous version, because switch_released can only be active for one cycle!
      switch_released = new_switch_released;
    }
  }
  //DEBUGMSG("******* Switches read on board " + String(Current_board) + " at " + String(micros() - time_switch_pressed) + "!!! ********");
}

void Select_next_board() {
  Current_board++;
  if (Current_board >= NUMBER_OF_DISPLAY_BOARDS) Current_board = 0;
}
#endif

// ********************************* Section 3: External switch and Expression Pedal Reading ********************************************

#ifdef JACK1_PINS
void SC_check_external_switches() {
  for (uint8_t j = 0; j < NUMBER_OF_CTL_JACKS; j++) {
    if (ctl_jack[j].update()) {
      uint8_t my_press = ctl_jack[j].pressed();
      if (my_press > 0) switch_pressed = NUMBER_OF_SWITCHES + (j * 2) + my_press;
      uint8_t my_release = ctl_jack[j].released();
      if (my_release > 0) switch_released = NUMBER_OF_SWITCHES + (j * 2) + my_release;
      Expr_ped_value = ctl_jack[j].pedal_value();
      switch_is_expression_pedal = ctl_jack[j].check_expr_pedal();
    }
  }
}
#endif

void SC_display_raw_value() {
  if (calibrate_exp_pedal < NUMBER_OF_CTL_JACKS) {
    LCD_show_status_message("EXP" + String(calibrate_exp_pedal + 1) + ": " + String(ctl_jack[calibrate_exp_pedal].get_raw_value()));
  }
}

void SC_set_expr_min() { // Set minimum value for selected expression pedal in the menu
#ifdef JACK1_PINS
  if (calibrate_exp_pedal < NUMBER_OF_CTL_JACKS) {
    ctl_jack[calibrate_exp_pedal].calibrate_min();
    Setting.exp_min[calibrate_exp_pedal] = ctl_jack[calibrate_exp_pedal].get_min();
    LCD_show_status_message("Min set for EXP" + String(calibrate_exp_pedal + 1));
  }
#endif
}

void SC_set_expr_max() { // Set maximum value for selected expression pedal in the menu
#ifdef JACK1_PINS
  if (calibrate_exp_pedal < NUMBER_OF_CTL_JACKS) {
    ctl_jack[calibrate_exp_pedal].calibrate_max();
    Setting.exp_max[calibrate_exp_pedal] = ctl_jack[calibrate_exp_pedal].get_max();
    LCD_show_status_message("Max set for EXP" + String(calibrate_exp_pedal + 1));
  }
#endif
}

void SC_set_auto_calibrate() { // Set expression pedal to auto calibrate in meu
#ifdef JACK1_PINS
  if (calibrate_exp_pedal < NUMBER_OF_CTL_JACKS) {
    ctl_jack[calibrate_exp_pedal].set_max(0);
    Setting.exp_max[calibrate_exp_pedal] = 0;
    LCD_show_status_message("AutoCalibr. EXP" + String(calibrate_exp_pedal + 1));
  }
#endif
}

void SC_check_calibration() {
#ifdef JACK1_PINS
  DEBUGMSG("Checking calibration...");
  for (uint8_t j = 0; j < NUMBER_OF_CTL_JACKS; j++) {
    DEBUGMSG("EXP PEDAL " + String(j + 1) + "! Max value: " + String(Setting.exp_max[j]) + ", Min value: " + String(Setting.exp_min[j]));
    if (ctl_jack[j].check_calibration() == false) {
      LCD_show_status_message("Recalibrate EXP" + String(j + 1));
      DEBUGMAIN("Please re-calibrate EXP PEDAL " + String(j + 1) + "! Max value: " + String(Setting.exp_max[j]) + ", Min value: " + String(Setting.exp_min[j]));
    }
  }
#endif
}

uint8_t SC_current_exp_pedal() {
  DEBUGMSG("Current switch: " + String(switch_pressed));
  return (last_switch_pressed - NUMBER_OF_SWITCHES) / 2;
}

void SC_set_expression_pedals() { // Called from EEP_read_eeprom_common_data()
#ifdef JACK1_PINS
  for (uint8_t j = 0; j < NUMBER_OF_CTL_JACKS; j++) { // Set max and min values in ctl_jack objects
    DEBUGMSG("Expression pedal " + String(j) + "...");
    ctl_jack[j].set_max(Setting.exp_max[j]);
    if (Setting.exp_max[j] > 0) ctl_jack[j].set_min(Setting.exp_min[j]);
  }
#endif
}

bool SC_switch_is_expr_pedal() {
  return switch_is_expression_pedal;
}

// ********************************* Section 4: Switch Long Press / Extra Long Press and Hold Detection ********************************************
void SC_update_long_presses_and_hold() {
  if (switch_released > 0) {
    DEBUGMAIN("Switch released: " + String(switch_released) + " at " + String(micros() - time_switch_pressed));
    Long_press_timer = 0;  //Reset the timer on switch released
    Extra_long_press_timer = 0;
    Hold_timer = 0;
  }

  // Now check for Long pressing a button
  if (switch_pressed > 0) {
    if  (switch_is_expression_pedal) {
      DEBUGMAIN("Expression pedal " + String(switch_pressed) + ": " + String(Expr_ped_value));
    }
    else {
      DEBUGMAIN("Switch pressed: " + String(switch_pressed) + " at " + String(micros() - time_switch_pressed));
    }
    Long_press_timer = millis(); // Set timer on switch pressed
    Extra_long_press_timer = millis(); // Set timer on switch extra long pressed
    Hold_timer = millis(); // Set timer on updown
    Hold_time = 700;
    switch_held_times_triggered = 0;
    last_switch_pressed = switch_pressed; // Remember the button that was pressed
  }

  if ((millis() - Long_press_timer > LONG_PRESS_TIMER_LENGTH) && (Long_press_timer > 0) && (!switch_is_expression_pedal)) {
    //if (!recheck_switches_on_current_board()) {
    switch_long_pressed = last_switch_pressed; //pass on the buttonvalue we remembered before
    Long_press_timer = 0;
    DEBUGMAIN("Switch long pressed: " + String(switch_long_pressed));
    //}
  }

  // Also check for extra long pressing a button
  if ((millis() - Extra_long_press_timer > EXTRA_LONG_PRESS_TIMER_LENGTH) && (Extra_long_press_timer > 0) && (!switch_is_expression_pedal)) {
    //if (!recheck_switches_on_current_board()) {
    switch_extra_long_pressed = last_switch_pressed; //pass on the buttonvalue we remembered before
    Extra_long_press_timer = 0;
    DEBUGMAIN("Switch extra long pressed: " + String(switch_extra_long_pressed));
    //}
  }

  if ((millis() - Hold_timer > Hold_time) && (Hold_timer > 0) && (!switch_is_expression_pedal)) { // To check if a switch is held down
    //if (!recheck_switches_on_current_board()) {
    switch_held = last_switch_pressed; //pass on the buttonvalue we remembered before
    Hold_timer = millis();
    switch_held_times_triggered++;
    Hold_time = 500;
    if (switch_held_times_triggered > 3) Hold_time = 250; // Increase speed when switch is held longer
    if (switch_held_times_triggered > 11) Hold_time = 125;
    if (switch_held_times_triggered > 27) Hold_time = 63;
    DEBUGMAIN("Switch held: " + String(switch_held));
    //}
  }
}

void SC_remote_switch_pressed(uint8_t sw) {
  if (sw < NUMBER_OF_SWITCHES + NUMBER_OF_EXTERNAL_SWITCHES + 1) {
    time_switch_pressed = micros();
    switch_pressed = sw;
  }
}

void SC_remote_switch_released(uint8_t sw) {
  if (sw < NUMBER_OF_SWITCHES + NUMBER_OF_EXTERNAL_SWITCHES + 1) switch_released = sw;
}
