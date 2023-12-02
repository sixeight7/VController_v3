#include "device.h"
#include "VController/config.h"
#include "VController/globals.h"
#include "VController/hardware.h"
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
        case 11: return dev_type;
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
      case 11: dev_type = value; break;
    }
}

uint8_t Device_class::get_number_of_dev_types()
{
  return 1;
}

QString Device_class::get_dev_type_name(uint8_t number)
{
  return "Default";
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
        "enabled",          // 10
        "dev_type"          // 11
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

QString Device_class::build_patch_number(uint16_t number, QString firstPatch, QString lastPatch)
{
    int length = firstPatch.length();
    if ((length == 0) || (length != lastPatch.length())) return firstPatch;

    QString outputPatch = firstPatch;

    int factor = 1;

    int i = length - 1;
    while (i >= 0) {
        QChar firstPatchChar = firstPatch[i];
        QChar lastPatchChar = lastPatch[i];

        if ((firstPatchChar == 'U') && (lastPatchChar == 'P')) {
            if (number == 1) outputPatch[i] = 'P';
            number /= 2;
        }
        else if ((firstPatchChar == 'P') && (lastPatchChar == 'U')) {
            if (number == 1) outputPatch[i] = 'U';
            number /= 2;
        }
        else if (firstPatchChar.isLetter() && lastPatchChar.isLetter()) {
            factor = lastPatchChar.toLatin1() - firstPatchChar.toLatin1() + 1;
            if (factor < 1) return firstPatch;
            int currentDigit = number % factor;
            outputPatch[i] = (char)(firstPatchChar.toLatin1() + currentDigit);
            number /= factor;
        }

        if (firstPatchChar.isDigit() && lastPatchChar.isDigit()) {
            // Find numeric sequences in both patch numbers.
            QString firstNumSeq, lastNumSeq;
            while (i >= 0 && (firstPatch[i].isDigit()) && (lastPatch[i].isDigit())) {
                firstNumSeq = firstPatch[i] + firstNumSeq;
                lastNumSeq = lastPatch[i] + lastNumSeq;
                i--;
            }
            i++;

            int firstNum = firstNumSeq.toInt();
            int lastNum = lastNumSeq.toInt();
            factor = lastNum - firstNum + 1;
            if (factor < 1) return firstPatch;

            QString currentNum = QString::number((number % factor) + firstNum).rightJustified(lastNumSeq.length(), '0');
            for(int j = 0; j < currentNum.length(); j++) {
                outputPatch[i + j] = currentNum[j];
            }
            number /= factor;
        }

        i--;
    }

    return outputPatch;
}

uint8_t Device_class::convert_mask_number(QChar c)
{
    if (c == '0') return 10; // A '0' is 10
    if (c == '#') return 10; // A '#' is also 10, but the number to the left, can be calculated with the same number as this one.
    if ((c >= '1') && (c <= '9')) return c.toLatin1() - '0';
    if ((c >= 'A') && (c <= 'Z')) return (c.toLatin1() - 'A' + 10);
    //if ((c >= 'a') && (c <= 'j')) return 10;
    return 0;
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

uint8_t Device_class::get_number_of_snapscenes()
{
    return 0;
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

QString Device_class::DefaultPatchFileName(int index)
{
    QString name = device_name;
    int patch_no = (Device_patches[index][1] << 8) + Device_patches[index][2];
    name.append('_');
    name.append(number_format(patch_no));
    return name;
}

QString Device_class::numConv(int number) const
{
    QString num = "";
    if (number < 10) num.append("0");
    num.append(QString::number(number));
    return num;
}

uint16_t Device_class::patch_min_as_stored_on_VC()
{
    return 0;
}

QString Device_class::setlist_song_full_item_format(uint16_t item)
{
  return device_name + ": " + number_format(item);
}

QString Device_class::setlist_song_short_item_format(uint16_t item)
{
    return number_format(item);
}

int Device_class::setlist_song_get_number_of_items()
{
    return patch_max + 1;
}

uint8_t Device_class::MIDI_port_number(uint8_t port)
{
   if (VC_type == VCONTROLLER) {
      if (port == VC_PORT1_TYPE) return 0;
      if (port == VC_PORT2_TYPE) return 1;
      if (port == VC_PORT3_TYPE) return 2;
      if (port == VC_PORT4_TYPE) return 3;
      if (port == VC_PORT5_TYPE) return 4;
      if (port == VC_PORT6_TYPE) return 5;
      if (port == VC_PORT7_TYPE) return 6;
      if (port == VC_PORT8_TYPE) return 7;
      if (port == VC_PORT9_TYPE) return 8;
   }

   if (VC_type == VCMINI) {
      if (port == VCMINI_PORT1_TYPE) return 0;
      if (port == VCMINI_PORT2_TYPE) return 1;
      if (port == VCMINI_PORT3_TYPE) return 2;
      if (port == VCMINI_PORT4_TYPE) return 3;
      if (port == VCMINI_PORT5_TYPE) return 4;
      if (port == VCMINI_PORT6_TYPE) return 5;
      if (port == VCMINI_PORT7_TYPE) return 6;
      if (port == VCMINI_PORT8_TYPE) return 7;
      if (port == VCMINI_PORT9_TYPE) return 8;
   }

   if (VC_type == VCTOUCH) {
      if (port == VCTOUCH_PORT1_TYPE) return 0;
      if (port == VCTOUCH_PORT2_TYPE) return 1;
      if (port == VCTOUCH_PORT3_TYPE) return 2;
      if (port == VCTOUCH_PORT4_TYPE) return 3;
      if (port == VCTOUCH_PORT5_TYPE) return 4;
      if (port == VCTOUCH_PORT6_TYPE) return 5;
      if (port == VCTOUCH_PORT7_TYPE) return 6;
      if (port == VCTOUCH_PORT8_TYPE) return 7;
      if (port == VCTOUCH_PORT9_TYPE) return 8;
   }

   return 0;
}

