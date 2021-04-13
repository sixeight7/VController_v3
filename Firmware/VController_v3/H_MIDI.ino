// Please read VController_v3.ino for information about the license and authors

// This page has the following parts:
// Section 1: MIDI Port Initialization
// Section 2: MIDI In Functions
// Section 3: MIDI Out Functions
// Section 4: MIDI Functions for Device Detection
// Section 5: MIDI PC and CC Ledger
// Section 6: MIDI Editor Communication
// Section 7: MIDI switch command reading

// ********************************* Section 1: MIDI Port Initialization ********************************************

// Setup of usbMIDI and Serial midi in and out
// * midi ports are setup and the sysex buffer size is custom set to 256.
//   In order for the Serial MIDI ports to receive larger messages the following files have to be edited:
//   Win: C:\Program Files (x86)\Arduino\hardware\teensy\avr\cores\teensy3\serial1.c
//   Mac: /Applications/Arduino.app/Contents/Java/hardware/teensy/avr/cores/teensy3/serial1.c
//   Change the number 64 to 255 on the following line:
//   #define SERIAL1_RX_BUFFER_SIZE 255
//   Do the same thing for serial2.c and serial3.c
// * the connection/disconnection of devices is triggered from here.
// * the midi ports are read here and the devices are called after receiving PC, CC or sysex messages
// * the basic procedures for sending PC, CC and sysex messages to a specific midi port are specified here
// * the control of virtual ports via VCbridge on a raspberry pi is controlled from here. A virtual port is selected
//   with CC #119 on MIDI channel 16. Both the VController and VCbridge will send and receive this message to select the port.
// * A MIDI port consists of two parts. First hexadecimal digit is the real port number: 0x00 = USB_MIDI, 0x10 is serial port 1, 0x20 is serial port 2, etc.
//   The second nibble is the virtual port number from VCbridge: 0x10 is all virtual ports, 0x11 is virtual port 1, 0x12 is virtual port 2,etc
//   The USB host port also supports up to eight virtual ports. Via a USB hub eight USB MIDI devices can be connected. Virtual port 0 will send the message to all connected USB devices.

// Online list of manufacturers MIDI codes: https://github.com/strymon/spl/blob/master/lib/DcMidi/DcMidiIdent.h

#include <MIDI.h>

// Midi IDs for sysex messages of the VController
#define VC_MANUFACTURING_ID 0x7D // Universal for simple midi device
#define VC_FAMILY_CODE 0x68      // Family code for Sixeight's products
#ifndef IS_VCMINI
#define VC_MODEL_NUMBER 0x01     // The product code for the VController
#else
#define VC_MODEL_NUMBER 0x02     // The product code for the VC-mini
#endif
#define VC_DEVICE_ID 0x01        // The device id of the VController

// Midi sysex commands of the V-Controller
#define VC_REMOTE_CONTROL_ENABLE 1
#define VC_REMOTE_CONTROL_SWITCH_PRESSED 2
#define VC_REMOTE_CONTROL_SWITCH_RELEASED 3
#define VC_REMOTE_UPDATE_DISPLAY 4
#define VC_REMOTE_UPDATE_LEDS 5
#define VC_REQUEST_ALL_SETTINGS 6
#define VC_SET_GENERAL_SETTINGS 7
#define VC_SET_DEVICE_SETTINGS 8
#define VC_SET_MIDI_SWITCH_SETTINGS 13
#define VC_SAVE_SETTINGS 16
#define VC_REQUEST_COMMANDS_DUMP 9
#define VC_START_COMMANDS_DUMP 10
#define VC_SET_COMMAND 11
#define VC_FINISH_COMMANDS_DUMP 12
#define VC_REQUEST_DEVICE_PATCHES 14
#define VC_SET_DEVICE_PATCH 15
#define VC_SAVE_SETTINGS 16
#define VC_FINISH_DEVICE_PATCH_DUMP 17
#define VC_REQUEST_SEQ_PATTERNS 18
#define VC_SET_SEQ_PATTERN 19


#define CHECK4DEVICES_TIMER_LENGTH 250 // Check every second which Roland devices are connected
#define MAX_TIMES_NO_RESPONSE 4 // The number of times the device may not respond before a disconnect
unsigned long Check4DevicesTimer = 0;
bool device_check_enabled = true; // This check should not occur during page sysex reading.

uint8_t Current_MIDI_port; // The MIDI port that is being read.
uint8_t VCbridge_in_port[NUMBER_OF_MIDI_PORTS];
uint8_t VCbridge_out_port[NUMBER_OF_MIDI_PORTS];
bool bank_selection_active = false;

bool remote_control_active = false;
uint8_t remote_control_port;
uint16_t editor_dump_size = 0;

// Setup MIDI ports. The USB port is set from the Arduino menu.
struct MySettings : public midi::DefaultSettings
{
  //static const bool UseRunningStatus = false; // Messes with my old equipment!
  //static const bool Use1ByteParsing = false; // More focus on reading messages
  static const unsigned SysExMaxSize = 650; // Change sysex buffersize - Zoom devices send packets up to 146 bytes, GMajor up to 615
};

MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial1, MIDI1, MySettings); // Enables serial1 port for MIDI communication with custom settings
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial2, MIDI2, MySettings); // Enables serial2 port for MIDI communication with custom settings
#ifdef MIDI3_ENABLED
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial3, MIDI3, MySettings); // Enables serial3 port for MIDI communication with custom settings
#endif

#ifdef MIDI_T36_HOST_PORT_ENABLED
#include <USBHost_t36.h>

USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
USBHub hub3(myusb);
USBHub hub4(myusb);

#define NUMBER_OF_USB_HOST_MIDI_PORTS 8
MIDIDevice_BigBuffer usbhMIDI01(myusb); // With bigbuffer, newer Roland devices are working properly!
MIDIDevice_BigBuffer usbhMIDI02(myusb);
MIDIDevice_BigBuffer usbhMIDI03(myusb);
MIDIDevice_BigBuffer usbhMIDI04(myusb);
MIDIDevice_BigBuffer usbhMIDI05(myusb);
MIDIDevice_BigBuffer usbhMIDI06(myusb);
MIDIDevice_BigBuffer usbhMIDI07(myusb);
MIDIDevice_BigBuffer usbhMIDI08(myusb);
MIDIDevice_BigBuffer * usbhMIDI[NUMBER_OF_USB_HOST_MIDI_PORTS] = {
  &usbhMIDI01, &usbhMIDI02, &usbhMIDI03, &usbhMIDI04, &usbhMIDI05, &usbhMIDI06, &usbhMIDI07, &usbhMIDI08
};
#endif

void setup_MIDI_common()
{
  for (uint8_t i = 0; i < NUMBER_OF_MIDI_PORTS; i++) {
    VCbridge_in_port[i] = 0;
    VCbridge_out_port[i] = 0;
  }
  usbMIDI.setHandleNoteOff(OnNoteOff);
  usbMIDI.setHandleNoteOn(OnNoteOn) ;
  usbMIDI.setHandleProgramChange(OnProgramChange);
  usbMIDI.setHandleControlChange(OnControlChange);
  usbMIDI.setHandleSysEx(OnSysEx);
  usbMIDI.setHandleActiveSensing(OnActiveSense);
  usbMIDI.setHandleClock(OnClock);

  //pinMode(0, INPUT_PULLUP); //Add the internal pullup resistor to pin 0 (Rx)
  delay(100);
  MIDI1.begin(MIDI_CHANNEL_OMNI);
  MIDI1.turnThruOff();
  MIDI1.setHandleNoteOff(OnNoteOff);
  MIDI1.setHandleNoteOn(OnNoteOn) ;
  MIDI1.setHandleProgramChange(OnProgramChange);
  MIDI1.setHandleControlChange(OnControlChange);
  MIDI1.setHandleSystemExclusive(OnSerialSysEx);
  MIDI1.setHandleActiveSensing(OnActiveSense);
  MIDI1.setHandleClock(OnClock);

  delay(100);
  MIDI2.begin(MIDI_CHANNEL_OMNI);
  MIDI2.turnThruOff();
  MIDI2.setHandleNoteOff(OnNoteOff);
  MIDI2.setHandleNoteOn(OnNoteOn) ;
  MIDI2.setHandleProgramChange(OnProgramChange);
  MIDI2.setHandleControlChange(OnControlChange);
  MIDI2.setHandleSystemExclusive(OnSerialSysEx);
  MIDI2.setHandleActiveSensing(OnActiveSense);
  MIDI2.setHandleClock(OnClock);

#ifdef MIDI3_ENABLED
  delay(100);
  MIDI3.begin(MIDI_CHANNEL_OMNI);
  MIDI3.turnThruOff();
  MIDI3.setHandleNoteOff(OnNoteOff);
  MIDI3.setHandleNoteOn(OnNoteOn) ;
  MIDI3.setHandleProgramChange(OnProgramChange);
  MIDI3.setHandleControlChange(OnControlChange);
  MIDI3.setHandleSystemExclusive(OnSerialSysEx);
  MIDI3.setHandleActiveSensing(OnActiveSense);
  MIDI3.setHandleClock(OnClock);
#endif

#ifdef MIDI_T36_HOST_PORT_ENABLED
  myusb.begin();
  for (uint8_t p = 0; p < NUMBER_OF_USB_HOST_MIDI_PORTS; p++) {
    usbhMIDI[p]->setHandleNoteOff(OnNoteOff);
    usbhMIDI[p]->setHandleNoteOn(OnNoteOn) ;
    usbhMIDI[p]->setHandleProgramChange(OnProgramChange);
    usbhMIDI[p]->setHandleControlChange(OnControlChange);
    usbhMIDI[p]->setHandleSysEx(OnSysEx);
    usbhMIDI[p]->setHandleActiveSensing(OnActiveSense);
    usbhMIDI[p]->setHandleClock(OnClock);
  }
#endif
}

