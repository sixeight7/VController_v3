#include "scenedialog.h"
#include "ui_scenedialog.h"
#include "VController/globaldevices.h"
#include "VController/globals.h"

scenedialog::scenedialog(QWidget *parent, int _index) :
    QDialog(parent),
    ui(new Ui::scenedialog)
{
    ui->setupUi(this);
    my_index = _index;
    uint8_t my_type = Device_patches[my_index][0];
    if ((my_type > 0) && (my_type <= NUMBER_OF_DEVICES)) {
      uint8_t dev = my_type - 1;

      // Load scene names
      ui->lineEditScene1->setText(Device[dev]->read_scene_name_from_buffer(my_index, 0));
      ui->lineEditScene2->setText(Device[dev]->read_scene_name_from_buffer(my_index, 1));
      ui->lineEditScene3->setText(Device[dev]->read_scene_name_from_buffer(my_index, 2));
      ui->lineEditScene4->setText(Device[dev]->read_scene_name_from_buffer(my_index, 3));
      ui->lineEditScene5->setText(Device[dev]->read_scene_name_from_buffer(my_index, 4));
      ui->lineEditScene6->setText(Device[dev]->read_scene_name_from_buffer(my_index, 5));
      ui->lineEditScene7->setText(Device[dev]->read_scene_name_from_buffer(my_index, 6));
      ui->lineEditScene8->setText(Device[dev]->read_scene_name_from_buffer(my_index, 7));
    }
}

scenedialog::~scenedialog()
{
    delete ui;
}

void scenedialog::on_buttonBox_accepted()
{
    uint8_t my_type = Device_patches[my_index][0];
    if ((my_type > 0) && (my_type <= NUMBER_OF_DEVICES)) {
      uint8_t dev = my_type - 1;
      // Save scene names
      Device[dev]->store_scene_name_to_buffer(my_index, 0, ui->lineEditScene1->text());
      Device[dev]->store_scene_name_to_buffer(my_index, 1, ui->lineEditScene2->text());
      Device[dev]->store_scene_name_to_buffer(my_index, 2, ui->lineEditScene3->text());
      Device[dev]->store_scene_name_to_buffer(my_index, 3, ui->lineEditScene4->text());
      Device[dev]->store_scene_name_to_buffer(my_index, 4, ui->lineEditScene5->text());
      Device[dev]->store_scene_name_to_buffer(my_index, 5, ui->lineEditScene6->text());
      Device[dev]->store_scene_name_to_buffer(my_index, 6, ui->lineEditScene7->text());
      Device[dev]->store_scene_name_to_buffer(my_index, 7, ui->lineEditScene8->text());
    }
}

