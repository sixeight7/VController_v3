#include "vcmidiswitchsettings.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QProgressBar>

VCmidiSwitches::VCmidiSwitches(QObject *parent) : QObject(parent)
{
   //setup_devices();
}

void VCmidiSwitches::fillTreeWidget(QTreeWidget *my_tree, VCmidiSwitches *VCm)
{
    QTreeWidgetItem *parent = new QTreeWidgetItem(my_tree);
    parent->setText(0, "MIDI Switch Settings");
    my_tree->addTopLevelItem(parent);

    for (int s = 1; s < NUMBER_OF_MIDI_SWITCHES; s++) {
        QTreeWidgetItem *deviceChild = new QTreeWidgetItem(parent);
        deviceChild->setText(0, getSwitchNameForTree(s));
        //my_tree->addTopLevelItem(deviceChild);
        //parent->addChild(deviceChild);

        for (int i = 0; i < NUMBER_OF_MIDI_SWITCH_MENU_ITEMS; i++) {
            QTreeWidgetItem *child = new QTreeWidgetItem(deviceChild);
            child->setText(0, VCmidiSwitchMenu[i].name);

            CustomSlider *slider = new CustomSlider(my_tree, s, VCmidiSwitchMenu[i].parameter);
            //QProgressBar *slider = new QProgressBar(my_tree);
            slider->setRange( VCmidiSwitchMenu[i].min, VCmidiSwitchMenu[i].max);
            slider->setValue(getMidiSwitchSetting(s,VCmidiSwitchMenu[i].parameter));
            my_tree->setItemWidget(child, 4, slider);
            connect(slider, SIGNAL(new_value(int, int, int)), this, SLOT(midiSwitchSettingChanged(int, int, int)));

            if (VCmidiSwitchMenu[i].type == OPTION) {
                CustomComboBox *comboBox = new CustomComboBox(my_tree, s, VCmidiSwitchMenu[i].parameter);
                if (VCmidiSwitchMenu[i].sublist > 0) { // Fill combobox with items from the sublist
                    {
                        int number_of_items = VCmidiSwitchMenu[i].max - VCmidiSwitchMenu[i].min + 1;
                        for (int j = 0; j < number_of_items; j++)
                            comboBox->addItem(menu_sublist.at(j + VCmidiSwitchMenu[i].sublist - 1));
                        //comboBox->setCurrentIndex(Device[d]->get_setting(VCmidiSwitchMenu[i].parameter));
                        comboBox->setCurrentIndex(getMidiSwitchSetting(s,VCmidiSwitchMenu[i].parameter));
                    }
                    my_tree->setItemWidget(child, 2, comboBox);
                    connect(comboBox, SIGNAL(new_value(int, int, int)), this, SLOT(midiSwitchSettingChanged(int, int, int)));
                    connect(comboBox,SIGNAL(currentIndexChanged(int)), slider, SLOT(setValue(int)));
                    connect(slider, SIGNAL(valueChanged(int)), comboBox, SLOT(setCurrentIndex(int)));
                }
            }
            if (VCmidiSwitchMenu[i].type == VALUE) {
                child->setText(0, VCmidiSwitchMenu[i].name + " (" + QString::number(VCmidiSwitchMenu[i].min) + " - " + QString::number(VCmidiSwitchMenu[i].max) + ")");
                CustomSpinBox *spinBox = new CustomSpinBox(my_tree, s, VCmidiSwitchMenu[i].parameter);
                spinBox->setRange(VCmidiSwitchMenu[i].min, VCmidiSwitchMenu[i].max);
                spinBox->setValue(getMidiSwitchSetting(s,VCmidiSwitchMenu[i].parameter));
                my_tree->setItemWidget(child, 2, spinBox);
                connect(spinBox, SIGNAL(new_value(int, int, int)), this, SLOT(midiSwitchSettingChanged(int, int, int)));
                connect(spinBox, SIGNAL(valueChanged(int)), slider, SLOT(setValue(int)));
                connect(slider, SIGNAL(valueChanged(int)), spinBox, SLOT(setValue(int)));
            }
            //deviceChild->addChild(child);
        }
    }
}

void VCmidiSwitches::read(const QJsonObject &json)
{
    QJsonObject deviceHeader = json["MIDI Switches"].toObject();
    for (int s = 0; s < NUMBER_OF_MIDI_SWITCHES; s++) {
        QJsonObject deviceObject = deviceHeader[getSwitchNameForJson(s)].toObject();
        if (!deviceObject.isEmpty()) {
            for (int i = 0; i < NUMBER_OF_MIDI_SWITCH_MENU_ITEMS; i++) {
                QString settingName = VCmidiSwitchMenu[i].name;
                if (!deviceObject[settingName].isNull()) setMidiSwitchSetting(s, VCmidiSwitchMenu[i].parameter, deviceObject[settingName].toInt());
            }
        }
    }
}

