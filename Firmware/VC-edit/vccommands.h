#ifndef VCCOMMANDS_H
#define VCCOMMANDS_H

// VCcommands is the interface with the Commands[] and Fixed_commands[] arrays.
// The structure is the same as on the VController, including the indexes for both programmable and fixed commands.
// The data at the bottom of this page is the same as on the VController

#include "VController/hardware.h"
#include "VController/config.h"
#include "customlistwidget.h"

#include <QObject>
#include <QComboBox>
#include <QListWidget>
#include <QTableWidget>

#define MAX_NUMBER_OF_PAGES 256
#define EXT_EEP_MAX_NUMBER_OF_COMMANDS 2997
#define MAX_NUMBER_OF_INTERNAL_COMMANDS 1000
#define INTERNAL_CMD 0x8000 // MSB of an index set indicates the command is an internal command

class VCcommands : public QObject
{
    Q_OBJECT

public:
    explicit VCcommands(QObject *parent = nullptr);
    void recreate_indexes();
    // fill page widgets:
    void fillPageComboBox(QComboBox *cbox);
    void fillFixedPageComboBox(QComboBox *cbox);
    void fillSwitchComboBox(QComboBox *cbox);
    void fillCommandsListWidget(QObject *parent, customListWidget *cmdList, uint8_t pg, uint8_t sw, bool show_default_page, bool first_time);
    void fillCommandTableWidget(QTableWidget *table, uint8_t pg, uint8_t sw, uint8_t cmd);
    void updateCommandsTableWidget();
    void setCommandTabWidget(QTabWidget *tabWidget, uint8_t pg, uint8_t sw, uint8_t cmd);
    void saveCommand(uint8_t pg, uint8_t sw, uint8_t cmd);
    void checkSaved(uint8_t pg, uint8_t sw, uint8_t cmd);
    int createNewCommand(uint8_t pg, uint8_t sw);
    int deleteCommand(uint8_t pg, uint8_t sw, uint8_t item);
    QString customLabelString(uint8_t pg, uint8_t sw);
    bool setCustomLabelString(uint8_t pg, uint8_t sw, QString label);
    int valueFromIndex(uint8_t type, uint16_t index);
    int indexFromValue(uint8_t type, uint8_t pageNumber);
    void swapSwitches(int pg1, int sw1, int pg2, int sw2);
    uint8_t duplicatePage(int pg);
    void clearPage(int pg);
    int addPage();
    void readAll(const QJsonObject &json);
    void readPage(int pg, const QJsonObject &json);
    void readCommand(QJsonObject &json, int pgOverride);
    void writeAll(QJsonObject &json) const;
    void writePage(int pg, QJsonObject &json);
    void writeCommand(uint16_t cmd_no, QJsonObject &json) const;
    QString getPageName(int pg);
    bool switchHasLabel(int pg, int sw);
    bool switchShowsDefaultItems(int pg, int sw);
    void copyItemsToBuffer(customListWidget *widget);
    void pasteItem(int sw);

signals:
    void updateCommandScreens(bool);
    void updateEditCommandScreen();
    void customContextMenuRequested(QPoint);

public slots:

    void copyCommand(customListWidget *sourceWidget, int sourceRow, customListWidget *destWidget);
    void moveCommand(customListWidget *widget, int sourceRow, int destRow);
    void cmdByteDataChanged(int, int, int);

private:
    // Low level interfacing with the command array.
    void create_indexes();
    uint16_t new_command_index();
    void delete_cmd(uint16_t number);
    Cmd_struct get_cmd(uint16_t number);
    void write_cmd(uint16_t number, Cmd_struct &cmd);
    uint16_t get_cmd_number(uint8_t pg, uint8_t sw, uint8_t number);
    uint16_t get_cmd_number_check_default(uint8_t pg, uint8_t sw, uint8_t number);
    uint16_t count_cmds(uint8_t pg, uint8_t sw);
    QString create_cmd_string(uint16_t number);
    QString read_title(uint8_t pg, uint8_t sw);
    void write_title(uint8_t pg, uint8_t sw, QString title);

    // Low level procedures for command creation
    QString cmdTypeString(uint8_t type);
    QString cmdDeviceString(uint8_t dev);
    void set_type_and_value(uint8_t number, uint8_t type, uint8_t value, bool in_edit_mode);
    void clear_cmd_bytes(uint8_t start_byte, bool in_edit_mode);
    void build_command_structure(uint8_t cmd_byte_no, uint8_t cmd_type, bool in_edit_mode);
    void load_cmd_byte(QTableWidget *table, uint8_t cmd_byte_no);
    QString read_cmd_sublist(uint8_t cmd_byte_no, uint8_t value);

    QTableWidget *MyTable = nullptr;
    int current_page = 0;
    int current_switch = 0;
    int current_item = 0;
    int pageDuplicateCounter = 2;
    QString addNewNumber(QString inputString);

    QVector<Cmd_struct> copyBuffer;
    QString copyBufferLabel = "";
    bool copyBufferContainsLabel = false;
    bool copyBufferFilled = false;

    customListWidget *lastWidget = 0;

