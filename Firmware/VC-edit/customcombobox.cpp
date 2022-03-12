#include "customcombobox.h"
#include <QLineEdit>

void CustomComboBox::triggerVariantActivated(int)
{
    emit new_value(m_device_index, m_parameter_index, currentIndex() );
}

CustomComboBox::CustomComboBox(QWidget *parent, int myDeviceIdex, int myParameterIndex)
:QComboBox(parent)
{
    m_device_index = myDeviceIdex;
    m_parameter_index = myParameterIndex;
    connect(this, SIGNAL(activated(int)), this, SLOT(triggerVariantActivated(int)));

    setEditable(true);
    lineEdit()->setAlignment(Qt::AlignCenter);
    //setStyleSheet("padding-left: 15px; padding-right: 15px; padding-top: 0px; padding-bottom: 3px;");
}
