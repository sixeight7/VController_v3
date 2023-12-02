#ifndef INITIALIZEUSERDEVICEDATADIALOG_H
#define INITIALIZEUSERDEVICEDATADIALOG_H

#include <QDialog>
#include "vcuserdevices.h"

namespace Ui {
class initializeUserDeviceDataDialog;
}

class initializeUserDeviceDataDialog : public QDialog
{
    Q_OBJECT

public:
    explicit initializeUserDeviceDataDialog(QWidget *parent = nullptr, uint8_t _instance = 0, VCuserdevices * myVCUD = NULL);
    ~initializeUserDeviceDataDialog();

private slots:
    void on_buttonBox_accepted();

    void on_checkAllBox_stateChanged(int arg1);

private:
    Ui::initializeUserDeviceDataDialog *ui;
    uint8_t instance = 0;
    VCuserdevices * MyVCuserDevice;

};

#endif // INITIALIZEUSERDEVICEDATADIALOG_H