    uint16_t First_cmd_index[MAX_NUMBER_OF_PAGES][NUMBER_OF_SWITCHES + NUMBER_OF_EXTERNAL_SWITCHES + 1]; // Gives the index number of every first command for every dwitch on every page
    uint16_t Next_cmd_index[EXT_EEP_MAX_NUMBER_OF_COMMANDS]; // Gives the number of the next command that needs to be read from EEPROM
    uint16_t Next_internal_cmd_index[MAX_NUMBER_OF_INTERNAL_COMMANDS]; // Gives the index of the next command that needs to be read from Fixed_commands[]
    uint16_t Title_index[MAX_NUMBER_OF_PAGES][NUMBER_OF_SWITCHES + NUMBER_OF_EXTERNAL_SWITCHES + 1]; // gives the index number of the title command for page 0 (page title) and every switch with a display
    bool isIndexed = false;

    uint8_t selected_device_cmd = PATCH_SEL - 100;
    uint8_t selected_common_cmd = OPEN_PAGE;
    bool command_edited = false;
    bool commandListBoxContainsLabel = false;

    struct cmdtype_struct {
      QString Title;
      uint8_t Sublist;
      uint8_t Min;
      uint8_t Max;
    };

    #define TYPE_OFF 0
    #define TYPE_DEVICE_SELECT 1
    #define TYPE_COMMON_COMMANDS 2
    #define TYPE_DEVICE_COMMANDS 3
    #define TYPE_PAGE 4
    #define TYPE_MIDI_PORT 5
    #define TYPE_MIDI_CHANNEL 6
    #define TYPE_CC_NUMBER 7
    #define TYPE_CC_TOGGLE 8
    #define TYPE_VALUE 9
    #define TYPE_NOTE_NUMBER 10
    #define TYPE_NOTE_VELOCITY 11
    #define TYPE_PC 12
    #define TYPE_REL_NUMBER 13
    #define TYPE_BANK_SIZE 14
    #define TYPE_PATCH_NUMBER 15
    #define TYPE_PATCH_100 16
    #define TYPE_PARAMETER 17
    #define TYPE_PAR_STATE 18
    #define TYPE_ASSIGN 19
    #define TYPE_ASSIGN_TRIGGER 20
    #define TYPE_TOGGLE 21
    #define TYPE_STEP 22
    #define TYPE_SWITCH 23
    #define TYPE_CMD 24
    #define TYPE_BPM 25
    #define TYPE_MIN 26
    #define TYPE_MAX 27
    #define TYPE_SNAPSCENE 28
    #define TYPE_LOOPER 29
    #define TYPE_EXP_PEDAL 30

    // Some of the data for the sublists below is not fixed, but must be read from a Device class or from EEPROM
    // Here we define these sublists
    #define SUBLIST_PATCH 255 // To show the patchnumber
    #define SUBLIST_PARAMETER 254 // To show the parameter name
    #define SUBLIST_PAR_STATE 253 // To show the parameter state
    #define SUBLIST_ASSIGN 252 // To show the assign name
    #define SUBLIST_TRIGGER 251 // To show the assign trigger
    #define SUBLIST_PAGE 250 // To show the page name
    #define SUBLIST_CMD 249 // To show the selected command
    #define SUBLIST_DEVICES 248 // To show the devices + Current and Common
    #define SUBLIST_PATCH_BANK 247

    const QVector<cmdtype_struct> cmdtype = {
      { "", 0, 0, 0 }, // TYPE_OFF 0
      { "DEVICE", SUBLIST_DEVICES, 0, (NUMBER_OF_DEVICES + 1) }, // TYPE_DEVICE_SELECT 1
      { "COMMAND", 1, 0, NUMBER_OF_COMMON_TYPES - 1 }, // TYPE_COMMON_COMMANDS 2
      { "COMMAND", 18, 0, NUMBER_OF_DEVICE_TYPES - 1 }, // TYPE_DEVICE_COMMANDS 3
      { "PAGE", SUBLIST_PAGE, 0, LAST_FIXED_CMD_PAGE }, // TYPE_PAGE 4
      { "MIDI PORT", 40, 0, NUMBER_OF_MIDI_PORTS }, // TYPE_MIDI_PORT 5
      { "MIDI CHANNEL", 0, 1, 16 }, // TYPE_MIDI_CHANNEL 6
      { "CC NUMBER", 0, 0, 127 }, // TYPE_CC_NUMBER 7
      { "CC TOGGLE TYPE", 83, 0, 6 }, // TYPE_CC_TOGGLE 8
      { "VALUE", 0, 0, 127 }, // TYPE_VALUE 9
      { "NOTE NUMBER", 0, 0, 127 }, // NOTE_NUMBER 10
      { "NOTE VELOCITY", 0, 0, 127 }, // TYPE_NOTE_VELOCITY 11
      { "PROGRAM", 0, 0, 127 }, // TYPE_PC 12
      { "NUMBER", 0, 1, 16 }, // TYPE_REL_NUMBER 13
      { "BANK SIZE", 0, 1, 16 }, // TYPE_BANK_SIZE 14
      { "PATCH NUMBER", SUBLIST_PATCH, 0, 99 }, // TYPE_PATCH_NUMBER 15
      { "PATCH BANK (100)", SUBLIST_PATCH_BANK, 0, 255 }, // TYPE_PATCH_100 16
      { "PARAMETER", SUBLIST_PARAMETER, 0, 255 }, // TYPE_PARAMETER 17
      { "VALUE", SUBLIST_PAR_STATE, 0, 255 }, // TYPE_PAR_STATE 18
      { "ASSIGN", SUBLIST_ASSIGN, 0, 255 }, // TYPE_ASSIGN 19
      { "TRIGGER", SUBLIST_TRIGGER, 0, 127 }, // TYPE_ASSIGN 20
      { "TOGGLE TYPE", 48, 0, 6 }, // TYPE_TOGGLE 21
      { "STEP", 0, 1, 127 }, // TYPE_STEP 22
      { "SWITCH", 58, 0, 24 }, // TYPE_SWITCH 23
      { "COMMAND", SUBLIST_CMD, 0, 255 }, // TYPE_CMD 24
      { "BPM", 0, 40, 250 }, // TYPE_BPM 25
      { "MIN", 0, 0, 127 }, // TYPE_MIN 26
      { "MAX", 0, 0, 127 }, // TYPE_MAX 27
      { "SNAPSHOT/SCENE", 0, 1, 8 }, // TYPE_SNAPSCENE 28
      { "LOOPER", 90, 0, 7 }, // TYPE_LOOPER 29
      { "EXP.PEDAL", 98, 0, 3 }, // TYPE_EXP_PEDAL
    };