void main_MIDI_common()
{
  Current_MIDI_port = USBMIDI_PORT;
  usbMIDI.read();
  Current_MIDI_port = MIDI1_PORT;
  MIDI1.read();
  Current_MIDI_port = MIDI2_PORT;
  MIDI2.read();
#ifdef MIDI3_ENABLED
  Current_MIDI_port = MIDI3_PORT;
  MIDI3.read();
#endif
#ifdef MIDI_T36_HOST_PORT_ENABLED
  Current_MIDI_port = USBHMIDI_PORT;
  myusb.Task();
  for (uint8_t p = 0; p < NUMBER_OF_USB_HOST_MIDI_PORTS; p++) {
    VCbridge_in_port[USBHMIDI_PORT >> 4] = p + 1;
    usbhMIDI[p]->read();
  }
#endif

  MIDI_check_for_devices();  // Check actively if any devices are out there
  PAGE_check_sysex_watchdog(); // check if the watchdog has not expired
}

void MIDI_check_USBHMIDI()
{
#ifdef MIDI_T36_HOST_PORT_ENABLED
  Current_MIDI_port = USBHMIDI_PORT;
  myusb.Task();
  for (uint8_t p = 0; p < NUMBER_OF_USB_HOST_MIDI_PORTS; p++) {
    VCbridge_in_port[USBHMIDI_PORT >> 4] = p;
    usbhMIDI[p]->read();
  }
#endif
}

// ********************************* Section 2: MIDI In Functions ********************************************

void OnNoteOn(byte channel, byte note, byte velocity)
{
  uint8_t VCbridge_index = Current_MIDI_port >> 4;
  DEBUGMIDI("NoteOn #" + String(note) + " with velocity " + String(velocity) + " received on channel " + String(channel) + " and port " + String(Current_MIDI_port >> 4) + ':' + String(VCbridge_in_port[VCbridge_index])); // Show on serial debug screen
  My_SY1000.check_note_in(note, velocity, channel, Current_MIDI_port | VCbridge_in_port[VCbridge_index]);
  SCO_bass_mode_note_on(note, velocity, channel, Current_MIDI_port | VCbridge_in_port[VCbridge_index]);
}

void OnNoteOff(byte channel, byte note, byte velocity)
{
  uint8_t VCbridge_index = Current_MIDI_port >> 4;
  DEBUGMIDI("NoteOff #" + String(note) + " with velocity " + String(velocity) + " received on channel " + String(channel) + " and port " + String(Current_MIDI_port >> 4) + ':' + String(VCbridge_in_port[VCbridge_index])); // Show on serial debug screen
  My_SY1000.check_note_in(note, 0, channel, Current_MIDI_port | VCbridge_in_port[VCbridge_index]);
  SCO_bass_mode_note_off(note, velocity, channel, Current_MIDI_port | VCbridge_in_port[VCbridge_index]);
}

void OnProgramChange(byte channel, byte program)
{
  uint8_t VCbridge_index = Current_MIDI_port >> 4;
  DEBUGMIDI("PC #" + String(program) + " received on channel " + String(channel) + " and port " + String(Current_MIDI_port >> 4) + ':' + String(VCbridge_in_port[VCbridge_index])); // Show on serial debug screen
  for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
    Device[d]->check_PC_in(program, channel, Current_MIDI_port | VCbridge_in_port[VCbridge_index]);
    if (Current_MIDI_port == USBMIDI_PORT) Device[d]->forward_PC_message(program, channel);
  }
  MIDI_check_switch_pc(program, channel, Current_MIDI_port);
}

void OnControlChange(byte channel, byte control, byte value)
{
  uint8_t VCbridge_index = Current_MIDI_port >> 4;
  if (value & 0x80) value = 0; // Check for false values - seems to happen with the FCB1010


  if ((channel == VCONTROLLER_MIDI_CHANNEL) && (control == LINE_SELECT_CC_NUMBER)) {
    VCbridge_in_port[VCbridge_index] = value;
#ifdef DEBUG_SYSEX_UNIVERSAL
    DEBUGMIDI("Changed virtual input channel for port " + String(VCbridge_index) + " to: " + String(VCbridge_in_port[VCbridge_index]));
#endif
  }
  else {
    DEBUGMIDI("CC #" + String(control) + " Value:" + String(value) + " received on channel " + String(channel) + " and port " + String(Current_MIDI_port >> 4) + ':' + String(VCbridge_in_port[VCbridge_index])); // Show on serial debug screen
    for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
      Device[d]->check_CC_in(control, value, channel, Current_MIDI_port | VCbridge_in_port[VCbridge_index]);
    }
    MIDI_check_switch_cc(control, value, channel, Current_MIDI_port);
  }
}

void OnSysEx(const unsigned char* sxdata, short unsigned int sxlength, bool sx_comp)
{
  uint8_t VCbridge_index = Current_MIDI_port >> 4;
  //MIDI1.sendSysEx(sxlength, sxdata); // MIDI through usb to serial
  //MIDI2.sendSysEx(sxlength, sxdata); // MIDI through usb to serial
  MIDI_debug_sysex(sxdata, sxlength, Current_MIDI_port, false);

  if (sxdata[1] == 0x7E) { //Check if it is a Universal Non-Real Time message
    MIDI_check_SYSEX_in_universal(sxdata, sxlength, Current_MIDI_port | VCbridge_in_port[VCbridge_index]);
  }
  else {
    for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
      Device[d]->check_SYSEX_in(sxdata, sxlength, Current_MIDI_port | VCbridge_in_port[VCbridge_index]);
    }
    MIDI_check_SYSEX_in_editor(sxdata, sxlength, Current_MIDI_port | VCbridge_in_port[VCbridge_index]);
  }

  // Midi forwarding
  if (Current_MIDI_port == USBMIDI_PORT) {
    for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
      Device[d]->forward_MIDI_message(sxdata, sxlength);
    }
  }
}


void OnSerialSysEx(byte *sxdata, unsigned sxlength)
{ uint8_t VCbridge_index = Current_MIDI_port >> 4;
  //usbMIDI.sendSysEx(sxlength, sxdata); // MIDI through serial to usb
  MIDI_debug_sysex(sxdata, sxlength, Current_MIDI_port | VCbridge_in_port[VCbridge_index], false);

  if (sxdata[1] == 0x7E) { //Check if it is a Universal Non-Real Time message
    MIDI_check_SYSEX_in_universal(sxdata, sxlength, Current_MIDI_port | VCbridge_in_port[VCbridge_index]);
  }
  else {
    for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
      Device[d]->check_SYSEX_in(sxdata, sxlength, Current_MIDI_port | VCbridge_in_port[VCbridge_index]);
    }
    MIDI_check_SYSEX_in_editor(sxdata, sxlength, Current_MIDI_port | VCbridge_in_port[VCbridge_index]);
  }
}

