#ifndef MIDI_H
#define MIDI_H

#include <MIDI.h>

#define NUMBER_OF_MIDI_PORTS 4
#define USBMIDI_PORT 0x00
#define MIDI1_PORT 0x10
#define MIDI2_PORT 0x20
#define MIDI3_PORT 0x30
#define ALL_PORTS 0xF0

// Setup MIDI ports. The USB port is set from the Arduino menu.
struct MySettings : public midi::DefaultSettings
{
  //static const bool UseRunningStatus = false; // Messes with my old equipment!
  //static const bool Use1ByteParsing = false; // More focus on reading messages - will this help the equipment from stopping with receiving data?
  static const unsigned SysExMaxSize = 256; // Change sysex buffersize - Zoom devices send packets up to 146 bytes
};

MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial1, MIDI1, MySettings); // Enables serial1 port for MIDI communication with custom settings
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial2, MIDI2, MySettings); // Enables serial2 port for MIDI communication with custom settings

uint8_t Current_MIDI_port;

void OnProgramChange(byte channel, byte program) {
  Serial.println("PC #" + String(program) + " received on channel " + String(channel) + " and port " + String(Current_MIDI_port >> 4)); // Show on serial debug screen
}

void OnSerialSysEx(byte *sxdata, unsigned sxlength) {
  Serial.print("<=");
  switch (Current_MIDI_port) {
    case USBMIDI_PORT:
      Serial.print("USB_M: ");
      break;
    case MIDI1_PORT:
      Serial.print("MIDI1: ");
      break;
    case MIDI2_PORT:
      Serial.print("MIDI2: ");
      break;
    case MIDI3_PORT:
      Serial.print("MIDI3: ");
      break;
    default:
      Serial.print("multiple: ");
  }
  Serial.print("(" + String(sxlength) + " bytes) ");
  for (uint8_t i = 0; i < sxlength; i++) {
    if (sxdata[i] < 0x10) Serial.print("0" + String(sxdata[i], HEX) + " ");
    else Serial.print(String(sxdata[i], HEX) + " ");
  }
  Serial.println();
}

void setup_MIDI_common()
{
  MIDI1.begin(MIDI_CHANNEL_OMNI);
  MIDI1.turnThruOff();
  //MIDI1.setHandleNoteOff(OnNoteOff);
  //MIDI1.setHandleNoteOn(OnNoteOn) ;
  MIDI1.setHandleProgramChange(OnProgramChange);
  //MIDI1.setHandleControlChange(OnControlChange);
  MIDI1.setHandleSystemExclusive(OnSerialSysEx);
  //MIDI1.setHandleActiveSensing(OnActiveSense);

  delay(100);
  MIDI2.begin(MIDI_CHANNEL_OMNI);
  MIDI2.turnThruOff();
  //MIDI2.setHandleNoteOff(OnNoteOff);
  //MIDI2.setHandleNoteOn(OnNoteOn) ;
  MIDI2.setHandleProgramChange(OnProgramChange);
  //MIDI2.setHandleControlChange(OnControlChange);
  MIDI2.setHandleSystemExclusive(OnSerialSysEx);
  //MIDI2.setHandleActiveSensing(OnActiveSense);
}

void main_MIDI_common()
{
  Current_MIDI_port = USBMIDI_PORT;
  usbMIDI.read();
  Current_MIDI_port = MIDI1_PORT;
  MIDI1.read();
  Current_MIDI_port = MIDI2_PORT;
  MIDI2.read();
}

#endif
