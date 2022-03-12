#include "customdisplaylabel.h"

#include "VController/globals.h"

customDisplayLabel::customDisplayLabel(QWidget *parent) : QLabel(parent)
{
   //connect(this, SIGNAL(myLabelClicked()), this, SLOT(slotLabelClicked()));
}

void customDisplayLabel::setColour(QColor c)
{
    my_colour = c;
    refresh_label();
}

void customDisplayLabel::setDisplayText(QString line1, QString line2)
{
    my_line1 = line1;
    my_line2 = line2;
    refresh_label();
}

uint8_t customDisplayLabel::switchNumber() const
{
    return m_switchNumber;
}

void customDisplayLabel::setSwitchNumber(const uint8_t &switchNumber)
{
    m_switchNumber = switchNumber;
}

bool customDisplayLabel::event(QEvent *myEvent)
{
    if (myEvent->type() == QEvent :: MouseButtonPress) emit pressed();
    if (myEvent->type() == QEvent :: MouseButtonRelease) emit released();
    return QWidget::event(myEvent);
}


void customDisplayLabel::refresh_label()
{
    if (my_colour == "gray") my_colour = QColor(6, 19, 59);
    QString colourName = my_colour.name();
    QString line1Colour = "white";
    QString line2Colour = "yellow";
    if (my_colour.green() > 0xBF) {
            line1Colour = "black";
            line2Colour = "blue";
    }
    QString styleSheet;
    if (VC_type == VCTOUCH) styleSheet = "QLabel { background-color : " + colourName + "; color : " + line1Colour + "; border-style: solid;border-color: white; border-width: 1px;}";
    else styleSheet = "QLabel { background-color : " + colourName + "; color : " + line1Colour + "; border-style: solid;border-color: black; border-width: 8px;}";
    setStyleSheet(styleSheet);
    //my_line1.resize(16);
    //my_line2.resize(16);
    QString line2string = my_line2;
    if (VC_type == VCTOUCH) {
        line2string = "<font color=\"" + line2Colour + "\">" + line2string + "</font>";
    }
    setText("<html><body><p style=\"line-height:.3\">" + my_line1 +
            "</p><p style=\"line-height:.6\">" + line2string + "</p></body></html>");
}
