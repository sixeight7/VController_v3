#include "customlineedit.h"
#include <QTimer>

customLineEdit::customLineEdit(QWidget *parent, int myDeviceIdex, int myParameterIndex)
:QLineEdit(parent)
{
    m_device_index = myDeviceIdex;
    m_parameter_index = myParameterIndex;
    connect(this, SIGNAL(activated(int)), this, SLOT(triggerVariantActivated(int)));
}

uint8_t customLineEdit::getDeviceIndex()
{
    return m_device_index;
}

uint8_t customLineEdit::getParameterIndex()
{
    return m_parameter_index;
}

void customLineEdit::focusInEvent(QFocusEvent *)
{
    QTimer::singleShot(0, [this](){
            selectAll();
    });
}

void customLineEdit::triggerVariantActivated(int index)
{
    emit new_value(m_device_index, m_parameter_index, index );
}
