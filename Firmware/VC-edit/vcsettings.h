#ifndef VCSETTINGS_H
#define VCSETTINGS_H

// VCsettings is the interface with the Settings struct, that contains the settings of the VController.

#include "customcombobox.h"
#include "customspinbox.h"
#include "customslider.h"
#include "VController/globals.h"
#include "VController/config.h"
#include "VController/leds.h"
#include "VController/hardware.h"

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
    void updateTreeWidget(QTreeWidget *my_tree);
    void showData();
    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;
    //void readMidi();

signals:
        void updateProgressBar(int value);
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
#define MIDI_PORT_SUBLIST 254

    const QVector<Setting_menu_struct> VCsettingMenu =
    {
      { "General Settings", HEADER, 0, 0, 0, 0 },
      { "Main disp top", OPTION, 19, 0, 5, &Setting.Main_display_top_line_mode },// Switch 1
      { "Main disp top+", OPTION, 53, 0, 5, &Setting.Main_display_show_top_right }, // Switch 2
      { "Main disp bottom", OPTION, 19, 0, 5, &Setting.Main_display_bottom_line_mode },// Switch 3
      { "CURNUM action", OPTION, 60, 0, 5, &Setting.CURNUM_action }, // Switch 4
      { "MEP also cntrols", OPTION, 40, 0, 2, &Setting.MEP_control }, // Switch 5
      { "Glob.tempo on PC", OPTION, 1, 0, 1, &Setting.Send_global_tempo_after_patch_change }, // Switch 6
      { "Hide tempo LED", OPTION, 1, 0, 1, &Setting.Hide_tap_tempo_LED }, // Switch 7
      { "Backlight Type", OPTION, 51, 0, 1, &Setting.RGB_Backlight_scheme }, // Switch 8

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
      { "BPM follow color", OPTION, 4, 0, NUMBER_OF_SELECTABLE_COLOURS - 1, &Setting.LED_bpm_follow_colour },// Switch 11
      { "RGB display colour scheme", OPTION, 48, 0, 1, &Setting.RGB_Backlight_scheme }, // Switch 12

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

      { "MIDI Advanced Settings", HEADER, 0, 0, 0, 0 }, // Menu title
      { "Read MIDI clock", OPTION, MIDI_PORT_SUBLIST, 0, 1, &Setting.Read_MIDI_clock_port }, // Switch 1
      { "Send MIDI clock", OPTION, MIDI_PORT_SUBLIST, 0, 1, &Setting.Send_MIDI_clock_port }, // Switch 2
      { "Block device detection messages (careful)", OPTION, MIDI_PORT_SUBLIST, 0, 1, &Setting.Block_identity_messages }, // Switch 3
      { "Bass mode Guitar-to-MIDI channel", VALUE, 0, 1, 16, &Setting.Bass_mode_G2M_channel }, // Switch 5
      { "Bass mode device", OPTION, DEVICE_SUBLIST, 0, NUMBER_OF_DEVICES - 1, &Setting.Bass_mode_device }, // Switch 6
      { "Bass mode CC number", VALUE, 0, 0, 127, &Setting.Bass_mode_cc_number }, // Switch 7
      { "Bass mode min velocity", VALUE, 0, 0, 127, &Setting.Bass_mode_min_velocity}, // Switch 8
      { "HighNotePriotyCC", VALUE, 0, 0, 127, &Setting.HNP_mode_cc_number }, // Switch 9
      { "Follow tempo from Guitar2MIDI", OPTION, 79, 0, 2, &Setting.Follow_tempo_from_G2M}, // Switch 10

      { "MIDI Forwarding Settings", HEADER }, // Menu title
      { "Rule 1: Source port", OPTION, MIDI_PORT_SUBLIST, 0, 0, &Setting.MIDI_forward_source_port[0] }, // Switch 1
      { "Rule 1: Dest port", OPTION, MIDI_PORT_SUBLIST, 0, 1, &Setting.MIDI_forward_dest_port[0] }, // Switch 2
      { "Rule 1: MIDI filter", OPTION, 67, 0, NUMBER_OF_MIDI_FORWARD_FILTERS - 1, &Setting.MIDI_forward_filter[0] }, // Switch 3
      { "Rule 2: Source port", OPTION, MIDI_PORT_SUBLIST, 0, 0, &Setting.MIDI_forward_source_port[1] }, // Switch 4
      { "Rule 2: Dest port", OPTION, MIDI_PORT_SUBLIST, 0, 1, &Setting.MIDI_forward_dest_port[1] }, // Switch 5
      { "Rule 2: MIDI filter", OPTION, 67, 0, NUMBER_OF_MIDI_FORWARD_FILTERS - 1, &Setting.MIDI_forward_filter[1] }, // Switch 6
      { "Rule 3: Source port", OPTION, MIDI_PORT_SUBLIST, 0, 0, &Setting.MIDI_forward_source_port[2] }, // Switch 7
      { "Rule 3: Dest port", OPTION, MIDI_PORT_SUBLIST, 0, 1, &Setting.MIDI_forward_dest_port[2] }, // Switch 8
      { "Rule 3: MIDI filter", OPTION, 67, 0, NUMBER_OF_MIDI_FORWARD_FILTERS - 1, &Setting.MIDI_forward_filter[2] }, // Switch 9
      { "Forward data bi-directional", OPTION, 75, 0, 7, &Setting.MIDI_forward_bidirectional }, // Switch 10


      { "WIRELESS MENU (VC-touch)", HEADER }, // Menu title
      { "Bluetooth", OPTION, 1, 0, 1, &Setting.BLE_mode }, // Switch 1
      { "WIFI mode", OPTION, 75, 0, 2, &Setting.WIFI_mode }, // Switch 2
      { "RTPMIDI enabled", OPTION, 1, 0, 1, &Setting.RTP_enabled }, // Switch 7
      { "WIFI server", OPTION, 1, 0, 1, &Setting.WIFI_server_enabled }, // Switch 8
    };

    const uint16_t NUMBER_OF_SETTINGS_MENU_ITEMS = VCsettingMenu.size();

    QStringList menu_sublist = {
           // Sublist 1 - 3: Booleans
          "OFF", "ON", "DETECT",

          // Sublist 4 - 18: LED colours
          "OFF", "GREEN", "RED", "BLUE", "ORANGE", "CYAN", "WHITE", "YELLOW", "PURPLE", "PINK", "SOFT GREEN", "LIGHT BLUE", "", "", "",

          // Sublist 19 - 22: Main display modes
          "PAGE NAME", "PATCH NAME", "PATCHES COMBINED", "VCMINI LABELS", "SCENE NAME", "ALL PATCH NUMBRS", "",

          // Sublist 26 - 35: MIDI ports
          "OFF", MIDI_PORT_NAMES,

           // Sublist 36 - 39: Expression pedals
          "EXP PEDAL #1", "EXP PEDAL #2", "EXP PEDAL #3", "EXP PEDAL #4",

          // Sublist 40 - 42: MEP control options
          "NONE", "UP/DOWN", "UP/DN + STEP",

          // Sublist 43 - 50: MIDI switch types
          "OFF", "CC MOMENTARY", "CC SINGLE SHOT", "CC RANGE", "PC", "", "", "",

          // Sublist 51 - 52: RGB Display colour schemes
          "ADAFRUIT", "BUYDISPLAY",

          // Sublist 53 - 59: Main display top right types
          "OFF", "CURRENT DEVICE", "CURRENT TEMPO", "SCENE NAME", "PATCH NUMBER", "SCENE NUMBER", "",

          // Sublist 60 - 66: Current number actions
          "OFF", "PREVIOUS PATCH", "TAP TEMPO", "TUNER", "US20 EMULATION", "DIRECT SELECT", "",

          // Sublist 67 - 74: MIDI forward filters
          "BLOCK ALL MIDI", "FORWARD ALL MIDI", "ALL BUT SYSEX", "FWD PC ONLY", "FORWARD CC ONLY", "FWD NOTES ONLY", "FWD SYSEX ONLY", "",

          // Sublist 75 - 78: WIFI modes
          "OFF", "Client", "Access point", "Client + AP",

          // Sublist 79 - 81: Tempo follow modes
          "DISABLED", "OFF", "ON",

          // Sublist 82 - 89: MIDI forwarding bidirectional settings
          "NONE", "Only rule 1", "Only rule 2", "Rule 1 and 2", "Only rule 3", "Rule 1 and 3", "Rule 2 and 3", "ALL RULES",

          // Sublist 90 - 91: Katana type
          "Katana100 (8 CH)", "Katana50 (4 CH)",
    };
};

#endif // VCSETTINGS_H
