// Bluetooth communication
#include <BLEMIDI_Transport.h>

#include <hardware/BLEMIDI_ESP32_NimBLE.h> // Uses fewer resources than BLEMIDI_ESP32.h
//#include <hardware/BLEMIDI_ESP32.h>
//#include <hardware/BLEMIDI_nRF52.h>
//#include <hardware/BLEMIDI_ArduinoBLE.h>

//BLEMIDI_CREATE_CUSTOM_INSTANCE("VC-touch BLE MIDI", MIDI_BLE, MySettings)
BLEMIDI_CREATE_INSTANCE("VC-touch BLE MIDI", MIDI_BLE)

bool BLE_enabled = false;
bool BLE_connected = false;

void setup_BLE() {
  if (EEP_read_ble_mode() == 1) start_BLE();
}



void run_BLE() {
  if (BLE_enabled) MIDI_BLE.read();
}

void start_BLE() {
  BLE_enabled = true;
  MIDI_BLE.begin(MIDI_CHANNEL_OMNI);
  MIDI_BLE.turnThruOff();
  MIDI_BLE.setHandleNoteOff(OnBleNoteOff);
  MIDI_BLE.setHandleNoteOn(OnBleNoteOn) ;
  MIDI_BLE.setHandleProgramChange(OnBleProgramChange);
  MIDI_BLE.setHandleControlChange(OnBleControlChange);
  MIDI_BLE.setHandleSystemExclusive(OnBleSysEx);
  MIDI_BLE.setHandleClock(OnBleClock);
  MIDI_BLE.setHandlePitchBend(OnBlePitchBend);
  
  BLEMIDI_BLE.setHandleConnected([]() {
    BLE_connected = true;
    digitalWrite(LED_BUILTIN, HIGH);
    led_on = true;
    DEBUGMAIN("BLEMIDI connected");
    MIDI_send_ble_state(1);
  });

  BLEMIDI_BLE.setHandleDisconnected([]() {
    BLE_connected = false;
    digitalWrite(LED_BUILTIN, LOW);
    led_on = true;
    DEBUGMAIN("BLEMIDI disconnected");
    MIDI_send_ble_state(0);
  });
}

void stop_BLE() {
  BLE_enabled = false;
  BLEMIDI_BLE.end();
  MIDI_send_ble_state(0);
}

void check_BLE_mode(uint8_t mode) {
  if (mode != EEP_read_ble_mode()) {
    EEP_set_ble_mode(mode);
    if (mode == 0) stop_BLE();
    else start_BLE();
  }
}

void check_BLE_status() {
  if (BLE_connected) MIDI_send_ble_state(1);
  else MIDI_send_ble_state(0);
}
