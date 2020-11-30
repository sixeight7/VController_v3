#include "gmajor2.h"
#include "VController/config.h"
#include "VController/leds.h"

void GM2_class::init()
{
    device_name = "GM2";
    full_device_name = "TC-E G-Major 2";
    patch_min = GM2_PATCH_MIN;
    patch_max = GM2_PATCH_MAX;
    enabled = DEVICE_DETECT; // Default value
    my_LED_colour = 2; // Default value: red
    MIDI_channel = GM2_MIDI_CHANNEL; // Default value
    bank_number = 0; // Default value
    is_always_on = true; // Default value
    my_device_page1 = GM2_DEFAULT_PAGE1; // Default value
    my_device_page2 = GM2_DEFAULT_PAGE2; // Default value
    my_device_page3 = GM2_DEFAULT_PAGE3; // Default value
    my_device_page4 = GM2_DEFAULT_PAGE4; // Default value
}

bool GM2_class::check_command_enabled(uint8_t cmd)
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

QString GM2_class::number_format(uint16_t patch_no)
{
    uint16_t number_plus_one = patch_no + 1;
    return QString::number(number_plus_one / 100) + QString::number((number_plus_one / 10) % 10) + QString::number(number_plus_one % 10);
}

struct GM2_FX_type_struct { // Combines all the data we need for controlling a parameter in a device
    uint8_t Address;
      uint8_t Address_type;
      QString Name; // The name for the label
      uint8_t Sublist;
      uint8_t Colour; // The colour for this effect.
};

QVector<GM2_FX_type_struct> GM2_FX_types = { // Table with the name and colour for every effect of the Zoom G3
      { 0x46, 0x37, "Filter", 1, FX_FILTER_TYPE },
      { 0x36, 0x00, "Comp", 0, FX_DYNAMICS_TYPE },
      { 0x56, 0x47, "Pitch", 8, FX_PITCH_TYPE },
      { 0x66, 0x57, "Mod", 13, FX_MODULATE_TYPE },
      { 0x76, 0x67, "Delay", 26, FX_DELAY_TYPE },
      { 0x86, 0x77, "Reverb", 30, FX_REVERB_TYPE },
      { 0x96, 0x00, "Gate", 0, FX_DYNAMICS_TYPE },
      { 0x8B, 0x00, "Eq", 0, FX_FILTER_TYPE },
};

const uint16_t GM2_NUMBER_OF_PARAMETERS = GM2_FX_types.size();

#define GM2_SW_EXP_TOE 10
#define GM2_SW_EXP1 11
#define GM2_SW_EXP2 12
#define GM2_SW_EXP3 13
#define GM2_SETLIST 14
#define GM2_SNAPSHOT 15

QString GM2_class::read_parameter_name(uint16_t par_no)
{
    if (par_no < number_of_parameters())  return GM2_FX_types[par_no].Name;
    else return "?";
}

QString GM2_class::read_parameter_state(uint16_t par_no, uint8_t value)
{
    if (par_no < number_of_parameters()) {
        if (value == 1) return "ON";
        else return "OFF";
    }
    return "?";
}

uint16_t GM2_class::number_of_parameters()
{
    return GM2_NUMBER_OF_PARAMETERS;
}

uint8_t GM2_class::max_value(uint16_t par_no)
{
    return 1;
}
