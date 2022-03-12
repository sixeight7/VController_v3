#ifndef ZMS70_H
#define ZMS70_H

#include "device.h"
#include <QApplication>

// Zoom MS70CDR settings:
#define ZMS70_MIDI_PORT USBHMIDI_PORT
#define ZMS70_MIDI_CHANNEL 1
#define ZMS70_PATCH_MIN 0
#define ZMS70_PATCH_MAX 49

class ZMS70_class : public Device_class
{
public:
    ZMS70_class(uint8_t _dev_no) : Device_class(_dev_no) {}

    virtual void init();
    virtual bool check_command_enabled(uint8_t cmd);
    virtual QString number_format(uint16_t patch_no);
    virtual QString read_parameter_name(uint16_t par_no);
    virtual QString read_parameter_state(uint16_t par_no, uint8_t value);
    virtual uint16_t number_of_parameters();
    virtual uint8_t max_value(uint16_t par_no);
};

#endif // ZMS70_H
