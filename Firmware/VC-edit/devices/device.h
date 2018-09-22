#ifndef DEVICE_H
#define DEVICE_H

#include <QApplication>

#define DEVICE_OFF 0
#define DEVICE_ON 1
#define DEVICE_DETECT 2

class Device_class
{
public:
    uint8_t my_device_number;
    Device_class(uint8_t _dev_no) : my_device_number(_dev_no) {}
    virtual void init();
    uint8_t get_setting(uint8_t variable);
    void set_setting(uint8_t variable, uint8_t value);
    QString get_setting_name(uint8_t variable);

    virtual bool check_command_enabled(uint8_t cmd);
    virtual QString number_format(uint16_t patch_no);
    virtual QString read_parameter_name(uint16_t par_no);
    virtual QString read_parameter_state(uint16_t par_no, uint8_t value);
    virtual uint16_t number_of_parameters();
    virtual uint8_t min_value(uint16_t par_no);
    virtual uint8_t max_value(uint16_t par_no);
    virtual uint8_t get_number_of_assigns();
    virtual QString read_assign_name(uint8_t assign_no);
    virtual QString read_assign_trigger(uint8_t assign_no);
    virtual uint8_t trigger_follow_assign(uint8_t assign_no);

    // Variables
    QString device_name;
    QString full_device_name;
    uint16_t patch_number;
    uint16_t patch_max;
    uint16_t patch_min;

    uint8_t my_LED_colour;
    uint8_t MIDI_channel;
    uint8_t MIDI_port_manual;
    uint8_t MIDI_device_id;
    uint8_t bank_number;
    uint8_t is_always_on;
    uint8_t my_device_page1;
    uint8_t my_device_page2;
    uint8_t my_device_page3;
    uint8_t my_device_page4;
    uint8_t enabled;

#define NUMBER_OF_DEVICE_SETTINGS 11
};

#endif // DEVICE_H
