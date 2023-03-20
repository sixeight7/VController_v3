#include "mainwindow.h"

// Load the proper ui
#include "ui_mainwindow_VC-full.h"

#include "vceditsettingsdialog.h"
#include "midi.h"
#include "vcsettings.h"
#include "vcmidiswitchsettings.h"
#include "vcseqpattern.h"
#include "vcdevices.h"
#include "commandeditdialog.h"
#include "aboutdialog.h"
#include "songeditdialog.h"
#include "setlisteditdialog.h"
#include "scenedialog.h"
#include "mainwindow.h"

#include <QSettings>
#include <QDir>
#include <QFileDialog>
#include <QDebug>
#include <QFontDatabase>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScrollBar>
#include <QMessageBox>
#include <QInputDialog>
#include <QProgressBar>
#include <QStyleFactory>
#include <QDesktopServices>
#include <QThread>
#include <QShortcut>
#include <QKeySequence>
#include <QProcess>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // Setup UI
    ui->setupUi(this);
    setMinimumSize(100, 100);
    QCoreApplication::setOrganizationName("Sixeight Sound Control");
    QCoreApplication::setApplicationName("VC-edit");
    //setWindowTitle( QCoreApplication::applicationName() + " for VController, VC-mini and VC-touch" );

    statusLabel = new QLabel();
    ui->statusbar->addPermanentWidget(statusLabel);

    editorStateLabel = new QLabel();
    ui->statusbar->addPermanentWidget(editorStateLabel);
    ui->tabWidget->setStyleSheet("QTabWidget::tab:disabled { width: 0; height: 0; margin: 0; padding: 0; border: none; }");

    // Setup MIDI
    MyMidi = new Midi();
    connect(MyMidi, SIGNAL(VControllerDetected(int,int,int,int)), this, SLOT(VControllerDetected(int,int,int,int)));
    connect(MyMidi, SIGNAL(updateLcdDisplay(int, QString, QString)),
                           this, SLOT(updateLcdDisplay(int, QString, QString)));
    connect(MyMidi, SIGNAL(updateLED(int,int)),
                           this, SLOT(setButtonColour(int,int)));
    connect(MyMidi, SIGNAL(updateSettings()),
                           this, SLOT(updateSettings()));
    connect(MyMidi, SIGNAL(updateCommands(int,int)),
                           this, SLOT(updateCommands(int,int)));
    connect(MyMidi, SIGNAL(startProgressBar(int, QString)), this, SLOT(startProgressBar(int, QString)));
    connect(MyMidi, SIGNAL(updateProgressBar(int)), this, SLOT(updateProgressBar(int)));
    connect(MyMidi, SIGNAL(closeProgressBar(QString)), this, SLOT(closeProgressBar(QString)));
    connect(MyMidi, SIGNAL(updatePatchListBox()), this, SLOT(updatePatchListBox()));

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(runEverySecond()));
    timer->start(1000);

    MyVCsettings = new VCsettings();
    MyVCmidiSwitches = new VCmidiSwitches();
    MyVCseqPatterns = new VCseqPattern();
    MyVCdevices = new VCdevices();
    MyVCcommands = new VCcommands();
    connect(MyVCcommands, SIGNAL(updateCommandScreens(bool)), this, SLOT(updateCommandScreens(bool)));

    loadAppSettings();
    MyVCdevices->setup_devices();
    drawPageComboBoxes();
    selectWidget(myListWidgets[0]);
    MyVCcommands->setup_VC_config();
    updateCommandScreens(true);
    drawRemoteControlArea();

    // Fill objects on screen
    fillTreeWidget(ui->treeWidget);
    fillPatchListBox(ui->patchListWidget);
    fillPatchTypeComboBox(ui->patchTypeComboBox);
    ui->patchListWidget->setDefaultDropAction(Qt::MoveAction);
    connect(ui->patchListWidget, SIGNAL(moveCommand(customListWidget*,int,int)), this, SLOT(movePatch(customListWidget*,int,int)));
    connect(ui->patchListWidget, SIGNAL(customDoubleClicked()), this, SLOT(on_actionEditPatch_triggered()));

    ui->currentPageComboBox->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->currentPageComboBox, SIGNAL(customContextMenuRequested(const QPoint)), this, SLOT(ShowPageContextMenu(QPoint)));
    ui->currentPageComboBox2->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->currentPageComboBox2, SIGNAL(customContextMenuRequested(const QPoint)), this, SLOT(ShowPageContextMenu(QPoint)));
    ui->patchListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->patchListWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ShowPatchContextMenu(QPoint)));
    setupLcdDisplays();
    resetRemoteControlButtons();
    on_tabWidget_currentChanged(ui->tabWidget->currentIndex());
}

MainWindow::~MainWindow()
{
    saveAppSettings();
    if (RemoteControlActive) MyMidi->sendSysexCommand(2, VC_REMOTE_CONTROL_ENABLE, 0);
    delete ui;
}

void MainWindow::runEverySecond()
{
    if (!VControllerConnected) {
        MyMidi->checkForVCmidi();
        try_reconnect_MIDI();
        MyMidi->send_universal_identity_request();
    }
    else {
        if (!MyMidi->checkMidiPortStillAvailable(MyMidiInPort, MyMidiOutPort)) {
            qDebug() << "MIDI offline";
            disconnect_VC();
        }
    }

}

void MainWindow::updateSettings()
{
    fillTreeWidget(ui->treeWidget);
    closeProgressBar("Settings read succesfully");
}

void MainWindow::updateCommands(int check_number_of_cmds, int check_number_of_pages)
{
    MyVCcommands->recreate_indexes();
    if (check_number_of_cmds != Commands.size()) {
        qDebug() << "Midi error: mismatch in number of commands";
    }
    else if (check_number_of_pages != Number_of_pages) {
        qDebug() << "Midi error: mismatch in number of pages";
    }
    else qDebug() << "Midi dump received succesful";
    fillPageComboBox(ui->currentPageComboBox);
    fillPageComboBox(ui->currentPageComboBox2);
    fillListBoxes(false);
}

void MainWindow::loadAppSettings() {
    QSettings appSettings;

    appSettings.beginGroup("MainWindow");
    uint8_t newType = appSettings.value("VC_type").toUInt();
    resize(appSettings.value("size", QSize(400, 400)).toSize());
    move(appSettings.value("pos", QPoint(200, 200)).toPoint());
    ui->tabWidget->setCurrentIndex(appSettings.value("currentTab", 0).toInt());
    MyFullBackupFile = appSettings.value("fullBackupFile").toString();
    if (MyFullBackupFile == "") MyFullBackupFile = QDir::homePath();
    MySavePageFile = appSettings.value("savePageFile").toString();
    if (MySavePageFile == "") MySavePageFile = QDir::homePath();
    MySavePatchFile = appSettings.value("savePatchFile").toString();
    if (MySavePatchFile == "") MySavePatchFile = QDir::homePath();
    currentPage = appSettings.value("currentPage").toInt();
    previousPage = currentPage;
    previousSwitchPage = currentPage;
    appSettings.endGroup();

    appSettings.beginGroup("Midi");
    MyMidiInPort = appSettings.value("midiInPort").toString();
    MyMidi->openMidiIn(MyMidiInPort);
    MyMidiOutPort = appSettings.value("midiOutPort").toString();
    MyMidi->openMidiOut(MyMidiOutPort);
    MyMidi->send_universal_identity_request(); //See if we can connect
    MyMidiSlowMode = appSettings.value("midiSlowMode").toBool();
    appSettings.endGroup();

    if (newType != VC_type) {
        if (!booted) {
            VC_type = newType;
            buildMainWindow();
            booted = true;
        }
        else {
            MyMidi->closeMidiIn(); // Bugfix: must close MIDI ports first, otherwise MIDI data will be read double!!!
            MyMidi->closeMidiOut();
#ifdef Q_OS_WIN
            // reboot VC-edit for windows
            QString program = qApp->arguments()[0];
            QStringList arguments = qApp->arguments().mid(1);
            qApp->quit();
            QProcess::startDetached(program, arguments);
#else
            // reboot VC-edit for Mac
            qApp->exit(EXIT_CODE_REBOOT);
#endif
        }
    }
}

void MainWindow::saveAppSettings() {
    QSettings appSettings;
    appSettings.beginGroup("MainWindow");
    appSettings.setValue("size", size());
    appSettings.setValue("pos", pos());
    appSettings.setValue("currentTab", ui->tabWidget->currentIndex());
    appSettings.setValue("fullBackupFile", MyFullBackupFile);
    appSettings.setValue("savePageFile", MySavePageFile);
    appSettings.setValue("savePatchFile", MySavePatchFile);
    appSettings.setValue("currentPage", currentPage);
    appSettings.setValue("currentPatchTypeComboBoxItem", ui->patchTypeComboBox->currentIndex());
    appSettings.endGroup();
}

void MainWindow::openEditWindow(int sw, int)
{
    if (currentPage > Number_of_pages) {
        QMessageBox msgBox;
        msgBox.setText("Fixed pages can not be edited");
        msgBox.exec();
        return;
    }
    commandEditDialog ce(this, MyVCcommands, currentPage, sw, currentWidget->currentRow());
    //ce.setModal(true);
    ce.exec();
    ce.close();

    fillListBoxes(false);
}

void MainWindow::remoteSwitchPressed(int sw) {
    if (VControllerConnected)
      MyMidi->sendSysexCommand(2, VC_REMOTE_CONTROL_SWITCH_PRESSED, sw);
    else offlineRemoteSwitchPressed(sw);
    qDebug() << "switch" << sw << "pressed";
}

void MainWindow::remoteSwitchReleased(int sw) {
    if (VControllerConnected)
      MyMidi->sendSysexCommand(2, VC_REMOTE_CONTROL_SWITCH_RELEASED, sw);
    else offlineRemoteSwitchReleased(sw);
    qDebug() << "switch" << sw << "released";
}

void MainWindow::setupLcdDisplays() {
    // Setup virtual lcd_displays:
    if (VC_type == VCTOUCH) setFont(ui->lcd0, 40);
    else setFont(ui->lcd0, 28);
}