void OnActiveSense() {
  uint8_t VCbridge_index = Current_MIDI_port >> 4;
  for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
    Device[d]->check_active_sense_in(Current_MIDI_port | VCbridge_in_port[VCbridge_index]);
  }
}

void OnClock() {
  if (Setting.Read_MIDI_clock_port == 0) return;
  uint8_t p = MIDI_set_port_number_from_menu(Setting.Read_MIDI_clock_port - 1);
  if ((p != Current_MIDI_port) && (p != ALL_MIDI_PORTS)) return;

  uint8_t VCbridge_index = Current_MIDI_port >> 4;
  SCO_receive_MIDI_clock_pulse(Current_MIDI_port | VCbridge_in_port[VCbridge_index]);

  // Forward to midi2
  //MIDI2.sendRealTime(midi::Clock);
}

// ********************************* Section 3: MIDI Out Functions ********************************************

// Send Program Change message
void MIDI_send_PC(uint8_t Program, uint8_t Channel, uint8_t Port) {
  DEBUGMIDI("PC #" + String(Program) + " sent on channel " + String(Channel) + " and port " + String(Port >> 4) + ':' + String(Port & 0x0F)); // Show on serial debug screen
  MIDI_check_port_message(Port);
  if ((Port & 0xF0) == USBMIDI_PORT) usbMIDI.sendProgramChange(Program, Channel);
  if ((Port & 0xF0) == MIDI1_PORT) MIDI1.sendProgramChange(Program, Channel);
  if ((Port & 0xF0) == MIDI2_PORT) MIDI2.sendProgramChange(Program, Channel);
#ifdef MIDI3_ENABLED
  if ((Port & 0xF0) == MIDI3_PORT) MIDI3.sendProgramChange(Program, Channel);
#endif
#ifdef MIDI_T36_HOST_PORT_ENABLED
  if ((Port & 0xF0) == USBHMIDI_PORT) {
    if ((Port & 0x0F) == 0) { // Send message to all USB host ports
      for (uint8_t p = 0; p < NUMBER_OF_USB_HOST_MIDI_PORTS; p++) {
        usbhMIDI[p]->sendProgramChange(Program, Channel);
      }
    }
    else {
      usbhMIDI[(Port & 0x0F) - 1]->sendProgramChange(Program, Channel);
    }
  }
#endif

  if ((Port & 0xF0) == ALL_MIDI_PORTS) {
    usbMIDI.sendProgramChange(Program, Channel);
    MIDI1.sendProgramChange(Program, Channel);
    MIDI2.sendProgramChange(Program, Channel);
#ifdef MIDI3_ENABLED
    MIDI3.sendProgramChange(Program, Channel);
#endif
#ifdef MIDI_T36_HOST_PORT_ENABLED
    for (uint8_t p = 0; p < NUMBER_OF_USB_HOST_MIDI_PORTS; p++) {
      usbhMIDI[p]->sendProgramChange(Program, Channel);
    }
#endif
  }
}

void MIDI_forward_PC_to_all_ports_but_mine(uint8_t Program, uint8_t Channel, uint8_t Exclude_port) {
  for (uint8_t p = 0; p < NUMBER_OF_MIDI_PORTS; p++) {
    if (MIDI_set_port_number_from_menu(p) != Exclude_port) {
      MIDI_send_PC(Program, Channel, MIDI_set_port_number_from_menu(p));
    }
  }
}

void MIDI_send_CC(uint8_t Controller, uint8_t Value, uint8_t Channel, uint8_t Port) {
  DEBUGMIDI("CC #" + String(Controller) + " with value " + String(Value) + " sent on channel " + String(Channel) + " and port " + String(Port >> 4) + ':' + String(Port & 0x0F)); // Show on serial debug screen
  MIDI_check_port_message(Port);
  if ((Port & 0xF0) == USBMIDI_PORT) usbMIDI.sendControlChange(Controller, Value, Channel);
  if ((Port & 0xF0) == MIDI1_PORT) MIDI1.sendControlChange(Controller, Value, Channel);
  if ((Port & 0xF0) == MIDI2_PORT) MIDI2.sendControlChange(Controller, Value, Channel);
#ifdef MIDI3_ENABLED
  if ((Port & 0xF0) == MIDI3_PORT) MIDI3.sendControlChange(Controller, Value, Channel);
#endif
#ifdef MIDI_T36_HOST_PORT_ENABLED
  if ((Port & 0xF0) == USBHMIDI_PORT) {
    if ((Port & 0x0F) == 0) { // Send message to all USB host ports
      for (uint8_t p = 0; p < NUMBER_OF_USB_HOST_MIDI_PORTS; p++) {
        usbhMIDI[p]->sendControlChange(Controller, Value, Channel);
      }
    }
    else {
      usbhMIDI[(Port & 0x0F) - 1]->sendControlChange(Controller, Value, Channel);
    }
  }
#endif
  if ((Port & 0xF0) == ALL_MIDI_PORTS) {
    usbMIDI.sendControlChange(Controller, Value, Channel);
    MIDI1.sendControlChange(Controller, Value, Channel);
    MIDI2.sendControlChange(Controller, Value, Channel);
#ifdef MIDI3_ENABLED
    MIDI3.sendControlChange(Controller, Value, Channel);
#endif
#ifdef MIDI_T36_HOST_PORT_ENABLED
    for (uint8_t p = 0; p < NUMBER_OF_USB_HOST_MIDI_PORTS; p++) {
      usbhMIDI[p]->sendControlChange(Controller, Value, Channel);
    }
#endif
  }
}

void MIDI_forward_CC_to_all_ports_but_mine(uint8_t Controller, uint8_t Value, uint8_t Channel, uint8_t Exclude_port) {
  for (uint8_t p = 0; p < NUMBER_OF_MIDI_PORTS; p++) {
    if (MIDI_set_port_number_from_menu(p) != Exclude_port) {
      MIDI_send_CC(Controller, Value, Channel, MIDI_set_port_number_from_menu(p));

    }
  }
}

void MIDI_send_note_on(uint8_t Note, uint8_t Velocity, uint8_t Channel, uint8_t Port) {
  DEBUGMIDI("NoteOn #" + String(Note) + " with velocity " + String(Velocity) + " sent on channel " + String(Channel) + " and port " + String(Port >> 4) + ':' + String(Port & 0x0F)); // Show on serial debug screen
  MIDI_check_port_message(Port);
  if ((Port & 0xF0) == USBMIDI_PORT) usbMIDI.sendNoteOn(Note, Velocity, Channel);
  if ((Port & 0xF0) == MIDI1_PORT) MIDI1.sendNoteOn(Note, Velocity, Channel);
  if ((Port & 0xF0) == MIDI2_PORT) MIDI2.sendNoteOn(Note, Velocity, Channel);
#ifdef MIDI3_ENABLED
  if ((Port & 0xF0) == MIDI3_PORT) MIDI3.sendNoteOn(Note, Velocity, Channel);
#endif
#ifdef MIDI_T36_HOST_PORT_ENABLED
  if ((Port & 0xF0) == USBHMIDI_PORT) {
    if ((Port & 0x0F) == 0) { // Send message to all USB host ports
      for (uint8_t p = 0; p < NUMBER_OF_USB_HOST_MIDI_PORTS; p++) {
        usbhMIDI[p]->sendNoteOn(Note, Velocity, Channel);
      }
    }
    else {
      usbhMIDI[(Port & 0x0F) - 1]->sendNoteOn(Note, Velocity, Channel);
    }
  }
#endif
  if ((Port & 0xF0) == ALL_MIDI_PORTS) {
    usbMIDI.sendNoteOn(Note, Velocity, Channel);
    MIDI1.sendNoteOn(Note, Velocity, Channel);
    MIDI2.sendNoteOn(Note, Velocity, Channel);
#ifdef MIDI3_ENABLED
    MIDI3.sendNoteOn(Note, Velocity, Channel);
#endif
#ifdef MIDI_T36_HOST_PORT_ENABLED
    for (uint8_t p = 0; p < NUMBER_OF_USB_HOST_MIDI_PORTS; p++) {
      usbhMIDI[p]->sendNoteOn(Note, Velocity, Channel);
    }
#endif
  }
}

