// MIDI

// Midi IDs for sysex messages of the VController
#define VC_MANUFACTURING_ID 0x7D // Universal for simple midi device
#define VC_WL_MESSAGE_CODE 0x77  // Used for communicating between the wireless module and the Teensy
#define VC_DEVICE_ID 0x01        // The device id of the VController

// Commands for communicating between Teensy and ESP
#define WL_MODULE_ACTIVE 0
#define WL_BLE_STATUS 1
#define WL_WIFI_STATUS 2
#define WL_RTPMIDI_STATUS 3
#define WL_SET_WIFI_SSID 4
#define WL_SET_WIFI_PASSWORD 5
#define WL_SET_SERVER_ADMIN_PASSWORD 6
#define WL_SET_AP_SSID 7
#define WL_SET_AP_PASSWORD 8
#define WL_DO_WIFI_SCAN 9
#define WL_SEND_SCANNED_SSID 10
#define WL_SET_CURRENT_SSID 11

#define MAX_MSG_SIZE 32

#define LINE_SELECT_CC_NUMBER 119
#define VCONTROLLER_MIDI_CHANNEL 16
uint8_t VCbridge_in_port = 0;
uint8_t VCbridge_out_port = 0;
uint8_t MIDI_sysex_buffer[1024];
uint16_t MIDI_sysex_length;
uint8_t BLE_sysex_buffer[1024];
uint16_t BLE_sysex_length;
uint8_t RPT_sysex_buffer[1024];
uint16_t RPT_sysex_length;

struct MySettings : public midi::DefaultSettings
{
  static const long BaudRate = 31250;
  static const unsigned SysExMaxSize = 1024; // Change sysex buffersize - Zoom devices send packets up to 146 bytes, GMajor up to 615
};

struct FastSettings : public midi::DefaultSettings
{
  static const long BaudRate = MIDI2_SPEED;
  static const unsigned SysExMaxSize = 1024;
  static const bool Use1ByteParsing = false;
};

MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial2, MIDI1, FastSettings); // Serial1 is used for programming, so we use Serial2

/*void MIDI1_forward_message(const MIDI_NAMESPACE::MidiMessage& message) {
  //if ((BLE_connected) && ((VCbridge_in_port == 0) || (VCbridge_in_port == 1))) MIDI_BLE.send(message);
  }*/



void setup_MIDI() {
  //Serial2.begin(2000000); // Workaround as MIDI_CREATE_CUSTOM_INSTANCE does not compile on the ESP32
  MIDI1.begin(MIDI_CHANNEL_OMNI);
  //Serial2.setRxBufferSize(2048);
  MIDI1.turnThruOff();
  MIDI1.setHandleNoteOff(OnNoteOff);
  MIDI1.setHandleNoteOn(OnNoteOn) ;
  MIDI1.setHandleProgramChange(OnProgramChange);
  MIDI1.setHandleControlChange(OnControlChange);
  MIDI1.setHandleSystemExclusive(OnSysEx);
  MIDI1.setHandleClock(OnClock);
  MIDI1.setHandlePitchBend(OnPitchBend);
}

void run_MIDI() {
  MIDI1.read();
}

// Forwarding from MIDI port
void OnNoteOn(byte channel, byte note, byte velocity) {
  if ((BLE_connected) && ((VCbridge_in_port == 0) || (VCbridge_in_port == 1))) MIDI_BLE.sendNoteOn(note, velocity, channel);
  if ((RTP_connected) && ((VCbridge_in_port == 0) || (VCbridge_in_port == 2))) MIDI_RTP.sendNoteOn(note, velocity, channel);
}

void OnNoteOff(byte channel, byte note, byte velocity) {
  if ((BLE_connected) && ((VCbridge_in_port == 0) || (VCbridge_in_port == 1))) MIDI_BLE.sendNoteOff(note, velocity, channel);
  if ((RTP_connected) && ((VCbridge_in_port == 0) || (VCbridge_in_port == 2))) MIDI_RTP.sendNoteOff(note, velocity, channel);
}

