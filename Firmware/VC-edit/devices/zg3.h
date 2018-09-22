#ifndef ZG3_H
#define ZG3_H

#include "device.h"
#include <QApplication>

// Zoom G3 settings:
#define ZG3_MIDI_CHANNEL 1
#define ZG3_PATCH_MIN 0
#define ZG3_PATCH_MAX 99

class ZG3_class : public Device_class
{
public:
    ZG3_class(uint8_t _dev_no) : Device_class(_dev_no) {}

    virtual void init();
    virtual bool check_command_enabled(uint8_t cmd);
    virtual QString number_format(uint16_t patch_no);
    virtual QString read_parameter_name(uint16_t par_no);
    virtual QString read_parameter_state(uint16_t par_no, uint8_t value);
    virtual uint16_t number_of_parameters();
    virtual uint8_t max_value(uint16_t par_no);
};

#endif // ZG3_H
