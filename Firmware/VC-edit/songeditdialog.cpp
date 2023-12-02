#include "songeditdialog.h"
#include "ui_songeditdialog.h"
#include "VController/globals.h"
#include "VController/config.h"
#include "vcdevices.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>

songEditDialog::songEditDialog(QWidget *parent, int _index) :
    QDialog(parent),
    ui(new Ui::songEditDialog)
{
    ui->setupUi(this);
    my_index = _index;
    song_number = (Device_patches[my_index][1] << 7) + Device_patches[my_index][2];
    my_tempo = Device_patches[my_index][SONG_TEMPO_INDEX];
    uint8_t my_type = Device_patches[my_index][0];
    for (uint8_t i = 0; i < NUMBER_OF_SONG_TARGETS; i++) {
        Current_song_midi_port[i] = (Device_patches[my_index][SONG_TARGET_MIDI_DATA1 + i] >> 4);
        Current_song_midi_channel[i] = (Device_patches[my_index][SONG_TARGET_MIDI_DATA1 + i] & 0x0F) + 1;
    }

    for (uint8_t i = 0; i < VC_PATCH_SIZE; i++) Backup_song_buffer[i] = Device_patches[my_index][i];

    if (my_type == EXT_SONG_TYPE) {

      // Load setlist name
      ui->songNumberLabel->setText(get_song_number_name(song_number));
      ui->songName->setText(get_song_name(my_index));
      //ui->lineEditScene1->setText(Device[dev]->read_scene_name_from_buffer(my_index, 0));
      fillTempoComboBox();
      fillTable();
    }
}

songEditDialog::~songEditDialog()
{
    delete ui;
}

QString songEditDialog::get_song_number_name(uint8_t number)
{
    QString numberString = "SNG";
    number++;
    numberString.append(QString::number(number / 10));
    numberString.append(QString::number(number % 10));
    return numberString;
}

QString songEditDialog::get_song_name(uint8_t number)
{
    if (Device_patches[number][0] != EXT_SONG_TYPE) return "New Song";
    QString name = "";
    for (uint8_t c = 0; c < LCD_DISPLAY_SIZE; c++) {
        name.append((char)Device_patches[number][c + SONG_NAME_INDEX]);
    }
    return name.trimmed();
}

void songEditDialog::set_song_name(uint8_t number, QString name)
{
    uint8_t len = name.length();
    if (len > LCD_DISPLAY_SIZE) len = LCD_DISPLAY_SIZE;
    for (uint8_t c = 0; c < len; c++) Device_patches[number][c + SONG_NAME_INDEX] = name.at(c).toLatin1();
    for (uint8_t c = len; c < LCD_DISPLAY_SIZE; c++) Device_patches[number][c + SONG_NAME_INDEX] = 0x20;
}

void songEditDialog::readSongData(int patch_no, const QJsonObject &json, int my_type, int my_index)
{
    if (my_type != EXT_SONG_TYPE) return;
    Device_patches[my_index][0] = EXT_SONG_TYPE;
    Device_patches[my_index][1] = patch_no >> 8;
    Device_patches[my_index][2] = patch_no & 0xFF;

    QJsonObject headerBlock = json["PatchHeader"].toObject();
    if (!headerBlock.isEmpty()) {
        for (int i = 3; i < 32; i++) {
            int value = headerBlock["Header data"+ QString::number(i)].toInt();
            Device_patches[my_index][i] = value;
        }
    }

    int b = 32;
    for (uint8_t s = 0; s < 8; s++ ) {
        QJsonObject partBlock = json["Part" + QString::number(s + 1)].toObject();
        if (!partBlock.isEmpty()) {
            for (int i = 0; i < 20; i++) {
                int value = partBlock["Data"+ QString::number(i)].toInt();
                Device_patches[my_index][b++] = value;
            }
        }
    }
}

void songEditDialog::writeSongData(int patch_no, QJsonObject &json)
{
    QJsonObject headerBlock;
    for (int i = 0; i < 32; i++) {
        int value = Device_patches[patch_no][i];
        headerBlock["Header data"+ QString::number(i)] = value;
    }
    json["PatchHeader"] = headerBlock;

    QJsonObject partBlock;
    int b = 32;
    for (uint8_t s = 0; s < 8; s++ ) {
        partBlock["Name"] = "Part" + QString::number(s + 1);
        for (int i = 0; i < 20; i++) {
            int value = Device_patches[patch_no][b++];
            partBlock["Data"+ QString::number(i)] = value;
        }
        json["Part" + QString::number(s + 1)] = partBlock;
    }
}

