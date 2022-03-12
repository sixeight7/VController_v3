#ifndef GP10_H
#define GP10_H

#include "device.h"
#include <QApplication>

#define GP10_MIDI_CHANNEL 1 // Was unable to change patch when GP-10 channel was not 1. Seems to be a bug in the GP-10
#define GP10_MIDI_PORT USBHMIDI_PORT
#define GP10_PATCH_MIN 0
#define GP10_PATCH_MAX 99
#define GP10_NUMBER_OF_ASSIGNS 8

class GP10_class : public Device_class
{
public:
    GP10_class(uint8_t _dev_no) : Device_class(_dev_no) {}

    virtual void init();
    virtual bool check_command_enabled(uint8_t cmd);
    virtual QString number_format(uint16_t patch_no);
    virtual QString read_parameter_name(uint16_t par_no);
    virtual QString read_parameter_state(uint16_t par_no, uint8_t value);
    virtual uint16_t number_of_parameters();
    virtual uint8_t number_of_values(uint16_t parameter);
    virtual uint8_t max_value(uint16_t par_no);
    virtual uint8_t get_number_of_assigns();
    virtual QString read_assign_name(uint8_t assign_no);
    virtual QString read_assign_trigger(uint8_t trigger_no);
    virtual uint8_t trigger_follow_assign(uint8_t assign_no);
};

#endif // GP10_H
