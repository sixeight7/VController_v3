#ifndef CUSTOMLISTWIDGET_H
#define CUSTOMLISTWIDGET_H

// customListwidget: used on the mai window and in the edit screen.
// It supports drag and drop. It sends a copyCommand or moveCommand signal when this happens.
// The widget also stores variables for switch number, left and right widget items.

#include <QListWidget>

class customListWidget : public QListWidget
{
    Q_OBJECT

public:
    customListWidget(QWidget *parent);

    uint8_t switchNumber() const;
    void setSwitchNumber(const uint8_t &switchNumber);
    void setLeftWidget(customListWidget *widget);
    void setRightWidget(customListWidget *widget);

signals:
    void copyCommand(customListWidget *sourceWidget, int sourceRow, customListWidget *destWidget);
    void moveCommand(customListWidget *widget, int sourceRow, int destRow);
    void customDoubleClicked();
    void customClicked();
    void selectWidget(customListWidget *widget);

protected:
    // Drag and drop support:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);
    // eventFilter is used to support clicking and doubleclicking anywhere in the listwidget
    bool eventFilter(QObject *obj, QEvent *event);
    // keyPressEvent is used to catch arrow left and right key presses.
    void keyPressEvent(QKeyEvent*);

private:
    uint8_t m_switchNumber;
    customListWidget *leftWidget = 0;
    customListWidget *rightWidget = 0;
};

#endif // CUSTOMLISTWIDGET_H
