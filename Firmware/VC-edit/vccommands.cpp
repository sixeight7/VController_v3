#include "vccommands.h"
#include "VController/config.h"
#include "VController/globals.h"
#include "VController/globaldevices.h"
#include "customcombobox.h"
#include "customspinbox.h"
#include "customslider.h"
#include "vccommands.h"

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

void VCcommands::setup_VC_config()
{
    if (VC_type == VCONTROLLER) {
        Fixed_commands = VC_fixed_commands;
        number_of_cmds = VC_fixed_commands.size();
        first_fixed_cmd_page = FIRST_FIXED_CMD_PAGE_VC;
        last_fixed_cmd_page = LAST_FIXED_CMD_PAGE_VC;
        Commands = VC_commands;
        page_default = PAGE_VC_DEFAULT;
        number_of_midi_ports = VC_NUMBER_OF_MIDI_PORTS;
        midi_port_names.clear();
        midi_port_names.append(VC_PORT1_NAME);
        midi_port_names.append(VC_PORT2_NAME);
        midi_port_names.append(VC_PORT3_NAME);
        midi_port_names.append(VC_PORT4_NAME);
        midi_port_names.append(VC_PORT5_NAME);
        midi_port_names.append(VC_PORT6_NAME);
        midi_port_names.append(VC_PORT7_NAME);
        midi_port_names.append(VC_PORT8_NAME);
        midi_port_names.append(VC_PORT9_NAME);
        switchnames = VC_switch_names;
        VCmidi_model_number = 0x01;
    }
    if (VC_type == VCMINI) {
        Fixed_commands = VCmini_fixed_commands;
        number_of_cmds = VCmini_fixed_commands.size();
        first_fixed_cmd_page = FIRST_FIXED_CMD_PAGE_VCMINI;
        last_fixed_cmd_page = LAST_FIXED_CMD_PAGE_VCMINI;
        Commands = VCmini_commands;
        page_default = PAGE_VCMINI_DEFAULT;
        number_of_midi_ports = VCMINI_NUMBER_OF_MIDI_PORTS;
        midi_port_names.clear();
        midi_port_names.append(VCMINI_PORT1_NAME);
        midi_port_names.append(VCMINI_PORT2_NAME);
        midi_port_names.append(VCMINI_PORT3_NAME);
        midi_port_names.append(VCMINI_PORT4_NAME);
        midi_port_names.append(VCMINI_PORT5_NAME);
        midi_port_names.append(VCMINI_PORT6_NAME);
        midi_port_names.append(VCMINI_PORT7_NAME);
        midi_port_names.append(VCMINI_PORT8_NAME);
        midi_port_names.append(VCMINI_PORT9_NAME);
        switchnames = VCmini_switch_names;
        VCmidi_model_number = 0x02;
    }
    if (VC_type == VCTOUCH) {
        Fixed_commands = VCtouch_fixed_commands;
        number_of_cmds = VCtouch_fixed_commands.size();
        first_fixed_cmd_page = FIRST_FIXED_CMD_PAGE_VCTOUCH;
        last_fixed_cmd_page = LAST_FIXED_CMD_PAGE_VCTOUCH;
        page_default = PAGE_VCTOUCH_DEFAULT;
        Commands = VCtouch_commands;
        number_of_midi_ports = VCTOUCH_NUMBER_OF_MIDI_PORTS;
        midi_port_names.clear();
        midi_port_names.append(VCTOUCH_PORT1_NAME);
        midi_port_names.append(VCTOUCH_PORT2_NAME);
        midi_port_names.append(VCTOUCH_PORT3_NAME);
        midi_port_names.append(VCTOUCH_PORT4_NAME);
        midi_port_names.append(VCTOUCH_PORT5_NAME);
        midi_port_names.append(VCTOUCH_PORT6_NAME);
        midi_port_names.append(VCTOUCH_PORT7_NAME);
        midi_port_names.append(VCTOUCH_PORT8_NAME);
        midi_port_names.append(VCTOUCH_PORT9_NAME);
        switchnames = VCtouch_switch_names;
        VCmidi_model_number = 0x03;
    }
}

void VCcommands::recreate_indexes()
{
    create_indexes();
}