void MainWindow::setFont(QLabel* display, uint8_t size) {
    int id = QFontDatabase::addApplicationFont(":/fonts/LCD_Solid.ttf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);
    QFont LCD_Solid(family);
    LCD_Solid.setPixelSize(size);
    display->setFont(LCD_Solid);
}

void MainWindow::resetRemoteControlButtons() {
    for (int i = 1; i <= myLCDs.count(); i++)
        updateLcdDisplay(i, "                ", "                ");
    for (int i = 1; i <= myLEDs.count(); i++)
        setButtonColour(i, 0); // Make the button grey
}

void MainWindow::fillTreeWidget(QTreeWidget *my_tree)
{
    my_tree->clear();
    my_tree->setColumnCount(5);
    my_tree->setHeaderLabels(QStringList() << "Setting" << "" << "Value" << "" << "Slider");
    //my_tree->setColumnWidth(0, 250);
    //my_tree->setColumnWidth(1, 250);
    //my_tree->setAutoScroll(false);
    my_tree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    my_tree->setColumnWidth(1, 4);
    my_tree->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    my_tree->setColumnWidth(3, 4);
    my_tree->header()->setSectionResizeMode(4, QHeaderView::Stretch);
    //my_tree->verticalScrollBar()->setStyleSheet("QScrollBar:vertical { width: 100px; }");
    //my_tree->verticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    my_tree->setStyleSheet("QTreeWidget::item { padding: 2 px; }");
    MyVCsettings->fillTreeWidget(my_tree);
    MyVCmidiSwitches->fillTreeWidget(my_tree, MyVCmidiSwitches);
    MyVCseqPatterns->fillTreeWidget(my_tree, MyVCseqPatterns);
    MyVCdevices->fillTreeWidget(my_tree, MyVCcommands);
    connect(my_tree, SIGNAL(activated(QModelIndex)), this, SLOT(treeWidgetActivated(QModelIndex)));
}

void MainWindow::treeWidgetActivated(QModelIndex)
{
    QApplication::processEvents();
}

void MainWindow::movePatch(customListWidget *, int sourceRow, int destRow)
{
    MyVCdevices->movePatch(sourceRow, destRow, currentDevicePatchType);
    fillPatchListBox(ui->patchListWidget);
}


void MainWindow::fillPageComboBox(QComboBox *my_combobox)
{
    my_combobox->clear();
    my_combobox->addItem("Default page");  //rgb(215, 214, 230)
    my_combobox->setItemData(my_combobox->count() - 1, QColor( 215, 214, 230 ), Qt::BackgroundRole);
    MyVCcommands->fillPageComboBox(my_combobox);
    MyVCcommands->fillFixedPageComboBox(my_combobox);
    if ((currentPage >= Number_of_pages) && (currentPage < first_fixed_cmd_page)) currentPage = Number_of_pages - 1;
    my_combobox->setCurrentIndex(MyVCcommands->indexFromValue(TYPE_PAGE, currentPage));
    qDebug() << "PageNumber: " << currentPage << ", index: " << MyVCcommands->indexFromValue(TYPE_PAGE, currentPage);
}

void MainWindow::fillListBoxes(bool first_time)
{
    for(int w = 0; w < myListWidgets.count(); w++) {
      MyVCcommands->fillCommandsListWidget(this, myListWidgets[w], currentPage, w + 1, true, first_time);
    }
    MyVCcommands->fillCommandsListWidget(this, ui->onPageSelect0ListWidget, currentPage, 0, true, first_time);

    // Create pacman shortcuts for first and last item
    if (first_time) {
        myListWidgets.first()->setLeftWidget(myListWidgets.last());
        myListWidgets.last()->setRightWidget(myListWidgets.first());
    }

    updateStatusLabel();
}

void MainWindow::fillPatchListBox(QListWidget *my_patchList)
{
    my_patchList->clear();
    if (currentDevicePatchType == 0) { // Memory view
        for (int p = 0; p < MAX_NUMBER_OF_DEVICE_PRESETS; p++) {
          if (Device_patches[p][0] == EXT_SONG_TYPE) my_patchList->addItem(readSongStringForPatchListWidget(p, 0));
          else if (Device_patches[p][0] == EXT_SETLIST_TYPE) my_patchList->addItem(readSetlistStringForPatchListWidget(p, 0));
          else my_patchList->addItem(MyVCdevices->ReadPatchStringForListWidget(p, 0));
      }
    }
    else if (currentDevicePatchType == EXT_SONG_TYPE) { // SONG view
        for (int p = 0; p <= MAX_NUMBER_OF_SONGS; p++) {
            my_patchList->addItem(readSongStringForPatchListWidget(p, currentDevicePatchType));
        }
    }
    else if (currentDevicePatchType == EXT_SETLIST_TYPE) { // SETLIST view
        for (int p = 1; p <= MAX_NUMBER_OF_SETLISTS; p++) {
            my_patchList->addItem(readSetlistStringForPatchListWidget(p, currentDevicePatchType));
        }
    }
    else { // Device view
        uint8_t dev = (currentDevicePatchType & 0xFF) - 1;
        if (dev < NUMBER_OF_DEVICES) {
          for (int p = Device[dev]->patch_min_as_stored_on_VC(); p <= Device[dev]->patch_max_as_stored_on_VC; p++) {
            my_patchList->addItem(MyVCdevices->ReadPatchStringForListWidget(p, currentDevicePatchType));
          }
        }
    }

    updateStatusLabel();
}

QString MainWindow::readSongStringForPatchListWidget(uint16_t number, uint8_t type)
{
    QString line;
    int my_index;
    if (type == 0) {
        line = QString::number(number);
        my_index = number;
    }
    else {
        line = songEditDialog::get_song_number_name(number);
        my_index = MyVCdevices->findIndex(type, number);
    }
    uint8_t my_type;
    if (my_index != PATCH_INDEX_NOT_FOUND) my_type = Device_patches[my_index][0];
    else my_type = 0;

    line.append(":\t");
    if (my_type == EXT_SONG_TYPE) {
        uint8_t song_no = (Device_patches[my_index][1] << 7) + Device_patches[my_index][2];
        line.append("SONG\t");
        line.append(songEditDialog::get_song_number_name(song_no));
        line.append("\t");
        line.append(songEditDialog::get_song_name(my_index));
    }
    else {
      line.append("New Song");
    }

    return line;
}

QString MainWindow::readSetlistStringForPatchListWidget(uint16_t number, uint8_t type)
{
    QString line;
    int my_index;
    if (type == 0) {
        line = QString::number(number);
        my_index = number;
    }
    else {
        line = setlistEditDialog::get_setlist_number_name(number);
        my_index = MyVCdevices->findIndex(type, number);
    }

    uint8_t my_type;
    if (my_index != PATCH_INDEX_NOT_FOUND) my_type = Device_patches[my_index][0];
    else my_type = 0;

    line.append(":\t");
    if (my_type == EXT_SETLIST_TYPE) {
        uint8_t setlist_no = (Device_patches[my_index][1] << 7) + Device_patches[my_index][2];
        line.append("SETLIST\t");
        line.append(setlistEditDialog::get_setlist_number_name(setlist_no));
        line.append("\t");
        line.append(setlistEditDialog::get_setlist_name(my_index));
    }
    else {
      line.append("New Setlist");
    }

    return line;
}

void MainWindow::fillPatchTypeComboBox(QComboBox *my_combobox)
{
   my_combobox->clear();
   QString memitem = VC_name;
   memitem.append(" memory");
   my_combobox->addItem(memitem);
   my_combobox->addItem("SONGS");
   my_combobox->addItem("SETLISTS");
   for (uint8_t dev = 0; dev < NUMBER_OF_DEVICES; dev++) {
       if (Device[dev]->supportPatchSaving() > 0) my_combobox->addItem(Device[dev]->full_device_name);
       if (Device[dev]->supportPatchSaving() > 1) my_combobox->addItem(Device[dev]->full_device_name + " bass mode");
   }

   QSettings appSettings;
   appSettings.beginGroup("MainWindow");
   int index = appSettings.value("currentPatchTypeComboBoxItem", 1).toInt();
   if (index > my_combobox->count()) index = 0;
   my_combobox->setCurrentIndex(index);
   appSettings.endGroup();
}

void MainWindow::updateStatusLabel()
{
    uint8_t cmd_percentage = Commands.size() * 100 / EXT_EEP_MAX_NUMBER_OF_COMMANDS;
    //statusLabel->setText("Cmd memory: " + QString::number(cmd_percentage) + "%");

    uint8_t patch_percentage = MyVCdevices->numberOfPatches() * 100 / MAX_NUMBER_OF_DEVICE_PRESETS;
    statusLabel->setText("Cmd memory: " + QString::number(cmd_percentage) + "%, Patch memory: " + QString::number(patch_percentage) + "%");

    uint8_t max_percentage = cmd_percentage;
    if (patch_percentage > cmd_percentage) max_percentage = patch_percentage;
    if (max_percentage >= 95) statusLabel->setStyleSheet("background-color: orange");
    else statusLabel->setStyleSheet("background: transparent");
    if (max_percentage >= 100) statusLabel->setStyleSheet("background-color: red");

    if (VControllerConnected) {
        editorStateLabel->setText("Online");
    }
    else {
        editorStateLabel->setText("Offline");
    }
}

void MainWindow::updateCommandScreens(bool first_time)
{
    fillPageComboBox(ui->currentPageComboBox);
    fillPageComboBox(ui->currentPageComboBox2);
    bool showPageUp = (MyVCcommands->indexFromValue(TYPE_PAGE, currentPage) < (Number_of_pages + last_fixed_cmd_page - first_fixed_cmd_page));
    bool showPageDown = (currentPage > 0);
    ui->toolButtonPageUp->setEnabled(showPageUp);
    ui->toolButtonPageUp2->setEnabled(showPageUp);
    ui->actionNextPage->setEnabled(showPageUp);
    ui->toolButtonPageDown->setEnabled(showPageDown);
    ui->toolButtonPageDown2->setEnabled(showPageDown);
    ui->actionPreviousPage->setEnabled(showPageDown);
    checkMenuItems();
    fillListBoxes(first_time);
}

void MainWindow::checkMenuItems()
{
   bool pageCanBeEdited = (currentPage < Number_of_pages);
   bool switchShowsDefaultItems = MyVCcommands->switchShowsDefaultItems(currentPage, currentSwitch);
   bool onDefaultPage = (currentPage == 0);
   ui->actionRenamePage->setEnabled(pageCanBeEdited);
   ui->actionClearPage->setEnabled(pageCanBeEdited && !onDefaultPage);
   ui->actionEditSwitch->setEnabled(pageCanBeEdited);
   ui->actionClearSwitch->setEnabled(pageCanBeEdited && !switchShowsDefaultItems);
   ui->actionCutCommand->setEnabled(pageCanBeEdited && !switchShowsDefaultItems);
   ui->actionPasteCommand->setEnabled(pageCanBeEdited && copyBufferFilled);
   ui->actionDeleteCommand->setEnabled(pageCanBeEdited && !switchShowsDefaultItems);

   //if (menuItemsEnabled) ui->currentPageComboBox->setStyleSheet("QComboBox { background-color: white; }");
   //else ui->currentPageComboBox->setStyleSheet("QComboBox { background-color: gray; }");
}

void MainWindow::updateLcdDisplay(int lcd_no, QString line1, QString line2) {
    if (lcd_no > myLCDs.count()) return;
    QString richText;
    //line1.resize(16);
    QString line1string = addNonBreakingSpaces(line1.toHtmlEscaped());
    //line2.resize(16);
    QString line2string = addNonBreakingSpaces(line2.toHtmlEscaped());
    if (lcd_no == 0) {
        richText = "<html><body><p style=\"line-height:.5\">" + line1string +
                "</p><p style=\"line-height:1\">" + line2string + "</p></body></html>";
        if (VC_type == VCTOUCH) {
            line2string = "<font color=\"yellow\">" + line2string + "</font>";
        }
        ui->lcd0->setText(richText);
    }
    else {
        myLCDs[lcd_no - 1]->setDisplayText(line1string, line2string);
    }
}

void MainWindow::setButtonColour(int button, int colour) {
    QColor my_colour = getColour(colour);
    if (VC_type != VCTOUCH) {
        if ((button > 0) && (button <= myLEDs.count())) {
            myLEDs[button - 1]->setLedColor(my_colour);
            myLEDs[button - 1]->repaint();
        }
    }
    else {
        if (button <= myLCDs.count()) {
            if (my_colour == "gray") my_colour = QColor(6, 19, 59);
            myLCDs[button - 1]->setColour(my_colour);
        }
    }

    // Create a stylesheet with the colour.
    //QString colourName = my_colour.name();
    /*QString styleSheetString_small = "QPushButton{background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                               "stop: 0 white, stop: 1 " + colourName + ");" "border-style: solid;" "border-color: black;"
                               "border-width: 2px;" "border-radius: 10px;}";
    QString styleSheetString_large = "QPushButton{background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                               "stop: 0 white, stop: 1 " + colourName + ");" "border-style: solid;" "border-color: black;"
                               "border-width: 2px;" "border-radius: 50px;}";
    QString styleSheetString_medium = "QPushButton{background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                               "stop: 0 white, stop: 1 " + colourName + ");" "border-style: solid;" "border-color: black;"
                               "border-width: 2px;" "border-radius: 20px;}";*/
    // Set the colour for the switch by changing the stylesheet.

}

QColor MainWindow::getColour(uint8_t colour) {
    // Create a string with the colourname;
    switch (colour) {
      case 1: return "green"; // Colour 1 is Green
      case 2: return "red"; //  Colour 2 is Red
      case 3: return "blue"; // Colour 3 is Blue
      case 4: return QColor(255, 150, 20); // Colour 4 is Orange
      case 5: return "cyan"; // Colour 5 is Cyan
      case 6: return "white"; // Colour 6 is White
      case 7: return "yellow";  // Colour 7 is Yellow
      case 8: return "magenta";  // Colour 8 is Magenta
      case 9: return QColor(250, 20, 147);  // Colour 9 is Pink
      case 10: return QColor(102, 240, 150); // Colour 10 is Soft green
      case 11: return QColor(0, 142, 255); // Colour 11 is Light Blue

      case 17: return "darkGreen";  // Colour 17 is Green dimmed
      case 18: return "darkRed";  //  Colour 18 is Red dimmed
      case 19: return "darkBlue";  // Colour 19 is Blue dimmed
      case 20: return QColor(240, 80, 0);  // Colour 20 is Orange dimmed
      case 21: return "darkCyan";  // Colour 21 is Cyan dimmed
      case 22: return QColor(80, 80, 80);  // Colour 22 is White dimmed
      case 23: return QColor(0xE8, 0xB8, 0x28);   // Colour 23 is Yellow dimmed
      case 24: return "darkMagenta";   // Colour 24 is Magenta dimmed
      case 25: return QColor(180, 15, 100);   // Colour 25 is Pink dimmed
      case 26: return QColor(75, 150, 100); // Colour 10 is Soft green dimmed
      case 27: return QColor(0, 92, 184); // Colour 10 is Light Blue dimmed

      default: return "gray";
    }
}

void MainWindow::listWidgetClicked()
{
    if ((currentWidget != (customListWidget *)sender()) && (currentWidget != 0)) {
        currentWidget->setCurrentRow(-1); // Clear selection of previous listWidget.
        currentWidget->clearSelection(); // Need both commands to get it working properly
    }

    currentWidget = (customListWidget *)sender();
    int newSwitch = currentWidget->switchNumber();
    if ((newSwitch != currentSwitch) || (previousSwitchPage != currentPage)) {
        previousSwitch = currentSwitch;
        currentSwitch = newSwitch;
        previousSwitchPage = previousPage;
        previousPage = currentPage;
    }
    currentItem = currentWidget->currentRow();
    if (MyVCcommands->switchShowsDefaultItems(currentPage, currentSwitch)) currentItem--;
    if (currentItem == -1) {
        currentWidget->setCurrentRow(0);
        currentItem = 0;
    }
    if (currentItem == -2) { // Default page
        currentWidget->setCurrentRow(1);
        currentItem = 0;
    }
    //ui->menuSwitch->setTitle("Switch#" + QString::number(currentSwitch));
    checkMenuItems();
}

void MainWindow::selectWidget(customListWidget *widget)
{
    if (currentWidget != 0) {
        currentWidget->setCurrentRow(-1); // Clear selection of previous listWidget.
        currentWidget->clearSelection(); // Need both commands to get it working properly
    }

    currentWidget = widget;
    int newSwitch = currentWidget->switchNumber();
    if ((newSwitch != currentSwitch) || (previousSwitchPage != currentPage)) {
        previousSwitch = currentSwitch;
        currentSwitch = newSwitch;
        previousSwitchPage = previousPage;
        previousPage = currentPage;
    }
    if (MyVCcommands->switchShowsDefaultItems(currentPage, currentSwitch)) {
        currentWidget->setCurrentRow(1);
        currentItem = 0;
    }
    else {
        currentWidget->setCurrentRow(0);
        currentItem = 0;
    }

    // Select the right tab in the tabwidget
    if (currentWidget->parent() == ui->pageCommandsFrame) ui->tabWidget->setCurrentIndex(0);
    else ui->tabWidget->setCurrentIndex(1);

    currentWidget->setFocus();
    checkMenuItems();
}

void MainWindow::updatePatchListBox()
{
    fillPatchListBox(ui->patchListWidget);
}

// Menu items

void MainWindow::on_actionOpen_triggered()
{
    // File Open
    QString MyFile = QFileDialog::getOpenFileName(this, "Open VC-edit file", QFileInfo(MyFullBackupFile).filePath(), tr("VC-edit data (*.vcd)"));
    QFile loadFile(MyFile);

    if (!loadFile.open(QIODevice::ReadOnly)) {
        ui->statusbar->showMessage("Couldn't open selected file.", STATUS_BAR_MESSAGE_TIME);
        return;
    }

    QByteArray saveData = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
    fileLoaded = true;

    QString jsonType = readHeader(loadDoc.object());
    if (jsonType == "") {
        ui->statusbar->showMessage("Couldn't read selected. No VC-edit settings file", STATUS_BAR_MESSAGE_TIME);
        return;
    }

    QString file_VCtype = checkHeaderContainsRightVCtype(loadDoc.object());
    if (file_VCtype != "") {
        if (QMessageBox::No == QMessageBox(QMessageBox::Warning, "Loading full backup", "File contains data for " + file_VCtype +" and VC-edit is set for " + VC_name + ". Are you sure you want to load this data?",
                                        QMessageBox::Yes|QMessageBox::No).exec()) return;
    }

    if (jsonType == "FullBackup") {
        if (dataEdited) {
            if (QMessageBox::No == QMessageBox(QMessageBox::Warning, "Loading full backup", "All current data will be lost! Are you sure you want to proceed?",
                                            QMessageBox::Yes|QMessageBox::No).exec()) return;
        }
        MyVCsettings->read(loadDoc.object());
        MyVCmidiSwitches->read(loadDoc.object());
        MyVCseqPatterns->read(loadDoc.object());
        MyVCdevices->read(loadDoc.object());
        MyVCcommands->readAll(loadDoc.object());
        //MyVCdevices->readAll(loadDoc.object());
        readAllPatchData(loadDoc.object());
        MyVCdevices->readAllLegacyKatana(loadDoc.object());
        fillTreeWidget(ui->treeWidget); // Will refresh the settings in the widget
        updateCommandScreens(false);
        fillPatchListBox(ui->patchListWidget);
        ui->statusbar->showMessage(MyFile + " opened", STATUS_BAR_MESSAGE_TIME);
    }

    if (jsonType == "PageData") {
        if (currentPage < Number_of_pages) {
            QMessageBox msgBox; msgBox.setText(tr("Loading page data\nAdd to a new page or overwrite current page?"));
            QAbstractButton* pButtonAddNew = msgBox.addButton(tr("Add to new page"), QMessageBox::ApplyRole); // Role determines order of buttons! ApplyRole is default and one the left
            QAbstractButton* pButtonOverwrite = msgBox.addButton(tr("Overwrite current page"), QMessageBox::NoRole);
            QAbstractButton* pButtonCancel = msgBox.addButton(tr("Cancel"), QMessageBox::YesRole);
            msgBox.setEscapeButton(pButtonCancel);
            msgBox.exec();
            if (msgBox.clickedButton()==pButtonAddNew) currentPage = Number_of_pages;
            else if (msgBox.clickedButton()==pButtonOverwrite) MyVCcommands->clearPage(currentPage);
            else return;
        }
        else {
            currentPage = MyVCcommands->addPage();
        }
        MyVCcommands->readPage(currentPage, loadDoc.object());
        fillTreeWidget(ui->treeWidget); // Will refresh the settings in the widget
        updateCommandScreens(false);
        ui->statusbar->showMessage(MyFile + " opened", STATUS_BAR_MESSAGE_TIME);
    }
}

void MainWindow::on_actionSave_triggered()
{
    // File Save
    MyFullBackupFile = QFileDialog::getSaveFileName(this, "Save VC-edit file:", MyFullBackupFile, tr("VC-edit data (*.vcd)"));
    QFile saveFile(MyFullBackupFile);

    if (!saveFile.open(QIODevice::WriteOnly)) {
        ui->statusbar->showMessage("Couldn't open selected file.", STATUS_BAR_MESSAGE_TIME);
        return;
    }

    QJsonObject saveObject;
    writeHeader(saveObject, "FullBackup");
    MyVCsettings->write(saveObject);
    MyVCmidiSwitches->write(saveObject);
    MyVCseqPatterns->write(saveObject);
    MyVCdevices->write(saveObject);
    MyVCcommands->writeAll(saveObject);
    writeAllPatchData(saveObject);
    //MyVCdevices->writeAll(saveObject);
    QJsonDocument saveDoc(saveObject);
    saveFile.write(saveDoc.toJson());
    ui->statusbar->showMessage(MyFullBackupFile + " saved", STATUS_BAR_MESSAGE_TIME);
    dataEdited = false;
}

void MainWindow::writeHeader(QJsonObject &json, QString type)
{
    QJsonObject headerObject;
    headerObject["Source"] = "VC-edit";
    headerObject["Version"] = QString::number(VCMINI_FIRMWARE_VERSION_MAJOR) + "." + QString::number(VCMINI_FIRMWARE_VERSION_MINOR) + "." + QString::number(VCMINI_FIRMWARE_VERSION_BUILD);
    headerObject["Type"] = type;
    headerObject["VC_type"] = VC_type;
    headerObject["VC_name"] = VC_name;
    json["Header"] = headerObject;
}

void MainWindow::writeAllPatchData(QJsonObject &json) const
{
    QJsonObject allPatches;
    for (int p = 0; p < MAX_NUMBER_OF_DEVICE_PRESETS; p++) {
        uint8_t my_type = Device_patches[p][0];
        int patch_no = (Device_patches[p][1] << 8) + Device_patches[p][2];
        if (my_type > 0) {
            QJsonObject patchObject;
            patchObject["Device type"] = my_type;
            patchObject["Device patch number"] = patch_no;
            if ((my_type > 0) && (my_type <= NUMBER_OF_DEVICES)) MyVCdevices->writePatchData(p, patchObject);
            if (my_type == EXT_SETLIST_TYPE) setlistEditDialog::writeSetlistData(p, patchObject);
            if (my_type == EXT_SONG_TYPE) songEditDialog::writeSongData(p, patchObject);
            allPatches["Patch_" + QString::number(p)] = patchObject;
        }
        //patchArray.append(patchObject);
    }
    json["Device Patches"] = allPatches;
}

QString MainWindow::readHeader(const QJsonObject &json)
{
    if (!(json.contains("Header"))) return "";
    QJsonObject headerObject = json["Header"].toObject();
    if (!(headerObject["Source"].toString() == "VC-edit")) return "";
    return headerObject["Type"].toString();
}

void MainWindow::readAllPatchData(const QJsonObject &json)
{
    int my_type, patch_no;
    QJsonObject allPatches = json["Device Patches"].toObject();
    for (int p = 0; p < MAX_NUMBER_OF_DEVICE_PRESETS; p++) {
        QJsonObject patchObject = allPatches["Patch_" + QString::number(p)].toObject();
        if (!patchObject.isEmpty()) {
            my_type = patchObject["Device type"].toInt();
            patch_no = patchObject["Device patch number"].toInt();
            int index = MyVCdevices->findIndex(my_type, patch_no);
            if (index == PATCH_INDEX_NOT_FOUND) index = MyVCdevices->newIndex();
            if (index != PATCH_INDEX_NOT_FOUND) { // Not out of memory
                if ((my_type > 0) && (my_type <= NUMBER_OF_DEVICES)) MyVCdevices->readPatchData(patch_no, patchObject, my_type);
                if (my_type == EXT_SETLIST_TYPE) setlistEditDialog::readSetlistData(patch_no, patchObject, my_type, index);
                if (my_type == EXT_SONG_TYPE) songEditDialog::readSongData(patch_no, patchObject, my_type, index);
                allPatches["Patch_" + QString::number(p)] = patchObject;
            }
        }
    }
}

QString MainWindow::checkHeaderContainsRightVCtype(const QJsonObject &json)
{
    if (!(json.contains("Header"))) return "";
    QJsonObject headerObject = json["Header"].toObject();
    if (!headerObject.contains("VC_type")) return ""; // Older versions of VC-edit files do not contain this data
    if (headerObject["VC_type"].toInt() != VC_type) return headerObject["VC_name"].toString();
    return "";
}

void MainWindow::try_reconnect_MIDI()
{
    MyMidi->openMidiIn(MyMidiInPort);
    MyMidi->openMidiOut(MyMidiOutPort);
}

QString MainWindow::addNonBreakingSpaces(QString text)
{
    QString output = "";
    for (int i = 0; i < text.size(); i++) {
        if (text.at(i) == ' ') output.append("&nbsp;");
        else output.append(text.at(i));
    }
    return output;
}

void MainWindow::startOfflineRemoteControl()
{
    ui->lcd0->setText("<html><head/><body><p style=\"line-height:.6\"><span>   Connect to   </span></p><p style=\"line-height:1\"><span>  " + VC_name + "  </p></span></body></html>");
    resetRemoteControlButtons();

    if (VC_type == VCMINI) remoteBankSize = 3;
    else remoteBankSize = 10;
    for (uint8_t d = 0; d < remoteBankSize; d++) updateLcdDisplay(d + 1, centerLabel(Device[remoteDevice]->number_format(d)), centerLabel(Device[remoteDevice]->device_name + " Patch"));
    setButtonColour(remotePatchNumber + 1, Device[remoteDevice]->my_LED_colour);
    updateLcdDisplay(remoteBankSize + 1, centerLabel("<BANK DOWN>"), centerLabel(Device[remoteDevice]->device_name));
    setButtonColour(remoteBankSize + 1, Device[remoteDevice]->my_LED_colour + 16);
    updateLcdDisplay(remoteBankSize + 2, centerLabel("<BANK UP>"), centerLabel(Device[remoteDevice]->device_name));
    setButtonColour(remoteBankSize + 2, Device[remoteDevice]->my_LED_colour + 16);
    uint8_t next_device = remoteDevice + 1;
    if (next_device >= NUMBER_OF_DEVICES) next_device = 0;
    updateLcdDisplay(remoteBankSize + 3, centerLabel("<NEXT DEVICE>"), centerLabel(Device[next_device]->device_name));
    setButtonColour(remoteBankSize + 3, Device[next_device]->my_LED_colour + 16);
}

void MainWindow::offlineRemoteSwitchPressed(uint8_t sw)
{
   if (sw <= remoteBankSize) {
     setButtonColour(remotePatchNumber + 1, 0);
     remotePatchNumber = sw - 1;
     setButtonColour(remotePatchNumber + 1, Device[remoteDevice]->my_LED_colour);
   }
   if (sw == remoteBankSize + 3) {
       remoteDevice++;
       if (remoteDevice >= NUMBER_OF_DEVICES) remoteDevice = 0;
       startOfflineRemoteControl();
   }
   if ((sw >= remoteBankSize + 1) && (sw <= remoteBankSize + 3))
       setButtonColour(sw, Device[remoteDevice]->my_LED_colour);
}

void MainWindow::offlineRemoteSwitchReleased(uint8_t sw)
{
    if ((sw >= remoteBankSize + 1) && (sw <= remoteBankSize + 2))
        setButtonColour(sw, Device[remoteDevice]->my_LED_colour + 16);
    if (sw == remoteBankSize + 3) {
        uint8_t next_device = remoteDevice + 1;
        if (next_device >= NUMBER_OF_DEVICES) next_device = 0;
        setButtonColour(remoteBankSize + 3, Device[next_device]->my_LED_colour + 16);
    }
}

QString MainWindow::centerLabel(QString lbl)
{
    uint8_t msg_length = lbl.length();
    if (msg_length >= LCD_DISPLAY_SIZE) return lbl.left(LCD_DISPLAY_SIZE);
      uint8_t spaces_right = (LCD_DISPLAY_SIZE - msg_length) / 2;
      uint8_t spaces_left = LCD_DISPLAY_SIZE - spaces_right - msg_length;
      QString newmsg = "";
      for (uint8_t s = 0; s < spaces_left; s++) newmsg += ' ';
      newmsg += lbl;
      for (uint8_t s = 0; s < spaces_right; s++) newmsg += ' ';
      return newmsg;
}

void MainWindow::on_actionPreferences_triggered()
{
    // Preferences - open settingswindow
    saveAppSettings();
    vcEditSettingsDialog s(this, MyMidi);
    //connect(s, SIGNAL(appSettingsSaved()), this, SLOT(loadAppSettings()));
    s.setModal(true);
    s.exec();
    loadAppSettings();
}

void MainWindow::ShowPageContextMenu(const QPoint &pos)
{
    QComboBox *widget = (QComboBox *)sender();
    QPoint myPos = widget->mapToGlobal(pos);

    QMenu submenu;
    submenu.addAction(ui->actionNewPage);
    submenu.addAction(ui->actionRenamePage);
    submenu.addAction(ui->actionSavePage);
    submenu.addAction(ui->actionDuplicatePage);
    submenu.addAction(ui->actionClearPage);
    submenu.exec(myPos);
}

// Remote control switches

void MainWindow::on_switch_13_pressed()
{
    remoteSwitchPressed(13);
}

void MainWindow::on_switch_13_released()
{
    remoteSwitchReleased(13);
}

void MainWindow::on_switch_14_pressed()
{
    remoteSwitchPressed(14);
}

void MainWindow::on_switch_14_released()
{
    remoteSwitchReleased(14);
}

void MainWindow::on_switch_15_pressed()
{
    remoteSwitchPressed(15);
}

void MainWindow::on_switch_15_released()
{
    remoteSwitchReleased(15);
}

void MainWindow::on_switch_16_pressed()
{
    remoteSwitchPressed(16);
}

void MainWindow::on_switch_16_released()
{
    remoteSwitchReleased(16);
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    if (ui->tabWidget->tabText(index) == "Remote control") {
        RemoteControlActive = true;
        if (VControllerConnected) MyMidi->sendSysexCommand(2, VC_REMOTE_CONTROL_ENABLE, 1);
        else startOfflineRemoteControl();
        qDebug() << "Remote control enabled";
    }
    else {
        RemoteControlActive = false;
        if (VControllerConnected) MyMidi->sendSysexCommand(2, VC_REMOTE_CONTROL_ENABLE, 0);
        qDebug() << "Remote control disabled";
    }

    if ((ui->tabWidget->tabText(index) == "Page commands") || (ui->tabWidget->tabText(index) == "External switch commands")) {
        ui->menuPage->menuAction()->setVisible(true);
        ui->menuCommand->menuAction()->setVisible(true);
    }
    else {
        ui->menuPage->menuAction()->setVisible(false);
        ui->menuCommand->menuAction()->setVisible(false);
    }

    if (ui->tabWidget->tabText(index) == "Songs/Setlists/Patches") {
        ui->menuPatch->menuAction()->setVisible(true);
        fillPatchListBox(ui->patchListWidget);
    }
    else {
        ui->menuPatch->menuAction()->setVisible(false);
    }    
}

void MainWindow::on_readSysexButton_clicked()
{
    try_reconnect_MIDI();
    MyMidi->MIDI_editor_request_settings();
}

void MainWindow::on_writeSysexButton_clicked()
{
    try_reconnect_MIDI();
    startProgressBar(NUMBER_OF_DEVICES + NUMBER_OF_MIDI_SWITCHES, "Uploading settings...");
    MyMidi->MIDI_editor_send_settings();
    for (int d = 0; d < NUMBER_OF_DEVICES; d++) {
        updateProgressBar(d);
        MyMidi->MIDI_editor_send_device_settings(d);
    }
    for (int s = 0; s < NUMBER_OF_MIDI_SWITCHES; s++) {
        updateProgressBar(s + NUMBER_OF_DEVICES);
        MyMidi->MIDI_editor_send_midi_switch_settings(s);
    }
    MyMidi->MIDI_editor_send_save_settings();
    closeProgressBar("Settings upload complete.");
}

void MainWindow::on_readSysexCmdButton_clicked()
{
    try_reconnect_MIDI();
    MyMidi->MIDI_editor_request_all_commands();
}

void MainWindow::on_writeSysexCmdButton_clicked()
{
    if (Commands.size() > EXT_EEP_MAX_NUMBER_OF_COMMANDS) {
        QMessageBox msg;
        msg.critical(this, "Out of memory", "Too many commands in current configuration to fit the " + VC_name + ". Please delete some commands or pages and try again.");
        return;
    }
    try_reconnect_MIDI();
    startProgressBar(Commands.size(), "Uploading MIDI commands..");
    //MyMidi->MIDI_send_all_commands(progressBar);
    MyMidi->MIDI_editor_send_start_commands_dump();
    for (uint16_t c = 0; c < Commands.size(); c++) {
        MyMidi->MIDI_editor_send_command(c);
        updateProgressBar(c);
    }
    MyMidi->MIDI_editor_send_finish_commands_dump();
    closeProgressBar("Midi command upload complete");
}

void MainWindow::on_currentPageComboBox_activated(int index)
{
    // Select new page
    previousPage = currentPage;
    int newPage = MyVCcommands->valueFromIndex(TYPE_PAGE, index);
    if (newPage != currentPage) {
        currentPage = newPage;
        ui->currentPageComboBox2->setCurrentIndex(index);
        updateCommandScreens(false);
    }
}

void MainWindow::on_actionNewPage_triggered()
{
    if (!checkSpaceForNewPageOK()) return;
    currentPage = MyVCcommands->addPage();
    updateCommandScreens(false);
    on_actionRenamePage_triggered();
    dataEdited = true;
}

void MainWindow::on_actionRenamePage_triggered()
{
    bool ok;

    if (currentPage >= Number_of_pages) {
        QMessageBox msgBox;
        msgBox.setText("Fixed pages can not be edited");
        msgBox.exec();
        return;
    }

    QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"),
                                         tr("New page title:"), QLineEdit::Normal,
                                         MyVCcommands->customLabelString(currentPage, 0).trimmed(), &ok);

    if (ok && !text.isEmpty()) {
        MyVCcommands->setCustomLabelString(currentPage, 0, text);
        fillPageComboBox(ui->currentPageComboBox);
        fillPageComboBox(ui->currentPageComboBox2);
        dataEdited = true;
    }
}

