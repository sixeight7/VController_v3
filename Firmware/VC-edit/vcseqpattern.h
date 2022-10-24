#ifndef VCSEQPATTERN_H
#define VCSEQPATTERN_H

// VCSEQPATTERNS is the interface with the Midi_seq_pattern array.

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

class VCseqPattern : public QObject
{
    Q_OBJECT
public:
    explicit VCseqPattern(QObject *parent = nullptr);
    void fillTreeWidget(QTreeWidget *my_tree, VCseqPattern *VCptrn);
    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;
    //void readMidi();
    //void receiveMidiSwitchSettings(uint8_t sw, uint8_t *switchSettings);
    int getSeqPatternSetting(uint8_t pattern, uint8_t parameter) const;
    void setSeqPatternSetting(uint8_t pattern, uint8_t parameter, uint8_t value);

signals:
    void updateProgressBar(int value);

public slots:

private slots:
    void midiSeqPatternChanged(int, int, int);

private:
    QString getPatternNameForTree(int pattern) const;
    QString getPatternNameForJson(int pattern) const;

    enum type_enum { HEADER, OPTION, VALUE };
    struct Seq_pattern_menu_struct {
        QString name;
        type_enum type;
        int sublist;
        int min;
        int max;
        uint8_t parameter;
    };

    const QVector<Seq_pattern_menu_struct> VCseqPatternMenu
    {
        { "Beat divider",  VALUE, 0, 1, 32, 1 },
        { "Number of steps", VALUE, 0, 1, 32, 2 },
        { "STEP 1",  VALUE, 0, 0, 127, 5 },
        { "STEP 2",  VALUE, 0, 0, 127, 6 },
        { "STEP 3",  VALUE, 0, 0, 127, 7 },
        { "STEP 4",  VALUE, 0, 0, 127, 8 },
        { "STEP 5",  VALUE, 0, 0, 127, 9 },
        { "STEP 6",  VALUE, 0, 0, 127, 10 },
        { "STEP 7",  VALUE, 0, 0, 127, 11 },
        { "STEP 8",  VALUE, 0, 0, 127, 12 },
        { "STEP 9",  VALUE, 0, 0, 127, 13 },
        { "STEP 10", VALUE, 0, 0, 127, 14 },
        { "STEP 11", VALUE, 0, 0, 127, 15 },
        { "STEP 12", VALUE, 0, 0, 127, 16 },
        { "STEP 13", VALUE, 0, 0, 127, 17 },
        { "STEP 14", VALUE, 0, 0, 127, 18 },
        { "STEP 15", VALUE, 0, 0, 127, 19 },
        { "STEP 16", VALUE, 0, 0, 127, 20 },
        { "STEP 17", VALUE, 0, 0, 127, 21 },
        { "STEP 18", VALUE, 0, 0, 127, 22 },
        { "STEP 19", VALUE, 0, 0, 127, 23 },
        { "STEP 20", VALUE, 0, 0, 127, 24 },
        { "STEP 21", VALUE, 0, 0, 127, 25 },
        { "STEP 22", VALUE, 0, 0, 127, 26 },
        { "STEP 23", VALUE, 0, 0, 127, 27 },
        { "STEP 24", VALUE, 0, 0, 127, 28 },
        { "STEP 25", VALUE, 0, 0, 127, 29 },
        { "STEP 26", VALUE, 0, 0, 127, 30 },
        { "STEP 27", VALUE, 0, 0, 127, 31 },
        { "STEP 28", VALUE, 0, 0, 127, 32 },
        { "STEP 29", VALUE, 0, 0, 127, 33 },
        { "STEP 30", VALUE, 0, 0, 127, 34 },
        { "STEP 31", VALUE, 0, 0, 127, 35 },
        { "STEP 32", VALUE, 0, 0, 127, 36 },
    };

    const uint16_t NUMBER_OF_SEQ_PATTERN_MENU_ITEMS = VCseqPatternMenu.size();

    QStringList menu_sublist = {
          // Sublist 1 - 3: Booleans
          "OFF", "ON", "DETECT",

          // Sublist 4 - 19: LED colours
          "OFF", "GREEN", "RED", "BLUE", "ORANGE", "CYAN", "WHITE", "YELLOW", "PURPLE", "PINK", "SOFT GREEN", "", "", "", "", "",

          // Sublist 20 - 22: Main display modes
          "PAGE NAME", "PATCH NAME", "PATCHES COMBINED",

          // Sublist 23 - 32: MIDI ports
          "OFF", MIDI_PORT_NAMES,

          // Sublist 33 - 36: Expression pedals
          "EXP PEDAL #1", "EXP PEDAL #2", "EXP PEDAL #3", "EXP PEDAL #4",

          // Sublist 37 - 39: MEP control options
          "NONE", "UP/DOWN", "UP/DN + STEP",

          // Sublist 40 - 47: MIDI switch types
          "OFF", "CC MOMENTARY", "CC TOGGLE", "CC RANGE", "PC", "", "", "",
    };
};

#endif // VCSEQPATTERN_H
