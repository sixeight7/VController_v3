// Please read VController_v3.ino for information about the license and authors

// Reading of switches, encoders and expression pedals

// This page has the following parts:
// Section 1: Switch Initialization
// Section 2: Internal Switch Reading
// Section 3: External Switch and Expression Pedal Reading
// Section 4: Switch Dual Press / Long Press / Extra Long Press and Hold Detection
// Section 5: Remote MIDI control of switches

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

// External library used for encoders:

// Create object for power switch
#ifdef POWER_SWITCH_PIN
#include <Bounce.h>
Bounce power_switch = Bounce(POWER_SWITCH_PIN, 50);
#endif

#ifdef SWITCH1_PIN
#include <Bounce.h>
Bounce switch_direct[NUMBER_OF_CONNECTED_SWITCHES] = {
  Bounce(SWITCH1_PIN, SWITCH_BOUNCE_TIME),
#endif
#ifdef SWITCH2_PIN
  Bounce(SWITCH2_PIN, SWITCH_BOUNCE_TIME),
#endif
#ifdef SWITCH3_PIN
  Bounce(SWITCH3_PIN, SWITCH_BOUNCE_TIME),
#endif
#ifdef SWITCH4_PIN
  Bounce(SWITCH4_PIN, SWITCH_BOUNCE_TIME),
#endif
#ifdef SWITCH5_PIN
  Bounce(SWITCH5_PIN, SWITCH_BOUNCE_TIME),
#endif
#ifdef SWITCH6_PIN
  Bounce(SWITCH6_PIN, SWITCH_BOUNCE_TIME),
#endif
#ifdef SWITCH1_PIN
};
#endif

#ifdef CUSTOM_SWITCH_NUMBERS
const uint8_t switch_number[NUMBER_OF_CONNECTED_SWITCHES] = { CUSTOM_SWITCH_NUMBERS };
#endif

// Create objects for switchpad (max 1) if defined in hardware.h
// Functionality of the switchpad is described in switchpad.h
#ifdef ROWPINS
#include "switchpad_lib.h"
byte rowPins[ROWS] = {ROWPINS}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {COLUMNPINS}; //connect to the column pinouts of the keypad

Switchpad switchpad = Switchpad(rowPins, colPins, ROWS, COLS, SWITCH_BOUNCE_TIME);
#endif

#ifdef ENCODER1_A_PIN
#include <ClickEncoder.h>
#include <TimerOne.h>
ClickEncoder enc1 = ClickEncoder(ENCODER1_B_PIN, ENCODER1_A_PIN, -1, 2);
#endif
#ifdef ENCODER1_SWITCH_PIN
#include <Bounce.h>
Bounce enc1_switch = Bounce(ENCODER1_SWITCH_PIN, SWITCH_BOUNCE_TIME);
#endif
#ifdef ENCODER2_A_PIN
//Encoder enc2(ENCODER2_B_PIN, ENCODER2_A_PIN);
ClickEncoder enc2 = ClickEncoder(ENCODER2_B_PIN, ENCODER2_A_PIN, -1, 2);
#endif
#ifdef ENCODER2_SWITCH_PIN
Bounce enc2_switch = Bounce(ENCODER2_SWITCH_PIN, SWITCH_BOUNCE_TIME);
#endif

#ifdef ENCODER1_A_PIN
void timerIsr() {
  enc1.service();
#ifdef ENCODER2_A_PIN
  enc2.service();
#endif
}
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

#define SW_TYPE_SWITCH 0
#define SW_TYPE_EXPRESSION_PEDAL 1
#define SW_TYPE_ENCODER 2
#define SW_TYPE_MIDI_CC 3
#define SW_TYPE_MIDI_CC_NO_RELEASE 4
#define SW_TYPE_MIDI_PC 5
uint8_t switch_type = SW_TYPE_SWITCH;
uint8_t switch_pressed = 0; //Variable set when switch is pressed
uint8_t switch_released = 0; //Variable set when switch is released
uint8_t switch_long_pressed = 0; //Variable set when switch is pressed long (check LONG_PRESS_TIMER_LENGTH for when this will happen)
uint8_t switch_extra_long_pressed = 0; //Variable set when switch is pressed long (check LONG_PRESS_TIMER_LENGTH for when this will happen)
uint8_t last_switch_pressed;
bool switch_was_long_pressed = false;
uint32_t multi_switch_booleans = 0;
uint8_t multi_switch_pressed = 0;
#define SPECIAL_KEY_COMBINATION 255 // Special value for multi_switch_pressed
uint8_t Expr_ped_value = 0;
signed int Enc_value = 0;
uint8_t PC_value = 0;
uint8_t Enc1_value = 0;
uint8_t Enc2_value = 0;
uint8_t previous_switch_pressed = 0;
uint8_t previous_switch_released = 0;
uint8_t switch_held = 0;
uint8_t switch_held_times_triggered = 0;
uint32_t Hold_timer = 0;
uint32_t Hold_time;
bool inta_triggered = false;
uint8_t skip_release_and_hold_until_next_press = 0;
#define SKIP_RELEASE 1
#define SKIP_LONG_PRESS 2
#define SKIP_HOLD 4

