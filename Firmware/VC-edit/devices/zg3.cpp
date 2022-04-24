#include "zg3.h"
#include "VController/config.h"
#include "VController/leds.h"
#include "VController/globals.h"

void ZG3_class::init()
{
    device_name = "ZG3";
    full_device_name = "Zoom G3";
    patch_min = ZG3_PATCH_MIN;
    patch_max = ZG3_PATCH_MAX;
    enabled = DEVICE_DETECT; // Default value
    my_LED_colour = 1; // Default value: green
    MIDI_port_manual = MIDI_port_number(ZG3_MIDI_PORT);
    MIDI_channel = ZG3_MIDI_CHANNEL; // Default value
    bank_number = 0; // Default value
    is_always_on = true; // Default value
    if (VC_type == VCONTROLLER) {
      my_device_page1 = ZG3_DEFAULT_VC_PAGE1; // Default value
      my_device_page2 = ZG3_DEFAULT_VC_PAGE2; // Default value
      my_device_page3 = ZG3_DEFAULT_VC_PAGE3; // Default value
      my_device_page4 = ZG3_DEFAULT_VC_PAGE4; // Default value
    }
    if (VC_type == VCMINI) {
      my_device_page1 = ZG3_DEFAULT_VCMINI_PAGE1; // Default value
      my_device_page2 = ZG3_DEFAULT_VCMINI_PAGE2; // Default value
      my_device_page3 = ZG3_DEFAULT_VCMINI_PAGE3; // Default value
      my_device_page4 = ZG3_DEFAULT_VCMINI_PAGE4; // Default value
    }
    if (VC_type == VCTOUCH) {
      my_device_page1 = ZG3_DEFAULT_VCTOUCH_PAGE1; // Default value
      my_device_page2 = ZG3_DEFAULT_VCTOUCH_PAGE2; // Default value
      my_device_page3 = ZG3_DEFAULT_VCTOUCH_PAGE3; // Default value
      my_device_page4 = ZG3_DEFAULT_VCTOUCH_PAGE4; // Default value
    }
}

bool ZG3_class::check_command_enabled(uint8_t cmd)
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

QString ZG3_class::number_format(uint16_t patch_no)
{
    QChar BankChar = (QChar)(65 + (patch_no / 10));
    return BankChar + QString::number(patch_no % 10);
}

QString ZG3_class::read_parameter_name(uint16_t par_no)
{
    if (par_no < number_of_parameters())  return "FX" + QString::number(par_no + 1) + " SW";
    else return "?";
}

QString ZG3_class::read_parameter_state(uint16_t par_no, uint8_t value)
{
    if (par_no < number_of_parameters())  {
        if (value == 1) return "ON";
        else return "OFF";
      }
      return "?";
}

uint16_t ZG3_class::number_of_parameters()
{
    return 6;
}

uint8_t ZG3_class::max_value(uint16_t par_no)
{
    if (par_no < number_of_parameters()) return 1;
    else return 0;
}

