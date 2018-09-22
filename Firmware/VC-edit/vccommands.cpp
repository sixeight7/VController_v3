#include "vccommands.h"
#include "VController/config.h"
#include "VController/globals.h"
#include "VController/globaldevices.h"
#include "customcombobox.h"
#include "customspinbox.h"
#include "customslider.h"

#include <QDebug>
#include <QMenu>
#include <QHeaderView>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardItemModel>

/*
Commands are stored in two arrays:
* Commands[]
* Fixed_commands[]

Only the Commands[] can be edited.

To read the commands, we make use of the following indexes:
For Commands in Command[]:
* First_title[] - contains the command with the first eight title bytes of the page or switch
* First_cmd_index[] - contains the number of the first commands for this switch
* Next_cmd_index[] - contains the address of the following title or command byte in reference to the first.

For Fixed Commands:
* First_title[] - contains the command with the first eight title bytes of the page or switch
* First_cmd_index[] - contains the number of the first commands for this switch
* Next_internal_cmd_index[] - contains the address of the following title or command byte in reference to the first.

Programming commands:
* Every command has eight command bytes, together with a page and switch number
* Every byte in the command has type, value, min and max. These are stored in the cmdbyte[] array.This array is updated for
  every command that is read through update_command_structure().
* The cmdtype[] array has the data about each type of command, like name, min and max value
  and sublist (data for combobox)
* The cmd_sublist[] StringList contains the fixed items for the comboboxes. Some data is pulled from the device objects


*/

VCcommands::VCcommands(QObject *parent) : QObject(parent)
{
}

void VCcommands::recreate_indexes()
{
    create_indexes();
}

QString VCcommands::create_cmd_string(uint16_t number)
{
    Cmd_struct cmd = get_cmd(number);
    //qDebug() << "cmd-string number" << number << "page" << cmd.Page << "switch" << cmd.Switch;

    QString cmdString = cmdDeviceString(cmd.Device);
    cmdString.append("-");
    cmdString.append(cmdTypeString(cmd.Type));
    cmdString.append(" ");

    uint16_t patch_no;
    switch (cmd.Type) { // Check extra bytes:
    case PATCH_SEL:
        patch_no = (cmd.Data2 * 100) + cmd.Data1;
        if (cmd.Device < NUMBER_OF_DEVICES)
            cmdString.append(Device[cmd.Device]->number_format(patch_no));
        if (cmd.Device == CURRENT) cmdString.append(QString::number(patch_no));
        break;
    case PATCH_BANK:
    case PAR_BANK:
        cmdString.append(QString::number(cmd.Data1));
        cmdString.append("/");
        cmdString.append(QString::number(cmd.Data2));
        break;
    case PARAMETER:
        cmdString = cmdDeviceString(cmd.Device);
        cmdString.append("-PAR:");
        if (cmd.Device < NUMBER_OF_DEVICES)
            cmdString.append(Device[cmd.Device]->read_parameter_name(cmd.Data1));
        if (cmd.Device == CURRENT) cmdString.append(QString::number(cmd.Data1));
        cmdString.append(" ");
        cmdString.append(cmd_sublist.at(cmd.Data2 + 47));
        break;
    case ASSIGN:
        cmdString.append("(");
        if (cmd.Device < NUMBER_OF_DEVICES)
            cmdString.append(Device[cmd.Device]->read_assign_name(cmd.Data1));
        if (cmd.Device == CURRENT) cmdString.append(QString::number(cmd.Data1));
        cmdString.append(")");
        break;
    case SET_TEMPO:
    case BANK_DOWN:
    case BANK_UP:
    case PAR_BANK_DOWN:
    case PAR_BANK_UP:
    case PAR_BANK_CATEGORY:
    case MIDI_PC:
        cmdString.append(QString::number(cmd.Data1));
        break;
    case OPEN_PAGE:
    case OPEN_PAGE_DEVICE:
        cmdString = "SEL. PAGE ";
        cmdString.append(QString::number(cmd.Data1));
        cmdString.append(":");
        cmdString.append(read_title(cmd.Data1, 0));
        break;
    case MIDI_CC:
        cmdString.append("#");
        cmdString.append(QString::number(cmd.Data1));
        cmdString.append(" ");
        cmdString.append(cmd_sublist.at(cmd.Data2 + 82));
        break;
    case MIDI_NOTE:
        cmdString.append("#");
        cmdString.append(QString::number(cmd.Data1));
        cmdString.append(", vel:");
        cmdString.append(QString::number(cmd.Data2));
        break;
    case LOOPER:
        if (cmd.Data1 < NUMBER_OF_LOOPER_TYPES) cmdString.append(cmd_sublist[cmd.Data1 + 89]);
        break;
    case MASTER_EXP_PEDAL:
        if (cmd.Data1 < 4) cmdString.append(cmd_sublist[cmd.Data1 + 97]);
        break;
    }

    return cmdString;
}

QString VCcommands::cmdTypeString(uint8_t type)
{
    if (type < (NUMBER_OF_COMMON_TYPES + NUMBER_OF_COMMON_TYPES_NOT_SELECTABLE)) return cmd_sublist.at(type);
    if ((type >= 100) && (type < (100 + NUMBER_OF_DEVICE_TYPES + NUMBER_OF_DEVICE_TYPES_NOT_SELECTABLE))) return cmd_sublist.at(type - 100 + 17);
    return "unknown";
}

QString VCcommands::cmdDeviceString(uint8_t dev)
{
    if (dev < NUMBER_OF_DEVICES) return Device[dev]->device_name;
    if (dev == CURRENT) return "Current";
    if (dev == COMMON) return "Common";
    return "";
}

void VCcommands::fillPageComboBox(QComboBox *cbox)
{
    if (!isIndexed) create_indexes();

    for (uint8_t p = 1; p < Number_of_pages; p++) {
        QString pageName = read_title(p, 0).trimmed();
        if (pageName == "") pageName = "(empty page)";
        cbox->addItem(pageName);
        //cbox->setItemData(cbox->count() - 1, QColor( Qt::white ), Qt::BackgroundRole); // Change colour of last added item
    }
}

void VCcommands::fillFixedPageComboBox(QComboBox *cbox)
{
    for (uint8_t p = FIRST_FIXED_CMD_PAGE; p <= LAST_FIXED_CMD_PAGE; p++) {
        cbox->addItem(read_title(p, 0).trimmed());
        cbox->setItemData(cbox->count() - 1, QColor( Qt::gray ), Qt::BackgroundRole); // Change colour of last added item
    }
}

void VCcommands::fillSwitchComboBox(QComboBox *cbox)
{
    if (!isIndexed) create_indexes();
    for (uint8_t s = 0; s < (NUMBER_OF_SWITCHES + NUMBER_OF_EXTERNAL_SWITCHES + 1); s++) {
        cbox->addItem(cmd_sublist.at(s + 57));
    }
}