QString VCcommands::create_cmd_string(uint16_t number) // Strings shown on main page in the switch boxes
{
    Cmd_struct cmd = get_cmd(number);

    QString cmdString = cmdSwitchTypeString(cmd.Switch);
    cmdString.append(cmdDeviceString(cmd.Device));
    cmdString.append("-");
    cmdString.append(cmdTypeString(cmd.Type));
    cmdString.append(" ");

    const QStringList ShortCmdTypes = {"SEL", "BANKSEL", "BANK UP", "BANK DOWN", "NEXT", "PREV", "", "",};

    uint16_t patch_no;
    switch (cmd.Type) { // Check extra bytes:
    case PATCH:
        cmdString.append(ShortCmdTypes[cmd.Data1]); // Pick it from the sublist...
        cmdString.append(" ");
        if (cmd.Data1 == SELECT) {
            patch_no = (cmd.Value1 * 100) + cmd.Data2;
            if (cmd.Device < NUMBER_OF_DEVICES)
                cmdString.append(Device[cmd.Device]->number_format(patch_no));
            if (cmd.Device == CURRENT) cmdString.append(QString::number(patch_no));
        }
        if (cmd.Data1 == BANKSELECT) {
            cmdString.append(QString::number(cmd.Data2));
            cmdString.append("/");
            cmdString.append(QString::number(cmd.Value1));
        }
        break;
    case PAR_BANK:
        cmdString.append(QString::number(cmd.Data1));
        cmdString.append("/");
        cmdString.append(QString::number(cmd.Data2));
        break;
    case PAR_BANK_CATEGORY:
        cmdString = cmdSwitchTypeString(cmd.Switch);
        cmdString.append(cmdDeviceString(cmd.Device));
        cmdString.append("-CATEGORY ");
        cmdString.append(QString::number(cmd.Data1));
        break;
    case PARAMETER:
        cmdString = cmdSwitchTypeString(cmd.Switch);
        cmdString.append(cmdDeviceString(cmd.Device));
        cmdString.append("-PAR:");
        if (cmd.Device < NUMBER_OF_DEVICES)
            cmdString.append(Device[cmd.Device]->read_parameter_name(cmd.Data1));
        if (cmd.Device == CURRENT) cmdString.append(QString::number(cmd.Data1));
        cmdString.append(" ");
        cmdString.append(cmd_sublist.at(cmd.Data2 + 47));
        break;
    case ASSIGN:
        cmdString.append(ShortCmdTypes[cmd.Data1]); // Pick it from the sublist...
        cmdString.append(" ");
        if (cmd.Data1 == SELECT) {
            cmdString.append("(");
            if (cmd.Device < NUMBER_OF_DEVICES)
                cmdString.append(Device[cmd.Device]->read_assign_name(cmd.Data1));
            if (cmd.Device == CURRENT) cmdString.append(QString::number(cmd.Data1));
            cmdString.append(")");
        }
        if (cmd.Data1 == BANKSELECT) {
            cmdString.append(QString::number(cmd.Data2));
            cmdString.append("/");
            cmdString.append(QString::number(cmd.Value1));
        }
        break;
    case SNAPSCENE:
        cmdString.append(QString::number(cmd.Data1));
        if (cmd.Data2 != 0) cmdString.append('/' + QString::number(cmd.Data2));
        if (cmd.Value1 != 0) cmdString.append('/' + QString::number(cmd.Value1));
        break;
    case SET_TEMPO:
    case PAR_BANK_DOWN:
    case PAR_BANK_UP:
    case MIDI_PC:
        cmdString.append(QString::number(cmd.Data1));
        break;
    case PAGE:
        cmdString.append(ShortCmdTypes[cmd.Data1]); // Pick it from the sublist...
        cmdString.append(" ");
        if (cmd.Data1 == SELECT) {
            cmdString.append(QString::number(cmd.Data2));
            cmdString.append(":");
            cmdString.append(read_title(cmd.Data2, 0));
        }
        if (cmd.Data1 == BANKSELECT) {
            cmdString.append(QString::number(cmd.Data2));
            cmdString.append("/");
            cmdString.append(QString::number(cmd.Value1));
        }
        break;
    case OPEN_PAGE_DEVICE:
        cmdString = cmdSwitchTypeString(cmd.Switch);
        cmdString.append(cmdDeviceString(cmd.Device));
        cmdString.append("-PAGE:");
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
    case MENU:
        if (cmd.Data1 == MENU_SELECT) cmdString.append("SELECT");
        else if (cmd.Data1 == MENU_BACK) cmdString.append("BACK");
        else if (cmd.Data1 == MENU_SET_VALUE) cmdString.append("SET VALUE");
        else if (cmd.Data1 == MENU_PREV) cmdString.append("PREV");
        else if (cmd.Data1 == MENU_NEXT) cmdString.append("NEXT");
        else cmdString.append(QString::number(cmd.Data1));
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
    if (dev == CURRENT) return "CUR";
    if (dev == COMMON) return "COM";
    return "";
}

QString VCcommands::cmdSwitchTypeString(uint8_t sw)
{
    switch(sw & SWITCH_TYPE_MASK) {
        case 0: return "P:";
        case ON_RELEASE: return "R:";
        case ON_LONG_PRESS: return "LP:";
        case ON_DUAL_PRESS: return "DP:";
        case ON_DUAL_PRESS | ON_RELEASE: return "DR:";
        case ON_DUAL_PRESS | ON_LONG_PRESS: return "DLP:";
        default: return "?:";
      }
}

void VCcommands::fillPageComboBox(QComboBox *cbox)
{
    if (!isIndexed) create_indexes();

    for (uint8_t p = 1; p < Number_of_pages; p++) {
        QString pageName = read_title(p, 0).trimmed();
        if (pageName == "") pageName = "Page " + QString::number(p); // Use default page name
        cbox->addItem(pageName);
        //cbox->setItemData(cbox->count() - 1, QColor( Qt::white ), Qt::BackgroundRole); // Change colour of last added item
    }
}

void VCcommands::fillFixedPageComboBox(QComboBox *cbox)
{
    for (uint8_t p = first_fixed_cmd_page; p <= last_fixed_cmd_page; p++) {
        cbox->addItem(read_title(p, 0).trimmed());
        cbox->setItemData(cbox->count() - 1, QColor( Qt::gray ), Qt::BackgroundRole); // Change colour of last added item
    }
}

void VCcommands::fillSwitchComboBox(QComboBox *cbox)
{
    if (!isIndexed) create_indexes();
    for (uint8_t s = 0; s < (NUMBER_OF_SWITCHES + NUMBER_OF_EXTERNAL_SWITCHES + 1); s++) {
        cbox->addItem(switchnames.at(s));
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
    if (sw > 0) customLabel = read_title(pg, sw); // The label for switch 0 is the page name. We don't want to see that here.
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

void VCcommands::fillCommandTableWidget(QTableWidget *table, uint8_t pg, uint8_t sw, int item)
{
    MyTable = table; // So updateCommandsTableWidget() knows the pointer to the tableWidget
    current_page = pg;
    current_switch = sw;
    current_item = item;
    if ((switchHasLabel(pg, sw)) && (item > 0)) item--;

    if (count_cmds(pg, sw) == 0) return; // Exit if there are no commands for this switch

    int cmd_no = get_cmd_number(pg, sw, item);
    Cmd_struct sel_cmd = get_cmd(cmd_no);

    cmdbyte[CB_SWITCH_TRIGGER].Type = TYPE_SWITCH_TRIGGER;
    cmdbyte[CB_SWITCH_TRIGGER].Value = get_switch_trigger_number(sel_cmd.Switch);

    if (sel_cmd.Device < NUMBER_OF_DEVICES) {
        cmdbyte[CB_DEVICE].Type = TYPE_DEVICE_SELECT;
        cmdbyte[CB_DEVICE].Value = sel_cmd.Device;
        cmdbyte[CB_TYPE].Type =  TYPE_DEVICE_COMMANDS;
        cmdbyte[CB_TYPE].Value = sel_cmd.Type - 100;
    }
    else if (sel_cmd.Device == CURRENT) {
        cmdbyte[CB_DEVICE].Type = TYPE_DEVICE_SELECT;
        cmdbyte[CB_DEVICE].Value = NUMBER_OF_DEVICES; // Set to current
        cmdbyte[CB_TYPE].Type =  TYPE_DEVICE_COMMANDS;
        cmdbyte[CB_TYPE].Value = sel_cmd.Type - 100;
    }
    else { // Common device type
        cmdbyte[CB_DEVICE].Type = TYPE_DEVICE_SELECT;
        cmdbyte[CB_DEVICE].Value = NUMBER_OF_DEVICES + 1; // Set to common
        cmdbyte[CB_TYPE].Type =  TYPE_COMMON_COMMANDS;
        cmdbyte[CB_TYPE].Value = sel_cmd.Type;
    }
    cmdbyte[CB_DATA1].Value = sel_cmd.Data1;
    cmdbyte[CB_DATA2].Value = sel_cmd.Data2;
    cmdbyte[CB_VAL1].Value = sel_cmd.Value1;
    cmdbyte[CB_VAL2].Value = sel_cmd.Value2;
    cmdbyte[CB_VAL3].Value = sel_cmd.Value3;
    cmdbyte[CB_VAL4].Value = sel_cmd.Value4;

    for (uint8_t i = 0; i < NUMBER_OF_CMD_BYTES; i++) {
        uint8_t cmd_type = cmdbyte[i].Type;
        cmdbyte[i].Min = cmdtype[cmd_type].Min;
        uint8_t max = cmdtype[cmd_type].Max;
        if (cmdtype[cmd_type].Sublist == SUBLIST_PAGE) max = last_fixed_cmd_page;
        if (cmdtype[cmd_type].Sublist == SUBLIST_MIDI_PORT) max = number_of_midi_ports;
        cmdbyte[i].Max = max;
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
    uint8_t item = current_item;
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
    if ((switchHasLabel(current_page, current_switch)) && (cmd > 0)) cmd--;

    int cmd_no = get_cmd_number(pg, sw, cmd);
    Cmd_struct my_cmd = get_cmd(cmd_no);

    my_cmd.Switch = get_switch_trigger_type(cmdbyte[CB_SWITCH_TRIGGER].Value) | sw;
    if (cmdbyte[CB_DEVICE].Value < NUMBER_OF_DEVICES) {
        my_cmd.Device = cmdbyte[CB_DEVICE].Value;
        my_cmd.Type = cmdbyte[CB_TYPE].Value + 100;
    }
    if (cmdbyte[CB_DEVICE].Value == NUMBER_OF_DEVICES) { // Current device
        my_cmd.Device = CURRENT;
        my_cmd.Type = cmdbyte[CB_TYPE].Value + 100;
    }
    if (cmdbyte[CB_DEVICE].Value == NUMBER_OF_DEVICES + 1) {
        my_cmd.Device = COMMON;
        my_cmd.Type = cmdbyte[CB_TYPE].Value;
    }
    my_cmd.Data1 = cmdbyte[CB_DATA1].Value;
    my_cmd.Data2 = cmdbyte[CB_DATA2].Value;
    my_cmd.Value1 = cmdbyte[CB_VAL1].Value;
    my_cmd.Value2 = cmdbyte[CB_VAL2].Value;
    my_cmd.Value3 = cmdbyte[CB_VAL3].Value;
    my_cmd.Value4 = cmdbyte[CB_VAL4].Value;

    write_cmd(cmd_no, my_cmd);
    //updateCommandsTableWidget();
    command_edited = false;
}

void VCcommands::checkSaved(uint8_t pg, uint8_t sw, uint8_t cmd)
{
    if (!command_edited) return;

    saveCommand(pg, sw, cmd);
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

    if ((pg == page_default) && (count_cmds(pg, sw) == 0)) createNewCommand(pg, sw); // default page can never be empty!

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
        else return (index - Number_of_pages + first_fixed_cmd_page);
        break;
    case TYPE_MIDI_PORT:
        if (index == number_of_midi_ports) return 15;
        else return index;
        break;
    default:
        return index;
    }
}

int VCcommands::indexFromValue(uint8_t type, uint16_t value) // Remove the gap in the pagenumbers
{
    switch (type) {
    case TYPE_PAGE:
        if (value < Number_of_pages) return value;
        else return (value - first_fixed_cmd_page + Number_of_pages);
        break;
    case TYPE_MIDI_PORT:
        if (value < number_of_midi_ports) return value;
        else return number_of_midi_ports; // The index to all ports
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
            cmd.Switch = destSwitch | (cmd.Switch & SWITCH_TYPE_MASK);

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
    updateCommandsTableWidget();
}

void VCcommands::copy_command_structure(QVector<Cmd_struct> *source, QVector<Cmd_struct> *dest, uint16_t size)
{
    for(uint16_t c = 0; c < size; c++) {
        dest->append(source[c]);
    }
}

void VCcommands::swapSwitches(int pg1, int sw1, int pg2, int sw2)
{
    qDebug() << "Swapping switch #" << sw1 << "on page" << pg1 << "with switch#" << sw2 << "on page" << pg2;
    bool noSwitchZero = (sw1 != 0) && (sw2 != 0);

    for (int c = 0; c < number_of_cmds; c++) {
        // Swap commands
        if ((Commands[c].Page == pg1) && ((Commands[c].Switch & SWITCH_MASK) == sw1)) {
            Commands[c].Page = pg2;
            Commands[c].Switch = sw2 | (Commands[c].Switch & SWITCH_TYPE_MASK);
        }
        else if ((Commands[c].Page == pg2) && ((Commands[c].Switch & SWITCH_MASK) == sw2)) {
            Commands[c].Page = pg1;
            Commands[c].Switch = sw1 | (Commands[c].Switch & SWITCH_TYPE_MASK);
        }

        // Swap label
        /*if (noSwitchZero) { // Cannot swap labels for switch 0, as its label contains the page name
            if ((Commands[c].Page == pg1) && (Commands[c].Switch == sw1 + LABEL)) {
                Commands[c].Page = pg2;
                Commands[c].Switch = sw2 + LABEL;
            }
            else if ((Commands[c].Page == pg2) && (Commands[c].Switch == sw2 + LABEL)) {
                Commands[c].Page = pg1;
                Commands[c].Switch = sw1 + LABEL;
            }
        }*/
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
        cmd.Switch = sw | (copyBuffer[i].Switch & SWITCH_TYPE_MASK);

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

QString VCcommands::getSwitchName(int sw) const
{
    return switchnames[sw];
}


bool VCcommands::switchHasLabel(int pg, int sw)
{
    if (sw == -1) return false;
    return (Title_index[pg][sw & SWITCH_MASK] != 0);
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
        if (is_label(cmd.Switch)) { // Check if it is a name label
            uint16_t first_title = Title_index[cmd.Page][cmd.Switch & SWITCH_MASK];
            if (first_title != 0) {// Title array is already filled!!!
                Next_cmd_index[c] = first_title; // Move first title to the Next_cmd_index
            }
            Title_index[cmd.Page][cmd.Switch & SWITCH_MASK] = c; // Add to the Title index
        }
        else { // It is a command
            uint16_t first_cmd = First_cmd_index[cmd.Page][cmd.Switch & SWITCH_MASK];
            if (first_cmd != 0) {// First command array is already filled!!!
                Next_cmd_index[c] = first_cmd; // Move first command to the Next_cmd_index
            }
            First_cmd_index[cmd.Page][cmd.Switch & SWITCH_MASK] = c; // Store the first command
        }
    }

    // Fill the indexes with internal commands
    for (uint16_t c = Fixed_commands.size(); c-- > 0; ) { //Run backwards through the EEPROM command array
        uint8_t pg = Fixed_commands[c].Page;
        uint8_t sw = Fixed_commands[c].Switch;

        if (is_label(sw)) { // Check if it is a name label
            uint16_t first_title = Title_index[pg][sw & SWITCH_MASK];
            if (first_title != 0) {// Title array is already filled!!!
                Next_internal_cmd_index[c] = first_title; // Move first title to the Next_internal_cmd_index
            }
            if (pg >= first_fixed_cmd_page) Title_index[pg][sw & SWITCH_MASK] = c | INTERNAL_CMD; // Add to the Title index
        }
        else { // It is a command
            uint16_t first_cmd = First_cmd_index[pg][sw & SWITCH_MASK];
            if (first_cmd != 0) {// First command array is already filled!!!
                Next_internal_cmd_index[c] = first_cmd; // Move first command to the Next_internal_cmd_index
            }
            if (pg >= first_fixed_cmd_page) First_cmd_index[pg][sw & SWITCH_MASK] = c | INTERNAL_CMD; // Store the first command
        }
    }

    isIndexed = true;
    qDebug() << "Indexes created";
}

uint8_t VCcommands::get_switch_trigger_number(uint8_t sw)
{
    switch(sw & SWITCH_TYPE_MASK) {
        case 0: return 0;
        case ON_RELEASE: return 1;
        case ON_LONG_PRESS: return 2;
        case ON_DUAL_PRESS: return 3;
        case ON_DUAL_PRESS | ON_RELEASE: return 4;
        case ON_DUAL_PRESS | ON_LONG_PRESS: return 5;
        default: return 0;
    }
}

uint8_t VCcommands::get_switch_trigger_type(uint8_t value)
{
    switch(value) {
        case 0: return 0;
        case 1: return ON_RELEASE;
        case 2: return ON_LONG_PRESS;
        case 3: return ON_DUAL_PRESS;
        case 4: return ON_DUAL_PRESS | ON_RELEASE;
        case 5: return ON_DUAL_PRESS | ON_LONG_PRESS;
        default: return 0;
    }
}

inline bool VCcommands::is_label(uint8_t sw) {
  return ((sw & SWITCH_TYPE_MASK) == LABEL);
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
    else if (number < Commands.size()) return Commands[number];
    else return {0,0,0,0,0,0,0,0,0,0}; // This fixed a weird crash on reading patches at startup on the regular VController
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
    uint16_t i = First_cmd_index[pg][sw & SWITCH_MASK];
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
    sw &= SWITCH_MASK;
    if (First_cmd_index[pg][sw] == 0) return get_cmd_number(0, sw, number);
    else return get_cmd_number(pg, sw, number);
}

uint16_t VCcommands::count_cmds(uint8_t pg, uint8_t sw)
{
    sw &= SWITCH_MASK;
    if ((pg >= Number_of_pages) && (pg < first_fixed_cmd_page)) return 0; // Just in case we are creating a new page

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
    uint16_t i = Title_index[pg][sw & SWITCH_MASK];
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
    if (pg >= first_fixed_cmd_page) return;

    Cmd_struct cmd;
    cmd.Page = pg;
    cmd.Switch = sw | LABEL;
    uint8_t* cmdbytes = (uint8_t*)&cmd;
    uint8_t len = title.length();
    if (len > 8) len = 8;
    for (uint8_t i = 0; i < len; i++) cmdbytes[i + 2] = title.at(i).unicode();
    for (uint8_t i = len; i < 8; i++) cmdbytes[i + 2] = 32; // ascii number for space

    uint16_t cmd_index = Title_index[pg][sw & SWITCH_MASK];
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

    uint8_t max = cmdtype[type].Max;
    if (cmdtype[type].Sublist == SUBLIST_PAGE) max = last_fixed_cmd_page;
    if (cmdtype[type].Sublist == SUBLIST_MIDI_PORT) max = number_of_midi_ports;
    cmdbyte[number].Max = max;
    cmdbyte[number].Min = cmdtype[type].Min;
    if (in_edit_mode) {
        cmdbyte[number].Value = index;
    }
}

void VCcommands::clear_cmd_bytes(uint8_t start_byte, bool in_edit_mode)
{
    for (uint8_t b = start_byte; b < NUMBER_OF_CMD_BYTES; b++) {
        set_type_and_value(b, TYPE_OFF, 0, in_edit_mode);
        cmdbyte[b].Max = 0;
        cmdbyte[b].Min = 0;
    }
}

void VCcommands::build_command_structure(uint8_t cmd_byte_no, uint8_t cmd_type, bool in_edit_mode)
{
    // Will update the status of the other command fields, according to the device, type, or number of parameters
    // Function is called after a command is loaded or the switch of the command has been pressed:

    uint8_t dev = 0;
    uint8_t my_trigger = 0;

    // *******************************************
    // **********     BYTE1 updated     **********
    // *******************************************
    if (cmd_byte_no == CB_DEVICE) { // The first byte (device select) has been changed
      // *****************************************
      // * BYTE1: Device byte updated            *
      // *****************************************
      if (cmdbyte[CB_DEVICE].Value == NUMBER_OF_DEVICES + 1) { // If Device is "Common"
        // Set command to: COMMON, NONE
        set_type_and_value(CB_TYPE, TYPE_COMMON_COMMANDS, selected_common_cmd, in_edit_mode);

        build_command_structure(CB_TYPE, cmdbyte[CB_TYPE].Type, false);

        //clear_cmd_bytes(CB_DATA1, in_edit_mode); // Clear bytes 2-7
      }
      else { // Device is not "common"
        // Set command to: <selected device>, PATCH, SELECT, <current_patch_number>
        set_type_and_value(CB_TYPE, TYPE_DEVICE_COMMANDS, selected_device_cmd, in_edit_mode);

        build_command_structure(CB_TYPE, cmdbyte[CB_TYPE].Type, false);

        if (selected_device_cmd == PATCH - 100) {
          uint8_t dev = cmdbyte[CB_DEVICE].Value;
          uint16_t patch_no = 0;
          if (dev < NUMBER_OF_DEVICES) patch_no = Device[dev]->patch_number;
          set_type_and_value(CB_DATA1, TYPE_CMDTYPE, SELECT, in_edit_mode);
          set_type_and_value(CB_DATA2, TYPE_PATCH_NUMBER, patch_no % 100, in_edit_mode);
          set_type_and_value(CB_VAL1, TYPE_PATCH_100, patch_no / 100, in_edit_mode);
          //clear_cmd_bytes(CB_VAL1, in_edit_mode); // Clear bytes 2-7
        }
      }
      //reload_cmd_menus();
      //update_page = REFRESH_PAGE;
    }

    // *******************************************
    // **********     BYTE2 updated     **********
    // *******************************************

    if (cmd_byte_no == CB_TYPE) {
      // *****************************************
      // * BYTE2: Common command byte updated    *
      // *****************************************
      if (cmd_type == TYPE_COMMON_COMMANDS) { // The common command byte has been changed
        selected_common_cmd = cmdbyte[cmd_byte_no].Value;
        switch (selected_common_cmd) {
          case PAGE:
            // Command: COMMON, PAGE, 0
            set_type_and_value(CB_DATA1, TYPE_CMDTYPE, 0, in_edit_mode);
            set_type_and_value(CB_DATA2, TYPE_PAGE, 0, in_edit_mode);
            clear_cmd_bytes(CB_VAL1, in_edit_mode); // Clear bytes 3-7
            break;
          case MIDI_PC:
            // Command: COMMON, MIDI_PC, NUMBER, CHANNEL, PORT
            set_type_and_value(CB_DATA1, TYPE_PC, 0, in_edit_mode);
            set_type_and_value(CB_DATA2, TYPE_MIDI_CHANNEL, 1, in_edit_mode);
            set_type_and_value(CB_VAL1, TYPE_MIDI_PORT, 0, in_edit_mode);
            clear_cmd_bytes(CB_VAL2, in_edit_mode); // Clear bytes 4-7
            break;
          case MIDI_CC:
            // Command: COMMON, CC_number, CC_TOGGLE_TYPE, Value1, Value2, Channel, Port
            set_type_and_value(CB_DATA1, TYPE_CC_NUMBER, 0, in_edit_mode);
            set_type_and_value(CB_DATA2, TYPE_CC_TOGGLE, 0, in_edit_mode);
            set_type_and_value(CB_VAL1, TYPE_MAX, 127, in_edit_mode);
            set_type_and_value(CB_VAL2, TYPE_MIN, 0, in_edit_mode);
            set_type_and_value(CB_VAL3, TYPE_MIDI_CHANNEL, 1, in_edit_mode);
            set_type_and_value(CB_VAL4, TYPE_MIDI_PORT, 0, in_edit_mode);
            break;
          case MIDI_NOTE:
            // Command: COMMON, MIDI_NOTE, NUMBER, VELOCITY, CHANNEL, PORT
            set_type_and_value(CB_DATA1, TYPE_NOTE_NUMBER, 0, in_edit_mode);
            set_type_and_value(CB_DATA2, TYPE_NOTE_VELOCITY, 100, in_edit_mode);
            set_type_and_value(CB_VAL1, TYPE_MIDI_CHANNEL, 1, in_edit_mode);
            set_type_and_value(CB_VAL2, TYPE_MIDI_PORT, 0, in_edit_mode);
            clear_cmd_bytes(CB_VAL3, in_edit_mode); // Clear bytes 6-7
            break;
          case SET_TEMPO:
            // Command: COMMON, SET_TEMPO, number
            set_type_and_value(CB_DATA1, TYPE_BPM, Setting.Bpm, in_edit_mode);
            clear_cmd_bytes(CB_DATA2, in_edit_mode); // Clear bytes 3-7
            break;
          default:
            // For all two byte commands
            clear_cmd_bytes(CB_DATA1, in_edit_mode); // Clear bytes 2-7
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
        selected_device_cmd = cmdbyte[cmd_byte_no].Value;
        dev = cmdbyte[CB_DEVICE].Value;
        switch (selected_device_cmd + 100) {
          case PATCH:
            // Command: <selected device>, PATCH, SELECT, <current_patch_number>
            if (dev < NUMBER_OF_DEVICES) patch_no = Device[dev]->patch_number;
            set_type_and_value(CB_DATA1, TYPE_CMDTYPE, SELECT, in_edit_mode);
            set_type_and_value(CB_DATA2, TYPE_PATCH_NUMBER, patch_no % 100, in_edit_mode);
            set_type_and_value(CB_VAL1, TYPE_PATCH_100, patch_no / 100, in_edit_mode);
            if (dev < NUMBER_OF_DEVICES) {
                cmdbyte[CB_DATA2].Max = Device[dev]->patch_max % 100;
                cmdbyte[CB_VAL1].Max = Device[dev]->patch_max / 100;
                cmdbyte[CB_DATA2].Min = Device[dev]->patch_min % 100;
                cmdbyte[CB_VAL1].Min = Device[dev]->patch_min / 100;
            }
            clear_cmd_bytes(CB_VAL2, in_edit_mode); // Clear bytes 4-7
            break;
          case PARAMETER:
            // Command: <selected device>, PARAMETER, NUMBER, TOGGLE, VALUE 1, VALUE 2
            set_type_and_value(CB_DATA1, TYPE_PARAMETER, 0, in_edit_mode);
            if (dev < NUMBER_OF_DEVICES) {
                cmdbyte[CB_DATA1].Max = Device[dev]->number_of_parameters() - 1;
                cmdbyte[CB_DATA1].Min = 0;
            }
            set_type_and_value(CB_DATA2, TYPE_TOGGLE, 1, in_edit_mode);
            set_type_and_value(CB_VAL1, TYPE_PAR_VALUE, Device[dev]->min_value(cmdbyte[CB_DATA1].Value), in_edit_mode);
            if (dev < NUMBER_OF_DEVICES) {
                cmdbyte[CB_VAL1].Max = Device[dev]->max_value(cmdbyte[CB_DATA1].Value);
                cmdbyte[CB_VAL1].Min = Device[dev]->min_value(cmdbyte[CB_DATA1].Value);
            }
            set_type_and_value(CB_VAL2, TYPE_PAR_VALUE, Device[dev]->max_value(cmdbyte[CB_DATA1].Value), in_edit_mode);
            if (dev < NUMBER_OF_DEVICES) {
                cmdbyte[CB_VAL2].Max = Device[dev]->max_value(cmdbyte[CB_DATA1].Value);
                cmdbyte[CB_VAL2].Min = Device[dev]->min_value(cmdbyte[CB_DATA1].Value);
            }
            clear_cmd_bytes(CB_VAL3, in_edit_mode); // Clear bytes 6-7
            set_default_parameter_values(in_edit_mode);
            break;
          case ASSIGN:
            // Command: <selected device>, ASSIGN, NUMBER
            set_type_and_value(CB_DATA1, TYPE_CMDTYPE_ASSIGN, 0, in_edit_mode);
            set_type_and_value(CB_DATA2, TYPE_ASSIGN, 0, in_edit_mode);
            if (dev < NUMBER_OF_DEVICES) {
                if (Device[dev]->get_number_of_assigns() > 0)
                    cmdbyte[CB_DATA2].Max = Device[dev]->get_number_of_assigns() - 1;
                else
                    cmdbyte[CB_DATA2].Max = 0;
                my_trigger = Device[dev]->trigger_follow_assign(0);
            }
            else {
                my_trigger = 0;
                cmdbyte[CB_DATA2].Max = 0;
            }
            set_type_and_value(CB_VAL1, TYPE_ASSIGN_TRIGGER, my_trigger, in_edit_mode);
            clear_cmd_bytes(CB_VAL2, in_edit_mode); // Clear bytes 3-7
            break;
          case OPEN_PAGE_DEVICE:
            // Command: <selected device>, OPEN_PAGE, 0
            set_type_and_value(CB_DATA1, TYPE_PAGE, 0, in_edit_mode);
            clear_cmd_bytes(CB_DATA2, in_edit_mode); // Clear bytes 3-7
            break;
          case SNAPSCENE:
            // Command: <selected device>, SNAPSCENE, 0
            set_type_and_value(CB_DATA1, TYPE_SNAPSCENE, 1, in_edit_mode);
            set_type_and_value(CB_DATA2, TYPE_SNAPSCENE, 0, in_edit_mode);
            set_type_and_value(CB_VAL1, TYPE_SNAPSCENE, 0, in_edit_mode);
            clear_cmd_bytes(CB_VAL2, in_edit_mode); // Clear bytes 5-7
            break;
          case LOOPER:
            // Command: <selected device>, LOOPER, 0
            set_type_and_value(CB_DATA1, TYPE_LOOPER, 2, in_edit_mode);
            clear_cmd_bytes(CB_DATA2, in_edit_mode); // Clear bytes 3-7
            break;
          case MASTER_EXP_PEDAL:
            // Command: <selected device>, MASTER_EXP_EDAL, 0
            set_type_and_value(CB_DATA1, TYPE_EXP_PEDAL, 2, in_edit_mode);
            clear_cmd_bytes(CB_DATA2, in_edit_mode); // Clear bytes 3-7
            break;
          default:
            clear_cmd_bytes(CB_DATA1, in_edit_mode); // Clear bytes 2-7
            break;
        }
        //reload_cmd_menus();
        //update_page = REFRESH_PAGE;
      }
    }

    // *******************************************
    // **********     BYTE3 updated     **********
    // *******************************************

    if (cmd_byte_no == CB_DATA1) {

      // *****************************************
      // * BYTE3: Command type updated           *
      // *****************************************
      if ((cmd_type == TYPE_CMDTYPE) || (cmd_type == TYPE_CMDTYPE_ASSIGN)) {
        switch (cmdbyte[cmd_byte_no].Value) {
          case SELECT:
            if (current_cmd_function() == PAGE) {
              set_type_and_value(CB_DATA2, TYPE_PAGE, 0, in_edit_mode);
              clear_cmd_bytes(CB_VAL1, in_edit_mode); // Clear bytes 3-7
            }
            if (current_cmd_function() == PATCH) {
              uint8_t dev = cmdbyte[CB_DEVICE].Value;
              uint16_t patch_no = 0;
              if (dev < NUMBER_OF_DEVICES) patch_no = Device[dev]->patch_number;
              set_type_and_value(CB_DATA2, TYPE_PATCH_NUMBER, patch_no % 100, in_edit_mode);
              set_type_and_value(CB_VAL1, TYPE_PATCH_100, patch_no / 100, in_edit_mode);
              clear_cmd_bytes(CB_VAL2, in_edit_mode); // Clear bytes 4-7
              if (dev < NUMBER_OF_DEVICES) {
                  cmdbyte[CB_DATA2].Max = Device[dev]->patch_max % 100;
                  cmdbyte[CB_VAL1].Max = Device[dev]->patch_max / 100;
                  cmdbyte[CB_DATA2].Min = Device[dev]->patch_min % 100;
                  cmdbyte[CB_VAL1].Min = Device[dev]->patch_min / 100;
              }
            }
            if (current_cmd_function() == ASSIGN) {
              set_type_and_value(CB_DATA2, TYPE_ASSIGN, 0, in_edit_mode);
              dev = cmdbyte[CB_DEVICE].Value;
              if (dev < NUMBER_OF_DEVICES) {
                  if (Device[dev]->get_number_of_assigns() > 0)
                      cmdbyte[CB_DATA2].Max = Device[dev]->get_number_of_assigns() - 1;
                  else
                      cmdbyte[CB_DATA2].Max = 0;
                  my_trigger = Device[dev]->trigger_follow_assign(0);
              }
              else {
                  my_trigger = 0;
                  cmdbyte[CB_DATA2].Max = 0;
              }
              set_type_and_value(CB_VAL1, TYPE_ASSIGN_TRIGGER, my_trigger, in_edit_mode);
              clear_cmd_bytes(CB_VAL2, in_edit_mode); // Clear bytes 3-7
            }
            break;
          case NEXT:
          case PREV:
            clear_cmd_bytes(CB_DATA2, in_edit_mode); // Clear bytes 3-7
            break;
          case BANKSELECT:
            set_type_and_value(CB_DATA2, TYPE_REL_NUMBER, 1, in_edit_mode);
            set_type_and_value(CB_VAL1, TYPE_BANK_SIZE, 10, in_edit_mode);
            clear_cmd_bytes(CB_VAL2, in_edit_mode); // Clear bytes 3-7
            break;
          case BANKUP:
          case BANKDOWN:
            set_type_and_value(CB_DATA2, TYPE_BANK_SIZE, 10, in_edit_mode);
            clear_cmd_bytes(CB_VAL1, in_edit_mode); // Clear bytes 3-7
            break;
        }
      }
    }

    // *****************************************
    // * BYTE3: Parameter byte updated            *
    // *****************************************
    if (cmd_type == TYPE_PARAMETER) {
      uint8_t dev = cmdbyte[CB_DEVICE].Value;
      if (dev < NUMBER_OF_DEVICES) {
          set_default_parameter_values(in_edit_mode);
      }
    }

    // *******************************************
    // **********     BYTE4 updated     **********
    // *******************************************

    if (cmd_byte_no == CB_DATA2) {

      // *****************************************
      // * BYTE4: Toggle type byte updated       *
      // *****************************************
      if (cmd_type == TYPE_TOGGLE) {
          uint8_t max = 0;
                  uint8_t min = 0;
                  if (dev < NUMBER_OF_DEVICES) {
                      max = Device[dev]->max_value(cmdbyte[CB_DATA1].Value);
                      min = Device[dev]->min_value(cmdbyte[CB_DATA1].Value);
                  }

                  switch (cmdbyte[CB_DATA2].Value) {
                  case MOMENTARY:
                  case TOGGLE:
                      cmdbyte[CB_VAL1].Title = "ON-VALUE";
                      cmdbyte[CB_VAL2].Title = "OFF-VALUE";
                      clear_cmd_bytes(CB_VAL3, in_edit_mode);
                      break;
                  case TRISTATE:
                      // Command: <selected device>, PARAMETER, NUMBER, TRISTATE, VALUE 1, VALUE 2, VALUE 3
                      set_type_and_value(CB_VAL3, TYPE_PAR_VALUE, max, in_edit_mode);
                      clear_cmd_bytes(CB_VAL4, in_edit_mode);
                      cmdbyte[CB_VAL1].Title = "VALUE 1";
                      cmdbyte[CB_VAL2].Title = "VALUE 2";
                      cmdbyte[CB_VAL3].Title = "VALUE 3";
                      break;
                  case FOURSTATE:
                      // Command: <selected device>, PARAMETER, NUMBER, TRISTATE, VALUE 1, VALUE 2, VALUE 3, VALUE 4
                      set_type_and_value(CB_VAL3, TYPE_PAR_VALUE, max, in_edit_mode);
                      set_type_and_value(CB_VAL4, TYPE_PAR_VALUE, max, in_edit_mode);
                      cmdbyte[CB_VAL1].Title = "VALUE 1";
                      cmdbyte[CB_VAL2].Title = "VALUE 2";
                      cmdbyte[CB_VAL3].Title = "VALUE 3";
                      cmdbyte[CB_VAL4].Title = "VALUE 4";
                      break;
                  case STEP:
                      // Command: <selected device>, PARAMETER, NUMBER, TRISTATE, VALUE 1, VALUE 2, VALUE3
                      cmdbyte[CB_VAL1].Title = "MIN VALUE";
                      cmdbyte[CB_VAL2].Title = "MAX VALUE";
                      set_type_and_value(CB_VAL3, TYPE_STEP, 1, in_edit_mode);
                      clear_cmd_bytes(CB_VAL4, in_edit_mode);
                      break;
                  case RANGE:
                  case UPDOWN:
                      cmdbyte[CB_VAL1].Title = "MIN VALUE";
                      cmdbyte[CB_VAL2].Title = "MAX VALUE";
                      clear_cmd_bytes(CB_VAL3, in_edit_mode);
                      break;

                  default:
                      clear_cmd_bytes(CB_VAL3, in_edit_mode);
                      break;
                  }
                  set_default_parameter_values(in_edit_mode);
      }
    }

    // *******************************************
    // **********   BYTE4 - 10 updated  **********
    // *******************************************

    if ((cmd_byte_no >= CB_DATA1) && (cmd_byte_no <= CB_VAL4)) {
      // *****************************************
      // * BYTE4+: Parameter value byte updated  *
      // *****************************************
      /*if (cmd_type == TYPE_PAR_STATE) {
        uint8_t dev = cmdbyte[CB_DEVICE].Value;
        if (dev < NUMBER_OF_DEVICES) {
          // Check if we have reached the max value
          if (cmdbyte[cmd_byte_no].Value >= Device[dev]->number_of_values(cmdbyte[CB_DATA1].Value)) cmdbyte[cmd_byte_no].Value = 0;
        }
      }*/
      // *****************************************
      // * BYTE4+: Page byte updated             *
      // *****************************************
      if (cmd_type == TYPE_PAGE) {
        if (cmdbyte[cmd_byte_no].Value == Number_of_pages) cmdbyte[cmd_byte_no].Value = first_fixed_cmd_page; // Jump over the gap between the external and internal pages
        if (cmdbyte[cmd_byte_no].Value == first_fixed_cmd_page - 1) cmdbyte[cmd_byte_no].Value = Number_of_pages - 1;
      }

      // *****************************************
      // * BYTE4+: Type patch 100 byte updated    *
      // *****************************************
      if (cmd_type == TYPE_PATCH_100) { // Check if we reached the minimum or maximum value
        uint16_t patch_no = (cmdbyte[CB_VAL1].Value * 100);
        uint8_t Dev = cmdbyte[CB_DEVICE].Value;
        if (Dev < NUMBER_OF_DEVICES) {
          if (patch_no >= Device[Dev]->patch_max) {
            cmdbyte[CB_VAL1].Value = Device[Dev]->patch_min / 100;
          }
          if (patch_no < Device[Dev]->patch_min) {
            cmdbyte[CB_VAL1].Value = Device[Dev]->patch_max / 100;
          }
        }
      }

      // *****************************************
      // * BYTE4+: Assign number byte updated    *
      // *****************************************

      if (cmd_type == TYPE_ASSIGN) {
        uint8_t dev = cmdbyte[CB_DEVICE].Value;
        if (dev < NUMBER_OF_DEVICES) {
          // Check if we have reached the max value
          if (cmdbyte[cmd_byte_no].Value >= Device[dev]->get_number_of_assigns()) cmdbyte[cmd_byte_no].Value = 0;
          // Let the trigger field follow the assign field if neccesary
          uint8_t my_trigger = Device[dev]->trigger_follow_assign(cmdbyte[cmd_byte_no].Value + 1);
          cmdbyte[CB_VAL1].Value = my_trigger;
        }
      }
    }
}

void VCcommands::set_default_parameter_values(bool in_edit_mode)
{
    uint8_t dev = cmdbyte[CB_DEVICE].Value;
      if (dev >= NUMBER_OF_DEVICES) return;
      uint8_t max = Device[dev]->max_value(cmdbyte[CB_DATA1].Value);
      uint8_t min = Device[dev]->min_value(cmdbyte[CB_DATA1].Value);
      cmdbyte[CB_VAL1].Max = max;
      cmdbyte[CB_VAL1].Min = min;
      cmdbyte[CB_VAL2].Max = max;
      cmdbyte[CB_VAL2].Min = min;
      switch (cmdbyte[CB_DATA2].Value) {
        case TRISTATE:
          if (in_edit_mode) {
            cmdbyte[CB_VAL1].Value = 0;
            cmdbyte[CB_VAL2].Value = max / 2;
            cmdbyte[CB_VAL3].Value = max;
          }
          cmdbyte[CB_VAL3].Max = max;
          cmdbyte[CB_VAL3].Min = min;
          break;
        case FOURSTATE:
          if (in_edit_mode) {
            cmdbyte[CB_VAL1].Value = 0;
            cmdbyte[CB_VAL2].Value = max / 3;
            cmdbyte[CB_VAL3].Value = (max * 2) / 3;
            cmdbyte[CB_VAL4].Value = max;
          }
          cmdbyte[CB_VAL3].Max = max;
          cmdbyte[CB_VAL3].Min = min;
          cmdbyte[CB_VAL4].Max = max;
          cmdbyte[CB_VAL4].Min = min;
          break;
      case TOGGLE:
      case MOMENTARY:
          if (in_edit_mode) {
            cmdbyte[CB_VAL1].Value = max;
            cmdbyte[CB_VAL2].Value = 0;
          }
          break;
      default:
          if (in_edit_mode) {
            cmdbyte[CB_VAL1].Value = 0;
            cmdbyte[CB_VAL2].Value = max;
          }
      }
}

uint8_t VCcommands::current_cmd_function() {
  if (cmdbyte[CB_DEVICE].Value == NUMBER_OF_DEVICES + 1) { // Common device
    return cmdbyte[CB_TYPE].Value;
  }
  else { // Current device
    return cmdbyte[CB_TYPE].Value + 100;
  }
}

void VCcommands::load_cmd_byte(QTableWidget *table, uint8_t cmd_byte_no)
{
    uint8_t cmd_type = cmdbyte[cmd_byte_no].Type;
    uint16_t max = cmdbyte[cmd_byte_no].Max;
    uint16_t min = cmdbyte[cmd_byte_no].Min;
    uint16_t value = cmdbyte[cmd_byte_no].Value;

    if (max > 255) max = 255; // We only support max values up to 255

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
        for (uint16_t i = indexFromValue(cmd_type, min); i <= indexFromValue(cmd_type, max); i++) {
            comboBox->addItem(read_cmd_sublist(cmd_byte_no, valueFromIndex(cmd_type, i - min)));
        }

        // Disable items for devices that are not supported
        if ((cmdbyte[CB_DEVICE].Value < NUMBER_OF_DEVICES) && (cmd_byte_no == CB_TYPE)) {
            const QStandardItemModel* model = qobject_cast<const QStandardItemModel*>(comboBox->model());
            for (int i = indexFromValue(cmd_type, min); i <= indexFromValue(cmd_type, max); i++) {
              QStandardItem* item = model->item(i - min);
              item->setEnabled(Device[cmdbyte[CB_DEVICE].Value]->check_command_enabled(i + 100));
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

QString VCcommands::read_cmd_sublist(uint8_t cmd_byte_no, uint16_t value)
{
    QString msg, msg1;
    uint8_t index;
    uint8_t dev = cmdbyte[CB_DEVICE].Value;
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
        patch_no = value + (cmdbyte[CB_VAL1].Value * 100);
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
        dev = cmdbyte[CB_DEVICE].Value;
        msg = "";
        if (dev < NUMBER_OF_DEVICES) msg = Device[dev]->read_parameter_name(value);
        else msg = QString::number(value);
        break;
    case SUBLIST_PAR_VALUE: // Copy the parameter state from the device
        dev = cmdbyte[CB_DEVICE].Value;
        msg = "";
        if (dev < NUMBER_OF_DEVICES) msg = Device[dev]->read_parameter_state(cmdbyte[CB_DATA1].Value, value);
        else msg = QString::number(value);
        break;
    case SUBLIST_ASSIGN:  // Copy the assign name from the device
        dev = cmdbyte[CB_DEVICE].Value;
        msg = "";
        if (dev < NUMBER_OF_DEVICES) msg = Device[dev]->read_assign_name(value);
        else msg = QString::number(value);
        break;
    case SUBLIST_TRIGGER:  // Copy the assign trigger name from the device
        dev = cmdbyte[CB_DEVICE].Value;
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
        else if ((page_index >= Number_of_pages) && (page_index < (Number_of_pages + last_fixed_cmd_page - first_fixed_cmd_page + 1))) {
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
    case SUBLIST_SWITCH:
        msg = getSwitchName(value);
        break;
    case SUBLIST_MIDI_PORT:
        msg = midi_port_names.at(value);
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

