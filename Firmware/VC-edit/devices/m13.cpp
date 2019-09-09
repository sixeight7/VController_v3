#include "m13.h"

#include "VController/config.h"
#include "VController/leds.h"

void M13_class::init()
{
    device_name = "M13";
    full_device_name = "Line6 M13";
    patch_min = M13_PATCH_MIN;
    patch_max = M13_PATCH_MAX;
    enabled = DEVICE_DETECT; // Default value
    my_LED_colour = 1; // Default value: green
    MIDI_channel = M13_MIDI_CHANNEL; // Default value
    bank_number = 0; // Default value
    is_always_on = true; // Default value
    my_device_page1 = M13_DEFAULT_PAGE1; // Default value
    my_device_page2 = M13_DEFAULT_PAGE2; // Default value
    my_device_page3 = M13_DEFAULT_PAGE3; // Default value
    my_device_page4 = M13_DEFAULT_PAGE4; // Default value
}

bool M13_class::check_command_enabled(uint8_t cmd)
{
    switch (cmd) {
    case PATCH:
    case PARAMETER:
    //case ASSIGN:
    //case MUTE:
    case OPEN_PAGE_DEVICE:
    case OPEN_NEXT_PAGE_OF_DEVICE:
    case TOGGLE_EXP_PEDAL:
    //case SNAPSCENE:
    case LOOPER:
        return true;
    }
    return false;
}

struct M13_CC_type_struct { // Combines all the data we need for controlling a parameter in a device
  QString Name; // The name for the label
  uint8_t CC; // The colour for this effect.
};

QVector<M13_CC_type_struct> M13_CC_types = { // Table with the name and colour for every effect of the Zoom G3
  {"FX UNIT 1A", 11},
  {"FX UNIT 2A", 14},
  {"FX UNIT 3A", 17},
  {"FX UNIT 4A", 20},
  {"FX UNIT 1B", 12},
  {"FX UNIT 2B", 15},
  {"FX UNIT 3B", 18},
  {"FX UNIT 4B", 21},
  {"FX UNIT 1C", 13},
  {"FX UNIT 2C", 16},
  {"FX UNIT 3C", 19},
  {"FX UNIT 4C", 22},
  {"BYPASS ALL+LOOP", 23},
  {"BYPASS ALL-LOOP", 24},
  {"M13 EXP1", 1},
  {"M13 EXP2", 2},
};

uint16_t M13_NUMBER_OF_PARAMETERS = M13_CC_types.size();

#define M13_SW_BYPASS 12
#define M13_SW_BYPASS_LOOP 13
#define M13_SW_EXP1 14
#define M13_SW_EXP2 15

QString M13_class::read_parameter_name(uint16_t par_no)
{
    if (par_no < number_of_parameters())  return M13_CC_types[par_no].Name;
    else return "?";
}

QString M13_class::read_parameter_state(uint16_t par_no, uint8_t value)
{
    if ((par_no == M13_SW_EXP1) || (par_no == M13_SW_EXP2)) return QString::number(value); // Return the number for the expression pedals
      else if (par_no < number_of_parameters())  {
        if (value == 1) return "ON";
        else return "OFF";
      }
      return "?";
}

uint16_t M13_class::number_of_parameters()
{
    return M13_NUMBER_OF_PARAMETERS;
}

uint8_t M13_class::max_value(uint16_t par_no)
{
    if ((par_no == M13_SW_EXP1) || (par_no == M13_SW_EXP2)) return 127; // Return 128 for the expression pedals
    if (par_no < number_of_parameters()) return 1;
    else return 0;
}
