#ifndef VCSWICHSETTINGS_H
#define VCSWICHSETTINGS_H

// VCSWICHSETTINGS is the interface with the Device[] class objects. Device settings are stored in the class objects.

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
    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;
    //void readMidi();
    //void receiveMidiSwitchSettings(uint8_t sw, uint8_t *switchSettings);
    int getMidiSwitchSetting(uint8_t sw, uint8_t parameter) const;
    void setMidiSwitchSetting(uint8_t sw, uint8_t parameter, uint8_t value);

signals:

public slots:

private slots:
    void midiSwitchSettingChanged(int, int, int);

private:
    //void setup_devices();
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

#define SWITCH_SUBLIST 252
    const QVector<Switch_midi_menu_struct> VCmidiSwitchMenu
    {
         { "Type",  OPTION, 38, 0, 4, 1 }, // Switch 2
         { "Midi port", OPTION, 23, 0, NUMBER_OF_MIDI_PORTS, 2 }, // Switch 3
         { "Midi channel", VALUE, 0, 1, 16, 3 }, // Switch 4
         { "CC",  VALUE, 0, 0, 127, 4 }, // Switch 5
    };

    const uint16_t NUMBER_OF_MIDI_SWITCH_MENU_ITEMS = VCmidiSwitchMenu.size();

    QStringList menu_sublist = {
          // Sublist 1 - 3: Booleans
          "OFF", "ON", "DETECT",

          // Sublist 4 - 19: LED colours
          "OFF", "GREEN", "RED", "BLUE", "ORANGE", "CYAN", "WHITE", "YELLOW", "PURPLE", "PINK", "SOFT GREEN", "", "", "", "", "",

          // Sublist 20 - 22: Main display modes
          "PAGE NAME", "PATCH NAME", "PATCHES COMBINED",

          // Sublist 23 - 30: MIDI ports
          "USB MIDI", "MIDI 1", "MIDI2/RRC", "MIDI 3", "USB HOST PORT", "ALL PORTS", "", "",

          // Sublist 31 - 34: Expression pedals
          "EXP PEDAL #1", "EXP PEDAL #2", "EXP PEDAL #3", "EXP PEDAL #4",

          // Sublist 35 - 37: MEP control options
          "NONE", "UP/DOWN", "UP/DN + STEP",

          // Sublist 38 - 45: MIDI switch types
          "OFF", "CC MOMENTARY", "CC TOGGLE", "CC RANGE", "PC", "", "", "",
    };

    QStringList switch_name = {
    #ifndef IS_VCMINI
        "On Page Select", "Switch 1", "Switch 2", "Switch 3", "Switch 4", "Switch 5", "Switch 6", "Switch 7", "Switch 8",
        "Switch 9", "Switch 10", "Switch 11", "Switch 12", "Switch 13", "Switch 14", "Switch 15", "Switch 16",
        "Ext 1 / Exp1", "Ext 2", "Ext 3 / Exp 2", "Ext 4", "Ext 5 / Exp 3", "Ext 6", "Ext 7 / Exp 4", "Ext 8",
    #else
        "On Page Select", "Switch 1", "Switch 2", "Switch 3", "Encoder #1", "Encoder #1 Switch", "Encoder #2", "Encoder #2 Switch", "Ext 1 / Exp1",
        "Ext 2", "MIDI Switch 1 / SW4", "MIDI Switch 2 / SW5", "MIDI Switch 3 / SW6", "MIDI Switch 4", "MIDI Switch 5", "MIDI Switch 6", "MIDI Switch 7",
        "MIDI Switch 8", "MIDI Switch 9", "MIDI Switch 10", "MIDI Switch 11", "MIDI Switch 12", "MIDI Switch 13", "MIDI Switch 14", "MIDI Switch 15",
    #endif
    };

};

#endif // VCSWICHSETTINGS_H