void VCcommands::fillCommandsListWidget(QObject *parent, customListWidget *cmdList, uint8_t pg, uint8_t sw, bool show_default_page, bool first_time)
{
    if (!isIndexed) create_indexes();

    current_page = pg;
    cmdList->setSwitchNumber(sw);
    int myRow = cmdList->currentRow();
    cmdList->clear();
    int no_of_cmds = count_cmds(pg, sw);

    if ((no_of_cmds == 0) && (show_default_page) && (sw > 0)) { // Show default page instead of selected page
        cmdList->addItem("(default page)");
        cmdList->item(0)->setFlags(cmdList->item(0)->flags()
                                   .setFlag(Qt::ItemIsEnabled, false)
                                   .setFlag(Qt::ItemIsSelectable, false));
        pg = 0;
        no_of_cmds = count_cmds(pg, sw);
        cmdList->setStyleSheet("QListWidget { background-color: rgb(215, 214, 230); }");
    }
    else { // Stay with the selected page and set colour
        if (no_of_cmds == 0) {
            cmdList->addItem("(no items)");
        }
        if (pg < Number_of_pages) cmdList->setStyleSheet("QListWidget { background-color: white; }");
        else cmdList->setStyleSheet("QListWidget { background-color: gray; }");
    }

    QString customLabel = "";
    if (sw > 0) customLabel = read_title(pg, sw); // The label for page 0 is the page name. We don't want to see that here.
    commandListBoxContainsLabel = (customLabel != "");

    if (commandListBoxContainsLabel) {
        cmdList->addItem("Label: " + customLabel );
    }

    for (uint16_t c = 0; c < no_of_cmds; c++) {
        uint16_t cmd_no = get_cmd_number(pg, sw, c);
        cmdList->addItem(create_cmd_string(cmd_no));
    }

    if (myRow >= cmdList->count()) myRow = cmdList->count() - 1;
    cmdList->setCurrentRow(myRow);

    if (first_time) {
        cmdList->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(cmdList, SIGNAL(customDoubleClicked()), parent, SLOT(on_actionEditSwitch_triggered()));
        connect(cmdList, SIGNAL(customClicked()), parent, SLOT(listWidgetClicked()));
        connect(cmdList, SIGNAL(customContextMenuRequested(QPoint)), parent, SLOT(ShowListWidgetContextMenu(QPoint)));
        connect(cmdList, SIGNAL(moveCommand(customListWidget*,int,int)), this, SLOT(moveCommand(customListWidget*,int,int)));
        connect(cmdList, SIGNAL(copyCommand(customListWidget*,int,customListWidget*)), this, SLOT(copyCommand(customListWidget*,int,customListWidget*)));
        connect(cmdList, SIGNAL(selectWidget(customListWidget*)), parent, SLOT(selectWidget(customListWidget*)));

        if (lastWidget != 0) {
            lastWidget->setRightWidget(cmdList);
            cmdList->setLeftWidget(lastWidget);
        }
        lastWidget = cmdList;
    }
}

void VCcommands::fillCommandTableWidget(QTableWidget *table, uint8_t pg, uint8_t sw, uint8_t item)
{
    MyTable = table; // So updateCommandsTableWidget() knows the pointer to the tableWidget
    current_page = pg;
    current_switch = sw;
    current_item = item;
    if ((switchHasLabel(pg, sw)) && (item > 0)) item--;

    if (count_cmds(pg, sw) == 0) return; // Exit if there are no commands for this switch

    int cmd_no = get_cmd_number(pg, sw, item);
    Cmd_struct sel_cmd = get_cmd(cmd_no);

    if (sel_cmd.Device < NUMBER_OF_DEVICES) {
        cmdbyte[0].Type = TYPE_DEVICE_SELECT;
        cmdbyte[0].Value = sel_cmd.Device;
        cmdbyte[1].Type =  TYPE_DEVICE_COMMANDS;
        cmdbyte[1].Value = sel_cmd.Type - 100;
    }
    else if (sel_cmd.Device == CURRENT) {
        cmdbyte[0].Type = TYPE_DEVICE_SELECT; // Set to current
        cmdbyte[0].Value = NUMBER_OF_DEVICES;
        cmdbyte[1].Type =  TYPE_DEVICE_COMMANDS;
        cmdbyte[1].Value = sel_cmd.Type - 100;
    }
    else { // Common device type
        cmdbyte[0].Type = TYPE_DEVICE_SELECT;
        cmdbyte[0].Value = NUMBER_OF_DEVICES + 1; // Set to common
        cmdbyte[1].Type =  TYPE_COMMON_COMMANDS;
        cmdbyte[1].Value = sel_cmd.Type;
    }
    cmdbyte[2].Value = sel_cmd.Data1;
    cmdbyte[3].Value = sel_cmd.Data2;
    cmdbyte[4].Value = sel_cmd.Value1;
    cmdbyte[5].Value = sel_cmd.Value2;
    cmdbyte[6].Value = sel_cmd.Value3;
    cmdbyte[7].Value = sel_cmd.Value4;

    for (uint8_t i = 0; i < NUMBER_OF_CMD_BYTES; i++) {
        uint8_t cmd_type = cmdbyte[i].Type;
        cmdbyte[i].Min = cmdtype[cmd_type].Min;
        cmdbyte[i].Max = cmdtype[cmd_type].Max;
    }

    qDebug() << "Command" << cmd_no << "loaded";

    for (uint8_t i = 0; i < NUMBER_OF_CMD_BYTES; i++) {
        build_command_structure(i, cmdbyte[i].Type, false);
        load_cmd_byte(table, i);
    }
}

void VCcommands::updateCommandsTableWidget()
{
    if (MyTable != nullptr) {
        MyTable->clear();
        for (uint8_t i = 0; i < NUMBER_OF_CMD_BYTES; i++) {
            build_command_structure(i, cmdbyte[i].Type, false);
            load_cmd_byte(MyTable, i);
        }
    }
    saveCommand(current_page, current_switch, current_item);
    emit updateEditCommandScreen();
    qDebug() << "Command TableWidget updated";
}

void VCcommands::setCommandTabWidget(QTabWidget *tabWidget, uint8_t pg, uint8_t sw, uint8_t item)
{
    if (count_cmds(pg, sw) == 0) return; // Exit if there are no commands for this switch

    int cmd_no = get_cmd_number(pg, sw, item);
    Cmd_struct sel_cmd = get_cmd(cmd_no);

    if (sel_cmd.Device == COMMON) tabWidget->setCurrentIndex(1);
    else tabWidget->setCurrentIndex(0);
}

void VCcommands::saveCommand(uint8_t pg, uint8_t sw, uint8_t cmd)
{
    int cmd_no = get_cmd_number(pg, sw, cmd);
    Cmd_struct my_cmd = get_cmd(cmd_no);

    if (cmdbyte[0].Value < NUMBER_OF_DEVICES) {
        my_cmd.Device = cmdbyte[0].Value;
        my_cmd.Type = cmdbyte[1].Value + 100;
    }
    if (cmdbyte[0].Value == NUMBER_OF_DEVICES) { // Current device
        my_cmd.Device = CURRENT;
        my_cmd.Type = cmdbyte[1].Value + 100;
    }
    if (cmdbyte[0].Value == NUMBER_OF_DEVICES + 1) {
        my_cmd.Device = COMMON;
        my_cmd.Type = cmdbyte[1].Value;
    }
    my_cmd.Data1 = cmdbyte[2].Value;
    my_cmd.Data2 = cmdbyte[3].Value;
    my_cmd.Value1 = cmdbyte[4].Value;
    my_cmd.Value2 = cmdbyte[5].Value;
    my_cmd.Value3 = cmdbyte[6].Value;
    my_cmd.Value4 = cmdbyte[7].Value;

    write_cmd(cmd_no, my_cmd);
    //updateCommandsTableWidget();
    command_edited = false;
}

void VCcommands::checkSaved(uint8_t pg, uint8_t sw, uint8_t cmd)
{
    if (!command_edited) return;

    //if (QMessageBox::Yes == QMessageBox(QMessageBox::Information, "Data not saved..", "Command has been edited. Do you want to save it?",
    //                                    QMessageBox::Yes|QMessageBox::No).exec()) {
    saveCommand(pg, sw, cmd);
    //}
}

