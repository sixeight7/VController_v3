#include "vcsettings.h"
#include "midi.h"
#include "VController/globaldevices.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>

VCsettings::VCsettings(QObject *parent) : QObject(parent)
{

}

void VCsettings::fillTreeWidget(QTreeWidget *my_tree)
{
  QTreeWidgetItem* parent = NULL;

  for (int i = 0; i < NUMBER_OF_SETTINGS_MENU_ITEMS; i++) {
      if (VCsettingMenu[i].type == HEADER) {
          parent = new QTreeWidgetItem(my_tree);
          parent->setText(0, VCsettingMenu[i].name);
          my_tree->addTopLevelItem(parent);
      }
      else {
          if (parent) {
            QTreeWidgetItem *child = new QTreeWidgetItem(parent);

            CustomSlider *slider = new CustomSlider(my_tree, 0, i);
            //QProgressBar *slider = new QProgressBar(my_tree);
            slider->setRange(VCsettingMenu[i].min, VCsettingMenu[i].max);
            slider->setValue((int)(size_t)*VCsettingMenu[i].value);
            my_tree->setItemWidget(child, 4, slider);
            connect(slider, SIGNAL(new_value(int, int, int)), this, SLOT(settingChanged(int, int, int)));

            if (VCsettingMenu[i].type == OPTION) {
                child->setText(0, VCsettingMenu[i].name);
                CustomComboBox *comboBox = new CustomComboBox(my_tree, 0, i);
                if (VCsettingMenu[i].sublist > 0) { // Fill combobox with items from
                    if (VCsettingMenu[i].sublist == DEVICE_SUBLIST) {
                        for(int d = 0; d < NUMBER_OF_DEVICES; d++)
                            comboBox->addItem(Device[d]->full_device_name);
                    }
                    else if (VCsettingMenu[i].sublist == MIDI_PORT_SUBLIST) {
                        comboBox->addItem("OFF");
                        for (int p = 0; p < number_of_midi_ports + VCsettingMenu[i].max; p++) {
                            comboBox->addItem(midi_port_names[p]);
                        }
                    }
                    else {
                        int number_of_items = VCsettingMenu[i].max - VCsettingMenu[i].min + 1;
                        for (int j = 0; j < number_of_items; j++)
                            comboBox->addItem(menu_sublist.at(j + VCsettingMenu[i].sublist - 1));
                    }
                    comboBox->setCurrentIndex(*VCsettingMenu[i].value);
                }
                my_tree->setItemWidget(child, 2, comboBox);
                connect(comboBox, SIGNAL(new_value(int, int, int)), this, SLOT(settingChanged(int, int, int)));
                connect(comboBox,SIGNAL(currentIndexChanged(int)), slider, SLOT(setValue(int)));
                connect(slider, SIGNAL(valueChanged(int)), comboBox, SLOT(setCurrentIndex(int)));
            }
            if (VCsettingMenu[i].type == VALUE) {
                child->setText(0, VCsettingMenu[i].name + " (" + QString::number(VCsettingMenu[i].min) + " - " + QString::number(VCsettingMenu[i].max) + ")");
                CustomSpinBox *spinBox = new CustomSpinBox(my_tree, 0, i);
                spinBox->setRange(VCsettingMenu[i].min, VCsettingMenu[i].max);
                spinBox->setValue((int)(size_t)*VCsettingMenu[i].value);
                my_tree->setItemWidget(child, 2, spinBox);
                connect(spinBox, SIGNAL(new_value(int, int, int)), this, SLOT(settingChanged(int, int, int)));
                connect(spinBox, SIGNAL(valueChanged(int)), slider, SLOT(setValue(int)));
                connect(slider, SIGNAL(valueChanged(int)), spinBox, SLOT(setValue(int)));
            }
            parent->addChild(child);
          }
      }
  }

}

void VCsettings::settingChanged(int, int index, int value) // Called when a comboBox or spinBox in tree is updated
{
    *VCsettingMenu[index].value = value;
    //qDebug() << index << value;
}


void VCsettings::showData() // For debugging
{
    for (int i = 0; i < NUMBER_OF_SETTINGS_MENU_ITEMS; i++) {
        if (VCsettingMenu[i].value != nullptr)
            qDebug() << VCsettingMenu[i].name << *VCsettingMenu[i].value ;
    }
}

void VCsettings::read(const QJsonObject &json)
{
    QJsonObject settingObject = json["Settings"].toObject();
    for (int i = 0; i < NUMBER_OF_SETTINGS_MENU_ITEMS; i++) {
        if ((VCsettingMenu[i].value != nullptr) || (!settingObject[VCsettingMenu[i].name].isNull()))
            *VCsettingMenu[i].value = settingObject[VCsettingMenu[i].name].toInt();
    }

    QJsonArray calibrationArray = json["Expression pedal calibration"].toArray();
    for (int i = 0; i < calibrationArray.size(); i++) {
        QJsonObject calibrationObject = calibrationArray[i].toObject();
        if (!calibrationObject.isEmpty()){
            Setting.exp_max[i] = calibrationObject["max"].toInt();
            Setting.exp_min[i] = calibrationObject["min"].toInt();
        }
    }
}

void VCsettings::write(QJsonObject &json) const
{
    QJsonObject settingObject;
    for (int i = 0; i < NUMBER_OF_SETTINGS_MENU_ITEMS; i++) {
        if ((VCsettingMenu[i].type != HEADER) && (VCsettingMenu[i].value != nullptr)) {
            int value = *VCsettingMenu[i].value;
            settingObject[VCsettingMenu[i].name] = value; //Adds the value to the current header
        }
    }
    json["Settings"] = settingObject;

    QJsonArray calibrationArray;
    for (int i = 0; i < 4; i++) {
        QJsonObject calibrationObject;
        calibrationObject["max"] = Setting.exp_max[i];
        calibrationObject["min"] = Setting.exp_min[i];
        calibrationArray.append(calibrationObject);
    }
    json["Expression pedal calibration"] = calibrationArray;
}


