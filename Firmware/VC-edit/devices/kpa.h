#ifndef KPA_H
#define KPA_H

#include "device.h"
#include <QApplication>

// Kemper settings:
#define KPA_MIDI_CHANNEL 1
#define KPA_MIDI_PORT 1 // Default port is MIDI1
#define KPA_PATCH_MIN 0
#define KPA_PATCH_MAX 624 // 125 performances of 5 rigs
#define KPA_BROWSE_PATCH_MIN 0
#define KPA_BROWSE_PATCH_MAX 1004 // 5 browse numbers + 1000 rigs
#define KPA_PERFORMANCE_MAX 124 // numbered 0 - 124

#define KPA_MODE_ADDRESS 0x7F7D

class KPA_class : public Device_class
{
public:
    KPA_class(uint8_t _dev_no) : Device_class(_dev_no) {}

    virtual void init();
    virtual bool check_command_enabled(uint8_t cmd);
    virtual QString number_format(uint16_t patch_no);
    virtual QString read_parameter_name(uint16_t par_no);
    virtual QString read_parameter_state(uint16_t par_no, uint8_t value);
    virtual uint16_t number_of_parameters();
    virtual uint8_t max_value(uint16_t par_no);
    virtual QString get_patch_info(uint16_t number);

    uint8_t model_number;
};

#endif // KPA_H