int VCcommands::createNewCommand(uint8_t pg, uint8_t sw)
{
    int new_cmd_no = count_cmds(pg, sw);
    //qDebug() << "New command number" << new_cmd_no;
    Cmd_struct default_cmd = {pg, sw, NOTHING, COMMON, 0, 0, 0, 0, 0, 0};
    uint16_t new_index = new_command_index();
    write_cmd(new_index, default_cmd);
    create_indexes();
    return new_cmd_no;
}

int VCcommands::deleteCommand(uint8_t pg, uint8_t sw, uint8_t item)
{
    if (read_title(pg, sw) != "") {
        if (item == 0) { // Label selected
            write_title(pg, sw, ""); // Will delete the label
            qDebug() << "deleteCommand - label";
            return 0;
        }
        item--;
    }
    int cmd_no = get_cmd_number(pg, sw, item);
    delete_cmd(cmd_no);

    if ((pg == PAGE_DEFAULT) && (count_cmds(pg, sw) == 0)) createNewCommand(pg, sw); // default page can never be empty!

    command_edited = false;
    if ((item > 0) && (item >= count_cmds(pg, sw))) item--;
    qDebug() << "deleteCommand - number" << item;
    return item;
}

QString VCcommands::customLabelString(uint8_t pg, uint8_t sw)
{
    return read_title(pg, sw);
}

bool VCcommands::setCustomLabelString(uint8_t pg, uint8_t sw, QString label)
{
    if (label.trimmed() != read_title(pg, sw).trimmed()) {
        qDebug() << "Set label" << label;
        write_title(pg, sw, label);
        return true;
    }
    return false;
}

int VCcommands::valueFromIndex(uint8_t type, uint16_t index) // Recreate the gap in the pagenumbers
{
    switch (type) {
    case TYPE_PAGE:
        if (index < Number_of_pages) return index;
        else return (index - Number_of_pages + FIRST_FIXED_CMD_PAGE);
        break;
    case TYPE_MIDI_PORT:
        if (index == NUMBER_OF_MIDI_PORTS) return 15;
        else return index;
        break;
    default:
        return index;
    }
}

int VCcommands::indexFromValue(uint8_t type, uint8_t value) // Remove the gap in the pagenumbers
{
    switch (type) {
    case TYPE_PAGE:
        if (value < Number_of_pages) return value;
        else return (value - FIRST_FIXED_CMD_PAGE + Number_of_pages);
        break;
    case TYPE_MIDI_PORT:
        if (value < NUMBER_OF_MIDI_PORTS) return value;
        else return NUMBER_OF_MIDI_PORTS; // The index to all ports
        break;
    default:
        return value;
    }
    return false;
}

void VCcommands::copyCommand(customListWidget *sourceWidget, int sourceRow, customListWidget *destWidget)
{
    int sourcePage = current_page;
    int sourceSwitch = sourceWidget->switchNumber();
    int destSwitch = destWidget->switchNumber();
    if ((sourceSwitch == -1) || (destSwitch == -1)) return;
    bool hasLabel = switchHasLabel(current_page, sourceSwitch);

    foreach (QListWidgetItem* item, sourceWidget->selectedItems()) {
        sourceRow = item->listWidget()->row(item);

        if ((hasLabel) && (sourceRow == 0)) { // Copy label
            if (destSwitch == 0) return; // Cannot drag label to on_page_select switch as this label contains the page name
            QString label = read_title(current_page, sourceSwitch);
            write_title(current_page, destSwitch, label);
        }
        else {
            if (hasLabel) sourceRow--;

            if (switchShowsDefaultItems(sourcePage, sourceSwitch)) {
                sourcePage = 0;
                if (sourceRow > 0) sourceRow--;
            }

            uint16_t cmd_no = get_cmd_number(sourcePage, sourceSwitch, sourceRow);
            Cmd_struct cmd = Commands[cmd_no];

            cmd.Page = current_page;
            cmd.Switch = destSwitch;

            Commands.append(cmd);
        }
    }

    create_indexes();

    sourceWidget->setCurrentRow(-1); // Clears selected items
    destWidget->setFocus();
    destWidget->activateWindow();
    int lastRow = destWidget->count();

    emit updateCommandScreens(false);

    destWidget->setCurrentRow(lastRow); // Have to do this after the command screens are updated, otherwise the row does not exist.

    qDebug() << "Copied Command from page" << sourcePage << ", switch" << sourceSwitch << ", item" << sourceRow
             << "to switch" << destSwitch;
}

void VCcommands::moveCommand(customListWidget *widget, int sourceRow, int destRow)
{
    int sw = widget->switchNumber();
    if (sw == -1) return;

    QString firstLine = widget->item(0)->text();
    if ((firstLine == "(default page)") || (firstLine.left(6) == "Label:")) {
        if ((sourceRow == 0) || (destRow == 0)) return;  // Cannot move labels or default page indicators
        sourceRow--;
        destRow--;
    }

    uint16_t sourceIndex = get_cmd_number(current_page, sw, sourceRow);
    uint16_t destindex = get_cmd_number(current_page, sw, destRow);
    Commands.move(sourceIndex, destindex);
    create_indexes();
    emit updateCommandScreens(false);

    qDebug() << "Moved" << widget->objectName() << "item" << sourceRow << "to" << destRow;
}

void VCcommands::cmdByteDataChanged(int cmd_byte_no, int cmd_type, int index)
{
    command_edited = true;
    if (cmdtype[cmd_type].Sublist != 0)
        cmdbyte[cmd_byte_no].Value = valueFromIndex(cmd_type, index + cmdbyte[cmd_byte_no].Min);
    else cmdbyte[cmd_byte_no].Value = index;
    build_command_structure(cmd_byte_no, cmd_type, true);
    //for (int i = cmd_byte_no + 1; i < NUMBER_OF_CMD_BYTES; i++) build_command_structure(i, cmd_type, false);
    updateCommandsTableWidget();
}

void VCcommands::swapSwitches(int pg1, int sw1, int pg2, int sw2)
{
    qDebug() << "Swapping switch #" << sw1 << "on page" << pg1 << "with switch#" << sw2 << "on page" << pg2;
    bool noSwitchZero = (sw1 != 0) && (sw2 != 0);

    for (int c = 0; c < number_of_cmds; c++) {
        // Swap commands
        if ((Commands[c].Page == pg1) && (Commands[c].Switch == sw1)) {
            Commands[c].Page = pg2;
            Commands[c].Switch = sw2;
        }
        else if ((Commands[c].Page == pg2) && (Commands[c].Switch == sw2)) {
            Commands[c].Page = pg1;
            Commands[c].Switch = sw1;
        }

        // Swap label
        if (noSwitchZero) { // Cannot swap labels for switch 0, as its label contains the page name
            if ((Commands[c].Page == pg1) && (Commands[c].Switch == sw1 + LABEL)) {
                Commands[c].Page = pg2;
                Commands[c].Switch = sw2 + LABEL;
            }
            else if ((Commands[c].Page == pg2) && (Commands[c].Switch == sw2 + LABEL)) {
                Commands[c].Page = pg1;
                Commands[c].Switch = sw1 + LABEL;
            }
        }
    }
    create_indexes();
}