void  MIDI_send_note_off(uint8_t Note, uint8_t Velocity, uint8_t Channel, uint8_t Port) {
  DEBUGMIDI("NoteOff #" + String(Note) + " with velocity " + String(Velocity) + " sent on channel " + String(Channel) + " and port " + String(Port >> 4) + ':' + String(Port & 0x0F)); // Show on serial debug screen
  MIDI_check_port_message(Port);
  if ((Port & 0xF0) == USBMIDI_PORT) usbMIDI.sendNoteOff(Note, Velocity, Channel);
  if ((Port & 0xF0) == MIDI1_PORT) MIDI1.sendNoteOff(Note, Velocity, Channel);
  if ((Port & 0xF0) == MIDI2_PORT) MIDI2.sendNoteOff(Note, Velocity, Channel);
#ifdef MIDI3_ENABLED
  if ((Port & 0xF0) == MIDI3_PORT) MIDI3.sendNoteOff(Note, Velocity, Channel);
#endif
#ifdef MIDI_T36_HOST_PORT_ENABLED
  if ((Port & 0xF0) == USBHMIDI_PORT) {
    if ((Port & 0x0F) == 0) { // Send message to all USB host ports
      for (uint8_t p = 0; p < NUMBER_OF_USB_HOST_MIDI_PORTS; p++) {
        usbhMIDI[p]->sendNoteOff(Note, Velocity, Channel);
      }
    }
    else {
      usbhMIDI[(Port & 0x0F) - 1]->sendNoteOff(Note, Velocity, Channel);
    }
  }
#endif
  if ((Port & 0xF0) == ALL_MIDI_PORTS) {
    usbMIDI.sendNoteOff(Note, Velocity, Channel);
    MIDI1.sendNoteOff(Note, Velocity, Channel);
    MIDI2.sendNoteOff(Note, Velocity, Channel);
#ifdef MIDI3_ENABLED
    MIDI3.sendNoteOff(Note, Velocity, Channel);
#endif
#ifdef MIDI_T36_HOST_PORT_ENABLED
    for (uint8_t p = 0; p < NUMBER_OF_USB_HOST_MIDI_PORTS; p++) {
      usbhMIDI[p]->sendNoteOff(Note, Velocity, Channel);
    }
#endif
  }
}

void MIDI_send_sysex(const unsigned char* sxdata, short unsigned int sxlength, uint8_t Port, uint8_t cable = 0) {
  MIDI_check_port_message(Port);
  switch (Port & 0xF0) {
    case USBMIDI_PORT:
#if defined(ARDUINO) && ARDUINO >= 10800
      usbMIDI.sendSysEx(sxlength - 2, &sxdata[1]);
#else
      usbMIDI.sendSysEx(sxlength, sxdata);
#endif
      break;
    case MIDI1_PORT:
      MIDI1.sendSysEx(sxlength - 2, &sxdata[1]); // Serial Midi library adds oxF0 and oxF7 sysex header itself, so we leave them out
      break;
    case MIDI2_PORT:
      MIDI2.sendSysEx(sxlength - 2, &sxdata[1]); // Otherwise they will be send twice.
      break;
#ifdef MIDI3_ENABLED
    case MIDI3_PORT:
      MIDI3.sendSysEx(sxlength - 2, &sxdata[1]);
      break;
#endif
#ifdef MIDI_T36_HOST_PORT_ENABLED
    case USBHMIDI_PORT:
      if ((Port & 0x0F) == 0) { // Send message to all USB host ports
        for (uint8_t p = 0; p < NUMBER_OF_USB_HOST_MIDI_PORTS; p++) {
          usbhMIDI[p]->sendSysEx(sxlength, sxdata, true, cable);  // hasterm = true, which means no extra 0xF0 and 0xF7 bytes will be added
        }
      }
      else {
        usbhMIDI[(Port & 0x0F) - 1]->sendSysEx(sxlength, sxdata, true, cable); // hasterm = true, which means no extra 0xF0 and 0xF7 bytes will be added
        //usbhMIDI[(Port & 0x0F) - 1]->send_now();
      }
      break;
#endif
    case ALL_MIDI_PORTS:
#if defined(ARDUINO) && ARDUINO >= 10800
      usbMIDI.sendSysEx(sxlength - 2, &sxdata[1]);
#else
      usbMIDI.sendSysEx(sxlength, sxdata);
#endif
      MIDI1.sendSysEx(sxlength - 2, &sxdata[1]);
      MIDI2.sendSysEx(sxlength - 2, &sxdata[1]);
#ifdef MIDI3_ENABLED
      MIDI3.sendSysEx(sxlength - 2, &sxdata[1]);
#endif
#ifdef MIDI_T36_HOST_PORT_ENABLED
      for (uint8_t p = 0; p < NUMBER_OF_USB_HOST_MIDI_PORTS; p++) {
        usbhMIDI[p]->sendSysEx(sxlength, sxdata, true, cable);  // hasterm = true, which means no extra 0xF0 and 0xF7 bytes will be added
      }
#endif
      break;
  }

  MIDI_debug_sysex(sxdata, sxlength, Port, true);
}

void MIDI_check_port_message(uint8_t Port) { // Check if we need to tell the VCbridge on the RPi what port to use

  uint8_t VCbridge_index = Port >> 4; // First nibble is MIDI port number
  uint8_t new_port_number = Port & 0x0F;

  if (VCbridge_index > NUMBER_OF_MIDI_PORTS) return;

  if (new_port_number != VCbridge_out_port[VCbridge_index]) {
    VCbridge_out_port[VCbridge_index] = new_port_number;
    //Send the new port number to the VCbridge software
    if ((Port & 0xF0) == USBMIDI_PORT) usbMIDI.sendControlChange(LINE_SELECT_CC_NUMBER, new_port_number, VCONTROLLER_MIDI_CHANNEL);
    if ((Port & 0xF0) == MIDI1_PORT) MIDI1.sendControlChange(LINE_SELECT_CC_NUMBER, new_port_number, VCONTROLLER_MIDI_CHANNEL);
    if ((Port & 0xF0) == MIDI2_PORT) MIDI2.sendControlChange(LINE_SELECT_CC_NUMBER, new_port_number, VCONTROLLER_MIDI_CHANNEL);
#ifdef MIDI3_ENABLED
    if ((Port & 0xF0) == MIDI3_PORT) MIDI3.sendControlChange(LINE_SELECT_CC_NUMBER, new_port_number, VCONTROLLER_MIDI_CHANNEL);
#endif
    if ((Port & 0xF0) != USBHMIDI_PORT) { // The USB host port does not need the CC messages to change port
      DEBUGMSG("!!!Changed virtual output channel for port " + String(VCbridge_index) + " to: " + String(new_port_number));
    }
  }

  if ((Port & 0xF0) == ALL_MIDI_PORTS) { // Call it recursively for all ports
    MIDI_check_port_message(USBMIDI_PORT | (Port & 0x0F));
    MIDI_check_port_message(MIDI1_PORT | (Port & 0x0F));
    MIDI_check_port_message(MIDI2_PORT | (Port & 0x0F));
#ifdef MIDI3_ENABLED
    MIDI_check_port_message(MIDI3_PORT | (Port & 0x0F));
#endif
    // The USB host port does not need these messages
  }
}

void MIDI_send_clock() {
  if (Setting.Send_MIDI_clock_port == 0) return;
  switch (MIDI_set_port_number_from_menu(Setting.Send_MIDI_clock_port - 1)) {
    case USBMIDI_PORT:
      usbMIDI.sendRealTime(usbMIDI.Clock);
      break;
    case MIDI1_PORT:
      MIDI1.sendRealTime(MIDI_NAMESPACE::Clock);
      break;
    case MIDI2_PORT:
      MIDI2.sendRealTime(MIDI_NAMESPACE::Clock);
      break;
    case MIDI3_PORT:
#ifdef MIDI3_ENABLED
      MIDI3.sendRealTime(MIDI_NAMESPACE::Clock);
#endif
      break;
    case USBHMIDI_PORT:
#ifdef MIDI_T36_HOST_PORT_ENABLED
      /*for (uint8_t p = 0; p < NUMBER_OF_USB_HOST_MIDI_PORTS; p++) {
        usbhMIDI[p]->sendRealTime(usbhMIDI[p]->Clock);
        }*/
#endif
      break;
    case ALL_MIDI_PORTS:
      usbMIDI.sendRealTime(usbMIDI.Clock);
      MIDI1.sendRealTime(MIDI_NAMESPACE::Clock);
      MIDI2.sendRealTime(MIDI_NAMESPACE::Clock);
#ifdef MIDI3_ENABLED
      MIDI3.sendRealTime(MIDI_NAMESPACE::Clock);
#endif
#ifdef MIDI_T36_HOST_PORT_ENABLED
      /*for (uint8_t p = 0; p < NUMBER_OF_USB_HOST_MIDI_PORTS; p++) {
        usbhMIDI[p]->sendRealTime(usbhMIDI[p]->Clock);
        }*/
#endif
      break;
  }
}

