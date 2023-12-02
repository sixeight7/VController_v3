#include "vcuserdevices.h"

#include "QtCore/qjsonobject.h"
#include "customlineedit.h"
#include "customcheckbox.h"
#include "customcombobox.h"
#include <QSettings>
#include <QDebug>
#include <QMenu>
#include <QKeyEvent>
#include <QLabel>
#include <QHeaderView>

VCuserdevices::VCuserdevices(QObject *parent) : QObject(parent)
{

}

void VCuserdevices::fillTreeWidget(QTreeWidget *my_tree)
{
    current_user_device_data = USER_device[current_user_device]->get_device_data();
    USER_current_device_data_patch_min = (current_user_device_data.patch_min_msb << 8) + current_user_device_data.patch_min_lsb;
    USER_current_device_data_patch_max = (current_user_device_data.patch_max_msb << 8) + current_user_device_data.patch_max_lsb;

    QTreeWidgetItem *parent1 = new QTreeWidgetItem(my_tree);

    // Device name settings
    parent1->setText(0, "Device name settings");
    my_tree->addTopLevelItem(parent1);
    addTextItem(my_tree, parent1, "Full device name", current_user_device_data.full_name, 16, 0);
    addTextItem(my_tree, parent1, "Short device name", current_user_device_data.short_name, 6, 1);

    // Patch number settings
    QTreeWidgetItem *parent2 = new QTreeWidgetItem(my_tree);
    parent2->setText(0, "Patch number settings");
    my_tree->addTopLevelItem(parent2);
    addIntItem(my_tree, parent2, "Minumum patch number", USER_current_device_data_patch_min, 2048, 0);
    addIntItem(my_tree, parent2, "Maxumum patch number", USER_current_device_data_patch_max, 2048, 1);
    addTextItem(my_tree, parent2, "First patch number format", current_user_device_data.first_patch_format, 6, 2);
    addTextItem(my_tree, parent2, "Last patch number format", current_user_device_data.last_patch_format, 6, 3);
    addOptionItem(my_tree, parent2, "PC type", current_user_device_data.pc_type, 5, 3);

    // Midi settings
    QTreeWidgetItem *parent3 = new QTreeWidgetItem(my_tree);
    parent3->setText(0, "Midi settings");
    my_tree->addTopLevelItem(parent3);
    for(uint8_t p = 0; p < USER_NUMBER_OF_CC_PARAMETERS; p++) {
        QTreeWidgetItem *parameter = new QTreeWidgetItem(parent3);
        parameter->setText(0, USER_device[current_user_device]->read_parameter_name_for_table(p));
        addIntItem(my_tree, parameter, "CC number", current_user_device_data.parameter_CC[p], 127, (p * 4) + 10);
        if (p < USER_NUMBER_OF_CC_PARAMETERS_WITH_TYPE) {
            addIntItem(my_tree, parameter, "CC max value", current_user_device_data.parameter_value_max[p], 127, (p * 4) + 11);
            addIntItem(my_tree, parameter, "CC min value", current_user_device_data.parameter_value_min[p], 127, (p * 4) + 12);
            addOptionItem(my_tree, parameter, "Parameter type", current_user_device_data.parameter_type[p], 5, (p * 4) + 13);
        }
        else {
            addIntItem(my_tree, parameter, "CC value", current_user_device_data.parameter_value_max[p], 127, (p * 4) + 11);

        }
    }
    addIntItem(my_tree, parent3, "Looper length (in seconds)", current_user_device_data.looper_length, 255, 2);
}

void VCuserdevices::fillPatchNameTableWidget(int instance, QTableWidget *patchWidget)
{
    patchWidget->clear();
    patchWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    patchWidget->setColumnCount(2);
    patchWidget->setHorizontalHeaderLabels({"Patch number", "Patch name"});
    patchWidget->setColumnWidth(0, 120);
    //patchWidget->setColumnWidth(1, 120);
    patchWidget->horizontalHeader()->setStretchLastSection(true);
    patchWidget->setRowCount(USER_current_device_data_patch_max - USER_current_device_data_patch_min + 1);
    //for (int r = 0; r < NUMBER_OF_CMD_BYTES; r++) my_widget->setRowHeight(r, 30);
    patchWidget->setShowGrid(false);
    patchWidget->setStyleSheet("QTableWidget::item { padding: 2px }");

    for (int i = USER_current_device_data_patch_min; i <= USER_current_device_data_patch_max; i++) {
        patchWidget->setVerticalHeaderItem(i - USER_current_device_data_patch_min, new QTableWidgetItem(QString::number(i)));
        QTableWidgetItem *patchNumber = new QTableWidgetItem(USER_device[instance]->number_format(i));
        patchNumber->setFlags(patchNumber->flags() ^ Qt::ItemIsEditable);
        patchWidget->setItem(i - USER_current_device_data_patch_min, 0, patchNumber);

        customLineEdit *patchNameLabel = new customLineEdit(patchWidget, instance, i);
        patchNameLabel->setText(read_user_item_name(USER_DEVICE_PATCH_NAME_TYPE, instance, i));
        patchNameLabel->setMaxLength(12);
        patchWidget->setCellWidget(i - USER_current_device_data_patch_min, 1, patchNameLabel);
        connect(patchNameLabel, SIGNAL(new_text(int, int, const QString &)), this, SLOT(patchNameChanged(int, int, const QString &)));
    }

    patchWidget->repaint();
}