void VCcommands::copyItemsToBuffer(customListWidget *widget)
{   
    int sw = widget->switchNumber();
    if ((sw == 0) && (count_cmds(current_page, sw) == 0)) return; // Cannot copy from empty on-page_select field
    copyBuffer.clear();
    copyBufferContainsLabel = false;
    bool hasLabel = switchHasLabel(current_page, sw);

    foreach (QListWidgetItem* item, widget->selectedItems()) {
        int row = item->listWidget()->row(item);
        qDebug() << "Copying row" << row;

        if (hasLabel && (row == 0)) {
            copyBufferContainsLabel = true;
            copyBufferLabel = read_title(current_page, sw);
        }
        else {
            if (hasLabel) row--;
            uint16_t cmd_no = get_cmd_number_check_default(current_page, sw, row);
            if (cmd_no & INTERNAL_CMD) copyBuffer.append(Fixed_commands[cmd_no - INTERNAL_CMD]);
            else copyBuffer.append(Commands[cmd_no]);
        }
    }
    qDebug() << "Items in bufer:" << copyBuffer.size();
    copyBufferFilled = true;
}

void VCcommands::pasteItem(int sw)
{
    if (!copyBufferFilled) return;

    if (copyBufferContainsLabel) {
        if (sw == 0) return; // Cannot paste label to on_page_select switch as this label contains the page name
        write_title(current_page, sw, copyBufferLabel);
    }


    for (int i = 0; i < copyBuffer.size(); i++) {
        Cmd_struct cmd = copyBuffer[i];
        cmd.Page = current_page;
        cmd.Switch = sw;

        Commands.append(cmd);
    }

    create_indexes();
}

uint8_t VCcommands::duplicatePage(int pg)
{
    uint8_t newPage = Number_of_pages;
    Number_of_pages++;

    for (int s = 0; s < NUMBER_OF_SWITCHES + NUMBER_OF_EXTERNAL_SWITCHES; s++) {
        for (int c = 0; c < count_cmds(pg, s); c++) {
            uint16_t cmd_no = get_cmd_number(pg, s, c);
            Cmd_struct cmd = get_cmd(cmd_no);
            cmd.Page = newPage;
            Commands.append(cmd);
        }

        QString switchLabel = read_title(pg, s);
        if (switchLabel != "") write_title(newPage, s, switchLabel);
    }

    QString pageName = read_title(pg, 0).trimmed();
    //if (pageName.length() < 14) pageName.append(" 2");
    pageName = addNewNumber(pageName);
    write_title(newPage, 0, pageName);

    create_indexes();

    return newPage;
}

void VCcommands::clearPage(int pg)
{
    for (int c = Commands.size(); c --> 0;) {
        if (Commands[c].Page == pg) Commands.removeAt(c);
    }
    create_indexes();
}

int VCcommands::addPage()
{
    int new_page = Number_of_pages;
    write_title(new_page, 0, "New Page");
    create_indexes();
    return new_page;
}

void VCcommands::readAll(const QJsonObject &json)
{
    if (json.contains("Commands") && json["Commands"].isArray()) {
        QJsonArray commandArray = json["Commands"].toArray();
        Commands.clear();
        for (int c = 0; c < commandArray.size(); ++c) {
            QJsonObject commandObject = commandArray[c].toObject();
            readCommand(commandObject, -1);
        }
        create_indexes();
    }
}

void VCcommands::readPage(int pg, const QJsonObject &json)
{
    if (json.contains("Commands") && json["Commands"].isArray()) {
        write_title(pg, 0, ""); // Clear title first
        QJsonArray commandArray = json["Commands"].toArray();
        for (int c = 0; c < commandArray.size(); ++c) {
            QJsonObject commandObject = commandArray[c].toObject();
            readCommand(commandObject, pg);
        }
        create_indexes();
    }
}

void VCcommands::readCommand(QJsonObject &json, int pgOverride)
{
    Cmd_struct cmd;
    if (pgOverride == -1) cmd.Page = json["Page"].toInt();
    else cmd.Page = pgOverride;
    cmd.Switch = json["Switch"].toInt();
    cmd.Device = json["Device"].toInt();
    cmd.Type = json["Type"].toInt();
    cmd.Data1 = json["Data1"].toInt();
    cmd.Data2 = json["Data2"].toInt();
    cmd.Value1 = json["Value1"].toInt();
    cmd.Value2 = json["Value2"].toInt();
    cmd.Value3 = json["Value3"].toInt();
    cmd.Value4 = json["Value4"].toInt();
    Commands.append(cmd);
}

void VCcommands::writeAll(QJsonObject &json) const
{
    QJsonArray commandArray;
    for (int c = 0; c < Commands.size(); c++) {
        QJsonObject commandObject;
        writeCommand(c, commandObject);
        commandArray.append(commandObject);
    }
    json["Commands"] = commandArray;
}

void VCcommands::writePage(int pg, QJsonObject &json)
{
    QJsonArray commandArray;
    for (int c = 0; c < Commands.size(); c++) {
        if (Commands[c].Page == pg) {
            QJsonObject commandObject;
            writeCommand(c, commandObject);
            commandArray.append(commandObject);
        }
    }
    json["Commands"] = commandArray;
}



void VCcommands::writeCommand(uint16_t cmd_no, QJsonObject &json) const
{
    Cmd_struct cmd = Commands[cmd_no];
    json["Page"] = cmd.Page;
    json["Switch"] = cmd.Switch;
    json["Device"] = cmd.Device;
    json["Type"] = cmd.Type;
    json["Data1"] = cmd.Data1;
    json["Data2"] = cmd.Data2;
    json["Value1"] = cmd.Value1;
    json["Value2"] = cmd.Value2;
    json["Value3"] = cmd.Value3;
    json["Value4"] = cmd.Value4;
}

QString VCcommands::getPageName(int pg)
{
    return read_title(pg, 0);
}



bool VCcommands::switchHasLabel(int pg, int sw)
{
    if (sw == -1) return false;
    return (Title_index[pg][sw] != 0);
}

bool VCcommands::switchShowsDefaultItems(int pg, int sw)
{
    if (sw <= 0) return false;
    return (count_cmds(pg, sw) == 0);
}

//******************************************************************************

void VCcommands::create_indexes()
{
    // Clear indexes first
    memset(First_cmd_index, 0, sizeof(First_cmd_index));
    memset(Next_cmd_index, 0, sizeof(Next_cmd_index));
    memset(Next_internal_cmd_index, 0, sizeof(Next_internal_cmd_index));
    memset(Title_index, 0, sizeof(Title_index));

    Cmd_struct cmd;

    // Fill the indexes with external commands
    Number_of_pages = 0;
    for (uint16_t c = Commands.size(); c-- > 0; ) { //Run backwards through the EEPROM command array
        cmd = Commands[c];
        if (cmd.Page >= Number_of_pages) Number_of_pages = cmd.Page + 1; //update the number of pages
        if (cmd.Switch & LABEL) { // Check if it is a name label
            uint16_t first_title = Title_index[cmd.Page][cmd.Switch - LABEL];
            if (first_title != 0) {// Title array is already filled!!!
                Next_cmd_index[c] = first_title; // Move first title to the Next_cmd_index
            }
            Title_index[cmd.Page][cmd.Switch - LABEL] = c; // Add to the Title index
        }
        else { // It is a command
            uint16_t first_cmd = First_cmd_index[cmd.Page][cmd.Switch];
            if (first_cmd != 0) {// First command array is already filled!!!
                Next_cmd_index[c] = first_cmd; // Move first command to the Next_cmd_index
            }
            First_cmd_index[cmd.Page][cmd.Switch] = c; // Store the first command
        }
    }

    // Fill the indexes with internal commands
    for (uint16_t c = NUMBER_OF_INTERNAL_COMMANDS; c-- > 0; ) { //Run backwards through the EEPROM command array
        uint8_t pg = Fixed_commands[c].Page;
        uint8_t sw = Fixed_commands[c].Switch;

        if (sw & LABEL) { // Check if it is a name label
            uint16_t first_title = Title_index[pg][sw - LABEL];
            if (first_title != 0) {// Title array is already filled!!!
                Next_internal_cmd_index[c] = first_title; // Move first title to the Next_internal_cmd_index
            }
            if (pg >= FIRST_FIXED_CMD_PAGE) Title_index[pg][sw - LABEL] = c | INTERNAL_CMD; // Add to the Title index
        }
        else { // It is a command
            uint16_t first_cmd = First_cmd_index[pg][sw];
            if (first_cmd != 0) {// First command array is already filled!!!
                Next_internal_cmd_index[c] = first_cmd; // Move first command to the Next_internal_cmd_index
            }
            if (pg >= FIRST_FIXED_CMD_PAGE) First_cmd_index[pg][sw] = c | INTERNAL_CMD; // Store the first command
        }
    }

    isIndexed = true;
    qDebug() << "Indexes created";
}