// ********************************* Section 4: MIDI Functions for Device Detection ********************************************

// Sysex for detecting MIDI devices
#define Anybody_out_there {0xF0, 0x7E, 0x7F, 0x06, 0x01, 0xF7}  // Ask all MIDI devices to respond with their Manufacturing ID, Device ID and version number

void MIDI_check_SYSEX_in_universal(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port) // Check for universal SYSEX message - identity reply
{
  // Check if it is an identity reply from a device
  // There is no check on the second byte (device ID), in case a device has a different device ID
  if ((sxdata[3] == 0x06) && (sxdata[4] == 0x02)) {
    for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
      Device[d]->identity_check(sxdata, sxlength, port);
    }
  }

  // Check for identity request
  // Format: 0xF0, 0x7E, 0x7F, 0x06, 0x01, 0xF7
  if ((sxdata[3] == 0x06) && (sxdata[4] == 0x01)) {
    // Check for matching device ID
    if ((sxdata[2] == VC_DEVICE_ID) || (sxdata[2] == 0x7F)) {
      uint8_t sysexbuffer[15] = {0xF0, 0x7E, VC_DEVICE_ID, 0x06, 0x02, VC_MANUFACTURING_ID, 0x00, VC_FAMILY_CODE, 0x00, VC_MODEL_NUMBER,
                                 VCONTROLLER_FIRMWARE_VERSION_MAJOR, VCONTROLLER_FIRMWARE_VERSION_MINOR, VCONTROLLER_FIRMWARE_VERSION_BUILD,
                                 0x00, 0xF7
                                };
      MIDI_send_sysex(sysexbuffer, 15, port); // Will send F0 7E 01 06 02 7D 00 68 00 02 (03 03 05) 00 F7 - version number between brackets
    }
    if ((port == USBMIDI_PORT) && (My_KTN.connected)) { // Send Katana response
      My_KTN.respond_to_identity_request_of_editor();
    }
  }
}

// check for devices on each MIDI port sequentially
// We send both a Universal Identity Request (supported by Boss/Roland, Zoom and Line6) and alternative requests (for AxeFX)
uint8_t check_device_no = 0;
void MIDI_check_for_devices()
{
  // Check if timer needs to be set
  if ((Check4DevicesTimer == 0)) {
    Check4DevicesTimer = millis();
  }

  // Check if timer runs out
  if (millis() - Check4DevicesTimer > CHECK4DEVICES_TIMER_LENGTH) {
    Check4DevicesTimer = millis(); // Reset the timer

    // Send Universal Identity Request message to all MIDI ports if device_check_enabled is true!!
    if (device_check_enabled) {
      uint8_t sysexbuffer[6] = Anybody_out_there;
      if (check_device_no == 0 ) {
        MIDI_send_sysex(sysexbuffer, 6, USBMIDI_PORT);
        MIDI_check_all_devices_still_connected();
      }
      if (check_device_no == 1 ) MIDI_send_sysex(sysexbuffer, 6, MIDI1_PORT);
      if (check_device_no == 2 ) MIDI_send_sysex(sysexbuffer, 6, MIDI2_PORT);
#ifdef MIDI3_ENABLED
      if (check_device_no == 3 ) MIDI_send_sysex(sysexbuffer, 6, MIDI3_PORT);
#endif
#ifdef MIDI_T36_HOST_PORT_ENABLED
      if (check_device_no == 4 ) {
        MIDI_send_sysex(sysexbuffer, 6, USBHMIDI_PORT); // Send message on default cable 0.
        MIDI_send_sysex(sysexbuffer, 6, USBHMIDI_PORT, 1); // Also send message on cable 1, so the GP-10 can be detected, which only supports sysex messages on cable 1
      }
#endif


      for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
        Device[d]->send_alternative_identity_request(check_device_no);
      }

      //MIDI_debug_sysex(sysexbuffer, 6, "CKout");
      check_device_no++;
      if (check_device_no >= NUMBER_OF_MIDI_PORTS) check_device_no = 0;
    }
  }
}

void MIDI_check_all_devices_still_connected() {
  //DEBUGMSG("Check devices");
  for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
    Device[d]->check_still_connected();
  }
}

inline void MIDI_disable_device_check() {
  device_check_enabled = false;
}

inline void MIDI_enable_device_check() {
  device_check_enabled = true;
  Check4DevicesTimer = millis(); // Reset the timer
}

uint8_t MIDI_set_port_number_from_menu(uint8_t mport) {
  switch (mport) {
    case 0: return USBMIDI_PORT;
    case 1: return MIDI1_PORT;
    case 2: return MIDI2_PORT;
    case 3: return MIDI3_PORT;
    case 4: return USBHMIDI_PORT;
    case 5: return ALL_MIDI_PORTS;
  }
  return USBMIDI_PORT;
}

// ********************************* Section 5: MIDI PC and CC Ledger ********************************************

// PC ledger - we will remember a number of PC values in memory, so the VController will show the correct state when returning to the PC values
#define PC_LEDGER_SIZE 25
#define NOT_FOUND 255

struct PC_ledger_struct {
  uint8_t program;
  uint8_t channel;
  uint8_t port;
};

PC_ledger_struct PC_ledger[PC_LEDGER_SIZE] = { 0 };
uint8_t PC_ledger_size = 0;
uint8_t PC_ledger_position = 0;

void MIDI_remember_PC(uint8_t program, uint8_t channel, uint8_t port) {
  // For speed reasons, we first check if the current position contains our data
  if ((PC_ledger[PC_ledger_position].channel == channel) && (PC_ledger[PC_ledger_position].port == port)) {
    PC_ledger[PC_ledger_position].program = program;
    DEBUGMSG("PC" + String(program) + " stored in position " + String(PC_ledger_position));
    return;
  }

  // Then we check the entire ledger
  for (uint8_t i = 0; i < PC_ledger_size; i++) { // First check if we already have this PC stored
    if ((PC_ledger[i].channel == channel) && (PC_ledger[i].port == port)) {
      PC_ledger[i].program = program;
      DEBUGMSG("PC" + String(program) + " stored in position " + String(i));
      return;
    }
  }

  // Add a new record.
  if (PC_ledger_size < PC_LEDGER_SIZE) PC_ledger_size++; // Once ledger is full, it cannot grow
  PC_ledger_position++;
  if (PC_ledger_position >= PC_LEDGER_SIZE) PC_ledger_position = 0; // The ledger position will go round
  PC_ledger[PC_ledger_position].program = program;
  PC_ledger[PC_ledger_position].channel = channel;
  PC_ledger[PC_ledger_position].port = port;
  DEBUGMSG("PC" + String(program) + " stored in position " + String(PC_ledger_position));
}

uint8_t MIDI_recall_PC(uint8_t channel, uint8_t port) {
  // For speed reasons, we first check if the current position contains our data
  if ((PC_ledger[PC_ledger_position].channel == channel) && (PC_ledger[PC_ledger_position].port == port)) {
    //DEBUGMSG("PC" + String(PC_ledger[PC_ledger_position].program) + " read from position " + String(PC_ledger_position));
    return PC_ledger[PC_ledger_position].program;
  }

  // Then we check the entire ledger
  for (uint8_t i = 0; i < PC_ledger_size; i++) {
    if ((PC_ledger[i].channel == channel) && (PC_ledger[i].port == port)) {
      //DEBUGMSG("PC" + String(PC_ledger[i].program) + " read from position " + String(i));
      return PC_ledger[i].program;
    }
  }
  return NOT_FOUND; // Not found
}


