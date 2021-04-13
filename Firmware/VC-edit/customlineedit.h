#ifndef CUSTOMLINEEDIT_H
#define CUSTOMLINEEDIT_H

#include <QLineEdit>

class customLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    customLineEdit(QWidget *parent);

protected:
    void focusInEvent(QFocusEvent *);
};

#endif // CUSTOMLINEEDIT_H
