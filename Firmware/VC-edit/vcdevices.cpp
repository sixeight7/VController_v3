#include "vcdevices.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QProgressBar>

VCdevices::VCdevices(QObject *parent) : QObject(parent)
{
   setup_devices();
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
            slider->setRange( VCdeviceMenu[i].min, VCdeviceMenu[i].max);
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
                        int pageNumber = Device[d]->get_setting(VCdeviceMenu[i].parameter);
                        comboBox->setCurrentIndex(VCd->indexFromValue(TYPE_PAGE, pageNumber));
                        slider->setRange(0, VCd->indexFromValue(TYPE_PAGE, LAST_FIXED_CMD_PAGE));
                        slider->setValue(VCd->indexFromValue(TYPE_PAGE, pageNumber));
                    }
                    else {
                        int number_of_items = VCdeviceMenu[i].max - VCdeviceMenu[i].min + 1;
                        for (int j = 0; j < number_of_items; j++)
                            comboBox->addItem(menu_sublist.at(j + VCdeviceMenu[i].sublist - 1));
                        comboBox->setCurrentIndex(Device[d]->get_setting(VCdeviceMenu[i].parameter));
                    }
                    my_tree->setItemWidget(child, 2, comboBox);
                    connect(comboBox, SIGNAL(new_value(int, int, int)), this, SLOT(devicePageSettingChanged(int, int, int)));
                    connect(comboBox,SIGNAL(currentIndexChanged(int)), slider, SLOT(setValue(int)));
                    connect(slider, SIGNAL(valueChanged(int)), comboBox, SLOT(setCurrentIndex(int)));
                }               
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

void VCdevices::readMidi()
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
}


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
