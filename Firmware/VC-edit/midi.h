#ifndef MIDI_H
#define MIDI_H

// Class for all midi communication with the VController.
// This class makes use of the rtmidi library.

#include "RtMidi.h"
#include <QDialog>
#include <QString>
#include <QProgressBar>
#include "VController/config.h"

// Midi IDs for sysex messages of the VController
#define VC_MANUFACTURING_ID 0x7D // Universal for simple midi device
#define VC_FAMILY_CODE 0x68 // Family code for Sixeight's products
#define VC_DEVICE_ID 0x01

// Midi commands for VController
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
#define VC_REQUEST_KATANA_PATCHES 14
#define VC_SET_DEVICE_PATCH 15
#define VC_SAVE_SETTINGS 16
#define VC_FINISH_DEVICE_PATCH_DUMP 17
#define VC_REQUEST_SEQ_PATTERNS 18
#define VC_SET_SEQ_PATTERN 19
#define VC_INITIALIZE_DEVICE_PATCH 20
#define VC_REQUEST_HARDWARE_VERSION 21
#define VC_START_UPDATE_MODE 22
#define VC_SAVE_USER_DEVICE_SETTINGS 23
#define VC_SAVE_USER_DEVICE_NAME_ITEM 24
#define VC_REQUEST_ALL_USER_DEVICE_SETTINGS 25
#define VC_INITIALIZE_USER_DEVICE_DATA 26
#define VC_SELECT_PATCH_FROM_EDITOR 27

class Midi : public QObject
{
    Q_OBJECT

public:
    explicit Midi(QObject *parent = 0);
    void openMidiIn(QString port);
    void openMidiOut(QString port);
    void closeMidiIn();
    void closeMidiOut();
    void checkForVCmidi();
    bool checkMidiPortStillAvailable(QString inPort, QString outPort);
    QStringList fillMidiInPortItems();
    QStringList fillMidiOutPortItems();
    void sendSysexCommand(int size, ...);
    void checkMidiIn(std::vector<unsigned char> *message);
    void MIDI_editor_request_settings();
    void MIDI_editor_request_all_commands();
    void MIDI_editor_request_all_patches();
    void MIDI_editor_request_sequence_patterns();
    void MIDI_send_data(uint8_t cmd, uint8_t *my_data, uint16_t my_len);
    void MIDI_editor_send_settings();
    void MIDI_editor_send_device_settings(uint8_t dev);
    void MIDI_editor_send_midi_switch_settings(uint8_t sw);
    void MIDI_editor_send_seq_pattern(uint8_t pattern);
    void MIDI_editor_send_save_settings();
    void MIDI_editor_send_finish_commands_dump();
    void MIDI_editor_send_start_commands_dump();
    void MIDI_editor_send_command(uint16_t cmd_no);
    void MIDI_send_device_patch(uint16_t patch_no);
    void MIDI_send_initialize_device_patch(uint16_t patch_no);
    void MIDI_editor_finish_device_patch_dump();
    void MIDI_editor_request_hardware_version();
    void MIDI_editor_request_all_user_device_settings();
    void MIDI_editor_send_all_user_device_data();
    void send_universal_identity_request();
    void MIDI_select_patch_on_device(uint8_t dev, uint16_t patch);

signals:
    void updateSettings();
    void updateCommands(int, int);
    void updateLcdDisplay(int, QString, QString);
    void updateLED(int, int);
    void startProgressBar(int, QString);
    void updateProgressBar(int);
    void closeProgressBar(QString);
    void VControllerDetected(int type, int versionMajor, int versionMinor, int versionBuild);
    void updatePatchListBox();
    void updateUserDeviceTab();

private:
    static void staticMidiCallback(double, std::vector< unsigned char > *message, void *userData);
    RtMidiIn* _midiIn = 0;
    RtMidiOut* _midiOut = 0;
    //QString sysxBuffer;
    QString addChar(unsigned char c);
    void MIDI_read_data(std::vector< unsigned char > *message, uint8_t *my_data, uint16_t my_len);
    void MIDI_debug_data(std::vector<unsigned char> *message, bool isMidiIn);
    void MIDI_show_error();
    void MIDI_editor_receive_settings(std::vector< unsigned char > *message);
    void MIDI_editor_receive_device_settings(std::vector< unsigned char > *message);
    void MIDI_editor_receive_start_commands_dump(int size);
    void MIDI_editor_receive_finish_commands_dump(std::vector< unsigned char > *message);
    void MIDI_editor_receive_command(std::vector< unsigned char > *message);
    void MIDI_editor_receive_midi_switch_settings(std::vector< unsigned char > *message);
    void MIDI_editor_receive_seq_pattern(std::vector< unsigned char > *message);
    void MIDI_editor_receive_device_patch(std::vector< unsigned char > *message);
    void MIDI_editor_receive_initialize_device_patch(std::vector< unsigned char > *message);
    void MIDI_editor_receive_finish_device_patch_dump(std::vector< unsigned char > *message);
    void MIDI_editor_receive_user_device_settings(std::vector< unsigned char > *message);
    void MIDI_editor_receive_user_name_item(std::vector< unsigned char > *message);
    bool receive_7_bit_overflow_data(QByteArray *data, uint16_t datalen, std::vector<unsigned char> *message);
    QByteArray ReadPatch(int number);
    void WritePatch(int number, QByteArray patch);
    void InitializePatch(int number);
    void send_7_bit_overflow_data(QByteArray data, uint16_t datalen, uint8_t command, uint16_t index);
};

#endif // MIDI_H
