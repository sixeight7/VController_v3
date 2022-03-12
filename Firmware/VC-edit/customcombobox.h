#ifndef CUSTOMCOMBOBOX_H
#define CUSTOMCOMBOBOX_H

// The custom combobox is used as a delegate in the settings treeWidget and the command tableWidget

#include <QComboBox>

class CustomComboBox : public QComboBox {

    Q_OBJECT

private slots:

    void triggerVariantActivated(int index);

public:

    CustomComboBox(QWidget *parent, int deviceIndex, int parameterIndex);

signals:

    void new_value(int, int, int);

private:
    int m_device_index;
    int m_parameter_index;

};

#endif // CUSTOMCOMBOBOX_H
