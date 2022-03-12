#include "customswitch.h"
#include <QKeyEvent>
#include <QDebug>

customSwitch::customSwitch(QWidget *parent) : QPushButton(parent)
{
    Q_UNUSED(parent);

    setFixedSize(switchSize, switchSize);
    updateStyleSheet();
}

uint8_t customSwitch::switchNumber() const
{
    return m_switchNumber;
}

void customSwitch::setSwitchNumber(const uint8_t &switchNumber)
{
    m_switchNumber = switchNumber;
}

void customSwitch::setSwitchSize(const uint8_t &size)
{
    switchSize = size;
    updateStyleSheet();
}

/*void customSwitch::keyPressEvent(QKeyEvent *keyEvent)
{
    QString key = keyEvent->text();
    QString myKey = this->text();
    qDebug() << key << myKey;
    if (key[0] == myKey[0]) emit pressed();
    else QPushButton::keyPressEvent(keyEvent);
}

void customSwitch::keyReleaseEvent(QKeyEvent *keyEvent)
{
    QString key = keyEvent->text();
    QString myKey = this->text();
    if (key[0] == myKey[0]) emit released();
    else QPushButton::keyReleaseEvent(keyEvent);
}*/

void customSwitch::updateStyleSheet()
{
    setStyleSheet("QPushButton{background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                  "stop: 0 white, stop: 1 gray);" "border-style: outset;" "border-color: black;"
                  "border-width: 2px;" "border-radius: " + QString::number(switchSize / 2) + "px;}"
                  "QPushButton:pressed { background-color: white; border-style: inset; }");
}
