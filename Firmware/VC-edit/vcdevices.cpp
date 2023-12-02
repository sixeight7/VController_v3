#include "vcdevices.h"
#include "VController/globals.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QProgressBar>

VCdevices::VCdevices(QObject *parent) : QObject(parent)
{
    InitializePatchArea();
    //setup_devices();
}

void VCdevices::fillTreeWidget(QTreeWidget *my_tree, VCcommands *VCd)
{
    QTreeWidgetItem *parent = new QTreeWidgetItem(my_tree);
    parent->setText(0, "Device Settings");
    my_tree->addTopLevelItem(parent);

    for (int d = 0; d < NUMBER_OF_DEVICES; d++) {
        QTreeWidgetItem *deviceChild = new QTreeWidgetItem(parent);
        deviceChild->setText(0, Device[d]->full_device_name);
        //my_tree->addTopLevelItem(deviceChild);
        //parent->addChild(deviceChild);

        for (int i = 0; i < NUMBER_OF_DEVICE_MENU_ITEMS; i++) {
            QTreeWidgetItem *child = new QTreeWidgetItem(deviceChild);
            child->setText(0, VCdeviceMenu[i].name);

            CustomSlider *slider = new CustomSlider(my_tree, 0, i);
            //QProgressBar *slider = new QProgressBar(my_tree);
            uint8_t max = VCdeviceMenu[i].max;
            if (VCdeviceMenu[i].sublist == MIDI_PORT_SUBLIST1) max = number_of_midi_ports;
            slider->setRange( VCdeviceMenu[i].min, max);
            slider->setValue(Device[d]->get_setting(VCdeviceMenu[i].parameter));
            my_tree->setItemWidget(child, 4, slider);
            connect(slider, SIGNAL(new_value(int, int, int)), this, SLOT(deviceSettingChanged(int, int, int)));

            if (VCdeviceMenu[i].type == OPTION) {
                CustomComboBox *comboBox = new CustomComboBox(my_tree, d, VCdeviceMenu[i].parameter);
                if (VCdeviceMenu[i].sublist > 0) { // Fill combobox with items from the sublist
                    if (VCdeviceMenu[i].sublist == PAGE_SUBLIST) { // Special sublist: read from page
                        comboBox->addItem("---"); // First item is "no page selected"
                        VCd->fillPageComboBox(comboBox);
                        VCd->fillFixedPageComboBox(comboBox);
                        connect(comboBox, SIGNAL(new_value(int, int, int)), this, SLOT(devicePageSettingChanged(int, int, int)));
                        int pageNumber = Device[d]->get_setting(VCdeviceMenu[i].parameter);
                        comboBox->setCurrentIndex(VCd->indexFromValue(TYPE_PAGE, pageNumber));
                        slider->setRange(0, VCd->indexFromValue(TYPE_PAGE, last_fixed_cmd_page));
                        slider->setValue(VCd->indexFromValue(TYPE_PAGE, pageNumber));
                    }
                    else if (VCdeviceMenu[i].sublist == MIDI_PORT_SUBLIST1) {
                        for (int p = 0; p < number_of_midi_ports; p++)
                            comboBox->addItem(midi_port_names[p]);
                        comboBox->setCurrentIndex(Device[d]->get_setting(VCdeviceMenu[i].parameter));
                        connect(comboBox, SIGNAL(new_value(int, int, int)), this, SLOT(deviceSettingChanged(int, int, int)));
                        slider->setRange(0, number_of_midi_ports - 1);
                        slider->setValue(Device[d]->get_setting(VCdeviceMenu[i].parameter));
                    }
                    else if (VCdeviceMenu[i].sublist == TYPE_MODE_SUBLIST) {
                        for(int j = 0; j < Device[d]->get_number_of_dev_types(); j++) {
                            comboBox->addItem(Device[d]->get_dev_type_name(j));
                        }
                        comboBox->setCurrentIndex(Device[d]->get_setting(VCdeviceMenu[i].parameter));
                        connect(comboBox, SIGNAL(new_value(int, int, int)), this, SLOT(deviceSettingChanged(int, int, int)));
                        slider->setRange(0, Device[d]->get_number_of_dev_types() - 1);
                        slider->setValue(Device[d]->get_setting(VCdeviceMenu[i].parameter));
                    }
                    else {
                        int number_of_items = VCdeviceMenu[i].max - VCdeviceMenu[i].min + 1;
                        for (int j = 0; j < number_of_items; j++)
                            comboBox->addItem(menu_sublist.at(j + VCdeviceMenu[i].sublist - 1));
                        comboBox->setCurrentIndex(Device[d]->get_setting(VCdeviceMenu[i].parameter));
                        connect(comboBox, SIGNAL(new_value(int, int, int)), this, SLOT(deviceSettingChanged(int, int, int)));
                    }
                }
                my_tree->setItemWidget(child, 2, comboBox);
                connect(comboBox,SIGNAL(currentIndexChanged(int)), slider, SLOT(setValue(int)));
                connect(slider, SIGNAL(valueChanged(int)), comboBox, SLOT(setCurrentIndex(int)));
            }
            if (VCdeviceMenu[i].type == VALUE) {
                child->setText(0, VCdeviceMenu[i].name + " (" + QString::number(VCdeviceMenu[i].min) + " - " + QString::number(VCdeviceMenu[i].max) + ")");
                CustomSpinBox *spinBox = new CustomSpinBox(my_tree, d, VCdeviceMenu[i].parameter);
                spinBox->setRange(VCdeviceMenu[i].min, VCdeviceMenu[i].max);
                spinBox->setValue(Device[d]->get_setting(VCdeviceMenu[i].parameter));
                my_tree->setItemWidget(child, 2, spinBox);
                connect(spinBox, SIGNAL(new_value(int, int, int)), this, SLOT(deviceSettingChanged(int, int, int)));
                connect(spinBox, SIGNAL(valueChanged(int)), slider, SLOT(setValue(int)));
                connect(slider, SIGNAL(valueChanged(int)), spinBox, SLOT(setValue(int)));
            }
            //deviceChild->addChild(child);
        }
    }
}