#define DEBOUNCE_TIMER_LENGTH 20 // Time between reading the display boards
uint32_t Debounce_timer = 0;
bool Debounce_timer_active = false;
bool Debounce_normal_state_timer_active = false;

#define LONG_PRESS_TIMER_LENGTH 1000 // Timer used for detecting long-pressing switch. Time is in milliseconds
#define EXTRA_LONG_PRESS_TIMER_LENGTH 3000
uint32_t Long_press_timer = 0;
uint32_t Extra_long_press_timer = 0;
uint8_t Current_board = 0; // The current display board that is read for switches pressed
uint32_t time_switch_pressed;

#define ENCODER_TURN_TIME 1000 // The time that needs to pass before an encoder is allowed to go from max to min or visa versa
uint32_t encoder_timer = 0;


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

  // initialize direct connected switches
#ifdef SWITCH1_PIN
  pinMode(SWITCH1_PIN, INPUT_PULLUP);
#endif

#ifdef SWITCH2_PIN
  pinMode(SWITCH2_PIN, INPUT_PULLUP);
#endif

#ifdef SWITCH3_PIN
  pinMode(SWITCH3_PIN, INPUT_PULLUP);
#endif

#ifdef SWITCH4_PIN
  pinMode(SWITCH4_PIN, INPUT_PULLUP);
#endif

#ifdef SWITCH5_PIN
  pinMode(SWITCH5_PIN, INPUT_PULLUP);
#endif

#ifdef SWITCH6_PIN
  pinMode(SWITCH6_PIN, INPUT_PULLUP);
#endif

#ifdef NUMBER_OF_CONNECTED_SWITCHES
  for (uint8_t s = 0; s < NUMBER_OF_CONNECTED_SWITCHES; s++) {
    switch_direct[s].update();
  }
#endif

  // initialize switchpad
#ifdef ROWPINS
  switchpad.init();
#endif

  // Initialize encoders
#ifdef ENCODER1_A_PIN
  enc1.setAccelerationEnabled(true);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);
#endif
#ifdef ENCODER1_SWITCH_PIN
  pinMode(ENCODER1_SWITCH_PIN, INPUT_PULLUP);
  enc1_switch.update();
#endif
#ifdef ENCODER2_A_PIN
  enc2.setAccelerationEnabled(true);
#endif
#ifdef ENCODER2_SWITCH_PIN
  pinMode(ENCODER2_SWITCH_PIN, INPUT_PULLUP);
  enc2_switch.update();
#endif

  // Initialize external jacks
#ifdef JACK1_PINS
  for (uint8_t j = 0; j < NUMBER_OF_CTL_JACKS; j++) {
    ctl_jack[j].init();
  }
#endif

  // Check for reset settings
#if defined SWITCH1_PIN && defined SWITCH3_PIN
  if ((digitalRead(SWITCH1_PIN) == LOW) && (digitalRead(SWITCH3_PIN) == LOW)) EEP_initialize_internal_eeprom_data(); // Initialize settings on pressing switch 1 and 3
#endif
}

