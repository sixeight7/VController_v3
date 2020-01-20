#ifndef FRACTAL_H
#define FRACTAL_H

#include "device.h"
#include <QApplication>

// Fractal AxeFX settings:
#define AXEFX_MIDI_CHANNEL 1
#define AXEFX_MIDI_PORT 1 // Default port is MIDI1
#define AXEFX_PATCH_MIN 0
#define AXEFX_PATCH_MAX 383

// AxeFX moel numbers:
#define AXE_MODEL_STANDARD 0x00 // Axe-Fx Standard
#define AXE_MODEL_ULTRA 0x01 //Axe-Fx Ultra
#define AXE_MODEL_AF2 0x03 //Axe-Fx II
#define AXE_MODEL_FX8 0x05 //FX8
#define AXE_MODEL_XL 0x06 //Axe-Fx II XL
#define AXE_MODEL_XLP 0x07 //Axe-Fx II XL+
#define AXE_MODEL_AX8 0x08 //AX8
#define AXE_MODEL_FX8mk2 0x0A //FX8 mk2
#define AXE_MODEL_AF3 0x10 //Axe-Fx III

// Max number of patches per model - not sure if all these numbers are correct yet...
#define AXEFX1_PATCH_MAX 383
#define AXEFX1U_PATCH_MAX 383
#define AXEFX2_PATCH_MAX 383
#define FX8_PATCH_MAX 127
#define AXEFX2XL_PATCH_MAX 767
#define AXEFX2XLP_PATCH_MAX 767
#define AX8_PATCH_MAX 511
#define FX8M2_PATCH_MAX 511
#define AXEFX3_PATCH_MAX 511

class AXEFX_class : public Device_class
{
public:
    AXEFX_class(uint8_t _dev_no) : Device_class(_dev_no) {}

    virtual void init();
    virtual bool check_command_enabled(uint8_t cmd);
    virtual QString number_format(uint16_t patch_no);
    virtual QString read_parameter_name(uint16_t par_no);
    virtual QString read_parameter_state(uint16_t par_no, uint8_t value);
    virtual uint16_t number_of_parameters();
    virtual uint8_t max_value(uint16_t par_no);

    uint8_t model_number;
};

#endif // FRACTAL_H
