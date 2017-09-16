#ifndef SWITCHEXT_H
#define SWITCHEXT_H

// Library for dual purpose jacks. Both expression pedals and normal switches can be connected
// Can detect switch type by measuring voltage on jack ring.
// Also detects and supports both normally open and normally closed switches by checking the value at the tip on startup.

// Define switch types
#define DETECT 0
#define SWITCH_NORMALLY_OPEN 1
#define SWITCH_NORMALLY_CLOSED 2
#define EXPRESSION_PEDAL 3

#define DEFAULT_UPDATE_TIME 25

class SwitchExt
{
  public:
    SwitchExt(uint8_t dpin_tip, uint8_t dpin_ring, uint8_t apin_tip, uint8_t apin_ring, uint32_t debounceTime, uint8_t switch_type, bool pull_up);
    void init();
    bool update();
    uint8_t pedal_value();
    uint8_t pressed();
    uint8_t released();
    bool check_expr_pedal();

  private:
    void detect_type();
    bool update_expr_pedal();
    bool update_switches();
    uint8_t dpin_tip; // Pin number of digital pin connected to the tip of the Jack connector
    uint8_t dpin_ring; // Pin number of digital pin connected to the ring of the Jack connector
    uint8_t apin_tip; // Pin number of analog pin connected to the tip of the Jack connector
    uint8_t apin_ring; // Pin number of analog pin connected to the ring of the Jack connector
    bool pull_up; // Do the inputs need the internal pullup resiostors activated
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
    uint16_t expr_ped_prev_value;
    uint16_t expr_ped_min; // Minimal value the expression pedal has reached
    uint16_t expr_ped_max; // maximum value the expression pedal has reached
};

SwitchExt::SwitchExt(uint8_t dpin_tip, uint8_t dpin_ring, uint8_t apin_tip, uint8_t apin_ring, uint32_t debounceTime, uint8_t switch_type, bool pull_up) {
  this->dpin_tip = dpin_tip;
  this->dpin_ring = dpin_ring;
  this->apin_tip = apin_tip;
  this->apin_ring = apin_ring;
  newState = 0;
  State = 0;
  releasedState = 0;
  do_not_check_ring = false;
  this->debounceTime = debounceTime;
  this->switch_type = switch_type;
  this->pull_up = pull_up;
  this->updateTime = DEFAULT_UPDATE_TIME;
  this->expr_ped_max = 0;
  this->expr_ped_min = 1023;
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
  //DEBUGMSG("Ring value: " + String(ring_value));
  if ((ring_value > 100) && (ring_value < 800)) {
    switch_type = EXPRESSION_PEDAL;
    return;
  }

  // Then check if it is normally open or normally closed by checking the digital tip pin
  if (digitalRead(dpin_tip) == HIGH) {
    switch_type = SWITCH_NORMALLY_OPEN;
    if (digitalRead(dpin_ring) == LOW) do_not_check_ring = true; // Exception for mono plug
  }
  else {
    switch_type = SWITCH_NORMALLY_CLOSED;
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
  if (new_state < expr_ped_min) expr_ped_min = new_state;
  if (new_state > expr_ped_max) expr_ped_max = new_state;

  uint16_t new_value = map(new_state, expr_ped_min, expr_ped_max, 0, 137); // Map to 137 instead of 127, so we have five on either side, so we always reach min and max value.
  if (new_value > 132) new_value = 132;
  if (new_value < 5) new_value = 5;
  if (((new_value < expr_ped_prev_value - 1) || (new_value > expr_ped_prev_value + 1)) && (expr_ped_max - expr_ped_min > 100)) { // check for calibration
    expr_ped_prev_value = new_value;
    expr_value = new_value - 5; // Set the expression value variable
    State = 1; // we will also return state as one, to simulate pressing a button
    return true;
  }
  return false;
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

#endif

