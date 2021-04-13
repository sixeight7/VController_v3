#ifndef SCENEDIALOG_H
#define SCENEDIALOG_H

#include <QDialog>

namespace Ui {
class scenedialog;
}

class scenedialog : public QDialog
{
    Q_OBJECT

public:
    explicit scenedialog(QWidget *parent = nullptr, int patch = 0);
    ~scenedialog();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::scenedialog *ui;
    int my_index;
};

#endif // SCENEDIALOG_H
