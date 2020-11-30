#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// The main window of VC-edit.
// The stylesheet for this window is set in the form mainwindow.ui in the centralWidget!

#include <QMainWindow>
#include <QString>
#include <QLabel>
#include <QTimer>
#include "midi.h"
#include "vcsettings.h"
#include "vcmidiswitchsettings.h"
#include "vcdevices.h"
#include "vccommands.h"
#include "customlistwidget.h"

// This version number is shown in the about dialog and also appears in all files created by VC-edit.
// Version should be the same as the version shown on the VController of VC-mini
#define VCMINI_FIRMWARE_VERSION_MAJOR 3
#define VCMINI_FIRMWARE_VERSION_MINOR 6
#define VCMINI_FIRMWARE_VERSION_BUILD 1

#define STATUS_BAR_MESSAGE_TIME 2000

#ifndef IS_VCMINI
#define NUMBER_OF_ON_SCREEN_BUTTONS 16
#else
#define NUMBER_OF_ON_SCREEN_BUTTONS 7
#endif

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    Midi midi_port;

public slots:
    void runEverySecond();
    void updateSettings();
    void updateCommands(int, int);
    void updateCommandScreens(bool);
    void updateLcdDisplay(int, QString, QString);
    void setButtonColour(int, int);
    void listWidgetClicked();
    void selectWidget(customListWidget *widget);
    void updatePatchListBox();

    // Menu actions - also appear in submenus
    void checkMenuItems();
    void ShowPageContextMenu(const QPoint &pos); // For right-click on page comboBox
    void ShowListWidgetContextMenu(const QPoint &pos); // For right click on the listWidgets
    void ShowPatchContextMenu(const QPoint &pos);

    // Menu File
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionPreferences_triggered();

    // menu Page
    void on_actionNewPage_triggered();
    void on_actionRenamePage_triggered();
    void on_actionClearPage_triggered();
    void on_actionSavePage_triggered();
    void on_actionDuplicatePage_triggered();
    void on_actionNextPage_triggered();
    void on_actionPreviousPage_triggered();

    // Menu Command
    void on_actionEditSwitch_triggered();
    void on_actionCopyCommand_triggered();
    void on_actionCutCommand_triggered();
    void on_actionPasteCommand_triggered();
    void on_actionDeleteCommand_triggered();

    // Menu Patch
    void on_actionCopyPatch_triggered();
    void on_actionPastePatch_triggered();
    void on_actionRename_triggered();
    void on_actionExport_triggered();
    void on_actionImport_triggered();

    // Menu Help
    void on_actionAbout_triggered();
    void on_actionWebsite_triggered();
    void on_actionDonate_triggered();

    void startProgressBar(int size, QString message);
    void updateProgressBar(int value);
    void closeProgressBar(QString message);
    bool checkSpaceForNewPageOK();

private slots:
    void VControllerDetected(int type, int versionMajor, int versionMinor, int versionBuild);
    void loadAppSettings();
    void saveAppSettings();
    void openEditWindow(int sw, int item);
    void remoteSwitchPressed(int sw);
    void remoteSwitchReleased(int sw);

    // Action in widgets
    void on_currentPageComboBox_activated(int index);
    void on_currentPageComboBox2_activated(int index);
    void on_tabWidget_currentChanged(int index);
    void treeWidgetActivated(QModelIndex);
    void movePatch(customListWidget *widget, int sourceRow, int destRow);

    // Pressing buttons
    void on_readSysexButton_clicked();
    void on_writeSysexButton_clicked();

    void on_readSysexCmdButton_clicked();
    void on_readSysexCmdButton2_clicked();
    void on_writeSysexCmdButton_clicked();
    void on_writeSysexCmdButton2_clicked();

    void on_toolButtonPageDown_clicked();
    void on_toolButtonPageUp_clicked();
    void on_toolButtonPageDown2_clicked();
    void on_toolButtonPageUp2_clicked();

    // Remote control switches
    void on_switch_1_pressed();
    void on_switch_1_released();
    void on_switch_2_pressed();
    void on_switch_2_released();
    void on_switch_3_pressed();
    void on_switch_3_released();
    void on_switch_4_pressed();
    void on_switch_4_released();
    void on_switch_5_pressed();
    void on_switch_5_released();
    void on_switch_6_pressed();
    void on_switch_6_released();
    void on_switch_7_pressed();
    void on_switch_7_released();
    void on_switch_8_pressed();
    void on_switch_8_released();
    void on_switch_9_pressed();
    void on_switch_9_released();
    void on_switch_10_pressed();
    void on_switch_10_released();
    void on_switch_11_pressed();
    void on_switch_11_released();
    void on_switch_12_pressed();
    void on_switch_12_released();
    void on_switch_13_pressed();
    void on_switch_13_released();
    void on_switch_14_pressed();
    void on_switch_14_released();
    void on_switch_15_pressed();
    void on_switch_15_released();
    void on_switch_16_pressed();
    void on_switch_16_released();

    void on_refreshSettingsTreeButton_clicked();

    void on_readPatchButton_clicked();

    void on_writePatchButton_clicked();

private:
    void setupLcdDisplays();
    void setupButtons();
    void setupEditTab();
    void fillTreeWidget(QTreeWidget *my_tree);
    void fillPageComboBox(QComboBox *my_combobox);
    void fillListBoxes(bool first_time);
    void fillPatchListBox(QListWidget *my_patchList);
    void updateStatusLabel();

    // Loading and saving files
    void writeHeader(QJsonObject &json, QString type);
    QString readHeader(const QJsonObject &json);

    void try_reconnect_MIDI();
    QString addNonBreakingSpaces(QString text);

    Ui::MainWindow *ui;
    QString MyFullBackupFile;
    QString MySavePageFile;
    QString MySavePatchFile;
    QString MyMidiInPort, MyMidiOutPort;
    Midi *MyMidi;
    QTimer *timer;
    VCsettings *MyVCsettings;
    VCmidiSwitches *MyVCmidiSwitches;
    VCdevices *MyVCdevices;
    VCcommands *MyVCcommands;
    int currentPage = 0;
    int previousPage = 0;
    int currentSwitch = -1;
    int previousSwitch = -1;
    int previousSwitchPage = 0;
    int currentItem = 0;
    customListWidget *currentWidget = 0;
    bool RemoteControlActive = false;
    bool copyBufferFilled = false;
    QProgressBar *statusBar;
    QLabel *statusLabel;
    QLabel *editorStateLabel;
    bool dataEdited = false;
    bool fileLoaded = false;
    bool VControllerConnected = false;
};

#endif // MAINWINDOW_H