void songEditDialog::createNewSong(int patch_no)
{
    int index = newIndex();
    Device_patches[index][0] = EXT_SONG_TYPE;
    Device_patches[index][1] = (patch_no) >> 8;
    Device_patches[index][2] = (patch_no) & 0xFF;

    for (uint8_t i = 3; i < VC_PATCH_SIZE; i++) Device_patches[index][i] = 0;
    Device_patches[index][SONG_TEMPO_INDEX] = GLOBAL_TEMPO;
    set_song_name(index, "New Song");
    qDebug() << "New song index " << index;
}

void songEditDialog::on_buttonBox_accepted()
{
    set_song_name(my_index, ui->songName->text());
    for (uint8_t i = 0; i < NUMBER_OF_SONG_TARGETS; i++) {
        Device_patches[my_index][SONG_TARGET_MIDI_DATA1 + i] = (Current_song_midi_port[i] << 4) + (Current_song_midi_channel[i] - 1);
    }
    Device_patches[my_index][SONG_TEMPO_INDEX] = my_tempo;
}


void songEditDialog::on_buttonBox_rejected()
{
    for (uint8_t i = 0; i < VC_PATCH_SIZE; i++) Device_patches[my_index][i] = Backup_song_buffer[i];
}

void songEditDialog::fillTempoComboBox()
{
    ui->tempoComboBox->clear();
    ui->tempoComboBox->addItem("GLOBAL");
    for (uint8_t t = MIN_BPM; t <= MAX_BPM; t++) ui->tempoComboBox->addItem(QString::number(t) + " BPM");
    uint8_t tempo = Device_patches[my_index][SONG_TEMPO_INDEX];
    if ((tempo < GLOBAL_TEMPO) || (tempo > MAX_BPM)) tempo = GLOBAL_TEMPO;
    ui->tempoComboBox->setCurrentIndex(tempo - GLOBAL_TEMPO);
}

void songEditDialog::fillTable()
{
    ui->tableWidget->clear();
    ui->tableWidget->setColumnCount(7);
    ui->tableWidget->setHorizontalHeaderLabels({"Enabled", "Name", "Item #1", "Item #2", "Item #3", "Item #4", "Item #5"});
    ui->tableWidget->setColumnWidth(0, 60); // Width of checkbox column
    //ui->tableWidget->setColumnWidth(1, 120);
    //ui->tableWidget->setColumnWidth(2, 120);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->setRowCount(11);
    ui->tableWidget->setVerticalHeaderLabels({"", "", "", "Part 1", "Part 2", "Part 3", "Part 4", "Part 5", "Part 6", "Part 7", "Part 8"});
    //for (int r = 0; r < NUMBER_OF_CMD_BYTES; r++) my_widget->setRowHeight(r, 30);
    ui->tableWidget->setShowGrid(true);
    ui->tableWidget->setStyleSheet("QTableWidget::item { padding: 2px }");

    // Fill first row
    ui->tableWidget->setItem(0, 1, new QTableWidgetItem("Target"));
    ui->tableWidget->setItem(1, 1, new QTableWidgetItem("MIDI channel"));
    ui->tableWidget->setItem(2, 1, new QTableWidgetItem("MIDI port"));

    // Fill part names
    for (uint8_t part = 0; part < NUMBER_OF_PARTS; part++) {
        // Set enabled
        CustomCheckBox *checkBox = new CustomCheckBox(ui->tableWidget, 0, part);
        checkBox->setChecked(check_part_active(part));
        checkBox->setStyleSheet("margin-left:17%; margin-right:83%;");
        ui->tableWidget->setCellWidget(part + 3, 0, checkBox);
        myCheckBoxes[part] = checkBox;
        connect(checkBox, SIGNAL(custom_clicked(int, int, bool)), this, SLOT(partEnabledStateChanged(int, int, bool)));

        // Set name
        customLineEdit *lineEdit = new customLineEdit(ui->tableWidget, part + 3, 1);
        lineEdit->setText(get_part_name(part).trimmed());
        ui->tableWidget->setCellWidget(part + 3, 1, lineEdit);
        connect(lineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(partNameChanged(const QString &)));
    }

    // Fill song items
    for (uint8_t i = 0; i < NUMBER_OF_SONG_TARGETS; i++) {
        fillParts(i);
    }
    //ui->tableWidget->resizeColumnsToContents();
}