void VCmidiSwitches::write(QJsonObject &json) const
{
    QJsonObject deviceHeader;
    for (int s = 0; s < NUMBER_OF_MIDI_SWITCHES; s++) {
        QJsonObject deviceObject;
        for (int i = 0; i < NUMBER_OF_MIDI_SWITCH_MENU_ITEMS; i++) {
            int value = getMidiSwitchSetting(s,VCmidiSwitchMenu[i].parameter);
            deviceObject[VCmidiSwitchMenu[i].name] = value;
            qDebug() << VCmidiSwitchMenu[i].name << getMidiSwitchSetting(s,VCmidiSwitchMenu[i].parameter);
        }
        deviceHeader[getSwitchNameForJson(s)] = deviceObject;
    }
    json["MIDI Switches"] = deviceHeader;
}

/*void VCmidiSwitches::readMidi()
{

}

void VCmidiSwitches::receiveMidiSwitchSettings(uint8_t sw, uint8_t *switchSettings)
{
   if (sw >= NUMBER_OF_MIDI_SWITCHES) return;

   for (uint8_t i = 0; i < NUMBER_OF_MIDI_SWITCHES; i++) {
       MIDI_switch[i].type = switchSettings[0];
       MIDI_switch[i].port = switchSettings[1];
       MIDI_switch[i].channel = switchSettings[2];
       MIDI_switch[i].cc = switchSettings[3];
   }
   //qDebug() << "Device Settings updated for" << Device[dev]->device_name;
}*/

int VCmidiSwitches::getMidiSwitchSetting(uint8_t sw, uint8_t parameter) const
{
    if (sw < NUMBER_OF_MIDI_SWITCHES) {
      switch (parameter) {
        case 1:
            return MIDI_switch[sw].type;
            break;
        case 2:
            return MIDI_switch[sw].port;
            break;
        case 3:
            return MIDI_switch[sw].channel;
            break;
        case 4:
            return MIDI_switch[sw].cc;
            break;
        default:
            return 0;
          break;
        };
    }
    else return 0;
}

void VCmidiSwitches::setMidiSwitchSetting(uint8_t sw, uint8_t parameter, uint8_t value)
{
    if (sw < NUMBER_OF_MIDI_SWITCHES) {
      switch (parameter) {
        case 1:
            MIDI_switch[sw].type = value;
            break;
        case 2:
            MIDI_switch[sw].port = value;
            break;
        case 3:
            MIDI_switch[sw].channel = value;
            break;
        case 4:
            MIDI_switch[sw].cc = value;
            break;
        default:
            break;
        }
    };
}

QString VCmidiSwitches::getSwitchNameForTree(int sw) const
{
    QString target;
    switch (MIDI_switch[sw].type) {
      case MIDI_SWITCH_OFF:
        target = "";
        break;
      case MIDI_SWITCH_CC_MOMENTARY:
        target = " (CC MOMENTARY #" + QString::number(MIDI_switch[sw].cc) + " - ch:" + QString::number(MIDI_switch[sw].channel) + ")";
        break;
      case MIDI_SWITCH_CC_RANGE:
        target = " (CC RANGE #" + QString::number(MIDI_switch[sw].cc) + " - ch:" + QString::number(MIDI_switch[sw].channel) + ")";
        break;
      case MIDI_SWITCH_CC_TOGGLE:
        target = " (CC TOGGLE #" + QString::number(MIDI_switch[sw].cc) + " - ch:" + QString::number(MIDI_switch[sw].channel) + ")";
        break;
      case MIDI_SWITCH_PC:
        target = " (PC #" + QString::number(MIDI_switch[sw].cc) + " - ch:" + QString::number(MIDI_switch[sw].channel) + ")";
        break;
      default:
        target = "?";
    }

    if ((sw) < switch_name.size()) return switch_name.at(sw) + target;
    return "Switch " + QString::number(sw + 1) + target;
}

QString VCmidiSwitches::getSwitchNameForJson(int sw) const
{
    return "Switch " + QString::number(sw + 1);
}

void VCmidiSwitches::midiSwitchSettingChanged(int sw, int variable, int value)
{
    setMidiSwitchSetting(sw, variable, value);
    //qDebug() << "Switch:" << sw << ", variable" << variable << ", value:" << value;
}


