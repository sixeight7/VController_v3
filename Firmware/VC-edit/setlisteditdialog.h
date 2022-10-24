#ifndef SETLISTEDITDIALOG_H
#define SETLISTEDITDIALOG_H

#include "customlistwidget.h"
#include "vccommands.h"

#include <QDialog>
#include <QComboBox>

// Setlist buffer structure:
// byte 0 - 2:  Type and number
// byte 3: T    Target
// byte 4 - 19  Setlist name
// byte 20      Number of items in list
// byte 24 - 25 Item 1 setlist 1: (patch msb, patch lsb, tempo)
// --
// byte 167 - 170 Item 50 setlist 99: (patch msb, patch lsb, tempo)

#define SETLIST_TARGET_INDEX 3
#define SETLIST_NAME_INDEX 4
#define SETLIST_NUMBER_OF_ITEMS_INDEX 20
#define SETLIST_ITEM_BASE_INDEX 24

namespace Ui {
class setlistEditDialog;
}

class setlistEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit setlistEditDialog(QWidget *parent = nullptr, int patch = 0, VCcommands *my_cmd = NULL);
    ~setlistEditDialog();

    static QString get_setlist_number_name(uint8_t number);
    static QString get_setlist_name(uint8_t number);
    static void set_setlist_name(uint8_t number, QString name);
    static void readSetlistData(int patch_no, const QJsonObject &json, int my_type, int my_index);
    static void writeSetlistData(int patch_no, QJsonObject &json);
    static int findIndex(int type, int patch_no);
    static void createNewSetlist(int patch_no);

private slots:
    void on_buttonBox_accepted();
    void on_targetComboBox_currentIndexChanged(int index);
    void moveItem(customListWidget *widget, int sourceRow, int destRow);

    void on_addPushButton_clicked();

    void on_insertPushButton_clicked();

    void on_actionDelete_triggered();

    void on_buttonBox_rejected();

    void ShowListWidgetContextMenu(const QPoint &pos); // For right click on the listWidgets

    void on_deletePushButton_clicked();

private:
    Ui::setlistEditDialog *ui;
    void build_page();
    void fill_target_combobox(QComboBox *cbox);
    void fill_item_combobox(QComboBox *cbox);
    void fill_tempo_combobox(QComboBox *cbox);

    int read_setlist_item(int item_no);
    int read_setlist_tempo(int item_no);
    void set_setlist_item(int item_no, int item);
    void set_setlist_tempo(int item_no, int tempo);
    QString get_setlist_full_item_name(uint16_t item);
    int get_setlist_number_of_items();
    void fill_setlist_listwidget(customListWidget *widget);
    void swapItems(int item1, int item2);

    static int newIndex();

    uint8_t Backup_setlist_buffer[VC_PATCH_SIZE];
    VCcommands *MyVCcommands;
    int my_index;
    int setlist_number;
    int my_target = 0;
    int number_of_items = 0;
    bool supress_target_warning = true;
};

#endif // SETLISTEDITDIALOG_H
