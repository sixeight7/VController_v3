#include "midi.h"
#include "VController/config.h"
#include "VController/globals.h"
#include "VController/globaldevices.h"
#include "devices/katana.h"
#include <QDebug>
#include <QThread>

Midi::Midi(QObject *parent) : QObject(parent),
    _midiIn(new RtMidiIn), _midiOut(new RtMidiOut)
{
    _midiIn->setCallback(&Midi::staticMidiCallback, (void *)this); // register callback and let it know where it has to go
}

void Midi::openMidiIn(QString port) {
    //std::cout << "Try to connect to port" ;
    unsigned int numInPorts = _midiIn->getPortCount();
    if(_midiIn && !_midiIn->isPortOpen() && (numInPorts > 0)) {
        int port_number = 255;
        for ( unsigned int i = 0; i < numInPorts; i++ ) {
            if (port == QString::fromStdString(_midiIn->getPortName(i)))
                port_number = i;
        }
        if (port_number == 255) {
            std::cout << "Midi in port not available" ;
            return;
        }
        _midiIn->openPort(port_number);
        _midiIn->ignoreTypes( false, true, true ); // Ignore timing and active sensing, but not sysex
        std::cout << "Connected to MIDI in port" << _midiIn->getPortName();
    }
}

void Midi::openMidiOut(QString port){
    //std::cout << "Try to connect to port" ;
    unsigned int numOutPorts = _midiOut->getPortCount();
    if(_midiOut && !_midiOut->isPortOpen() && (numOutPorts > 0)) {
        int port_number = 255;
        for ( unsigned int i = 0; i < numOutPorts; i++ ) {
            if (port == QString::fromStdString(_midiOut->getPortName(i)))
                port_number = i;
        }
        if (port_number == 255) {
            std::cout << "Midi in port not available" ;
            return;
        }
        _midiOut->openPort(port_number);
        std::cout << "Connected to MIDI out port" << _midiOut->getPortName();
    }
}

QStringList Midi::fillMidiInPortItems() { // Will return a list of the input ports
  std::string portName;
  QStringList items;

  unsigned int numInPorts = _midiIn->getPortCount();

  // retrieve all names
  if(numInPorts > 0)
  {
    for ( unsigned int i = 0; i < numInPorts; i++ )
    {
        try {
          portName = _midiIn->getPortName(i);
          items<<QString::fromStdString(portName);
        }
        catch ( RtMidiError &error ) {
          error.printMessage();
          std::cout<<"Error getting input ports."<<std::endl;
        }
        std::cout << "Input Port #" << i+1 << ": " << portName << '\n';
     }

  } else {
      std::cout<<"No input ports available."<<std::endl;
  }
  return items;
}

QStringList Midi::fillMidiOutPortItems() { // Will return a list of the output ports
  std::string portName;
  QStringList items;
  unsigned int numOutPorts = _midiOut->getPortCount();

  // retrieve all names
  if(numOutPorts > 0)
  {
    for ( unsigned int i = 0; i < numOutPorts; i++ )
    {
        try {
          portName = _midiOut->getPortName(i);
          items<<QString::fromStdString(portName);
        }
        catch ( RtMidiError &error ) {
          error.printMessage();
          std::cout<<"Error getting Output ports."<<std::endl;
        }
        std::cout << "Output Port #" << i+1 << ": " << portName << '\n';
     }

  } else {
      std::cout<<"No Output ports available."<<std::endl;
  }
  return items;
}

void Midi::sendSysexCommand(int size, ...) {
    if (_midiOut && !_midiOut->isPortOpen()) {
        std::cout<<"No Output port open."<<std::endl;
        return; // Exit if port is not open.
    }

    va_list data;

    std::vector<unsigned char> message;
    message.clear();
    message.push_back( 0xF0 );
    message.push_back( VC_MANUFACTURING_ID );
    message.push_back( VC_FAMILY_CODE );
    message.push_back( VC_MODEL_NUMBER );
    message.push_back( VC_DEVICE_ID );
    va_start(data, size);
    for(int i = 0; i < size; i++)
        message.push_back( va_arg(data, int ));
    message.push_back( 0xF7 );
    _midiOut->sendMessage(&message);
}

void Midi::staticMidiCallback(double, std::vector<unsigned char> *message, void *userData) // Static member!
{
    Midi* midiInstance = (Midi*) userData; // We have set userData as a pointer to the instance of the class
    if ((message->size() > 0) && (midiInstance != NULL))
    { midiInstance->checkMidiIn(message); }; // Here we are redirected back to the correct instance of this Class
}

