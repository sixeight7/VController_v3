#include "user.h"
#include "VController/config.h"


void USER_class::init()
{
    my_instance = my_device_number - USER1;
    initialize_device_data();

    init_from_device_data();

    enabled = DEVICE_DETECT; // Default value
    my_LED_colour = 1; // Default value: green
    MIDI_port_manual = MIDI_port_number(USER_MIDI_PORT);
    MIDI_channel = USER_MIDI_CHANNEL; // Default value
    bank_number = 0; // Default value
    is_always_on = true; // Default value
    if (VC_type == VCONTROLLER) {
        my_device_page1 = USER_DEFAULT_VC_PAGE1; // Default value
        my_device_page2 = USER_DEFAULT_VC_PAGE2; // Default value
        my_device_page3 = USER_DEFAULT_VC_PAGE3; // Default value
        my_device_page4 = USER_DEFAULT_VC_PAGE4; // Default value
    }
    if (VC_type == VCMINI) {
        my_device_page1 = USER_DEFAULT_VCMINI_PAGE1; // Default value
        my_device_page2 = USER_DEFAULT_VCMINI_PAGE2; // Default value
        my_device_page3 = USER_DEFAULT_VCMINI_PAGE3; // Default value
        my_device_page4 = USER_DEFAULT_VCMINI_PAGE4; // Default value
    }
    if (VC_type == VCTOUCH) {
        my_device_page1 = USER_DEFAULT_VCTOUCH_PAGE1; // Default value
        my_device_page2 = USER_DEFAULT_VCTOUCH_PAGE2; // Default value
        my_device_page3 = USER_DEFAULT_VCTOUCH_PAGE3; // Default value
        my_device_page4 = USER_DEFAULT_VCTOUCH_PAGE4; // Default value
    }
}

void USER_class::init_from_device_data()
{
    device_name = device_data.short_name;
    full_device_name = device_data.full_name;
    patch_min = (device_data.patch_min_msb << 8) + device_data.patch_min_lsb;
    patch_max = (device_data.patch_max_msb << 8) + device_data.patch_max_lsb;
}

void USER_class::initialize_device_data()
{
    device_data = default_device_data;
    char number_char = '1' + my_instance;
    if (my_instance == 9) number_char = '0';
    device_data.full_name[12] = number_char; // Replace X in default full name.
    device_data.short_name[4] = number_char; // Replace X in default short name.
}

QString USER_class::number_format(uint16_t number)
{
    return build_patch_number(number, device_data.first_patch_format, device_data.last_patch_format);
}

bool USER_class::check_command_enabled(uint8_t cmd)
{
    switch (cmd) {
    case PATCH:
    case PARAMETER:
    //case ASSIGN:
    //case MUTE:
    case OPEN_PAGE_DEVICE:
    case OPEN_NEXT_PAGE_OF_DEVICE:
    case MASTER_EXP_PEDAL:
    case TOGGLE_EXP_PEDAL:
    case SNAPSCENE:
    case LOOPER:
        return true;
    }
    return false;
}

void USER_class::set_device_data(User_device_struct *data)
{
    device_data = *data;
    init_from_device_data();
}

void USER_class::set_full_name(QString name)
{
    uint8_t len = name.length();
    if (len > 16) len = 16;
    for (uint8_t c = 0; c < len; c++) device_data.full_name[c] = name[c].toLatin1();
    for (uint8_t c = len; c < 16; c++) device_data.full_name[c] = ' ';
}

void USER_class::set_short_name(QString name)
{
    uint8_t len = name.length();
    if (len > 6) len = 6;
    for (uint8_t c = 0; c < len; c++) device_data.short_name[c] = name[c].toLatin1();
    for (uint8_t c = len; c < 6; c++) device_data.short_name[c] = ' ';
}

QString USER_class::read_parameter_name(uint16_t par_no)
{
    uint8_t typedev = (USER_DEVICE_FX_NAME_TYPE << 4) + (my_instance & 0x0F);
    int index = find_user_data_index(typedev, par_no << 11);
    //qDebug() << "Checking" << type << USER_dev << patch_no << index;
    if (index != DATA_INDEX_NOT_FOUND) {
        QString name = "";
        for (uint8_t c = 0; c < 12; c++) name += (char) User_device_data_item[index].name[c];
        return name.trimmed();
    }
    else {
        return par_name[par_no];
    }
}

QString USER_class::read_parameter_name_for_table(uint16_t par_no)
{
    return par_name[par_no];
}

int USER_class::find_user_data_index(uint8_t typedev, uint16_t patch_number)
{
    uint8_t patch_msb = patch_number >> 8;
    uint8_t patch_lsb = patch_number & 0xFF;
    for (int i = 0; i < User_device_data_item.size(); i++) {
        if ((User_device_data_item[i].type_and_dev == typedev) && (User_device_data_item[i].patch_msb == patch_msb) && (User_device_data_item[i].patch_lsb == patch_lsb)) return i;
    }
    return DATA_INDEX_NOT_FOUND;
}


uint16_t USER_class::number_of_parameters()
{
    return USER_NUMBER_OF_CC_PARAMETERS;
}

uint8_t USER_class::get_number_of_snapscenes()
{
    if (device_data.parameter_CC[USER_PAR_SCENE_SELECT] > 0) {
        return device_data.parameter_value_max[USER_PAR_SCENE_SELECT] + 1;
    }
    else {
        return 0;
    }
}
