#include "vceditsettingsdialog.h"
#include "ui_vceditsettingsdialog.h"

#include <QSettings>
#include <QDebug>
#include <QMessageBox>

vcEditSettingsDialog::vcEditSettingsDialog(QWidget *parent, Midi *midi) :
    QDialog(parent),
    ui(new Ui::vcEditSettingsDialog)
{
    ui->setupUi(this);

    // Populate the combo boxes
    QStringList deviceModeItems = {"VController", "VC-mini", "VC-touch"};
    ui->deviceModeComboBox->addItems(deviceModeItems);
    _midi = midi;
    QStringList inPortItems = _midi->fillMidiInPortItems();
    QStringList outPortItems = _midi->fillMidiOutPortItems();
    ui->MidiInComboBox->addItems(inPortItems);
    ui->MidiOutComboBox->addItems(outPortItems);
    loadSettings();
}

vcEditSettingsDialog::~vcEditSettingsDialog()
{
    saveSettings();
    delete ui;
}

void vcEditSettingsDialog::loadSettings()
{
    QSettings appSettings;

    appSettings.beginGroup("SettingsWindow");
    resize(appSettings.value("size", QSize(400, 400)).toSize());
    move(appSettings.value("pos", QPoint(200, 200)).toPoint());
    appSettings.endGroup();

    appSettings.beginGroup("Midi");
    ui->MidiInComboBox->setCurrentText(appSettings.value("midiInPort").toString());
    ui->MidiOutComboBox->setCurrentText(appSettings.value("midiOutPort").toString());
    ui->SlowMidiCheckBox->setChecked(appSettings.value("midiSlowMode").toBool());
    appSettings.endGroup();

    appSettings.beginGroup("MainWindow");
    my_VC_type = appSettings.value("VC_type").toUInt();
    ui->deviceModeComboBox->setCurrentIndex(my_VC_type);
    appSettings.endGroup();

    booted = true;
}

void vcEditSettingsDialog::saveSettings()
{
    QSettings appSettings;

    appSettings.beginGroup("SettingsWindow");
    appSettings.setValue("size", size());
    appSettings.setValue("pos", pos());
    appSettings.endGroup();
}


void vcEditSettingsDialog::on_buttonBox_accepted()
{
    QSettings appSettings;
    appSettings.beginGroup("Midi");
    QString inPort = ui->MidiInComboBox->currentText();
    if (inPort != "") appSettings.setValue("midiInPort", inPort);
    QString outPort = ui->MidiOutComboBox->currentText();
    if (outPort != "") appSettings.setValue("midiOutPort", outPort);
    bool goSlow = ui->SlowMidiCheckBox->isChecked();
    appSettings.setValue("midiSlowMode", goSlow);
    appSettings.endGroup();
    qDebug() << "Midi settings saved:" << inPort << outPort;

    appSettings.beginGroup("MainWindow");
    appSettings.setValue("VC_type", ui->deviceModeComboBox->currentIndex());
    appSettings.endGroup();

    emit appSettingsChanged();
}

void vcEditSettingsDialog::on_deviceModeComboBox_currentIndexChanged(int index)
{
    if (!booted) return;
    if (index == my_VC_type) return;
    if (QMessageBox::No == QMessageBox(QMessageBox::Warning, "Change VController type", "VC-edit needs to restart to change to VController type! Are you sure you want to proceed?",
                                    QMessageBox::Yes|QMessageBox::No).exec()) {ui->deviceModeComboBox->setCurrentIndex(my_VC_type); return;};
    on_buttonBox_accepted();
    close();
}


void vcEditSettingsDialog::on_MidiInComboBox_currentTextChanged(const QString &arg1)
{
    if (arg1 == "VC MIDI") ui->SlowMidiCheckBox->setChecked(false);
    else ui->SlowMidiCheckBox->setChecked(true);
}


void vcEditSettingsDialog::on_MidiOutComboBox_currentTextChanged(const QString &arg1)
{
    if (arg1 == "VC MIDI") ui->SlowMidiCheckBox->setChecked(false);
    else ui->SlowMidiCheckBox->setChecked(true);
}

