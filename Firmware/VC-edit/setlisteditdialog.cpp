#include "setlisteditdialog.h"
#include "ui_setlisteditdialog.h"
#include "VController/globals.h"
#include "VController/config.h"
#include "vcdevices.h"
#include "songeditdialog.h"
#include "VController/globals.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QMenu>

setlistEditDialog::setlistEditDialog(QWidget *parent, int _index, VCcommands *my_cmd) :
    QDialog(parent),
    ui(new Ui::setlistEditDialog)
{
    ui->setupUi(this);
    my_index = _index;
    MyVCcommands = my_cmd;
    uint8_t my_type = Device_patches[my_index][0];
    setlist_number = (Device_patches[my_index][1] << 7) + Device_patches[my_index][2];
    number_of_items = Device_patches[my_index][SETLIST_NUMBER_OF_ITEMS_INDEX];

    for (uint8_t i = 0; i < VC_PATCH_SIZE; i++) Backup_setlist_buffer[i] = Device_patches[my_index][i];

    if (my_type == EXT_SETLIST_TYPE) {
      build_page();
      connect(ui->listWidget, SIGNAL(moveCommand(customListWidget*,int,int)), this, SLOT(moveItem(customListWidget*,int,int)));
      ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
      connect(ui->listWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ShowListWidgetContextMenu(QPoint)));

    }
    supress_target_warning = false;
}

setlistEditDialog::~setlistEditDialog()
{
    delete ui;
}

QString setlistEditDialog::get_setlist_number_name(uint8_t number)
{
    QString numberString = "SETLIST ";
    numberString.append(QString::number(number / 10));
    numberString.append(QString::number(number % 10));
    return numberString;
}

QString setlistEditDialog::get_setlist_name(uint8_t number)
{
    QString name = "";
    for (uint8_t c = 0; c < LCD_DISPLAY_SIZE; c++) {
        name.append((char)Device_patches[number][c + SETLIST_NAME_INDEX]);
    }
    return name;
}

void setlistEditDialog::set_setlist_name(uint8_t number, QString name)
{
    uint8_t len = name.length();
    if (len > LCD_DISPLAY_SIZE) len = LCD_DISPLAY_SIZE;
    for (uint8_t c = 0; c < len; c++) Device_patches[number][c + SETLIST_NAME_INDEX] = name.at(c).toLatin1();
    for (uint8_t c = len; c < LCD_DISPLAY_SIZE; c++) Device_patches[number][c + SETLIST_NAME_INDEX] = 0x20;
}

void setlistEditDialog::readSetlistData(int patch_no, const QJsonObject &json, int my_type, int my_index)
{
    if (my_type == EXT_SETLIST_TYPE) {
        Device_patches[my_index][0] = EXT_SETLIST_TYPE;
        Device_patches[my_index][1] = patch_no >> 8;
        Device_patches[my_index][2] = patch_no & 0xFF;

        QJsonObject headerBlock = json["PatchHeader"].toObject();
        if (!headerBlock.isEmpty()) {
            for (int i = 3; i < 24; i++) {
                int value = headerBlock["Header data"+ QString::number(i)].toInt();
                Device_patches[my_index][i] = value;
            }
        }

        int b = 24;
        for (uint8_t s = 0; s < 50; s++ ) {
            QJsonObject setlistBlock = json["Item " + QString::number(s + 1)].toObject();
            if (!setlistBlock.isEmpty()) {
                for (int i = 0; i < 3; i++) {
                    int value = setlistBlock["Data"+ QString::number(i)].toInt();
                    Device_patches[my_index][b++] = value;
                }
            }
        }
    }
}

void setlistEditDialog::writeSetlistData(int patch_no, QJsonObject &json)
{
    QJsonObject headerBlock;
    for (int i = 0; i < 24; i++) {
        int value = Device_patches[patch_no][i];
        headerBlock["Header data"+ QString::number(i)] = value;
    }
    json["PatchHeader"] = headerBlock;

    QJsonObject setlistBlock;
    int b = 24;
    for (uint8_t s = 0; s < 50; s++ ) {
        setlistBlock["Name"] = "Item " + QString::number(s);
        for (int i = 0; i < 3; i++) {
            int value = Device_patches[patch_no][b++];
            setlistBlock["Data"+ QString::number(i)] = value;
        }
        json["Item " + QString::number(s + 1)] = setlistBlock;
    }
}

void setlistEditDialog::on_buttonBox_accepted()
{
    set_setlist_name(my_index, ui->setlistName->text());
    Device_patches[my_index][SETLIST_TARGET_INDEX] = my_target;
    Device_patches[my_index][SETLIST_NUMBER_OF_ITEMS_INDEX] = number_of_items;
}