void MainWindow::on_actionClearPage_triggered()
{
    MyVCcommands->clearPage(currentPage);
    if (currentPage > Number_of_pages) currentPage = 0;
    updateCommandScreens(false);
    qDebug() << "Page deleted";
    dataEdited = true;
}

void MainWindow::on_actionSavePage_triggered()
{
    // File Save
    QFileInfo fileInfo(MySavePageFile);
    qDebug() << MySavePageFile << fileInfo.absoluteFilePath();
    MySavePageFile = fileInfo.absoluteFilePath() + "/" + MyVCcommands->getPageName(currentPage).trimmed() + ".vcd";
    MySavePageFile = QFileDialog::getSaveFileName(this, "Save VC-edit file:", MySavePageFile, tr("VC-edit data (*.vcd)"));
    QFile saveFile(MySavePageFile);

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open json settings file.");
        return;
    }

    QJsonObject saveObject;
    writeHeader(saveObject, "PageData");
    MyVCcommands->writePage(currentPage, saveObject);
    QJsonDocument saveDoc(saveObject);
    saveFile.write(saveDoc.toJson());
    ui->statusbar->showMessage(MySavePageFile + " opened", STATUS_BAR_MESSAGE_TIME);
}

void MainWindow::on_actionEditSwitch_triggered()
{
    if (currentSwitch == -1) return;
    openEditWindow(currentSwitch, currentItem);
    dataEdited = true;
}