void VCuserdevices::fillFxAndSceneNameTableWidget(int instance, QTableWidget *fxAndSceneWidget)
{
#define NUMBER_OF_EFFECTS_SHOWN 10
    uint8_t number_of_items = NUMBER_OF_EFFECTS_SHOWN + USER_device[instance]->get_number_of_snapscenes();
    fxAndSceneWidget->clear();
    fxAndSceneWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    fxAndSceneWidget->setColumnCount(4);
    fxAndSceneWidget->setHorizontalHeaderLabels({"FX/Scene", "FX state on/off", "Name", "Colour"});
    fxAndSceneWidget->setColumnWidth(0, 100);
    //fxAndSceneWidget->setColumnWidth(1, 120);
    fxAndSceneWidget->horizontalHeader()->setStretchLastSection(true);
    fxAndSceneWidget->setRowCount(number_of_items);
    //for (int r = 0; r < NUMBER_OF_CMD_BYTES; r++) my_widget->setRowHeight(r, 30);
    fxAndSceneWidget->setShowGrid(false);
    fxAndSceneWidget->setStyleSheet("QTableWidget::item { padding: 2px }");

    fx_state = read_user_item_data(1, instance, selectedPatchNumber);
    uint8_t colour = 0;
    for (int i = 0; i < number_of_items; i++) {
        QString itemName;
        bool itemIsFX = (i < NUMBER_OF_EFFECTS_SHOWN);
        if (itemIsFX) itemName = USER_device[instance]->read_parameter_name_for_table(i);
        else itemName = "Scene " + QString::number(i - NUMBER_OF_EFFECTS_SHOWN + 1);
        QTableWidgetItem *titleItem = new QTableWidgetItem(itemName);
        titleItem->setFlags(titleItem->flags() ^ Qt::ItemIsEditable);
        fxAndSceneWidget->setItem(i, 0, titleItem);

        if (i < 8) {
            CustomCheckBox *fxOnOff = new CustomCheckBox(fxAndSceneWidget, instance, i);
            fxOnOff->setChecked(check_fx_active(i));
            fxOnOff->setStyleSheet("margin-left:40%; margin-right:60%;");
            fxAndSceneWidget->setCellWidget(i, 1, fxOnOff);
            connect(fxOnOff, SIGNAL(custom_clicked(int, int, bool)), this, SLOT(fxStateChanged(int, int, bool)));
        }

        customLineEdit *fxAndSceneNameLabel = new customLineEdit(fxAndSceneWidget, instance, i);
        QString myName = getFxOrSceneName(instance, i, globalChecked);
        if ((!globalChecked) && (myName == "")) {
            myName = getFxOrSceneName(instance, i, true);
            fxAndSceneNameLabel->setStyleSheet("customLineEdit { background-color: rgb(215, 214, 230); }");
        }
        fxAndSceneNameLabel->setText(myName);
        fxAndSceneNameLabel->setMaxLength(12);
        fxAndSceneWidget->setCellWidget(i, 2, fxAndSceneNameLabel);
        connect(fxAndSceneNameLabel, SIGNAL(new_text(int, int, const QString &)), this, SLOT(fxOrSceneNameChanged(int, int, const QString &)));

        if (i < number_of_items) {
            CustomComboBox *colourBox = new CustomComboBox(fxAndSceneWidget, instance, i);
            for (int c = 0; c < NUMBER_OF_COLOURS; c++) colourBox->addItem(colours[c]);
            if (globalChecked) colour = read_user_item_data(2, instance, i << 11);
            else colour = read_user_item_data(2, instance, (i << 11) + selectedPatchNumber + 1);
            colourBox->setCurrentIndex(colour);
            fxAndSceneWidget->setCellWidget(i, 3, colourBox);
            connect(colourBox, SIGNAL(new_value(int, int, int)), this, SLOT(fxColourChanged(int, int, int)));
        }
    }
}

void VCuserdevices::updateFxAndSceneNameTableWidget(int instance, QTableWidget *fxAndSceneWidget)
{
    uint8_t number_of_items = NUMBER_OF_EFFECTS_SHOWN + USER_device[instance]->get_number_of_snapscenes();
    fx_state = read_user_item_data(1, instance, selectedPatchNumber);
    uint8_t colour = 0;
    for (int i = 0; i < number_of_items; i++) {

        if (i < 8) {
            CustomCheckBox *fxOnOff;
            fxOnOff = (CustomCheckBox*) fxAndSceneWidget->cellWidget(i, 1);
            fxOnOff->setChecked(check_fx_active(i));
        }

        customLineEdit *fxAndSceneNameLabel;
        fxAndSceneNameLabel = (customLineEdit*) fxAndSceneWidget->cellWidget(i, 2);
        QString myName = getFxOrSceneName(instance, i, globalChecked);
        if ((!globalChecked) && (myName == "")) {
            myName = getFxOrSceneName(instance, i, true);
            fxAndSceneNameLabel->setStyleSheet("customLineEdit { background-color: rgb(215, 214, 230); }");
        }
        fxAndSceneNameLabel->setText(myName);

        if (i < number_of_items) {
            CustomComboBox *colourBox;
            colourBox = (CustomComboBox*) fxAndSceneWidget->cellWidget(i, 3);
            for (int c = 0; c < NUMBER_OF_COLOURS; c++) colourBox->addItem(colours[c]);
            if (globalChecked) colour = read_user_item_data(2, instance, i << 11);
            else colour = read_user_item_data(2, instance, (i << 11) + selectedPatchNumber + 1);
            colourBox->setCurrentIndex(colour);
        }
    }

}

