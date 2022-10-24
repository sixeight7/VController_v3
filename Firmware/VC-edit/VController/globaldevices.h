#ifndef GLOBALDEVICES_H
#define GLOBALDEVICES_H

#include "VController/config.h"
#include "devices/device.h"
#include "devices/gp10.h"
#include "devices/gr55.h"
#include "devices/vg99.h"
#include "devices/zg3.h"
#include "devices/zms70.h"
#include "devices/m13.h"
#include "devices/helix.h"
#include "devices/fractal.h"
#include "devices/katana.h"
#include "devices/kpa.h"
#include "devices/svl.h"
#include "devices/sy1000.h"
#include "devices/gmajor2.h"
#include "devices/mg300.h"

extern GP10_class My_GP10;
extern GR55_class My_GR55;
extern VG99_class My_VG99;
extern ZG3_class My_ZG3;
extern ZMS70_class My_ZMS70;
extern M13_class My_M13;
extern HLX_class My_HLX;
extern AXEFX_class My_AXEFX;
extern KTN_class My_KTN;
extern SVL_class My_SVL;

extern Device_class * Device[NUMBER_OF_DEVICES];

#define MAX_NUMBER_OF_SETLISTS 99
#define MAX_NUMBER_OF_SONGS 99

#endif // GLOBALDEVICES_H