void MainWindow::ShowListWidgetContextMenu(const QPoint &pos)
{
    listWidgetClicked();

    customListWidget *widget = (customListWidget *)sender();
    QPoint myPos = widget->mapToGlobal(pos);

    QMenu submenu;
    submenu.addAction(ui->actionCopyCommand);
    submenu.addAction(ui->actionCutCommand);
    submenu.addAction(ui->actionPasteCommand);
    submenu.addAction(ui->actionDeleteCommand);
    submenu.addAction(ui->actionEditSwitch);
    submenu.addAction(ui->actionClearSwitch);
    if (previousSwitch != -1) {
        if (currentSwitch > previousSwitch)
            submenu.addAction("Swap switch " + QString::number(previousSwitch) + " and " + QString::number(currentSwitch));
        else
            submenu.addAction("Swap switch " + QString::number(currentSwitch) + " and " + QString::number(previousSwitch));
    }
    QAction* rightClickItem = submenu.exec(myPos);
    if ((rightClickItem) && (rightClickItem->text().left(4) == "Swap")) {
        MyVCcommands->swapSwitches(currentPage, currentSwitch, previousSwitchPage, previousSwitch);
        updateCommandScreens(false);
    }
}

void MainWindow::ShowPatchContextMenu(const QPoint &pos)
{
    customListWidget *widget = (customListWidget *)sender();
    QPoint myPos = widget->mapToGlobal(pos);

    QMenu submenu;
    submenu.addAction(ui->actionCopyPatch);
    submenu.addAction(ui->actionPastePatch);
    submenu.addAction(ui->actionEditPatch);
    submenu.addAction(ui->actionImport);
    submenu.addAction(ui->actionExport);
    submenu.addAction((ui->actionInitialize_patch));
    submenu.exec(myPos);
}

