// Please read VController_v3.ino for information about the license and authors

#ifndef SWITCHEXT_H
#define SWITCHEXT_H

#include "debug.h"

// Library for dual purpose jacks. Both expression pedals and normal switches can be connected
// Can detect switch type by measuring voltage on jack ring.
// Also detects and supports both normally open and normally closed switches by checking the value at the tip on startup.

// Define switch types
#define DETECT 0
#define SWITCH_NORMALLY_OPEN 1
#define SWITCH_NORMALLY_CLOSED 2
#define EXPRESSION_PEDAL 3

#define DEFAULT_EXP_PEDAL_UPDATE_TIME 25 // Time between two updates of the expression pedal.
#define NUMBER_OF_UNIQUE_READINGS 3 // Number of values remembered. To avoid jitter of the expression pedal, a new value must be unique.

class SwitchExt
{
  public:
    SwitchExt(uint8_t dpin_tip, uint8_t dpin_ring, uint8_t apin_tip, uint8_t apin_ring, uint16_t max_ring_for_exp_pedal, uint32_t debounceTime, uint8_t switch_type, bool pull_up);
    void init();
    bool update();
    uint8_t pedal_value();
    uint8_t pressed();
    uint8_t released();
    bool check_expr_pedal();
    uint16_t get_raw_value();
    void calibrate_max();
    void calibrate_min();
    void set_max(uint8_t value);
    void set_min(uint8_t value);
    uint8_t get_max();
    uint8_t get_min();
    bool check_calibration();

  private:
    void detect_type();
    bool update_expr_pedal();
    bool update_switches();
    bool check_unique_value(uint16_t new_value);

    uint8_t dpin_tip; // Pin number of digital pin connected to the tip of the Jack connector
    uint8_t dpin_ring; // Pin number of digital pin connected to the ring of the Jack connector
    uint8_t apin_tip; // Pin number of analog pin connected to the tip of the Jack connector
    uint8_t apin_ring; // Pin number of analog pin connected to the ring of the Jack connector
    uint16_t max_ring_for_exp_pedal;
    bool pull_up; // Do the inputs need the internal pullup resistors activated
    bool auto_calibrate;
    uint8_t switch_type;
    uint32_t updateTimer; // Timer for update - will determine the number of updates on an expression pedal
    uint32_t updateTime; // Time between two updates of the pedal

    uint8_t newState; // New digital state for external switches
    uint8_t State; // Current state of external switches
    uint8_t releasedState;
    uint32_t bounceDelay;
    uint32_t debounceTime; // Debounce time for switches
    bool do_not_check_ring; // Do not check ring with normally open switch with mono jack

    uint16_t expr_value;
    uint16_t expr_ped_min; // Minimal value the expression pedal has reached
    uint16_t expr_ped_max; // maximum value the expression pedal has reached

    uint16_t reading[NUMBER_OF_UNIQUE_READINGS] = { 255 };
    uint8_t read_index;
};

SwitchExt::SwitchExt(uint8_t dpin_tip, uint8_t dpin_ring, uint8_t apin_tip, uint8_t apin_ring, uint16_t max_ring_for_exp_pedal, uint32_t debounceTime, uint8_t switch_type, bool pull_up) {
  this->dpin_tip = dpin_tip;
  this->dpin_ring = dpin_ring;
  this->apin_tip = apin_tip;
  this->apin_ring = apin_ring;
  this->max_ring_for_exp_pedal = max_ring_for_exp_pedal;
  auto_calibrate = true;
  newState = 0;
  State = 0;
  releasedState = 0;
  do_not_check_ring = false;
  this->debounceTime = debounceTime;
  this->switch_type = switch_type;
  this->pull_up = pull_up;
  this->updateTime = DEFAULT_EXP_PEDAL_UPDATE_TIME;
  this->expr_ped_max = 0;
  this->expr_ped_min = 1023;
  read_index = 0;
}

void SwitchExt::init() {
  // Check if internal pull-up resistors should be activated
  if (pull_up) {
    pinMode(dpin_tip, INPUT_PULLUP);
    pinMode(dpin_ring, INPUT_PULLUP);
    delay(10); //Short delay to allow the ports to settle
  }
  else {
    pinMode(dpin_tip, INPUT);
    pinMode(dpin_ring, INPUT);
  }

  // Detect type
  if (switch_type == DETECT) detect_type();
  //DEBUGMSG("Type: " + String(switch_type));

  // If an expression pedal is connected and there are no external pull up resistors, put power on the ring
  if ((switch_type == EXPRESSION_PEDAL) && (pull_up)) {
    pinMode(dpin_ring, OUTPUT);
    digitalWrite(dpin_ring, HIGH);
  }
}

void SwitchExt::detect_type() {
  // First check if it is an expression pedal by checking the voltage on the analog ring pin
  uint16_t ring_value = analogRead(apin_ring);
  DEBUGMSG("Ring value: " + String(ring_value));
  if ((ring_value > 100) && (ring_value < max_ring_for_exp_pedal)) {
    switch_type = EXPRESSION_PEDAL;
    DEBUGMAIN("Expression pedal detected on pin " + String(apin_ring));
    for (uint8_t i = 0; i < NUMBER_OF_UNIQUE_READINGS; i++) update_expr_pedal(); // Do a first couple of readings, so we do not get a false trigger at startup
    return;
  }

  // Then check if it is normally open or normally closed by checking the digital tip pin
  if (digitalRead(dpin_tip) == HIGH) {
    switch_type = SWITCH_NORMALLY_OPEN;
    if (digitalRead(dpin_ring) == LOW) do_not_check_ring = true; // Exception for mono plug
    DEBUGMAIN("Normally open switch detected on pin " + String(dpin_tip));
  }
  else {
    switch_type = SWITCH_NORMALLY_CLOSED;
    DEBUGMAIN("Normally closed switch detected on pin " + String(dpin_tip));
  }
}

