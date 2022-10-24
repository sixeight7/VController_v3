#ifndef CUSTOMLINEEDIT_H
#define CUSTOMLINEEDIT_H

#include <QLineEdit>

class customLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    customLineEdit(QWidget *parent, int deviceIndex = 0, int parameterIndex = 0);
    uint8_t getDeviceIndex();
    uint8_t getParameterIndex();

protected:
    void focusInEvent(QFocusEvent *);

signals:

    void new_value(int, int, int);

private slots:

    void triggerVariantActivated(int index);

private:
    int m_device_index;
    int m_parameter_index;
};
#endif // CUSTOMLINEEDIT_H
