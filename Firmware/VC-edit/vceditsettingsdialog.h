#ifndef VCEDITSETTINGSDIALOG_H
#define VCEDITSETTINGSDIALOG_H

// The application settings window

#include <QDialog>
#include "midi.h"

namespace Ui {
class vcEditSettingsDialog;
}

class vcEditSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit vcEditSettingsDialog(QWidget *parent = 0, Midi *midi = nullptr);
    ~vcEditSettingsDialog();

signals:
    void appSettingsChanged();

private slots:
    void on_buttonBox_accepted();

    void on_deviceModeComboBox_currentIndexChanged(int index);

    void on_MidiInComboBox_currentTextChanged(const QString &arg1);

    void on_MidiOutComboBox_currentTextChanged(const QString &arg1);

private:
    Ui::vcEditSettingsDialog *ui;
    void loadSettings();
    void saveSettings();
    Midi *_midi;
    uint8_t my_VC_type;
    bool booted = false;
};


#endif // VCEDITSETTINGSDIALOG_H