void VCdevices::updateTreeWidget(QTreeWidget *my_tree, VCcommands *VCd)
{
    QTreeWidgetItem *parent = findTopLevelItemByName(my_tree, "Device Settings");
    if (parent) {
        for (int d = 0; d < NUMBER_OF_DEVICES; d++) {
            QTreeWidgetItem *deviceChild = parent->child(d);
            deviceChild->setText(0, Device[d]->full_device_name);

            for (int i = 0; i < NUMBER_OF_DEVICE_MENU_ITEMS; i++) {
                QTreeWidgetItem *child = deviceChild->child(i);

                // Update CustomSlider value
                CustomSlider *slider = dynamic_cast<CustomSlider*>(my_tree->itemWidget(child, 4));
                if (slider) {
                    uint8_t max = VCdeviceMenu[i].max;
                    if (VCdeviceMenu[i].sublist == MIDI_PORT_SUBLIST1) max = number_of_midi_ports;
                    slider->setRange(VCdeviceMenu[i].min, max);
                    slider->setValue(Device[d]->get_setting(VCdeviceMenu[i].parameter));
                }

                // Update CustomComboBox current item (if applicable)
                if (VCdeviceMenu[i].type == OPTION) {
                    CustomComboBox *comboBox = dynamic_cast<CustomComboBox*>(my_tree->itemWidget(child, 2));
                    if (comboBox) {
                        comboBox->clear(); // Clear existing items
                        if (VCdeviceMenu[i].sublist == PAGE_SUBLIST) {
                            comboBox->addItem("---"); // First item is "no page selected"
                            VCd->fillPageComboBox(comboBox);
                            VCd->fillFixedPageComboBox(comboBox);
                            int pageNumber = Device[d]->get_setting(VCdeviceMenu[i].parameter);
                            comboBox->setCurrentIndex(VCd->indexFromValue(TYPE_PAGE, pageNumber));
                            slider->setRange(0, VCd->indexFromValue(TYPE_PAGE, last_fixed_cmd_page));
                            slider->setValue(VCd->indexFromValue(TYPE_PAGE, pageNumber));
                        }
                        else if (VCdeviceMenu[i].sublist == MIDI_PORT_SUBLIST1) {
                            for (int p = 0; p < number_of_midi_ports; p++)
                                comboBox->addItem(midi_port_names[p]);
                            comboBox->setCurrentIndex(Device[d]->get_setting(VCdeviceMenu[i].parameter));
                            slider->setRange(0, number_of_midi_ports - 1);
                            slider->setValue(Device[d]->get_setting(VCdeviceMenu[i].parameter));
                        }
                        else if (VCdeviceMenu[i].sublist == TYPE_MODE_SUBLIST) {
                            for(int j = 0; j < Device[d]->get_number_of_dev_types(); j++) {
                                comboBox->addItem(Device[d]->get_dev_type_name(j));
                            }
                            comboBox->setCurrentIndex(Device[d]->get_setting(VCdeviceMenu[i].parameter));
                            slider->setRange(0, Device[d]->get_number_of_dev_types() - 1);
                            slider->setValue(Device[d]->get_setting(VCdeviceMenu[i].parameter));
                        }
                        else {
                            int number_of_items = VCdeviceMenu[i].max - VCdeviceMenu[i].min + 1;
                            for (int j = 0; j < number_of_items; j++)
                                comboBox->addItem(menu_sublist.at(j + VCdeviceMenu[i].sublist - 1));
                            comboBox->setCurrentIndex(Device[d]->get_setting(VCdeviceMenu[i].parameter));
                        }
                    }
                }

                // Update CustomSpinBox value (if applicable)
                if (VCdeviceMenu[i].type == VALUE) {
                    CustomSpinBox *spinBox = dynamic_cast<CustomSpinBox*>(my_tree->itemWidget(child, 2));
                    if (spinBox) {
                        spinBox->setRange(VCdeviceMenu[i].min, VCdeviceMenu[i].max);
                        spinBox->setValue(Device[d]->get_setting(VCdeviceMenu[i].parameter));
                    }
                }
            }
        }
    }
}

