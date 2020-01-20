#include "customlistwidget.h"

#include <QDebug>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>

customListWidget::customListWidget(QWidget *parent) : QListWidget(parent)
{
    Q_UNUSED(parent);

    setAcceptDrops(true);
    setDragEnabled(true);

    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setDefaultDropAction(Qt::CopyAction);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn); // Does not seem to make any difference on Mac

    viewport()->installEventFilter(this);
}

void customListWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void customListWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void customListWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
    //setDefaultDropAction(Qt::CopyAction); //
}

void customListWidget::dropEvent(QDropEvent *event)
{
    customListWidget *source = (customListWidget*) event->source();

    int sourceRow = source->currentRow();

    if (event->source() == this) {
        if ((this->count() > 1) && (item(0)->text() != "(default page)")) {
                event->setDropAction(Qt::MoveAction);
                QListWidget::dropEvent(event);
                emit moveCommand(source, sourceRow, currentRow());
        }
        else event->ignore();
    }
    else {
        event->acceptProposedAction();
        emit copyCommand(source, sourceRow, this);
    }
}

uint8_t customListWidget::switchNumber() const
{
    return m_switchNumber;
}

void customListWidget::setSwitchNumber(const uint8_t &switchNumber)
{
    m_switchNumber = switchNumber;
}

void customListWidget::setLeftWidget(customListWidget *widget)
{
    leftWidget = widget;
}

void customListWidget::setRightWidget(customListWidget *widget)
{
    rightWidget = widget;
}

bool customListWidget::eventFilter(QObject *obj, QEvent *event) // To allow clicking anywhere in the listwidget.
{
    if (event->type() == QEvent::MouseButtonDblClick)
    {
        emit customDoubleClicked();
    }
    if (event->type() == QEvent::MouseButtonPress)
    {
        emit customClicked();
    }

    if (event->type() == QEvent::MouseButtonRelease) return true;

    return QListWidget::eventFilter(obj, event);
}

void customListWidget::keyPressEvent(QKeyEvent *keyEvent)
{
    if (keyEvent->key() == Qt::Key_Left) {
        if (leftWidget != 0) emit selectWidget(leftWidget);
    }
    else if (keyEvent->key() == Qt::Key_Right) {
        if (rightWidget != 0) emit selectWidget(rightWidget);
    }
    else QListWidget::keyPressEvent(keyEvent);
}
