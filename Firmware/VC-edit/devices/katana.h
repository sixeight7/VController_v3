#ifndef KATANA_H
#define KATANA_H

#include "device.h"
#include <QApplication>

// Boss KATANA settings:
#define KTN_MIDI_CHANNEL 1
#define KTN_PATCH_MIN 0
#define KTN_PATCH_MAX 88

class KTN_class : public Device_class
{
public:
    KTN_class(uint8_t _dev_no) : Device_class(_dev_no) {}

    virtual void init();
    virtual bool check_command_enabled(uint8_t cmd);
    virtual QString number_format(uint16_t patch_no);
    virtual QString read_parameter_name(uint16_t par_no);
    virtual QString read_parameter_state(uint16_t par_no, uint8_t value);
    virtual uint16_t number_of_parameters();
    virtual uint8_t max_value(uint16_t par_no);
};

#endif // KATANA_H
