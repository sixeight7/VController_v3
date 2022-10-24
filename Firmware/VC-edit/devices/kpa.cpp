#include "kpa.h"
#include "VController/config.h"
#include "VController/leds.h"
#include "VController/globals.h"

void KPA_class::init()
{
    device_name = "KPA";
    full_device_name = "Kemper Profiler";
    patch_min = KPA_PATCH_MIN;
    patch_max = KPA_PATCH_MAX;
    enabled = DEVICE_DETECT; // Default value
    my_LED_colour = 10; // Default value: soft green
    MIDI_port_manual = MIDI_port_number(KPA_MIDI_PORT);
    MIDI_channel = KPA_MIDI_CHANNEL; // Default value
    bank_number = 0; // Default value
    is_always_on = true; // Default value
    if (VC_type == VCONTROLLER) {
      my_device_page1 = KPA_DEFAULT_VC_PAGE1; // Default value
      my_device_page2 = KPA_DEFAULT_VC_PAGE2; // Default value
      my_device_page3 = KPA_DEFAULT_VC_PAGE3; // Default value
      my_device_page4 = KPA_DEFAULT_VC_PAGE4; // Default value
    }
    if (VC_type == VCMINI) {
      my_device_page1 = KPA_DEFAULT_VCMINI_PAGE1; // Default value
      my_device_page2 = KPA_DEFAULT_VCMINI_PAGE2; // Default value
      my_device_page3 = KPA_DEFAULT_VCMINI_PAGE3; // Default value
      my_device_page4 = KPA_DEFAULT_VCMINI_PAGE4; // Default value
    }
    if (VC_type == VCTOUCH) {
      my_device_page1 = KPA_DEFAULT_VCTOUCH_PAGE1; // Default value
      my_device_page2 = KPA_DEFAULT_VCTOUCH_PAGE2; // Default value
      my_device_page3 = KPA_DEFAULT_VCTOUCH_PAGE3; // Default value
      my_device_page4 = KPA_DEFAULT_VCTOUCH_PAGE4; // Default value
    }
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
    uint16_t performance_no = patch_no + 1;
    Output += QString::number(performance_no / 100) + QString::number((performance_no / 10) % 10) + QString::number(performance_no % 10);

    return Output;
}

struct KPA_CC_type_struct { // Combines all the data we need for controlling a parameter in a device
  QString Name; // The name for the label
  uint16_t Address;
  uint8_t CC; // The cc for this effect.
  uint8_t latch_type;
};

QVector<KPA_CC_type_struct> KPA_CC_types = {
    {"STOMP A", 0x3203, 17, TOGGLE}, // 0
    {"STOMP B", 0x3303, 18, TOGGLE},
    {"STOMP C", 0x3403, 19, TOGGLE},
    {"STOMP D", 0x3503, 20, TOGGLE},
    {"STOMP X", 0x3803, 22, TOGGLE},
    {"STOMP MOD", 0x3A03, 24, TOGGLE},
    {"STOMP DLY", 0x3C03, 27, TOGGLE},
    {"STOMP RVB", 0x3D03, 29, TOGGLE},
    {"ROTARY SPD", 0x0000, 33, TOGGLE},
    {"DLY FB INF", 0x0000, 34, TOGGLE},
    {"DELAY HOLD", 0x0000, 35, TOGGLE}, // 10
    {"MORPH BTN", 0x0000, 80, MOMENTARY},
    {"WAH PDL", 0x0000, 1, UPDOWN},
    {"MORPH PDL", 0x0000, 11, UPDOWN},
    {"VOLUME", 0x0000, 7, UPDOWN},
    {"PITCH PDL", 0x0000, 2, UPDOWN},
    {"MODE", KPA_MODE_ADDRESS, 0, TOGGLE},
    {"RIG UP", 0x0000, 48, ONE_SHOT},
    {"RIG DOWN", 0x0000, 49, ONE_SHOT},
    {"LOOPER POS", 0x7F35, 0, TOGGLE}
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

QString KPA_class::get_patch_info(uint16_t number)
{
    uint16_t patch_no = (Device_patches[number][1] << 8) + Device_patches[number][2] - 1;
    QString line;
    if (patch_no < 25) {
        line = number_format(patch_no * 8);
        line.append('-');
        line.append(number_format((patch_no * 8) + 7));
        line.append("\tPerformance names");
    }
    else {
        patch_no -= 25;
        line = "RIG";
        line.append(number_format(patch_no * 8));
        line.append("-");
        line.append(number_format((patch_no * 8) + 7));
        line.append("\tRig names");
    }
    //line.append(QString::number(patch_no));
    return line;
}