void main_switch_check() {
  // Reset switch variables

#ifdef INTA_PIN
  SC_check_display_board_switch();
#endif

  // check direct connected switches
#ifdef NUMBER_OF_CONNECTED_SWITCHES
  for (uint8_t s = 0; s < NUMBER_OF_CONNECTED_SWITCHES; s++) {
    switch_direct[s].update();
    if (switch_direct[s].fallingEdge()) { // Check if switch is pressed
#ifdef CUSTOM_SWITCH_NUMBERS
      switch_pressed = switch_number[s];
#else
      switch_pressed = s + 1;
#endif
      switch_type = SW_TYPE_SWITCH;
    }
    if (switch_direct[s].risingEdge()) { // Check if switch is released
#ifdef CUSTOM_SWITCH_NUMBERS
      switch_released = switch_number[s];
#else
      switch_released = s + 1;
#endif
      switch_type = SW_TYPE_SWITCH;
    }
  }
#endif


#ifdef ROWPINS
  // Check for switch pressed on switchpad
  if (switchpad.update() ) {
    switch_pressed = switchpad.pressed();
    switch_released = switchpad.released();
    switch_type = SW_TYPE_SWITCH;
  }
#endif

  // Check encoders
  Enc_value = 0;
#ifdef ENCODER1_A_PIN
  int16_t new_reading = enc1.getValue();
  if (new_reading != 0) {
    Enc_value = new_reading;
    switch_pressed = NUMBER_OF_SWITCHES + 1;
    switch_type = SW_TYPE_ENCODER;
  }
#endif
#ifdef ENCODER1_SWITCH_PIN
  enc1_switch.update();
  if (enc1_switch.fallingEdge()) { // Check if power switch is pressed
    switch_pressed = NUMBER_OF_SWITCHES + 2;
    switch_type = SW_TYPE_SWITCH;
  }
  if (enc1_switch.risingEdge()) { // Check if power switch is released
    switch_released = NUMBER_OF_SWITCHES + 2;
    switch_type = SW_TYPE_SWITCH;
  }
#endif
#ifdef ENCODER2_A_PIN
  new_reading = enc2.getValue();
  if (new_reading != 0) {
    Enc_value = new_reading;
    switch_pressed = NUMBER_OF_SWITCHES + 3;
    switch_type = SW_TYPE_ENCODER;
  }
#endif
#ifdef ENCODER2_SWITCH_PIN
  enc2_switch.update();
  if (enc2_switch.fallingEdge()) { // Check if power switch is pressed
    switch_pressed = NUMBER_OF_SWITCHES + 4;
    switch_type = SW_TYPE_SWITCH;
  }
  if (enc2_switch.risingEdge()) { // Check if power switch is released
    switch_released = NUMBER_OF_SWITCHES + 4;
    switch_type = SW_TYPE_SWITCH;
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
    switch_type = SW_TYPE_SWITCH;
  }

  if (power_switch.risingEdge()) { // Check if power switch is released
    switch_released = POWER_SWITCH_NUMBER;
    switch_type = SW_TYPE_SWITCH;
  }
}
#endif


#ifdef INTA_PIN
void SC_check_display_board_switch() {
  // Check for a switch pressed which is connected to a display board
  if (millis() - Debounce_timer > DEBOUNCE_TIMER_LENGTH) { // Check if switch is debounced..
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
      switch_type = SW_TYPE_SWITCH;
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
      switch_type = SW_TYPE_SWITCH;
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
      if (my_press > 0) switch_pressed = NUMBER_OF_SWITCHES + (NUMBER_OF_ENCODERS * 2) + (j * 2) + my_press;
      uint8_t my_release = ctl_jack[j].released();
      if (my_release > 0) switch_released = NUMBER_OF_SWITCHES + (NUMBER_OF_ENCODERS * 2) + (j * 2) + my_release;
      Expr_ped_value = ctl_jack[j].pedal_value();
      if (ctl_jack[j].check_expr_pedal()) switch_type = SW_TYPE_EXPRESSION_PEDAL;
      else switch_type = SW_TYPE_SWITCH;
    }
  }
}
#endif

void SC_display_raw_value() {
  if (calibrate_exp_pedal < NUMBER_OF_CTL_JACKS) {
    LCD_show_popup_label("EXP" + String(calibrate_exp_pedal + 1) + ": " + String(ctl_jack[calibrate_exp_pedal].get_raw_value()), MESSAGE_TIMER_LENGTH);
  }
}

void SC_set_expr_min() { // Set minimum value for selected expression pedal in the menu
#ifdef JACK1_PINS
  if (calibrate_exp_pedal < NUMBER_OF_CTL_JACKS) {
    ctl_jack[calibrate_exp_pedal].calibrate_min();
    Setting.exp_min[calibrate_exp_pedal] = ctl_jack[calibrate_exp_pedal].get_min();
    LCD_show_popup_label("Min set for EXP" + String(calibrate_exp_pedal + 1), MESSAGE_TIMER_LENGTH);
  }
#endif
}