void MainWindow::on_actionDuplicatePage_triggered()
{
    if (!checkSpaceForNewPageOK()) return;
    int newPage = MyVCcommands->duplicatePage(currentPage);
    currentPage = newPage;
    updateCommandScreens(false);
    qDebug() << "Page duplicated";
    dataEdited = true;
}

void MainWindow::on_writeSysexCmdButton2_clicked()
{
    on_writeSysexCmdButton_clicked();
}

void MainWindow::on_readSysexCmdButton2_clicked()
{
    on_readSysexCmdButton_clicked();
}


void MainWindow::on_currentPageComboBox2_activated(int index)
{
    previousPage = currentPage;
    int newPage = MyVCcommands->valueFromIndex(TYPE_PAGE, index);
    if (newPage != currentPage) {
        currentPage = newPage;
        ui->currentPageComboBox->setCurrentIndex(index);
        updateCommandScreens(false);
    }
}

void MainWindow::on_actionCopyCommand_triggered()
{
    copyBufferFilled = true;
    MyVCcommands->copyItemsToBuffer(currentWidget);
}

void MainWindow::on_actionPasteCommand_triggered()
{
    MyVCcommands->pasteItem(currentSwitch);
    updateCommandScreens(false);
    dataEdited = true;
}

void MainWindow::on_actionDeleteCommand_triggered()
{
    foreach (QListWidgetItem* item, currentWidget->selectedItems()) {
        int row = item->listWidget()->row(item);
        currentItem = MyVCcommands->deleteCommand(currentPage, currentSwitch, row);
    }
    updateCommandScreens(false);
    dataEdited = true;
}

void MainWindow::startProgressBar(int size, QString message)
{
    statusLabel->hide();
    editorStateLabel->hide();
    statusBar = new QProgressBar(this);
    statusBar->setRange(0, size);
    statusBar->setTextVisible(true);
    ui->statusbar->showMessage(message, STATUS_BAR_MESSAGE_TIME);
    ui->statusbar->addPermanentWidget(statusBar);
    ui->statusbar->repaint();
}

void MainWindow::updateProgressBar(int value)
{
    statusBar->setValue(value);
    QApplication::processEvents(); // So the bar will show
}

void MainWindow::closeProgressBar(QString message)
{
    ui->statusbar->showMessage(message, STATUS_BAR_MESSAGE_TIME);
    ui->statusbar->removeWidget(statusBar);
    statusBar->close();
    statusLabel->show();
    editorStateLabel->show();
    updateStatusLabel();
}

bool MainWindow::checkSpaceForNewPageOK()
{
    if (Number_of_pages >= first_fixed_cmd_page - 1) {
        QMessageBox msg;
        msg.critical(this, "Out of page numbers", "Too many pages in current configuration to fit the VController. Please delete some other pages and try again.");
        return false;
    }
    return true;
}

void MainWindow::VControllerDetected(int type, int versionMajor, int versionMinor, int versionBuild)
{
    if (type != VCmidi_model_number) {
      QString detectedName = "Unknown device";
      if (type == 0x01) detectedName = "VController";
      if (type == 0x02) detectedName = "VC-mini";
      if (type == 0x03) detectedName = "VC-touch";
      ui->statusbar->showMessage(detectedName + " detected. VC-edit is set to " + VC_name, STATUS_BAR_MESSAGE_TIME);
        return;
    }

    QString versionString = 'v' + QString::number(versionMajor) + '.' + QString::number(versionMinor) + '.' + QString::number(versionBuild);

    ui->statusbar->showMessage(VC_name + " " + versionString + " connected", STATUS_BAR_MESSAGE_TIME);


    if ((versionMajor != VCMINI_FIRMWARE_VERSION_MAJOR) || (versionMinor != VCMINI_FIRMWARE_VERSION_MINOR)) {
        ui->statusbar->showMessage("VController version (" + QString::number(versionMajor) + "." + QString::number(versionMinor) + ") does not match version of VC-edit ("
                                   + QString::number(VCMINI_FIRMWARE_VERSION_MAJOR) + "." + QString::number(VCMINI_FIRMWARE_VERSION_MINOR) + ")", STATUS_BAR_MESSAGE_TIME);
    }
    else if ((!VControllerConnected) && (!dataEdited) && (!fileLoaded)) {
        MyMidi->MIDI_editor_request_all_commands();
        MyMidi->MIDI_editor_request_all_KTN_patches();
        MyMidi->MIDI_editor_request_settings();
    }
    VControllerConnected = true;
    updateStatusLabel();
    if (RemoteControlActive) {
        // Bring the remote control online
        MyMidi->sendSysexCommand(2, VC_REMOTE_CONTROL_ENABLE, 1);
    }
    MyMidi->MIDI_editor_request_hardware_version();
}