void OnProgramChange(byte channel, byte program) {
  if ((BLE_connected) && ((VCbridge_in_port == 0) || (VCbridge_in_port == 1))) MIDI_BLE.sendProgramChange(program, channel);
  if ((RTP_connected) && ((VCbridge_in_port == 0) || (VCbridge_in_port == 2))) MIDI_RTP.sendProgramChange(program, channel);
}

void OnControlChange(byte channel, byte control, byte value) {
  if ((channel == VCONTROLLER_MIDI_CHANNEL) && (control == LINE_SELECT_CC_NUMBER)) {
    VCbridge_in_port = value;
  }
  if ((BLE_connected) && ((VCbridge_in_port == 0) || (VCbridge_in_port == 1))) MIDI_BLE.sendControlChange(control, value, channel);
  if ((RTP_connected) && ((VCbridge_in_port == 0) || (VCbridge_in_port == 2))) MIDI_RTP.sendControlChange(control, value, channel);
}

void OnSysEx(byte *sxdata, unsigned sxlength) {
  if (sxlength < 3) return;
  memcpy(MIDI_sysex_buffer, sxdata, sxlength);
  MIDI_sysex_length = sxlength;
  MIDI_debug_sysex(MIDI_sysex_buffer, MIDI_sysex_length, MIDI_PORT, false);
  if (MIDI_check_SYSEX_in_Teensy(MIDI_sysex_buffer, MIDI_sysex_length)) return;


  if ((BLE_connected) && ((VCbridge_in_port == 0) || (VCbridge_in_port == 1))) MIDI_BLE.sendSysEx(MIDI_sysex_length - 2, &MIDI_sysex_buffer[1]);
  if ((RTP_connected) && ((VCbridge_in_port == 0) || (VCbridge_in_port == 2))) MIDI_RTP.sendSysEx(MIDI_sysex_length - 2, &MIDI_sysex_buffer[1]);
}

void OnClock() {
  if ((BLE_connected) && ((VCbridge_in_port == 0) || (VCbridge_in_port == 1))) MIDI_BLE.sendRealTime(MIDI_NAMESPACE::Clock);
  if ((RTP_connected) && ((VCbridge_in_port == 0) || (VCbridge_in_port == 2))) MIDI_RTP.sendRealTime(MIDI_NAMESPACE::Clock);
}

void OnPitchBend(byte channel, int bend) {
  if ((BLE_connected) && ((VCbridge_in_port == 0) || (VCbridge_in_port == 1))) MIDI_BLE.sendPitchBend(channel, bend);
  if ((RTP_connected) && ((VCbridge_in_port == 0) || (VCbridge_in_port == 2))) MIDI_RTP.sendPitchBend(channel, bend);
}

// Forwarding from MIDI_BLE port
void OnBleNoteOn(byte channel, byte note, byte velocity) {
  MIDI_check_port_message(1);
  DEBUGMIDI("NoteOn #" + String(note) + " with velocity " + String(velocity) + " received on channel " + String(channel)); // Show on serial debug screen
  MIDI1.sendNoteOn(note, velocity, channel);
  flash_LED();
}

void OnBleNoteOff(byte channel, byte note, byte velocity) {
  MIDI_check_port_message(1);
  DEBUGMIDI("NoteOff #" + String(note) + " with velocity " + String(velocity) + " received on channel " + String(channel)); // Show on serial debug screen
  MIDI1.sendNoteOff(note, velocity, channel);
  flash_LED();
}

void OnBleProgramChange(byte channel, byte program) {
  MIDI_check_port_message(1);
  DEBUGMIDI("PC #" + String(program) + " received on channel " + String(channel)); // Show on serial debug screen
  MIDI1.sendProgramChange(program, channel);
  flash_LED();
}

void OnBleControlChange(byte channel, byte control, byte value) {
  MIDI_check_port_message(1);
  DEBUGMIDI("CC #" + String(control) + " Value:" + String(value) + " received on channel " + String(channel)); // Show on serial debug screen
  MIDI1.sendControlChange(control, value, channel);
  flash_LED();
}