void VCuserdevices::intItemChanged(int, int index, int value)
{
    switch (index) {
    case 0:
        USER_current_device_data_patch_min = value;
        current_user_device_data.patch_min_msb = value >> 8;
        current_user_device_data.patch_min_lsb = value & 0xFF;
        emit updateUserDeviceNameWidget();
        break;
    case 1:
        USER_current_device_data_patch_max = value;
        current_user_device_data.patch_max_msb = value >> 8;
        current_user_device_data.patch_max_lsb = value & 0xFF;
        emit updateUserDeviceNameWidget();
        break;
    case 2:
        current_user_device_data.looper_length = value;
        break;
    case 3:
        current_user_device_data.pc_type = value;
        break;
    default:
        if (index >= 10) {
            int parameter = (index - 10) / 4;
            int item = (index - 10) % 4;
            if (item == 0) current_user_device_data.parameter_CC[parameter] = value;
            if (item == 1) current_user_device_data.parameter_value_max[parameter] = value;
            if (item == 2) current_user_device_data.parameter_value_min[parameter] = value;
            if (item == 3) current_user_device_data.parameter_type[parameter] = value;
        }
    }
    USER_device[current_user_device]->set_device_data(&current_user_device_data);
}

void VCuserdevices::textItemChanged(int, int index, const QString &text)
{
    uint8_t len;
    bool updateUserDeviceNames = false;
    switch (index) {
    case 0:
        len = text.length();
        if (len > 16) len = 16;
        for (uint8_t c = 0; c < len; c++) current_user_device_data.full_name[c] = text[c].toLatin1();
        for (uint8_t c = len; c < 16; c++) current_user_device_data.full_name[c] = ' ';
        emit updateFullName();
        break;
    case 1:
        len = text.length();
        if (len > 6) len = 6;
        for (uint8_t c = 0; c < len; c++) current_user_device_data.short_name[c] = text[c].toLatin1();
        for (uint8_t c = len; c < 6; c++) current_user_device_data.short_name[c] = ' ';
        break;
    case 2:
        len = text.length();
        if (len > 6) len = 6;
        for (uint8_t c = 0; c < len; c++) current_user_device_data.first_patch_format[c] = text[c].toLatin1();
        for (uint8_t c = len; c < 6; c++) current_user_device_data.first_patch_format[c] = ' ';
        updateUserDeviceNames = true;
        break;
    case 3:
        len = text.length();
        if (len > 6) len = 6;
        for (uint8_t c = 0; c < len; c++) current_user_device_data.last_patch_format[c] = text[c].toLatin1();
        for (uint8_t c = len; c < 6; c++) current_user_device_data.last_patch_format[c] = ' ';
        updateUserDeviceNames = true;
        break;
    }
    USER_device[current_user_device]->set_device_data(&current_user_device_data);
    if (updateUserDeviceNames) emit updateUserDeviceNameWidget();
}

void VCuserdevices::patchNameChanged(int instance, int patch_no, const QString &name)
{
    qDebug() << "Name:" << name;
    store_user_item_name(USER_DEVICE_PATCH_NAME_TYPE, instance, patch_no, name);
}

void VCuserdevices::fxOrSceneNameChanged(int instance, int item_no, const QString &name)
{
    qDebug() << "Name:" << name;
    if (item_no < NUMBER_OF_EFFECTS_SHOWN) {
        if (globalChecked) store_user_item_name(USER_DEVICE_FX_NAME_TYPE, instance, item_no << 11, name);
        else store_user_item_name(USER_DEVICE_FX_NAME_TYPE, instance, (item_no << 11) + selectedPatchNumber + 1, name);
    }
    else {
        uint8_t scene = item_no - NUMBER_OF_EFFECTS_SHOWN + 1;
        if (globalChecked) store_user_item_name(USER_DEVICE_SCENE_NAME_TYPE, instance, scene << 11, name);
        else store_user_item_name(USER_DEVICE_SCENE_NAME_TYPE, instance, (scene << 11) + selectedPatchNumber + 1, name);
    }
    if (!globalChecked) {
        customLineEdit* obj = qobject_cast<customLineEdit*>(sender());
        if (name != "") obj->setStyleSheet("customLineEdit { background-color: rgb(255, 255, 255); }");
        else obj->setStyleSheet("customLineEdit { background-color: rgb(215, 214, 230); }");
    }
}

void VCuserdevices::fxStateChanged(int instance, int fx, bool state)
{
    set_fx_state(fx, state);
    store_user_item_data(USER_DEVICE_PATCH_NAME_TYPE, instance, selectedPatchNumber, fx_state);
}