void MainWindow::disconnect_VC() {
    VControllerConnected = false;
    updateStatusLabel();
    if (RemoteControlActive) {
        startOfflineRemoteControl();
    }
}

void MainWindow::on_actionCutCommand_triggered()
{
    copyBufferFilled = true;
    MyVCcommands->copyItemsToBuffer(currentWidget);
    on_actionDeleteCommand_triggered();
}

void MainWindow::on_toolButtonPageDown_clicked()
{
    on_actionPreviousPage_triggered();
}

void MainWindow::on_toolButtonPageUp_clicked()
{
   on_actionNextPage_triggered();
}

void MainWindow::on_toolButtonPageDown2_clicked()
{
    on_toolButtonPageDown_clicked();
}

void MainWindow::on_toolButtonPageUp2_clicked()
{
    on_toolButtonPageUp_clicked();
}

void MainWindow::on_switch_pressed()
{
    customSwitch* sw = qobject_cast<customSwitch*>(sender());
    remoteSwitchPressed(sw->switchNumber());
}

void MainWindow::on_switch_released()
{
    customSwitch* sw = qobject_cast<customSwitch*>(sender());
    remoteSwitchReleased(sw->switchNumber());
}

void MainWindow::on_switchLabel_pressed()
{
    customDisplayLabel* lcd = qobject_cast<customDisplayLabel*>(sender());
    remoteSwitchPressed(lcd->switchNumber());
}

void MainWindow::on_switchLabel_released()
{
    customDisplayLabel* lcd = qobject_cast<customDisplayLabel*>(sender());
    remoteSwitchReleased(lcd->switchNumber());
}

void MainWindow::on_actionAbout_triggered()
{
    AboutDialog s(this, QString::number(VCMINI_FIRMWARE_VERSION_MAJOR) + "." + QString::number(VCMINI_FIRMWARE_VERSION_MINOR) + "." + QString::number(VCMINI_FIRMWARE_VERSION_BUILD));
    s.setModal(true);
    s.exec();
}

void MainWindow::on_actionWebsite_triggered()
{
    QDesktopServices::openUrl(QUrl("https://www.vguitarforums.com/smf/index.php?topic=15154.0"));
}

void MainWindow::on_actionDonate_triggered()
{
    QDesktopServices::openUrl(QUrl("https://www.paypal.me/sixeight"));
}

void MainWindow::on_actionNextPage_triggered()
{
    int index = MyVCcommands->indexFromValue(TYPE_PAGE, currentPage);
    if (index < (Number_of_pages + last_fixed_cmd_page - first_fixed_cmd_page)) index++;
    currentPage = MyVCcommands->valueFromIndex(TYPE_PAGE, index);
    updateCommandScreens(false);
}

void MainWindow::on_actionPreviousPage_triggered()
{
    int index = MyVCcommands->indexFromValue(TYPE_PAGE, currentPage);
    if (index > 0) index--;
    currentPage = MyVCcommands->valueFromIndex(TYPE_PAGE, index);
    updateCommandScreens(false);
}

void MainWindow::on_refreshSettingsTreeButton_clicked()
{
    fillTreeWidget(ui->treeWidget);
}

void MainWindow::on_readPatchButton_clicked()
{
    try_reconnect_MIDI();
    MyMidi->MIDI_editor_request_all_KTN_patches();
}

void MainWindow::on_writePatchButton_clicked()
{
    try_reconnect_MIDI();
    startProgressBar(MAX_NUMBER_OF_DEVICE_PRESETS, "Uploading Device patches...");
    for (uint16_t p = 0; p < MAX_NUMBER_OF_DEVICE_PRESETS; p++) {
        if (Device_patches[p][0] != 0) {
            MyMidi->MIDI_send_device_patch(p);
            if (MyMidiSlowMode) QThread().msleep(100);
        }
        else {
            MyMidi->MIDI_send_initialize_device_patch(p);
        }
        updateProgressBar(p);
        //QThread().msleep(5);
    }
    MyMidi->MIDI_editor_finish_device_patch_dump();
    closeProgressBar("Katana patch upload complete");
}

void MainWindow::on_actionEditPatch_triggered()
{
    // Edit patch
    bool ok;
    int item = ui->patchListWidget->currentRow();
    if (item == -1) return;
    if (currentDevicePatchType == EXT_SETLIST_TYPE) item++;

    int index = MyVCdevices->findIndex(currentDevicePatchType, item);

    uint8_t my_type;
    if (index != PATCH_INDEX_NOT_FOUND) my_type = Device_patches[index][0];
    else my_type = 0;

    if (my_type == EXT_SONG_TYPE) {
        songEditDialog sd(this, index);
        sd.exec();
        sd.close();
        fillPatchListBox(ui->patchListWidget);
        dataEdited = true;
    }

    if (my_type == EXT_SETLIST_TYPE) {
        setlistEditDialog sd(this, index, MyVCcommands);
        sd.exec();
        sd.close();
        fillPatchListBox(ui->patchListWidget);
        dataEdited = true;
    }

    if (my_type == KTN + 1) {
      QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"),
                                         tr("New patch name:"), QLineEdit::Normal,
                                         My_KTN.ReadPatchName(index).trimmed(), &ok);

      if (ok && !text.isEmpty()) {
        My_KTN.WritePatchName(index, text);
        fillPatchListBox(ui->patchListWidget);
        dataEdited = true;
      }
    }

    if ((my_type == GR55 + 1) || (my_type == SY1000 + 1)) {
        scenedialog sd(this, index);
        sd.exec();
        sd.close();
        fillPatchListBox(ui->patchListWidget);
        dataEdited = true;
    }

    // New items
    if (my_type == 0) {
        if (currentDevicePatchType == EXT_SETLIST_TYPE) {
            setlistEditDialog::createNewSetlist(item);
            on_actionEditPatch_triggered();
        }
        if (currentDevicePatchType == EXT_SONG_TYPE) {
            songEditDialog::createNewSong(item);
            on_actionEditPatch_triggered();
        }
    }
}

void MainWindow::on_actionExport_triggered()
{
    // Export patch
    int number = ui->patchListWidget->currentRow();
    if (number == -1) return;

    if (currentDevicePatchType == EXT_SETLIST_TYPE) number++; // Setlist 0 is not shown in the list.

    int index = MyVCdevices->findIndex(currentDevicePatchType, number);
    qDebug() << "Export triggered - index:" << index;
    if (index == PATCH_INDEX_NOT_FOUND) return;

    // Check if patch is empty
    int type = Device_patches[index][0];
    if (type == 0) {
        ui->statusbar->showMessage("Patch is empty", STATUS_BAR_MESSAGE_TIME);
        return;
    }

    int dev = type - 1;
    QString defaultPatchName = "patch";
    if (type == EXT_SETLIST_TYPE) defaultPatchName = setlistEditDialog::get_setlist_number_name(number) + "-" + setlistEditDialog::get_setlist_name(index).trimmed();
    if (type == EXT_SONG_TYPE) defaultPatchName = songEditDialog::get_song_number_name(number) + "-" + songEditDialog::get_song_name(index).trimmed();
    if (type <= NUMBER_OF_DEVICES) defaultPatchName = Device[dev]->DefaultPatchFileName(number);


    // File Save
    QFileInfo fileInfo(MySavePatchFile);
    qDebug() << MySavePatchFile << fileInfo.absoluteFilePath();
    MySavePatchFile = fileInfo.absoluteFilePath() + "/" + defaultPatchName + ".vcp";
    MySavePatchFile = QFileDialog::getSaveFileName(this, "Save patch:", MySavePatchFile, tr("VC-edit data (*.vcp)"));
    QFile saveFile(MySavePatchFile);

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open json settings file.");
        return;
    }

    QJsonObject saveObject;
    QString patchHeader;
    if (type == EXT_SETLIST_TYPE) {
        patchHeader = "Setlist";
        writeHeader(saveObject, patchHeader);
        setlistEditDialog::writeSetlistData(index, saveObject);
    }
    if (type == EXT_SONG_TYPE) {
        patchHeader = "Song";
        writeHeader(saveObject, patchHeader);
        songEditDialog::writeSongData(index, saveObject);
    }
    if (type <= NUMBER_OF_DEVICES) {
        patchHeader = Device[dev]->patchFileHeader() + "Patch";
        writeHeader(saveObject, patchHeader);
        MyVCdevices->writePatchData(index, saveObject);
    }
    QJsonDocument saveDoc(saveObject);
    saveFile.write(saveDoc.toJson());
    ui->statusbar->showMessage(MySavePatchFile + " saved", STATUS_BAR_MESSAGE_TIME);
}

void MainWindow::on_actionImport_triggered()
{
    // Import device patch
    int patch_no = ui->patchListWidget->currentRow();
    if (patch_no == -1) return; // Nothing selected

    if (currentDevicePatchType == 0) return; // Cannot import in memory view

    // File Open
    QString MyFile = QFileDialog::getOpenFileName(this, "Open VC-edit patch file", QFileInfo(MySavePatchFile).filePath(), tr("VC-edit data (*.vcp)"));
    QFile loadFile(MyFile);

    if (!loadFile.open(QIODevice::ReadOnly)) {
        ui->statusbar->showMessage("Couldn't open selected file.", STATUS_BAR_MESSAGE_TIME);
        return;
    }

    QByteArray saveData = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
    fileLoaded = true;
    QString jsonType = readHeader(loadDoc.object());

    int index = MyVCdevices->findIndex(currentDevicePatchType, patch_no);
    if (index == PATCH_INDEX_NOT_FOUND) index = MyVCdevices->newIndex();
    if (index == PATCH_INDEX_NOT_FOUND) return; // Out of memory

    if (jsonType == "Setlist") {
        setlistEditDialog::readSetlistData(patch_no, loadDoc.object(), currentDevicePatchType, index);
        fillPatchListBox(ui->patchListWidget);
        ui->statusbar->showMessage(MyFile + " opened", STATUS_BAR_MESSAGE_TIME);
    }
    else if (jsonType == "Song") {
        songEditDialog::readSongData(patch_no, loadDoc.object(), currentDevicePatchType, index);
        fillPatchListBox(ui->patchListWidget);
        ui->statusbar->showMessage(MyFile + " opened", STATUS_BAR_MESSAGE_TIME);
    }
    else {
        uint8_t dev = currentDevicePatchType - 1;
        QString patchHeader = Device[dev]->patchFileHeader() + "Patch";

        if (jsonType == patchHeader) {
            MyVCdevices->readPatchData(patch_no, loadDoc.object(), currentDevicePatchType);
            fillPatchListBox(ui->patchListWidget);
            ui->statusbar->showMessage(MyFile + " opened", STATUS_BAR_MESSAGE_TIME);
        }

        else {
            ui->statusbar->showMessage("Couldn't read selected VC-edit " + patchHeader + " file", STATUS_BAR_MESSAGE_TIME);
            return;
        }
    }
}

