#ifndef M13_H
#define M13_H

#include "device.h"
#include <QApplication>

// Line6 M13 settings:
#define M13_MIDI_PORT MIDI1_PORT
#define M13_MIDI_CHANNEL 1
#define M13_PATCH_MIN 0
#define M13_PATCH_MAX 11

class M13_class : public Device_class
{
public:
    M13_class(uint8_t _dev_no) : Device_class(_dev_no) {}

    virtual void init();
    virtual bool check_command_enabled(uint8_t cmd);
    virtual QString read_parameter_name(uint16_t par_no);
    virtual QString read_parameter_state(uint16_t par_no, uint8_t value);
    virtual uint16_t number_of_parameters();
    virtual uint8_t max_value(uint16_t par_no);
};

#endif // M13_H