    const QStringList cmd_sublist = {

        // Sublist 1 - 17: Common Types
        "NO COMMAND", "SELECT PAGE", "PAGE UP", "PAGE DOWN", "TAP TEMPO", "SET TEMPO", "GLOBAL TUNER", "MIDI PC", "MIDI CC", "MIDI NOTE", "NEXT DEVICE", "MENU", "", "", "", "", "",

        // Sublist 18 - 39: Device Types
        "PATCH SELECT", "PARAMETER", "ASSIGN", "PATCHBANK SELECT", "BANK UP", "BANK DOWN", "NEXT PATCH", "PREV PATCH",
        "MUTE", "SEL DEVICE PAGE", "SEL NEXT PAGE", "TOGGL MASTER EXP", "SNAPSHOT/SCENE", "LOOPER", "MASTER EXP PEDAL", "DIR.SELECT", "PAR BANK", "PAR BANK UP", "PAR BANK DOWN", "PARBANK_CATEGORY", "SAVE PATCH", "",

        // Sublist 40 - 47: MIDI ports
        "USB MIDI", "MIDI 1", "MIDI2/RRC", "MIDI 3", "ALL PORTS", "", "", "",

        // Sublist 48 - 57: Toggle types
        "MOMENTARY", "TOGGLE", "TRISTATE", "FOURSTATE", "STEP",  "RANGE",  "UPDOWN", "", "", "",

        // Sublist 58 - 82: Switch types
        "On Page Select", "Switch 1", "Switch 2", "Switch 3", "Switch 4", "Switch 5", "Switch 6", "Switch 7", "Switch 8",
        "Switch 9", "Switch 10", "Switch 11", "Switch 12", "Switch 13", "Switch 14", "Switch 15", "Switch 16",
        "Ext 1 / Exp1", "Ext 2", "Ext 3 / Exp 2", "Ext 4", "Ext 5 / Exp 3", "Ext 6", "Ext 7 / Exp 4", "Ext 8",

        // Sublist 83 - 89: CC toggle types
        "ONE SHOT", "MOMENTARY", "TOGGLE", "TOGGLE ON", "RANGE", "STEP", "UPDOWN",

        // Sublist 90 - 97: looper types
        "OFF", "ON/OFF", "PLAY/STOP", "REC/OVERDUB", "UNDO/REDO", "HALF SPEED", "REVERSE", "PLAY ONCE",

        // Sublist 98 - 101: EXP pedal types
        "TOGGLE", "EXP1", "EXP2", "EXP3",
    };

    struct cmdbyte_struct {
      uint8_t Type;
      QString Title;
      uint8_t Value;
      uint8_t Min;
      uint8_t Max;
    };

    #define NUMBER_OF_CMD_BYTES 8

    cmdbyte_struct cmdbyte[NUMBER_OF_CMD_BYTES] = {
      // Default command to edit
      { TYPE_DEVICE_SELECT, "DEVICE", (NUMBER_OF_DEVICES + 1), 0, (NUMBER_OF_DEVICES + 1) },
      { TYPE_COMMON_COMMANDS, "COMMAND", 0, 0, NUMBER_OF_COMMON_TYPES},
      { TYPE_OFF, "", 0, 0, 0},
      { TYPE_OFF, "", 0, 0, 0},
      { TYPE_OFF, "", 0, 0, 0},
      { TYPE_OFF, "", 0, 0, 0},
      { TYPE_OFF, "", 0, 0, 0},
      { TYPE_OFF, "", 0, 0, 0},
    };
};

#endif // VCCOMMANDS_H
