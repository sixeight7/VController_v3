#ifndef HELIX_H
#define HELIX_H

#include "device.h"
#include <QApplication>

// Line6 HELIX settings:
#define HLX_MIDI_CHANNEL 1
#define HLX_MIDI_PORT 1 // Default port is MIDI1
#define HLX_PATCH_MIN 0
#define HLX_PATCH_MAX 127
#define HLX_NUMBER_OF_SETLISTS 7

class HLX_class : public Device_class
{
public:
    HLX_class(uint8_t _dev_no) : Device_class(_dev_no) {}

    virtual void init();
    virtual bool check_command_enabled(uint8_t cmd);
    virtual QString number_format(uint16_t patch_no);
    virtual QString read_parameter_name(uint16_t par_no);
    virtual QString read_parameter_state(uint16_t par_no, uint8_t value);
    virtual uint16_t number_of_parameters();
    virtual uint8_t max_value(uint16_t par_no);
    virtual QString setlist_song_full_item_format(uint16_t item);
    virtual QString setlist_song_short_item_format(uint16_t item);
    virtual int setlist_song_get_number_of_items();
};

#endif // HELIX_H