void Midi::checkMidiIn(std::vector<unsigned char> *message)
{
    QString line1 = "";
    QString line2 = "";
    int number_of_leds = 0;

    //MIDI_debug_data(message, true);

    if (message->size() < 7) return;

    // Check if it is a universal response message from the VController
    // VC-mini will send F0 7E 01 06 02 7D 00 68 00 02 (03 03 05) 00 F7 - version number between brackets
    if ((message->at(0) == 0xF0) && (message->at(1) == 0x7E) && (message->at(2) == VC_DEVICE_ID) && (message->at(3) == 0x06)
            && (message->at(4) == 0x02) && (message->at(5) == VC_MANUFACTURING_ID) && (message->at(7) == VC_FAMILY_CODE)) {
        emit VControllerDetected(message->at(9), message->at(10), message->at(11), message->at(12));
        qDebug() << "VController detected";
    }

    // Check if sysexmessage is from the VController
    if ((message->at(0) == 0xF0) && (message->at(1) == VC_MANUFACTURING_ID) && (message->at(2) == VC_FAMILY_CODE) &&
            (message->at(3) == VC_MODEL_NUMBER) && (message->at(4) == VC_DEVICE_ID)) {
        MIDI_debug_data(message, true);
        switch (message->at(5)) { // Check for the sysex command
        case VC_REMOTE_UPDATE_DISPLAY:
            for (int i = 0; i < 16; i++) { // Read line 1
                line1.append(addChar(message->at(i + 7)));
            }
            for (int i = 0; i < 16; i++) { // Read line 2
                line2.append(addChar(message->at(i + 23)));
            }
            emit updateLcdDisplay(message->at(6), line1, line2);
            break;
        case VC_REMOTE_UPDATE_LEDS:
            number_of_leds = message->at(6);
            for (int i = 0; i < number_of_leds; i++) {
                emit updateLED(i + 1, message->at(i + 7));
            }
            break;
        case VC_SET_GENERAL_SETTINGS:
            MIDI_editor_receive_settings(message);
            break;
        case VC_START_COMMANDS_DUMP:
            MIDI_editor_receive_start_commands_dump();
            break;
        case VC_SET_DEVICE_SETTINGS:
            MIDI_editor_receive_device_settings(message);
            break;
        case VC_SET_MIDI_SWITCH_SETTINGS:
            MIDI_editor_receive_midi_switch_settings(message);
            break;
        case VC_FINISH_COMMANDS_DUMP:
            MIDI_editor_receive_finish_commands_dump(message);
            break;
        case VC_SET_COMMAND:
            MIDI_editor_receive_command(message);
            break;
        case VC_SET_KATANA_PATCH:
            MIDI_editor_receive_KTN_patch(message);
            break;
        case VC_FINISH_KATANA_PATCH_DUMP:
            MIDI_editor_receive_finish_KTN_patch_dump(message);
            break;
        }
    }

}

void Midi::MIDI_editor_request_settings()
{
    sendSysexCommand(1, VC_REQUEST_ALL_SETTINGS);
}

void Midi::MIDI_editor_request_all_commands()
{
    //Commands.clear();
    sendSysexCommand(1, VC_REQUEST_COMMANDS_DUMP);
}

void Midi::MIDI_editor_request_all_KTN_patches()
{
    sendSysexCommand(1, VC_REQUEST_KATANA_PATCHES);
}



void Midi::MIDI_editor_send_settings()
{
    uint8_t* settingbytes = (uint8_t*)&Setting;
    MIDI_send_data(VC_SET_GENERAL_SETTINGS, settingbytes, sizeof(Setting));
}

void Midi::MIDI_editor_send_device_settings(uint8_t dev)
{
   if (dev >= NUMBER_OF_DEVICES) return;

   uint8_t dsettings[NUMBER_OF_DEVICE_SETTINGS + 1];
   dsettings[0] = dev;
   for (uint8_t i = 0; i < NUMBER_OF_DEVICE_SETTINGS; i++) {
       dsettings[i + 1] = Device[dev]->get_setting(i);
   }

   MIDI_send_data(VC_SET_DEVICE_SETTINGS, dsettings, NUMBER_OF_DEVICE_SETTINGS + 1);

}

void Midi::MIDI_editor_send_midi_switch_settings(uint8_t sw)
{
    if (sw >= NUMBER_OF_MIDI_SWITCHES) return;

    uint8_t mssettings[5];
    mssettings[0] = sw;
    mssettings[1] = MIDI_switch[sw].type;
    mssettings[2] = MIDI_switch[sw].port;
    mssettings[3] = MIDI_switch[sw].channel;
    mssettings[4] = MIDI_switch[sw].cc;
    MIDI_send_data(VC_SET_MIDI_SWITCH_SETTINGS, mssettings, 5);
}

void Midi::MIDI_editor_send_save_settings()
{
    uint8_t dummy[1] = {0};
    MIDI_send_data(VC_SAVE_SETTINGS, dummy, 1);
}