bool SwitchExt::update() {
  if (millis() > updateTimer + updateTime) {
    updateTime = millis(); // Reset the timer
    if (switch_type == EXPRESSION_PEDAL) {
      return update_expr_pedal();
    }
    else {
      return update_switches();
    }
  }
  return 0;
}

bool SwitchExt::update_expr_pedal() {

  uint16_t new_state = analogRead(apin_tip);
  //DEBUGMSG("Analog pin " + String(apin_tip) + ": " + String(new_state));

  if (auto_calibrate) {
    if (new_state < expr_ped_min) expr_ped_min = new_state;
    if (new_state > expr_ped_max) expr_ped_max = new_state;
  }
  else {
    if (new_state < expr_ped_min) new_state = expr_ped_min;
    if (new_state > expr_ped_max) new_state = expr_ped_max;
  }

  uint16_t new_value = map(new_state, expr_ped_min, expr_ped_max, 0, 137); // Map to 137 instead of 127, so we have five on either side, so we always reach min and max value.
  if ((expr_ped_max - expr_ped_min < 137)) return false; // exit if we have not calibrated yet...
  if (check_unique_value(new_value) == false) return false; // exit if this is not a unique value
  if (new_value > 132) new_value = 132;
  if (new_value < 5) new_value = 5;
  expr_value = new_value - 5; // Set the expression value variable
  State = 1; // we will also return state as one, to simulate pressing a button
  return true;
}

bool SwitchExt::check_unique_value(uint16_t new_value) {

  for (uint8_t i = 0; i < NUMBER_OF_UNIQUE_READINGS; i++) { // Check if the new-value is in the reading array
    if (reading[i] == new_value) return false;
  }

  reading[read_index] = new_value; // Add new unique value to reading array
  read_index++;
  if (read_index >= NUMBER_OF_UNIQUE_READINGS) read_index = 0;
  return true;
}

uint8_t SwitchExt::pedal_value() {
  if (switch_type == EXPRESSION_PEDAL) {
    return expr_value;
  }
  else { // If it is a switch return 127 when pressed and 0 when not.
    if (State == 1) return 127;
    else return 0;
  }
}

bool SwitchExt::update_switches() {
  if (millis() > bounceDelay + debounceTime) {
    // Read new state
    newState = 0;
    if (switch_type == SWITCH_NORMALLY_CLOSED) {
      if (digitalRead(dpin_tip) == HIGH) newState = 1;
      if (digitalRead(dpin_ring) == HIGH) newState = 2;
    }
    else {
      if (digitalRead(dpin_tip) == LOW) newState = 1;
      if ((digitalRead(dpin_ring) == LOW) && (!do_not_check_ring)) newState = 2; // Exception for mono jack
    }

    // Reset bounce delay timer
    bounceDelay = millis();

    if (newState != State) {
      //DEBUGMSG("Pressed switch " + String(newState));
      if (newState == 0) releasedState = State; //Remember the last state when we go back to zero
      else releasedState = 0;
      State = newState;
      return true;
    }
  }

  return false;
}

uint8_t SwitchExt::pressed() {
  return State;
}

uint8_t SwitchExt::released() {
  return releasedState;
}

bool SwitchExt::check_expr_pedal() {
  if (switch_type == EXPRESSION_PEDAL) return true;
  return false;
}

// Calibration stuff

uint16_t SwitchExt::get_raw_value() {
  return analogRead(apin_tip);
}

void SwitchExt::calibrate_max() {
  if (switch_type == EXPRESSION_PEDAL) {
    expr_ped_max = analogRead(apin_tip);
    auto_calibrate = false;
    DEBUGMSG("Calibration - max value = " + String(expr_ped_max));
  }
}

void SwitchExt::calibrate_min() {
  if (switch_type == EXPRESSION_PEDAL) {
    expr_ped_min = analogRead(apin_tip);
    auto_calibrate = false;
    DEBUGMSG("Calibration - min value = " + String(expr_ped_min));
  }
}

void SwitchExt::set_max(uint8_t value) {
  if (value == 0) {
    auto_calibrate = true;
    expr_ped_max = 0;  // Reset the calibration values so the pedal will recalibrate.
    expr_ped_min = 1023;
    DEBUGMSG("...set to auto-calibrate");
  }
  else {
    expr_ped_max = value << 4;
    auto_calibrate = false;
    DEBUGMSG("...max. value set to " + String(expr_ped_max));
  }
}

void SwitchExt::set_min(uint8_t value) {
  expr_ped_min = value << 4;
  DEBUGMSG("...min. value set to " + String(expr_ped_min));
}

uint8_t SwitchExt::get_max() {
  if (auto_calibrate) return 0;
  return (expr_ped_max >> 4);
}

uint8_t SwitchExt::get_min() {
  if (auto_calibrate) return 0;
  return (expr_ped_min >> 4);
}

bool SwitchExt::check_calibration() {
  if (auto_calibrate) return true;
  if (expr_ped_max - expr_ped_min >= 137) return true;
  return false;
}

#endif
