#include "fractal.h"
#include "VController/config.h"
#include "VController/leds.h"
#include "VController/globals.h"

void AXEFX_class::init()
{
    device_name = "AXEFX";
    full_device_name = "Fractal device";
    patch_min = AXEFX_PATCH_MIN;
    patch_max = AXEFX_PATCH_MAX;
    model_number = AXE_MODEL_AF2;
    enabled = DEVICE_DETECT; // Default value
    my_LED_colour = 2; // Default value: red
    MIDI_port_manual = MIDI_port_number(FAS_MIDI_PORT);
    MIDI_channel = AXEFX_MIDI_CHANNEL; // Default value
    bank_number = 0; // Default value
    is_always_on = true; // Default value
    if (VC_type == VCONTROLLER) {
      my_device_page1 = FAS_DEFAULT_VC_PAGE1; // Default value
      my_device_page2 = FAS_DEFAULT_VC_PAGE2; // Default value
      my_device_page3 = FAS_DEFAULT_VC_PAGE3; // Default value
      my_device_page4 = FAS_DEFAULT_VC_PAGE4; // Default value
    }
    if (VC_type == VCMINI) {
      my_device_page1 = FAS_DEFAULT_VCMINI_PAGE1; // Default value
      my_device_page2 = FAS_DEFAULT_VCMINI_PAGE2; // Default value
      my_device_page3 = FAS_DEFAULT_VCMINI_PAGE3; // Default value
      my_device_page4 = FAS_DEFAULT_VCMINI_PAGE4; // Default value
    }
    if (VC_type == VCTOUCH) {
      my_device_page1 = FAS_DEFAULT_VCTOUCH_PAGE1; // Default value
      my_device_page2 = FAS_DEFAULT_VCTOUCH_PAGE2; // Default value
      my_device_page3 = FAS_DEFAULT_VCTOUCH_PAGE3; // Default value
      my_device_page4 = FAS_DEFAULT_VCTOUCH_PAGE4; // Default value
    }
}

bool AXEFX_class::check_command_enabled(uint8_t cmd)
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

QString AXEFX_class::number_format(uint16_t patch_no)
{
    QChar BankChar = (QChar)(65 + (patch_no >> 7));
    uint16_t number_plus_one = patch_no + 1;
    return BankChar + QString::number(number_plus_one / 100) + QString::number((number_plus_one / 10) % 10)
            + QString::number(number_plus_one % 10);
}

struct AXEFX_parameter_struct { // Combines all the data we need for controlling a parameter in a device
  QString Name; // The name for the label
  uint8_t Colour; // The colour for this effect.
};