// CC ledger - we will remember a number of cc values in memory, so the VController will show the correct state when returning to the CC values
#define CC_LEDGER_SIZE 25
#define NOT_FOUND 255

struct CC_ledger_struct {
  uint8_t controller;
  uint8_t value;
  uint8_t channel;
  uint8_t port;
};

CC_ledger_struct CC_ledger[CC_LEDGER_SIZE] = { 0 };
uint8_t CC_ledger_size = 0;
uint8_t CC_ledger_position = 0;

void MIDI_remember_CC(uint8_t controller, uint8_t value, uint8_t channel, uint8_t port) {
  // For speed reasons, we first check if the current position contains our data
  if ((CC_ledger[CC_ledger_position].controller == controller) && (CC_ledger[CC_ledger_position].channel == channel) && (CC_ledger[CC_ledger_position].port == port)) {
    CC_ledger[CC_ledger_position].value = value;
    DEBUGMSG("CC" + String(controller) + "value " + String(value) + " stored in position " + String(CC_ledger_position));
    return;
  }

  // Then we check the entire ledger
  for (uint8_t i = 0; i < CC_ledger_size; i++) { // First check if we already have this CC stored
    if ((CC_ledger[i].controller == controller) && (CC_ledger[i].channel == channel) && (CC_ledger[i].port == port)) {
      CC_ledger[i].value = value;
      DEBUGMSG("CC" + String(controller) + "value " + String(value) + " stored in position " + String(i));
      return;
    }
  }

  // Add a new record.
  if (CC_ledger_size < CC_LEDGER_SIZE) CC_ledger_size++; // Once ledger is full, it cannot grow
  CC_ledger_position++;
  if (CC_ledger_position >= CC_LEDGER_SIZE) CC_ledger_position = 0; // The ledger position will go round
  CC_ledger[CC_ledger_position].controller = controller;
  CC_ledger[CC_ledger_position].value = value;
  CC_ledger[CC_ledger_position].channel = channel;
  CC_ledger[CC_ledger_position].port = port;
  DEBUGMSG("CC" + String(controller) + "value " + String(value) + " stored in position " + String(CC_ledger_position));
}

uint8_t MIDI_recall_CC(uint8_t controller, uint8_t channel, uint8_t port) {
  // For speed reasons, we first check if the current position contains our data
  if ((CC_ledger[CC_ledger_position].controller == controller) && (CC_ledger[CC_ledger_position].channel == channel) && (CC_ledger[CC_ledger_position].port == port)) {
    DEBUGMSG("CC" + String(controller) + "value " + String(CC_ledger[CC_ledger_position].value) + " read from position " + String(CC_ledger_position));
    return CC_ledger[CC_ledger_position].value;
  }

  // Then we check the entire ledger
  for (uint8_t i = 0; i < CC_ledger_size; i++) {
    if ((CC_ledger[i].controller == controller) && (CC_ledger[i].channel == channel) && (CC_ledger[i].port == port)) {
      DEBUGMSG("CC" + String(controller) + "value " + String(CC_ledger[i].value) + " read from position " + String(i));
      return CC_ledger[i].value;
    }
  }
  return NOT_FOUND; // Not found
}


// ********************************* Section 6: MIDI Editor Communication ********************************************


void MIDI_check_SYSEX_in_editor(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port) // Check for universal SYSEX message - identity reply
{
  if ((sxdata[1] == VC_MANUFACTURING_ID) && (sxdata[2] == VC_FAMILY_CODE) && (sxdata[3] == VC_MODEL_NUMBER) && (sxdata[4] == VC_DEVICE_ID)) {
    // Message from the editor
    switch (sxdata[5]) {
      case VC_REMOTE_CONTROL_ENABLE:
        remote_control_active = sxdata[6];
        remote_control_port = port;
        if (remote_control_active) {
          LCD_clear_memory();
          update_main_lcd = true;
          update_page = RELOAD_PAGE;
          LED_start_remote_control();
        }
        break;
      case VC_REMOTE_CONTROL_SWITCH_PRESSED:
        SC_remote_switch_pressed(sxdata[6], true);
        break;
      case VC_REMOTE_CONTROL_SWITCH_RELEASED:
        SC_remote_switch_released(sxdata[6], true);
        break;
      case VC_REQUEST_ALL_SETTINGS:
        MIDI_editor_send_settings();
        for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
          MIDI_editor_send_device_settings(d);
          //MIDI_show_dump_progress(d, NUMBER_OF_DEVICES + TOTAL_NUMBER_OF_SWITCHES);
        }
        for (uint8_t s = 0; s <= TOTAL_NUMBER_OF_SWITCHES; s++) {
          MIDI_editor_send_midi_switch_settings(s);
          //MIDI_show_dump_progress(NUMBER_OF_DEVICES + s, NUMBER_OF_DEVICES + TOTAL_NUMBER_OF_SWITCHES);
        }
        for (uint8_t p = 0; p <= EXT_MAX_NUMBER_OF_SEQ_PATTERNS; p++) {
          MIDI_editor_send_seq_pattern(p);
        }
        //LCD_show_popup_label("Settings sent!");
        break;
      case VC_SET_GENERAL_SETTINGS:
        MIDI_editor_receive_settings(sxdata, sxlength);
        break;
      case VC_SET_DEVICE_SETTINGS:
        MIDI_editor_receive_device_settings(sxdata, sxlength);
        break;
      case VC_SET_MIDI_SWITCH_SETTINGS:
        MIDI_editor_receive_midi_switch_settings(sxdata, sxlength);
        break;
      case VC_SET_SEQ_PATTERN:
        MIDI_editor_receive_pattern_data(sxdata, sxlength);
        break;
      case VC_REQUEST_SEQ_PATTERNS:
        for (uint8_t p = 0; p <= EXT_MAX_NUMBER_OF_SEQ_PATTERNS; p++) {
          MIDI_editor_send_seq_pattern(p);
          MIDI_show_dump_progress(p, EXT_MAX_NUMBER_OF_SEQ_PATTERNS);
        }
        break;
      case VC_SAVE_SETTINGS:
        EEP_write_eeprom_common_data(); // Save settings to EEPROM
        delay(100);
        update_page = RELOAD_PAGE;
        break;
      case VC_REQUEST_COMMANDS_DUMP:
        MIDI_send_commands_dump();
        break;
      case VC_START_COMMANDS_DUMP:
        EEPROM_clear_all_commands();
        editor_dump_size = (sxdata[6] << 7) + sxdata[7];
        break;
      case VC_SET_COMMAND:
        MIDI_editor_receive_command(sxdata, sxlength);
        MIDI_show_dump_progress(number_of_cmds, editor_dump_size);
        break;
      case VC_FINISH_COMMANDS_DUMP:
        MIDI_editor_receive_finish_commands_dump(sxdata, sxlength);
        editor_dump_size = 0;
        break;
      case VC_REQUEST_DEVICE_PATCHES:
        MIDI_send_device_patch_dump();
        break;
      case VC_SET_DEVICE_PATCH:
        MIDI_editor_receive_device_patch(sxdata, sxlength);
        break;
      case VC_FINISH_DEVICE_PATCH_DUMP:
        //Reload patches after patch dump from editor
        if (My_KTN.connected) My_KTN.load_patch(My_KTN.patch_number);
        if (My_SY1000.connected) My_SY1000.load_patch(My_SY1000.patch_number);
        update_page = RELOAD_PAGE;
        break;
    }
  }
}

void MIDI_remote_update_display(uint8_t number, const char *line1, const char *line2) {
  if (!remote_control_active) return;
  uint8_t sysexmessage[40] = { 0xF0, VC_MANUFACTURING_ID, VC_FAMILY_CODE, VC_MODEL_NUMBER, VC_DEVICE_ID, VC_REMOTE_UPDATE_DISPLAY, number};
  for (uint8_t i = 0; i < LCD_DISPLAY_SIZE; i++) sysexmessage[i + 7] = line1[i];
  for (uint8_t i = 0; i < LCD_DISPLAY_SIZE; i++) sysexmessage[i + 23] = line2[i];
  sysexmessage[39] = 0xF7;
  MIDI_send_sysex(sysexmessage, 40, remote_control_port);
}