QTreeWidgetItem* VCdevices::findTopLevelItemByName(QTreeWidget* my_tree, const QString &name) {
    int topLevelItemCount = my_tree->topLevelItemCount();
    for (int i = 0; i < topLevelItemCount; i++) {
        QTreeWidgetItem* item = my_tree->topLevelItem(i);
        if (item && item->text(0) == name) {
            return item;
        }
    }
    return nullptr;  // Item not found
}

void VCdevices::read(const QJsonObject &json)
{
    QJsonObject deviceHeader = json["Devices"].toObject();
    for (int d = 0; d < NUMBER_OF_DEVICES; d++) {
        QJsonObject deviceObject = deviceHeader[Device[d]->device_name].toObject();
        if (!deviceObject.isEmpty()) {
            for (int i = 0; i < NUMBER_OF_DEVICE_SETTINGS; i++) {
                QString settingName = Device[d]->get_setting_name(i);
                if (!deviceObject[settingName].isNull()) Device[d]->set_setting(i, deviceObject[settingName].toInt());
            }
        }
    }
}

void VCdevices::write(QJsonObject &json) const
{
    QJsonObject deviceHeader;
    for (int d = 0; d < NUMBER_OF_DEVICES; d++) {
        QJsonObject deviceObject;
        for (int i = 0; i < NUMBER_OF_DEVICE_SETTINGS; i++) {
            QString settingName = Device[d]->get_setting_name(i);
            deviceObject[settingName] = Device[d]->get_setting(i);
            qDebug() << settingName << Device[d]->get_setting(i);
        }
        deviceHeader[Device[d]->device_name] = deviceObject;
    }
    json["Devices"] = deviceHeader;
}

/*void VCdevices::readMidi()
{

}

void VCdevices::receiveDeviceSettings(uint8_t dev, uint8_t *settings)
{
   if (dev >= NUMBER_OF_DEVICES) return;

   for (uint8_t i = 0; i < NUMBER_OF_DEVICE_SETTINGS; i++) {
       Device[dev]->set_setting(i, settings[i + 1]);
   }
   //qDebug() << "Device Settings updated for" << Device[dev]->device_name;
}

uint8_t VCdevices::getDeviceSetting(uint8_t dev, uint8_t variable)
{
    if (dev < NUMBER_OF_DEVICES) return Device[dev]->get_setting(variable);
    else return 0;
}*/


void VCdevices::deviceSettingChanged(int dev, int index, int value)
{
    Device[dev]->set_setting(index, value);
    qDebug() << "Device:" << dev << ", index" << index << ", value:" << value;
}

void VCdevices::devicePageSettingChanged(int dev, int index, int value)
{
    VCcommands v;
    int pageNumber = v.valueFromIndex(TYPE_PAGE, value);
    Device[dev]->set_setting(index, pageNumber);
    qDebug() << "Device:" << dev << ", index" << index << ", value:" << value << ", pageNumber" << pageNumber;
}



void VCdevices::setup_devices()
{
    for (uint8_t d = 0; d < NUMBER_OF_DEVICES; d++) {
      Device[d]->init();
    }
}

QByteArray VCdevices::ReadPatch(int number)
{
    if (number >= MAX_NUMBER_OF_DEVICE_PRESETS) return 0;
    QByteArray patch;
    for (int i = 0; i < VC_PATCH_SIZE; i++) {
        patch.append(Device_patches[number][i]);
    }
    return patch;
}

