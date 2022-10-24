// Please read VController_v3.ino for information about the license and authors

#ifndef DEBUG_H
#define DEBUG_H

void StartFreeTimer();

// Enable normal debugging (put // in front of #define to disable it
#define DEBUG_NORMAL

// Enable debugging of main events (put // in front of the next line to disable it
#define DEBUG_MAIN

// Midi debugging
#define DEBUG_MIDI
#define DEBUG_SYSEX
//#define DEBUG_SYSEX_UNIVERSAL

#define DEBUG_SYSEX_MAX_LENGTH 256 // The maximum numbers of bytes shown in the debug window

// Check for free memory - to detect memory leaks
//#define DEBUG_FREE

// Setup debug procedures.
#define SERIAL_STARTUP_TIMER_LENGTH 3000 // Will wait max three seconds max before serial starts
uint32_t serial_timer;

void setup_debug() {
#if defined(DEBUG_NORMAL) || defined(DEBUG_MAIN) || defined(DEBUG_MIDI) || defined(DEBUG_SYSEX)
  Serial.begin(115200);
  serial_timer = millis();
  while ((!Serial) && (serial_timer - millis() < SERIAL_STARTUP_TIMER_LENGTH)) {}; // Wait while the serial communication is not ready or while the SERIAL_START_UP time has not elapsed.
  String msg = VC_NAME;
  Serial.println(msg + " version " + String(VCONTROLLER_FIRMWARE_VERSION_MAJOR) + "." + String(VCONTROLLER_FIRMWARE_VERSION_MINOR) + "." + String(VCONTROLLER_FIRMWARE_VERSION_BUILD) + " debugging started...");
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

#define USBMIDI_PORT 0x00
#define MIDI1_PORT 0x10
#define MIDI2_PORT 0x20
#define MIDI3_PORT 0x30
#define MIDI4_PORT 0x40
#define USBHMIDI_PORT 0x50
#define MIDI5_PORT 0x60
#define ALL_MIDI_PORTS 0xF0

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
    case USBMIDI_PORT:
      Serial.print("USB_M:" + String(VCbridge_port) + ' ');
      break;
    case MIDI1_PORT:
      Serial.print("MIDI1:" + String(VCbridge_port) + ' ');
      break;
    case MIDI2_PORT:
      Serial.print("MIDI2:" + String(VCbridge_port) + ' ');
      break;
    case MIDI3_PORT:
      Serial.print("MIDI3:" + String(VCbridge_port) + ' ');
      break;
    case MIDI4_PORT:
      Serial.print("MIDI4:" + String(VCbridge_port) + ' ');
      break;
    case MIDI5_PORT:
      Serial.print("MIDI5:" + String(VCbridge_port) + ' ');
      break;
    case USBHMIDI_PORT:
      Serial.print("USBH_M:" + String(VCbridge_port) + ' ');
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