void VCuserdevices::fxColourChanged(int instance, int fx, int colour)
{
    if (globalChecked) store_user_item_data(USER_DEVICE_FX_NAME_TYPE, instance, fx << 11, colour);
        else store_user_item_data(USER_DEVICE_FX_NAME_TYPE, instance, (fx << 11) + selectedPatchNumber + 1, colour);
}

void VCuserdevices::addTextItem(QTreeWidget *my_tree, QTreeWidgetItem *parent, QString label, QString text, uint8_t len, uint8_t index) {
    QTreeWidgetItem *child = new QTreeWidgetItem(parent);
    child->setText(0, label);
    customLineEdit *textItem = new customLineEdit(my_tree, 0, index);
    textItem->setMaxLength(len);
    textItem->setText(text.trimmed());
    textItem->setAlignment(Qt::AlignCenter);
    my_tree->setItemWidget(child, 2, textItem);
    connect(textItem, SIGNAL(new_text(int, int, const QString &)), this, SLOT(textItemChanged(int, int, const QString &)));
}

void VCuserdevices::addIntItem(QTreeWidget *my_tree, QTreeWidgetItem *parent, QString label, uint16_t number, uint16_t max, uint8_t index)
{
    QTreeWidgetItem *child = new QTreeWidgetItem(parent);
    child->setText(0, label);
    CustomSlider *slider = new CustomSlider(my_tree, 0, index);
    slider->setRange(0, max);
    slider->setValue(number);
    my_tree->setItemWidget(child, 4, slider);
    connect(slider, SIGNAL(new_value(int, int, int)), this, SLOT(intItemChanged(int, int, int)));

    CustomSpinBox *spinBox = new CustomSpinBox(my_tree, 0, index);
    spinBox->setRange(0, max);
    spinBox->setValue(number);
    my_tree->setItemWidget(child, 2, spinBox);
    connect(spinBox, SIGNAL(new_value(int, int, int)), this, SLOT(intItemChanged(int, int, int)));
    connect(spinBox, SIGNAL(valueChanged(int)), slider, SLOT(setValue(int)));
    connect(slider, SIGNAL(valueChanged(int)), spinBox, SLOT(setValue(int)));
}

void VCuserdevices::addOptionItem(QTreeWidget *my_tree, QTreeWidgetItem *parent, QString label, uint16_t number, uint16_t max, uint8_t index)
{
    QTreeWidgetItem *child = new QTreeWidgetItem(parent);
    child->setText(0, label);
    CustomSlider *slider = new CustomSlider(my_tree, 0, index);
    slider->setRange(0, max);
    slider->setValue(number);
    my_tree->setItemWidget(child, 4, slider);
    connect(slider, SIGNAL(new_value(int, int, int)), this, SLOT(intItemChanged(int, int, int)));

    CustomComboBox *comboBox = new CustomComboBox(my_tree, 0, index);
    if (index == 3) comboBox->addItems({"PC only", "PC and CC 0", "PC and CC32", "PC/CC0 mod 100", "PC/CC32 mod 100", "NUX CC 60"});
    else comboBox->addItems({"MOMENTARY", "TOGGLE", "STEP", "RANGE",  "UPDOWN", "ONE SHOT"});
    comboBox->setCurrentIndex(number);
    my_tree->setItemWidget(child, 2, comboBox);
    connect(comboBox, SIGNAL(new_value(int, int, int)), this, SLOT(intItemChanged(int, int, int)));
    connect(comboBox,SIGNAL(currentIndexChanged(int)), slider, SLOT(setValue(int)));
    connect(slider, SIGNAL(valueChanged(int)), comboBox, SLOT(setCurrentIndex(int)));
}

int VCuserdevices::find_user_data_index(uint8_t typedev, uint16_t patch_number)
{
    uint8_t patch_msb = patch_number >> 8;
    uint8_t patch_lsb = patch_number & 0xFF;
    for (int i = 0; i < User_device_data_item.size(); i++) {
        if ((User_device_data_item[i].type_and_dev == typedev) && (User_device_data_item[i].patch_msb == patch_msb) && (User_device_data_item[i].patch_lsb == patch_lsb)) return i;
    }
    return DATA_INDEX_NOT_FOUND;
}

int VCuserdevices::find_new_user_data_index()
{
    for (uint16_t i = 0; i < User_device_data_item.size(); i++) {
        if (User_device_data_item[i].type_and_dev == 0) return i;
    }
    User_device_name_struct new_item = { 0, 0, 0, 0, {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}};
    User_device_data_item.append(new_item);
    return User_device_data_item.size() - 1;
}

void VCuserdevices::deleteUserItemsForInstance(uint8_t instance)
{
    for (int i = 0; i < User_device_data_item.size(); i++) {
        if ((User_device_data_item[i].type_and_dev & 0x0F) == instance) User_device_data_item[i].type_and_dev = 0;
    }
}

