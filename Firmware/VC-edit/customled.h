#ifndef MYTOOLBUTTON_H
#define MYTOOLBUTTON_H

#include <QObject>
#include <QToolButton>
#include <QTimer>
#include <QTime>
#define LED_WIDTH 32

class CustomLED : public QToolButton
{
public:
    CustomLED(QWidget *parent=NULL);
    ~CustomLED();

    inline void setLedColor(QColor c) { m_ledColor = c; }
    inline void setLedWidth(int w) { m_Width = w; }
protected:
    void paintEvent(QPaintEvent *event);

private:
    void drawLed();
    QColor m_ledColor;
    QString m_label, m_label2;
    int iDir, m_Width;
};


#endif // MYTOOLBUTTON_H
