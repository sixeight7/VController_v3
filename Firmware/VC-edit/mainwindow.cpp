#include "mainwindow.h"

// Load the proper ui
#ifdef IS_VCMINI
#include "ui_mainwindow_VC-mini.h"
#define DEVICENAME "VC-mini"
#else
#include "ui_mainwindow_VC-full.h"
#define DEVICENAME "VController"
#endif

#include "vceditsettingsdialog.h"
#include "midi.h"
#include "vcsettings.h"
#include "vcmidiswitchsettings.h"
#include "vcseqpattern.h"
#include "vcdevices.h"
#include "commandeditdialog.h"
#include "aboutdialog.h"
#include "scenedialog.h"

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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // Setup UI
    ui->setupUi(this);
    QCoreApplication::setOrganizationName("Sixeight Sound Control");
    QCoreApplication::setApplicationName("VC-edit");

    //QApplication::setStyle(QStyleFactory::create("Fusion"));
#ifdef IS_VCMINI
    setWindowTitle( QCoreApplication::applicationName() + " for VC-mini" );
#else
    setWindowTitle( QCoreApplication::applicationName() + " for the VController" );
#endif
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

    selectWidget(ui->switch1ListWidget);
    loadAppSettings();

    MyVCsettings = new VCsettings();
    MyVCmidiSwitches = new VCmidiSwitches();
    MyVCseqPatterns = new VCseqPattern();
    MyVCdevices = new VCdevices();
    MyVCcommands = new VCcommands();
    connect(MyVCcommands, SIGNAL(updateCommandScreens(bool)), this, SLOT(updateCommandScreens(bool)));

    // Fill objects on screen
    updateCommandScreens(true);
    fillTreeWidget(ui->treeWidget);
    fillPatchListBox(ui->patchListWidget);
    fillPatchTypeComboBox(ui->patchTypeComboBox);
    ui->patchListWidget->setDefaultDropAction(Qt::MoveAction);
    connect(ui->patchListWidget, SIGNAL(moveCommand(customListWidget*,int,int)), this, SLOT(movePatch(customListWidget*,int,int)));
    connect(ui->patchListWidget, SIGNAL(customDoubleClicked()), this, SLOT(on_actionRename_triggered()));

    ui->currentPageComboBox->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->currentPageComboBox, SIGNAL(customContextMenuRequested(const QPoint)), this, SLOT(ShowPageContextMenu(QPoint)));
    ui->currentPageComboBox2->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->currentPageComboBox2, SIGNAL(customContextMenuRequested(const QPoint)), this, SLOT(ShowPageContextMenu(QPoint)));
    ui->patchListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->patchListWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ShowPatchContextMenu(QPoint)));
    setupLcdDisplays();
    setupButtons();
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
    if (VControllerConnected) return;
    try_reconnect_MIDI();
    MyMidi->send_universal_identity_request();
}

void MainWindow::updateSettings()
{
    fillTreeWidget(ui->treeWidget);
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
    fillListBoxes(false);
}

void MainWindow::loadAppSettings() {
    QSettings appSettings;

    appSettings.beginGroup("MainWindow");
    resize(appSettings.value("size", QSize(400, 400)).toSize());
    move(appSettings.value("pos", QPoint(200, 200)).toPoint());
    ui->tabWidget->setCurrentIndex(appSettings.value("currentTab", 0).toInt());
    if (appSettings.value("hideKatanaTab").toBool()) {
        ui->tabWidget->setTabEnabled(2, false);
    }
    else {
        ui->tabWidget->setTabEnabled(2, true);
    }
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
    appSettings.endGroup();
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
    MyMidi->sendSysexCommand(2, VC_REMOTE_CONTROL_SWITCH_PRESSED, sw);
    qDebug() << "switch" << sw << "pressed";
}

void MainWindow::remoteSwitchReleased(int sw) {
    MyMidi->sendSysexCommand(2, VC_REMOTE_CONTROL_SWITCH_RELEASED, sw);
    qDebug() << "switch" << sw << "released";
}

