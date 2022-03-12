#include "customslider.h"
#include <QDebug>

void CustomSlider::triggerVariantSliderReleased()
{
    emit new_value(m_device_index, m_parameter_index, value() );
}

CustomSlider::CustomSlider(QWidget *parent, int myDeviceIdex, int myParameterIndex)
:QSlider(parent)
{
    m_device_index = myDeviceIdex;
    m_parameter_index = myParameterIndex;
    connect(this, SIGNAL(sliderReleased()), this, SLOT(triggerVariantSliderReleased()));

    setOrientation(Qt::Horizontal); // Must set this, otherwise slider does not work!!
    setMaximumHeight(25);
    //setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setStyleSheet("QSlider::groove:horizontal {border: 1px solid #bbb; background: white; height: 10px; border-radius: 4px;}"
                  "QSlider::sub-page:horizontal {background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,  stop: 0 #06e, stop: 1 #0bf);background: qlineargradient(x1: 0, y1: 0.2, x2: 1, y2: 1, stop: 0 #0bf, stop: 1 #05f); border: 1px solid #777; height: 10px; border-radius: 4px; }"
                  "QSlider::add-page:horizontal { background: #fff; border: 1px solid #777; height: 10px; border-radius: 4px;}"
                  "QSlider::handle:horizontal { background: qlineargradient(x1:0, y1:0, x2:1, y2:1,  stop:0 #eee, stop:1 #ccc); border: 1px solid #777;  width: 13px; margin-top: -2px; margin-bottom: -2px; border-radius: 4px;}"
                  "QSlider::handle:horizontal:hover {background: qlineargradient(x1:0, y1:0, x2:1, y2:1,  stop:0 #fff, stop:1 #ddd);border: 1px solid #444;border-radius: 4px; }"
                  "QSlider::sub-page:horizontal:disabled { background: #bbb; border-color: #999; }"
                  "QSlider::add-page:horizontal:disabled { background: #eee;  border-color: #999; }"
                  "QSlider::handle:horizontal:disabled { background: #eee;border: 1px solid #aaa;border-radius: 4px;}");
}