uint16_t VCcommands::new_command_index()
{
    ; // we now have a new command
    return Commands.size();
}

void VCcommands::delete_cmd(uint16_t number)
{
    Commands.removeAt(number);
    create_indexes();
    qDebug() << "Clear command" << number;
}

Cmd_struct VCcommands::get_cmd(uint16_t number)
{
    if (number & INTERNAL_CMD) return Fixed_commands[number - INTERNAL_CMD];
    else return Commands[number];
}

void VCcommands::write_cmd(uint16_t number, Cmd_struct &cmd)
{
    if (number & INTERNAL_CMD) return; // We can not edit the fixed commands

    if (number >= Commands.size()) {
        Commands.append(cmd);
        qDebug() << "Wrote new command " << number << ":" << cmd.Page << cmd.Switch << cmd.Device << cmd.Type << cmd.Data1 << cmd.Data2 << cmd.Value1 << cmd.Value2 << cmd.Value3 << cmd.Value4;
    }
    else {
        Commands[number] = cmd;
        qDebug() << "Wrote command" << number << ":" << cmd.Page << cmd.Switch << cmd.Device << cmd.Type << cmd.Data1 << cmd.Data2 << cmd.Value1 << cmd.Value2 << cmd.Value3 << cmd.Value4;
    }
}

uint16_t VCcommands::get_cmd_number(uint8_t pg, uint8_t sw, uint8_t number)
{
    uint16_t i = First_cmd_index[pg][sw];
    uint16_t prev_i = i;
    //if (i == 0) i = First_cmd_index[PAGE_DEFAULT][sw]; // Read from default page if no command is found for this switch

    if (i & INTERNAL_CMD) { // Read fixed cmd
        for (uint8_t n = 0; n < number; n++) {
            i = Next_internal_cmd_index[i - INTERNAL_CMD]; // Find the right command
            if (i == 0) i = prev_i; // Oh dear, command is empty - revert to the previous command
            else prev_i = i;
        }
        return (i);
    }

    else { // Read use programmable cmd
        for (uint8_t n = 0; n < number; n++) {
            i = Next_cmd_index[i]; // Find the right command
            if (i == 0) i = prev_i; // Oh dear, command is empty - revert to the previous command
            else prev_i = i;
        }
        return (i);
    }
}

uint16_t VCcommands::get_cmd_number_check_default(uint8_t pg, uint8_t sw, uint8_t number)
{
    if (First_cmd_index[pg][sw] == 0) return get_cmd_number(0, sw, number);
    else return get_cmd_number(pg, sw, number);
}

uint16_t VCcommands::count_cmds(uint8_t pg, uint8_t sw)
{
    if ((pg >= Number_of_pages) && (pg < FIRST_FIXED_CMD_PAGE)) return 0; // Just in case we are creating a new page

    uint16_t i = First_cmd_index[pg][sw];

    if ((pg == 0) && (sw == 1)) return 1; // First command has index 0

    if (i == 0) return 0; // No commands for this switch

    uint8_t number = 1;
    if (i & INTERNAL_CMD) {
        while (Next_internal_cmd_index[i - INTERNAL_CMD] != 0) {
            i = Next_internal_cmd_index[i - INTERNAL_CMD];
            number++;
        }
    }
    else {
        while (Next_cmd_index[i] != 0) {
            i = Next_cmd_index[i];
            number++;
        }
    }

    return number;
}

QString VCcommands::read_title(uint8_t pg, uint8_t sw)
{
    Cmd_struct cmd;
    QString title = "";
    uint16_t i = Title_index[pg][sw];
    if (i > 0) {
        cmd = get_cmd(i);

        uint8_t* cmdbytes = (uint8_t*)&cmd;
        for (unsigned int i = 2; i < sizeof(cmd); i++) title.append((QChar)cmdbytes[i]);

        if (i & INTERNAL_CMD) i = Next_internal_cmd_index[i - INTERNAL_CMD];
        else i = Next_cmd_index[i];

        if (i != 0) { // Read second part of title
            cmd = get_cmd(i);

            uint8_t* cmdbytes = (uint8_t*)&cmd;
            for (unsigned int i = 2; i < sizeof(cmd); i++) title.append((QChar)cmdbytes[i]);
        }
    }
    return title;
}

void VCcommands::write_title(uint8_t pg, uint8_t sw, QString title)
{
    if (pg >= FIRST_FIXED_CMD_PAGE) return;

    Cmd_struct cmd;
    cmd.Page = pg;
    cmd.Switch = sw | LABEL;
    uint8_t* cmdbytes = (uint8_t*)&cmd;
    uint8_t len = title.length();
    if (len > 8) len = 8;
    for (uint8_t i = 0; i < len; i++) cmdbytes[i + 2] = title.at(i).unicode();
    for (uint8_t i = len; i < 8; i++) cmdbytes[i + 2] = 32; // ascii number for space

    uint16_t cmd_index = Title_index[pg][sw];
    if (len == 0) { // title is empty
        // Clear cmd bytes if they exist
        if (Next_cmd_index[cmd_index] != 0) delete_cmd(Next_cmd_index[cmd_index]);
        if (cmd_index != 0) delete_cmd(cmd_index);
        goto cleanup;
    }
    if (cmd_index == 0) cmd_index = new_command_index(); // Create new command if neccesary
    write_cmd(cmd_index, cmd);

    if (title.length() > 8) { //Add second part if string is longer than 8 characters
        len = title.length() - 8;
        if (len > 8) len = 8;
        for (unsigned int i = 0; i < len; i++) cmdbytes[i + 2] = title.at(i + 8).unicode();
        for (uint8_t i = len; i < 8; i++) cmdbytes[i + 2] = 32;  //ascii number for space

        cmd_index = Next_cmd_index[cmd_index]; // Find next cmd
        if (cmd_index == 0) cmd_index = new_command_index();
        write_cmd(cmd_index, cmd);
    }
    else {
        cmd_index = Next_cmd_index[cmd_index]; // Find next cmd
        if (cmd_index != 0) delete_cmd(cmd_index);
    }
cleanup:
    create_indexes();
}

void VCcommands::set_type_and_value(uint8_t number, uint8_t type, uint8_t index, bool in_edit_mode)
{
    cmdbyte[number].Type = type;
    cmdbyte[number].Title = cmdtype[type].Title;
    cmdbyte[number].Max = cmdtype[type].Max;
    cmdbyte[number].Min = cmdtype[type].Min;
    if (in_edit_mode) {
        cmdbyte[number].Value = index;
    }
}

