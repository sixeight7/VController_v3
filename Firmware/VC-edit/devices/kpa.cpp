#include "kpa.h"
#include "VController/config.h"
#include "VController/leds.h"

void KPA_class::init()
{
    device_name = "KPA";
    full_device_name = "Kemper Profiler";
    patch_min = KPA_PATCH_MIN;
    patch_max = KPA_PATCH_MAX;
    enabled = DEVICE_DETECT; // Default value
    my_LED_colour = 10; // Default value: soft green
    MIDI_channel = KPA_MIDI_CHANNEL; // Default value
    bank_number = 0; // Default value
    is_always_on = true; // Default value
    my_device_page1 = KPA_DEFAULT_PAGE1; // Default value
    my_device_page2 = KPA_DEFAULT_PAGE2; // Default value
    my_device_page3 = KPA_DEFAULT_PAGE3; // Default value
    my_device_page4 = KPA_DEFAULT_PAGE4; // Default value
}

bool KPA_class::check_command_enabled(uint8_t cmd)
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
    //case SNAPSCENE:
    case LOOPER:
        return true;
    }
    return false;
}

QString KPA_class::number_format(uint16_t patch_no)
{
    QString Output = "";
    if (patch_no < 5) Output += "BROWSE";
    else {
       uint16_t rig_no = patch_no - 4;
       Output += "RIG" + QString::number(rig_no / 100) + QString::number((rig_no / 10) % 10) + QString::number(rig_no % 10);
    }
    Output += "/";
    uint16_t performance_no = (patch_no / 5) + 1;
    Output += QString::number(performance_no / 100) + QString::number((performance_no / 10) % 10) + QString::number(performance_no % 10) + "-" + QString::number((patch_no % 5) + 1);

    return Output;
}

struct KPA_CC_type_struct { // Combines all the data we need for controlling a parameter in a device
  QString Name; // The name for the label
  uint16_t Address;
  uint8_t CC; // The cc for this effect.
};

QVector<KPA_CC_type_struct> KPA_CC_types = {
      {"STOMP A", 0x3203, 17}, // 0
      {"STOMP B", 0x3303, 18},
      {"STOMP C", 0x3403, 19},
      {"STOMP D", 0x3503, 20},
      {"STOMP X", 0x3803, 22},
      {"STOMP MOD", 0x3A03, 24},
      {"STOMP DLY", 0x3C03, 26},
      {"STOMP RVB", 0x4B02, 28},
      {"ROTARY SPEED", 0x0000, 33},
      {"DLY FBACK INF", 0x0000, 34},
      {"DELAY HOLD", 0x0000, 35}, // 10
      {"WAH PDL", 0x0000, 1},
      {"MORPH PDL", 0x0000, 11},
      {"VOLUME", 0x0000, 7},
      {"PITCH PDL", 0x0000, 2},
      {"MODE", KPA_MODE_ADDRESS, 0},
      {"RIG LEFT", 0x0000, 48},
      {"RIG RIGHT", 0x0000, 49}
};

const uint16_t KPA_NUMBER_OF_PARAMETERS = KPA_CC_types.size();

#define KPA_NUMBER_OF_FX_SLOTS 8

#define KPA_STOMP_DLY 6
#define KPA_STOMP_RVB 7
#define KPA_WAH_PEDAL 11
#define KPA_PITCH_PEDAL 14
#define KPA_VOL_PEDAL 13
#define KPA_MORPH_PEDAL 12
#define KPA_MODE 15
#define KPA_RIG_LEFT 16
#define KPA_RIG_RIGHT 17

QString KPA_class::read_parameter_name(uint16_t par_no)
{
    if (par_no < number_of_parameters())  return KPA_CC_types[par_no].Name;
    else return "?";
}

QString KPA_class::read_parameter_state(uint16_t par_no, uint8_t value)
{
    if (par_no < number_of_parameters()) return QString::number(value);
      return "?";
}

uint16_t KPA_class::number_of_parameters()
{
    return KPA_NUMBER_OF_PARAMETERS;
}

uint8_t KPA_class::max_value(uint16_t par_no)
{
    if ((par_no == KPA_WAH_PEDAL) || (par_no == KPA_PITCH_PEDAL) || (par_no == KPA_VOL_PEDAL) || (par_no == KPA_MORPH_PEDAL)) return 128; // Return 128 for the expression pedals
      if (par_no < number_of_parameters()) return 2;
      else return 0;
}
