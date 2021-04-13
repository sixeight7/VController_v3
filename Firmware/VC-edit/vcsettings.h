#ifndef VCSETTINGS_H
#define VCSETTINGS_H

// VCsettings is the interface with the Settings struct, that contains the settings of the VController.

#include "customcombobox.h"
#include "customspinbox.h"
#include "customslider.h"
#include "VController/globals.h"
#include "VController/config.h"
#include "VController/leds.h"

#include <QObject>
#include <QDialog>
#include <QTreeWidget>
#include <QString>
#include <QVector>

class VCsettings : public QObject
{
    Q_OBJECT

public:
    explicit VCsettings(QObject *parent = nullptr);
    void fillTreeWidget(QTreeWidget *my_tree);
    void showData();
    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;
    //void readMidi();

public slots:

private slots:
    void settingChanged(int, int, int);

private:

    enum type_enum { HEADER, OPTION, VALUE };
    struct Setting_menu_struct {
        QString name;
        type_enum type;
        int sublist;
        int min;
        int max;
        uint8_t* value;
    };

#define DEVICE_SUBLIST 255

    const QVector<Setting_menu_struct> VCsettingMenu =
    {
      { "General Settings", HEADER, 0, 0, 0, 0 },
      { "Main display mode shows on bottom line", OPTION, 19, 0, 3, &Setting.Main_display_mode },// Switch 1
      { "Main display shows top right", OPTION, 48, 0, 2, &Setting.Main_display_show_top_right }, // Switch 2
      { "CURNUM action", OPTION, 51, 0, 5, &Setting.CURNUM_action }, // Switch 3
      { "Master Expression Pedal also controls", OPTION, 35, 0, 2, &Setting.MEP_control }, // Switch 4
      { "Send Global Tempo on patch change", OPTION, 1, 0, 1, &Setting.Send_global_tempo_after_patch_change }, // Switch 5
      { "Hide tap tempo LED", OPTION, 1, 0, 1, &Setting.Hide_tap_tempo_LED }, // Switch 6

      { "LED Settings", HEADER, 0, 0, 0, 0 }, // Menu title
      { "LED Brightness", VALUE, 0, 0, 100, &Setting.LED_brightness }, // Switch 1
      { "Backlight Brightness", VALUE, 0, 0, 254, &Setting.Backlight_brightness }, // Switch 2
      { "Show Virtual LEDs", OPTION, 1, 0, 1, &Setting.Virtual_LEDs },// Switch 3
      { "FX off is dimmed", OPTION, 1, 0, 1, &Setting.LED_FX_off_is_dimmed }, // Switch 4
      { "Global colour", OPTION, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.LED_global_colour }, // Switch 5
      { "MIDI PC colour", OPTION, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.MIDI_PC_colour }, // Switch 6
      { "MIDI CC colour", OPTION, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.MIDI_CC_colour }, // Switch 7
      { "MIDI note colour", OPTION, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.MIDI_note_colour }, // Switch 8
      { "BPM colour", OPTION, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.LED_bpm_colour }, // Switch 9
      { "BPM synced colour", OPTION, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.LED_bpm_synced_colour }, // Switch 10
      { "RGB display colour scheme", OPTION, 46, 0, 1, &Setting.RGB_Backlight_scheme }, // Switch 11

      { "LED FX colours", HEADER, 0, 0, 0, 0 }, // Menu title
      { "GTR/COSM colour", OPTION, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_GTR_colour }, // Switch 1
      { "PITCH FX colour", OPTION, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_PITCH_colour }, // Switch 2
      { "FILTER FX colour", OPTION, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_FILTER_colour },// Switch 3
      { "DIST FX colour", OPTION, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_DIST_colour }, // Switch 4
      { "AMP FX colour", OPTION, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_AMP_colour }, // Switch 5
      { "MOD FX colour", OPTION, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_MODULATION_colour }, // Switch 5
      { "DELAY FX colour", OPTION, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_DELAY_colour }, // Switch 7
      { "REVERB FX colour", OPTION, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_REVERB_colour}, // Switch 8
      { "LOOPER colour", OPTION, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_LOOPER_colour }, // Switch 9
      { "DEFAULT colour", OPTION, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_default_colour }, // Switch 10
      { "WAH COLOUR", OPTION, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_WAH_colour }, // Switch 11
      { "DYNAMICS COLOUR", OPTION, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.FX_DYNAMICS_colour }, // Switch 12

      { "MIDI advanced settings", HEADER, 0, 0, 0, 0 }, // Menu title
      { "Read MIDI clock", OPTION, 23, 0, NUMBER_OF_MIDI_PORTS + 1, &Setting.Read_MIDI_clock_port }, // Switch 1
      { "Send MIDI clock", OPTION, 23, 0, NUMBER_OF_MIDI_PORTS + 1, &Setting.Send_MIDI_clock_port }, // Switch 2
      { "Bass mode Guitar-to-MIDI channel", VALUE, 0, 1, 16, &Setting.Bass_mode_G2M_channel }, // Switch 5
      { "Bass mode device", OPTION, DEVICE_SUBLIST, 0, NUMBER_OF_DEVICES - 1, &Setting.Bass_mode_device }, // Switch 6
      { "Bass mode CC number", VALUE, 0, 0, 127, &Setting.Bass_mode_cc_number }, // Switch 7
      { "Bass mode min velocity", VALUE, 0, 0, 127, &Setting.Bass_mode_min_velocity}, // Switch 8
      { "HighNotePriotyCC", VALUE, 0, 0, 127, &Setting.HNP_mode_cc_number }, // Switch 9
    };

    const uint16_t NUMBER_OF_SETTINGS_MENU_ITEMS = VCsettingMenu.size();

    QStringList menu_sublist = {
      // Sublist 1 - 3: Booleans
      "OFF", "ON", "DETECT",

      // Sublist 4 - 18: LED colours
      "OFF", "GREEN", "RED", "BLUE", "ORANGE", "CYAN", "WHITE", "YELLOW", "PURPLE", "PINK", "SOFT GREEN", "LIGHT BLUE", "", "", "",

      // Sublist 19 - 22: Main display modes
      "PAGE NAME", "PATCH NAME", "PATCHES COMBINED", "VCMINI LABELS",

      // Sublist 23 - 30: MIDI ports
      "OFF", "USB MIDI", "MIDI 1", "MIDI2/RRC", "MIDI 3", "USB MIDI HOST", "ALL PORTS", "",

      // Sublist 31 - 34: Expression pedals
      "EXP PEDAL #1", "EXP PEDAL #2", "EXP PEDAL #3", "EXP PEDAL #4",

      // Sublist 35 - 37: MEP control options
      "NONE", "UP/DOWN buttons", "UP/DN + STEP buttons",

      // Sublist 38 - 45: MIDI switch types
      "OFF", "CC MOMENTARY", "CC SINGLE SHOT", "CC RANGE", "PC", "", "", "",

      // Sublist 46 - 47: RGB Display colour schemes
      "ADAFRUIT", "BUYDISPLAY",

      // Sublist 48 - 50: Main display top right types
      "CURRENT DEVICE", "CURRENT TEMPO", "SCENE NAME",

      // Sublist 51 - 56: Current number actions
      "OFF", "PREVIOUS PATCH", "TAP TEMPO", "TUNER", "US20 EMULATION", "DIRECT SELECT", "",
    };
};

#endif // VCSETTINGS_H
