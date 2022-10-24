#ifndef CUSTOMCHECKBOX_H
#define CUSTOMCHECKBOX_H

#include <QCheckbox>

class CustomCheckBox : public QCheckBox
{
    Q_OBJECT

public:
    explicit CustomCheckBox(QWidget *parent = nullptr, uint8_t number = 0);
    
    uint8_t getMyNumber() const;

signals:

private:
    uint8_t my_number;
};

#endif // CUSTOMCHECKBOX_H