void setlistEditDialog::on_buttonBox_rejected()
{
    for (uint8_t i = 0; i < VC_PATCH_SIZE; i++) Device_patches[my_index][i] = Backup_setlist_buffer[i];
}

void setlistEditDialog::ShowListWidgetContextMenu(const QPoint &pos)
{
    QComboBox *widget = (QComboBox *)sender();
    QPoint myPos = widget->mapToGlobal(pos);

    QMenu submenu;
    submenu.addAction(ui->actionDelete);
    submenu.exec(myPos);
}

void setlistEditDialog::fill_target_combobox(QComboBox *cbox)
{
    cbox->clear();
    cbox->addItem("SONG");
    cbox->addItem("PAGE");
    for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) cbox->addItem(Device[d]->full_device_name);
}

void setlistEditDialog::fill_item_combobox(QComboBox *cbox)
{
    cbox->clear();
    for (int i = 0; i < get_setlist_number_of_items(); i++) {
        cbox->addItem(get_setlist_full_item_name(i));
    }
}

void setlistEditDialog::fill_tempo_combobox(QComboBox *cbox)
{
    cbox->clear();
    cbox->addItem("GLOBAL TEMPO");
    for (uint8_t t = 40; t <= 250; t++) cbox->addItem(QString::number(t) + " BPM");
}

int setlistEditDialog::read_setlist_item(int item_no)
{
    int index = SETLIST_ITEM_BASE_INDEX + (item_no * 3);
    return (Device_patches[my_index][index] << 8) + Device_patches[my_index][index + 1];
}

int setlistEditDialog::read_setlist_tempo(int item_no)
{
    uint8_t index = SETLIST_ITEM_BASE_INDEX + (item_no * 3) + 2;
    return Device_patches[my_index][index];
}

void setlistEditDialog::set_setlist_item(int item_no, int item)
{
    int index = SETLIST_ITEM_BASE_INDEX + (item_no * 3);
    Device_patches[my_index][index] = item >> 8;
    Device_patches[my_index][index + 1] = item & 0xFF;
}

void setlistEditDialog::set_setlist_tempo(int item_no, int tempo)
{
    uint8_t index = SETLIST_ITEM_BASE_INDEX + (item_no * 3) + 2;
    Device_patches[my_index][index] = tempo;
}

QString setlistEditDialog::get_setlist_full_item_name(uint16_t item)
{
  if (my_target == SETLIST_TARGET_SONG) return songEditDialog::get_song_number_name(item) + ": " + songEditDialog::get_song_name(findIndex(EXT_SONG_TYPE, item));
  if (my_target == SETLIST_TARGET_PAGE) return "PAGE " + QString::number(item) + ": " + MyVCcommands->getPageName(item);
  if ((my_target >= SETLIST_TARGET_FIRST_DEVICE) && (my_target < (NUMBER_OF_DEVICES + SETLIST_TARGET_FIRST_DEVICE)))
      return (Device[my_target - SETLIST_TARGET_FIRST_DEVICE]->setlist_song_full_item_format(item));
  return ("-");
}

int setlistEditDialog::get_setlist_number_of_items()
{
    if (my_target == SETLIST_TARGET_SONG) return MAX_NUMBER_OF_SONGS;
    if (my_target == SETLIST_TARGET_PAGE) return Number_of_pages;
    if ((my_target >= SETLIST_TARGET_FIRST_DEVICE) && (my_target < (NUMBER_OF_DEVICES + SETLIST_TARGET_FIRST_DEVICE)))
        return Device[my_target - SETLIST_TARGET_FIRST_DEVICE]->setlist_song_get_number_of_items();
    return 0;
}

void setlistEditDialog::fill_setlist_listwidget(customListWidget *widget)
{
    widget->clear();
    for (int i = 0; i < number_of_items; i++) {
        int item = read_setlist_item(i);
        int tempo = read_setlist_tempo(i);
        QString tempoString = "";
        if (tempo >= 40) tempoString = " (tempo: " + QString::number(tempo) + " BPM)";
        widget->addItem(get_setlist_full_item_name(item) + tempoString);
    }
}

void setlistEditDialog::moveItem(customListWidget *widget, int sourceRow, int destRow)
{
    if (destRow > sourceRow) {
        for (int i = sourceRow; i < destRow; i++) {
            swapItems(i, i + 1);
        }
    }
    if (destRow < sourceRow) {
        for (int i = sourceRow; i --> destRow;) {
            swapItems(i, i + 1);
        }
    }
    fill_setlist_listwidget(widget);
}

void setlistEditDialog::swapItems(int item1, int item2)
{

    uint16_t temp_item = read_setlist_item(item1);
    uint8_t temp_tempo = read_setlist_tempo(item1);

    set_setlist_item(item1, read_setlist_item(item2));
    set_setlist_tempo(item1, read_setlist_tempo(item2));

    set_setlist_item(item2, temp_item);
    set_setlist_tempo(item2, temp_tempo);

    qDebug() << "swapping" << item1 << item2;
}

