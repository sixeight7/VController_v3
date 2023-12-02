#ifndef VCSWICHSETTINGS_H
#define VCSWICHSETTINGS_H

// VCSWICHSETTINGS is the interface with the Midi_switch array.

#include "customcombobox.h"
#include "customspinbox.h"
#include "customslider.h"
#include "VController/globals.h"
#include "vccommands.h"
#include "VController/leds.h"

#include <QObject>
#include <QDialog>
#include <QTreeWidget>
#include <QString>
#include <QVector>

class VCmidiSwitches : public QObject
{
    Q_OBJECT
public:
    explicit VCmidiSwitches(QObject *parent = nullptr);
    void fillTreeWidget(QTreeWidget *my_tree, VCmidiSwitches *VCm);
    void updateTreeWidget(QTreeWidget *my_tree, VCmidiSwitches *VCm);
    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;
    //void readMidi();
    //void receiveMidiSwitchSettings(uint8_t sw, uint8_t *switchSettings);
    int getMidiSwitchSetting(uint8_t sw, uint8_t parameter) const;
    void setMidiSwitchSetting(uint8_t sw, uint8_t parameter, uint8_t value);

signals:
    void updateProgressBar(int value);

public slots:

private slots:
    void midiSwitchSettingChanged(int, int, int);

private:
    //void setup_devices();
    QTreeWidgetItem *findTopLevelItemByName(QTreeWidget *my_tree, const QString &name);
    QString getSwitchNameForTree(int sw) const;
    QString getSwitchNameForJson(int sw) const;

    enum type_enum { HEADER, OPTION, VALUE };
    struct Switch_midi_menu_struct {
        QString name;
        type_enum type;
        int sublist;
        int min;
        int max;
        uint8_t parameter;
    };

#define MIDI_PORT_SUBLIST 254
    const QVector<Switch_midi_menu_struct> VCmidiSwitchMenu
    {
         { "Type",  OPTION, 1, 0, 4, 1 }, // Switch 2
         { "Midi port", OPTION, MIDI_PORT_SUBLIST, 0, number_of_midi_ports - 1, 2 }, // Switch 3
         { "Midi channel", VALUE, 0, 1, 16, 3 }, // Switch 4
         { "CC",  VALUE, 0, 0, 127, 4 }, // Switch 5
    };

    const uint16_t NUMBER_OF_MIDI_SWITCH_MENU_ITEMS = VCmidiSwitchMenu.size();

    QStringList menu_sublist = {
        // Sublist 1 - 8: MIDI switch types
          "OFF", "CC MOMENTARY", "CC TOGGLE", "CC RANGE", "PC", "", "", "",
    };
};

#endif // VCSWICHSETTINGS_H
