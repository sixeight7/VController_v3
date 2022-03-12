#include "mg300.h"
#include "VController/config.h"
#include "VController/leds.h"
#include "VController/globals.h"

void MG300_class::init()
{
    device_name = "MG300";
    full_device_name = "NUX MG300";
    patch_min = MG300_PATCH_MIN;
    patch_max = MG300_PATCH_MAX;
    enabled = DEVICE_DETECT; // Default value
    my_LED_colour = 2; // Default value: red
    MIDI_port_manual = MIDI_port_number(MG300_MIDI_PORT);
    MIDI_channel = MG300_MIDI_CHANNEL; // Default value
    bank_number = 0; // Default value
    is_always_on = true; // Default value
    if (VC_type == VCONTROLLER) {
      my_device_page1 = MG300_DEFAULT_VC_PAGE1; // Default value
      my_device_page2 = MG300_DEFAULT_VC_PAGE2; // Default value
      my_device_page3 = MG300_DEFAULT_VC_PAGE3; // Default value
      my_device_page4 = MG300_DEFAULT_VC_PAGE4; // Default value
    }
    if (VC_type == VCMINI) {
      my_device_page1 = MG300_DEFAULT_VCMINI_PAGE1; // Default value
      my_device_page2 = MG300_DEFAULT_VCMINI_PAGE2; // Default value
      my_device_page3 = MG300_DEFAULT_VCMINI_PAGE3; // Default value
      my_device_page4 = MG300_DEFAULT_VCMINI_PAGE4; // Default value
    }
    if (VC_type == VCTOUCH) {
      my_device_page1 = MG300_DEFAULT_VCTOUCH_PAGE1; // Default value
      my_device_page2 = MG300_DEFAULT_VCTOUCH_PAGE2; // Default value
      my_device_page3 = MG300_DEFAULT_VCTOUCH_PAGE3; // Default value
      my_device_page4 = MG300_DEFAULT_VCTOUCH_PAGE4; // Default value
    }
}

bool MG300_class::check_command_enabled(uint8_t cmd)
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

QString MG300_class::number_format(uint16_t patch_no)
{
    char patchChar = (patch_no % 4) + 65;
    if (patch_no < 36) return QString::number(patch_no / 4 + 1) + patchChar; // For user patches
    else return 'F' + QString::number(patch_no / 4 - 8) + patchChar; // For factory patches
}

struct MG300_CC_type_struct { // Combines all the data we need for controlling a parameter in a device
    QString Name; // The name for the label
    uint8_t CC; // The colour for this effect.
    uint8_t NumVals; // The number of values for this parameter
    uint8_t Sublist;
    uint8_t Colour;
};

QVector<MG300_CC_type_struct> MG300_CC_types = { // Table with the name and colour for every effect of the Zoom G3
    {"PEDAL", 62, 6, 1, FX_WAH_TYPE},
    {"CMP", 1, 2, 7, FX_DYNAMICS_TYPE},
    {"EFX", 2, 13, 9, FX_DIST_TYPE},
    {"AMP", 3, 26, 22, FX_AMP_TYPE},
    {"EQ", 4, 2, 48, FX_FILTER_TYPE},
    {"NR", 5, 2, 50, FX_DYNAMICS_TYPE},
    {"MOD", 6, 14, 52, FX_MODULATE_TYPE},
    {"DLY", 7, 7, 66, FX_DELAY_TYPE},
    {"RVB", 8, 5, 73, FX_REVERB_TYPE},
};

const uint16_t MG300_NUMBER_OF_PARAMETERS = MG300_CC_types.size();

QString MG300_class::read_parameter_name(uint16_t par_no)
{
    if (par_no < number_of_parameters())  return MG300_CC_types[par_no].Name;
    else return "?";
}

QString MG300_class::read_parameter_state(uint16_t par_no, uint8_t value)
{
    if (par_no < number_of_parameters()) {
        if (max_value(par_no) == 1) {
            if (value == 0) return "OFF";
            else return "ON";
        }
        else return QString::number(value);
    }
    return "?";
}

uint16_t MG300_class::number_of_parameters()
{
    return MG300_NUMBER_OF_PARAMETERS;
}

uint8_t MG300_class::max_value(uint16_t par_no)
{
    if (par_no < MG300_NUMBER_OF_FX) {
        return 1;
    }
    else return 0;
}

