// Please read VController_v3.ino for information about the license and authors

#ifndef SWITCHPAD_H
#define SWITCHPAD_H

#include "debug.h"

// Library designed for guitar switches in a keypad style
// Based on a keypad library.
// This library uses interrupts for every switch column so in idle state it takes almost no time to run.
// When a switch is pressed, the column of that switch will be scanned in one go.

// In idle state all the row pins are set low and the column pins are set to input
// When a button is pressed the interrupt of that column is triggered and the SwitchPadColumnChanged variable is changed
// On update() and after an interrupt the switch column is debounced and then read.
// Now all rows become inputs and the triggered column becomes an output and is set low.
// By checking the row pins we identify the pressed switch (which still is pressed)
// The keypad returns to idle state  

// Needs a global variable for interrupt routine
uint8_t SwitchPadStatusColumnChanged = 0;

// And it needs a global function for the interrupts
void SwitchPadColumn1InterruptRoutine() {
  SwitchPadStatusColumnChanged |= 1;
  //DEBUGMSG("Interrupt 1...");
}

void SwitchPadColumn2InterruptRoutine() {
  SwitchPadStatusColumnChanged |= 2;
  //DEBUGMSG("Interrupt 2...");
}

void SwitchPadColumn3InterruptRoutine() {
  SwitchPadStatusColumnChanged |= 4;
  //DEBUGMSG("Interrupt 3...");
}

void SwitchPadColumn4InterruptRoutine() {
  SwitchPadStatusColumnChanged |= 8;
  //DEBUGMSG("Interrupt 4...");
}

void SwitchPadColumn5InterruptRoutine() {
  SwitchPadStatusColumnChanged |= 16;
}

void SwitchPadColumn6InterruptRoutine() {
  SwitchPadStatusColumnChanged |= 32;
}

void SwitchPadColumn7InterruptRoutine() {
  SwitchPadStatusColumnChanged |= 64;
}

void SwitchPadColumn8InterruptRoutine() {
  SwitchPadStatusColumnChanged |= 128;
}


class Switchpad
{
  public:
    Switchpad(uint8_t *rowPins, uint8_t *colPins, uint8_t numRows, uint8_t numCols, uint32_t debounceTime);

    void init();
    bool update();
    bool readColumn(uint8_t Col);
    uint8_t pressed();
    uint8_t released();

  private:
    void attachInterrupts();
    void idleState();
    uint8_t findFirstTriggeredColumn();
    uint8_t numRows;
    uint8_t numCols;
    uint8_t *rowPins;
    uint8_t *columnPins;
    bool switchState[32]; // Support for max 32 switches
    uint8_t switchPressed;
    uint8_t switchReleased;
    uint32_t bounceDelay;
    uint32_t debounceTime;
};

Switchpad::Switchpad(uint8_t *rowPins, uint8_t *colPins, uint8_t numRows, uint8_t numCols,  uint32_t debounceTime) {
  this->rowPins = rowPins;
  this->columnPins = colPins;
  this->numRows = numRows;
  this->numCols = numCols;
  this->debounceTime = debounceTime;
  
  for(uint8_t s = 0; s < numRows * numCols; s++) switchState[s] = HIGH;
  switchPressed = 0;
  switchReleased = 0;
  
}

void Switchpad::init() {
  // Initialize interrupts
  for (uint8_t c = 0; c < numCols; c++) { // And all columns inputs
    pinMode(columnPins[c], INPUT_PULLUP);
    //DEBUGMSG("Column " + String(c) + "is input");
    //digitalWrite(columnPins[c], LOW);
  }
  idleState(); // Will enable all rows, to wait for an interrupt
  
  attachInterrupts();
  SwitchPadStatusColumnChanged = 0;
  bounceDelay = millis();
}