void Midi::MIDI_editor_send_finish_commands_dump()
{
    if (_midiOut && !_midiOut->isPortOpen()) return; // Exit if port is not open.

    std::vector<unsigned char> message;
    message.clear();
    message.push_back( 0xF0 );
    message.push_back( VC_MANUFACTURING_ID );
    message.push_back( VC_FAMILY_CODE );
    message.push_back( VC_MODEL_NUMBER );
    message.push_back( VC_DEVICE_ID );
    message.push_back( VC_FINISH_COMMANDS_DUMP );
    message.push_back((uint8_t)((Number_of_pages >> 7) & 0x7F));
    message.push_back((uint8_t)(Number_of_pages & 0x7F));
    message.push_back((uint8_t)((Commands.size() >> 7) & 0x7F));
    message.push_back((uint8_t)(Commands.size() & 0x7F));
    message.push_back( 0xF7 );
    _midiOut->sendMessage(&message);
    MIDI_debug_data(&message, false);
}

void Midi::MIDI_editor_send_start_commands_dump()
{
    sendSysexCommand(3, VC_START_COMMANDS_DUMP, (uint8_t)((Commands.size() >> 7) & 0x7F), (uint8_t)(Commands.size() & 0x7F));
}

void Midi::MIDI_editor_send_command(uint16_t cmd_no)
{
    Cmd_struct cmd = Commands[cmd_no];
    uint8_t* cmdbytes = (uint8_t*)&cmd;
    MIDI_send_data(VC_SET_COMMAND, cmdbytes, sizeof(cmd));
}

void Midi::MIDI_send_KTN_patch(uint8_t patch_no)
{
    if (_midiOut && !_midiOut->isPortOpen()) return; // Exit if port is not open.

    std::vector<unsigned char> message;
    message.clear();
    message.push_back( 0xF0 );
    message.push_back( VC_MANUFACTURING_ID );
    message.push_back( VC_FAMILY_CODE );
    message.push_back( VC_MODEL_NUMBER );
    message.push_back( VC_DEVICE_ID );
    message.push_back( VC_SET_KATANA_PATCH);
    message.push_back( patch_no );
    QByteArray patch = My_KTN.ReadPatch(patch_no);
    for (uint8_t i = 0; i < KTN_PATCH_SIZE; i++) {
        message.push_back(patch[i] & 0x7F);
    }
    message.push_back( 0xF7 );
    _midiOut->sendMessage(&message);
    MIDI_debug_data(&message, false);
    QThread::msleep(10);
}

void Midi::MIDI_editor_send_finish_KTN_patch_dump()
{
    uint8_t dummy[1] = {0};
    MIDI_send_data(VC_FINISH_KATANA_PATCH_DUMP, dummy, 1);
}

void Midi::send_universal_identity_request()
{
    // Format: 0xF0, 0x7E, 0x7F, 0x06, 0x01, 0xF7

    std::vector<unsigned char> message;
    message.clear();
    message.push_back( 0xF0 );
    message.push_back( 0x7E );
    message.push_back( 0x7F );
    message.push_back( 0x06 );
    message.push_back( 0x01 );
    message.push_back( 0xF7 );
    _midiOut->sendMessage(&message);

    qDebug() << "Sent Universal Identity Request";
}

QString Midi::addChar(unsigned char c) {
    switch ((int)c) {
      case 5: return "^";
      case 6: return "v";
      case 7: return " ";
      default: return (QChar)c;
    }
}

void Midi::MIDI_send_data(uint8_t cmd, uint8_t *my_data, uint16_t my_len)
{
    if (_midiOut && !_midiOut->isPortOpen()) return; // Exit if port is not open.

    std::vector<unsigned char> message;
    message.clear();
    message.push_back( 0xF0 );
    message.push_back( VC_MANUFACTURING_ID );
    message.push_back( VC_FAMILY_CODE );
    message.push_back( VC_MODEL_NUMBER );
    message.push_back( VC_DEVICE_ID );
    message.push_back( cmd );
    for (uint8_t i = 0; i < my_len; i++) {
        message.push_back(my_data[i] >> 7);
        message.push_back(my_data[i] & 0x7F);
    }
    message.push_back( 0xF7 );
    _midiOut->sendMessage(&message);
    MIDI_debug_data(&message, false);
    QThread::msleep(10);
}

void Midi::MIDI_read_data(std::vector<unsigned char> *message, uint8_t *my_data, uint16_t my_len)
{
    for (uint8_t i = 0; i < my_len; i++) {
        uint8_t pos = (i * 2) + 6;
        my_data[i] = (message->at(pos) << 7) + message->at(pos + 1);
    }
}

