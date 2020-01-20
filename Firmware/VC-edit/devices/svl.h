#ifndef SVL_H
#define SVL_H

#include "device.h"
#include <QApplication>

// Strymon Volante settings:
#define SVL_MIDI_CHANNEL 1
#define SVL_MIDI_PORT 1 // Default port is MIDI1
#define SVL_PATCH_MIN 0
#define SVL_PATCH_MAX 299

class SVL_class : public Device_class
{
public:
    SVL_class(uint8_t _dev_no) : Device_class(_dev_no) {}

    virtual void init();
    virtual bool check_command_enabled(uint8_t cmd);
    virtual QString number_format(uint16_t patch_no);
    virtual QString read_parameter_name(uint16_t par_no);
    virtual QString read_parameter_state(uint16_t par_no, uint8_t value);
    virtual uint16_t number_of_parameters();
    virtual uint8_t max_value(uint16_t par_no);
};

#endif // SVL_H
