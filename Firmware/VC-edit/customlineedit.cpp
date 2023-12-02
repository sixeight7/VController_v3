#include "customlineedit.h"
#include "QtWidgets/qapplication.h"
#include <QTimer>
#include <QKeyEvent>

customLineEdit::customLineEdit(QWidget *parent, int myDeviceIdex, int myParameterIndex)
:QLineEdit(parent)
{
    m_device_index = myDeviceIdex;
    m_parameter_index = myParameterIndex;
    //connect(this, SIGNAL(activated(int)), this, SLOT(triggerVariantActivated(int)));
    connect(this, &customLineEdit::textChanged, this, &customLineEdit::handleTextChanged);
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

void customLineEdit::keyPressEvent(QKeyEvent *keyEvent)
{
    if ((keyEvent->key() == Qt::Key_Up) || (keyEvent->key() == Qt::Key_Down)) {
        keyEvent->ignore(); // So the parent will pick it up
    }
    else if ((keyEvent->key() == Qt::Key_Return) || (keyEvent->key() == Qt::Key_Enter)) {
        QKeyEvent event(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
        QApplication::sendEvent(parent(), &event);
    }
    else QLineEdit::keyPressEvent(keyEvent);
}

/*void customLineEdit::triggerVariantActivated(int index)
{

}*/

void customLineEdit::handleTextChanged(const QString &text)
{
  emit new_text(m_device_index, m_parameter_index, this->text() );
}
