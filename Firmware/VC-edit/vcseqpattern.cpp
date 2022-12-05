#include "vcseqpattern.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QProgressBar>

VCseqPattern::VCseqPattern(QObject *parent) : QObject(parent)
{

}

void VCseqPattern::fillTreeWidget(QTreeWidget *my_tree, VCseqPattern *VCptrn)
{
    QTreeWidgetItem *parent = new QTreeWidgetItem(my_tree);
    parent->setText(0, "Sequence Patterns");
    my_tree->addTopLevelItem(parent);

    for (int p = 0; p < NUMBER_OF_SEQ_PATTERNS; p++) {
        QTreeWidgetItem *deviceChild = new QTreeWidgetItem(parent);
        deviceChild->setText(0, getPatternNameForTree(p));
        //my_tree->addTopLevelItem(deviceChild);
        //parent->addChild(deviceChild);

        for (int i = 0; i < NUMBER_OF_SEQ_PATTERN_MENU_ITEMS; i++) {
            QTreeWidgetItem *child = new QTreeWidgetItem(deviceChild);
            child->setText(0, VCseqPatternMenu[i].name);

            CustomSlider *slider = new CustomSlider(my_tree, p, VCseqPatternMenu[i].parameter);
            //QProgressBar *slider = new QProgressBar(my_tree);
            slider->setRange( VCseqPatternMenu[i].min, VCseqPatternMenu[i].max);
            slider->setValue(getSeqPatternSetting(p,VCseqPatternMenu[i].parameter));
            my_tree->setItemWidget(child, 4, slider);
            connect(slider, SIGNAL(new_value(int, int, int)), this, SLOT(midiSeqPatternChanged(int, int, int)));

            if (VCseqPatternMenu[i].type == OPTION) {
                CustomComboBox *comboBox = new CustomComboBox(my_tree, p, VCseqPatternMenu[i].parameter);
                if (VCseqPatternMenu[i].sublist > 0) { // Fill combobox with items from the sublist
                    {
                        int number_of_items = VCseqPatternMenu[i].max - VCseqPatternMenu[i].min + 1;
                        for (int j = 0; j < number_of_items; j++)
                            comboBox->addItem(menu_sublist.at(j + VCseqPatternMenu[i].sublist - 1));
                        //comboBox->setCurrentIndex(Device[d]->get_setting(VCseqPatternMenu[i].parameter));
                        comboBox->setCurrentIndex(getSeqPatternSetting(p,VCseqPatternMenu[i].parameter));
                    }
                }
                my_tree->setItemWidget(child, 2, comboBox);
                connect(comboBox, SIGNAL(new_value(int, int, int)), this, SLOT(midiSeqPatternChanged(int, int, int)));
                connect(comboBox,SIGNAL(currentIndexChanged(int)), slider, SLOT(setValue(int)));
                connect(slider, SIGNAL(valueChanged(int)), comboBox, SLOT(setCurrentIndex(int)));
            }
            if (VCseqPatternMenu[i].type == VALUE) {
                child->setText(0, VCseqPatternMenu[i].name + " (" + QString::number(VCseqPatternMenu[i].min) + " - " + QString::number(VCseqPatternMenu[i].max) + ")");
                CustomSpinBox *spinBox = new CustomSpinBox(my_tree, p, VCseqPatternMenu[i].parameter);
                spinBox->setRange(VCseqPatternMenu[i].min, VCseqPatternMenu[i].max);
                spinBox->setValue(getSeqPatternSetting(p,VCseqPatternMenu[i].parameter));
                my_tree->setItemWidget(child, 2, spinBox);
                connect(spinBox, SIGNAL(new_value(int, int, int)), this, SLOT(midiSeqPatternChanged(int, int, int)));
                connect(spinBox, SIGNAL(valueChanged(int)), slider, SLOT(setValue(int)));
                connect(slider, SIGNAL(valueChanged(int)), spinBox, SLOT(setValue(int)));
            }
            //deviceChild->addChild(child);
        }
    }
}

void VCseqPattern::read(const QJsonObject &json)
{
    QJsonObject deviceHeader = json["Sequence Patterns"].toObject();
    for (int p = 0; p < NUMBER_OF_SEQ_PATTERNS; p++) {
        QJsonObject deviceObject = deviceHeader[getPatternNameForJson(p)].toObject();
        if (!deviceObject.isEmpty()) {
            for (int i = 0; i < NUMBER_OF_SEQ_PATTERN_MENU_ITEMS; i++) {
                QString settingName = VCseqPatternMenu[i].name;
                if (!deviceObject[settingName].isNull()) setSeqPatternSetting(p, VCseqPatternMenu[i].parameter, deviceObject[settingName].toInt());
            }
        }
    }
}

void VCseqPattern::write(QJsonObject &json) const
{
    QJsonObject deviceHeader;
    for (int p = 0; p < NUMBER_OF_SEQ_PATTERNS; p++) {
        QJsonObject deviceObject;
        for (int i = 0; i < NUMBER_OF_SEQ_PATTERN_MENU_ITEMS; i++) {
            int value = getSeqPatternSetting(p,VCseqPatternMenu[i].parameter);
            deviceObject[VCseqPatternMenu[i].name] = value;
            qDebug() << VCseqPatternMenu[i].name << getSeqPatternSetting(p,VCseqPatternMenu[i].parameter);
        }
        deviceHeader[getPatternNameForJson(p)] = deviceObject;
    }
    json["Sequence Patterns"] = deviceHeader;
}

int VCseqPattern::getSeqPatternSetting(uint8_t pattern, uint8_t parameter) const
{
  if ((pattern < NUMBER_OF_SEQ_PATTERNS) && (parameter > 0)) {
      return MIDI_seq_pattern[pattern][parameter - 1];
  }
  return 0;
}

void VCseqPattern::setSeqPatternSetting(uint8_t pattern, uint8_t parameter, uint8_t value)
{
  if (parameter > 0) {
    MIDI_seq_pattern[pattern][parameter - 1] = value;
  }
}

void VCseqPattern::midiSeqPatternChanged(int pattern, int parameter, int value)
{
    setSeqPatternSetting(pattern, parameter, value);
}

QString VCseqPattern::getPatternNameForTree(int pattern) const
{
  return "Pattern " + QString::number(pattern + 1);
}

QString VCseqPattern::getPatternNameForJson(int pattern) const
{
   return "Seq Pattern " + QString::number(pattern + 1);
}