void VCuserdevices::initializeUserDeviceData(uint8_t instance, bool deviceData, bool patchNames, bool fxStates, bool fxNames, bool sceneNames, bool customColours)
{
    if (deviceData) USER_device[instance]->initialize_device_data();

    for (int i = 0; i < User_device_data_item.size(); i++) {
        if ((User_device_data_item[i].type_and_dev & 0x0F) == instance) {
            uint8_t type = User_device_data_item[i].type_and_dev >> 4;
            if ((patchNames) && (type == USER_DEVICE_PATCH_NAME_TYPE)) for (uint8_t c = 0; c < 12; c++) User_device_data_item[i].name[c] = ' ';
            if ((fxStates) && (type == USER_DEVICE_PATCH_NAME_TYPE)) User_device_data_item[i].value = 0;
            if ((fxNames) && (type == USER_DEVICE_FX_NAME_TYPE)) for (uint8_t c = 0; c < 12; c++) User_device_data_item[i].name[c] = ' ';
            if ((sceneNames) && (type == USER_DEVICE_SCENE_NAME_TYPE)) for (uint8_t c = 0; c < 12; c++) User_device_data_item[i].name[c] = ' ';
            if ((customColours) && (type == USER_DEVICE_FX_NAME_TYPE)) User_device_data_item[i].value = 0;
        }
    }

    cleanUpUserDeviceNameItems();
}

void VCuserdevices::clearPatchItems(uint8_t instance, uint16_t patch_no)
{
    for (int i = User_device_data_item.size(); i --> 0;) {
        if ((User_device_data_item[i].type_and_dev & 0x0F) == instance) {
            uint8_t type = User_device_data_item[i].type_and_dev >> 4;
            int number = (User_device_data_item[i].patch_msb << 8) + User_device_data_item[i].patch_lsb;
            if (((type == USER_DEVICE_PATCH_NAME_TYPE) &&  (number == patch_no)) ||
                ((type == USER_DEVICE_FX_NAME_TYPE)    && ((number & 0x07FF)) == (patch_no + 1)) ||
                ((type == USER_DEVICE_SCENE_NAME_TYPE) && ((number & 0x07FF)) == (patch_no + 1))) {
                User_device_data_item.removeAt(i);
                qDebug() << "Type: " << type << " number: " << (number & 0x07FF) << "removed from memory at location" << i;
            }
        }
    }
}

void VCuserdevices::clearCopyBuffer(int offset)
{
    copyBuffer.clear();
    copyBufferNumberOffset = offset;
}

void VCuserdevices::copyPatchItems(uint8_t instance, uint16_t patch_no)
{
    for (int i = 0; i < User_device_data_item.size(); i++) {
        if ((User_device_data_item[i].type_and_dev & 0x0F) == instance) {
            uint8_t type = User_device_data_item[i].type_and_dev >> 4;
            int number = (User_device_data_item[i].patch_msb << 8) + User_device_data_item[i].patch_lsb;
            if (((type == USER_DEVICE_PATCH_NAME_TYPE) &&  (number == patch_no)) ||
                ((type == USER_DEVICE_FX_NAME_TYPE)    && ((number & 0x07FF)) == (patch_no + 1)) ||
                ((type == USER_DEVICE_SCENE_NAME_TYPE) && ((number & 0x07FF)) == (patch_no + 1))) {
                copyBuffer.append(User_device_data_item[i]);
                qDebug() << "Type: " << type << " number: " << (number & 0x07FF) << "copied to buffer from location" << i;
            }
        }
    }
    qDebug() << "Buffersize: " << copyBuffer.size();
}

void VCuserdevices::pastePatchItems(uint8_t instance, uint16_t patch_index)
{
    for (int i = 0; i < copyBuffer.size(); i++) {
        int number = (copyBuffer[i].patch_msb << 8) + copyBuffer[i].patch_lsb;
        int patch_no = patch_index + number - copyBufferNumberOffset;
        uint8_t type = (copyBuffer[i].type_and_dev & 0xF0) >> 4;
        if (type == USER_DEVICE_PATCH_NAME_TYPE) clearPatchItems(instance, patch_no);
    }
    for (int i = 0; i < copyBuffer.size(); i++) {
        int number = (copyBuffer[i].patch_msb << 8) + copyBuffer[i].patch_lsb;
        int patch_no = patch_index + number - copyBufferNumberOffset;
        uint8_t typedev = (copyBuffer[i].type_and_dev & 0xF0) + (instance & 0x0F);
        uint16_t index = find_user_data_index(typedev, patch_no);
        if (index == DATA_INDEX_NOT_FOUND) index = find_new_user_data_index();
        User_device_data_item[index].type_and_dev = typedev;
        User_device_data_item[index].patch_msb = patch_no >> 8;
        User_device_data_item[index].patch_lsb = patch_no & 0xFF;
        for (uint8_t c = 0; c < 12; c++) User_device_data_item[index].name[c] = copyBuffer[i].name[c];
        User_device_data_item[index].value = copyBuffer[i].value;
        qDebug() << "Type: " << ((copyBuffer[i].type_and_dev & 0xF0) >> 4) << " number: " << (patch_no & 0x07FF) << "pasted from buffer to location" << index;
   }
}

int VCuserdevices::getNumberOfUserItems()
{
    return User_device_data_item.size();
}

void VCuserdevices::setCurrentUserDevice(uint8_t newCurrentUserDevice)
{
    current_user_device = newCurrentUserDevice;
}