void SC_set_expr_max() { // Set maximum value for selected expression pedal in the menu
#ifdef JACK1_PINS
  if (calibrate_exp_pedal < NUMBER_OF_CTL_JACKS) {
    ctl_jack[calibrate_exp_pedal].calibrate_max();
    Setting.exp_max[calibrate_exp_pedal] = ctl_jack[calibrate_exp_pedal].get_max();
    LCD_show_popup_label("Max set for EXP" + String(calibrate_exp_pedal + 1), MESSAGE_TIMER_LENGTH);
  }
#endif
}

void SC_set_auto_calibrate() { // Set expression pedal to auto calibrate in meu
#ifdef JACK1_PINS
  if (calibrate_exp_pedal < NUMBER_OF_CTL_JACKS) {
    ctl_jack[calibrate_exp_pedal].set_max(0);
    Setting.exp_max[calibrate_exp_pedal] = 0;
    LCD_show_popup_label("AutoCalibr. EXP" + String(calibrate_exp_pedal + 1), MESSAGE_TIMER_LENGTH);
  }
#endif
}

void SC_check_calibration() {
#ifdef JACK1_PINS
  DEBUGMSG("Checking calibration...");
  for (uint8_t j = 0; j < NUMBER_OF_CTL_JACKS; j++) {
    DEBUGMSG("EXP PEDAL " + String(j + 1) + "! Max value: " + String(Setting.exp_max[j]) + ", Min value: " + String(Setting.exp_min[j]));
    if (ctl_jack[j].check_calibration() == false) {
      LCD_show_popup_label("Recalibrate EXP" + String(j + 1), MESSAGE_TIMER_LENGTH);
      DEBUGMAIN("Please re-calibrate EXP PEDAL " + String(j + 1) + "! Max value: " + String(Setting.exp_max[j]) + ", Min value: " + String(Setting.exp_min[j]));
    }
  }
#endif
}

