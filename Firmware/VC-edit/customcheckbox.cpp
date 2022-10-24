#include "customcheckbox.h"

CustomCheckBox::CustomCheckBox(QWidget *parent, uint8_t number)
    : QCheckBox{parent}
{
    my_number = number;
}

uint8_t CustomCheckBox::getMyNumber() const
{
    return my_number;
}