void MainWindow::on_actionCopyPatch_triggered()
{
    MyVCdevices->clearCopyBuffer();

    // Copy selected patches
    foreach (QListWidgetItem* item, ui->patchListWidget->selectedItems()) {
        int patch = item->listWidget()->row(item);
        if (patch == -1) return;

        MyVCdevices->copyPatch(patch, currentDevicePatchType);
    }
}

void MainWindow::on_actionPastePatch_triggered()
{
    // Paste patches in buffer
    int patch = ui->patchListWidget->currentRow();

    if (patch == -1) {
        return;
    }

    MyVCdevices->pastePatch(patch, currentDevicePatchType);

    fillPatchListBox(ui->patchListWidget);
    dataEdited = true;
}

void MainWindow::on_actionInitialize_patch_triggered()
{
    foreach (QListWidgetItem* item, ui->patchListWidget->selectedItems()) {
        int patch = item->listWidget()->row(item);
        if (patch == -1) return;

        MyVCdevices->initializePatch(patch, currentDevicePatchType);
    }

    fillPatchListBox(ui->patchListWidget);
    dataEdited = true;
}



void MainWindow::on_patchTypeComboBox_currentIndexChanged(int index)
{
    qDebug() << "Ïndex" << index;
    int prevType = currentDevicePatchType;
    currentDevicePatchType = 0;
    if (index == 1) currentDevicePatchType = EXT_SONG_TYPE;
    if (index == 2) currentDevicePatchType = EXT_SETLIST_TYPE;
    if (index > 2) {
        uint8_t t = 2;
        for (uint8_t dev = 0; dev < NUMBER_OF_DEVICES; dev++) {
            if (Device[dev]->supportPatchSaving() > 0) {
                t++;
                if (t == index) {
                    currentDevicePatchType = dev + 1;
                }
            }
            if (Device[dev]->supportPatchSaving() > 1) {
                t++;
                if (t == index) {
                    currentDevicePatchType = (dev + 1) | BASS_MODE_NUMBER_OFFSET;
                }
            }
        }
    }

    if (currentDevicePatchType != prevType){
        qDebug() << "Fill patchlistbox";
        fillPatchListBox(ui->patchListWidget);
    }

    ui->actionImport->setEnabled(index != 0); // Hide the Import Action when in memory mode
    ui->actionPastePatch->setEnabled(index != 0);
}



void MainWindow::on_readPatternsButton_clicked()
{
    // Request patterns
    try_reconnect_MIDI();
    MyMidi->MIDI_editor_request_sequence_patterns();
}

void MainWindow::on_writePatternsButton_clicked()
{
    // Send patterns
    try_reconnect_MIDI();
    startProgressBar(NUMBER_OF_SEQ_PATTERNS, "Uploading sequence patterns...");
    //MyMidi->MIDI_editor_send_settings();
    for (int p = 0; p < NUMBER_OF_SEQ_PATTERNS; p++) {
        updateProgressBar(p);
        MyMidi->MIDI_editor_send_seq_pattern(p);
    }
    //MyMidi->MIDI_editor_send_save_settings();
    closeProgressBar("Pattern upload complete.");
}

void MainWindow::buildMainWindow()
{
    if (VC_type == VCONTROLLER) {
        VC_name = "VController";
    }
    if (VC_type == VCMINI) {
        VC_name = "VC-mini";
    }
    if (VC_type == VCTOUCH) {
        VC_name = "VC-touch";
    }
    setWindowTitle( QCoreApplication::applicationName() + " for the " + VC_name  );

    ui->readSysexCmdButton->setText("Read commands from " + VC_name);
    ui->readSysexCmdButton2->setText("Read commands from " + VC_name);
    ui->writeSysexCmdButton->setText("Write commands to " + VC_name);
    ui->writeSysexCmdButton2->setText("Write commands to " + VC_name);
    ui->readPatchButton->setText("Read patches from " + VC_name);
    ui->writePatchButton->setText("Write patches to " + VC_name);
    ui->readSysexButton->setText("Read settings from " + VC_name);
    ui->writeSysexButton->setText("Write settings to " + VC_name);
    ui->readPatternsButton->setText("Read patterns from " + VC_name);
    ui->writePatternsButton->setText("Write patterns to " + VC_name);
}

void MainWindow::drawPageComboBoxes()
{
    // Remove all current items in the frames
    qDeleteAll(ui->pageCommandsFrame->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly));
    qDeleteAll(ui->externalSwitchCommandsFrame->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly));

    myListWidgets.clear();

    const QStringList labelTags_VController = {"Switch 1", "Switch 2", "Switch 3", "Switch 4", "Switch 5", "Switch 6", "Switch 7", "Switch 8",
                             "Switch 9", "Switch 10", "Switch 11", "Switch 12", "Switch 13", "Switch 14", "Switch 15", "Switch 16",
                             "Ext SW 1/EXP1", "Ext SW 2", "Ext SW 3/EXP2", "Ext SW 4", "Ext SW 5/EXP3", "Ext SW 6", "Ext SW 7/EXP4", "Ext SW 8"};

    const QStringList labelTags_VCmini = {"Switch 1", "Switch 2", "Switch 3", "Encoder 1", "Enc sw 1", "Encoder 2", "Enc sw 2", "Ext SW 1/EXP1",
                             "Ext SW 2", "MIDI SW1 / SW4", "MIDI SW2 / SW5", "MIDI SW3 / SW6", "MIDI switch 4", "MIDI switch 5", "MIDI switch 6", "MIDI switch 7",
                             "MIDI switch 8", "MIDI switch 9", "MIDI switch 10", "MIDI switch 11", "MIDI switch 12", "MIDI switch 13", "MIDI switch 14", "MIDI switch 15"};

    const QStringList labelTags_VCtouch = {"Switch 1", "Switch 2", "Switch 3", "Switch 4", "Switch 5", "Switch 6", "Switch 7", "Switch 8",
                             "Switch 9", "Switch 10", "Switch 11", "Switch 12", "Switch 13", "Switch 14", "Switch 15",
                             "Ext SW 1/EXP1", "Ext SW 2", "Ext SW 3/EXP2", "Ext SW 4", "Ext SW 5/EXP3", "Ext SW 6", "MIDI switch 1", "MIDI switch 2", "MIDI switch 3"};
    QStringList labelTags;
    const uint8_t VCmini_switch_order[9] = { 0, 1, 2, 3, 5, 7, 4, 6, 8};
    int numberOfRows = 4;
    int rowItems = 4;
    int numberOfRowsPg2 = 2;
    int rowItemsPg2 = 4;

    if (VC_type == VCONTROLLER) {
      labelTags.clear();
      labelTags = labelTags_VController;
      numberOfRows = 4;
      rowItems = 4;
      numberOfRowsPg2 = 2;
      rowItemsPg2 = 4;
    }

    if (VC_type == VCMINI) {
      labelTags.clear();
      labelTags = labelTags_VCmini;
      numberOfRows = 3;
      rowItems = 3;
      numberOfRowsPg2 = 3;
      rowItemsPg2 = 5;
    }

    if (VC_type == VCTOUCH) {
      labelTags.clear();
      labelTags = labelTags_VCtouch;
      numberOfRows = 3;
      rowItems = 5;
      numberOfRowsPg2 = 3;
      rowItemsPg2 = 3;
    }

    for(int i = 0; i < numberOfRows * rowItems; i++) myListWidgets.append(NULL);

    // Build pageCommandsFrame
    int switchNumber = 0;
    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui->pageCommandsFrame->layout());
    for (int row = 0; row < numberOfRows; row++) {
        QHBoxLayout* newLineLayout = new QHBoxLayout(NULL);

        for(int i = 0; i < rowItems; i++) {
            QVBoxLayout* newItemLayout = new QVBoxLayout(NULL);

            uint8_t sw = switchNumber;
            if (VC_type == VCMINI) sw = VCmini_switch_order[switchNumber];
            QLabel* label = new QLabel(labelTags[sw], ui->pageCommandsFrame);
            newItemLayout->addWidget(label);
            label->setAlignment(Qt::AlignCenter);

            customListWidget* listBox = new customListWidget(ui->pageCommandsFrame);
            newItemLayout->addWidget(listBox);
            listBox->setContentsMargins(-1, 0, -1, 0);
            //myListWidgets.append(listBox);
            myListWidgets[sw] = listBox;

            newLineLayout->addLayout(newItemLayout);
            switchNumber++;
        }
        layout->insertLayout(0, newLineLayout);
    }

    // Build externalSwitchCommandsFrame
    QVBoxLayout* layout2 = qobject_cast<QVBoxLayout*>(ui->externalSwitchCommandsFrame->layout());
    for (int row = 0; row < numberOfRowsPg2; row++) {
        QHBoxLayout* newLineLayout = new QHBoxLayout(NULL);

        for(int i = 0; i < rowItemsPg2; i++) {
            QVBoxLayout* newItemLayout = new QVBoxLayout(NULL);

            QLabel* label = new QLabel(labelTags[switchNumber++], ui->externalSwitchCommandsFrame);
            newItemLayout->addWidget(label);
            label->setAlignment(Qt::AlignCenter);

            customListWidget* listBox = new customListWidget(ui->pageCommandsFrame);
            newItemLayout->addWidget(listBox);
            listBox->setContentsMargins(-1, 0, -1, 0);
            myListWidgets.append(listBox);

            newLineLayout->addLayout(newItemLayout);
        }
        layout2->insertLayout(row, newLineLayout);
    }
}

void MainWindow::drawRemoteControlArea()
{
    //qDeleteAll(ui->remoteControlFrame->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly));
    myLCDs.clear();
    myLEDs.clear();

    if (VC_type == VCONTROLLER) drawRemoteControlAreaVController();
    if (VC_type == VCMINI) drawRemoteControlAreaVCmini();
    if (VC_type == VCTOUCH) drawRemoteControlAreaVCtouch();
}

