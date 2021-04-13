#include "customlineedit.h"
#include <QTimer>

customLineEdit::customLineEdit(QWidget *parent)
:QLineEdit(parent)
{

}

void customLineEdit::focusInEvent(QFocusEvent *)
{
    QTimer::singleShot(0, [this](){
            selectAll();
        });
}