void MIDI_update_LEDs(uint8_t *MIDI_LEDs, uint8_t number_of_leds) {
  if (!remote_control_active) return;
  uint8_t messagesize = number_of_leds + 8;
  uint8_t sysexmessage[messagesize] = { 0xF0, VC_MANUFACTURING_ID, VC_FAMILY_CODE, VC_MODEL_NUMBER, VC_DEVICE_ID, VC_REMOTE_UPDATE_LEDS, number_of_leds};
  for (uint8_t i = 0; i < number_of_leds; i++) sysexmessage[i + 7] = MIDI_LEDs[i];
  sysexmessage[messagesize - 1] = 0xF7;
  MIDI_send_sysex(sysexmessage, messagesize, remote_control_port);
}

void MIDI_send_data(uint8_t cmd, uint8_t *my_data, uint16_t my_len) {
  // We will send the data per 7 bits, because any sysex data has to be smaller than 128
  uint16_t messagesize = (my_len * 2) + 7;
  uint8_t sysexmessage[messagesize] = { 0xF0, VC_MANUFACTURING_ID, VC_FAMILY_CODE, VC_MODEL_NUMBER, VC_DEVICE_ID, cmd };
  for (uint8_t i = 0; i < my_len; i++) {
    sysexmessage[(i * 2) + 6] = my_data[i] >> 7;
    sysexmessage[(i * 2) + 7] = my_data[i] & 0x7F;
  }
  sysexmessage[messagesize - 1] = 0xF7;
  MIDI_send_sysex(sysexmessage, messagesize, remote_control_port);
}

void MIDI_read_data(const unsigned char* sxdata, short unsigned int sxlength, uint8_t *my_data, uint16_t my_len) {
  for (uint8_t i = 0; i < my_len; i++) {
    uint8_t pos = (i * 2) + 6;
    my_data[i] = (sxdata[pos] << 7) + sxdata[pos + 1];
  }
}

void MIDI_show_error() {
  LCD_show_popup_label("MIDI read error!", MESSAGE_TIMER_LENGTH);
  DEBUGMSG("MIDI error - message length incorrect");
}

void MIDI_editor_send_settings() {
  uint8_t* settingbytes = (uint8_t*)&Setting;
  MIDI_send_data(VC_SET_GENERAL_SETTINGS, settingbytes, sizeof(Setting));
}

void MIDI_editor_receive_settings(const unsigned char* sxdata, short unsigned int sxlength) {
  if (sxlength != sizeof(Setting) * 2 + 7) {
    MIDI_show_error();
    return;
  }
  uint8_t* settingbytes = (uint8_t*)&Setting;
  MIDI_read_data(sxdata, sxlength, settingbytes, sizeof(Setting));
  //EEP_write_eeprom_common_data(); // Save to EEPROM
  //update_page = RELOAD_PAGE;
}

void MIDI_editor_send_device_settings(uint8_t dev) { // Will send the device number, followed by the device settings
  uint8_t dsettings[NUMBER_OF_DEVICE_SETTINGS + 1];
  dsettings[0] = dev;
  if (dev >= NUMBER_OF_DEVICES) return;
  for (uint8_t i = 0; i < NUMBER_OF_DEVICE_SETTINGS; i++) {
    dsettings[i + 1] = Device[dev]->get_setting(i);
  }
  MIDI_send_data(VC_SET_DEVICE_SETTINGS, dsettings, NUMBER_OF_DEVICE_SETTINGS + 1);
}

void MIDI_editor_receive_device_settings(const unsigned char* sxdata, short unsigned int sxlength) {
  if (sxlength != NUMBER_OF_DEVICE_SETTINGS * 2 + 9) {
    MIDI_show_error();
    return;
  }
  uint8_t dsettings[NUMBER_OF_DEVICE_SETTINGS + 1];
  MIDI_read_data(sxdata, sxlength, dsettings, NUMBER_OF_DEVICE_SETTINGS + 1);
  uint8_t dev = dsettings[0];
  if (dev >= NUMBER_OF_DEVICES) return;
  for (uint8_t i = 0; i < NUMBER_OF_DEVICE_SETTINGS; i++) {
    Device[dev]->set_setting(i, dsettings[i + 1]);
    DEBUGMSG("Setting " + String(i) + " has value " + String(dsettings[i + 1]));
  }
  Device[dev]->check_manual_connection();
  //EEP_write_eeprom_common_data(); // Save to EEPROM
  update_page = RELOAD_PAGE;
}

void MIDI_editor_send_midi_switch_settings(uint8_t sw) { // Will send the device number, followed by the device settings
  uint8_t mssettings[5];
  if (sw > TOTAL_NUMBER_OF_SWITCHES) return;
  mssettings[0] = sw;
  mssettings[1] = MIDI_switch[sw].type;
  mssettings[2] = MIDI_switch[sw].port;
  mssettings[3] = MIDI_switch[sw].channel;
  mssettings[4] = MIDI_switch[sw].cc;
  MIDI_send_data(VC_SET_MIDI_SWITCH_SETTINGS, mssettings, 5);
}

void MIDI_editor_receive_midi_switch_settings(const unsigned char* sxdata, short unsigned int sxlength) {
  if (sxlength != EEPROM_MIDI_SWITCH_SETTINGS_DATA_SIZE * 2 + 9) {
    MIDI_show_error();
    return;
  }
  uint8_t mssettings[EEPROM_MIDI_SWITCH_SETTINGS_DATA_SIZE + 1];
  MIDI_read_data(sxdata, sxlength, mssettings, EEPROM_MIDI_SWITCH_SETTINGS_DATA_SIZE + 1);
  uint8_t sw = mssettings[0];
  if (sw > TOTAL_NUMBER_OF_SWITCHES) return;
  MIDI_switch[sw].type = mssettings[1];
  MIDI_switch[sw].port = mssettings[2];
  MIDI_switch[sw].channel = mssettings[3];
  MIDI_switch[sw].cc = mssettings[4];
  DEBUGMSG("!!! Received MIDI switch #" + String(sw) + ": type " + String(mssettings[1]) + ", port " + String(mssettings[2]) + ", chan " + String(mssettings[3]) + ", cc " + String(mssettings[4]));
  // EEP_write_eeprom_common_data(); // Save to EEPROM
  //  update_page = RELOAD_PAGE;
}

void MIDI_editor_send_seq_pattern(uint8_t pattern) {
  uint8_t mpatterndata[EEPROM_SEQ_PATTERN_SIZE + 1];
  mpatterndata[0] = pattern;
  EEPROM_load_seq_pattern(pattern, mpatterndata + 1);
  MIDI_send_data(VC_SET_SEQ_PATTERN, mpatterndata, EEPROM_SEQ_PATTERN_SIZE + 1);
}

void MIDI_editor_receive_pattern_data(const unsigned char* sxdata, short unsigned int sxlength) {
  if (sxlength != EEPROM_SEQ_PATTERN_SIZE * 2 + 9) {
    MIDI_show_error();
    return;
  }
  uint8_t mpatterndata[EEPROM_SEQ_PATTERN_SIZE + 1];
  MIDI_read_data(sxdata, sxlength, mpatterndata, EEPROM_SEQ_PATTERN_SIZE + 1);
  uint8_t pattern = mpatterndata[0];
  if (pattern > EXT_MAX_NUMBER_OF_SEQ_PATTERNS) return;
  EEPROM_store_seq_pattern(pattern, mpatterndata + 1);
  DEBUGMSG("!!! Received seq pattern #" + String(pattern));
}

void MIDI_editor_send_start_commands_dump() {
  uint8_t sysexmessage[9] = { 0xF0, VC_MANUFACTURING_ID, VC_FAMILY_CODE, VC_MODEL_NUMBER, VC_DEVICE_ID, VC_START_COMMANDS_DUMP,
                              (uint8_t)((number_of_cmds >> 7) & 0x7F), (uint8_t)(number_of_cmds & 0x7F), 0xF7
                            };
  MIDI_send_sysex(sysexmessage, 9, remote_control_port);
}


void MIDI_editor_send_finish_commands_dump() {
  uint8_t sysexmessage[11] = { 0xF0, VC_MANUFACTURING_ID, VC_FAMILY_CODE, VC_MODEL_NUMBER, VC_DEVICE_ID, VC_FINISH_COMMANDS_DUMP,
                               (uint8_t)((Number_of_pages >> 7) & 0x7F), (uint8_t)(Number_of_pages & 0x7F),
                               (uint8_t)((number_of_cmds >> 7) & 0x7F), (uint8_t)(number_of_cmds & 0x7F), 0xF7
                             };
  MIDI_send_sysex(sysexmessage, 11, remote_control_port);
}