QString VCdevices::ReadPatchStringForListWidget(int number, int type)
{
    QString line;
    int my_index;
    if (type & BASS_MODE_NUMBER_OFFSET) number |= BASS_MODE_NUMBER_OFFSET;
    uint8_t my_type = getDevicePatchType(number, type);
    uint8_t dev = getDevicePatchDeviceNumber(number, type);
    if (type == 0) {
        line = QString::number(number);
        my_index = number;
    }
    else {
        int n = number - Device[dev]->patch_min_as_stored_on_VC();
        line = Device[dev]->number_format(number);
        my_index = findIndex(type, n);
    }

    line.append(":\t");
    if (my_type == 0) {
        line.append("-empty-");
    }
    else {
        if (dev < NUMBER_OF_DEVICES) {
            line.append(Device[dev]->device_name);
            line.append(" \t");
            line.append(Device[dev]->get_patch_info(my_index));
        }
    }

    return line;
}

int VCdevices::findIndex(int type, int patch_no)
{
    if (type == 0) return patch_no;
    if (type & BASS_MODE_NUMBER_OFFSET) patch_no |= BASS_MODE_NUMBER_OFFSET;
    for (int i = 0; i < MAX_NUMBER_OF_DEVICE_PRESETS; i++) {
        int n = (Device_patches[i][1] << 8) + Device_patches[i][2];
        if ((Device_patches[i][0] == (type & 0xFF)) && (n == patch_no)) return i;
    }
    return PATCH_INDEX_NOT_FOUND;
}

int VCdevices::newIndex()
{
    for (int i = 0; i < MAX_NUMBER_OF_DEVICE_PRESETS; i++) {
        if (Device_patches[i][0] == 0) return i;
    }
    return PATCH_INDEX_NOT_FOUND;
}

int VCdevices::numberOfPatches()
{
    int count = 0;
    for (int i = 0; i < MAX_NUMBER_OF_DEVICE_PRESETS; i++) {
        if (Device_patches[i][0] > 0) count++;
    }
    return count;
}

int VCdevices::indexFromMode(int type, int number)
{
    if (type == 0) return number;
    return findIndex(type, number);

}

int VCdevices::getDevicePatchDeviceNumber(int number, int type)
{
    if (type == 0) return Device_patches[number][0] - 1;
    else return (type & 0xFF) - 1;
}

int VCdevices::getDevicePatchType(int number, int type)
{
    if (type == 0) return Device_patches[number][0];
    uint8_t dev = (type &= 0xFF) - 1;
    if (type & BASS_MODE_NUMBER_OFFSET) number |= BASS_MODE_NUMBER_OFFSET;
    if (findIndex(type, number - Device[dev]->patch_min_as_stored_on_VC()) == PATCH_INDEX_NOT_FOUND) return 0;
    else return type;
}

int VCdevices::getDevicePatchNumber(int number, int type)
{
    if (type == 0) return (Device_patches[number][1] << 8) + Device_patches[number][2];
    if (type & BASS_MODE_NUMBER_OFFSET) return number | BASS_MODE_NUMBER_OFFSET;
    return number;
}


void VCdevices::WritePatch(int type, int number, QByteArray patch)
{
    if (number >= MAX_NUMBER_OF_DEVICE_PRESETS) return;
    if (patch.size() > VC_PATCH_SIZE) return;
    int index = indexFromMode(type, number);
    if (index == PATCH_INDEX_NOT_FOUND) return;
    for (int i = 0; i < VC_PATCH_SIZE; i++) {
        Device_patches[index][i] = patch[i];
    }
}

void VCdevices::InitializePatchArea()
{
    for (int p = 0; p < MAX_NUMBER_OF_DEVICE_PRESETS; p++) {
        for (int i = 0; i < VC_PATCH_SIZE; i++) {
            Device_patches[p][i] = 0;
        }
    }
}


void VCdevices::readPatchData(int patch_no, const QJsonObject &json, int my_type)
{
    if (my_type > 0) {
        if (my_type & BASS_MODE_NUMBER_OFFSET) patch_no |= BASS_MODE_NUMBER_OFFSET;
        int index = findIndex(my_type, patch_no);
        if (index == PATCH_INDEX_NOT_FOUND) index = newIndex();
        if (index == PATCH_INDEX_NOT_FOUND) return; // Out of memory

        uint8_t dev = my_type - 1;
        if (dev < NUMBER_OF_DEVICES) Device[dev]->readPatchData(index, patch_no, json);
    }
}

