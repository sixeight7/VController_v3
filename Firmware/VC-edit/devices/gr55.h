#ifndef GR55_H
#define GR55_H

#include "device.h"
#include <QApplication>

// Roland GR-55 settings:
#define GR55_MIDI_PORT MIDI1_PORT
#define GR55_MIDI_CHANNEL 8
#define GR55_PATCH_MIN 0
#define GR55_PATCH_MAX 656 // Assuming we are in guitar mode!!!
#define GR55_NUMBER_OF_SCENE_ASSIGNS 8
#define GR55_NUMBER_OF_CONTROL_PEDALS 4

#define GR55_PATCH_TYPE 0
#define GR55_PATCH_NUMBER_MSB 1
#define GR55_PATCH_NUMBER_LSB 2
#define GR55_CURRENT_SCENE_BYTE 5
#define GR55_SCENE_ACTIVE_BYTE 6

#define GR55_SCENE_SIZE 23
#define GR55_COMMON_DATA_SIZE 8

#define GR55_INST_DATA_OFFSET 12
#define GR55_SCENE_ASSIGN_BYTE_OFFSET 14
#define GR55_SCENE_NAME_BYTE 15

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

    virtual QString get_patch_info(uint16_t number);
    virtual QString read_scene_name_from_buffer(int number, uint8_t scene) const;
    virtual void store_scene_name_to_buffer(int number, uint8_t scene, QString name);
    int get_scene_index(uint8_t scene) const;
    bool check_snapscene_active(const int number, const uint8_t scene) const;

    virtual uint8_t supportPatchSaving();
    virtual void readPatchData(int index, int patch_no, const QJsonObject &json);
    virtual void writePatchData(int patch_no, QJsonObject &json) const;

    uint8_t preset_banks;
};

#endif // GR55_H
