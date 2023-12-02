#include "initializeuserdevicedatadialog.h"
#include "ui_initializeuserdevicedatadialog.h"

initializeUserDeviceDataDialog::initializeUserDeviceDataDialog(QWidget *parent, uint8_t _instance, VCuserdevices * myVCUD) :
    QDialog(parent),
    ui(new Ui::initializeUserDeviceDataDialog)
{
    instance = _instance;
    MyVCuserDevice = myVCUD;
    ui->setupUi(this);
    setWindowTitle("User Device " + QString::number(instance + 1));
}

initializeUserDeviceDataDialog::~initializeUserDeviceDataDialog()
{
    delete ui;
}

void initializeUserDeviceDataDialog::on_buttonBox_accepted()
{
    MyVCuserDevice->initializeUserDeviceData(instance, ui->userDeviceSettingsCheckBox->isChecked(), ui->patchNamesCheckBox->isChecked(), ui->effectStatesCheckBox->isChecked(), ui->FXNamesCheckBox->isChecked(), ui->sceneNamesCheckBox->isChecked(), ui->colourSetingsCheckBox->isChecked());
}


void initializeUserDeviceDataDialog::on_checkAllBox_stateChanged(int arg1)
{
    ui->userDeviceSettingsCheckBox->setChecked(arg1);
    ui->patchNamesCheckBox->setChecked(arg1);
    ui->effectStatesCheckBox->setChecked(arg1);
    ui->FXNamesCheckBox->setChecked(arg1);
    ui->sceneNamesCheckBox->setChecked(arg1);
    ui->colourSetingsCheckBox->setChecked(arg1);
}

