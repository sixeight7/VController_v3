#include "commandeditdialog.h"
#include "ui_commandeditdialog.h"

#include <QSettings>
#include <QDebug>
#include <QMenu>
#include <QKeyEvent>
#include <QLineEdit>

commandEditDialog::commandEditDialog(QWidget *parent, VCcommands *VCc, int pg, int sw, int item) :
    QDialog(parent),
    ui(new Ui::commandEditDialog)
{
    ui->setupUi(this);
    MyVCcommands = VCc;
    connect(VCc, SIGNAL(updateEditCommandScreen()), this, SLOT(updateEditCommandScreen()));
    currentPage = pg;
    currentSwitch = sw;
    currentItem = item;
    loadWindowSettings();
    buildPage(true);
    ui->lineEditCustomLabel->installEventFilter(this);
    checkFocus();
    qDebug() << "currentItem" << currentItem;
}

commandEditDialog::~commandEditDialog()
{
    saveWindowSettings();
    ui->lineEditCustomLabel->removeEventFilter(this);
    delete ui;
}

void commandEditDialog::ShowListWidgetContextMenu(const QPoint &pos)
{
    QListWidget *widget = (QListWidget *)sender();
        QPoint item = widget->mapToGlobal(pos);

        QMenu submenu;
        submenu.addAction("New");
        submenu.addAction("Delete");
        submenu.addAction("Move Up");
        submenu.addAction("Move Down");

        QAction* rightClickItem = submenu.exec(item);
        if (rightClickItem)
        {
            if (rightClickItem->text() == "New") on_pushButtonNewCommand_clicked();
            if (rightClickItem->text() == "Delete") on_pushButtonDeleteCommand_clicked();
            if (rightClickItem->text() == "Move Up") on_toolButtonMoveUp_clicked();
            if (rightClickItem->text() == "Move Down") on_toolButtonMoveDown_clicked();
        }
}

void commandEditDialog::updateEditCommandScreen()
{
  MyVCcommands->fillCommandsListWidget(this, ui->commandListWidget, currentPage, currentSwitch, false, false);
}

void commandEditDialog::buildPage(bool first_time)
{
    rebuilding = true;

    // Page comboBox
    ui->comboBoxPage->clear();
    ui->comboBoxPage->addItem("Default Page");
    MyVCcommands->fillPageComboBox(ui->comboBoxPage);
    int index = MyVCcommands->indexFromValue(TYPE_PAGE, currentPage);
    ui->comboBoxPage->setCurrentIndex(index);

    // Switch comboBox
    ui->comboBoxSwitch->clear();
    MyVCcommands->fillSwitchComboBox(ui->comboBoxSwitch);
    ui->comboBoxSwitch->setCurrentIndex(currentSwitch);

    // Command ListWidget
    ui->commandListWidget->clear();
    MyVCcommands->fillCommandsListWidget(this, ui->commandListWidget, currentPage, currentSwitch, false, false);
    ui->commandListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->commandListWidget->setSwitchNumber(currentSwitch);
    ui->commandListWidget->setCurrentRow(currentItem);
    ui->commandListWidget->repaint(); // Listwidget did not always show current item correctly - this fixed it.
    //QApplication::processEvents();
    qDebug() << "Rebuild: currentRow set to" << currentItem;

    if (first_time) {
        ui->commandListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(ui->commandListWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ShowListWidgetContextMenu(QPoint)));
    }

    updateUpDownButtons();

    // Custom label
    ui->lineEditCustomLabel->clear();
    if (currentSwitch == 0) {
        ui->lineEditCustomLabel->setDisabled(true);
    }
    else {
        ui->lineEditCustomLabel->setDisabled(false);
        ui->lineEditCustomLabel->setText(MyVCcommands->customLabelString(currentPage, currentSwitch).trimmed());
    }

    // Fill command table
    ui->tableWidget->clear();
    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->setHorizontalHeaderLabels({"Command byte", "Slider", "Value"});
    ui->tableWidget->setColumnWidth(0, 120);
    //ui->tableWidget->setColumnWidth(1, 120);
    //ui->tableWidget->setColumnWidth(2, 120);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->setRowCount(NUMBER_OF_CMD_BYTES);
    //for (int r = 0; r < NUMBER_OF_CMD_BYTES; r++) my_widget->setRowHeight(r, 30);
    ui->tableWidget->setShowGrid(false);
    ui->tableWidget->setStyleSheet("QTableWidget::item { padding: 2px }");

    MyVCcommands->fillCommandTableWidget(ui->tableWidget, currentPage, currentSwitch, currentItem);

    ui->tableWidget->repaint();

    rebuilding = false;
    qDebug() << "Page rebuilt!";
}

