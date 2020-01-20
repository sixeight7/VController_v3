#ifndef CUSTOMPATCHLISTWIDGET_H
#define CUSTOMPATCHLISTWIDGET_H

#include <QListWidget>

class customPatchListWidget : public QListWidget
{
public:
    customPatchListWidget(QWidget *parent);

signals:
    void dragMovePatch(int sourceRow, int destRow);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
};

#endif // CUSTOMPATCHLISTWIDGET_H
