#include "aboutdialog.h"
#ifdef IS_VCMINI
#include "ui_aboutdialog_VC-mini.h"
#else
#include "ui_aboutdialog_VC-full.h"
#endif

AboutDialog::AboutDialog(QWidget *parent, QString version) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);

    QString text = ui->label_info->text().arg(version).arg(QString::fromLatin1(__DATE__)).arg(QT_VERSION_STR);
    ui->label_info->setText(text);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