void commandEditDialog::updateUpDownButtons()
{
    int numberOfItems = ui->commandListWidget->count();
    if (MyVCcommands->switchHasLabel(currentPage, currentSwitch)) {
        ui->toolButtonMoveUp->setEnabled(currentItem > 1);
        ui->toolButtonMoveDown->setEnabled((numberOfItems > 2) && (currentItem < numberOfItems - 1) && (currentItem > 0));
    }
    else {
        ui->toolButtonMoveUp->setEnabled(currentItem > 0);
        ui->toolButtonMoveDown->setEnabled((numberOfItems > 1) && (currentItem < numberOfItems - 1));
    }
}

void commandEditDialog::checkFocus()
{
    if ((MyVCcommands->switchHasLabel(currentPage, currentSwitch)) && (currentItem == 0)) {
        ui->lineEditCustomLabel->setFocus();
    }
    else if (ui->lineEditCustomLabel->hasFocus()) {
        ui->commandListWidget->setFocus();
    }
}

void commandEditDialog::loadWindowSettings()
{
    QSettings appSettings;

    appSettings.beginGroup("EditWindow");
    resize(appSettings.value("size", QSize(400, 400)).toSize());
    move(appSettings.value("pos", QPoint(200, 200)).toPoint());
    appSettings.endGroup();
}

void commandEditDialog::saveWindowSettings()
{
    QSettings appSettings;
    appSettings.beginGroup("EditWindow");
    appSettings.setValue("size", size());
    appSettings.setValue("pos", pos());
    appSettings.endGroup();
}

void commandEditDialog::on_comboBoxPage_activated(int index)
{
    if (index != currentPage) {
        MyVCcommands->checkSaved(currentPage, currentSwitch, currentItem);
        currentPage = index;
        buildPage(false);
    }
}

void commandEditDialog::on_comboBoxSwitch_activated(int index)
{
    if (index != currentSwitch) {
        currentSwitch = index;
        ui->commandListWidget->setSwitchNumber(index);
        buildPage(false);
    }
}

void commandEditDialog::on_commandListWidget_currentRowChanged(int currentRow)
{
    qDebug() << "onCurrentRowChanged" << currentRow;
    if ((currentRow != currentItem) && (currentRow >= 0)) {
        currentItem = currentRow;
        if (!rebuilding) {
            MyVCcommands->fillCommandTableWidget(ui->tableWidget, currentPage, currentSwitch, currentItem);
            updateUpDownButtons();
            checkFocus();
        }
    }
}


void commandEditDialog::on_lineEditCustomLabel_textEdited(const QString &lbl)
{
    labelEdited = true;
    if (lbl == "") {
        ui->lineEditCustomLabel->repaint();
        on_lineEditCustomLabel_editingFinished();
    }
}

void commandEditDialog::on_lineEditCustomLabel_editingFinished()
{
    QString lbl = ui->lineEditCustomLabel->text();
    //qDebug() << "New label" << lbl;
    bool labelChanged = MyVCcommands->setCustomLabelString(currentPage, currentSwitch, lbl);
    labelEdited = false;

    if (labelChanged) {
        // Update the listWidget
        ui->commandListWidget->clear();
        MyVCcommands->fillCommandsListWidget(this, ui->commandListWidget, currentPage, currentSwitch, false, false);
    }
}


void commandEditDialog::on_pushButtonNewCommand_clicked()
{
   MyVCcommands->checkSaved(currentPage, currentSwitch, currentItem);
   currentItem = MyVCcommands->createNewCommand(currentPage, currentSwitch);
   if (MyVCcommands->switchHasLabel(currentPage, currentSwitch)) currentItem++;
   buildPage(true);
   //ui->tableWidget->repaint();
}

void commandEditDialog::on_pushButtonDeleteCommand_clicked()
{
    currentItem = MyVCcommands->deleteCommand(currentPage, currentSwitch, currentItem);
    buildPage(false);
}

void commandEditDialog::on_pushButtonClose_clicked()
{
    MyVCcommands->checkSaved(currentPage, currentSwitch, currentItem);
    this->close();
}

void commandEditDialog::on_toolButtonMoveUp_clicked()
{
    if (currentItem > 0) {
        MyVCcommands->moveCommand(ui->commandListWidget, currentItem, currentItem - 1);
        currentItem--;
        buildPage(false);
    }
}

void commandEditDialog::on_toolButtonMoveDown_clicked()
{
    if (currentItem < (ui->commandListWidget->count() - 1)) {
        MyVCcommands->moveCommand(ui->commandListWidget, currentItem, currentItem + 1);
        currentItem++;
        buildPage(false);
    }
}

bool commandEditDialog::eventFilter(QObject *obj, QEvent *event)
{
    bool done = obj->eventFilter(obj, event);
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if ((keyEvent->key() == Qt::Key_Down) || (keyEvent->key() == Qt::Key_Up)) {
            ui->commandListWidget->setCurrentRow(1);
            currentItem = 1;
            ui->commandListWidget->setFocus();
            return true;
        }
    }
    return done;
}
