#include "customspinbox.h"

void CustomSpinBox::triggerVariantActivated()
{
    emit new_value(m_device_index, m_parameter_index, value() );
}

CustomSpinBox::CustomSpinBox(QWidget *parent, int myDeviceIndex, int myParameterIndex)
:QSpinBox(parent)
{
    m_device_index = myDeviceIndex;
    m_parameter_index = myParameterIndex;
    connect(this, SIGNAL(textChanged(QString)), this, SLOT(triggerVariantActivated()));
    setAlignment(Qt::AlignHCenter);
    setStyleSheet("padding-left: 3px; padding-right: 5px; padding-top: 0px; padding-bottom: 3px;");
}
