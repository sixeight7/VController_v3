#ifndef GR55_H
#define GR55_H

#include "device.h"
#include <QApplication>

// Roland GR-55 settings:
#define GR55_MIDI_PORT MIDI1_PORT
#define GR55_MIDI_CHANNEL 8
#define GR55_PATCH_MIN 0
#define GR55_PATCH_MAX 656 // Assuming we are in guitar mode!!!
#define GR55_NUMBER_OF_ASSIGNS 8

class GR55_class : public Device_class
{
public:
    GR55_class(uint8_t _dev_no) : Device_class(_dev_no) {}

    virtual void init();
    virtual bool check_command_enabled(uint8_t cmd);
    virtual QString number_format(uint16_t patch_no);
    virtual QString read_parameter_name(uint16_t par_no);
    virtual QString read_parameter_state(uint16_t par_no, uint8_t value);
    virtual uint16_t number_of_parameters();
    virtual uint8_t max_value(uint16_t par_no);
    virtual uint8_t min_value(uint16_t par_no);
    virtual uint8_t get_number_of_assigns();
    virtual QString read_assign_name(uint8_t assign_no);
    virtual QString read_assign_trigger(uint8_t trigger_no);
    virtual uint8_t trigger_follow_assign(uint8_t assign_no);

    uint8_t preset_banks;
};

#endif // GR55_H
