#include "helix.h"
#include "VController/config.h"
#include "VController/leds.h"

void HLX_class::init()
{
    device_name = "HELIX";
    full_device_name = "Line6 HELIX";
    patch_min = HLX_PATCH_MIN;
    patch_max = HLX_PATCH_MAX;
    enabled = DEVICE_DETECT; // Default value
    my_LED_colour = 2; // Default value: red
    MIDI_channel = HLX_MIDI_CHANNEL; // Default value
    bank_number = 0; // Default value
    is_always_on = true; // Default value
    my_device_page1 = HLX_DEFAULT_PAGE1; // Default value
    my_device_page2 = HLX_DEFAULT_PAGE2; // Default value
    my_device_page3 = HLX_DEFAULT_PAGE3; // Default value
    my_device_page4 = HLX_DEFAULT_PAGE4; // Default value
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
    case SNAPSCENE:
    case LOOPER:
        return true;
    }
    return false;
}

QString HLX_class::number_format(uint16_t patch_no)
{
    uint8_t bank_no = patch_no >> 2;
    QChar PatchChar = 65 + patch_no % 4;
    return QString::number((bank_no + 1) / 10) + QString::number((bank_no + 1) % 10) + PatchChar;
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