void OnBleSysEx(byte *sxdata, unsigned sxlength) {
  MIDI_check_port_message(1);
  memcpy(BLE_sysex_buffer, sxdata, sxlength);
  BLE_sysex_length = sxlength;
  MIDI_debug_sysex(BLE_sysex_buffer, BLE_sysex_length, BLEMIDI_PORT, false);
  if (MIDI_check_SYSEX_in_Teensy(BLE_sysex_buffer, BLE_sysex_length)) return;
  MIDI1.sendSysEx(BLE_sysex_length - 2, &BLE_sysex_buffer[1]);
  flash_LED();
}

void OnBleClock() {
  MIDI_check_port_message(1);
  MIDI1.sendRealTime(MIDI_NAMESPACE::Clock);
  flash_LED();
}

void OnBlePitchBend(byte channel, int bend) {
  MIDI_check_port_message(1);
  DEBUGMIDI("Pitch bend " + String(bend) + " received on channel " + String(channel)); // Show on serial debug screen
  MIDI1.sendPitchBend(channel, bend);
  flash_LED();
}

// Forwarding from MIDI_RTP port
void OnRtpNoteOn(byte channel, byte note, byte velocity) {
  MIDI_check_port_message(2);
  MIDI1.sendNoteOn(note, velocity, channel);
  flash_LED();
}

void OnRtpNoteOff(byte channel, byte note, byte velocity) {
  MIDI_check_port_message(2);
  MIDI1.sendNoteOff(note, velocity, channel);
  flash_LED();
}

void OnRtpProgramChange(byte channel, byte program) {
  MIDI_check_port_message(2);
  MIDI1.sendProgramChange(program, channel);
  flash_LED();
}

void OnRtpControlChange(byte channel, byte control, byte value) {
  MIDI_check_port_message(2);
  MIDI1.sendControlChange(control, value, channel);
  flash_LED();
}

void OnRtpSysEx(byte *sxdata, unsigned sxlength) {
  MIDI_check_port_message(2);
  memcpy(RPT_sysex_buffer, sxdata, sxlength);
  RPT_sysex_length = sxlength;
  MIDI_debug_sysex(RPT_sysex_buffer, RPT_sysex_length, RTPMIDI_PORT, false);
  if (MIDI_check_SYSEX_in_Teensy(RPT_sysex_buffer, RPT_sysex_length)) return;
  MIDI1.sendSysEx(RPT_sysex_length - 2, &RPT_sysex_buffer[1]);
  flash_LED();
}

void OnRtpClock() {
  MIDI_check_port_message(2);
  MIDI1.sendRealTime(MIDI_NAMESPACE::Clock);
  flash_LED();
}

void OnRtpPitchBend(byte channel, int bend) {
  MIDI_check_port_message(2);
  MIDI1.sendPitchBend(channel, bend);
  flash_LED();
}

void MIDI_check_port_message(uint8_t new_port_number) { // Check if we need to tell the VCbridge on the RPi what port to use
  if (new_port_number != VCbridge_out_port) {
    VCbridge_out_port = new_port_number;
    //Send the new port number to the VController
    MIDI1.sendControlChange(LINE_SELECT_CC_NUMBER, new_port_number, VCONTROLLER_MIDI_CHANNEL);

  }
}

