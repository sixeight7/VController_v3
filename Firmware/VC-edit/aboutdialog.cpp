#include "aboutdialog.h"
#include "VController/globals.h"
#include "ui_aboutdialog_VC-full.h"

AboutDialog::AboutDialog(QWidget *parent, QString version) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);

    QString hardware = "not connected";
    switch(VC_hardware_version) {
        case 1: hardware = "VController v2"; break;
        case 2: hardware = "VController"; break;
        case 3: hardware = "VC-mini (Teensy 3.6)"; break;
        case 4: hardware = "VC-mini (Teensy 4.1)"; break;
        case 5: hardware = "VC-touch"; break;
        case 6: hardware = "Custom VController"; break;
    };

    QString text = ui->label_info->text().arg(version).arg(hardware).arg(QT_VERSION_STR);
    ui->label_info->setText(text);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
