#include "customled.h"
#include <QPainter>
#include <QBrush>
#include <QLinearGradient>

CustomLED::CustomLED(QWidget *parent)
    : QToolButton(parent), m_ledColor(QColor(Qt::red)), m_label("label"), m_label2("label2"), iDir(1), m_Width(LED_WIDTH)
{
}

CustomLED::~CustomLED()
{

}

void CustomLED::paintEvent(QPaintEvent * e)
{
    Q_UNUSED(e)

    QFont l_qFont;
    QFontMetrics l_qFontMetrics(l_qFont);

    setFixedWidth(m_Width);
    setFixedHeight(m_Width);
    drawLed();
}


void CustomLED::drawLed()
{
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing, true);

    qreal l_margin = 4;
    qreal l_height = height()-l_margin;
    QRadialGradient gradient(1+l_height/2, l_height/2+l_margin/2, m_Width*3/5);

    // Draw outside
    painter.setBrush(m_ledColor.darker(300));
    painter.setPen( Qt::NoPen );
    painter.drawEllipse(1,static_cast<int>(l_margin/2),static_cast<int>(l_height),static_cast<int>(l_height));

    // Draw LED
    gradient.setColorAt(0.0, m_ledColor.lighter(130));
    gradient.setColorAt(0.2, m_ledColor.lighter(130));
    gradient.setColorAt(1, m_ledColor.darker(200));
    painter.setBrush(gradient);
    painter.setPen( Qt::NoPen );
    painter.drawEllipse(1 + height() / 10,static_cast<int>(l_margin/2 + height() / 10),static_cast<int>(height() * 0.8 - l_margin),static_cast<int>(height() * 0.8 - l_margin));

    // Draw flare
    qreal l_highlight = m_Width*2/5;
    QRadialGradient radGrad(1+l_height/2,l_height/2+l_margin/2-m_Width/4, m_Width/4);

    radGrad.setColorAt(0.0, Qt::white);
    radGrad.setColorAt(1.0, m_ledColor);
    painter.setBrush(radGrad);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(static_cast<int>(1+l_height/2-l_highlight/2),static_cast<int>(l_height/2-l_highlight/2+l_margin/2-m_Width/8),static_cast<int>(l_highlight),static_cast<int>(l_highlight-m_Width/8));
}


