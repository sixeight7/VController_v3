#include "helix.h"
#include "VController/config.h"
#include "VController/leds.h"
#include "VController/globals.h"

void HLX_class::init()
{
    device_name = "HELIX";
    full_device_name = "Line6 HELIX";
    patch_min = HLX_PATCH_MIN;
    patch_max = HLX_PATCH_MAX;
    enabled = DEVICE_DETECT; // Default value
    my_LED_colour = 2; // Default value: red
    MIDI_port_manual = MIDI_port_number(HLX_MIDI_PORT);
    MIDI_channel = HLX_MIDI_CHANNEL; // Default value
    bank_number = 0; // Default value
    is_always_on = true; // Default value
    if (VC_type == VCONTROLLER) {
      my_device_page1 = HLX_DEFAULT_VC_PAGE1; // Default value
      my_device_page2 = HLX_DEFAULT_VC_PAGE2; // Default value
      my_device_page3 = HLX_DEFAULT_VC_PAGE3; // Default value
      my_device_page4 = HLX_DEFAULT_VC_PAGE4; // Default value
    }
    if (VC_type == VCMINI) {
      my_device_page1 = HLX_DEFAULT_VCMINI_PAGE1; // Default value
      my_device_page2 = HLX_DEFAULT_VCMINI_PAGE2; // Default value
      my_device_page3 = HLX_DEFAULT_VCMINI_PAGE3; // Default value
      my_device_page4 = HLX_DEFAULT_VCMINI_PAGE4; // Default value
    }
    if (VC_type == VCTOUCH) {
      my_device_page1 = HLX_DEFAULT_VCTOUCH_PAGE1; // Default value
      my_device_page2 = HLX_DEFAULT_VCTOUCH_PAGE2; // Default value
      my_device_page3 = HLX_DEFAULT_VCTOUCH_PAGE3; // Default value
      my_device_page4 = HLX_DEFAULT_VCTOUCH_PAGE4; // Default value
    }
}

uint8_t HLX_class::get_number_of_dev_types()
{
  return 8;
}

QString HLX_class::get_dev_type_name(uint8_t number)
{
  switch (number) {
      case TYPE_HELIX_01A: return "Helix (01A)";
      case TYPE_HELIX_000: return "Helix (000)";
      case TYPE_HX_STOMP_01A: return "HX stomp (01A)";
      case TYPE_HX_STOMP_000: return "HX stomp (000)";
      case TYPE_HX_STOMP_XL_01A: return "HX stomp XL(01A)";
      case TYPE_HX_STOMP_XL_000: return "HX stomp XL(000)";
      case TYPE_HX_EFFECTS_01A: return "HX effects (01A)";
      case TYPE_HX_EFFECTS_000: return "HX effects (000)";
      default: return "?";
    }
}

bool HLX_class::check_command_enabled(uint8_t cmd)
{
    switch (cmd) {
    case PATCH:
    case PARAMETER:
    //case ASSIGN:
    //case MUTE:
    case OPEN_PAGE_DEVICE:
    case OPEN_NEXT_PAGE_OF_DEVICE:
    case TOGGLE_EXP_PEDAL:
    case MASTER_EXP_PEDAL:
    case SNAPSCENE:
    case LOOPER:
        return true;
    }
    return false;
}

QString HLX_class::number_format(uint16_t patch_no)
{
    char PatchChar;
      uint8_t hlx_bank_size;
      uint8_t bank_no;

      switch (dev_type) {
        case TYPE_HELIX_01A:
        case TYPE_HX_STOMP_01A:
        case TYPE_HX_STOMP_XL_01A:
        case TYPE_HX_EFFECTS_01A:
          hlx_bank_size = 4;
          if (dev_type == TYPE_HX_STOMP_01A) hlx_bank_size = 3;
          bank_no = patch_no / hlx_bank_size;
          PatchChar = 65 + patch_no % hlx_bank_size;
          return QString::number((bank_no + 1) / 10) + QString::number((bank_no + 1) % 10) + PatchChar;
          break;
        case TYPE_HELIX_000:
        case TYPE_HX_STOMP_000:
        case TYPE_HX_STOMP_XL_000:
        case TYPE_HX_EFFECTS_000:
          return QString::number(patch_no / 100) + QString::number((patch_no / 10) % 10) + QString::number(patch_no % 10);
          break;
      }
      return "?";
}

struct HLX_CC_type_struct { // Combines all the data we need for controlling a parameter in a device
  QString Name; // The name for the label
  uint8_t CC; // The colour for this effect.
};

QVector<HLX_CC_type_struct> HLX_CC_types = { // Table with the name and colour for every effect of the Zoom G3
  {"FS 01", 49}, // 0
  {"FS 02", 50},
  {"FS 03", 51},
  {"FS 04", 52},
  {"FS 05", 53},
  {"FS 07", 54},
  {"FS 08", 55},
  {"FS 09", 56},
  {"FS 10", 57},
  {"FS 11", 58},
  {"EXP TOE SW", 59}, // 10
  {"EXP 1", 1}, // 11
  {"EXP 2", 2},
  {"EXP 3", 3},
  {"SETLIST", 32}, // 14
  {"SNAPSHOT", 69},
};

const uint16_t HLX_NUMBER_OF_PARAMETERS = HLX_CC_types.size();

#define HLX_SW_EXP_TOE 10
#define HLX_SW_EXP1 11
#define HLX_SW_EXP2 12
#define HLX_SW_EXP3 13
#define HLX_SETLIST 14
#define HLX_SNAPSHOT 15

QString HLX_class::read_parameter_name(uint16_t par_no)
{
    if (par_no < number_of_parameters())  return HLX_CC_types[par_no].Name;
    else return "?";
}

QString HLX_class::read_parameter_state(uint16_t par_no, uint8_t value)
{
    if (par_no < number_of_parameters()) return QString::number(value);
      return "?";
}

uint16_t HLX_class::number_of_parameters()
{
    return HLX_NUMBER_OF_PARAMETERS;
}

uint8_t HLX_class::max_value(uint16_t par_no)
{
    if (par_no == HLX_SNAPSHOT) return 7;
    if (par_no == HLX_SETLIST) return 7;
    if (par_no < number_of_parameters()) return 127;
    else return 0;
}

QString HLX_class::setlist_song_full_item_format(uint16_t item)
{
    QString Output = device_name;
    Output += ": ";
    Output += setlist_song_short_item_format(item);
    return Output;
}

QString HLX_class::setlist_song_short_item_format(uint16_t item)
{
    uint8_t setlist = item / 128;
    uint8_t patch = item % 128;
    QString Output = "SET";
    Output += QString::number(setlist + 1);
    Output += ' ';
    Output += number_format(patch);
    return Output;
}

int HLX_class::setlist_song_get_number_of_items()
{
    return (HLX_NUMBER_OF_SETLISTS * (patch_max + 1));
}

uint8_t HLX_class::get_number_of_snapscenes()
{
    return 8;
}
