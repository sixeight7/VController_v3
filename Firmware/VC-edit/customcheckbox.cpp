#include "customcheckbox.h"

CustomCheckBox::CustomCheckBox(QWidget *parent, int deviceIndex, int parameterIndex)
    : QCheckBox{parent}
{
    m_device_index = deviceIndex;
    m_parameter_index = parameterIndex;
    connect(this, SIGNAL(clicked(bool)), this, SLOT(my_custom_clicked(bool)));
}

void CustomCheckBox::my_custom_clicked(bool state)
{
    emit custom_clicked(m_device_index, m_parameter_index, state);
}