void songEditDialog::fillParts(uint8_t index)
{
    // Target
    CustomComboBox *comboBox = new CustomComboBox(ui->tableWidget, 0, index + 2);
    fill_target_combobox(comboBox);
    comboBox->setCurrentIndex(get_song_target(index));
    ui->tableWidget->setCellWidget(0, index + 2, comboBox);
    connect(comboBox, SIGNAL(new_value(int, int, int)), this, SLOT(tableDataChanged(int, int, int)));

    // MIDI channel
    if ((get_song_target(index) == SONG_TARGET_PC) || (get_song_target(index) == SONG_TARGET_CC)) {
        CustomComboBox *comboBox = new CustomComboBox(ui->tableWidget, 1, index + 2);
        for (uint8_t c = 0; c < 16; c++) comboBox->addItem(QString::number(c + 1));
        comboBox->setCurrentIndex(Current_song_midi_channel[index] - 1);
        ui->tableWidget->setCellWidget(1, index + 2, comboBox);
        connect(comboBox, SIGNAL(new_value(int, int, int)), this, SLOT(tableDataChanged(int, int, int)));
    }
    else {
        ui->tableWidget->setCellWidget(1, index + 2, NULL);
    }

    // MIDI port
    if ((get_song_target(index) == SONG_TARGET_PC) || (get_song_target(index) == SONG_TARGET_CC)) {
        CustomComboBox *comboBox = new CustomComboBox(ui->tableWidget, 2, index + 2);
        for (uint8_t c = 0; c < number_of_midi_ports; c++) comboBox->addItem(midi_port_names.at(c));
        comboBox->setCurrentIndex(Current_song_midi_port[index]);
        ui->tableWidget->setCellWidget(2, index + 2, comboBox);
        connect(comboBox, SIGNAL(new_value(int, int, int)), this, SLOT(tableDataChanged(int, int, int)));
    }
    else {
        ui->tableWidget->setCellWidget(2, index + 2, NULL);
    }

    // Fill item data
    QStringList itemList;
    for (uint16_t item = 0; item < get_song_item_max(index); item++) itemList.append(get_song_item_name(index, item));

    for (uint8_t part = 0; part < NUMBER_OF_PARTS; part++) {
        CustomComboBox *comboBox = new CustomComboBox(ui->tableWidget, part + 3, index + 2);
        comboBox->addItems(itemList);
        comboBox->setCurrentIndex(get_song_item(part, index));
        ui->tableWidget->setCellWidget(part + 3, index + 2, comboBox);
        connect(comboBox, SIGNAL(new_value(int, int, int)), this, SLOT(tableDataChanged(int, int, int)));
    }
}

void songEditDialog::tableDataChanged(int row, int col, int data)
{
    qDebug() << "Data changed" << row << col << data;

    if (row == 0) { // Change target
        if (col > 1) {
            set_song_target(col - 2, data);
            for (uint8_t part = 0; part < NUMBER_OF_PARTS; part++) set_song_item(part, col - 2, 0);
            fillParts(col - 2);
        }
    }

    if (row == 1) { // Change MIDI channel
        if (col > 1) {
            Current_song_midi_channel[col - 2] = data;
        }
    }

    if (row == 2) { // Change MIDI port
        if (col > 1) {
            Current_song_midi_port[col - 2] = data;
        }
    }

    if ((row >= 3) && (row < (NUMBER_OF_PARTS + 3))) { // Change part item data
        if (col > 1) {
            set_song_item(row - 3, col - 2, data);
            if (!check_part_active(row - 3)) {
                set_part_active_state(row - 3, true);
            }
        }
    }
}

void songEditDialog::partEnabledStateChanged(int, int part, bool state)
{
    set_part_active_state(part, state);
}

void songEditDialog::partNameChanged(QString new_name)
{
    customLineEdit* lineEdit = qobject_cast<customLineEdit*>(sender());
    uint8_t part = lineEdit->getDeviceIndex() - 3;
    qDebug() << part << new_name;

    if (part < NUMBER_OF_PARTS) set_part_name(part, new_name);
}

void songEditDialog::fill_target_combobox(QComboBox *cbox)
{
    cbox->clear();
    cbox->addItem("OFF");
    cbox->addItem("MIDI PC");
    cbox->addItem("MIDI CC");
    cbox->addItem("TEMPO");
    for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) cbox->addItem(Device[d]->device_name);
}

int songEditDialog::get_song_target(uint8_t target)
{
   return Device_patches[my_index][SONG_TARGET_DEVICE1 + target];
}

void songEditDialog::set_song_target(uint8_t target, uint8_t value)
{
    Device_patches[my_index][SONG_TARGET_DEVICE1 + target] = value;
}

QString songEditDialog::get_part_name(uint8_t part)
{
    QString name = "";
    if (part >= NUMBER_OF_PARTS) return "";
    uint8_t part_index = SONG_PART_BASE_INDEX + (SONG_PART_SIZE * part);
    for (uint8_t i = 0; i < SONG_PART_NAME_SIZE; i++) name += (char)Device_patches[my_index][part_index++];
    return name;
}