void Midi::MIDI_debug_data(std::vector<unsigned char> *message, bool isMidiIn)
{
    QString messageString;
    if (isMidiIn) messageString = "In: ";
    else messageString = "Out:";
    int size = message->size();
    for (int i = 0; i < size; i++) {
        int n = ((int)message->at(i));					// convert std::vector to QString
        QString hex = QString::number(n, 16).toUpper();
        if (hex.length() < 2) hex.prepend("0");
        messageString.append(hex);
        messageString.append(" ");
    }
    qDebug() << messageString;
}

void Midi::MIDI_show_error()
{
    qDebug() << "MIDI read error!";
}

void Midi::MIDI_editor_receive_device_settings(std::vector<unsigned char> *message)
{
    if (message->size() != NUMBER_OF_DEVICE_SETTINGS * 2 + 9) {
       MIDI_show_error();
       return;
    }
    uint8_t dsettings[NUMBER_OF_DEVICE_SETTINGS + 1];
    MIDI_read_data(message, dsettings, sizeof(dsettings));
    uint8_t dev = dsettings[0];

    if (dev < NUMBER_OF_DEVICES) {
        for (uint8_t i = 0; i < NUMBER_OF_DEVICE_SETTINGS; i++) {
            Device[dev]->set_setting(i, dsettings[i + 1]);
        }
        emit updateProgressBar(dev);
        //if (dev >= NUMBER_OF_DEVICES - 1) emit closeProgressBar("Settings download succesful");
    }
}

void Midi::MIDI_editor_receive_start_commands_dump()
{
    emit startProgressBar(Commands.size(), "Downloading MIDI commands");
    Commands.clear();
}

void Midi::MIDI_editor_receive_finish_commands_dump(std::vector<unsigned char> *message)
{
    if (message->size() != 11) {
        MIDI_show_error();
        return;
      }
      uint8_t check_number_of_pages = (message->at(6) << 7) + message->at(7);
      uint16_t check_number_of_cmds = (message->at(8) << 7) + message->at(9);

      emit updateCommands(check_number_of_cmds, check_number_of_pages);
      emit closeProgressBar("Midi commands received succesfully");
}

void Midi::MIDI_editor_receive_command(std::vector<unsigned char> *message)
{
  Cmd_struct cmd;
  uint8_t* cmdbytes = (uint8_t*)&cmd;
  MIDI_read_data(message, cmdbytes, sizeof(cmd));
  Commands.append(cmd);
  emit updateProgressBar(Commands.size());
}

void Midi::MIDI_editor_receive_midi_switch_settings(std::vector<unsigned char> *message)
{
    if (message->size() != 4 * 2 + 9) {
       MIDI_show_error();
       return;
    }
    uint8_t mssettings[5];
    MIDI_read_data(message, mssettings, sizeof(mssettings));
    uint8_t sw = mssettings[0];

    if (sw < NUMBER_OF_MIDI_SWITCHES) {
        MIDI_switch[sw].type = mssettings[1];
        MIDI_switch[sw].port = mssettings[2];
        MIDI_switch[sw].channel = mssettings[3];
        MIDI_switch[sw].cc = mssettings[4];
        emit updateProgressBar(sw + NUMBER_OF_DEVICES);
        if (sw >= NUMBER_OF_MIDI_SWITCHES - 1) emit closeProgressBar("Settings download succesful");
    }
}

void Midi::MIDI_editor_receive_KTN_patch(std::vector<unsigned char> *message)
{
    if (message->size() != KTN_PATCH_SIZE + 8) {
        MIDI_show_error();
        return;
    }
    QByteArray patch;
    int patch_no = message->at(6);
    for (int i = 0; i < KTN_PATCH_SIZE; i++) {
        int pos = i + 7;
        patch.append(message->at(pos));
    }
    My_KTN.WritePatch(patch_no, patch);
    if (patch_no == 0) emit startProgressBar(NUMBER_OF_DEVICES + NUMBER_OF_MIDI_SWITCHES, "Receiving Katana patches");
    emit updateProgressBar(patch_no);
    emit updatePatchListBox();
}

void Midi::MIDI_editor_receive_finish_KTN_patch_dump(std::vector<unsigned char> *message)
{
    emit closeProgressBar("Patch download succesful");
    emit updatePatchListBox();
}

void Midi::MIDI_editor_receive_settings(std::vector<unsigned char> *message)
{
    if (message->size() != sizeof(Setting) * 2 + 7) {
        MIDI_show_error();
        return;
    }
    uint8_t* settingbytes = (uint8_t*)&Setting;
    MIDI_read_data(message, settingbytes, sizeof(Setting));
    emit startProgressBar(NUMBER_OF_DEVICES + NUMBER_OF_MIDI_SWITCHES, "Receiving settings");
    emit updateSettings();
}