QVector<AXEFX_parameter_struct> AXEFX_parameters = {
  {"Amp 1 Byp", FX_AMP_TYPE}, // CC #37
  {"Amp 2 Byp", FX_AMP_TYPE}, // CC #38
  {"Cabinet 1", FX_AMP_TYPE}, // CC #39
  {"Cabinet 2", FX_AMP_TYPE}, // etc.
  {"Chorus 1", FX_MODULATE_TYPE},
  {"Chorus 2", FX_MODULATE_TYPE},
  {"Compressor 1", FX_FILTER_TYPE},
  {"Compressor 2", FX_FILTER_TYPE},
  {"Crossover 1", FX_FILTER_TYPE},
  {"Crossover 2", FX_FILTER_TYPE},
  {"Delay 1", FX_DELAY_TYPE},
  {"Delay 2", FX_DELAY_TYPE},
  {"Drive 1", FX_DIST_TYPE},
  {"Drive 2", FX_DIST_TYPE},
  {"Enhancer 1", FX_FILTER_TYPE},
  {"Filter 1", FX_FILTER_TYPE},
  {"Filter 2", FX_FILTER_TYPE},
  {"Filter 3", FX_FILTER_TYPE},
  {"Filter 4", FX_FILTER_TYPE},
  {"Flanger 1", FX_MODULATE_TYPE},
  {"Flanger 2", FX_MODULATE_TYPE},
  {"Formant", FX_FILTER_TYPE},
  {"FX Loop", FX_MODULATE_TYPE},
  {"Gate 1", FX_FILTER_TYPE},
  {"Gate 2", FX_FILTER_TYPE},
  {"Graphic EQ 1", FX_FILTER_TYPE},
  {"Graphic EQ 2", FX_FILTER_TYPE},
  {"Graphic EQ 3", FX_FILTER_TYPE},
  {"Graphic EQ 4", FX_FILTER_TYPE},
  {"Megatap", FX_DELAY_TYPE},
  {"Multicomp 1", FX_FILTER_TYPE},
  {"Multicomp 2", FX_FILTER_TYPE},
  {"Multidly 1", FX_DELAY_TYPE},
  {"Multidly 2", FX_DELAY_TYPE},
  {"ParamEq 1", FX_FILTER_TYPE},
  {"ParamEq 2", FX_FILTER_TYPE},
  {"ParamEq 3", FX_FILTER_TYPE},
  {"ParamEq 4", FX_FILTER_TYPE},
  {"Phaser 1", FX_MODULATE_TYPE},
  {"Phaser 2", FX_MODULATE_TYPE},
  {"Pitch 1", FX_PITCH_TYPE},
  {"Pitch 2", FX_PITCH_TYPE},
  {"QChorus 1", FX_MODULATE_TYPE},
  {"QChorus 2", FX_MODULATE_TYPE},
  {"Resonator 1", FX_MODULATE_TYPE},
  {"Resonator 2", FX_MODULATE_TYPE},
  {"Reverb 1", FX_REVERB_TYPE},
  {"Reverb 2", FX_REVERB_TYPE},
  {"RingMod", FX_MODULATE_TYPE},
  {"Rotary 1", FX_MODULATE_TYPE},
  {"Rotary 2", FX_MODULATE_TYPE},
  {"Synth 1", FX_MODULATE_TYPE},
  {"Synth 2", FX_MODULATE_TYPE},
  {"Tremolo 1", FX_MODULATE_TYPE},
  {"Tremolo 2", FX_MODULATE_TYPE},
  {"Vocoder", FX_MODULATE_TYPE},
  {"Volume 1", FX_MODULATE_TYPE},
  {"Volume 2", FX_MODULATE_TYPE},
  {"Volume 3", FX_MODULATE_TYPE},
  {"Volume 4", FX_MODULATE_TYPE},
  {"Wah 1 Byp", FX_FILTER_TYPE},
  {"Wah 2 Byp", FX_FILTER_TYPE}, // CC #98 - last one supported by AXE-FX 1 and ultra
  {"Tonematch", FX_FILTER_TYPE},
  {"AMP 1 X/Y", FX_AMP_TYPE},
  {"AMP 2 X/Y", FX_AMP_TYPE},
  {"CABINET 1 X/Y", FX_AMP_TYPE},
  {"CABINET 2 X/Y", FX_AMP_TYPE},
  {"CHORUS 1 X/Y", FX_MODULATE_TYPE},
  {"CHORUS 2 X/Y", FX_MODULATE_TYPE},
  {"DELAY 1 X/Y", FX_DELAY_TYPE},
  {"DELAY 2 X/Y", FX_DELAY_TYPE},
  {"DRIVE 1 X/Y", FX_DIST_TYPE},
  {"DRIVE 2 X/Y", FX_DIST_TYPE},
  {"FLANGER 1 X/Y", FX_MODULATE_TYPE},
  {"FLANGER 2 X/Y", FX_MODULATE_TYPE},
  {"PHASER 1 X/Y", FX_MODULATE_TYPE},
  {"PHASER 2 X/Y", FX_MODULATE_TYPE},
  {"PITCH 1 X/Y", FX_PITCH_TYPE},
  {"PITCH 2 X/Y", FX_PITCH_TYPE},
  {"REVERB 1 X/Y", FX_REVERB_TYPE},
  {"REVERB 2 X/Y", FX_REVERB_TYPE},
  {"ROTARY 1 X/Y", FX_MODULATE_TYPE},
  {"ROTARY 2 X/Y", FX_MODULATE_TYPE},
  {"WAHWAH 1 X/Y", FX_FILTER_TYPE},
  {"WAHWAH 2 X/Y", FX_FILTER_TYPE}, // CC #121
  {"Ext 1", FX_DEFAULT_TYPE}, // CC #16 - number 85 in the list
  {"Ext 2", FX_DEFAULT_TYPE}, // CC #17
  {"Ext 3", FX_DEFAULT_TYPE}, // CC #18
  {"Ext 4", FX_DEFAULT_TYPE}, // CC #19
  {"Ext 5", FX_DEFAULT_TYPE}, // CC #20
  {"Ext 6", FX_DEFAULT_TYPE}, // CC #21
  {"Ext 7", FX_DEFAULT_TYPE}, // CC #22
  {"Ext 8", FX_DEFAULT_TYPE}, // CC #23
};

const uint16_t AXEFX_NUMBER_OF_PARAMETERS = AXEFX_parameters.size();

#define AXE_FIRST_EXTERNAL_PEDAL 85
#define AXE_FIRST_EXTERNAL_PEDAL_CC 16

QString AXEFX_class::read_parameter_name(uint16_t par_no)
{
    if (par_no < number_of_parameters())  return AXEFX_parameters[par_no].Name;
    else return "?";
}

QString AXEFX_class::read_parameter_state(uint16_t par_no, uint8_t value)
{
    if (par_no < AXE_FIRST_EXTERNAL_PEDAL) { // Show ON/OFF for IA and XY switches
        if (value == 1) return "ON";
        else return "OFF";
      }
    if (par_no < number_of_parameters()) return QString::number(value);
    return "?";
}

uint16_t AXEFX_class::number_of_parameters()
{
    return AXEFX_NUMBER_OF_PARAMETERS;
}

uint8_t AXEFX_class::max_value(uint16_t par_no)
{
    if (par_no < AXE_FIRST_EXTERNAL_PEDAL) return 1; // IA or XY switch
      if (par_no < AXEFX_NUMBER_OF_PARAMETERS) return 127; // External pedal
      return 0; // Out of range
}
