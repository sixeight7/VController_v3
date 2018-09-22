#include "cmdmodel.h"

CmdModel::CmdModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

CmdModel::CmdModel(QList<Cmd_struct> commands, QObject *parent)
    : QAbstractTableModel(parent)
    , commands(commands)
{
}

int CmdModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return commands.size();
}

int CmdModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 10;
}

QVariant CmdModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= commands.size() || index.row() < 0)
        return QVariant();

    if (role == Qt::DisplayRole) {
        const auto &contact = commands.at(index.row());

        switch (index.column()) {
            case 0: return contact.Page;
            case 1: return contact.Switch;
            case 2: return contact.Type;
            case 3: return contact.Device;
            case 4: return contact.Data1;
            case 5: return contact.Data2;
            case 6: return contact.Value1;
            case 7: return contact.Value2;
            case 8: return contact.Value3;
            case 9: return contact.Value4;
        }
    }
    return QVariant();
}

QVariant CmdModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
            case 0:  return tr("Page");
            case 1:  return tr("Switch");
            case 2:  return tr("Type");
            case 3:  return tr("Device");
            case 4:  return tr("Data1");
            case 5:  return tr("Data2");
            case 6:  return tr("Value1");
            case 7:  return tr("Value2");
            case 8:  return tr("Value3");
            case 9:  return tr("Value4");

            default:
                return QVariant();
        }
    }
    return QVariant();
}

bool CmdModel::insertRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginInsertRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row)
        commands.insert(position, { uint8_t(), uint8_t(), uint8_t(), uint8_t(), uint8_t(),
                                    uint8_t(), uint8_t(), uint8_t(), uint8_t(), uint8_t(),});

    endInsertRows();
    return true;
}

bool CmdModel::removeRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row)
        commands.removeAt(position);

    endRemoveRows();
    return true;
}

bool CmdModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        int row = index.row();

        auto contact = commands.value(row);

        switch (index.column()) {
            case 0: contact.Page = value.toInt(); break;
            case 1: contact.Switch = value.toInt(); break;
            case 2: contact.Type = value.toInt(); break;
            case 3: contact.Device = value.toInt(); break;
            case 4: contact.Data1 = value.toInt(); break;
            case 5: contact.Data2 = value.toInt(); break;
            case 6: contact.Value1 = value.toInt(); break;
            case 7: contact.Value2 = value.toInt(); break;
            case 8: contact.Value3 = value.toInt(); break;
            case 9: contact.Value4 = value.toInt(); break;
            default: return false;
        }

        commands.replace(row, contact);
        emit(dataChanged(index, index));

        return true;
    }

    return false;
}

Qt::ItemFlags CmdModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

QList<Cmd_struct> CmdModel::getCommands() const
{
    return commands;
}