void MainWindow::drawRemoteControlAreaVController()
{
    int numberOfRows = 3;
    int rowItems = 4;
    int switchNumber = 1;

    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui->remoteControlFrame->layout());
    for (int row = 0; row < numberOfRows; row++) {
        QHBoxLayout* newLineLayout = new QHBoxLayout(NULL);
        int switchSize = 40;
        int displayPixelSize = 8;

        for(int i = 0; i < rowItems; i++) {
            QVBoxLayout* newItemLayout = new QVBoxLayout(NULL);
            newItemLayout->setAlignment(Qt::AlignCenter);

            // Display
            customDisplayLabel* label = new customDisplayLabel(ui->remoteControlFrame);
            newItemLayout->addWidget(label, Qt::AlignCenter);
            label->setFixedSize(displayPixelSize * 25, displayPixelSize * 8);
            label->setStyleSheet("QLabel { background-color : rgb(6, 19, 59); color : white; border-style: solid; border-color: black; border-width: " + QString::number(displayPixelSize) + "px;}");
            setFont(label, 18);
            myLCDs.append(label);

            // Switch
            QHBoxLayout* newSwitchLayout = new QHBoxLayout(NULL);
            CustomLED *dummy = new CustomLED;
            dummy->setLedColor(QColor(Qt::black));
            dummy->setLedWidth(28);
            // Hide the widget
            QSizePolicy sp_retain = dummy->sizePolicy();
            sp_retain.setRetainSizeWhenHidden(true);
            dummy->setSizePolicy(sp_retain);
            dummy->hide();
            newSwitchLayout->addWidget(dummy, Qt::AlignRight);
            drawRemoteSwitch(newSwitchLayout, switchNumber++, switchSize);
            CustomLED *led = new CustomLED;
            led->setLedColor(QColor(Qt::gray));
            led->setLedWidth(28);
            newSwitchLayout->addWidget(led, Qt::AlignRight);
            myLEDs.append(led);
            newItemLayout->addLayout(newSwitchLayout);

            newLineLayout->addLayout(newItemLayout);
        }
        layout->insertLayout(1, newLineLayout);
    }
}

void MainWindow::drawRemoteControlAreaVCmini()
{
    ui->remoteControlFrame->setStyleSheet("QFrame { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 rgb(50, 50, 50), stop: 1 black);} QLabel { color : white; background-color: rgb(70 ,70 ,70)}");
    ui->remoteControlFrame->setFixedSize(550, 350);
    int rowItems = 3;
    int switchNumber = 1;

    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui->remoteControlFrame->layout());
    //layout->addSpacing(200);

    // Add three switches with LEDs
    QHBoxLayout* newLineLayout = new QHBoxLayout(NULL);
    //newLineLayout->addSpacing(100);
    int switchSize = 40;

    for(int i = 0; i < rowItems; i++) {
        QVBoxLayout* newItemLayout = new QVBoxLayout(NULL);
        newItemLayout->setAlignment(Qt::AlignCenter);

        // LED
        QHBoxLayout* newLedLayout = new QHBoxLayout(NULL);
        CustomLED *led = new CustomLED;
        led->setLedColor(QColor(Qt::gray));
        led->setLedWidth(28);
        newLedLayout->addWidget(led, Qt::AlignHCenter);
        newItemLayout->addLayout(newLedLayout, Qt::AlignHCenter);
        myLEDs.append(led);
        QHBoxLayout* newSwitchLayout = new QHBoxLayout(NULL);
        drawRemoteSwitch(newSwitchLayout, switchNumber++, switchSize);
        newItemLayout->addLayout(newSwitchLayout, Qt::AlignHCenter);

        newLineLayout->addLayout(newItemLayout);
    }
    //newLineLayout->addSpacing(100);
    layout->insertLayout(1, newLineLayout);

    // Add encoders
    int numberOfEncoders = 2;

    QHBoxLayout* newLineLayout2 = new QHBoxLayout(NULL);
    //newLineLayout2->addSpacing(100);
    for(int i = 0; i < numberOfEncoders; i++) {
        QVBoxLayout* newEncoderLayout = new QVBoxLayout(NULL);
        newEncoderLayout->setAlignment(Qt::AlignCenter);
        // Encoder
        QHBoxLayout* newSwitchLayout = new QHBoxLayout(NULL);
        drawRemoteSwitch(newSwitchLayout, switchNumber++, switchSize);
        newEncoderLayout->insertLayout(0, newSwitchLayout, Qt::AlignHCenter);
        // Label encoder
        QLabel* label = new QLabel("Encoder #" + QString::number(i + 1));
        label->setFixedHeight(15);
        newEncoderLayout->insertWidget(0, label);
        // Label encoder switch
        QLabel* label1 = new QLabel("Encoder sw#" + QString::number(i + 1));
        label1->setFixedHeight(15);
        newEncoderLayout->insertWidget(0, label1);
        // Encoder switch
        QHBoxLayout* newSwitchLayout1 = new QHBoxLayout(NULL);
        drawRemoteSwitch(newSwitchLayout1, switchNumber++, switchSize);
        newEncoderLayout->insertLayout(0, newSwitchLayout1, Qt::AlignHCenter);

        newLineLayout2->addLayout(newEncoderLayout);
    }
    //newLineLayout2->addSpacing(100);
    layout->insertLayout(1, newLineLayout2);

    ui->switch_13->hide();
    ui->switch_14->hide();
    ui->switch_15->hide();
    ui->switch_16->hide();
}

void MainWindow::drawRemoteControlAreaVCtouch()
{
    int numberOfRows = 3;
    int rowItems = 5;
    int switchNumber = 1;
    int switchSize = 40;
    int displayPixelSize = 8;

    ui->remoteControlFrame->setStyleSheet("QFrame { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 rgb(50, 50, 50), stop: 1 rgb(20, 20, 20)); "
                                          "border-style: solid; border-color: black; border-left-width: 20px; border-right-width: 20px;} "
                                          "QLabel { color : white; background-color: rgb(70 ,70 ,70)}");
    ui->remoteControlFrame->setFixedSize(displayPixelSize * rowItems * 25 + 100, displayPixelSize * 72);
    ui->remoteControlFrame->layout()->setContentsMargins(0, 0, 0, 0);
    ui->remoteControlFrame->layout()->setSpacing(0);
    ui->tab_RemoteControl->layout()->setContentsMargins(0, 0, 0, 0);
    ui->lcd0->setFixedWidth(displayPixelSize * rowItems * 25);
    ui->lcd0->setFixedHeight(100);
    ui->lcd0->setStyleSheet("QLabel { background-color : rgb(6, 19, 59); color : white; border-style: none; border-color: black; border-width: 0px;}");
    ui->switch_13->setShortcut(NULL);
    ui->switch_13->hide();
    ui->switch_14->setShortcut(NULL);
    ui->switch_14->hide();
    ui->switch_15->setShortcut(NULL);
    ui->switch_15->hide();
    ui->switch_16->setShortcut(NULL);
    ui->switch_16->hide();

    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui->remoteControlFrame->layout());

    for (int row = 0; row < numberOfRows; row++) {
        QHBoxLayout* newLineLayout = new QHBoxLayout(NULL);

        // Spacer
        auto spacer2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
        newLineLayout->addSpacerItem(spacer2);

        for(int i = 0; i < rowItems; i++) {
            QVBoxLayout* newItemLayout = new QVBoxLayout(NULL);
            newItemLayout->setAlignment(Qt::AlignCenter);

            // Display
            customDisplayLabel* label = new customDisplayLabel(ui->remoteControlFrame);
            newItemLayout->addWidget(label, Qt::AlignCenter);
            label->setFixedSize(displayPixelSize * 25, displayPixelSize * 6);
            label->setStyleSheet("QLabel { background-color : rgb(6, 19, 59); color : white; border-style: solid;border-color: white; border-width: 1px;}");
            label->setContentsMargins(0, 0, 0, 0);
            setFont(label, 18);
            label->setSwitchNumber(switchNumber++);
            myLCDs.append(label);

            newLineLayout->addLayout(newItemLayout);
            newLineLayout->addSpacing(0);
            connect(label, SIGNAL(pressed()), this, SLOT(on_switchLabel_pressed()));
            connect(label, SIGNAL(released()), this, SLOT(on_switchLabel_released()));
        }

        // Spacer
        auto spacer3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
        newLineLayout->addSpacerItem(spacer3);

        if (row == 2) layout->insertLayout(0, newLineLayout);
        else layout->insertLayout(1, newLineLayout);
    }
    switchNumber = 1;
    for (int row = 0; row < numberOfRows; row++) {
        QHBoxLayout* newLineLayout = new QHBoxLayout(NULL);
        for(int i = 0; i < rowItems; i++) {
            QVBoxLayout* newItemLayout = new QVBoxLayout(NULL);
            newItemLayout->setAlignment(Qt::AlignCenter);
            // Spacer
            auto spacer1 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
            newItemLayout->addSpacerItem(spacer1);
            // Switch
            QHBoxLayout* newSwitchLayout = new QHBoxLayout(NULL);

            drawRemoteSwitch(newSwitchLayout, switchNumber++, switchSize);

            newItemLayout->addLayout(newSwitchLayout);
            // Spacer
            auto spacer2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
            newItemLayout->addSpacerItem(spacer2);

            newLineLayout->addLayout(newItemLayout);
        }
        if (row >= 2) layout->insertLayout(0, newLineLayout);
        else layout->insertLayout(4, newLineLayout);
    }
    layout->insertSpacing(6, 80); // Space between bottom switch rows
}

void MainWindow::drawRemoteSwitch(QHBoxLayout *layout, uint8_t switchNumber, int switchSize)
{
    customSwitch* roundSwitch = new customSwitch(ui->remoteControlFrame);
    roundSwitch->setSwitchNumber(switchNumber);
    roundSwitch->setSwitchSize(switchSize);
    char myShortCut;
    if (switchNumber < 10) myShortCut = '0' + switchNumber;
    else if (switchNumber == 10) myShortCut = '0';
    else myShortCut = 'A' + switchNumber - 11;
    // Connect to shortcut
    auto action = new QAction(this);
    action->setAutoRepeat(false);
    action->setShortcuts({ QString(QChar::fromLatin1(myShortCut)) });
    this->addAction(action);
    connect(action, &QAction::triggered, [roundSwitch](){ roundSwitch->animateClick(); });

    roundSwitch->setText(QString(QChar::fromLatin1(myShortCut)));
    switchNumber++;
    layout->addWidget(roundSwitch, Qt::AlignCenter);
    connect(roundSwitch, &customSwitch::pressed, this, &MainWindow::on_switch_pressed);
    connect(roundSwitch, &customSwitch::released, this, &MainWindow::on_switch_released);
}