void MIDI_editor_receive_finish_commands_dump(const unsigned char* sxdata, short unsigned int sxlength) {
  if (sxlength != 11) {
    MIDI_show_error();
    return;
  }
  uint8_t check_number_of_pages = (sxdata[6] << 7) + sxdata[7];
  uint16_t check_number_of_cmds = (sxdata[8] << 7) + sxdata[9];
  EEPROM_check_data_received(check_number_of_pages, check_number_of_cmds);
  if ((Current_page > Number_of_pages) && (Current_page < FIRST_FIXED_CMD_PAGE)) Current_page = Number_of_pages - 1;
  update_page = RELOAD_PAGE;
}

void MIDI_send_commands_dump() {
  MIDI_editor_send_start_commands_dump();
  editor_dump_size = number_of_cmds;
  for (uint16_t c = 0; c < number_of_cmds; c++) {
    MIDI_editor_send_command(c);
    MIDI_show_dump_progress(c, number_of_cmds);
    delay(10); // Matching progress bar speed with the editor
  }
  MIDI_editor_send_finish_commands_dump();
  editor_dump_size = 0;
  LCD_show_popup_label("Upload complete", MESSAGE_TIMER_LENGTH);
}

void MIDI_editor_send_command(uint16_t cmd_no) {
  Cmd_struct cmd;
  read_cmd_EEPROM(cmd_no, &cmd);
  uint8_t* cmdbytes = (uint8_t*)&cmd;
  MIDI_send_data(VC_SET_COMMAND, cmdbytes, sizeof(cmd));
}

void MIDI_send_device_patch_dump() {
  for (uint16_t p = 0; p < EXT_MAX_NUMBER_OF_PATCH_PRESETS; p++) {
    MIDI_editor_send_patch(p);
    MIDI_show_dump_progress(p, EXT_MAX_NUMBER_OF_PATCH_PRESETS);
    delay(10); // Matching progress bar speed with the editor
  }
  MIDI_editor_send_finish_device_patch_dump();
  LCD_show_popup_label("Upload complete", MESSAGE_TIMER_LENGTH);
}

// Device patch data is sent using the overflow byte system from Zoom
// MIDI sysex data can not use the 8th bit of a data byte. So we use the overflow byte to store the 8th bits of the next 7 bytes.
void MIDI_editor_send_patch(uint16_t index) {
  uint8_t patch_buffer[VC_PATCH_SIZE];
  EEPROM_load_device_patch_by_index(index, patch_buffer, VC_PATCH_SIZE);
  uint8_t number_of_overflow_bytes = (VC_PATCH_SIZE + 6) / 7;
  uint16_t messagesize = VC_PATCH_SIZE + number_of_overflow_bytes + 9;
  uint8_t sysexmessage[messagesize] = { 0xF0, VC_MANUFACTURING_ID, VC_FAMILY_CODE, VC_MODEL_NUMBER, VC_DEVICE_ID, VC_SET_DEVICE_PATCH, (uint8_t)(index >> 7), (uint8_t)(index & 0x7F) };
  uint8_t buffer_index = 0;
  for (uint8_t obi = 0; obi < number_of_overflow_bytes; obi++) {
    uint8_t overflow_byte = 0;
    uint8_t byte_index = 9 + (obi * 8);
    for (uint8_t i = 0; i < 7; i++) {
      if (buffer_index < VC_PATCH_SIZE) {
        overflow_byte |= (patch_buffer[buffer_index] >> 7) << i;
        sysexmessage[byte_index++] = patch_buffer[buffer_index] & 0x7F;
      }
      buffer_index++;
    }
    sysexmessage[8 + (obi * 8)] = overflow_byte;
  }
  sysexmessage[messagesize - 1] = 0xF7;
  MIDI_send_sysex(sysexmessage, messagesize, remote_control_port);
}

void MIDI_show_dump_progress(uint16_t number, uint16_t total) {
  //if (editor_dump_size == 0) return;

  LCD_show_bar(0, map(number, 0, total, 0, 127));
}

void MIDI_editor_receive_command(const unsigned char* sxdata, short unsigned int sxlength) {
  Cmd_struct cmd;
  uint8_t* cmdbytes = (uint8_t*)&cmd;
  MIDI_read_data(sxdata, sxlength, cmdbytes, sizeof(cmd));
  EEPROM_write_command_from_editor(&cmd);
}

// Device patch data is sent using the overflow byte system from Zoom
// MIDI sysex data can not use the 8th bit of a data byte. So we use the overflow byte to store the 8th bits of the next 7 bytes.
void MIDI_editor_receive_device_patch(const unsigned char* sxdata, short unsigned int sxlength) {
  uint8_t number_of_overflow_bytes = (VC_PATCH_SIZE + 6) / 7;
  if (sxlength < VC_PATCH_SIZE + number_of_overflow_bytes + 9) return;
  uint16_t number = (sxdata[6] << 7) + sxdata[7];
  if (number >= EXT_MAX_NUMBER_OF_PATCH_PRESETS) return;

  uint8_t patch_buffer[VC_PATCH_SIZE];
  uint8_t buffer_index = 0;
  for (uint8_t obi = 0; obi < number_of_overflow_bytes; obi++) {
    uint8_t overflow_byte = sxdata[8 + (obi * 8)];
    uint8_t byte_index = 9 + (obi * 8);
    for (uint8_t i = 0; i < 7; i++) {
      if (buffer_index < VC_PATCH_SIZE) {
        uint8_t new_byte = sxdata[byte_index++];
        if ((overflow_byte & (1 << i)) != 0) new_byte |= 0x80;
        patch_buffer[buffer_index] = new_byte;
      }
      buffer_index++;
    }
  }
  
  EEPROM_save_device_patch_by_index(number, patch_buffer, VC_PATCH_SIZE);
  patch_data_index[number].Type = patch_buffer[0];
  patch_data_index[number].Patch_number = (patch_buffer[1] << 8) + patch_buffer[2];
}

void MIDI_editor_send_finish_device_patch_dump() {
  uint8_t sysexmessage[7] = { 0xF0, VC_MANUFACTURING_ID, VC_FAMILY_CODE, VC_MODEL_NUMBER, VC_DEVICE_ID, VC_FINISH_DEVICE_PATCH_DUMP, 0xF7
                            };
  MIDI_send_sysex(sysexmessage, 7, remote_control_port);
  update_page = RELOAD_PAGE;
}

// ********************************* Section 7: MIDI switch command reading ********************************************

void MIDI_check_switch_cc(uint8_t control, uint8_t value, uint8_t channel, uint8_t port) {
  for (uint8_t s = 1; s <= TOTAL_NUMBER_OF_SWITCHES; s++) {
    if (MIDI_switch[s].type != MIDI_SWITCH_OFF) {
      uint8_t p = MIDI_set_port_number_from_menu(MIDI_switch[s].port);
      if (((p == port) || (p == ALL_MIDI_PORTS)) && (MIDI_switch[s].channel == channel) && (MIDI_switch[s].cc == control)) {
        switch (MIDI_switch[s].type) {
          case MIDI_SWITCH_CC_MOMENTARY:
            if (value >= 64) SC_remote_switch_pressed(s, false);
            else SC_remote_switch_released(s, false);
            break;
          case MIDI_SWITCH_PRESSED_NO_RELEASE:
            SC_remote_switch_pressed_no_release(s);
            break;
          case MIDI_SWITCH_CC_RANGE:
            SC_remote_expr_pedal(s, value);
            break;
        }
      }
    }
  }
}

void MIDI_check_switch_pc(uint8_t program, uint8_t channel, uint8_t port) {
  for (uint8_t s = 1; s <= TOTAL_NUMBER_OF_SWITCHES; s++) {
    if (MIDI_switch[s].type == MIDI_SWITCH_PC) {
      uint8_t p = MIDI_set_port_number_from_menu(MIDI_switch[s].port);
      if (((p == port) || (p == ALL_MIDI_PORTS)) && (MIDI_switch[s].channel == channel)) {
        SC_remote_switch_select_program(s, program);
      }
    }
  }
}