void VCcommands::clear_cmd_bytes(uint8_t start_byte, bool in_edit_mode)
{
    for (uint8_t b = start_byte; b < 8; b++) {
        set_type_and_value(b, TYPE_OFF, 0, in_edit_mode);
        cmdbyte[b].Max = 0;
        cmdbyte[b].Min = 0;
    }
}

void VCcommands::build_command_structure(uint8_t cmd_byte_no, uint8_t cmd_type, bool in_edit_mode)
{
    // Will update the status of the other command fields, according to the device, type, or number of parameters
    // Function is called after a command is loaded or the switch of the command has been pressed:

    uint8_t dev = cmdbyte[0].Value;

    //cmdbyte[cmd_byte_no].Min = cmdtype[cmd_type].Min;
    //cmdbyte[cmd_byte_no].Max = cmdtype[cmd_type].Max;

    // *****************************************
    // * BYTE1: Device byte updated            *
    // *****************************************
    if (cmd_byte_no == 0) { // The first byte (device select) has been changed
        if (cmdbyte[0].Value == NUMBER_OF_DEVICES + 1) { // If Device is "Common"
            // Set command to: COMMON, NONE
            set_type_and_value(1, TYPE_COMMON_COMMANDS, selected_common_cmd, in_edit_mode);

            build_command_structure(1, cmdbyte[1].Type, false);

            //clear_cmd_bytes(2, in_edit_mode); // Clear bytes 2-7
        }
        else { // Device is not "common"
            // Set command to: <selected device>, PATCH_SEL, <current_patch_number>
            set_type_and_value(1, TYPE_DEVICE_COMMANDS, selected_device_cmd, in_edit_mode);

            build_command_structure(1, cmdbyte[1].Type, false);

            if (selected_device_cmd == PATCH_SEL - 100) {
                uint16_t patch_no = 0;
                if (dev < NUMBER_OF_DEVICES) patch_no = Device[dev]->patch_number;
                set_type_and_value(2, TYPE_PATCH_NUMBER, patch_no % 100, in_edit_mode);
                set_type_and_value(3, TYPE_PATCH_100, patch_no / 100, in_edit_mode);
                //clear_cmd_bytes(4, in_edit_mode); // Clear bytes 2-7
            }
        }
        //reload_cmd_menus();
        //update_page = REFRESH_PAGE;
    }

    if (cmd_byte_no == 1) {
        // *****************************************
        // * BYTE2: Common command byte updated    *
        // *****************************************
        if (cmd_type == TYPE_COMMON_COMMANDS) { // The common command byte has been changed
            selected_common_cmd = cmdbyte[cmd_byte_no].Value;
            switch (selected_common_cmd) {
            case OPEN_PAGE:
                // Command: COMMON, OPEN_PAGE, 0
                set_type_and_value(2, TYPE_PAGE, 0, in_edit_mode);
                clear_cmd_bytes(3, in_edit_mode); // Clear bytes 3-7
                break;
            case MIDI_PC:
                // Command: COMMON, MIDI_PC, NUMBER, CHANNEL, PORT
                set_type_and_value(2, TYPE_PC, 0, in_edit_mode);
                set_type_and_value(3, TYPE_MIDI_CHANNEL, 1, in_edit_mode);
                set_type_and_value(4, TYPE_MIDI_PORT, 0, in_edit_mode);
                clear_cmd_bytes(5, in_edit_mode); // Clear bytes 4-7
                break;
            case MIDI_CC:
                // Command: COMMON, CC_number, CC_TOGGLE_TYPE, Value1, Value2, Channel, Port
                set_type_and_value(2, TYPE_CC_NUMBER, 0, in_edit_mode);
                set_type_and_value(3, TYPE_CC_TOGGLE, 0, in_edit_mode);
                set_type_and_value(4, TYPE_MAX, 127, in_edit_mode);
                set_type_and_value(5, TYPE_MIN, 0, in_edit_mode);
                set_type_and_value(6, TYPE_MIDI_CHANNEL, 1, in_edit_mode);
                set_type_and_value(7, TYPE_MIDI_PORT, 0, in_edit_mode);
                break;
            case MIDI_NOTE:
                // Command: COMMON, MIDI_NOTE, NUMBER, VELOCITY, CHANNEL, PORT
                set_type_and_value(2, TYPE_NOTE_NUMBER, 0, in_edit_mode);
                set_type_and_value(3, TYPE_NOTE_VELOCITY, 100, in_edit_mode);
                set_type_and_value(4, TYPE_MIDI_CHANNEL, 1, in_edit_mode);
                set_type_and_value(5, TYPE_MIDI_PORT, 0, in_edit_mode);
                clear_cmd_bytes(6, in_edit_mode); // Clear bytes 6-7
                break;
            case SET_TEMPO:
                // Command: COMMON, SET_TEMPO, number
                set_type_and_value(2, TYPE_BPM, Setting.Bpm, in_edit_mode);
                clear_cmd_bytes(3, in_edit_mode); // Clear bytes 3-7
                break;
            default:
                // For all two byte commands
                clear_cmd_bytes(2, in_edit_mode); // Clear bytes 2-7
                break;
            }
            //reload_cmd_menus();
            //update_page = REFRESH_PAGE;
        }

        // *****************************************
        // * BYTE2: Device command byte updated    *
        // *****************************************
        if (cmd_type == TYPE_DEVICE_COMMANDS) {
            uint16_t patch_no = 0;
            uint8_t my_trigger = 0;
            selected_device_cmd = cmdbyte[cmd_byte_no].Value;
            switch (selected_device_cmd + 100) {
            case PATCH_SEL:
                // Command: <selected device>, PATCH_SEL, <current_patch_number>
                if (dev < NUMBER_OF_DEVICES) patch_no = Device[dev]->patch_number;
                set_type_and_value(2, TYPE_PATCH_NUMBER, patch_no % 100, in_edit_mode);
                set_type_and_value(3, TYPE_PATCH_100, patch_no / 100, in_edit_mode);
                if (dev < NUMBER_OF_DEVICES) {
                    cmdbyte[2].Max = Device[dev]->patch_max % 100;
                    cmdbyte[3].Max = Device[dev]->patch_max / 100;
                    cmdbyte[2].Min = Device[dev]->patch_min % 100;
                    cmdbyte[3].Min = Device[dev]->patch_min / 100;
                }
                clear_cmd_bytes(4, in_edit_mode); // Clear bytes 3-7
                break;
            case PATCH_BANK:
                // Command: <selected device>, PATCH_BANK, NUMBER, BANK_SIZE
                set_type_and_value(2, TYPE_REL_NUMBER, 1, in_edit_mode);
                set_type_and_value(3, TYPE_BANK_SIZE, 10, in_edit_mode);
                clear_cmd_bytes(4, in_edit_mode); // Clear bytes 3-7
                break;
            case BANK_UP:
            case BANK_DOWN:
                // Command: <selected device>, BANK_UP/DOWN, BANK_SIZE
                set_type_and_value(2, TYPE_BANK_SIZE, 10, in_edit_mode);
                clear_cmd_bytes(3, in_edit_mode); // Clear bytes 3-7
                break;
            case PARAMETER:
                // Command: <selected device>, PARAMETER, NUMBER, TOGGLE, VALUE 1, VALUE 2
                set_type_and_value(2, TYPE_PARAMETER, 0, in_edit_mode);
                if (dev < NUMBER_OF_DEVICES) cmdbyte[2].Max = Device[dev]->number_of_parameters() - 1;
                set_type_and_value(3, TYPE_TOGGLE, 1, in_edit_mode);
                set_type_and_value(4, TYPE_PAR_STATE, 1, in_edit_mode);
                if (dev < NUMBER_OF_DEVICES) {
                    cmdbyte[4].Max = Device[dev]->max_value(cmdbyte[2].Value);
                    cmdbyte[4].Min = Device[dev]->min_value(cmdbyte[2].Value);
                }
                set_type_and_value(5, TYPE_PAR_STATE, 0, in_edit_mode);
                if (dev < NUMBER_OF_DEVICES) {
                    cmdbyte[5].Max = Device[dev]->max_value(cmdbyte[2].Value);
                    cmdbyte[5].Min = Device[dev]->min_value(cmdbyte[2].Value);
                }
                clear_cmd_bytes(6, in_edit_mode); // Clear bytes 6-7
                break;
            case ASSIGN:
                // Command: <selected device>, ASSIGN, NUMBER
                set_type_and_value(2, TYPE_ASSIGN, 0, in_edit_mode);
                if (dev < NUMBER_OF_DEVICES) {
                    if (Device[dev]->get_number_of_assigns() > 0)
                        cmdbyte[2].Max = Device[dev]->get_number_of_assigns() - 1;
                    else
                        cmdbyte[2].Max = 0;
                    my_trigger = Device[dev]->trigger_follow_assign(cmdbyte[2].Value);
                }
                else my_trigger = 0;
                set_type_and_value(3, TYPE_ASSIGN_TRIGGER, my_trigger, in_edit_mode);
                clear_cmd_bytes(4, in_edit_mode); // Clear bytes 3-7
                break;
            case OPEN_PAGE_DEVICE:
                // Command: COMMON, OPEN_PAGE, 0
                set_type_and_value(2, TYPE_PAGE, 0, in_edit_mode);
                clear_cmd_bytes(3, in_edit_mode); // Clear bytes 3-7
                break;
            case SNAPSCENE:
                // Command: COMMON, SNAPSCENE, 0
                set_type_and_value(2, TYPE_SNAPSCENE, 1, in_edit_mode);
                clear_cmd_bytes(3, in_edit_mode); // Clear bytes 3-7
                break;
            case LOOPER:
                // Command: COMMON, LOOPER, 0
                set_type_and_value(2, TYPE_LOOPER, 2, in_edit_mode);
                clear_cmd_bytes(3, in_edit_mode); // Clear bytes 3-7
                break;
            case MASTER_EXP_PEDAL:
                // Command: <selected device>, MASTER_EXP_EDAL, 0
                set_type_and_value(2, TYPE_EXP_PEDAL, 2, in_edit_mode);
                clear_cmd_bytes(3, in_edit_mode); // Clear bytes 3-7
                break;
            default:
                clear_cmd_bytes(2, in_edit_mode); // Clear bytes 2-7
                break;
            }
            //reload_cmd_menus();
            //update_page = REFRESH_PAGE;
        }
    }

    if (cmd_byte_no == 2) {
        // *****************************************
        // * BYTE3: Assign byte updated            *
        // *****************************************
        if (cmd_type == TYPE_ASSIGN) {
            if (dev < NUMBER_OF_DEVICES) {
                uint8_t my_trigger = Device[dev]->trigger_follow_assign(cmdbyte[2].Value);
                if (in_edit_mode) cmdbyte[3].Value = my_trigger;
            }
        }

        // *****************************************
        // * BYTE3: Page byte updated            *
        // *****************************************
        //if (cmd_type == TYPE_PAGE) {
        //  if (cmdbyte[cmd_byte_no].Value == Number_of_pages) cmdbyte[cmd_byte_no].Value = FIRST_FIXED_CMD_PAGE; //Jump over the gap between the external and internal pages
        //  if (cmdbyte[cmd_byte_no].Value == FIRST_FIXED_CMD_PAGE - 1) cmdbyte[cmd_byte_no].Value = Number_of_pages - 1;
        //}
    }

    // *****************************************
    // * BYTE4: Toggle type byte updated       *
    // *****************************************
    if (cmd_type == TYPE_TOGGLE) {
        uint8_t max = 1;
        uint8_t min = 0;
        if (dev < NUMBER_OF_DEVICES) {
            max = Device[dev]->max_value(cmdbyte[2].Value);
            min = Device[dev]->min_value(cmdbyte[2].Value);
        }
        switch (cmdbyte[cmd_byte_no].Value) {
        case MOMENTARY:
        case TOGGLE:
            cmdbyte[4].Title = "ON-VALUE";
            cmdbyte[5].Title = "OFF-VALUE";
            break;
        case TRISTATE:
            // Command: <selected device>, PARAMETER, NUMBER, TRISTATE, VALUE 1, VALUE 2, VALUE3
            cmdbyte[6].Max = max;
            cmdbyte[6].Min = min;
            set_type_and_value(6, TYPE_PAR_STATE, max, in_edit_mode);
            clear_cmd_bytes(7, in_edit_mode);
            break;
        case FOURSTATE:
            // Command: <selected device>, PARAMETER, NUMBER, TRISTATE, VALUE 1, VALUE 2, VALUE3
            cmdbyte[6].Max = max;
            cmdbyte[6].Min = min;
            cmdbyte[7].Max = max;
            cmdbyte[7].Min = min;
            set_type_and_value(6, TYPE_PAR_STATE, max, in_edit_mode);
            set_type_and_value(7, TYPE_PAR_STATE, max, in_edit_mode);
            break;
        case STEP:
            // Command: <selected device>, PARAMETER, NUMBER, TRISTATE, VALUE 1, VALUE 2, VALUE3
            cmdbyte[4].Title = "MIN VALUE";
            cmdbyte[5].Title = "MAX VALUE";
            set_type_and_value(6, TYPE_STEP, 1, in_edit_mode);
            clear_cmd_bytes(7, in_edit_mode);
            break;
        case RANGE:
        case UPDOWN:
            cmdbyte[4].Title = "MIN VALUE";
            cmdbyte[5].Title = "MAX VALUE";
            break;

        default:
            clear_cmd_bytes(6, in_edit_mode);
        }
    }
}