uint8_t SC_current_exp_pedal() {
  DEBUGMSG("Current switch: " + String(switch_pressed));
  return (last_switch_pressed - NUMBER_OF_SWITCHES - (NUMBER_OF_ENCODERS * 2)) / 2;
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

inline bool SC_switch_is_expr_pedal() {
  return (switch_type == SW_TYPE_EXPRESSION_PEDAL);
}

inline bool SC_switch_is_encoder() {
  return (switch_type == SW_TYPE_ENCODER);
}

inline bool SC_switch_triggered_by_PC() {
  return (switch_type == SW_TYPE_MIDI_PC);
}

// ********************************* Section 4: Switch Dual Press / Long Press / Extra Long Press and Hold Detection ********************************************
void SC_update_long_presses_and_hold() {
  if (switch_released > 0) { // First check if the release was valid when in multi_switch mode

    if (switch_type == SW_TYPE_SWITCH) {
      SP[switch_released].Pressed = false;
      update_LEDS = true;

      // Check for release after multiple press
      multi_switch_booleans &= ~(1 << (switch_released - 1)); // Clear this bit
      if (multi_switch_pressed != 0) { // Make sure no switch_release commands are sent before all switches are back to zero
        if (multi_switch_booleans == 0) {
          if (multi_switch_pressed != SPECIAL_KEY_COMBINATION) switch_released = multi_switch_pressed | ON_DUAL_PRESS;
          else switch_released = 0;
          multi_switch_pressed = 0;
        }
        else {
          switch_released = 0; // Do not activate the release
        }
        DEBUGMSG("Release while multiswitch_pressed > 0: " + String(switch_released));
      }
    }

    if (switch_was_long_pressed) switch_released |= ON_LONG_PRESS; // Activate the proper release
    Long_press_timer = 0;  //Reset the timer on switch released
    switch_was_long_pressed = false;
    Extra_long_press_timer = 0;
    Hold_timer = 0;

    if (skip_release_and_hold_until_next_press & SKIP_RELEASE) {
      switch_released = 0;
      DEBUGMAIN("RESET switch release");
    }
  }

  if (switch_released > 0) {
    switch (switch_type) {
      case SW_TYPE_SWITCH:
        DEBUGMAIN("Switch released: " + String(switch_released) + " at " + String(micros() - time_switch_pressed));
        break;
      case SW_TYPE_MIDI_CC:
        DEBUGMAIN("MIDI CC switch released: " + String(switch_released) + " at " + String(micros() - time_switch_pressed));
        break;
    };
  }

  if (switch_pressed > 0) {

    if (switch_type == SW_TYPE_SWITCH) {
      SP[switch_pressed].Pressed = true;
      update_LEDS = true;
      multi_switch_booleans |= (1 << (switch_pressed - 1)); // Set this bit

      if (multi_switch_booleans & (1 << switch_pressed)) { // Switch on the right is also pressed
        DEBUGMAIN("Dual switches " + String(switch_pressed) + " and " + String(switch_pressed + 1) + " pressed");
        multi_switch_pressed = switch_pressed | ON_DUAL_PRESS;
        DEBUGMSG("MS pressed: " + String(multi_switch_pressed));

        switch_pressed = multi_switch_pressed;
      }
      if ((switch_pressed > 1) && (multi_switch_booleans & (1 << (switch_pressed - 2)))) { // Switch on the left is also pressed
        DEBUGMAIN("Dual switches " + String(switch_pressed - 1) + " and " + String(switch_pressed) + " pressed");
        multi_switch_pressed = (switch_pressed - 1) | ON_DUAL_PRESS;
        DEBUGMSG("MS pressed: " + String(multi_switch_pressed));
        switch_pressed = multi_switch_pressed;
      }

      if (multi_switch_booleans == MENU_KEY_COMBINATION) {
        SCO_select_page(PAGE_SELECT);
        //multi_switch_booleans = MENU_KEY_COMBINATION;
        multi_switch_pressed = SPECIAL_KEY_COMBINATION;
      }
    }

    switch (switch_type) {
      case SW_TYPE_SWITCH:
        DEBUGMAIN("Switch pressed: " + String(switch_pressed) + " at " + String(micros() - time_switch_pressed));
        break;
      case SW_TYPE_EXPRESSION_PEDAL:
        DEBUGMAIN("Expression pedal: " + String(switch_pressed) + ": " + String(Expr_ped_value));
        break;
      case SW_TYPE_ENCODER:
        DEBUGMAIN("Encoder turned: " + String(switch_pressed) + ": " + String(Enc_value));
        break;
      case SW_TYPE_MIDI_CC:
        DEBUGMAIN("MIDI CC switch pressed: " + String(switch_pressed) + " at " + String(micros() - time_switch_pressed));
        break;
      case SW_TYPE_MIDI_PC:
        DEBUGMAIN("MIDI PC switch pressed: " + String(switch_pressed) + " at " + String(micros() - time_switch_pressed));
        break;
      case SW_TYPE_MIDI_CC_NO_RELEASE:
        DEBUGMAIN("MIDI CC switch (no release) pressed: " + String(switch_pressed) + " at " + String(micros() - time_switch_pressed));
        break;
    };
    switch_was_long_pressed = false;
    Long_press_timer = millis(); // Set timer on switch pressed
    Extra_long_press_timer = millis(); // Set timer on switch extra long pressed
    Hold_timer = millis(); // Set timer on switch held
    Hold_time = 700;
    switch_held_times_triggered = 0;
    last_switch_pressed = switch_pressed; // Remember the button that was pressed
    skip_release_and_hold_until_next_press = false;
  }

  if ((switch_type == SW_TYPE_SWITCH) || (switch_type == SW_TYPE_MIDI_CC)) {
    if ((millis() - Long_press_timer > LONG_PRESS_TIMER_LENGTH) && (Long_press_timer > 0) && (!(skip_release_and_hold_until_next_press & SKIP_LONG_PRESS))) {
      switch_long_pressed = last_switch_pressed; //pass on the switch number we remembered before
      Long_press_timer = 0;
      switch_was_long_pressed = true;
      DEBUGMAIN("Switch long pressed: " + String(switch_long_pressed));
      //}
    }

    // Also check for extra long pressing a button
    if ((millis() - Extra_long_press_timer > EXTRA_LONG_PRESS_TIMER_LENGTH) && (Extra_long_press_timer > 0)) {
      //if (!recheck_switches_on_current_board()) {
      switch_extra_long_pressed = last_switch_pressed; //pass on the switch number we remembered before
      Extra_long_press_timer = 0;
      DEBUGMAIN("Switch extra long pressed: " + String(switch_extra_long_pressed));
      //}
    }

    if ((millis() - Hold_timer > Hold_time) && (Hold_timer > 0) && (!(skip_release_and_hold_until_next_press & SKIP_HOLD))) { // To check if a switch is held down
      switch_held = last_switch_pressed; //pass on the switch number we remembered before
      Hold_timer = millis();
      switch_held_times_triggered++;
      Hold_time = 300;
      if (switch_held_times_triggered > 5) Hold_time = 150; // Increase speed when switch is held longer
      if (switch_held_times_triggered > 15) Hold_time = 75;
      if (switch_held_times_triggered > 45) Hold_time = 38;
      DEBUGMAIN("Switch held: " + String(switch_held));
    }
  }
}

// ********************************* Section 5: Remote MIDI control of switches ********************************************

void SC_remote_switch_pressed(uint8_t sw, bool from_editor) {
  DEBUGMAIN("Remote press SW " + String(sw));
  if (SC_check_valid_switch(sw)) {
    time_switch_pressed = micros();
    switch_pressed = sw;
    if (from_editor) switch_type = SW_TYPE_SWITCH;
    else switch_type = SW_TYPE_MIDI_CC;
  }
}

void SC_remote_switch_released(uint8_t sw, bool from_editor) {
  DEBUGMAIN("Remote release SW " + String(sw));
  if (SC_check_valid_switch(sw)) {
    time_switch_pressed = micros();
    switch_released = sw;
    if (from_editor) switch_type = SW_TYPE_SWITCH;
    else switch_type = SW_TYPE_MIDI_CC;
  }
}

void SC_remote_switch_pressed_no_release(uint8_t sw) {
  if (SC_check_valid_switch(sw)) {
    time_switch_pressed = micros();
    switch_pressed = sw;
    switch_type = SW_TYPE_MIDI_CC_NO_RELEASE;
  }
}

void SC_remote_expr_pedal(uint8_t sw, uint8_t value) {
  if (SC_check_valid_switch(sw)) {
    time_switch_pressed = micros();
    switch_type = SW_TYPE_EXPRESSION_PEDAL;
    Expr_ped_value = value;
    switch_pressed = sw;
  }
}

void SC_remote_switch_select_program(uint8_t sw, uint8_t program) {
  if (SC_check_valid_switch(sw)) {
    time_switch_pressed = micros();
    switch_type = SW_TYPE_MIDI_PC;
    PC_value = program;
    switch_pressed = sw;
  }
}

bool SC_check_valid_switch(uint8_t sw) {
  return (sw < TOTAL_NUMBER_OF_SWITCHES + 1);
}

void SC_skip_release_and_hold_until_next_press(uint8_t val) { // Called from SCO_select_page() and global tuner press. Make sure no switch_release, long_press, etc is triggered until we press a switch again..
  skip_release_and_hold_until_next_press = val;
  DEBUGMSG("Set switches to skip release, hold or long press - " + String(val));
  //multi_switch_booleans = 0;
}

void SC_set_enc1_acceleration(bool state) {
#ifdef ENCODER1_A_PIN
  enc1.setAccelerationEnabled(state);
#endif
}


#define ACCELERATION_TIME 50

uint16_t update_encoder_value(signed int delta, uint16_t value, uint16_t min, uint16_t max) {
  setAccelerationRange(max - min);

  if (delta > 0) {
    for (uint16_t i = 0; i < delta; i++) {
      if (value >= max) { // Check if we've reached the top
        if ((encoder_timer + ENCODER_TURN_TIME > millis()) || (delta > 1)) value = max;
        else value = min;
      }
      else value++;
    }
  }
  // Perform bank down:
  if (delta < 0) {
    for (uint16_t i = 0; i < abs(delta); i++) {
      if (value <= min) { // Check if we've reached the bottom
        if ((encoder_timer + ENCODER_TURN_TIME > millis()) || (delta < -1)) value = min;
        else value = max;
      }
      else value--;
    }
  }
  encoder_timer = millis();
  return value;
}

void setAccelerationRange(uint16_t range) {
#ifdef ENCODER1_A_PIN
  enc1.setAccelerationRange(range);
  enc2.setAccelerationRange(range);
#endif
}
