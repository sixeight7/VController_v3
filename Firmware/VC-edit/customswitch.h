#ifndef CUSTOMSWITCH_H
#define CUSTOMSWITCH_H

#include <QPushButton>

class customSwitch : public QPushButton
{
        Q_OBJECT
public:
    customSwitch(QWidget *parent);
    uint8_t switchNumber() const;
    void setSwitchNumber(const uint8_t &switchNumber);
    void setSwitchSize (const uint8_t &size);

protected:
    //void keyPressEvent(QKeyEvent*);
    //void keyReleaseEvent(QKeyEvent*);

private:
    void updateStyleSheet();
    uint8_t m_switchNumber;
    uint8_t switchSize = 40;
};

#endif // CUSTOMSWITCH_H