void VCcommands::load_cmd_byte(QTableWidget *table, uint8_t cmd_byte_no)
{
    uint8_t cmd_type = cmdbyte[cmd_byte_no].Type;
    uint8_t max = cmdbyte[cmd_byte_no].Max;
    uint8_t min = cmdbyte[cmd_byte_no].Min;
    uint8_t value = cmdbyte[cmd_byte_no].Value;

    if (value > max) {
        value = max;
        cmdbyte[cmd_byte_no].Value = max;
        for (int i = cmd_byte_no; i < NUMBER_OF_CMD_BYTES; i++) build_command_structure(i, cmd_type, true);
    }
    if (value < min) {
        value = min;
        cmdbyte[cmd_byte_no].Value = min;
        for (int i = cmd_byte_no; i < NUMBER_OF_CMD_BYTES; i++) build_command_structure(i, cmd_type, true);
    }

    if (cmdtype[cmd_type].Sublist == 0) { // Show sublist if neccesary
        if (max > 0) {
            table->setItem(cmd_byte_no, 0, new QTableWidgetItem(cmdbyte[cmd_byte_no].Title)); // Set the cmd title
            CustomSpinBox *spinBox = new CustomSpinBox(table, cmd_byte_no, cmd_type);
            spinBox->setRange(min, max);
            spinBox->setValue(value);
            table->setCellWidget(cmd_byte_no, 2, spinBox);
            connect(spinBox, SIGNAL(new_value(int, int, int)), this, SLOT(cmdByteDataChanged(int, int, int)));
            CustomSlider *slider = new CustomSlider(table, cmd_byte_no, cmd_type);
            slider->setRange(min, max);
            slider->setValue(value);
            table->setCellWidget(cmd_byte_no, 1, slider);
            connect(slider, SIGNAL(new_value(int, int, int)), this, SLOT(cmdByteDataChanged(int, int, int)));
            connect(spinBox, SIGNAL(valueChanged(int)), slider, SLOT(setValue(int)));
            connect(slider, SIGNAL(valueChanged(int)), spinBox, SLOT(setValue(int)));
        }
        else {
            table->setItem(cmd_byte_no, 0, new QTableWidgetItem(""));
            table->removeCellWidget(cmd_byte_no, 1);
            table->removeCellWidget(cmd_byte_no, 2);
        }
    }
    else {
        table->setItem(cmd_byte_no, 0, new QTableWidgetItem(cmdbyte[cmd_byte_no].Title)); // Set the cmd title
        CustomComboBox *comboBox = new CustomComboBox(table, cmd_byte_no, cmd_type);
        for (int i = indexFromValue(cmd_type, min); i <= indexFromValue(cmd_type, max); i++) {
            comboBox->addItem(read_cmd_sublist(cmd_byte_no, valueFromIndex(cmd_type, i - min)));
        }

        // Disable items for devices that are not supported
        if ((cmdbyte[0].Value < NUMBER_OF_DEVICES) && (cmd_byte_no == 1)) {
            const QStandardItemModel* model = qobject_cast<const QStandardItemModel*>(comboBox->model());
            for (int i = indexFromValue(cmd_type, min); i <= indexFromValue(cmd_type, max); i++) {
              QStandardItem* item = model->item(i - min);
              item->setEnabled(Device[cmdbyte[0].Value]->check_command_enabled(i + 100));
            }
        }
        comboBox->setCurrentIndex(indexFromValue(cmd_type, value - min));
        table->setCellWidget(cmd_byte_no, 2, comboBox);
        qDebug() << "set comboBoxWidget" << cmd_byte_no;
        connect(comboBox, SIGNAL(new_value(int, int, int)), this, SLOT(cmdByteDataChanged(int, int, int)));
        CustomSlider *slider = new CustomSlider(table, cmd_byte_no, cmd_type);
        slider->setRange(min, max);
        slider->setValue(value);
        table->setCellWidget(cmd_byte_no, 1, slider);
        connect(slider, SIGNAL(new_value(int, int, int)), this, SLOT(cmdByteDataChanged(int, int, int)));
        connect(comboBox, SIGNAL(currentIndexChanged(int)), slider, SLOT(setValue(int)));
        connect(slider, SIGNAL(valueChanged(int)), comboBox, SLOT(setCurrentIndex(int)));
    }

}