bool MIDI_check_SYSEX_in_Teensy(const unsigned char* sxdata, short unsigned int sxlength)
{
  if ((sxdata[1] == VC_MANUFACTURING_ID) && (sxdata[2] == VC_WL_MESSAGE_CODE) && (sxdata[3] == VC_DEVICE_ID)) {
    // Message from the Teensy
    String data = "";
    switch (sxdata[4]) {
      case WL_MODULE_ACTIVE:
        check_BLE_mode(sxdata[5]);
        check_WIFI_mode(sxdata[6]);
        check_RTP_mode(sxdata[7]);
        check_server_mode(sxdata[8]);
        {uint8_t status[3] = { BLE_connected, WIFI_connected, RTP_connected  };
        MIDI_send_data_Teensy(WL_MODULE_ACTIVE, status, 3);}
        return true;
      case WL_BLE_STATUS:
        check_BLE_status();
        return true;
      case WL_WIFI_STATUS:
        SendWifiStatus();
        return true;
      case WL_SET_WIFI_SSID:
        MIDI_read_string(sxdata, sxlength, data);
        EEP_set_ssid(data);
        DEBUGMSG("Received ssid " + data);
        StopWifiServices();
        StartWIFI();
        return true;
      case WL_SET_WIFI_PASSWORD:
        MIDI_read_string(sxdata, sxlength, data);
        EEP_set_ssid_passwd(data);
        StopWifiServices();
        StartWIFI();
        return true;
      case WL_SET_AP_SSID:
        MIDI_read_string(sxdata, sxlength, data);
        EEP_set_ap_ssid(data);
        DEBUGMSG("Received ssid " + data);
        StopWifiServices();
        StartWIFI();
        return true;
      case WL_SET_AP_PASSWORD:
        MIDI_read_string(sxdata, sxlength, data);
        EEP_set_ap_passwd(data);
        StopWifiServices();
        StartWIFI();
        return true;
      case WL_SET_SERVER_ADMIN_PASSWORD:
        MIDI_read_string(sxdata, sxlength, data);
        EEP_set_admin_passwd(data);
        return true;
      case WL_DO_WIFI_SCAN:
        ScanWIFI();
        break;
      default:
        return false;
    }
  }
  return false;
}

void MIDI_send_data_Teensy(uint8_t cmd) {
  uint8_t sysexmessage[6] = { 0xF0, VC_MANUFACTURING_ID, VC_WL_MESSAGE_CODE, VC_DEVICE_ID, cmd, 0xF7 };
  MIDI1.sendSysEx(4, &sysexmessage[1]);
}

void MIDI_send_data_Teensy(uint8_t cmd, uint8_t *my_data, uint16_t my_len) {
  uint16_t messagesize = my_len + 6;
  uint8_t sysexmessage[messagesize] = { 0xF0, VC_MANUFACTURING_ID, VC_WL_MESSAGE_CODE, VC_DEVICE_ID, cmd };
  for (uint8_t i = 0; i < my_len; i++) {
    sysexmessage[i + 5] = my_data[i] & 0x7F;
  }
  sysexmessage[messagesize - 1] = 0xF7;
  MIDI1.sendSysEx(messagesize - 2, &sysexmessage[1]);
}

void MIDI_read_string(const unsigned char* sxdata, short unsigned int sxlength, String &data) {
  for (uint8_t i = 5; i < sxlength - 1; i++) data += (char) sxdata[i];
}

void MIDI_send_ble_state(uint8_t state) {
  MIDI_send_data_Teensy(WL_BLE_STATUS, &state, 1);
}

void MIDI_send_wifi_state(uint8_t state) {
  MIDI_send_data_Teensy(WL_WIFI_STATUS, &state, 1);
}

void MIDI_send_rtpmidi_state(uint8_t state) {
  MIDI_send_data_Teensy(WL_RTPMIDI_STATUS, &state, 1);
}

void MIDI_send_ssid(uint8_t number, uint8_t total, uint8_t rssi, String ssid) {
  uint16_t messagesize = ssid.length() + 3;
  uint8_t msg[messagesize] = { number, total, rssi };
  for (uint8_t i = 0; i < ssid.length(); i++) {
    msg[i + 3] = (uint8_t) ssid[i];
  }
  MIDI_send_data_Teensy(WL_SEND_SCANNED_SSID, msg, messagesize);
}

void MIDI_send_current_ssid(uint8_t ssid_no) {
  MIDI_send_data_Teensy(WL_SET_CURRENT_SSID, &ssid_no, 1);
}
