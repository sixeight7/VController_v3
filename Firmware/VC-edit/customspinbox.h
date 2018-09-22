#ifndef CUSTOMSPINBOX_H
#define CUSTOMSPINBOX_H

// The custom spinbox is used as a delegate in the settings treeWidget and the command tableWidget

#include <QSpinBox>

class CustomSpinBox : public QSpinBox {

    Q_OBJECT

private slots:

    void triggerVariantActivated();

public:

    CustomSpinBox(QWidget *parent, int deviceIndex, int parameterIndex);

signals:

    void new_value(int, int, int);

private:
    int m_device_index;
    int m_parameter_index;

};

#endif // CUSTOMSPINBOX_H
