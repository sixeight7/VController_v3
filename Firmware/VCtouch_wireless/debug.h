// Please read VCCtouch_wireless.ino for information about the license and authors

#ifndef DEBUG_H
#define DEBUG_H

void StartFreeTimer();

// Enable normal debugging (put // in front of #define to disable it
#define DEBUG_NORMAL

// Enable debugging of main events (put // in front of the next line to disable it
#define DEBUG_MAIN

// Midi debugging
//#define DEBUG_MIDI
//#define DEBUG_SYSEX
//#define DEBUG_SYSEX_UNIVERSAL

#define DEBUG_SYSEX_MAX_LENGTH 256 // The maximum numbers of bytes shown in the debug window

// Check for free memory - to detect memory leaks
//#define DEBUG_FREE

// Setup debug procedures.
#define SERIAL_STARTUP_TIMER_LENGTH 30000 // Will wait max three seconds max before serial starts
uint32_t serial_timer;

void setup_debug() {
#if defined(DEBUG_NORMAL) || defined(DEBUG_MAIN) || defined(DEBUG_MIDI) || defined(DEBUG_SYSEX)
  Serial.begin(115200);
  serial_timer = millis();
  while ((!Serial) && (serial_timer - millis() < SERIAL_STARTUP_TIMER_LENGTH)) {}; // Wait while the serial communication is not ready or while the SERIAL_START_UP time has not elapsed.
  delay(3000);
  Serial.println("VC-touch WIFI/Bluetooth debugging started...");
  Serial.println("Arduino version: " + String(ARDUINO));
  StartFreeTimer();
#endif
}

#ifdef DEBUG_NORMAL
#define DEBUGMSG(arg) Serial.println(arg)
#else
#define DEBUGMSG(arg) //
#endif

#ifdef DEBUG_MAIN
#define DEBUGMAIN(arg) Serial.println(arg)
#else
#define DEBUGMAIN(arg) //
#endif

#ifdef DEBUG_MIDI
#define DEBUGMIDI(arg) Serial.println(arg)
#else
#define DEBUGMIDI(arg) //
#endif

#define MIDI_PORT 0x00
#define BLEMIDI_PORT 0x10
#define RTPMIDI_PORT 0x20

//Debug sysex messages by sending them to the serial monitor
void MIDI_debug_sysex(const unsigned char* sxdata, short unsigned int sxlength, uint8_t port, bool is_out) {

#ifndef DEBUG_SYSEX_UNIVERSAL
  //if (sxdata[1] == 0x7D) return; // Quit if we have a universal midi message
  if (sxdata[1] == 0x7E) return; // Quit if we have a universal midi message
  if (sxdata[2] == 0x7F) return; // Quit if we have an FC300 bloat message
  if ((sxdata[3] == 0x74) && (sxdata[4] == 0x7F)) return; // Quit if we have a Fractal connect message
  if ((sxdata[2] == 0x11) && (sxdata[3] == 0x22) && (sxdata[4] == 0x66) && (sxdata[6] <= 0x10)) return; // Quit if we have a NUX connect message
#endif

#ifdef DEBUG_SYSEX
  if (is_out) Serial.print("SND ");
  else Serial.print("RCV ");
  uint8_t VCbridge_port = (port & 0x0F);
  switch (port & 0xF0) {
    case MIDI_PORT:
      Serial.print("MIDI:" + String(VCbridge_port) + ' ');
      break;
    case BLEMIDI_PORT:
      Serial.print("BLEMIDI:" + String(VCbridge_port) + ' ');
      break;
    case RTPMIDI_PORT:
      Serial.print("RTPMIDI:" + String(VCbridge_port) + ' ');
      break;
    default:
      Serial.print("multiple:" + String(VCbridge_port) + ' ');
  }
  Serial.print('(' + String(sxlength) + " bytes) ");
  if (sxlength > DEBUG_SYSEX_MAX_LENGTH) {
    sxlength = DEBUG_SYSEX_MAX_LENGTH; // limit the largest messages
    Serial.print("(first " + String(DEBUG_SYSEX_MAX_LENGTH) + " bytes) ");
  }
  for (uint8_t i = 0; i < sxlength; i++) {
    if (sxdata[i] < 0x10) Serial.print("0" + String(sxdata[i], HEX) + ' ');
    else Serial.print(String(sxdata[i], HEX) + ' ');
  }
  Serial.println();
#endif
}

#ifdef DEBUG_FREE
IntervalTimer FreeTimer;

//Check free RAM
uint32_t FreeRam() { // for Teensy 3.0
  uint32_t stackTop;
  uint32_t heapTop;

  // current position of the stack.
  stackTop = (uint32_t) &stackTop;

  // current position of heap.
  void* hTop = malloc(1);
  heapTop = (uint32_t) hTop;
  free(hTop);

  // The difference is the free, available ram.
  return stackTop - heapTop;
}

void FreeTimerInterrupt() {
  Serial.println("Free RAM: " + String(FreeRam()));
}
#endif

void StartFreeTimer() {
#ifdef DEBUG_FREE
  FreeTimer.begin(FreeTimerInterrupt, 5000000); // Run every five seconds
#endif
}

#endif
