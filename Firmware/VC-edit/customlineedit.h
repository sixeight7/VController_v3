#ifndef CUSTOMLINEEDIT_H
#define CUSTOMLINEEDIT_H

#include <QLineEdit>
#include <QtGui>

class customLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    customLineEdit(QWidget *parent, int deviceIndex = 0, int parameterIndex = 0);
    uint8_t getDeviceIndex();
    uint8_t getParameterIndex();

protected:
    void focusInEvent(QFocusEvent *);
    void keyPressEvent(QKeyEvent*);

signals:

    void new_text(int, int, const QString &);

private slots:

    //void triggerVariantActivated(int index);
    void handleTextChanged(const QString &text);

private:
    int m_device_index;
    int m_parameter_index;
};
#endif // CUSTOMLINEEDIT_H
