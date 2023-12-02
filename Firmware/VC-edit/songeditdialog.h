#ifndef SONGEDITDIALOG_H
#define SONGEDITDIALOG_H

#include "customcheckbox.h"
#include "customlistwidget.h"
#include "customlineedit.h"
#include "vccommands.h"

#include <QDialog>

// Song buffer structure:
// byte 0 - 2:  Type and number
// byte 3 - 18  Song name
// byte 19      Part active byte
// byte 20      Song tempo
// byte 21      Target device #1
// byte 22      Target device #2
// byte 23      Target device #3
// byte 24      Target device #4
// byte 25      Target device #5
// byte 26      Target #1 midi data
// byte 27      Target #2 midi data
// byte 28      Target #3 midi data
// byte 29      Target #4 midi data
// byte 30      Target #5 midi data
// byte 31      Spare
// byte 32 - 51 Part 1 (10 bytes name + 10 bytes for target data)
// --
// byte 172 - 191 Part 8

#define SONG_NAME_INDEX 3
#define SONG_NAME_SIZE 16
#define SONG_PART_ACTIVE_INDEX 19
#define SONG_TEMPO_INDEX 20
#define SONG_TARGET_DEVICE1 21
#define SONG_TARGET_DEVICE2 22
#define SONG_TARGET_DEVICE3 23
#define SONG_TARGET_DEVICE4 24
#define SONG_TARGET_DEVICE5 25
#define SONG_TARGET_MIDI_DATA1 26
#define SONG_TARGET_MIDI_DATA2 27
#define SONG_TARGET_MIDI_DATA3 28
#define SONG_TARGET_MIDI_DATA4 29
#define SONG_TARGET_MIDI_DATA5 30
#define SONG_PART_BASE_INDEX 32
#define SONG_PART_SIZE 20
#define SONG_PART_NAME_SIZE 10

#define NUMBER_OF_PARTS 8
#define NUMBER_OF_SONG_TARGETS 5

namespace Ui {
class songEditDialog;
}

class songEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit songEditDialog(QWidget *parent = nullptr, int patch = 0);
    ~songEditDialog();

    static QString get_song_number_name(uint8_t number);
    static QString get_song_name(uint8_t number);
    static void set_song_name(uint8_t number, QString name);
    static void readSongData(int patch_no, const QJsonObject &json, int my_type, int my_index);
    static void writeSongData(int patch_no, QJsonObject &json);
    static void createNewSong(int patch_no);

public slots:
    void tableDataChanged(int, int, int);
    void partEnabledStateChanged(int, int, bool state);
    void partNameChanged(QString new_name);

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_tempoComboBox_currentIndexChanged(int index);

private:
    Ui::songEditDialog *ui;
    void fillTempoComboBox();
    void fillTable();
    void fillParts(uint8_t index);
    void fill_target_combobox(QComboBox *cbox);

    int get_song_target(uint8_t target);
    void set_song_target(uint8_t target, uint8_t value);

    QString get_part_name(uint8_t part);
    void set_part_name(uint8_t part, QString name);

    uint16_t get_song_item(uint8_t part, uint8_t index);
    void set_song_item(uint8_t part, uint8_t index, uint16_t item);
    QString get_song_item_name(uint8_t index, uint16_t item);
    uint16_t get_song_item_max(uint8_t index);

    uint8_t Backup_song_buffer[VC_PATCH_SIZE];
    int my_index;
    int song_number;
    uint8_t my_tempo;
    uint8_t Current_song_midi_port[NUMBER_OF_SONG_TARGETS] = { 0 };
    uint8_t Current_song_midi_channel[NUMBER_OF_SONG_TARGETS] = { 1 };
    bool check_part_active(uint8_t part);
    void set_part_active_state(uint8_t part, bool state);
    CustomCheckBox *myCheckBoxes[NUMBER_OF_PARTS];
    //void set_part_visible(uint8_t part, bool state);

    static int newIndex();
};

#endif // SONGEDITDIALOG_H
