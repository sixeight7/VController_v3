#ifndef SY1000_H
#define SY1000_H


#include "device.h"
#include <QApplication>

#define SY1000_MIDI_PORT USBHMIDI_PORT
#define SY1000_MIDI_CHANNEL 1
#define SY1000_PATCH_MIN 0
#define SY1000_PATCH_MAX 399
#define SY1000_NUMBER_OF_CTL_FUNCTIONS 18
#define SY1000_NUMBER_OF_ASSIGNS 16

#define SY1000_PATCH_TYPE 0
#define SY1000_PATCH_NUMBER_MSB 1
#define SY1000_PATCH_NUMBER_LSB 2
#define SY1000_CURRENT_SCENE_BYTE 5
#define SY1000_SCENE_ACTIVE_BYTE 6

#define SY1000_SCENE_SIZE 23
#define SY1000_COMMON_DATA_SIZE 8

#define SY1000_INST_DATA_OFFSET 8
#define SY1000_SCENE_ASSIGN_BYTE_OFFSET 14
#define SY1000_SCENE_NAME_BYTE 15

class SY1000_class : public Device_class
{
public:
    SY1000_class(uint8_t _dev_no) : Device_class(_dev_no) {}

    virtual void init();
    virtual bool check_command_enabled(uint8_t cmd);
    virtual QString number_format(uint16_t patch_no);
    virtual QString read_parameter_name(uint16_t par_no);
    virtual QString read_parameter_state(uint16_t par_no, uint8_t value);
    virtual uint16_t number_of_parameters();
    virtual uint8_t number_of_values(uint16_t parameter);
    virtual uint8_t max_value(uint16_t par_no);
    virtual uint8_t get_number_of_assigns();
    virtual QString read_assign_name(uint8_t assign_no);
    virtual QString read_assign_trigger(uint8_t trigger_no);
    virtual uint8_t trigger_follow_assign(uint8_t assign_no);

    virtual QString get_patch_info(uint16_t number);
    virtual QString read_scene_name_from_buffer(int number, uint8_t scene) const;
    virtual void store_scene_name_to_buffer(int number, uint8_t scene, QString name);
    int get_scene_index(uint8_t scene) const;
    bool check_snapscene_active(const int number, const uint8_t scene) const;

    virtual uint8_t supportPatchSaving();
    virtual void readPatchData(int index, int patch_no, const QJsonObject &json);
    virtual void writePatchData(int patch_no, QJsonObject &json) const;
};

#endif // SY1000_H
