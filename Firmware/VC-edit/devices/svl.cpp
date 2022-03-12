#include "svl.h"
#include "VController/config.h"
#include "VController/leds.h"
#include "VController/globals.h"

void SVL_class::init()
{
    device_name = "SVL";
    full_device_name = "Strymon Volante";
    patch_min = SVL_PATCH_MIN;
    patch_max = SVL_PATCH_MAX;
    enabled = DEVICE_DETECT; // Default value
    my_LED_colour = 7; // Default value: yellow
    MIDI_port_manual = MIDI_port_number(SVL_MIDI_PORT);
    MIDI_channel = SVL_MIDI_CHANNEL; // Default value
    bank_number = 0; // Default value
    is_always_on = true; // Default value
    if (VC_type == VCONTROLLER) {
      my_device_page1 = SVL_DEFAULT_VC_PAGE1; // Default value
      my_device_page2 = SVL_DEFAULT_VC_PAGE2; // Default value
      my_device_page3 = SVL_DEFAULT_VC_PAGE3; // Default value
      my_device_page4 = SVL_DEFAULT_VC_PAGE4; // Default value
    }
    if (VC_type == VCMINI) {
      my_device_page1 = SVL_DEFAULT_VCMINI_PAGE1; // Default value
      my_device_page2 = SVL_DEFAULT_VCMINI_PAGE2; // Default value
      my_device_page3 = SVL_DEFAULT_VCMINI_PAGE3; // Default value
      my_device_page4 = SVL_DEFAULT_VCMINI_PAGE4; // Default value
    }
    if (VC_type == VCTOUCH) {
      my_device_page1 = SVL_DEFAULT_VCTOUCH_PAGE1; // Default value
      my_device_page2 = SVL_DEFAULT_VCTOUCH_PAGE2; // Default value
      my_device_page3 = SVL_DEFAULT_VCTOUCH_PAGE3; // Default value
      my_device_page4 = SVL_DEFAULT_VCTOUCH_PAGE4; // Default value
    }
}

bool SVL_class::check_command_enabled(uint8_t cmd)
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
    //case LOOPER:
        return true;
    }
    return false;
}

QString SVL_class::number_format(uint16_t patch_no)
{
    uint16_t number_plus_one = patch_no + 1;
    return QString::number(number_plus_one / 100) + QString::number((number_plus_one / 10) % 10) + QString::number(number_plus_one % 10);
}

struct SVL_CC_type_struct { // Combines all the data we need for controlling a parameter in a device
  QString Name; // The name for the label
  uint8_t CC; // The colour for this effect.
  uint8_t NumVals; // The number of values for this parameter
  bool Momentary;
};

QVector<SVL_CC_type_struct> SVL_CC_types = { // Table with the name and colour for every effect of the Zoom G3
   {"FS ON", 80, 2, true}, // 0
   {"FS FAVORITE", 81, 2, true},
   {"FS TAP", 82, 2, true},
   {"REVERSE", 44, 2, false},
   {"PAUSE", 43, 2, false},
   {"INFINITE", 45, 2, false},
   {"SPEED", 19, 3, false},
   {"SOS mode", 41, 2, false},
   {"BYPASS", 102, 2, false},
   {"ECHO ON/OFF", 78, 2, false},
   {"REVERB ON/OFF", 79, 2, false}, // 10
   {"KILL DRY", 84, 2, false}, // 11
   {"EXP", 100, 128, true},
};

const uint16_t SVL_NUMBER_OF_PARAMETERS = SVL_CC_types.size();

QStringList SVL_sublists = {
  // SPEED types
  "DOUBLE", "HALF", "NORMAL",
};

#define SVL_EXP 12

QString SVL_class::read_parameter_name(uint16_t par_no)
{
    if (par_no < number_of_parameters())  return SVL_CC_types[par_no].Name;
    else return "?";
}

QString SVL_class::read_parameter_state(uint16_t par_no, uint8_t value)
{
    if ((SVL_CC_types[par_no].NumVals == 3) && (value < 3)) return SVL_sublists[value];
    if (par_no < number_of_parameters()) return QString::number(value);
      return "?";
}

uint16_t SVL_class::number_of_parameters()
{
    return SVL_NUMBER_OF_PARAMETERS;
}

uint8_t SVL_class::max_value(uint16_t par_no)
{
    if (par_no < SVL_NUMBER_OF_PARAMETERS) {
        return SVL_CC_types[par_no].NumVals - 1;
      }
      else return 0;
}
