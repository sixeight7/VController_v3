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

#define TYPE_HELIX_01A 0
#define TYPE_HELIX_000 1
#define TYPE_HX_STOMP_01A 2
#define TYPE_HX_STOMP_000 3
#define TYPE_HX_STOMP_XL_01A 4
#define TYPE_HX_STOMP_XL_000 5
#define TYPE_HX_EFFECTS_01A 6
#define TYPE_HX_EFFECTS_000 7

class HLX_class : public Device_class
{
public:
    HLX_class(uint8_t _dev_no) : Device_class(_dev_no) {}

    virtual void init();
    virtual uint8_t get_number_of_dev_types();
    virtual QString get_dev_type_name(uint8_t number);
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
