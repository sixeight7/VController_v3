#ifndef CUSTOMCHECKBOX_H
#define CUSTOMCHECKBOX_H

#include <QCheckBox>

class CustomCheckBox : public QCheckBox
{
    Q_OBJECT

public:
    explicit CustomCheckBox(QWidget *parent = nullptr, int deviceIndex = 0, int parameterIndex = 0);

signals:
    void custom_clicked(int, int, bool);

private slots:
    void my_custom_clicked(bool);

private:
    int m_device_index;
    int m_parameter_index;
};

#endif // CUSTOMCHECKBOX_H