QString VCcommands::read_cmd_sublist(uint8_t cmd_byte_no, uint8_t value)
{
    QString msg, msg1;
    uint8_t index;
    uint8_t dev = cmdbyte[0].Value;
    uint8_t page_index;
    //uint8_t sel_switch;
    //uint8_t sel_cmd_no;
    //Cmd_struct sel_cmd;
    uint16_t patch_no, bank_low, bank_high;
    uint8_t cmd_type = cmdbyte[cmd_byte_no].Type;

    switch (cmdtype[cmd_type].Sublist) { // Show sublist if neccesary
    case 0: // If Sublist is 0, we will show the value unless Max value is zero.
        if (cmdbyte[cmd_byte_no].Max > 0) {
            msg = QString::number(value);
        }
        else { // Or return empty label if max value is zero
            msg = "";
        }
        break;
    case SUBLIST_PATCH: // Copy the patchname from the device
        msg = "";
        patch_no = value + (cmdbyte[3].Value * 100);
        if (dev < NUMBER_OF_DEVICES) msg = Device[dev]->number_format(patch_no);
        else msg = QString::number(patch_no);
        break;
    case SUBLIST_PATCH_BANK:
        bank_low = value * 100;
        bank_high = bank_low + 99;
        if (dev < NUMBER_OF_DEVICES) {
            if (bank_high > Device[dev]->patch_max) bank_high = Device[dev]->patch_max;
            msg = Device[dev]->number_format(bank_low) + " - " + Device[dev]->number_format(bank_high);
        }
        else msg = QString::number(bank_low) + " - " + QString::number(bank_high);
        break;
    case SUBLIST_PARAMETER:  // Copy the parameter name from the device
        dev = cmdbyte[0].Value;
        msg = "";
        if (dev < NUMBER_OF_DEVICES) msg = Device[dev]->read_parameter_name(value);
        else msg = QString::number(value);
        break;
    case SUBLIST_PAR_STATE: // Copy the parameter state from the device
        dev = cmdbyte[0].Value;
        msg = "";
        if (dev < NUMBER_OF_DEVICES) msg = Device[dev]->read_parameter_state(cmdbyte[2].Value, value);
        else msg = QString::number(value);
        break;
    case SUBLIST_ASSIGN:  // Copy the assign name from the device
        dev = cmdbyte[0].Value;
        msg = "";
        if (dev < NUMBER_OF_DEVICES) msg = Device[dev]->read_assign_name(value);
        else msg = QString::number(value);
        break;
    case SUBLIST_TRIGGER:  // Copy the assign trigger name from the device
        dev = cmdbyte[0].Value;
        msg = "";
        if (dev < NUMBER_OF_DEVICES) msg = Device[dev]->read_assign_trigger(value);
        else msg = QString::number(value);
        break;
    case SUBLIST_PAGE: // Find the page name in EEPROM
        page_index = indexFromValue(cmd_type, value);
        qDebug() << "Page number:" << value << ", index:" << page_index;
        if (page_index == 0) msg = "0: Default";
        else if (page_index < Number_of_pages) {
            msg = QString::number(page_index) + ": " + read_title(value, 0); // Read page name
        }
        else if ((page_index >= Number_of_pages) && (page_index < (Number_of_pages + LAST_FIXED_CMD_PAGE - FIRST_FIXED_CMD_PAGE + 1))) {
            msg = "F" + QString::number(page_index) + ": " + read_title(value, 0);
        }
        else msg = QString::number(page_index) + ": New page";
        break;
    case SUBLIST_DEVICES:
        dev = value;
        if (dev < NUMBER_OF_DEVICES) msg = Device[dev]->full_device_name;
        else if (dev == NUMBER_OF_DEVICES) msg = "Current Device";
        else if (dev == NUMBER_OF_DEVICES + 1) msg = "Common Functions";
        else msg = "Unknown device";
        break;
    default: // Static sublist - read it from the cmd_sublist array
        index = indexFromValue(cmd_type, value) + cmdtype[cmd_type].Sublist;
        if (index > 0) msg = cmd_sublist[index - 1];
        break;
    }
    return msg;
}

QString VCcommands::addNewNumber(QString inputString)
{ // Checks if right of inputString contains a number and increments it. If no number exists, it will add number 2.
    int currentChar = inputString.length() - 1;
    int number;
    while ((inputString.at(currentChar).isDigit()) && (currentChar > 0)) {
        currentChar--;
    }
    if (currentChar >= inputString.length() - 1) number = 2;
    else number = inputString.right(inputString.length() - currentChar - 1).toInt() + 1;

    QString numberString = QString::number(number);

    if (currentChar >= LCD_DISPLAY_SIZE - numberString.size()) currentChar = LCD_DISPLAY_SIZE - numberString.size() - 1;

    return inputString.left(currentChar + 1) + numberString;
}