void Switchpad::attachInterrupts() {
  if (numCols >= 1) attachInterrupt(digitalPinToInterrupt(columnPins[0]), SwitchPadColumn1InterruptRoutine, CHANGE); // Interrupt will trigger on state change.
  if (numCols >= 2) attachInterrupt(digitalPinToInterrupt(columnPins[1]), SwitchPadColumn2InterruptRoutine, CHANGE); // Interrupt will trigger on state change.
  if (numCols >= 3) attachInterrupt(digitalPinToInterrupt(columnPins[2]), SwitchPadColumn3InterruptRoutine, CHANGE); // Interrupt will trigger on state change.
  if (numCols >= 4) attachInterrupt(digitalPinToInterrupt(columnPins[3]), SwitchPadColumn4InterruptRoutine, CHANGE); // Interrupt will trigger on state change.
  if (numCols >= 5) attachInterrupt(digitalPinToInterrupt(columnPins[4]), SwitchPadColumn5InterruptRoutine, CHANGE); // Interrupt will trigger on state change.
  if (numCols >= 6) attachInterrupt(digitalPinToInterrupt(columnPins[5]), SwitchPadColumn6InterruptRoutine, CHANGE); // Interrupt will trigger on state change.
  if (numCols >= 7) attachInterrupt(digitalPinToInterrupt(columnPins[6]), SwitchPadColumn7InterruptRoutine, CHANGE); // Interrupt will trigger on state change.
  if (numCols >= 8) attachInterrupt(digitalPinToInterrupt(columnPins[7]), SwitchPadColumn8InterruptRoutine, CHANGE); // Interrupt will trigger on state change.
  //delay(1); // Wait for the interrupt pin states to settle...
}

void Switchpad::idleState() { // Will set all switchrows LOW (active), so any switch state change will trigger an interrupt
  for (uint8_t r = 0; r < numRows; r++) { // Make all rows outputs
    pinMode(rowPins[r], OUTPUT);
    digitalWrite(rowPins[r], LOW);
    //DEBUGMSG("Row " + String(r) + "is output and low");
  }
  //delay(1); // Wait for the interrupt to settle...
  //SwitchPadStatusColumnChanged = 0;
}

uint8_t Switchpad::findFirstTriggeredColumn() {
  uint8_t bitMask = 1;
  for (uint8_t c = 0; c < 8; c++) {
    if ((SwitchPadStatusColumnChanged & bitMask) > 0) return c;
    bitMask *= 2;
  }
  return 0;
}

bool Switchpad::update() {
  if ((SwitchPadStatusColumnChanged > 0) && (millis() > bounceDelay + debounceTime)) {

    uint8_t c = findFirstTriggeredColumn();
    uint8_t switchStateChanged; 
    
    //Make all rows input
    for (uint8_t r = 0; r < numRows; r++) {
      digitalWrite(rowPins[r], HIGH);
      pinMode(rowPins[r], INPUT_PULLUP);
      //DEBUGMSG("Row " + String(r) + "is input");
    }

    // Now enable the column where we received the interrupt
    // Make this column active by making it an output and turning it low
    switchStateChanged = 0;
    pinMode(columnPins[c], OUTPUT);
    digitalWrite(columnPins[c], LOW);
    //DEBUGMSG("Column " + String(c) + "is output and low");
    //delay(1);

    // Now read the row pins for this column
    uint8_t numberOfSwitchesChanged = 0;
    for (uint8_t r = 0; r < numRows; r++) {
      uint8_t switchIndex = (r * numCols) + c;
      bool newState = digitalRead(rowPins[r]);
      //DEBUGMSG("Row " + String(r) + " of column " + String(c) + " has state " + String(newState) + " and was " + String(switchState[switchIndex]));
      if (newState != switchState[switchIndex]) {
        switchStateChanged = switchIndex + 1;
        switchState[switchIndex] = newState;
        numberOfSwitchesChanged++;
      }
    }

    // Turn off the column by making it an input
    digitalWrite(columnPins[c], HIGH);
    pinMode(columnPins[c], INPUT_PULLUP);
    //DEBUGMSG("Column " + String(c) + "is input");

    //Reset to interrupt state
    idleState(); // Will enable all columns, to wait for an interrupt

    // Reattach the interrupts for the port that has been an output
    attachInterrupts();

    // Reset bounce delay timer
    bounceDelay = millis();
    if (numberOfSwitchesChanged <= 1) bitClear(SwitchPadStatusColumnChanged, c);
    
    if (switchStateChanged > 0) {
      if (switchState[switchStateChanged - 1] == LOW) {
        switchPressed = switchStateChanged; //Remember the last state when we go back to zero
        switchReleased = 0;
        DEBUGMSG("Switchpad swith PRESSED " + String(switchStateChanged));
      }
      else {
        switchReleased = switchStateChanged;
        switchPressed = 0;
        DEBUGMSG("Switchpad swith RELEASED " + String(switchStateChanged));
      }
      return true;
    }
  }

  return false; // No key pressed or released, so return false
}

uint8_t Switchpad::pressed() {
  return switchPressed;
}

uint8_t Switchpad::released() {
  return switchReleased;
}

#endif