int setlistEditDialog::findIndex(int type, int patch_no)
{
    if (type == 0) return patch_no;
    for (int i = 0; i < MAX_NUMBER_OF_DEVICE_PRESETS; i++) {
        int n = (Device_patches[i][1] << 8) + Device_patches[i][2];
        if ((Device_patches[i][0] == (type & 0xFF)) && (n == patch_no)) return i;
    }
    return PATCH_INDEX_NOT_FOUND;
}

void setlistEditDialog::createNewSetlist(int patch_no)
{
    int index = newIndex();
    Device_patches[index][0] = EXT_SETLIST_TYPE;
    Device_patches[index][1] = (patch_no) >> 8;
    Device_patches[index][2] = (patch_no) & 0xFF;
    Device_patches[index][SETLIST_TARGET_INDEX] = 0;
    Device_patches[index][SETLIST_NUMBER_OF_ITEMS_INDEX] = 0;
    set_setlist_name(index, "New Setlist");
    qDebug() << "New setlist index " << index;
}

int setlistEditDialog::newIndex()
{
    for (int i = 0; i < MAX_NUMBER_OF_DEVICE_PRESETS; i++) {
        if (Device_patches[i][0] == 0) return i;
    }
    return PATCH_INDEX_NOT_FOUND;
}


void setlistEditDialog::on_targetComboBox_currentIndexChanged(int index)
{
    if (supress_target_warning) return;
    if (number_of_items > 0) {
        QMessageBox(QMessageBox::Warning, "Setlist target", "Setlist already contains items. Please make a new setlist for this device",
                                                QMessageBox::Ok).exec();
        supress_target_warning = true;
        ui->targetComboBox->setCurrentIndex(my_target);
        supress_target_warning = false;
    }
    else {
        my_target = index;
        fill_item_combobox(ui->newItemComboBox);
    }
}


void setlistEditDialog::on_addPushButton_clicked()
{
    if (number_of_items < 50) {
        set_setlist_item(number_of_items, ui->newItemComboBox->currentIndex());
        set_setlist_tempo(number_of_items, ui->newTempoComboBox->currentIndex() + 39);
        number_of_items++;
        fill_setlist_listwidget(ui->listWidget);
        ui->numberOfItemsLabel->setText(QString::number(number_of_items) + " items");
        ui->listWidget->setCurrentRow(number_of_items - 1);
    }
}


void setlistEditDialog::on_insertPushButton_clicked()
{
    int selectedRow = ui->listWidget->currentRow();
    if (selectedRow == -1) return;
    if (number_of_items < 50) {
        for (int i = (number_of_items); i --> selectedRow;) {
            swapItems(i, i + 1);
        }
        set_setlist_item(selectedRow, ui->newItemComboBox->currentIndex());
        set_setlist_tempo(selectedRow, ui->newTempoComboBox->currentIndex() + 39);
        number_of_items++;
        fill_setlist_listwidget(ui->listWidget);
        ui->numberOfItemsLabel->setText(QString::number(number_of_items) + " items");
        ui->listWidget->setCurrentRow(selectedRow);
    }
}


void setlistEditDialog::on_actionDelete_triggered()
{
    int selectedRow = ui->listWidget->currentRow();
    if (selectedRow == -1) return;
    if (number_of_items > 0) {
        for (int i = selectedRow; i < (number_of_items - 1); i++) {
            swapItems(i, i + 1);
        }
        number_of_items--;
        set_setlist_item(number_of_items, 0);
        set_setlist_tempo(number_of_items, 0);
        fill_setlist_listwidget(ui->listWidget);
        ui->numberOfItemsLabel->setText(QString::number(number_of_items) + " items");
        ui->listWidget->setCurrentRow(selectedRow);
    }
}

void setlistEditDialog::build_page()
{
    ui->setlistNumberLabel->setText(get_setlist_number_name(setlist_number));
    ui->setlistName->setText(get_setlist_name(my_index).trimmed());
    ui->numberOfItemsLabel->setText(QString::number(number_of_items) + " items");

    my_target = Device_patches[my_index][SETLIST_TARGET_INDEX];
    fill_target_combobox(ui->targetComboBox);
    ui->targetComboBox->setCurrentIndex(my_target);
    fill_setlist_listwidget(ui->listWidget);
    fill_item_combobox(ui->newItemComboBox);
    fill_tempo_combobox(ui->newTempoComboBox);
}


void setlistEditDialog::on_deletePushButton_clicked()
{
    on_actionDelete_triggered();
}