QString VCuserdevices::read_user_item_name(uint8_t type, uint8_t USER_dev, uint16_t patch_no)
{
    uint8_t typedev = (type << 4) + (USER_dev & 0x0F);
    int index = find_user_data_index(typedev, patch_no);
    //qDebug() << "Checking" << type << USER_dev << patch_no << index;
    if (index != PATCH_INDEX_NOT_FOUND) {
        QString name = "";
        for (uint8_t c = 0; c < 12; c++) name += (char) User_device_data_item[index].name[c];
        return name.trimmed();
    }
    else {
        return "";
    }
}

void VCuserdevices::store_user_item_name(uint8_t type, uint8_t USER_dev, uint16_t patch_no, QString name)
{
    uint8_t typedev = (type << 4) + (USER_dev & 0x0F);
    uint16_t index = find_user_data_index(typedev, patch_no);
    if (index == DATA_INDEX_NOT_FOUND) index = find_new_user_data_index();
    //qDebug() << "** Writing" << type << USER_dev << patch_no << index;
    uint8_t len = name.length();
    if (len > 12) len = 12;
    User_device_data_item[index].type_and_dev = typedev;
    User_device_data_item[index].patch_msb = patch_no >> 8;
    User_device_data_item[index].patch_lsb = patch_no & 0xFF;
    for (uint8_t c = 0; c < len; c++) User_device_data_item[index].name[c] = (uint8_t) name[c].toLatin1();
    for (uint8_t c = len; c < 12; c++) User_device_data_item[index].name[c] = ' ';
}

uint8_t VCuserdevices::read_user_item_data(uint8_t type, uint8_t USER_dev, uint16_t patch_no)
{
    uint8_t typedev = (type << 4) + (USER_dev & 0x0F);
    int index = find_user_data_index(typedev, patch_no);
    //qDebug() << "Checking for value" << type << USER_dev << patch_no << index;
    if (index != PATCH_INDEX_NOT_FOUND) {
        return User_device_data_item[index].value;
    }
    else {
        return 0;
    }
}

void VCuserdevices::store_user_item_data(uint8_t type, uint8_t USER_dev, uint16_t patch_no, uint8_t value)
{
    uint8_t typedev = (type << 4) + (USER_dev & 0x0F);
    uint16_t index = find_user_data_index(typedev, patch_no);
    if (index == DATA_INDEX_NOT_FOUND) index = find_new_user_data_index();
    //qDebug() << "** Writing value to " << type << USER_dev << patch_no << index;
    User_device_data_item[index].type_and_dev = typedev;
    User_device_data_item[index].patch_msb = patch_no >> 8;
    User_device_data_item[index].patch_lsb = patch_no & 0xFF;
    User_device_data_item[index].value = value;
}

bool VCuserdevices::check_fx_active(uint8_t number)
{
    if (number >= 8) return false;
    return ((fx_state & (1 << number)) != 0);
}

void VCuserdevices::set_fx_state(uint8_t number, bool state)
{
    if (number >= 8) return;
    if (state) {
        fx_state |= (1 << number); //Set the bit
    }
    else {
        fx_state &= ~(1 << number); //Reset the bit
    }
}

QString VCuserdevices::getFxOrSceneName(uint8_t instance, uint8_t item_no, bool isGlobal)
{
    if (item_no < NUMBER_OF_EFFECTS_SHOWN) {
        if (isGlobal) return read_user_item_name(USER_DEVICE_FX_NAME_TYPE, instance, item_no << 11);
        else return read_user_item_name(USER_DEVICE_FX_NAME_TYPE, instance, (item_no << 11) + selectedPatchNumber + 1);
    }
    else {
        uint8_t scene = item_no - NUMBER_OF_EFFECTS_SHOWN + 1;
        if (isGlobal) return read_user_item_name(USER_DEVICE_SCENE_NAME_TYPE, instance, scene << 11);
        else return read_user_item_name(USER_DEVICE_SCENE_NAME_TYPE, instance, (scene << 11) + selectedPatchNumber + 1);
    }
    return "";
}

bool VCuserdevices::checkUserDeviceDataItemIsEmpty(int index)
{
    if (User_device_data_item[index].type_and_dev == 0) return true;
    for(uint8_t c = 0; c < 12; c++) {
        if (User_device_data_item[index].name[c] != ' ') return false;
    }
    return (User_device_data_item[index].value == 0);
}

void VCuserdevices::cleanUpUserDeviceNameItems()
{
    for(int i = User_device_data_item.size(); i --> 0;) {
        bool removeItem = false;
        if (checkUserDeviceDataItemIsEmpty(i)) {
            removeItem = true;
        }
        else { // Check for duplicates
            for (int j = 0; j < i; j++) {
                if ((User_device_data_item[j].type_and_dev == User_device_data_item[i].type_and_dev)
                    && (User_device_data_item[j].patch_lsb == User_device_data_item[i].patch_lsb)
                    && (User_device_data_item[j].patch_msb == User_device_data_item[i].patch_msb)) removeItem = true;
            }
        }
        if (removeItem) User_device_data_item.removeAt(i);
    }
}

void VCuserdevices::setSelectedPatchNumber(int newSelectedPatchNumber)
{
    selectedPatchNumber = newSelectedPatchNumber;
}

