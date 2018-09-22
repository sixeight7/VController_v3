#include "vceditsettingsdialog.h"
#include "ui_vceditsettingsdialog.h"
#include <QSettings>
#include <QDebug>

vcEditSettingsDialog::vcEditSettingsDialog(QWidget *parent, Midi *midi) :
    QDialog(parent),
    ui(new Ui::vcEditSettingsDialog)
{
    ui->setupUi(this);

    // Populate the combo boxes
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
    appSettings.endGroup();
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
    appSettings.endGroup();
    qDebug() << "Midi settings saved:" << inPort << outPort;
    emit appSettingsChanged();
}