void songEditDialog::set_part_name(uint8_t part, QString name)
{
    if (part >= NUMBER_OF_PARTS) return;
    uint8_t len = name.length();
    if (len > SONG_PART_NAME_SIZE) len = SONG_PART_NAME_SIZE;
    uint8_t part_index = SONG_PART_BASE_INDEX + (SONG_PART_SIZE * part);
    for (uint8_t i = 0; i < len; i++) Device_patches[my_index][part_index++] = (uint8_t) name.at(i).unicode();
    for (uint8_t i = len; i < SONG_PART_NAME_SIZE; i++) Device_patches[my_index][part_index++] = ' ';
}

uint16_t songEditDialog::get_song_item(uint8_t part, uint8_t index)
{
    uint8_t item_index = SONG_PART_BASE_INDEX + SONG_PART_NAME_SIZE + (SONG_PART_SIZE * part) + (index * 2);
    return (Device_patches[my_index][item_index] << 8) + Device_patches[my_index][item_index + 1];
}

void songEditDialog::set_song_item(uint8_t part, uint8_t index, uint16_t item)
{
    uint8_t item_index = SONG_PART_BASE_INDEX + SONG_PART_NAME_SIZE + (SONG_PART_SIZE * part) + (index * 2);
    Device_patches[my_index][item_index] = item >> 8;
    Device_patches[my_index][item_index + 1] = item & 0xFF;
}

QString songEditDialog::get_song_item_name(uint8_t index, uint16_t item)
{
    if (index >= NUMBER_OF_SONG_TARGETS) return "";
      uint8_t target = Device_patches[my_index][SONG_TARGET_DEVICE1 + index];
      if (target == SONG_TARGET_OFF) return "--";
      if (target == SONG_TARGET_PC) return "PC#" + QString::number(item);
      if (target == SONG_TARGET_CC) return "CC#" + QString::number(item >> 7) + " val:" + QString::number(item & 0x7F);
      if (target == SONG_TARGET_TEMPO) return QString::number(item + MIN_BPM) + " BPM";
      if ((target >= SONG_TARGET_FIRST_DEVICE) && (target < NUMBER_OF_DEVICES + SONG_TARGET_FIRST_DEVICE))
          return Device[target - SONG_TARGET_FIRST_DEVICE]->setlist_song_short_item_format(item);
      return "";
}

uint16_t songEditDialog::get_song_item_max(uint8_t index)
{
    if (index >= NUMBER_OF_SONG_TARGETS) return 0;
      uint8_t target = Device_patches[my_index][SONG_TARGET_DEVICE1 + index];
      if (target == SONG_TARGET_PC) return 127;
      if (target == SONG_TARGET_CC) return 16383;
      if (target == SONG_TARGET_TEMPO) return MAX_BPM - MIN_BPM;
      if ((target >= SONG_TARGET_FIRST_DEVICE) && (target < NUMBER_OF_DEVICES + SONG_TARGET_FIRST_DEVICE)) return Device[target - SONG_TARGET_FIRST_DEVICE]->setlist_song_get_number_of_items();
      return 0;
}

bool songEditDialog::check_part_active(uint8_t part)
{
    if (part >= NUMBER_OF_PARTS) return false;
    return ((Device_patches[my_index][SONG_PART_ACTIVE_INDEX] & (1 << part)) != 0);
}

void songEditDialog::set_part_active_state(uint8_t part, bool state)
{
    if (part >= NUMBER_OF_PARTS) return;
    uint8_t byte = Device_patches[my_index][SONG_PART_ACTIVE_INDEX];
    if (state) {
        byte |= (1 << part); //Set the bit
    }
    else {
        byte &= ~(1 << part); //Reset the bit
    }
    Device_patches[my_index][SONG_PART_ACTIVE_INDEX] = byte;
    myCheckBoxes[part]->setChecked(state);
}

/*void songEditDialog::set_part_visible(uint8_t part, bool state)
{
    if (part > NUMBER_OF_PARTS) return;
    ui->tableWidget->setCurrentCell(1, part + 3);
    QBrush my_brush;
    my_brush.setColor(Qt::gray);
    ui->tableWidget->currentItem()->setBackground(my_brush);
}*/

int songEditDialog::newIndex()
{
    for (int i = 0; i < MAX_NUMBER_OF_DEVICE_PRESETS; i++) {
        if (Device_patches[i][0] == 0) return i;
    }
    return PATCH_INDEX_NOT_FOUND;
}


void songEditDialog::on_tempoComboBox_currentIndexChanged(int index)
{
    my_tempo = index + GLOBAL_TEMPO;
    qDebug() << "New tempo" << index + GLOBAL_TEMPO;
}

