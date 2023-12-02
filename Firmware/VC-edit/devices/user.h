#ifndef USER_H
#define USER_H

#include "device.h"
#include <QApplication>
#include "VController/globals.h"

// User Device settings:
#define USER_MIDI_CHANNEL 1
#define USER_MIDI_PORT MIDI1_PORT // Default port is MIDI1
#define USER_PATCH_MIN 0
#define USER_PATCH_MAX 299

#define USER_MOMENTARY 0
#define USER_TOGGLE 1
#define USER_STEP 2
#define USER_RANGE 3
#define USER_UPDOWN 4
#define USER_ONE_SHOT 5

#define USER_PAR_EXP1 8
#define USER_PAR_EXP2 9
#define USER_PAR_EXP_TGL 10
#define USER_PAR_TAP_TEMPO 11
#define USER_PAR_TUNER 12
#define USER_PAR_SCENE_SELECT 13
#define USER_PAR_LOOPER_STOP 14
#define USER_PAR_LOOPER_REC 15
#define USER_PAR_LOOPER_PLAY 16
#define USER_PAR_LOOPER_OVERDUB 17
#define USER_PAR_LOOPER_FORWARD 18
#define USER_PAR_LOOPER_REVERSE 19
#define USER_PAR_LOOPER_FULLSPEED 20
#define USER_PAR_LOOPER_HALFSPEED 21
#define USER_PAR_LOOPER_UNDO 22
#define USER_PAR_LOOPER_REDO 23
#define USER_PAR_LOOPER_PLAY_ONCE 24

class USER_class : public Device_class
{
public:
    USER_class(uint8_t _dev_no) : Device_class(_dev_no) {}
    virtual void init();
    void init_from_device_data();
    void initialize_device_data();
    virtual QString number_format(uint16_t number);
    virtual bool check_command_enabled(uint8_t cmd);
    User_device_struct get_device_data() { return device_data; }
    void set_device_data(User_device_struct *data);
    QString get_full_name() { return device_data.full_name; };
    void set_full_name(QString name);
    QString get_short_name() { return device_data.short_name; };
    void set_short_name(QString name);
    virtual QString read_parameter_name(uint16_t par_no);
    QString read_parameter_name_for_table(uint16_t par_no);
    virtual uint16_t number_of_parameters();
    virtual uint8_t get_number_of_snapscenes();

private:
    int find_user_data_index(uint8_t typedev, uint16_t patch_number);

    // Variables:
    uint8_t my_instance = 0;
    //uint8_t my_number = 1;
    //uint8_t last_selected_parameter = 255;
    const User_device_struct default_device_data = {
        "USER DEVICE X   ",  // char full_name[17];
        "USERX ",            // char short_name[7];
        0, 0,                // uint8_t patch_min_msb and lsb;
        0, 99,               // uint8_t patch_max_msb and lsb;
        "001   ",            // char first_patch_format[7];
        "099   ",            // char last_patch_format[7];
        {0, 0, 0, 0},        // device_detection[4]
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // uint8_t parameter_CC[25];
        {127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 7, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127},// uint8_t parameter_value_max[25];
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // uint8_t parameter_value_min[13];
        {USER_TOGGLE, USER_TOGGLE, USER_TOGGLE, USER_TOGGLE, USER_TOGGLE, USER_TOGGLE, USER_TOGGLE, USER_TOGGLE,
         USER_RANGE, USER_RANGE, USER_TOGGLE, USER_ONE_SHOT, USER_ONE_SHOT}, // uint8_t parameter_type[13];
        0,                   // uint8_t looper_length;
        1,                   // pc_type
    };
    User_device_struct device_data;
    const QStringList par_name = {
        "FX 1", "FX 2", "FX 3", "FX 4", "FX 5", "FX 6", "FX 7", "FX 8", "EXP1", "EXP2",
        "EXP TOGGLE", "TAP TEMPO", "TUNER", "SCENE SELECT", "LOOPER STOP", "LOOPER PLAY", "LOOPER REC", "LOOPER OVERDUB", "LOOPER FORWARD", "LOOPER REVERSE",
        "LOOPER FULLSPEED", "LOOPER HALFSPEED", "LOOPER UNDO", "LOOPER REDO", "LOOPER PLAY_ONCE"
    };
};

#endif // USER_H
