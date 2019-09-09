#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

// Popup window for menu Help - About

#include <QDialog>
#include "VController/config.h"

namespace Ui {
    class About;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = 0, QString version = "");
    ~AboutDialog();

private:
    Ui::About *ui;
};

#endif // ABOUTDIALOG_H
