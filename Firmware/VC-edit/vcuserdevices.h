#ifndef VCUSERDEVICES_H
#define VCUSERDEVICES_H

#include "VController/globals.h"
#include "VController/globaldevices.h"
#include "customcombobox.h"
#include "customspinbox.h"
#include "customslider.h"
#include "customlineedit.h"

#include <QObject>
#include <QDialog>
#include <QTreeWidget>
#include <QTableWidget>
#include <QString>
#include <QVector>

class VCuserdevices : public QObject
{
    Q_OBJECT
public:
    explicit VCuserdevices(QObject *parent = nullptr);
    void fillTreeWidget(QTreeWidget *my_tree);
    void fillPatchNameTableWidget(int instance, QTableWidget *patchWidget);
    void fillFxAndSceneNameTableWidget(int instance, QTableWidget *patchWidget);
    void updateFxAndSceneNameTableWidget(int instance, QTableWidget *patchWidget);

    void setCurrentUserDevice(uint8_t newCurrent_user_device);
    QString read_user_item_name(uint8_t type, uint8_t USER_dev, uint16_t patch_no);
    void store_user_item_name(uint8_t type, uint8_t USER_dev, uint16_t patch_no, QString name);
    uint8_t read_user_item_data(uint8_t type, uint8_t USER_dev, uint16_t patch_no);
    void store_user_item_data(uint8_t type, uint8_t USER_dev, uint16_t patch_no, uint8_t value);
    void setGlobalChecked(bool newGlobalChecked);
    void setSelectedPatchNumber(int newSelectedPatchNumber);
    void read(const QJsonObject &json, uint8_t instance, bool readNoInstanceData);
    void readAll(const QJsonObject &json);
    void write(QJsonObject &json, uint8_t instance, bool saveNoInstanceData) const;
    void writeAll(QJsonObject &json);
    void deleteUserItemsForInstance(uint8_t instance);
    void initializeUserDeviceData(uint8_t instance, bool deviceData, bool patchNames, bool fxStates, bool fxNames, bool sceneNames, bool customColours);
    void clearPatchItems(uint8_t instance, uint16_t patch_no);
    void clearCopyBuffer(int offset);
    void copyPatchItems(uint8_t instance, uint16_t patch_no);
    void pastePatchItems(uint8_t instance, uint16_t patch_index);
    int getNumberOfUserItems();
    void cleanUpUserDeviceNameItems();
signals:
    void updateUserDeviceNameWidget();
    void updateFullName();

private slots:
    void intItemChanged(int, int, int);
    void textItemChanged(int, int, const QString &);
    void patchNameChanged(int, int, const QString &);
    void fxOrSceneNameChanged(int, int, const QString &);
    void fxStateChanged(int, int, bool);
    void fxColourChanged(int, int, int);

private:
    void addTextItem(QTreeWidget *my_tree, QTreeWidgetItem *parent, QString label, QString text, uint8_t len, uint8_t index);
    void addIntItem(QTreeWidget *my_tree, QTreeWidgetItem *parent, QString label, uint16_t number, uint16_t max, uint8_t index);
    void addOptionItem(QTreeWidget *my_tree, QTreeWidgetItem *parent, QString label, uint16_t number, uint16_t max, uint8_t index);
    int find_user_data_index(uint8_t typedev, uint16_t patch_number);
    int find_new_user_data_index();
    bool check_fx_active(uint8_t number);
    void set_fx_state(uint8_t number, bool state);
    QString getFxOrSceneName(uint8_t instance, uint8_t item_no, bool isGlobal);
    bool checkUserDeviceDataItemIsEmpty(int index);

    uint8_t current_user_device = 0;
    uint8_t fx_state = 0;
    bool globalChecked = true;
    int selectedPatchNumber = 0;
    User_device_struct current_user_device_data;
    uint16_t USER_current_device_data_patch_min;
    uint16_t USER_current_device_data_patch_max;
    QVector<User_device_name_struct> copyBuffer = {};
    int copyBufferNumberOffset;

    enum type_enum { HEADER, OPTION, VALUE, TEXT };
    struct User_device_setting_menu_struct {
        QString name;
        type_enum type;
        int sublist;
        int min;
        int max;
        const uint8_t* value;
    };

    const QVector<User_device_setting_menu_struct> VCuserDeviceSettingsMenu =
        {
            { "Device name settings", HEADER, 0, 0, 0, 0 },
            //{ "Full name", TEXT, 0, 0, 5, &current_device_data.full_name },
            //{ "Short name", TEXT, 0, 0, 5, &current_device_data.short_name },

            { "Patch number settings", HEADER, 0, 0, 0, 0 },
            //{ "Min patch number", VALUE, 0, 0, 2048, &current_device_data.patch_min },
            //{ "Max patch number", VALUE, 0, 0, 2048, &current_device_data.patch_max },
            //{ "Patch number format", TEXT, 0, 0, 5, &current_device_data.patch_format },
            //{ "Patch number mask", TEXT, 0, 0, 5, &current_device_data.patch_mask },

            { "Midi settings", HEADER, 0, 0, 0, 0 },
        { "MIDI CC number", VALUE, 0, 0, 127, &current_user_device_data.parameter_CC[0] },
        { "Max value", VALUE, 0, 0, 127, &current_user_device_data.parameter_value_max[0] },
        { "Min value", VALUE, 0, 0, 127, &current_user_device_data.parameter_value_min[0] },
        { "Min value", OPTION, 4, 0, 3, &current_user_device_data.parameter_type[0] },
        };

    const uint16_t NUMBER_OF_USER_DEVICE_SETTINGS_MENU_ITEMS = VCuserDeviceSettingsMenu.size();

    QStringList menu_sublist = {
        // Sublist 1 - 3: Booleans
        "OFF", "ON", "DETECT",

        // Sublist 4 - 10: USER device toggle types
        "MOMENTARY", "TOGGLE", "STEP", "RANGE",  "UPDOWN", "ONE SHOT", "",
    };

    QStringList colours = {"DEFAULT", "GREEN", "RED", "BLUE", "ORANGE", "CYAN", "WHITE", "YELLOW", "PURPLE", "PINK", "SOFT GREEN", "", "", "", "", "",};
#define NUMBER_OF_COLOURS 11
};

#endif // VCUSERDEVICES_H
