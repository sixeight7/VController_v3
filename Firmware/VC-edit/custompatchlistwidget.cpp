#include "custompatchlistwidget.h"

#include <QDebug>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>

customPatchListWidget::customPatchListWidget(QWidget *parent) : QListWidget(parent)
{
    Q_UNUSED(parent);

    setDragDropMode(QAbstractItemView::InternalMove);
    setDefaultDropAction(Qt::MoveAction);
}

void customPatchListWidget::dragEnterEvent ( QDragEnterEvent *event )
{
    if ( event->source() != this ) {
        setDragDropMode ( QAbstractItemView:: DragDrop );
               event->accept();
    } else {
        setDragDropMode ( QAbstractItemView::InternalMove );
        event->accept();
    }

}

void customPatchListWidget::dropEvent(QDropEvent *event)
{
    customPatchListWidget *source = (customPatchListWidget*) event->source();

    int sourceRow = source->currentRow();

    if (event->source() == this) {
        event->acceptProposedAction();
        //emit dragMovePatch(sourceRow, currentRow());
    }
}