void MainWindow::setupLcdDisplays() {
    // Setup virtual lcd_displays:
    int id = QFontDatabase::addApplicationFont(":/fonts/LCD_Solid.ttf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);
    QFont LCD_Solid(family);
    LCD_Solid.setPixelSize(28);
    ui->lcd0->setFont(LCD_Solid);
#ifndef IS_VCMINI
    //LCD_Solid.setPointSize(18);
    LCD_Solid.setPixelSize(18);
    ui->lcd1->setFont(LCD_Solid);
    ui->lcd2->setFont(LCD_Solid);
    ui->lcd3->setFont(LCD_Solid);
    ui->lcd4->setFont(LCD_Solid);
    ui->lcd5->setFont(LCD_Solid);
    ui->lcd6->setFont(LCD_Solid);
    ui->lcd7->setFont(LCD_Solid);
    ui->lcd8->setFont(LCD_Solid);
    ui->lcd9->setFont(LCD_Solid);
    ui->lcd10->setFont(LCD_Solid);
    ui->lcd11->setFont(LCD_Solid);
    ui->lcd12->setFont(LCD_Solid);
#else
    LCD_Solid.setPixelSize(56);
    ui->lcd0->setFont(LCD_Solid);
#endif
}

void MainWindow::setupButtons() {
    for (int i = 1; i <= NUMBER_OF_ON_SCREEN_BUTTONS; i++) {
        updateLcdDisplay(i, "                ", "                ");
        setButtonColour(i, 0); // Make the button grey
    }
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
    if ((currentPage >= Number_of_pages) && (currentPage < FIRST_FIXED_CMD_PAGE)) currentPage = Number_of_pages - 1;
    my_combobox->setCurrentIndex(MyVCcommands->indexFromValue(TYPE_PAGE, currentPage));
    qDebug() << "PageNumber: " << currentPage << ", index: " << MyVCcommands->indexFromValue(TYPE_PAGE, currentPage);
}

void MainWindow::fillListBoxes(bool first_time)
{
    MyVCcommands->fillCommandsListWidget(this, ui->switch1ListWidget, currentPage, 1, true, first_time);
    MyVCcommands->fillCommandsListWidget(this, ui->switch2ListWidget, currentPage, 2, true, first_time);
    MyVCcommands->fillCommandsListWidget(this, ui->switch3ListWidget, currentPage, 3, true, first_time);
    MyVCcommands->fillCommandsListWidget(this, ui->switch4ListWidget, currentPage, 4, true, first_time);
    MyVCcommands->fillCommandsListWidget(this, ui->switch5ListWidget, currentPage, 5, true, first_time);
    MyVCcommands->fillCommandsListWidget(this, ui->switch6ListWidget, currentPage, 6, true, first_time);
    MyVCcommands->fillCommandsListWidget(this, ui->switch7ListWidget, currentPage, 7, true, first_time);
    MyVCcommands->fillCommandsListWidget(this, ui->switch8ListWidget, currentPage, 8, true, first_time);
    MyVCcommands->fillCommandsListWidget(this, ui->switch9ListWidget, currentPage, 9, true, first_time);
    MyVCcommands->fillCommandsListWidget(this, ui->switch10ListWidget, currentPage, 10, true, first_time);
    MyVCcommands->fillCommandsListWidget(this, ui->switch11ListWidget, currentPage, 11, true, first_time);
    MyVCcommands->fillCommandsListWidget(this, ui->switch12ListWidget, currentPage, 12, true, first_time);
    MyVCcommands->fillCommandsListWidget(this, ui->switch13ListWidget, currentPage, 13, true, first_time);
    MyVCcommands->fillCommandsListWidget(this, ui->switch14ListWidget, currentPage, 14, true, first_time);
    MyVCcommands->fillCommandsListWidget(this, ui->switch15ListWidget, currentPage, 15, true, first_time);
    MyVCcommands->fillCommandsListWidget(this, ui->switch16ListWidget, currentPage, 16, true, first_time);
    MyVCcommands->fillCommandsListWidget(this, ui->onPageSelect0ListWidget, currentPage, 0, true, first_time);

    MyVCcommands->fillCommandsListWidget(this, ui->switch17ListWidget, currentPage, 17, true, first_time);
    MyVCcommands->fillCommandsListWidget(this, ui->switch18ListWidget, currentPage, 18, true, first_time);
    MyVCcommands->fillCommandsListWidget(this, ui->switch19ListWidget, currentPage, 19, true, first_time);
    MyVCcommands->fillCommandsListWidget(this, ui->switch20ListWidget, currentPage, 20, true, first_time);
    MyVCcommands->fillCommandsListWidget(this, ui->switch21ListWidget, currentPage, 21, true, first_time);
    MyVCcommands->fillCommandsListWidget(this, ui->switch22ListWidget, currentPage, 22, true, first_time);
    MyVCcommands->fillCommandsListWidget(this, ui->switch23ListWidget, currentPage, 23, true, first_time);
    MyVCcommands->fillCommandsListWidget(this, ui->switch24ListWidget, currentPage, 24, true, first_time);

    // Create pacman shortcuts for first and last item
    if (first_time) {
        ui->switch1ListWidget->setLeftWidget(ui->switch24ListWidget);
        ui->switch24ListWidget->setRightWidget(ui->switch1ListWidget);
    }

    updateStatusLabel();
}

void MainWindow::fillPatchListBox(QListWidget *my_patchList)
{
    my_patchList->clear();
    if (currentDevicePatchType == 0) {
        for (int p = 0; p < MAX_NUMBER_OF_DEVICE_PRESETS; p++) {
          my_patchList->addItem(MyVCdevices->ReadPatchStringForListWidget(p, 0));
      }
    }
    else {
        uint8_t dev = (currentDevicePatchType & 0xFF) - 1;
        for (int p = Device[dev]->patch_min_as_stored_on_VC; p <= Device[dev]->patch_max_as_stored_on_VC; p++) {
            my_patchList->addItem(MyVCdevices->ReadPatchStringForListWidget(p, currentDevicePatchType));
        }
    }

    updateStatusLabel();
}

void MainWindow::fillPatchTypeComboBox(QComboBox *my_combobox)
{
   my_combobox->clear();
   QString memitem = DEVICENAME;
   memitem.append(" memory");
   my_combobox->addItem(memitem);
   for (uint8_t dev = 0; dev < NUMBER_OF_DEVICES; dev++) {
       if (Device[dev]->supportPatchSaving() > 0) my_combobox->addItem(Device[dev]->full_device_name);
       if (Device[dev]->supportPatchSaving() > 1) my_combobox->addItem(Device[dev]->full_device_name + " bass mode");
   }

   QSettings appSettings;
   appSettings.beginGroup("MainWindow");
   my_combobox->setCurrentIndex(appSettings.value("currentPatchTypeComboBoxItem", 1).toInt());
   appSettings.endGroup();
}

void MainWindow::updateStatusLabel()
{
    uint8_t cmd_percentage = Commands.size() * 100 / MAX_NUMBER_OF_INTERNAL_COMMANDS;
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
    bool showPageUp = (MyVCcommands->indexFromValue(TYPE_PAGE, currentPage) < (Number_of_pages + LAST_FIXED_CMD_PAGE - FIRST_FIXED_CMD_PAGE));
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
    QString richText;
    line1.resize(16);
    line2.resize(16);
    if (lcd_no == 0) {
    #ifndef IS_VCMINI
        richText = "<html><body><p style=\"line-height:.5\">" + addNonBreakingSpaces(line1.toHtmlEscaped()) +
                "</p><p style=\"line-height:1\">" + addNonBreakingSpaces(line2.toHtmlEscaped()) + "</p></body></html>";
    #else
        richText = "<html><body><p style=\"line-height:.8\">" + addNonBreakingSpaces(line1.toHtmlEscaped()) +
                "</p><p style=\"line-height:1\">" + addNonBreakingSpaces(line2.toHtmlEscaped()) + "</p></body></html>";
    #endif
    }
    else {
        richText = "<html><body><p style=\"line-height:.3\">" + addNonBreakingSpaces(line1.toHtmlEscaped()) +
                "</p><p style=\"line-height:.6\">" + addNonBreakingSpaces(line2.toHtmlEscaped()) + "</p></body></html>";
    }
    switch (lcd_no) {
      case 0: ui->lcd0->setText(richText); break;
    #ifndef IS_VCMINI
      case 1: ui->lcd1->setText(richText); break;
      case 2: ui->lcd2->setText(richText); break;
      case 3: ui->lcd3->setText(richText); break;
      case 4: ui->lcd4->setText(richText); break;
      case 5: ui->lcd5->setText(richText); break;
      case 6: ui->lcd6->setText(richText); break;
      case 7: ui->lcd7->setText(richText); break;
      case 8: ui->lcd8->setText(richText); break;
      case 9: ui->lcd9->setText(richText); break;
      case 10: ui->lcd10->setText(richText); break;
      case 11: ui->lcd11->setText(richText); break;
      case 12: ui->lcd12->setText(richText); break;
    #endif
    }
}

void MainWindow::setButtonColour(int button, int colour) {
    QString colourName;

    // Create a string with the colourname;
    switch (colour) {
      case 1: colourName = "green"; break; // Colour 1 is Green
      case 2: colourName = "red"; break; //  Colour 2 is Red
      case 3: colourName = "blue"; break; // Colour 3 is Blue
      case 4: colourName = "rgb(255, 150, 20)"; break; // Colour 4 is Orange
      case 5: colourName = "cyan"; break; // Colour 5 is Cyan
      case 6: colourName = "rgb(200, 200, 200)"; break; // Colour 6 is White
      case 7: colourName = "yellow"; break;  // Colour 7 is Yellow
      case 8: colourName = "magenta"; break;  // Colour 8 is Magenta
      case 9: colourName = "rgb(250, 20, 147)"; break;  // Colour 9 is Pink
      case 10: colourName = "rgb(102, 240, 150)"; break; // Colour 10 is Soft green
      case 11: colourName = "rgb(0, 102, 255)"; break; // Colour 11 is Light Blue

      case 17: colourName = "darkGreen"; break;  // Colour 17 is Green dimmed
      case 18: colourName = "darkRed"; break;  //  Colour 18 is Red dimmed
      case 19: colourName = "darkBlue"; break;  // Colour 19 is Blue dimmed
      case 20: colourName = "rgb(240, 80, 0)"; break;  // Colour 20 is Orange dimmed
      case 21: colourName = "darkCyan"; break;  // Colour 21 is Cyan dimmed
      case 22: colourName = "rgb(80, 80, 80)"; break;  // Colour 22 is White dimmed
      case 23: colourName = "darkYellow"; break;   // Colour 23 is Yellow dimmed
      case 24: colourName = "darkMagenta"; break;   // Colour 24 is Magenta dimmed
      case 25: colourName = "rgb(180, 15, 100)"; break;   // Colour 25 is Pink dimmed
      case 26: colourName = "rgb(75, 150, 100)"; break; // Colour 10 is Soft green dimmed
      case 27: colourName = "rgb(0, 72, 184)"; break; // Colour 10 is Light Blue dimmed

      default: colourName = "gray"; break;
    }

    // Create a stylesheet with the colour.
    QString styleSheetString_small = "QPushButton{background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                               "stop: 0 white, stop: 1 " + colourName + ");" "border-style: solid;" "border-color: black;"
                               "border-width: 2px;" "border-radius: 10px;}";
    QString styleSheetString_large = "QPushButton{background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                               "stop: 0 white, stop: 1 " + colourName + ");" "border-style: solid;" "border-color: black;"
                               "border-width: 2px;" "border-radius: 50px;}";
    QString styleSheetString_medium = "QPushButton{background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                               "stop: 0 white, stop: 1 " + colourName + ");" "border-style: solid;" "border-color: black;"
                               "border-width: 2px;" "border-radius: 20px;}";
    // Set the colour for the switch by changing the stylesheet.
    switch (button) {
#ifdef IS_VCMINI
      case 1: ui->switch_1->setStyleSheet(styleSheetString_large); break;
      case 2: ui->switch_2->setStyleSheet(styleSheetString_large); break;
      case 3: ui->switch_3->setStyleSheet(styleSheetString_large); break;
      case 4: ui->switch_4->setStyleSheet(styleSheetString_small); break;
      case 5: ui->switch_5->setStyleSheet(styleSheetString_small); break;
      case 6: ui->switch_6->setStyleSheet(styleSheetString_small); break;
      case 7: ui->switch_7->setStyleSheet(styleSheetString_small); break;
#else
      case 1: ui->switch_1->setStyleSheet(styleSheetString_medium); break;
      case 2: ui->switch_2->setStyleSheet(styleSheetString_medium); break;
      case 3: ui->switch_3->setStyleSheet(styleSheetString_medium); break;
      case 4: ui->switch_4->setStyleSheet(styleSheetString_medium); break;
      case 5: ui->switch_5->setStyleSheet(styleSheetString_medium); break;
      case 6: ui->switch_6->setStyleSheet(styleSheetString_medium); break;
      case 7: ui->switch_7->setStyleSheet(styleSheetString_medium); break;
      case 8: ui->switch_8->setStyleSheet(styleSheetString_medium); break;
      case 9: ui->switch_9->setStyleSheet(styleSheetString_medium); break;
      case 10: ui->switch_10->setStyleSheet(styleSheetString_medium); break;
      case 11: ui->switch_11->setStyleSheet(styleSheetString_medium); break;
      case 12: ui->switch_12->setStyleSheet(styleSheetString_medium); break;
      case 13: ui->switch_13->setStyleSheet(styleSheetString_medium); break;
      case 14: ui->switch_14->setStyleSheet(styleSheetString_medium); break;
      case 15: ui->switch_15->setStyleSheet(styleSheetString_medium); break;
      case 16: ui->switch_16->setStyleSheet(styleSheetString_medium); break;
      #endif
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
    if (currentWidget->switchNumber() > 16) ui->tabWidget->setCurrentIndex(1);
    else ui->tabWidget->setCurrentIndex(0);

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
        MyVCdevices->readAll(loadDoc.object());
        MyVCdevices->readAllLegacyKatana(loadDoc.object());
        fillTreeWidget(ui->treeWidget); // Will refresh the settings in the widget
        updateCommandScreens(false);
        fillPatchListBox(ui->patchListWidget);
        ui->statusbar->showMessage(MyFile + " opened", STATUS_BAR_MESSAGE_TIME);
    }

    if (jsonType == "PageData") {
        if (currentPage < Number_of_pages) {
            if (QMessageBox::Yes == QMessageBox(QMessageBox::Information, "Loading page data", "Overwrite current page? Choose no to save data to a new page.",
                                                QMessageBox::Yes|QMessageBox::No).exec()) {
                MyVCcommands->clearPage(currentPage);
            }
            else currentPage = Number_of_pages;
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
    MyVCdevices->writeAll(saveObject);
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
    json["Header"] = headerObject;
}

QString MainWindow::readHeader(const QJsonObject &json)
{
    if (!(json.contains("Header"))) return "";
    QJsonObject headerObject = json["Header"].toObject();
    if (!(headerObject["Source"].toString() == "VC-edit")) return "";
    return headerObject["Type"].toString();
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
        if (text.at(i) == " ") output.append("&nbsp;");
        else output.append(text.at(i));
    }
    return output;
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

void MainWindow::on_switch_1_pressed()
{
    remoteSwitchPressed(1);
}

void MainWindow::on_switch_1_released()
{
    remoteSwitchReleased(1);
}

void MainWindow::on_switch_2_pressed()
{
    remoteSwitchPressed(2);
}

void MainWindow::on_switch_2_released()
{
    remoteSwitchReleased(2);
}

void MainWindow::on_switch_3_pressed()
{
    remoteSwitchPressed(3);
}

void MainWindow::on_switch_3_released()
{
    remoteSwitchReleased(3);
}

void MainWindow::on_switch_4_pressed()
{
    remoteSwitchPressed(4);
}

void MainWindow::on_switch_4_released()
{
    remoteSwitchReleased(4);
}

void MainWindow::on_switch_5_pressed()
{
    remoteSwitchPressed(5);
}

void MainWindow::on_switch_5_released()
{
    remoteSwitchReleased(5);
}

void MainWindow::on_switch_6_pressed()
{
    remoteSwitchPressed(6);
}

void MainWindow::on_switch_6_released()
{
    remoteSwitchReleased(6);
}

void MainWindow::on_switch_7_pressed()
{
    remoteSwitchPressed(7);
}

void MainWindow::on_switch_7_released()
{
    remoteSwitchReleased(7);
}

void MainWindow::on_switch_8_pressed()
{
    remoteSwitchPressed(8);
}

void MainWindow::on_switch_8_released()
{
    remoteSwitchReleased(8);
}

void MainWindow::on_switch_9_pressed()
{
    remoteSwitchPressed(9);
}

void MainWindow::on_switch_9_released()
{
    remoteSwitchReleased(9);
}

void MainWindow::on_switch_10_pressed()
{
    remoteSwitchPressed(10);
}

void MainWindow::on_switch_10_released()
{
    remoteSwitchReleased(10);
}

void MainWindow::on_switch_11_pressed()
{
    remoteSwitchPressed(11);
}

void MainWindow::on_switch_11_released()
{
    remoteSwitchReleased(11);
}

void MainWindow::on_switch_12_pressed()
{
    remoteSwitchPressed(12);
}

void MainWindow::on_switch_12_released()
{
    remoteSwitchReleased(12);
}

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
        MyMidi->sendSysexCommand(2, VC_REMOTE_CONTROL_ENABLE, 1);
        qDebug() << "Remote control enabled";
    }
    else {
        RemoteControlActive = false;
        MyMidi->sendSysexCommand(2, VC_REMOTE_CONTROL_ENABLE, 0);
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

    if (ui->tabWidget->tabText(index) == "Device patches") {
        ui->menuPatch->menuAction()->setVisible(true);
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
    if (Commands.size() > MAX_NUMBER_OF_INTERNAL_COMMANDS) {
        QMessageBox msg;
        msg.critical(this, "Out of memory", "Too many commands in current configuration to fit the VController. Please delete some commands or pages and try again.");
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
        fillListBoxes(false);
        checkMenuItems();
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
    submenu.addAction(ui->actionRename);
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
        fillListBoxes(false);
        checkMenuItems();
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
    if (Number_of_pages >= FIRST_FIXED_CMD_PAGE - 1) {
        QMessageBox msg;
        msg.critical(this, "Out of page numbers", "Too many pages in current configuration to fit the VController. Please delete some other pages and try again.");
        return false;
    }
    return true;
}

void MainWindow::VControllerDetected(int type, int versionMajor, int versionMinor, int versionBuild)
{
    if (type != VC_MODEL_NUMBER) {
#ifdef IS_VCMINI
      ui->statusbar->showMessage("VController detected. This version of VC-edit is for the VC-mini", STATUS_BAR_MESSAGE_TIME);
#else
      ui->statusbar->showMessage("VC-mini detected. This version of VC-edit is for the VController", STATUS_BAR_MESSAGE_TIME);
#endif
        return;
    }

    QString versionString = 'v' + QString::number(versionMajor) + '.' + QString::number(versionMinor) + '.' + QString::number(versionBuild);

#ifdef IS_VCMINI
    ui->statusbar->showMessage("VC-mini " + versionString + " connected", STATUS_BAR_MESSAGE_TIME);
#else
    ui->statusbar->showMessage("VController " + versionString + " connected", STATUS_BAR_MESSAGE_TIME);
#endif

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
    if (index < (Number_of_pages + LAST_FIXED_CMD_PAGE - FIRST_FIXED_CMD_PAGE)) index++;
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
        MyMidi->MIDI_send_device_patch(p);
        updateProgressBar(p);
        QThread().msleep(60);
    }
    MyMidi->MIDI_editor_finish_device_patch_dump();
    closeProgressBar("Katana patch upload complete");
}

void MainWindow::on_actionRename_triggered()
{
    // Rename patch
    bool ok;
    int item = ui->patchListWidget->currentRow();
    if (item == -1) return;

    int index = MyVCdevices->findIndex(currentDevicePatchType, item);

    uint8_t my_type = Device_patches[index][0];
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

    if (my_type == SY1000 + 1) {
        scenedialog sd(this, index);
        sd.exec();
        sd.close();
        fillPatchListBox(ui->patchListWidget);
        dataEdited = true;
    }
}

void MainWindow::on_actionExport_triggered()
{
    // Export patch
    int number = ui->patchListWidget->currentRow();
    if (number == -1) return;

    int index = MyVCdevices->findIndex(currentDevicePatchType, number);
    if (index == PATCH_INDEX_NOT_FOUND) return;

    // Check if patch is empty
    int type = Device_patches[index][0];
    if (type == 0) {
        ui->statusbar->showMessage("Patch is empty", STATUS_BAR_MESSAGE_TIME);
        return;
    }
    int dev = type - 1;

    // File Save
    QFileInfo fileInfo(MySavePatchFile);
    qDebug() << MySavePatchFile << fileInfo.absoluteFilePath();
    MySavePatchFile = fileInfo.absoluteFilePath() + "/" + Device[dev]->DefaultPatchFileName(number) + ".vcp";
    MySavePatchFile = QFileDialog::getSaveFileName(this, "Save patch:", MySavePatchFile, tr("VC-edit data (*.vcp)"));
    QFile saveFile(MySavePatchFile);

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open json settings file.");
        return;
    }

    QString patchHeader = Device[dev]->patchFileHeader() + "Patch";
    QJsonObject saveObject;
    writeHeader(saveObject, patchHeader);
    MyVCdevices->writePatchData(index, saveObject);
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

    uint8_t dev = currentDevicePatchType - 1;
    QString patchHeader = Device[dev]->patchFileHeader() + "Patch";
    QString jsonType = readHeader(loadDoc.object());
    if (jsonType == patchHeader) {
        MyVCdevices->readPatchData(patch_no, loadDoc.object(), currentDevicePatchType);
        fillPatchListBox(ui->patchListWidget);
        ui->statusbar->showMessage(MyFile + " opened", STATUS_BAR_MESSAGE_TIME);
    }
    /*else if ((currentDevicePatchType == KTN + 1) && (jsonType == "KatanaPatch")) { // Open legacy format for Katana only
        MyVCdevices->readPatchData(patch_no, loadDoc.object(), KTN + 1);
        fillPatchListBox(ui->patchListWidget);
        ui->statusbar->showMessage(MyFile + " opened", STATUS_BAR_MESSAGE_TIME);
    }*/
    else {
        ui->statusbar->showMessage("Couldn't read selected. No VC-edit " + patchHeader + " file", STATUS_BAR_MESSAGE_TIME);
        return;
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
   int prevType = currentDevicePatchType;
    currentDevicePatchType = 0;
    if (index > 0) {
        uint8_t t = 0;
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
