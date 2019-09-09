#include "mainwindow.h"

// Load the proper ui
#ifdef IS_VCMINI
#include "ui_mainwindow_VC-mini.h"
#else
#include "ui_mainwindow_VC-full.h"
#endif

#include "vceditsettingsdialog.h"
#include "midi.h"
#include "vcsettings.h"
#include "vcmidiswitchsettings.h"
#include "vcdevices.h"
#include "commandeditdialog.h"
#include "aboutdialog.h"

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

    // Setup MIDI
    MyMidi = new Midi();
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

    selectWidget(ui->switch1ListWidget);
    loadAppSettings();

    MyVCsettings = new VCsettings();
    MyVCmidiSwitches = new VCmidiSwitches();
    MyVCdevices = new VCdevices();
    MyVCcommands = new VCcommands();
    connect(MyVCcommands, SIGNAL(updateCommandScreens(bool)), this, SLOT(updateCommandScreens(bool)));

    // Fill objects on screen
    updateCommandScreens(true);
    fillTreeWidget(ui->treeWidget);
    //connect(MyVCcommands, SIGNAL(openEditWindow(int, int)), this, SLOT(openEditWindow(int, int)));

    ui->currentPageComboBox->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->currentPageComboBox, SIGNAL(customContextMenuRequested(const QPoint)), this, SLOT(ShowPageContextMenu(QPoint)));
    ui->currentPageComboBox2->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->currentPageComboBox2, SIGNAL(customContextMenuRequested(const QPoint)), this, SLOT(ShowPageContextMenu(QPoint)));
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
    MyFullBackupFile = appSettings.value("fullBackupFile").toString();
    if (MyFullBackupFile == "") MyFullBackupFile = QDir::homePath();
    MySavePageFile = appSettings.value("savePageFile").toString();
    if (MySavePageFile == "") MySavePageFile = QDir::homePath();
    currentPage = appSettings.value("currentPage").toInt();
    previousPage = currentPage;
    previousSwitchPage = currentPage;
    appSettings.endGroup();

    appSettings.beginGroup("Midi");
    MyMidiInPort = appSettings.value("midiInPort").toString();
    MyMidi->openMidiIn(MyMidiInPort);
    MyMidiOutPort = appSettings.value("midiOutPort").toString();
    MyMidi->openMidiOut(MyMidiOutPort);
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
    appSettings.setValue("currentPage", currentPage);
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
    //LCD_Solid.setPointSize(28);
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
    MyVCdevices->fillTreeWidget(my_tree, MyVCcommands);
    connect(my_tree, SIGNAL(activated(QModelIndex)), this, SLOT(treeWidgetActivated(QModelIndex)));
}

void MainWindow::treeWidgetActivated(QModelIndex)
{
    QApplication::processEvents();
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

void MainWindow::updateStatusLabel()
{
    uint8_t percentage = Commands.size() * 100 / MAX_NUMBER_OF_INTERNAL_COMMANDS;
    statusLabel->setText("Cmd memory: " + QString::number(percentage) + "%");
    if (percentage >= 95) statusLabel->setStyleSheet("background-color: orange");
    if (percentage >= 100) statusLabel->setStyleSheet("background-color: red");
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
        richText = "<html><body><p style=\"line-height:.5\">" + addNonBreakingSpaces(line1.toHtmlEscaped()) +
                "</p><p style=\"line-height:1\">" + addNonBreakingSpaces(line2.toHtmlEscaped()) + "</p></body></html>";
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

      default: colourName = "gray"; break;
    }

    // Create a stylesheet with the colour.
    QString styleSheetString = "QPushButton{background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                               "stop: 0 white, stop: 1 " + colourName + ");" "border-style: solid;" "border-color: black;"
                               "border-width: 2px;" "border-radius: 10px;}";

    // Set the colour for the switch by changing the stylesheet.
    switch (button) {
      case 1: ui->switch_1->setStyleSheet(styleSheetString); break;
      case 2: ui->switch_2->setStyleSheet(styleSheetString); break;
      case 3: ui->switch_3->setStyleSheet(styleSheetString); break;
      case 4: ui->switch_4->setStyleSheet(styleSheetString); break;
      case 5: ui->switch_5->setStyleSheet(styleSheetString); break;
      case 6: ui->switch_6->setStyleSheet(styleSheetString); break;
      case 7: ui->switch_7->setStyleSheet(styleSheetString); break;
      #ifndef IS_VCMINI
      case 8: ui->switch_8->setStyleSheet(styleSheetString); break;
      case 9: ui->switch_9->setStyleSheet(styleSheetString); break;
      case 10: ui->switch_10->setStyleSheet(styleSheetString); break;
      case 11: ui->switch_11->setStyleSheet(styleSheetString); break;
      case 12: ui->switch_12->setStyleSheet(styleSheetString); break;
      case 13: ui->switch_13->setStyleSheet(styleSheetString); break;
      case 14: ui->switch_14->setStyleSheet(styleSheetString); break;
      case 15: ui->switch_15->setStyleSheet(styleSheetString); break;
      case 16: ui->switch_16->setStyleSheet(styleSheetString); break;
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
        MyVCdevices->read(loadDoc.object());
        MyVCcommands->readAll(loadDoc.object());
        fillTreeWidget(ui->treeWidget); // Will refresh the settings in the widget
        updateCommandScreens(false);
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
    MyVCdevices->write(saveObject);
    MyVCcommands->writeAll(saveObject);
    QJsonDocument saveDoc(saveObject);
    saveFile.write(saveDoc.toJson());
    ui->statusbar->showMessage(MyFullBackupFile + " saved", STATUS_BAR_MESSAGE_TIME);
    dataEdited = false;
}

void MainWindow::writeHeader(QJsonObject &json, QString type)
{
    QJsonObject headerObject;
    headerObject["Source"] = "VC-edit";
    headerObject["Version"] = APP_VERSION;
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
    //MyMidi->openMidiIn(MyMidiInPort);
    //MyMidi->openMidiOut(MyMidiOutPort);
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
    int lastTab = ui->tabWidget->count() - 1;
    if (index == lastTab) {
        RemoteControlActive = true;
        MyMidi->sendSysexCommand(2, VC_REMOTE_CONTROL_ENABLE, 1);
        qDebug() << "Remote control enabled";
    }
    else {
        RemoteControlActive = false;
        MyMidi->sendSysexCommand(2, VC_REMOTE_CONTROL_ENABLE, 0);
        qDebug() << "Remote control disabled";
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
    ui->statusbar->showMessage(MySavePageFile + " opened", STATUS_BAR_MESSAGE_TIME);;
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
    AboutDialog s(this, APP_VERSION);
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
