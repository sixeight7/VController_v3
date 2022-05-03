// Please read VCCtouch_wireless.ino for information about the license and authors

#include <Preferences.h>

#define CURRENT_EEPROM_VERSION 1

Preferences preferences;

void setup_eeprom() {
  preferences.begin("VC-touch", false);
  int version = preferences.getUInt("version", 0);
  if (version != CURRENT_EEPROM_VERSION) EEP_initialize_internal_eeprom_data();
  preferences.end();
}

void EEP_initialize_internal_eeprom_data() {
  DEBUGMAIN("Initializing EEPROM data...");
  preferences.clear();
  preferences.putUInt("version", CURRENT_EEPROM_VERSION);
  preferences.putUInt("ble_mode", 1);
  preferences.putUInt("wifi_mode", 1);
  preferences.putBool("rtp_enabled", true);
  preferences.putBool("server_enabled", true);
  preferences.putString("ssid", "Fednet1");
  preferences.putString("ssid_passwd", "a=KJzmidwmzn!");
  preferences.putString("ap_ssid", "VC-touch");
  preferences.putString("ap_passwd", "vguitar123");
  preferences.putString("admin_passwd", "12345");
}

void EEP_set_ssid(String ssid) {
  preferences.begin("VC-touch", false);
  preferences.remove("ssid");
  preferences.putString("ssid", ssid);
  preferences.end();
}

void EEP_read_ssid(String &ssid) {
  preferences.begin("VC-touch", false);
  ssid = preferences.getString("ssid", "");
  preferences.end();
}

void EEP_set_ssid_passwd(String ssid_passwd) {
  preferences.begin("VC-touch", false);
  preferences.remove("ssid_passwd");
  preferences.putString("ssid_passwd", ssid_passwd);
  preferences.end();
}

void EEP_read_ssid_passwd(String &ssid_passwd) {
  preferences.begin("VC-touch", false);
  ssid_passwd = preferences.getString("ssid_passwd", "");
  preferences.end();
}

void EEP_set_ap_ssid(String ssid) {
  preferences.begin("VC-touch", false);
  preferences.remove("ap_ssid");
  preferences.putString("ap_ssid", ssid);
  preferences.end();
}

void EEP_read_ap_ssid(String &ssid) {
  preferences.begin("VC-touch", false);
  ssid = preferences.getString("ap_ssid", "");
  preferences.end();
}

void EEP_set_ap_passwd(String ssid_passwd) {
  preferences.begin("VC-touch", false);
  preferences.remove("ap_passwd");
  preferences.putString("ap_passwd", ssid_passwd);
  preferences.end();
}

void EEP_read_ap_passwd(String &ssid_passwd) {
  preferences.begin("VC-touch", false);
  ssid_passwd = preferences.getString("ap_passwd", "vguitar123");
  preferences.end();
}

void EEP_set_admin_passwd(String admin_passwd) {
  preferences.begin("VC-touch", false);
  preferences.remove("admin_passwd");
  preferences.putString("admin_passwd", admin_passwd);
  preferences.end();
}

void EEP_read_admin_passwd(String &admin_passwd) {
  preferences.begin("VC-touch", false);
  admin_passwd = preferences.getString("admin_passwd", "unKN0WN!");
  preferences.end();
}

void EEP_set_ble_mode(uint8_t mode) {
  preferences.begin("VC-touch", false);
  preferences.putUInt("ble_mode", mode);
  preferences.end();
}

uint8_t EEP_read_ble_mode() {
  preferences.begin("VC-touch", false);
  uint8_t mode = preferences.getUInt("ble_mode");
  preferences.end();
  return mode;
}

void EEP_set_wifi_mode(uint8_t mode) {
  preferences.begin("VC-touch", false);
  preferences.putUInt("wifi_mode", mode);
  preferences.end();
}

uint8_t EEP_read_wifi_mode() {
  preferences.begin("VC-touch", false);
  uint8_t mode = preferences.getUInt("wifi_mode");
  preferences.end();
  return mode;
}

void EEP_set_rtpmidi_enabled(bool enabled) {
  preferences.begin("VC-touch", false);
  preferences.putBool("rtp_enabled", enabled);
  preferences.end();
}

bool EEP_read_rtpmidi_enabled() {
  preferences.begin("VC-touch", false);
  bool enabled =  preferences.getBool("rtp_enabled");
  preferences.end();
  return enabled;
}

void EEP_set_server_enabled(bool enabled) {
  preferences.begin("VC-touch", false);
  preferences.putBool("rtp_enabled", enabled);
  preferences.end();
}

bool EEP_read_server_enabled() {
  preferences.begin("VC-touch", false);
  bool enabled =  preferences.getBool("rtp_enabled");
  preferences.end();
  return enabled;
}

/*#include <EEPROM.h>
  #define EEPROM_SIZE 512

  #define CURRENT_EEPROM_VERSION 1

  // ************************ Internal EEPROM data addresses ***********************
  // Total size: 512 bytes (ESP32)
  #define EEPROM_VERSION_ADDR 0x00 // Address!!! Version update must be done above!!!
  #define EEPROM_SSID_ADDR 0x10
  #define EEPROM_SSID_PASSWD_ADDR 0x28
  #define EEPROM_ADMIN_PASSWD_ADDR 0x40

  #define EEP_MAX_SIZE 24

  void setup_eeprom() {
  if (!EEPROM.begin(EEPROM_SIZE))
  {
    Serial.println("failed to initialise EEPROM"); delay(10000);
  }

  if (EEPROM.read(EEPROM_VERSION_ADDR) != CURRENT_EEPROM_VERSION) EEP_initialize_internal_eeprom_data();
  }

  void EEP_initialize_internal_eeprom_data() {
  EEPROM_write(EEPROM_VERSION_ADDR, CURRENT_EEPROM_VERSION); // Save the current eeprom version
  EEP_write_string(EEPROM_SSID_ADDR, "SSID");
  EEP_write_string(EEPROM_SSID_PASSWD_ADDR, "WIFI_pass");
  EEP_write_string(EEPROM_ADMIN_PASSWD_ADDR, "12345");
  }

  void EEP_write_string(uint16_t addr, String data) {
  DEBUGMSG("EEP writing to address " + String(addr) + ", data: " + data);
  uint8_t len = data.length();
  if (len > EEP_MAX_SIZE) len = EEP_MAX_SIZE;
  for (uint8_t i = 0; i < len; i++) EEPROM_write(addr++, data[i]);
  for (uint8_t i = len; i < EEP_MAX_SIZE; i++) EEPROM_write(addr++, ' ');
  //EEPROM.commit();
  }

  void EEP_read_string(uint16_t addr, String data) {
  DEBUGMSG("EEP data at address " + String(addr));
  for (uint8_t i = 0; i < EEP_MAX_SIZE; i++) data[i] = EEPROM.read(addr++);
  DEBUGMSG(data);
  }

  void EEPROM_write(uint16_t address, uint8_t value) { // Only write data if it has changed
  if (value != EEPROM.read(address)) {
    EEPROM.write(address, value);
    EEPROM.commit();
    DEBUGMSG("writing " + String(value) + " to addr: " + String(address));
  }
  }*/