void VCdevices::readAllLegacyKatana(const QJsonObject &json)
{
    QJsonObject allPatches = json["KTN Patches"].toObject();
    for (int p = 0; p < MAX_NUMBER_OF_DEVICE_PRESETS; p++) {
        QJsonObject patchObject = allPatches["KTN_Patch_" + QString::number(p)].toObject();
        if (!patchObject.isEmpty()) {
            readPatchData(p, patchObject, KTN + 1); // Read existing patch data
        }
    }
}

void VCdevices::writePatchData(int patch_no, QJsonObject &json) const
{
    uint8_t my_type = Device_patches[patch_no][0];
    if (my_type > 0) {
        uint8_t dev = (my_type & 0xFF) - 1;
        if (dev < NUMBER_OF_DEVICES) Device[dev]->writePatchData(patch_no, json);
    }
}

void VCdevices::swapPatch(int patch_no1, int patch_no2, int type)
{
    if (patch_no1 == patch_no2) return;
    if (type == EXT_SETLIST_TYPE) {
            patch_no1++;
            patch_no2++;
    }

    if (type & BASS_MODE_NUMBER_OFFSET) {
        patch_no1 |= BASS_MODE_NUMBER_OFFSET;
        patch_no2 |= BASS_MODE_NUMBER_OFFSET;
    }

    int index1 = findIndex(type, patch_no1);
    int index2 = findIndex(type, patch_no2);

    if (index1 != PATCH_INDEX_NOT_FOUND) {
        Device_patches[index1][1] = patch_no2 >> 8;
        Device_patches[index1][2] = patch_no2 & 0xFF;
    }
    if (index2 != PATCH_INDEX_NOT_FOUND) {
        Device_patches[index2][1] = patch_no1 >> 8;
        Device_patches[index2][2] = patch_no1 & 0xFF;
    }
}

void VCdevices::movePatch(int source_patch, int dest_patch, int type)
{
    if (type == 0) return;
    if (dest_patch > source_patch) {
        for (int i = source_patch; i < dest_patch; i++) {
            swapPatch(i, i + 1, type);
        }
    }
    if (dest_patch < source_patch) {
        for (int i = source_patch; i --> dest_patch;) {
            swapPatch(i, i + 1, type);
        }
    }
}

void VCdevices::pastePatch(int number, int type)
{
    if (type == 0) return;
    if (type == EXT_SETLIST_TYPE) number++;

    if (!copyBufferFilled) return;
    int len = Patch_copy_buffer.length() / VC_PATCH_SIZE;
    qDebug() << "Copy buffer length:" << len;

    for (int p = 0; p < len; p++) {
        int buffer_index = p * VC_PATCH_SIZE;
        if (Patch_copy_buffer.at(buffer_index) == (type & 0xFF)) {

            if (type & BASS_MODE_NUMBER_OFFSET) number |= BASS_MODE_NUMBER_OFFSET;

            int index = findIndex(type, number);
            if (index == PATCH_INDEX_NOT_FOUND) index = newIndex();
            if (index == PATCH_INDEX_NOT_FOUND) return; // Out of memory

            Device_patches[index][0] = Patch_copy_buffer.at(buffer_index);
            Device_patches[index][1] = number >> 8;
            Device_patches[index][2] = number & 0xFF;

            for (int i = 3; i < VC_PATCH_SIZE; i++) {
                Device_patches[index][i] = Patch_copy_buffer.at(buffer_index + i);
            }
            qDebug() << "Pasted patch" << number;
        }
        number++;
    }
}

void VCdevices::clearCopyBuffer()
{
    Patch_copy_buffer.clear();
}

bool VCdevices::checkSelectedTypeMatchesCopyBufferType(uint8_t selected_type)
{
    if (!copyBufferFilled) return false;
    return (Patch_copy_buffer.at(0) == selected_type);
}

void VCdevices::initializePatch(int number, int type)
{
    if (type == EXT_SETLIST_TYPE) number++;
    int index = findIndex(type, number);
    if (index == PATCH_INDEX_NOT_FOUND) return;

    for (int i = 0; i < VC_PATCH_SIZE; i++) {
        Device_patches[index][i] = 0;
    }
}

void VCdevices::copyPatch(int number, int type)
{
    if (type == EXT_SETLIST_TYPE) number++;
    int index = findIndex(type, number);
    if (index == PATCH_INDEX_NOT_FOUND) return;

    for (int i = 0; i < VC_PATCH_SIZE; i++) {
        Patch_copy_buffer.append(Device_patches[index][i]);
    }
    copyBufferFilled = true;
    qDebug() << "Copied patch" << number;
}

