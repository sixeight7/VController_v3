#ifndef KATANA_H
#define KATANA_H

#include "device.h"
#include "VController/config.h"
#include <QApplication>

// Boss KATANA settings:
#define KTN_MIDI_PORT USBHMIDI_PORT
#define KTN_MIDI_CHANNEL 1
#define KTN_PATCH_MAX 158

class KTN_class : public Device_class
{
public:
    KTN_class(uint8_t _dev_no) : Device_class(_dev_no) {}

    virtual void init();
    virtual bool check_command_enabled(uint8_t cmd);
    virtual QString number_format(uint16_t patch_no);
    virtual QString read_parameter_name(uint16_t par_no);
    virtual QString read_parameter_state(uint16_t par_no, uint8_t value);
    virtual uint16_t number_of_parameters();
    virtual uint8_t max_value(uint16_t par_no);
    virtual QString get_patch_info(uint16_t number);
    void WritePatchName(int number, QString name);
    QString ReadPatchName(int number);
    virtual uint8_t supportPatchSaving();
    virtual void readPatchData(int index, int patch_no, const QJsonObject &json);
    virtual void writePatchData(int patch_no, QJsonObject &json) const;
    virtual QString patchFileHeader();
    virtual QString DefaultPatchFileName(int index);
    virtual uint16_t patch_min_as_stored_on_VC();

private:
    uint8_t ktn_patch_min;
    uint8_t ktn_bank_size;
};

#endif // KATANA_H