void VCuserdevices::setGlobalChecked(bool newGlobalChecked)
{
    globalChecked = newGlobalChecked;
}


void VCuserdevices::read(const QJsonObject &json, uint8_t instance, bool readNoInstanceData)
{
    User_device_struct data = USER_device[instance]->get_device_data();
    QString userDeviceSettingsTitle;
    if (readNoInstanceData) userDeviceSettingsTitle = "User device settings";
    else userDeviceSettingsTitle = "User device " + QString::number(instance + 1) + " settings";
    QJsonObject userDeviceSettingsObject = json[userDeviceSettingsTitle].toObject();
    if (!userDeviceSettingsObject.isEmpty()) {
        if (!userDeviceSettingsObject["Full name"].isNull()) {
            QString text = userDeviceSettingsObject["Full name"].toString();
            uint8_t len = text.length();
            if (len > 16) len = 16;
            for (uint8_t c = 0; c < len; c++) data.full_name[c] = text[c].toLatin1();
            for (uint8_t c = len; c < 16; c++) data.full_name[c] = ' ';
        }
        if (!userDeviceSettingsObject["Short name"].isNull()) {
            QString text = userDeviceSettingsObject["Short name"].toString();
            uint8_t len = text.length();
            if (len > 6) len = 6;
            for (uint8_t c = 0; c < len; c++) data.short_name[c] = text[c].toLatin1();
            for (uint8_t c = len; c < 6; c++) data.short_name[c] = ' ';
        }
        if (!userDeviceSettingsObject["Patch min MSB"].isNull()) data.patch_min_msb = userDeviceSettingsObject["Patch min MSB"].toInt();
        if (!userDeviceSettingsObject["Patch min LSB"].isNull()) data.patch_min_lsb = userDeviceSettingsObject["Patch min LSB"].toInt();
        if (!userDeviceSettingsObject["Patch max MSB"].isNull()) data.patch_max_msb = userDeviceSettingsObject["Patch max MSB"].toInt();
        if (!userDeviceSettingsObject["Patch max LSB"].isNull()) data.patch_max_lsb = userDeviceSettingsObject["Patch max LSB"].toInt();
        if (!userDeviceSettingsObject["First patch number format"].isNull()) {
            QString text = userDeviceSettingsObject["First patch number format"].toString();
            uint8_t len = text.length();
            if (len > 6) len = 6;
            for (uint8_t c = 0; c < len; c++) data.first_patch_format[c] = text[c].toLatin1();
            for (uint8_t c = len; c < 6; c++) data.first_patch_format[c] = ' ';
        }
        if (!userDeviceSettingsObject["Last patch number format"].isNull()) {
            QString text = userDeviceSettingsObject["Last patch number format"].toString();
            uint8_t len = text.length();
            if (len > 6) len = 6;
            for (uint8_t c = 0; c < len; c++) data.last_patch_format[c] = text[c].toLatin1();
            for (uint8_t c = len; c < 6; c++) data.last_patch_format[c] = ' ';
        }
        for (uint8_t i = 0; i < 4; i++) {
            if (!userDeviceSettingsObject["Device detect byte " + QString::number(i)].isNull()) data.device_detect[i] = userDeviceSettingsObject["Device detect byte " + QString::number(i)].toInt();
        }
        for (uint8_t i = 0; i < USER_NUMBER_OF_CC_PARAMETERS; i++) {
            QString parName = USER_device[instance]->read_parameter_name_for_table(i);
            if (!userDeviceSettingsObject[parName + " CC"].isNull()) data.parameter_CC[i] = userDeviceSettingsObject[parName + " CC"].toInt();
            if (!userDeviceSettingsObject[parName + " max value"].isNull()) data.parameter_value_max[i] = userDeviceSettingsObject[parName + " max value"].toInt();
            if (i < USER_NUMBER_OF_CC_PARAMETERS_WITH_TYPE) {
                if (!userDeviceSettingsObject[parName + " min value"].isNull()) data.parameter_value_min[i] = userDeviceSettingsObject[parName + " min value"].toInt();
                if (!userDeviceSettingsObject[parName + " type"].isNull()) data.parameter_type[i] = userDeviceSettingsObject[parName + " type"].toInt();
            }
        }
        if (!userDeviceSettingsObject["Looper length"].isNull()) data.looper_length = userDeviceSettingsObject["Looper length"].toInt();
        if (!userDeviceSettingsObject["PC type"].isNull()) data.pc_type = userDeviceSettingsObject["PC type"].toInt();
        USER_device[instance]->set_device_data(&data);
    }

    QString userDeviceNameItemsTitle;
    if (readNoInstanceData) userDeviceNameItemsTitle = "User device name items";
    else userDeviceNameItemsTitle = "User device " + QString::number(instance + 1) + " name items";
    QJsonObject userDeviceNamesObject = json[userDeviceNameItemsTitle].toObject();
    if (!userDeviceNamesObject.isEmpty()) {
        int number_of_items = 0;
        if (!userDeviceNamesObject["Number of items"].isNull()) number_of_items = userDeviceNamesObject["Number of items"].toInt();
        qDebug() << "JSON reading the following number of items: " << number_of_items;
        deleteUserItemsForInstance(instance);
        for (int item = 0; item < number_of_items; item++) {
            QJsonObject nameObject = userDeviceNamesObject["Item " + QString::number(item)].toObject();
            if (!nameObject.isEmpty()) {
                User_device_name_struct new_data_item = {0, 0, 0, 0, {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',' ', ' ', ' ', ' '}};
                if (!nameObject["Typedev"].isNull()) {
                    uint8_t typedev = nameObject["Typedev"].toInt() & 0xF0; // Filter out the instance
                    new_data_item.type_and_dev = typedev + (instance & 0x0F); // Add current instance
                }
                if (!nameObject["Patch MSB"].isNull()) new_data_item.patch_msb = nameObject["Patch MSB"].toInt();
                if (!nameObject["Patch LSB"].isNull()) new_data_item.patch_lsb = nameObject["Patch LSB"].toInt();
                if (!nameObject["Value"].isNull()) new_data_item.value = nameObject["Value"].toInt();
                if (!nameObject["Name"].isNull()) {
                    QString name = nameObject["Name"].toString();
                    uint8_t len = name.length();
                    for (uint8_t c = 0; c < len; c++) new_data_item.name[c] = (uint8_t) name[c].toLatin1();
                    for (uint8_t c = len; c < 12; c++) new_data_item.name[c] = ' ';
                    qDebug() << "item added" << name;
                }
                User_device_data_item.append(new_data_item);
                cleanUpUserDeviceNameItems();

                qDebug() << "User_device_data_item size" << User_device_data_item.size();
            }
        }

    }
}

void VCuserdevices::readAll(const QJsonObject &json)
{
    for (int d = 0; d < NUMBER_OF_USER_DEVICES; d++) read(json, d, false);
}

void VCuserdevices::write(QJsonObject &json, uint8_t instance, bool saveNoInstanceData) const
{
        QString userDeviceSettingsTitle;
        if (saveNoInstanceData) userDeviceSettingsTitle = "User device settings";
        else userDeviceSettingsTitle = "User device " + QString::number(instance + 1) + " settings";User_device_struct data = USER_device[instance]->get_device_data();
        QJsonObject userDeviceSettingsObject;
        userDeviceSettingsObject["Full name"] = data.full_name;
        userDeviceSettingsObject["Short name"] = data.short_name;
        userDeviceSettingsObject["Patch min MSB"] = data.patch_min_msb;
        userDeviceSettingsObject["Patch min LSB"] = data.patch_min_lsb;
        userDeviceSettingsObject["Patch max MSB"] = data.patch_max_msb;
        userDeviceSettingsObject["Patch max LSB"] = data.patch_max_lsb;
        userDeviceSettingsObject["First patch number format"] = data.first_patch_format;
        userDeviceSettingsObject["Last patch number format"] = data.last_patch_format;
        for (uint8_t i = 0; i < 4; i++) userDeviceSettingsObject["Device detect byte " + QString::number(i)] = data.device_detect[i];
        for (uint8_t i = 0; i < USER_NUMBER_OF_CC_PARAMETERS; i++) {
            QString parName = USER_device[instance]->read_parameter_name_for_table(i);
            userDeviceSettingsObject[parName + " CC"] = data.parameter_CC[i];
            userDeviceSettingsObject[parName + " max value"] = data.parameter_value_max[i];
            if (i < USER_NUMBER_OF_CC_PARAMETERS_WITH_TYPE) {
                userDeviceSettingsObject[parName + " min value"] = data.parameter_value_min[i];
                userDeviceSettingsObject[parName + " type"] = data.parameter_type[i];
            }
        }
        userDeviceSettingsObject["Looper length"] = data.looper_length;
        userDeviceSettingsObject["PC type"] = data.pc_type;
        json[userDeviceSettingsTitle] = userDeviceSettingsObject;

        QString userDeviceNameItemsTitle;
        if (saveNoInstanceData) userDeviceNameItemsTitle = "User device name items";
        else userDeviceNameItemsTitle = "User device " + QString::number(instance + 1) + " name items";
        QJsonObject userDeviceNamesObject;
        int item = 0;
        for (int i = 0; i < User_device_data_item.size(); i++) {
            if ((User_device_data_item[i].type_and_dev & 0x0F) == instance) {
                QJsonObject nameObject;
                nameObject["Typedev"] = User_device_data_item[i].type_and_dev;
                nameObject["Patch MSB"] = User_device_data_item[i].patch_msb;
                nameObject["Patch LSB"] = User_device_data_item[i].patch_lsb;
                nameObject["Value"] = User_device_data_item[i].value;
                QString name = "";
                for (uint8_t c = 0; c < 12; c++) name += (char) User_device_data_item[i].name[c];
                nameObject["Name"] = name;
                userDeviceNamesObject["Item " + QString::number(item)] = nameObject;
                item++;
            }
            userDeviceNamesObject["Number of items"] = item;
        }
        json[userDeviceNameItemsTitle] = userDeviceNamesObject;
}

void VCuserdevices::writeAll(QJsonObject &json)
{
        for (int d = 0; d < NUMBER_OF_USER_DEVICES; d++) write(json, d, false);
}
