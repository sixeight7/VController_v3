#ifndef CUSTOMSLIDER_H
#define CUSTOMSLIDER_H

// The custom slider is used as a delegate in the settings treeWidget and the command tableWidget.
// It has been styled with a custom stylesheet

#include <QSlider>

class CustomSlider : public QSlider
{
        Q_OBJECT

    private slots:

        void triggerVariantSliderReleased();

    public:

        CustomSlider(QWidget *parent, int deviceIndex, int parameterIndex);

signals:
        void new_value(int, int, int);

    private:
        int m_device_index;
        int m_parameter_index;

};
#endif // CUSTOMSLIDER_H
