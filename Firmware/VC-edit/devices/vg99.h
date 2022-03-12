#ifndef VG99_H
#define VG99_H

#include "device.h"
#include <QApplication>

// Roland VG-99 settings:
#define VG99_MIDI_PORT MIDI1_PORT
#define VG99_MIDI_CHANNEL 9
#define VG99_PATCH_MIN 0
#define VG99_PATCH_MAX 399

class VG99_class : public Device_class
{
public:
    VG99_class(uint8_t _dev_no) : Device_class(_dev_no) {}

    virtual void init();
    virtual bool check_command_enabled(uint8_t cmd);
    virtual QString number_format(uint16_t patch_no);
    virtual QString read_parameter_name(uint16_t par_no);
    virtual QString read_parameter_state(uint16_t par_no, uint8_t value);
    virtual uint16_t number_of_parameters();
    virtual uint8_t max_value(uint16_t par_no);
    virtual uint8_t get_number_of_assigns();
    virtual QString read_assign_name(uint8_t assign_no);
    virtual QString read_assign_trigger(uint8_t trigger_no);
    virtual uint8_t trigger_follow_assign(uint8_t assign_no);
};

#endif // VG99_H
