// AppleMidi communication

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <WiFiUdp.h>
#include <AppleMIDI.h>

const char* host = "VC-touch";

APPLEMIDI_CREATE_INSTANCE(WiFiUDP, MIDI_RTP, host, 5004);

unsigned long t1 = millis();

#define WIFI_STATE_OFF 0
#define WIFI_STATE_CLIENT 1
#define WIFI_STATE_AP 2
#define WIFI_STATE_CLIENT_AP 3
uint8_t WIFI_mode = WIFI_STATE_OFF;

bool WIFI_connected = false;
bool RTP_enabled = false;
int8_t RTP_connected = 0;

void setup_WIFI() {
  ScanWIFI();
}

void run_WIFI() {
  if (WIFI_mode == WIFI_STATE_CLIENT) CheckWifiConnected();
  if (RTP_enabled) MIDI_RTP.read();
}

void ScanWIFI() {
  StopWifiServices();
  DEBUGMAIN("Start scan of WIFI networks");
  WiFi.mode(WIFI_STA);
  int n = WiFi.scanNetworks();
  DEBUGMAIN("scan done");
  if (n == 0) {
    MIDI_send_ssid(0, 0,  0, "");
    DEBUGMAIN("no networks found");
  }
  else
  {
    DEBUGMAIN(String(n) + " networks found");
    String current_ssid = "";
    EEP_read_ap_ssid(current_ssid);
    for (int i = 0; i < n; ++i) {
      long int rssi = abs(WiFi.RSSI(i)) & 0x7F;
      MIDI_send_ssid(i, n,  rssi, WiFi.SSID(i));
      if (WiFi.SSID(i) == current_ssid) MIDI_send_current_ssid(i);
      DEBUGMAIN( String(i + 1) + ": " + WiFi.SSID(i) + " (" + WiFi.RSSI(i) + ")" );
    }
  }
  StartWIFI();
}

void StartWIFI() {
  WIFI_mode = EEP_read_wifi_mode();
  if (WIFI_mode == WIFI_STATE_CLIENT) StartWifiConnect();
  if (WIFI_mode == WIFI_STATE_AP) StartWifiAP();
}

void StartWifiConnect() {
  if (WiFi.getMode() != WIFI_STA) WiFi.disconnect(true);
  String ssid = "";
  String pass = "";
  EEP_read_ssid(ssid);
  EEP_read_ssid_passwd(pass);
  ssid.trim();
  pass.trim();
  DEBUGMAIN("Connecting WIFI to SSID: " + ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());
}

void StartWifiAP() {
  if (WiFi.getMode() != WIFI_AP) WiFi.disconnect(true);
  String ssid = "";
  String pass = "";
  EEP_read_ap_ssid(ssid);
  EEP_read_ap_passwd(pass);
  ssid.trim();
  pass.trim();
  DEBUGMAIN("Starting WIFI AP with SSID: " + ssid + " and password: " + pass);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid.c_str(), pass.c_str());
  StartWifiServices();
}

void CheckWifiConnected() {
  bool new_state = (WiFi.status() == WL_CONNECTED);
  if (new_state == WIFI_connected) return;
  if (new_state == true) {
    DEBUGMAIN("WIFI online");
    StartWifiServices();
  }
  else {
    DEBUGMAIN("WIFI offline");
    StopWifiServices();
    StartWIFI(); // See if we can get it back
  }

}

void SendWifiStatus() {
  if (WiFi.status() == WL_CONNECTED) MIDI_send_wifi_state(1);
  else MIDI_send_wifi_state(0);
}

void StartWifiServices() {
  if (EEP_read_rtpmidi_enabled()) StartAppleMIDI();
  if (EEP_read_server_enabled()) start_server();
  WIFI_connected = true;
  MIDI_send_wifi_state(1);
}

void StopWifiServices() {
  if (EEP_read_server_enabled()) stop_server();
  WiFi.mode(WIFI_OFF);
  WIFI_connected = false;
  MIDI_send_wifi_state(0);
}

void check_WIFI_mode(uint8_t mode) {
  if (mode != EEP_read_wifi_mode()) {
    EEP_set_wifi_mode(mode);
    if (mode == 0) StopWifiServices();
    else StartWifiServices();
  }
}

void StartAppleMIDI() {
  RTP_enabled = true;
  MIDI_RTP.begin();
  MIDI_RTP.turnThruOff();
  MIDI_RTP.setHandleNoteOff(OnRtpNoteOff);
  MIDI_RTP.setHandleNoteOn(OnRtpNoteOn) ;
  MIDI_RTP.setHandleProgramChange(OnRtpProgramChange);
  MIDI_RTP.setHandleControlChange(OnRtpControlChange);
  MIDI_RTP.setHandleSystemExclusive(OnRtpSysEx);
  MIDI_RTP.setHandleClock(OnRtpClock);
  MIDI_RTP.setHandlePitchBend(OnRtpPitchBend);

  AppleMIDI_RTP.setHandleConnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char* name) {
    RTP_connected++;
    digitalWrite(LED_BUILTIN, HIGH);
    led_on = true;
    DEBUGMAIN("RTPMIDI connected to session" + String(ssrc) + name);
    MIDI_send_rtpmidi_state(1);
  });
  AppleMIDI_RTP.setHandleDisconnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
    RTP_connected--;
    if (RTP_connected <= 0) {
      digitalWrite(LED_BUILTIN, LOW);
      led_on = true;
      MIDI_send_rtpmidi_state(0);
    }
    DEBUGMAIN("RTPMIDI disconnected " + String(ssrc));
  });
}

void check_RTP_mode(bool enabled) {
  if (enabled != EEP_read_rtpmidi_enabled()) {
    EEP_set_rtpmidi_enabled(enabled);
    if (enabled) StartAppleMIDI();
    else RTP_enabled = false;
  }
}
