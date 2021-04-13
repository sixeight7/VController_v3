#include "device.h"
#include "VController/config.h"
#include "VController/globals.h"
#include <QStringList>

void Device_class::init()
{

}

uint8_t Device_class::get_setting(uint8_t variable)
{
    switch (variable) {
        case 0: return my_LED_colour;
        case 1: return MIDI_channel;
        case 2: return MIDI_port_manual;
        case 3: return MIDI_device_id;
        case 4: return bank_number;
        case 5: return is_always_on;
        case 6: return my_device_page1;
        case 7: return my_device_page2;
        case 8: return my_device_page3;
        case 9: return my_device_page4;
        case 10: return enabled;
      }
      return 0;
}

void Device_class::set_setting(uint8_t variable, uint8_t value)
{
    switch (variable) {
      case 0: my_LED_colour = value; break;
      case 1: MIDI_channel = value; break;
      case 2: MIDI_port_manual = value; break;
      case 3: MIDI_device_id = value; break;
      case 4: bank_number = value; break;
      case 5: is_always_on = value; break;
      case 6: my_device_page1 = value; break;
      case 7: my_device_page2 = value; break;
      case 8: my_device_page3 = value; break;
      case 9: my_device_page4 = value; break;
      case 10: enabled = value; break;
    }
}

QString Device_class::get_setting_name(uint8_t variable) // Setting names for json output file
{
    QStringList setting_names = {
        "my_LED_colour",    // 0
        "MIDI_channel",     // 1
        "MIDI_port_manual", // 2
        "MIDI_device_id",   // 3
        "bank_number",      // 4
        "is_always_on",     // 5
        "my_device_page1",  // 6
        "my_device_page2",  // 7
        "my_device_page3",  // 8
        "my_device_page4",  // 9
        "enabled"           // 10
    };
    if (variable < setting_names.size()) return setting_names.at(variable);
    return "";
}

QString Device_class::get_patch_info(uint16_t number)
{
    uint8_t patch_no = (Device_patches[number][1] << 7) + Device_patches[number][2];
    return number_format(patch_no);
}

void Device_class::rename_patch(uint16_t number)
{

}

bool Device_class::check_command_enabled(uint8_t cmd)
{
    switch (cmd) {
    case PATCH:
    case PARAMETER:
    //case ASSIGN:
    //case MUTE:
    case OPEN_PAGE_DEVICE:
    case OPEN_NEXT_PAGE_OF_DEVICE:
    //case TOGGLE_EXP_PEDAL:
    //case MASTER_EXP_PEDAL:
    //case SNAPSCENE:
    //case LOOPER:
        return true;
    }
    return false;
}

QString Device_class::number_format(uint16_t patch_no)
{
  return QString::number(patch_no);;
}

QString Device_class::read_parameter_name(uint16_t)
{
  return "No parameters";
}

QString Device_class::read_parameter_state(uint16_t, uint8_t)
{
  return "";
}

uint16_t Device_class::number_of_parameters()
{
    return 0;
}

uint8_t Device_class::number_of_values(uint16_t)
{
    return 0;
}

uint8_t Device_class::min_value(uint16_t)
{
    return 0;
}

uint8_t Device_class::max_value(uint16_t)
{
  return 0;
}

uint8_t Device_class::get_number_of_assigns()
{
  return 0;
}

QString Device_class::read_assign_name(uint8_t)
{
  return "No assigns";
}

QString Device_class::read_assign_trigger(uint8_t)
{
  return "";
}

uint8_t Device_class::trigger_follow_assign(uint8_t)
{
    return 0;
}

QString Device_class::read_scene_name_from_buffer(int, uint8_t) const
{
    return "";
}

void Device_class::store_scene_name_to_buffer(int, uint8_t, QString)
{

}

uint8_t Device_class::supportPatchSaving()
{
    return 0;
}

void Device_class::readPatchData(int, int, const QJsonObject &)
{

}

void Device_class::writePatchData(int, QJsonObject &) const
{

}

QString Device_class::patchFileHeader()
{
    return device_name;
}

QString Device_class::DefaultPatchFileName(int number)
{
    QString name = device_name;
    name.append('_');
    name.append(number_format(number));
    return name;
}

QString Device_class::numConv(int number) const
{
    QString num = "";
    if (number < 10) num.append("0");
    num.append(QString::number(number));
    return num;
}

