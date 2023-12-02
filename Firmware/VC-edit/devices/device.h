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
    virtual uint8_t get_number_of_dev_types();
    virtual QString get_dev_type_name(uint8_t number);
    QString get_setting_name(uint8_t variable);
    virtual QString get_patch_info(uint16_t number);
    virtual void rename_patch(uint16_t number);

    virtual bool check_command_enabled(uint8_t cmd);
    virtual QString number_format(uint16_t patch_no);
    QString build_patch_number(uint16_t number, QString firstPatch, QString lastPatch);
    uint8_t convert_mask_number(QChar c);
    virtual QString read_parameter_name(uint16_t par_no);
    virtual QString read_parameter_state(uint16_t par_no, uint8_t value);
    virtual uint16_t number_of_parameters();
    virtual uint8_t number_of_values(uint16_t parameter);
    virtual uint8_t min_value(uint16_t par_no);
    virtual uint8_t max_value(uint16_t par_no);
    virtual uint8_t get_number_of_assigns();
    virtual QString read_assign_name(uint8_t assign_no);
    virtual QString read_assign_trigger(uint8_t assign_no);
    virtual uint8_t trigger_follow_assign(uint8_t assign_no);
    virtual QString read_scene_name_from_buffer(int number, uint8_t scene) const;
    virtual void store_scene_name_to_buffer(int number, uint8_t scene, QString name);
    virtual uint8_t get_number_of_snapscenes();

    virtual uint8_t supportPatchSaving();
    virtual void readPatchData(int index, int patch_no, const QJsonObject &json);
    virtual void writePatchData(int patch_no, QJsonObject &json) const;
    virtual QString patchFileHeader();
    virtual QString DefaultPatchFileName(int index);
    QString numConv(int number) const;
    virtual uint16_t patch_min_as_stored_on_VC();
    virtual QString setlist_song_full_item_format(uint16_t item);
    virtual QString setlist_song_short_item_format(uint16_t item);
    virtual int setlist_song_get_number_of_items();

    // Variables
    QString device_name;
    QString full_device_name;
    uint16_t patch_number;
    uint16_t patch_max;
    uint16_t patch_min;
    uint16_t patch_max_as_stored_on_VC = 0;

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
    uint8_t dev_type = 0;

protected:
    uint8_t MIDI_port_number(uint8_t port);

#define NUMBER_OF_DEVICE_SETTINGS 12
};

#endif // DEVICE_H
