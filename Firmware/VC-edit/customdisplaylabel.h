#ifndef CUSTOMDISPLAYLABEL_H
#define CUSTOMDISPLAYLABEL_H

#include <QLabel>
#include <QColor>

class customDisplayLabel : public QLabel
{
    Q_OBJECT
public:
    explicit customDisplayLabel(QWidget *parent = nullptr);
    void setColour(QColor c);
    void setDisplayText(QString line1, QString line2);
    uint8_t switchNumber() const;
    void setSwitchNumber(const uint8_t &switchNumber);

signals:
    void pressed();
    void released();

protected:
    bool event(QEvent *myEvent);

private:
    void refresh_label();
    QColor my_colour = QColor(6, 19, 59);
    QString my_line1 = "";
    QString my_line2 = "";
    uint8_t m_switchNumber;
};

#endif // CUSTOMDISPLAYLABEL_H
