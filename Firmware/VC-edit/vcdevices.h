#ifndef VCDEVICES_H
#define VCDEVICES_H

// VCdevices is the interface with the Device[] class objects. Device settings are stored in the class objects.

#include "customcombobox.h"
#include "customspinbox.h"
#include "customslider.h"
#include "VController/globaldevices.h"
#include "vccommands.h"
#include "VController/leds.h"
#include "VController/globals.h"

#include <QObject>
#include <QDialog>
#include <QTreeWidget>
#include <QString>
#include <QVector>

class VCdevices : public QObject
{
    Q_OBJECT
public:
    explicit VCdevices(QObject *parent = nullptr);
    void fillTreeWidget(QTreeWidget *my_tree, VCcommands *VCd);
    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;

    QByteArray ReadPatch(int number);
    QString ReadPatchStringForListWidget(int number, int type);
    int findIndex(int type, int patch_no);
    int newIndex();
    int numberOfPatches();
    int indexFromMode(int type, int number);
    int getDevicePatchDeviceNumber(int number, int type);
    int getDevicePatchType(int number, int type);
    int getDevicePatchNumber(int number, int type);
    void WritePatch(int type, int number, QByteArray patch);
    void InitializePatchArea();
    void readAll(const QJsonObject &json);
    void readPatchData(int patch_no, const QJsonObject &json, int my_type);
    void readAllLegacyKatana(const QJsonObject &json);
    void writeAll(QJsonObject &json) const;
    void writePatchData(int patch_no, QJsonObject &json) const;
    void swapPatch(int patch_no1, int patch_no2, int type);
    void movePatch(int source_patch, int dest_patch, int type);
    void copyPatch(int number, int type);
    void pastePatch(int number, int type);
    void clearCopyBuffer();
    void initializePatch(int number, int type);

#define PATCH_INDEX_NOT_FOUND 0xFFFF

signals:

public slots:

private slots:
    void deviceSettingChanged(int, int, int);
    void devicePageSettingChanged(int, int, int);

private:
    void setup_devices();

    QVector<uint8_t> Patch_copy_buffer;
    bool copyBufferFilled = false;

    enum type_enum { HEADER, OPTION, VALUE };
    struct Device_menu_struct {
        QString name;
        type_enum type;
        int sublist;
        int min;
        int max;
        uint8_t parameter;
    };

#define PAGE_SUBLIST 254

    const QVector<Device_menu_struct> VCdeviceMenu
    {
        { "Enabled", OPTION, 1, 0, 2, 10 }, // Switch 2
        { "Midi channel", VALUE, 0, 1, 16, 1 }, // Switch 3
        { "Midi port", OPTION, 23, 0, NUMBER_OF_MIDI_PORTS, 2 }, // Switch 4
        { "Colour", OPTION, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, 0 }, // Switch 9
        { "Is always on", OPTION, 1, 0, 1, 5 }, // Switch 10
        { "Device page #1", OPTION, PAGE_SUBLIST, 0, LAST_FIXED_CMD_PAGE, 6 },// Switch 5
        { "Device page #2", OPTION, PAGE_SUBLIST, 0, LAST_FIXED_CMD_PAGE, 7 }, // Switch 6
        { "Device page #3", OPTION, PAGE_SUBLIST, 0, LAST_FIXED_CMD_PAGE, 8 }, // Switch 7
        { "Device page #4", OPTION, PAGE_SUBLIST, 0, LAST_FIXED_CMD_PAGE, 9 }, // Switch 8
    };

    const uint16_t NUMBER_OF_DEVICE_MENU_ITEMS = VCdeviceMenu.size();

    QStringList menu_sublist = {
      // Sublist 1 - 2: Booleans
      "OFF", "ON", "DETECT",

      // Sublist 4 - 19: LED colours
      "OFF", "GREEN", "RED", "BLUE", "ORANGE", "CYAN", "WHITE", "YELLOW", "PURPLE", "PINK", "SOFT GREEN", "", "", "", "", "",

      // Sublist 20 - 22: Main display modes
      "PAGE NAME", "PATCH NAME", "PATCHES COMBINED",

      // Sublist 23 - 30: MIDI ports
      "USB MIDI", "MIDI 1", "MIDI2/RRC", "MIDI 3", "USB MIDI HOST", "ALL PORTS", "", "",

      // Sublist 31 - 34: Expression pedals
      "EXP PEDAL #1", "EXP PEDAL #2", "EXP PEDAL #3", "EXP PEDAL #4",
    };

};

#endif // VCDEVICES_H
