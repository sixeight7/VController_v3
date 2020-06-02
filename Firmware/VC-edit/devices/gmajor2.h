#ifndef GMAJOR2_H
#define GMAJOR2_H


#include "device.h"
#include <QApplication>

// Line6 HELIX settings:
#define GM2_MIDI_CHANNEL 1
#define GM2_MIDI_PORT 1 // Default port is MIDI1
#define GM2_PATCH_MIN 0
#define GM2_PATCH_MAX 199

class GM2_class : public Device_class
{
public:
    GM2_class(uint8_t _dev_no) : Device_class(_dev_no) {}

    virtual void init();
    virtual bool check_command_enabled(uint8_t cmd);
    virtual QString number_format(uint16_t patch_no);
    virtual QString read_parameter_name(uint16_t par_no);
    virtual QString read_parameter_state(uint16_t par_no, uint8_t value);
    virtual uint16_t number_of_parameters();
    virtual uint8_t max_value(uint16_t par_no);
};

#endif // GMAJOR2_H
